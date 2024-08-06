// Copyright (c) 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/opt/private_to_local_pass.h"

#include <memory>
#include <utility>
#include <vector>

#include "source/opt/ir_context.h"
#include "source/spirv_constant.h"

namespace spvtools {
namespace opt {
namespace {
constexpr uint32_t kVariableStorageClassInIdx = 0;
constexpr uint32_t kSpvTypePointerTypeIdInIdx = 1;
constexpr uint32_t kEntryPointFunctionIdInIdx = 1;
}  // namespace

Pass::Status PrivateToLocalPass::Process() {
  bool modified = false;

  // Private variables require the shader capability.  If this is not a shader,
  // there is no work to do.
  if (context()->get_feature_mgr()->HasCapability(spv::Capability::Addresses))
    return Status::SuccessWithoutChange;

  std::vector<std::pair<Instruction*, Function*>> variables_to_move;
  std::unordered_set<uint32_t> localized_variables;
  for (auto& inst : context()->types_values()) {
    if (inst.opcode() != spv::Op::OpVariable) {
      continue;
    }

    if (spv::StorageClass(inst.GetSingleWordInOperand(
            kVariableStorageClassInIdx)) != spv::StorageClass::Private) {
      continue;
    }

    // TODO: Handle all functions.
    // TODO: Might want to return the map from entry points to functions.
    std::set<Function*> target_functions = FindLocalFunctions(inst);
    if (!target_functions.empty()) {
      variables_to_move.push_back({&inst, *(target_functions.begin())});
    }
  }

  modified = !variables_to_move.empty();
  for (auto p : variables_to_move) {
    if (!CopyVariable(p.first, p.second)) {
      return Status::Failure;
    }
    localized_variables.insert(p.first->result_id());
  }

  if (get_module()->version() >= SPV_SPIRV_VERSION_WORD(1, 4)) {
    // In SPIR-V 1.4 and later entry points must list private storage class
    // variables that are statically used by the entry point. Go through the
    // entry points and remove any references to variables that were localized.
    for (auto& entry : get_module()->entry_points()) {
      std::vector<Operand> new_operands;
      for (uint32_t i = 0; i < entry.NumInOperands(); ++i) {
        // Execution model, function id and name are always kept.
        if (i < 3 ||
            !localized_variables.count(entry.GetSingleWordInOperand(i))) {
          new_operands.push_back(entry.GetInOperand(i));
        }
      }
      if (new_operands.size() != entry.NumInOperands()) {
        entry.SetInOperands(std::move(new_operands));
        context()->AnalyzeUses(&entry);
      }
    }
  }

  return (modified ? Status::SuccessWithChange : Status::SuccessWithoutChange);
}

std::set<Function*> PrivateToLocalPass::FindLocalFunctions(const Instruction& inst) const {
  // Create a map of entry points to the function id containing the first use of the instruction. There
  // must only be one function per entry point if we wish to substitute the private variable.
  std::unordered_map<Function*, Function*> ep_to_use {};

  auto const result_id = inst.result_id();
  context()->get_def_use_mgr()->ForEachUser(
    result_id,
    [&ep_to_use, &inst, this](Instruction* use) {
      BasicBlock* current_block = context()->get_instr_block(use);
      if (current_block == nullptr) {
        return;
      }

      // If use is invalid, then remove all references to the current functions.
      Function* current_function = current_block->GetParent();
      if (!IsValidUse(use)) {
        for (auto iter = std::begin(ep_to_use); iter != std::end(ep_to_use);) {
          if (iter->second == current_function) {
            iter = ep_to_use.erase(iter);
          }
        }
        return;
      }

      // Find all entry points that can reach the use instruction.
      std::unordered_set<Function*> entry_points;
      std::set<Function*> visited_ids;
      FindEntryPointFuncs(current_function, entry_points, visited_ids);

      // Update the map of entry points. If the function isn't found, then add it. If the function is found,
      // then it must match the current function; otherwise, substitution will not be allowed.  
      for (auto const entry_point : entry_points) {
        auto const ep_iter = ep_to_use.find(entry_point);
        if(ep_iter == std::end(ep_to_use)) {
          ep_to_use[entry_point] = current_function;
        } else if(ep_iter->second != current_function) {
          return;
        }
      }
    });
    
    // TODO: Return map and avoid the copy?
    // Return target functions that can substitute the variable.
    std::set<Function*> target_functions {};
    for(auto const [key, value] : ep_to_use) {
      target_functions.insert(value);
    }
    
  return target_functions;
}  // namespace opt

bool PrivateToLocalPass::MoveVariable(Instruction* variable,
                                      Function* function) {
  // The variable needs to be removed from the global section, and placed in the
  // header of the function.  First step remove from the global list.
  variable->RemoveFromList();
  std::unique_ptr<Instruction> var(variable);  // Take ownership.
  context()->ForgetUses(variable);

  // Update the storage class of the variable.
  variable->SetInOperand(kVariableStorageClassInIdx,
                         {uint32_t(spv::StorageClass::Function)});

  // Update the type as well.
  uint32_t new_type_id = GetNewType(variable->type_id());
  if (new_type_id == 0) {
    return false;
  }
  variable->SetResultType(new_type_id);

  // Place the variable at the start of the first basic block.
  context()->AnalyzeUses(variable);
  context()->set_instr_block(variable, &*function->begin());
  function->begin()->begin()->InsertBefore(std::move(var));

  // Update uses where the type may have changed.
  return UpdateUses(variable);
}

// Copy |variable| from the private storage class to the function storage
// class of |function|. Returns false if the variable could not be moved.
bool PrivateToLocalPass::CopyVariable(Instruction* variable, Function* function) {

  // TODO: See CloneSameBlockOps.

  // Clone the variable.
  std::unique_ptr<Instruction> new_variable(variable->Clone(context()));

  // Update the storage class of the new variable.
  new_variable->SetInOperand(kVariableStorageClassInIdx,
                         {uint32_t(spv::StorageClass::Function)});

  // Update the type of the new variable.
  uint32_t new_type_id = GetNewType(new_variable->type_id());
    if (new_type_id == 0) {
    return false;
  }
  new_variable->SetResultType(new_type_id);

  get_decoration_mgr()->CloneDecorations(new_variable->result_id(), new_type_id);

  context()->AnalyzeUses(new_variable.get());
  context()->set_instr_block(new_variable.get(), &*function->begin());
  function->begin()->begin()->InsertBefore(std::move(new_variable));

  // function->begin()->AddInstruction(std::move(new_variable));

  // TODO: // After all copies of variable have been made.
// variable->RemoveFromList();
// context()->ForgetUses(variable);

  // Update uses where the type may have changed.
  return UpdateUses(new_variable.get());
}

uint32_t PrivateToLocalPass::GetNewType(uint32_t old_type_id) {
  auto type_mgr = context()->get_type_mgr();
  Instruction* old_type_inst = get_def_use_mgr()->GetDef(old_type_id);
  uint32_t pointee_type_id =
      old_type_inst->GetSingleWordInOperand(kSpvTypePointerTypeIdInIdx);
  uint32_t new_type_id =
      type_mgr->FindPointerToType(pointee_type_id, spv::StorageClass::Function);
  if (new_type_id != 0) {
    context()->UpdateDefUse(context()->get_def_use_mgr()->GetDef(new_type_id));
  }
  return new_type_id;
}

bool PrivateToLocalPass::IsValidUse(const Instruction* inst) const {
  // The cases in this switch have to match the cases in |UpdateUse|.
  // If we don't know how to update it, it is not valid.
  if (inst->GetCommonDebugOpcode() == CommonDebugInfoDebugGlobalVariable) {
    return true;
  }
  switch (inst->opcode()) {
    case spv::Op::OpLoad:
    case spv::Op::OpStore:
    case spv::Op::OpImageTexelPointer:  // Treat like a load
      return true;
    case spv::Op::OpAccessChain:
      return context()->get_def_use_mgr()->WhileEachUser(
          inst, [this](const Instruction* user) {
            if (!IsValidUse(user)) return false;
            return true;
          });
    case spv::Op::OpName:
      return true;
    default:
      return spvOpcodeIsDecoration(inst->opcode());
  }
}

bool PrivateToLocalPass::UpdateUse(Instruction* inst, Instruction* user) {
  // The cases in this switch have to match the cases in |IsValidUse|.  If we
  // don't think it is valid, the optimization will not view the variable as a
  // candidate, and therefore the use will not be updated.
  if (inst->GetCommonDebugOpcode() == CommonDebugInfoDebugGlobalVariable) {
    context()->get_debug_info_mgr()->ConvertDebugGlobalToLocalVariable(inst,
                                                                       user);
    return true;
  }
  switch (inst->opcode()) {
    case spv::Op::OpLoad:
    case spv::Op::OpStore:
    case spv::Op::OpImageTexelPointer:  // Treat like a load
      // The type is fine because it is the type pointed to, and that does not
      // change.
      break;
    case spv::Op::OpAccessChain: {
      context()->ForgetUses(inst);
      uint32_t new_type_id = GetNewType(inst->type_id());
      if (new_type_id == 0) {
        return false;
      }
      inst->SetResultType(new_type_id);
      context()->AnalyzeUses(inst);

      // Update uses where the type may have changed.
      if (!UpdateUses(inst)) {
        return false;
      }
    } break;
    case spv::Op::OpName:
    case spv::Op::OpEntryPoint:  // entry points will be updated separately.
      break;
    default:
      assert(spvOpcodeIsDecoration(inst->opcode()) &&
             "Do not know how to update the type for this instruction.");
      break;
  }
  return true;
}

bool PrivateToLocalPass::UpdateUses(Instruction* inst) {
  uint32_t id = inst->result_id();
  std::vector<Instruction*> uses;
  context()->get_def_use_mgr()->ForEachUser(
      id, [&uses](Instruction* use) { uses.push_back(use); });

  for (Instruction* use : uses) {
    if (!UpdateUse(use, inst)) {
      return false;
    }
  }
  return true;
}

bool PrivateToLocalPass::IsEntryPointFunc(const Function* func) const {
  for (auto& entry_point : get_module()->entry_points()) {
    if (entry_point.GetSingleWordInOperand(kEntryPointFunctionIdInIdx) ==
      func->result_id()) {
      return true;
    }
  }

  return false;
}

// TODO: Remove?
Instruction PrivateToLocalPass::GetEntryPointFunc(const Function& func) const {
  // if(IsEntryPointFunc(func)) {
  //   return func.DefInst();
  // }

  Instruction* ep_func {nullptr};
  context()->get_def_use_mgr()->WhileEachUser(func.result_id(),
    [&ep_func](Instruction* use) {
      switch (use->opcode()) {
        case spv::Op::OpFunctionCall:
          ep_func = use;
          return false;
          break;
        default:
          return true;
          break;
      };
    });
  
  return *ep_func;
}

bool PrivateToLocalPass::IsEntryPointFunc(const uint32_t& func_id) const {
  for (auto& entry_point : get_module()->entry_points()) {
    if (entry_point.GetSingleWordInOperand(kEntryPointFunctionIdInIdx) == func_id) {
      return true;
    }
  }

  return false;
}

// A function may be reached from more than one entry point.
void PrivateToLocalPass::FindEntryPointFuncs(Function* func,
    std::unordered_set<Function*>& entry_points,
    std::set<Function*>& visited_funcs) const {
  // Ignore cycles. Stop if we've visited this function already.
  if(visited_funcs.find(func) != std::end(visited_funcs)) {
    return;
  } else {
    visited_funcs.insert(func);
  }

  if(IsEntryPointFunc(func)) {
    entry_points.insert(func);
  }

  context()->get_def_use_mgr()->ForEachUser(func->result_id(), [this, &entry_points, &visited_funcs](Instruction* use) {
    switch (use->opcode()) {
      case spv::Op::OpFunctionCall: {
        auto current_function = context()->get_instr_block(use)->GetParent();
        FindEntryPointFuncs(current_function, entry_points, visited_funcs);
        break;
      }
      default:
        break;
    };
  });
}

}  // namespace opt
}  // namespace spvtools
