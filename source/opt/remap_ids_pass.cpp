// Copyright (c) 2025 LunarG Inc.
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

#include "source/opt/remap_ids_pass.h"

namespace spv {
  static inline constexpr spv::Id NoResult = 0;
}

namespace spvtools {
namespace opt {

// TODO: There might be helpful code on the compact ids pass.

Pass::Status RemapIdsPass::Process() {
  // initialize the new id map
  new_id_.resize(GetBound(), unused_);

  // scan the ids and set to unmapped
  ScanIds();

  // TODO: debug/remove
  // PrintNewIds();

  // create new ids for types and consts
  RemapTypeAndConst();

  // TODO: debug/remove
  PrintNewIds();

  // create new ids for names
  RemapNames();

  // TODO: remap function ids

  // TODO: remap remainder ids

  // TODO: apply mapping
  ApplyMap();

  // update bound in header
  UpdateBound();

  return Status::SuccessWithChange;
}

void RemapIdsPass::ScanIds() {
  get_module()->ForEachInst([this](Instruction* inst) {
    // look for types and consts
    if (IsTypeOp(inst->opcode()) || IsConstOp(inst->opcode())) {
      type_and_const_ids_.push_back(inst->result_id());
      SetNewId(inst->result_id(), unmapped_);
    }
    // look for names.
    else if (inst->opcode() == spv::Op::OpName) {
      // store name string in map so that we can compute the hash later
      auto const name = inst->GetOperand(1).AsString();
      auto const target = inst->GetSingleWordInOperand(0);
      name_ids_[name] = target;
      // TODO: unmapped should be set where ever the target id is created; shouldn't need
      // to set it here.
      SetNewId(target, unmapped_);
    }
    // look for function ids
    else if (inst->opcode() == spv::Op::OpFunction) {
      auto const res_id = inst->result_id();
      function_ids_.push_back(res_id);
    }
    // look for remaining result ids
    else if (inst->HasResultId()) {
      auto const res_id = inst->result_id();
      remainders_ids.push_back(res_id);
    }
  });
}

void RemapIdsPass::RemapTypeAndConst(){
  // remap type ids
  static constexpr std::uint32_t softTypeIdLimit = 3011; // small prime.
  static constexpr std::uint32_t firstMappedID   = 8;    // offset into ID space
  for (auto const id : type_and_const_ids_) {
    // compute the hash value
    auto const hash_value = (HashType(id) % softTypeIdLimit) + firstMappedID;

    if (IsOldIdUnmapped(id)) {
      SetNewId(id, NextUnusedId(hash_value % softTypeIdLimit + firstMappedID));
    }
  }
}

void RemapIdsPass::RemapNames() {
  static constexpr std::uint32_t softTypeIdLimit = 3011;  // small prime.
  static constexpr std::uint32_t firstMappedID   = 3019;  // offset into ID space

  for (auto const id : name_ids_) {
    spv::Id hash_value = 1911;
    for (const char c : id.first) {
      hash_value = hash_value * 1009 + c;
    }

    if (IsOldIdUnmapped(id.second)) {
      SetNewId(id.second, NextUnusedId(hash_value % softTypeIdLimit + firstMappedID));
    }
  }
}

void RemapIdsPass::RemapFunctions() {
  static constexpr std::uint32_t softTypeIdLimit = 19071;  // small prime.
  static constexpr std::uint32_t firstMappedID   =  6203;  // offset into ID space
  // Window size for context-sensitive canonicalization values
  // Empirical best size from a single data set.  TODO: Would be a good tunable.
  // We essentially perform a little convolution around each instruction,
  // to capture the flavor of nearby code, to hopefully match to similar
  // code in other modules.
  static const unsigned windowSize = 2;

  // TODO: finish
  (void) softTypeIdLimit;
  (void) firstMappedID;
  (void) windowSize;
}

void RemapIdsPass::ApplyMap() {
  for(auto const id : type_and_const_ids_) {
    ReplaceId(id, GetNewId(id));
  }

  // TODO: debug/remove
  // auto const id = type_and_const_ids_.front();
  // ReplaceId(id, GetNewId(id));
}

// TODO: This needs to be delayed until the entire mapping is determined. Move
// this to the ReplaceId function.
// TODO: We need to update the definition before we can ReplaceAllUsesWith.
// auto const inst = get_def_use_mgr()->GetDef(id);
// context()->ReplaceAllUsesWith(inst->result_id(), hash_value);
// get_def_use_mgr()->AnalyzeInstUse(inst);
// TODO: We can't just assign IDs in spirv-tools. We either need to
// create a new interface or work with context()->TakeNextId().
void RemapIdsPass::ReplaceId(spv::Id old_id, spv::Id new_id) {
  // TODO: We need to update the definition before we can ReplaceAllUsesWith.
  auto const inst = get_def_use_mgr()->GetDef(old_id);
  inst->SetResultId(new_id);
  get_def_use_mgr()->AnalyzeInstDef(inst);

  context()->ReplaceAllUsesWith(old_id, new_id);

  get_def_use_mgr()->AnalyzeInstUse(inst);
}

// Return true if this opcode defines a type
bool RemapIdsPass::IsTypeOp(spv::Op opCode) const
{
    bool is_type_op = false;
    switch (opCode) {
      case spv::Op::OpTypeVoid:
      case spv::Op::OpTypeBool:
      case spv::Op::OpTypeInt:
      case spv::Op::OpTypeFloat:
      case spv::Op::OpTypeVector:
      case spv::Op::OpTypeMatrix:
      case spv::Op::OpTypeImage:
      case spv::Op::OpTypeSampler:
      case spv::Op::OpTypeArray:
      case spv::Op::OpTypeRuntimeArray:
      case spv::Op::OpTypeStruct:
      case spv::Op::OpTypeOpaque:
      case spv::Op::OpTypePointer:
      case spv::Op::OpTypeFunction:
      case spv::Op::OpTypeEvent:
      case spv::Op::OpTypeDeviceEvent:
      case spv::Op::OpTypeReserveId:
      case spv::Op::OpTypeQueue:
      case spv::Op::OpTypeSampledImage:
      case spv::Op::OpTypePipe:
        is_type_op = true;
        break;
      default:
        break;
    }
    return is_type_op;
}

// Return true if this opcode defines a constant
bool RemapIdsPass::IsConstOp(spv::Op opCode) const
{
  bool is_const_op = false;
  switch (opCode) {
    case spv::Op::OpConstantSampler:
      context()->consumer()(SPV_MSG_ERROR, "", {0, 0, 0}, "unimplemented constant type");
      is_const_op = true;
      break;
    case spv::Op::OpConstantNull:
    case spv::Op::OpConstantTrue:
    case spv::Op::OpConstantFalse:
    case spv::Op::OpConstantComposite:
    case spv::Op::OpConstant:
      is_const_op = true;
      break;
    default:
      break;
  }
  return is_const_op;
}

spv::Id RemapIdsPass::GetBound() const {
  return context()->module()->id_bound();
}

void RemapIdsPass::UpdateBound() {
  // TODO: remove the following line; use ComputeIdBound instead
  // context()->module()->SetIdBound(largest_new_id_);

  context()->module()->SetIdBound(context()->module()->ComputeIdBound());

  // TODO: the compact ids pass does this; why?
  context()->ResetFeatureManager();
}

spv::Id RemapIdsPass::NextUnusedId(spv::Id id)
{
    // search for an unused id
    while (IsNewIdMapped(id)) {
      ++id;
    }

    return id;
}

void RemapIdsPass::BuildLocalMaps() {
  // TODO: Move the initialization of the member variables to the constructor?
  // mapped_.clear();
  // id_map_l_.clear();
  // fn_pos_.clear();
  // fn_calls_.clear();
  // type_const_pos_.clear();
  // id_pos_r_.clear();
  // entry_point_ = spv::NoResult;
  // largest_new_id_ = 0;

  // id_map_l_.resize(bound(), spv::NoResult);

  // For efficiency, reserve name map space.  It can grow if needed.
  name_map_.reserve(32);

  // Make a pass over all the instructions and process them given appropriate functions
  get_module()->ForEachInst([this](Instruction* inst) {
    ProcessInstruction(inst);
  });

  // TODO: Use context()->ReplaceAllUsesWith(inst->result_id(), replId); See dead_branch_elim_pass.cpp.
}

void RemapIdsPass::ProcessInstruction(Instruction* inst) {
  auto opCode = inst->opcode();
  // if (instFn(opCode)) {
  //   return;
  // }
  // TODO: Process the instruction inline.
  if (inst->HasResultType()) {

  }

  inst->HasResultType();
  inst->HasResultId();

  // Process instruction type id.
  if (inst->HasResultType()) {
    SetNewId(inst->type_id(), unmapped_);
  }

  // Process instruction result id.
  if (inst->HasResultId()) {
    SetNewId(inst->result_id(), unmapped_);
  }

  // Extended instructions: currently, assume everything is an ID.
  // TODO: add whatever data we need for exceptions to that
  if (opCode == spv::Op::OpExtInst) {
    // TODO: Handle extended instructions.
  }

  // Store IDs from instruction in our map
  for (uint32_t op = 0; op < inst->NumOperands(); ++op) {
    // SpecConstantOp is special: it includes the operands of another opcode which is
    // given as a literal in the 3rd word.  We will switch over to pretending that the
    // opcode being processed is the literal opcode value of the SpecConstantOp.  See the
    // SPIRV spec for details.  This way we will handle IDs and literals as appropriate for
    // the embedded op.
    if (opCode == spv::Op::OpSpecConstantOp) {
      if (op == 0) {
        opCode = get_def_use_mgr()->GetDef(inst->GetSingleWordOperand(op))->opcode();
      }
      continue;
    }

    switch(inst->GetOperand(op).type) {
      case SPV_OPERAND_TYPE_ID: //spv::OperandId:
      case SPV_OPERAND_TYPE_SCOPE_ID: // spv::OperandScope:
      case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: // spv::OperandMemorySemantics:
        // TODO: OperandVariableLiteralId was used for the label <id>s of OpSwitch. It should now be
        // treated as a SPV_OPERAND_TYPE_ID with a special case for OpSwitch.
        switch(opCode) {
          // TODO: Do I really need to process the remaining operands at the same time? Why not let
          // the outer loop process them one at a time?
          case spv::Op::OpSwitch:
            // Skip the first literal operand. Process only label operands.
            op++;
            for (; op < inst->NumOperands(); op += 2) {
              SetNewId(inst->GetSingleWordOperand(op));
            }
            return;
          default:
            SetNewId(inst->GetSingleWordOperand(op));
            break;
        }
        break;
      case SPV_OPERAND_TYPE_VARIABLE_ID: // spv::OperandVariableIds:
        // TODO: Do I really need to process the remaining operands at the same time? Why not let
        // the outer loop process them one at a time?
        for (; op < inst->NumOperands(); ++op) {
          SetNewId(inst->GetSingleWordOperand(op));
        } 
        return;
      
      // TODO: OperandVariableLiterals was being used to update the number of remaining operands.
      // It should now be safe to ignore because our loop will handle it.

      default:
        context()->consumer()(SPV_MSG_ERROR, "", {0, 0, 0}, "Unhandled Operand Type.");
        break;
    }
  }
}

void RemapIdsPass::MapTypeConst() {
}

void RemapIdsPass::SetNewId(spv::Id id, spv::Id new_id) {
  if (id > GetBound()) {
    auto const message = std::string("ID out of range: ") + std::to_string(id);
    context()->consumer()(SPV_MSG_ERROR, "", {0, 0, 0}, message.c_str());
  }

  if (id >= new_id_.size()) {
    new_id_.resize(id+1, unused_);
  }

  if (new_id != unmapped_ && new_id != unused_) {
    if (IsOldIdUnused(id)) {
      auto const message = std::string("ID unused in module: ") + std::to_string(id);
      context()->consumer()(SPV_MSG_ERROR, "", {0, 0, 0}, message.c_str());
    }

    if (!IsOldIdUnmapped(id)) {
      auto const message = std::string("ID already mapped: ") + std::to_string(id) + " -> "
        + std::to_string(GetNewId(id));
      context()->consumer()(SPV_MSG_ERROR, "", {0, 0, 0}, message.c_str());
    }

    if (IsNewIdMapped(new_id)) {
      auto const message = std::string("ID already used in module: ") + std::to_string(new_id);
      context()->consumer()(SPV_MSG_ERROR, "", {0, 0, 0}, message.c_str());
    }

    SetMapped(new_id);
  }

  new_id_[id] = new_id;

  // TODO: For debug use. Change message type to SPV_MSG_DEBUG or remove.
  // {
  //   auto const message = std::string("map: ") + std::to_string(id) + " -> " + std::to_string(new_id);
  //   context()->consumer()(SPV_MSG_INFO, "", {0, 0, 0}, message.c_str());
  // }
}

// Hash types to canonical values.  This can return ID collisions (it's a bit
// inevitable): it's up to the caller to handle that gracefully.
spv::Id RemapIdsPass::HashType(spv::Id id) const
{
  spv::Id value = 0;

  auto const inst = get_def_use_mgr()->GetDef(id);
  auto const op_code = inst->opcode();
  switch (op_code) {
    case spv::Op::OpTypeVoid:
      value = 0;
      break;
    case spv::Op::OpTypeBool:
      value = 1;
      break;
    case spv::Op::OpTypeInt: {
      auto const signedness = inst->GetSingleWordOperand(2);
      value = 3 + signedness;
      break;
    }
    case spv::Op::OpTypeFloat:
      value = 5;
      break;
    case spv::Op::OpTypeVector: {
      auto const component_type = inst->GetSingleWordOperand(1);
      auto const component_count = inst->GetSingleWordOperand(2);
      value = 6 + HashType(component_type) * (component_count - 1);
      break;
    }
    case spv::Op::OpTypeMatrix: {
      auto const column_type = inst->GetSingleWordOperand(1);
      auto const column_count = inst->GetSingleWordOperand(2);
      value = 30 + HashType(column_type) * (column_count - 1);
      break;
    }
    case spv::Op::OpTypeImage: {
      // TODO: Why isn't the format used to compute the hash value?
      auto const sampled_type = inst->GetSingleWordOperand(1);
      auto const dim = inst->GetSingleWordOperand(2);
      auto const depth = inst->GetSingleWordOperand(3);
      auto const arrayed = inst->GetSingleWordOperand(4);
      auto const ms = inst->GetSingleWordOperand(5);
      auto const sampled = inst->GetSingleWordOperand(6);
      value = 120 +
        HashType(sampled_type) +
        dim +
        depth * 8 * 16 +
        arrayed * 4 * 16 +
        ms * 2 * 16 +
        sampled * 1 * 16;
      break;
    }
    case spv::Op::OpTypeSampler:
      value = 500;
      break;
    case spv::Op::OpTypeSampledImage:
      value = 502;
      break;
    case spv::Op::OpTypeArray: {
      auto const element_type = inst->GetSingleWordOperand(1);
      auto const length = inst->GetSingleWordOperand(2);
      value = 501 + HashType(element_type) * length;
      break;
    }
    case spv::Op::OpTypeRuntimeArray: {
      auto const element_type = inst->GetSingleWordOperand(1);
      value = 5000 + HashType(element_type);
      break;
    }
    case spv::Op::OpTypeStruct:
      value = 10000;
      for(uint32_t w=1; w < inst->NumOperandWords(); ++w) {
        value += w * HashType(inst->GetSingleWordOperand(w));
      }
      break;
    case spv::Op::OpTypeOpaque: {
      // TODO: name is a literal that may have more than one word.
      auto const name = inst->GetSingleWordOperand(1);
      value = 6000 + name;
      break;
    }
    case spv::Op::OpTypePointer: {
      auto const type = inst->GetSingleWordOperand(2);
      value = 100000 + HashType(type);
      break;
    }
    case spv::Op::OpTypeFunction:
      value = 200000;
      for(uint32_t w=1; w < inst->NumOperandWords(); ++w) {
        value += w * HashType(inst->GetSingleWordOperand(w));
      }
      break;
    case spv::Op::OpTypeEvent:
      value = 300000;
      break;
    case spv::Op::OpTypeDeviceEvent:
      value = 300001;
      break;
    case spv::Op::OpTypeReserveId:
      value = 300002;
      break;
    case spv::Op::OpTypeQueue:
      value = 300003;
      break;
    case spv::Op::OpTypePipe:
      value = 300004;
      break;
    case spv::Op::OpConstantTrue:
      value = 300007;
      break;
    case spv::Op::OpConstantFalse:
      value = 300008;
      break;
    case spv::Op::OpTypeRayQueryKHR:
      value = 300009;
      break;
    case spv::Op::OpTypeAccelerationStructureKHR:
      value = 300010;
      break;
    case spv::Op::OpConstantComposite: {
      auto const result_type = inst->GetSingleWordOperand(0);
      value = 300011 + HashType(result_type);
      for (uint32_t w=2; w < inst->NumOperandWords(); ++w) {
        value += w * HashType(inst->GetSingleWordOperand(w));
      }
      break;
    }
    case spv::Op::OpConstant: {
      auto const result_type = inst->GetSingleWordOperand(0);
      value = 400011 + HashType(result_type);
      for (uint32_t w=2; w < inst->NumOperandWords(); ++w) {
        value += w * inst->GetSingleWordOperand(w);
      }
      break;
    }
    case spv::Op::OpConstantNull: {
      auto const result_type = inst->GetSingleWordOperand(0);
      value = 500009 + HashType(result_type);
      break;
    }
    case spv::Op::OpConstantSampler: {
      auto const result_type = inst->GetSingleWordOperand(0);
      value = 600011 + HashType(result_type);
      for (uint32_t w=2; w < inst->NumOperandWords(); ++w) {
        value += w * inst->GetSingleWordOperand(w);
      }
      break;
    }
    default:
      context()->consumer()(SPV_MSG_ERROR, "", {0, 0, 0}, "unknown type opcode");
      break;
  }

  return value;
}

std::string RemapIdsPass::IdAsString(spv::Id const id) const {
  if (id == unused_) {
    return "unused";
  } else if (id == unmapped_) {
    return "unmapped";
  } else {
    return std::to_string(id);
  }
}

void RemapIdsPass::PrintId(spv::Id const id) const {
  context()->consumer()(SPV_MSG_INFO, "", {0, 0, 0}, IdAsString(id).c_str());
}

void RemapIdsPass::PrintNewIds() const {
  for (spv::Id id = 0; id < new_id_.size(); ++id) {
    auto const message = "new id[" + IdAsString(id) + "]: " +
      IdAsString(new_id_[id]);
    context()->consumer()(SPV_MSG_INFO, "", {0, 0, 0}, message.c_str());
  }
}

} // namespace opt
} // namespace spvtools