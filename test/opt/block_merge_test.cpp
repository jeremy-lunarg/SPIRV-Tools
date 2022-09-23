// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG Inc.
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

#include <string>

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using BlockMergeTest = PassTest<::testing::Test>;

TEST_F(BlockMergeTest, Simple) {
  // Note: SPIR-V hand edited to insert block boundary
  // between two statements in main.
  //
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  void main()
  //  {
  //      vec4 v = BaseColor;
  //      gl_FragColor = v;
  //  }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
OpBranch %15
%15 = OpLabel
%16 = OpLoad %v4float %v
OpStore %gl_FragColor %16
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
%16 = OpLoad %v4float %v
OpStore %gl_FragColor %16
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<BlockMergePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(BlockMergeTest, BlockMergeForLinkage) {
  const std::string before =
      R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpSource HLSL 630
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %bb_entry "bb.entry"
OpName %v "v"
OpDecorate %main LinkageAttributes "main" Export
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%8 = OpTypeFunction %v4float %_ptr_Function_v4float
%main = OpFunction %v4float None %8
%BaseColor = OpFunctionParameter %_ptr_Function_v4float
%bb_entry = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%9 = OpLoad %v4float %BaseColor
OpStore %v %9
OpBranch %10
%10 = OpLabel
%11 = OpLoad %v4float %v
OpBranch %12
%12 = OpLabel
OpReturnValue %11
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpSource HLSL 630
OpName %main "main"
OpName %BaseColor "BaseColor"
OpName %bb_entry "bb.entry"
OpName %v "v"
OpDecorate %main LinkageAttributes "main" Export
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%8 = OpTypeFunction %v4float %_ptr_Function_v4float
%main = OpFunction %v4float None %8
%BaseColor = OpFunctionParameter %_ptr_Function_v4float
%bb_entry = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%9 = OpLoad %v4float %BaseColor
OpStore %v %9
%11 = OpLoad %v4float %v
OpReturnValue %11
OpFunctionEnd
)";
  SinglePassRunAndCheck<BlockMergePass>(before, after, true, true);
}

TEST_F(BlockMergeTest, EmptyBlock) {
  // Note: SPIR-V hand edited to insert empty block
  // after two statements in main.
  //
  //  #version 140
  //
  //  in vec4 BaseColor;
  //
  //  void main()
  //  {
  //      vec4 v = BaseColor;
  //      gl_FragColor = v;
  //  }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %gl_FragColor "gl_FragColor"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
OpBranch %15
%15 = OpLabel
%16 = OpLoad %v4float %v
OpStore %gl_FragColor %16
OpBranch %17
%17 = OpLabel
OpBranch %18
%18 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %7
%13 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%14 = OpLoad %v4float %BaseColor
OpStore %v %14
%16 = OpLoad %v4float %v
OpStore %gl_FragColor %16
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<BlockMergePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(BlockMergeTest, NestedInControlFlow) {
  // Note: SPIR-V hand edited to insert block boundary
  // between OpFMul and OpStore in then-part.
  //
  // #version 140
  // in vec4 BaseColor;
  //
  // layout(std140) uniform U_t
  // {
  //     bool g_B ;
  // } ;
  //
  // void main()
  // {
  //     vec4 v = BaseColor;
  //     if (g_B)
  //       vec4 v = v * 0.25;
  //     gl_FragColor = v;
  // }

  const std::string predefs =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %BaseColor %gl_FragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %v "v"
OpName %BaseColor "BaseColor"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_B"
OpName %_ ""
OpName %v_0 "v"
OpName %gl_FragColor "gl_FragColor"
OpMemberDecorate %U_t 0 Offset 0
OpDecorate %U_t Block
OpDecorate %_ DescriptorSet 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%uint = OpTypeInt 32 0
%U_t = OpTypeStruct %uint
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%bool = OpTypeBool
%uint_0 = OpConstant %uint 0
%float_0_25 = OpConstant %float 0.25
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
)";

  const std::string before =
      R"(%main = OpFunction %void None %10
%24 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%v_0 = OpVariable %_ptr_Function_v4float Function
%25 = OpLoad %v4float %BaseColor
OpStore %v %25
%26 = OpAccessChain %_ptr_Uniform_uint %_ %int_0
%27 = OpLoad %uint %26
%28 = OpINotEqual %bool %27 %uint_0
OpSelectionMerge %29 None
OpBranchConditional %28 %30 %29
%30 = OpLabel
%31 = OpLoad %v4float %v
%32 = OpVectorTimesScalar %v4float %31 %float_0_25
OpBranch %33
%33 = OpLabel
OpStore %v_0 %32
OpBranch %29
%29 = OpLabel
%34 = OpLoad %v4float %v
OpStore %gl_FragColor %34
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(%main = OpFunction %void None %10
%24 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%v_0 = OpVariable %_ptr_Function_v4float Function
%25 = OpLoad %v4float %BaseColor
OpStore %v %25
%26 = OpAccessChain %_ptr_Uniform_uint %_ %int_0
%27 = OpLoad %uint %26
%28 = OpINotEqual %bool %27 %uint_0
OpSelectionMerge %29 None
OpBranchConditional %28 %30 %29
%30 = OpLabel
%31 = OpLoad %v4float %v
%32 = OpVectorTimesScalar %v4float %31 %float_0_25
OpStore %v_0 %32
OpBranch %29
%29 = OpLabel
%34 = OpLoad %v4float %v
OpStore %gl_FragColor %34
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<BlockMergePass>(predefs + before, predefs + after, true,
                                        true);
}

TEST_F(BlockMergeTest, PhiInSuccessorOfMergedBlock) {
  const std::string text = R"(
; CHECK: OpSelectionMerge [[merge:%\w+]] None
; CHECK-NEXT: OpBranchConditional {{%\w+}} [[then:%\w+]] [[else:%\w+]]
; CHECK: [[then]] = OpLabel
; CHECK-NEXT: OpBranch [[merge]]
; CHECK: [[else]] = OpLabel
; CHECK-NEXT: OpBranch [[merge]]
; CHECK: [[merge]] = OpLabel
; CHECK-NEXT: OpPhi {{%\w+}} %true [[then]] %false [[else]]
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpSelectionMerge %merge None
OpBranchConditional %true %then %else
%then = OpLabel
OpBranch %then_next
%then_next = OpLabel
OpBranch %merge
%else = OpLabel
OpBranch %merge
%merge = OpLabel
%phi = OpPhi %bool %true %then_next %false %else
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, UpdateMergeInstruction) {
  const std::string text = R"(
; CHECK: OpSelectionMerge [[merge:%\w+]] None
; CHECK-NEXT: OpBranchConditional {{%\w+}} [[then:%\w+]] [[else:%\w+]]
; CHECK: [[then]] = OpLabel
; CHECK-NEXT: OpBranch [[merge]]
; CHECK: [[else]] = OpLabel
; CHECK-NEXT: OpBranch [[merge]]
; CHECK: [[merge]] = OpLabel
; CHECK-NEXT: OpReturn
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpSelectionMerge %real_merge None
OpBranchConditional %true %then %else
%then = OpLabel
OpBranch %merge
%else = OpLabel
OpBranch %merge
%merge = OpLabel
OpBranch %real_merge
%real_merge = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, TwoMergeBlocksCannotBeMerged) {
  const std::string text = R"(
; CHECK: OpSelectionMerge [[outer_merge:%\w+]] None
; CHECK: OpSelectionMerge [[inner_merge:%\w+]] None
; CHECK: [[inner_merge]] = OpLabel
; CHECK-NEXT: OpBranch [[outer_merge]]
; CHECK: [[outer_merge]] = OpLabel
; CHECK-NEXT: OpReturn
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpSelectionMerge %outer_merge None
OpBranchConditional %true %then %else
%then = OpLabel
OpBranch %inner_header
%else = OpLabel
OpBranch %inner_header
%inner_header = OpLabel
OpSelectionMerge %inner_merge None
OpBranchConditional %true %inner_then %inner_else
%inner_then = OpLabel
OpBranch %inner_merge
%inner_else = OpLabel
OpBranch %inner_merge
%inner_merge = OpLabel
OpBranch %outer_merge
%outer_merge = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, MergeContinue) {
  const std::string text = R"(
; CHECK: OpBranch [[header:%\w+]]
; CHECK: [[header]] = OpLabel
; CHECK-NEXT: OpLogicalAnd
; CHECK-NEXT: OpLoopMerge {{%\w+}} [[header]] None
; CHECK-NEXT: OpBranch [[header]]
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpBranch %header
%header = OpLabel
OpLoopMerge %merge %continue None
OpBranch %continue
%continue = OpLabel
%op = OpLogicalAnd %bool %true %false
OpBranch %header
%merge = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, MergeContinueWithOpLine) {
  const std::string text = R"(
; CHECK: OpBranch [[header:%\w+]]
; CHECK: [[header]] = OpLabel
; CHECK-NEXT: OpLogicalAnd
; CHECK-NEXT: OpLine {{%\w+}} 1 1
; CHECK-NEXT: OpLoopMerge {{%\w+}} [[header]] None
; CHECK-NEXT: OpBranch [[header]]
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%src = OpString "test.shader"
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpBranch %header
%header = OpLabel
OpLoopMerge %merge %continue None
OpBranch %continue
%continue = OpLabel
%op = OpLogicalAnd %bool %true %false
OpLine %src 1 1
OpBranch %header
%merge = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, TwoHeadersCannotBeMerged) {
  const std::string text = R"(
; CHECK: OpBranch [[loop_header:%\w+]]
; CHECK: [[loop_header]] = OpLabel
; CHECK-NEXT: OpLoopMerge
; CHECK-NEXT: OpBranch [[if_header:%\w+]]
; CHECK: [[if_header]] = OpLabel
; CHECK-NEXT: OpSelectionMerge
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpBranch %header
%header = OpLabel
OpLoopMerge %merge %continue None
OpBranch %inner_header
%inner_header = OpLabel
OpSelectionMerge %if_merge None
OpBranchConditional %true %then %if_merge
%then = OpLabel
OpBranch %continue
%if_merge = OpLabel
OpBranch %continue
%continue = OpLabel
OpBranchConditional %false %merge %header
%merge = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, CannotMergeContinue) {
  const std::string text = R"(
; CHECK: OpBranch [[loop_header:%\w+]]
; CHECK: [[loop_header]] = OpLabel
; CHECK-NEXT: OpLoopMerge {{%\w+}} [[continue:%\w+]]
; CHECK-NEXT: OpBranchConditional {{%\w+}} [[if_header:%\w+]]
; CHECK: [[if_header]] = OpLabel
; CHECK-NEXT: OpSelectionMerge
; CHECK: [[continue]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%true = OpConstantTrue %bool
%false = OpConstantFalse  %bool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%entry = OpLabel
OpBranch %header
%header = OpLabel
OpLoopMerge %merge %continue None
OpBranchConditional %true %inner_header %merge
%inner_header = OpLabel
OpSelectionMerge %if_merge None
OpBranchConditional %true %then %if_merge
%then = OpLabel
OpBranch %continue
%if_merge = OpLabel
OpBranch %continue
%continue = OpLabel
OpBranchConditional %false %merge %header
%merge = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, RemoveStructuredDeclaration) {
  // Note: SPIR-V hand edited remove dead branch and add block
  // before continue block
  //
  // #version 140
  // in vec4 BaseColor;
  //
  // void main()
  // {
  //     while (true) {
  //         break;
  //     }
  //     gl_FragColor = BaseColor;
  // }

  const std::string assembly =
      R"(
; CHECK: OpLabel
; CHECK: [[header:%\w+]] = OpLabel
; CHECK-NOT: OpLoopMerge
; CHECK: OpReturn
; CHECK: [[continue:%\w+]] = OpLabel
; CHECK-NEXT: OpBranch [[block:%\w+]]
; CHECK: [[block]] = OpLabel
; CHECK-NEXT: OpBranch [[header]]
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %gl_FragColor %BaseColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 140
OpName %main "main"
OpName %gl_FragColor "gl_FragColor"
OpName %BaseColor "BaseColor"
%void = OpTypeVoid
%6 = OpTypeFunction %void
%bool = OpTypeBool
%true = OpConstantTrue %bool
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%gl_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%BaseColor = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %6
%13 = OpLabel
OpBranch %14
%14 = OpLabel
OpLoopMerge %15 %16 None
OpBranch %17
%17 = OpLabel
OpBranch %15
%18 = OpLabel
OpBranch %16
%16 = OpLabel
OpBranch %14
%15 = OpLabel
%19 = OpLoad %v4float %BaseColor
OpStore %gl_FragColor %19
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(assembly, true);
}

TEST_F(BlockMergeTest, DontMergeKill) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpKill
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%1 = OpLabel
OpBranch %2
%2 = OpLabel
OpLoopMerge %3 %4 None
OpBranch %5
%5 = OpLabel
OpKill
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, DontMergeTerminateInvocation) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpTerminateInvocation
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpExtension "SPV_KHR_terminate_invocation"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%1 = OpLabel
OpBranch %2
%2 = OpLabel
OpLoopMerge %3 %4 None
OpBranch %5
%5 = OpLabel
OpTerminateInvocation
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, DontMergeUnreachable) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpUnreachable
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%1 = OpLabel
OpBranch %2
%2 = OpLabel
OpLoopMerge %3 %4 None
OpBranch %5
%5 = OpLabel
OpUnreachable
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, false);
}

TEST_F(BlockMergeTest, DontMergeReturn) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpReturn
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%1 = OpLabel
OpBranch %2
%2 = OpLabel
OpLoopMerge %3 %4 None
OpBranch %5
%5 = OpLabel
OpReturn
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, DontMergeSwitch) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpSelectionMerge
; CHECK-NEXT: OpSwitch
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%functy = OpTypeFunction %void
%func = OpFunction %void None %functy
%1 = OpLabel
OpBranch %2
%2 = OpLabel
OpLoopMerge %3 %4 None
OpBranch %5
%5 = OpLabel
OpSelectionMerge %6 None
OpSwitch %int_0 %6
%6 = OpLabel
OpReturn
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, DontMergeReturnValue) {
  const std::string text = R"(
; CHECK: OpLoopMerge [[merge:%\w+]] [[cont:%\w+]] None
; CHECK-NEXT: OpBranch [[ret:%\w+]]
; CHECK: [[ret:%\w+]] = OpLabel
; CHECK-NEXT: OpReturn
; CHECK-DAG: [[cont]] = OpLabel
; CHECK-DAG: [[merge]] = OpLabel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%functy = OpTypeFunction %void
%otherfuncty = OpTypeFunction %bool
%true = OpConstantTrue %bool
%func = OpFunction %void None %functy
%1 = OpLabel
%2 = OpFunctionCall %bool %3
OpReturn
OpFunctionEnd
%3 = OpFunction %bool None %otherfuncty
%4 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %6 %7 None
OpBranch %8
%8 = OpLabel
OpReturnValue %true
%7 = OpLabel
OpBranch %5
%6 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, MergeHeaders) {
  // Merge two headers when the second is the merge block of the first.
  const std::string text = R"(
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: OpBranch [[header:%\w+]]
; CHECK-NEXT: [[header]] = OpLabel
; CHECK-NEXT: OpSelectionMerge [[merge:%\w+]]
; CHECK: [[merge]] = OpLabel
; CHECK: OpReturn
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %func "func"
OpExecutionMode %func OriginUpperLeft
%void = OpTypeVoid
%bool = OpTypeBool
%functy = OpTypeFunction %void
%otherfuncty = OpTypeFunction %bool
%true = OpConstantTrue %bool
%func = OpFunction %void None %functy
%1 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %8 %7 None
OpBranch %8
%7 = OpLabel
OpBranch %5
%8 = OpLabel
OpSelectionMerge %m None
OpBranchConditional %true %a %m
%a = OpLabel
OpBranch %m
%m = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, OpPhiInSuccessor) {
  // Checks that when merging blocks A and B, the OpPhi at the start of B is
  // removed and uses of its definition are replaced appropriately.
  const std::string prefix =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource ESSL 310
OpName %main "main"
OpName %x "x"
OpName %y "y"
%void = OpTypeVoid
%6 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%main = OpFunction %void None %6
%10 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%y = OpVariable %_ptr_Function_int Function
OpStore %x %int_1
%11 = OpLoad %int %x
)";

  const std::string suffix_before =
      R"(OpBranch %12
%12 = OpLabel
%13 = OpPhi %int %11 %10
OpStore %y %13
OpReturn
OpFunctionEnd
)";

  const std::string suffix_after =
      R"(OpStore %y %11
OpReturn
OpFunctionEnd
)";
  SinglePassRunAndCheck<BlockMergePass>(prefix + suffix_before,
                                        prefix + suffix_after, true, true);
}

TEST_F(BlockMergeTest, MultipleOpPhisInSuccessor) {
  // Checks that when merging blocks A and B, the OpPhis at the start of B are
  // removed and uses of their definitions are replaced appropriately.
  const std::string prefix =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource ESSL 310
OpName %main "main"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "f"
OpName %s "s"
OpName %g "g"
OpName %y "y"
OpName %t "t"
OpName %z "z"
%void = OpTypeVoid
%10 = OpTypeFunction %void
%int = OpTypeInt 32 1
%float = OpTypeFloat 32
%S = OpTypeStruct %int %float
%_ptr_Function_S = OpTypePointer Function %S
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%16 = OpConstantComposite %S %int_1 %float_2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_int = OpTypePointer Function %int
%int_3 = OpConstant %int 3
%int_0 = OpConstant %int 0
%main = OpFunction %void None %10
%21 = OpLabel
%s = OpVariable %_ptr_Function_S Function
%g = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_int Function
%t = OpVariable %_ptr_Function_S Function
%z = OpVariable %_ptr_Function_float Function
OpStore %s %16
OpStore %g %float_2
OpStore %y %int_3
%22 = OpLoad %S %s
OpStore %t %22
%23 = OpAccessChain %_ptr_Function_float %s %int_1
%24 = OpLoad %float %23
%25 = OpLoad %float %g
)";

  const std::string suffix_before =
      R"(OpBranch %26
%26 = OpLabel
%27 = OpPhi %float %24 %21
%28 = OpPhi %float %25 %21
%29 = OpFAdd %float %27 %28
%30 = OpAccessChain %_ptr_Function_int %s %int_0
%31 = OpLoad %int %30
OpBranch %32
%32 = OpLabel
%33 = OpPhi %float %29 %26
%34 = OpPhi %int %31 %26
%35 = OpConvertSToF %float %34
OpBranch %36
%36 = OpLabel
%37 = OpPhi %float %35 %32
%38 = OpFSub %float %33 %37
%39 = OpLoad %int %y
OpBranch %40
%40 = OpLabel
%41 = OpPhi %float %38 %36
%42 = OpPhi %int %39 %36
%43 = OpConvertSToF %float %42
%44 = OpFAdd %float %41 %43
OpStore %z %44
OpReturn
OpFunctionEnd
)";

  const std::string suffix_after =
      R"(%29 = OpFAdd %float %24 %25
%30 = OpAccessChain %_ptr_Function_int %s %int_0
%31 = OpLoad %int %30
%35 = OpConvertSToF %float %31
%38 = OpFSub %float %29 %35
%39 = OpLoad %int %y
%43 = OpConvertSToF %float %39
%44 = OpFAdd %float %38 %43
OpStore %z %44
OpReturn
OpFunctionEnd
)";
  SinglePassRunAndCheck<BlockMergePass>(prefix + suffix_before,
                                        prefix + suffix_after, true, true);
}

TEST_F(BlockMergeTest, UnreachableLoop) {
  const std::string spirv = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource ESSL 310
OpName %main "main"
%void = OpTypeVoid
%4 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%bool = OpTypeBool
%false = OpConstantFalse %bool
%main = OpFunction %void None %4
%9 = OpLabel
OpBranch %10
%11 = OpLabel
OpLoopMerge %12 %13 None
OpBranchConditional %false %13 %14
%13 = OpLabel
OpSelectionMerge %15 None
OpBranchConditional %false %16 %17
%16 = OpLabel
OpBranch %15
%17 = OpLabel
OpBranch %15
%15 = OpLabel
OpBranch %11
%14 = OpLabel
OpReturn
%12 = OpLabel
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<BlockMergePass>(spirv, spirv, true, true);
}

TEST_F(BlockMergeTest, DebugMerge) {
  // Verify merge can be done completely, cleanly and validly in presence of
  // NonSemantic.Shader.DebugInfo.100 instructions
  const std::string text = R"(
; CHECK: OpLoopMerge
; CHECK-NEXT: OpBranch
; CHECK-NOT: OpBranch
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %in_var_COLOR %out_var_SV_TARGET
OpExecutionMode %main OriginUpperLeft
%5 = OpString "lexblock.hlsl"
%20 = OpString "float"
%32 = OpString "main"
%33 = OpString ""
%46 = OpString "b"
%49 = OpString "a"
%58 = OpString "c"
%63 = OpString "color"
OpName %in_var_COLOR "in.var.COLOR"
OpName %out_var_SV_TARGET "out.var.SV_TARGET"
OpName %main "main"
OpDecorate %in_var_COLOR Location 0
OpDecorate %out_var_SV_TARGET Location 0
%float = OpTypeFloat 32
%float_0 = OpConstant %float 0
%v4float = OpTypeVector %float 4
%9 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_1 = OpConstant %float 1
%13 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%void = OpTypeVoid
%uint_3 = OpConstant %uint 3
%uint_0 = OpConstant %uint 0
%uint_4 = OpConstant %uint 4
%uint_1 = OpConstant %uint 1
%uint_5 = OpConstant %uint 5
%uint_12 = OpConstant %uint 12
%uint_13 = OpConstant %uint 13
%uint_20 = OpConstant %uint 20
%uint_15 = OpConstant %uint 15
%uint_17 = OpConstant %uint 17
%uint_16 = OpConstant %uint 16
%uint_14 = OpConstant %uint 14
%uint_10 = OpConstant %uint 10
%65 = OpTypeFunction %void
%in_var_COLOR = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
%62 = OpExtInst %void %1 DebugExpression
%22 = OpExtInst %void %1 DebugTypeBasic %20 %uint_32 %uint_3 %uint_0
%25 = OpExtInst %void %1 DebugTypeVector %22 %uint_4
%27 = OpExtInst %void %1 DebugTypeFunction %uint_3 %25 %25
%28 = OpExtInst %void %1 DebugSource %5
%29 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %28 %uint_5
%34 = OpExtInst %void %1 DebugFunction %32 %27 %28 %uint_12 %uint_1 %29 %33 %uint_3 %uint_13
%37 = OpExtInst %void %1 DebugLexicalBlock %28 %uint_13 %uint_1 %34
%52 = OpExtInst %void %1 DebugLexicalBlock %28 %uint_15 %uint_12 %37
%54 = OpExtInst %void %1 DebugLocalVariable %46 %25 %28 %uint_17 %uint_12 %52 %uint_4
%56 = OpExtInst %void %1 DebugLocalVariable %49 %25 %28 %uint_16 %uint_12 %52 %uint_4
%59 = OpExtInst %void %1 DebugLocalVariable %58 %25 %28 %uint_14 %uint_10 %37 %uint_4
%64 = OpExtInst %void %1 DebugLocalVariable %63 %25 %28 %uint_12 %uint_20 %34 %uint_4 %uint_1
%main = OpFunction %void None %65
%66 = OpLabel
%69 = OpLoad %v4float %in_var_COLOR
%168 = OpExtInst %void %1 DebugValue %64 %69 %62
%169 = OpExtInst %void %1 DebugScope %37
OpLine %5 14 10
%164 = OpExtInst %void %1 DebugValue %59 %9 %62
OpLine %5 15 3
OpBranch %150
%150 = OpLabel
%165 = OpPhi %v4float %9 %66 %158 %159
%167 = OpExtInst %void %1 DebugValue %59 %165 %62
%170 = OpExtInst %void %1 DebugScope %37
OpLine %5 15 12
%171 = OpExtInst %void %1 DebugNoScope
OpLoopMerge %160 %159 None
OpBranch %151
%151 = OpLabel
OpLine %5 16 12
%162 = OpExtInst %void %1 DebugValue %56 %9 %62
OpLine %5 17 12
%163 = OpExtInst %void %1 DebugValue %54 %13 %62
OpLine %5 18 15
%158 = OpFAdd %v4float %165 %13
OpLine %5 18 5
%166 = OpExtInst %void %1 DebugValue %59 %158 %62
%172 = OpExtInst %void %1 DebugScope %37
OpLine %5 19 3
OpBranch %159
%159 = OpLabel
OpLine %5 19 3
OpBranch %150
%160 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

TEST_F(BlockMergeTest, DebugScopeMerge) {
  const std::string text = R"(
; CHECK: OpLoopMerge
               OpCapability Shader
               OpCapability ImageQuery
               OpExtension "SPV_KHR_non_semantic_info"
          %2 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %inUV %outFragColor
               OpExecutionMode %main OriginUpperLeft
          %1 = OpString "../data/shaders/glsl/deferredshadows/deferred.frag"
          %9 = OpString "uint"
         %15 = OpString "main"
         %24 = OpString "float"
         %39 = OpString "textureProj"
         %45 = OpString "P"
         %49 = OpString "layer"
         %52 = OpString "offset"
         %59 = OpString "filterPCF"
         %65 = OpString "sc"
         %77 = OpString "shadow"
         %83 = OpString "fragcolor"
         %86 = OpString "fragpos"
        %100 = OpString "shadowCoord"
        %125 = OpString "bool"
        %145 = OpString "dist"
        %149 = OpString "type.2d.image"
        %150 = OpString "@type.2d.image"
        %154 = OpString "type.sampled.image"
        %155 = OpString "@type.sampled.image"
        %159 = OpString "samplerShadowMap"
        %207 = OpString "int"
        %214 = OpString "texDim"
        %227 = OpString "scale"
        %234 = OpString "dx"
        %247 = OpString "dy"
        %259 = OpString "shadowFactor"
        %265 = OpString "count"
        %271 = OpString "range"
        %278 = OpString "x"
        %300 = OpString "y"
        %364 = OpString "i"
        %384 = OpString "shadowClip"
        %391 = OpString "color"
        %397 = OpString "viewMatrix"
        %400 = OpString "Light"
        %406 = OpString "lights"
        %409 = OpString "debugDisplayTarget"
        %413 = OpString "UBO"
        %417 = OpString "ubo"
        %460 = OpString "fragPos"
        %469 = OpString "samplerposition"
        %474 = OpString "inUV"
        %482 = OpString "normal"
        %486 = OpString "samplerNormal"
        %495 = OpString "albedo"
        %499 = OpString "samplerAlbedo"
        %530 = OpString "outFragColor"
        %622 = OpString "N"
        %648 = OpString "L"
        %672 = OpString "V"
        %687 = OpString "lightCosInnerAngle"
        %694 = OpString "lightCosOuterAngle"
        %701 = OpString "lightRange"
        %708 = OpString "dir"
        %724 = OpString "cosDir"
        %733 = OpString "spotEffect"
        %743 = OpString "heightAttenuation"
        %752 = OpString "NdotL"
        %762 = OpString "diff"
        %770 = OpString "R"
        %780 = OpString "NdotR"
        %790 = OpString "spec"
               OpName %main "main"
               OpName %samplerShadowMap "samplerShadowMap"
               OpName %Light "Light"
               OpMemberName %Light 0 "position"
               OpMemberName %Light 1 "target"
               OpMemberName %Light 2 "color"
               OpMemberName %Light 3 "viewMatrix"
               OpName %UBO "UBO"
               OpMemberName %UBO 0 "viewPos"
               OpMemberName %UBO 1 "lights"
               OpMemberName %UBO 2 "useShadows"
               OpMemberName %UBO 3 "debugDisplayTarget"
               OpName %ubo "ubo"
               OpName %fragPos "fragPos"
               OpName %samplerposition "samplerposition"
               OpName %inUV "inUV"
               OpName %normal "normal"
               OpName %samplerNormal "samplerNormal"
               OpName %albedo "albedo"
               OpName %samplerAlbedo "samplerAlbedo"
               OpName %outFragColor "outFragColor"
               OpName %param "param"
               OpName %param_0 "param"
               OpName %fragcolor "fragcolor"
               OpName %N "N"
               OpName %i "i"
               OpName %L "L"
               OpName %dist "dist"
               OpName %V "V"
               OpName %lightCosInnerAngle "lightCosInnerAngle"
               OpName %lightCosOuterAngle "lightCosOuterAngle"
               OpName %lightRange "lightRange"
               OpName %dir "dir"
               OpName %cosDir "cosDir"
               OpName %spotEffect "spotEffect"
               OpName %heightAttenuation "heightAttenuation"
               OpName %NdotL "NdotL"
               OpName %diff "diff"
               OpName %R "R"
               OpName %NdotR "NdotR"
               OpName %spec "spec"
               OpName %param_1 "param"
               OpName %param_2 "param"
               OpModuleProcessed "entry-point main"
               OpModuleProcessed "client vulkan100"
               OpModuleProcessed "target-env spirv1.3"
               OpModuleProcessed "target-env vulkan1.2"
               OpModuleProcessed "entry-point main"
               OpDecorate %samplerShadowMap DescriptorSet 0
               OpDecorate %samplerShadowMap Binding 5
               OpMemberDecorate %Light 0 Offset 0
               OpMemberDecorate %Light 1 Offset 16
               OpMemberDecorate %Light 2 Offset 32
               OpMemberDecorate %Light 3 ColMajor
               OpMemberDecorate %Light 3 Offset 48
               OpMemberDecorate %Light 3 MatrixStride 16
               OpDecorate %_arr_Light_uint_3 ArrayStride 112
               OpMemberDecorate %UBO 0 Offset 0
               OpMemberDecorate %UBO 1 Offset 16
               OpMemberDecorate %UBO 2 Offset 352
               OpMemberDecorate %UBO 3 Offset 356
               OpDecorate %UBO Block
               OpDecorate %ubo DescriptorSet 0
               OpDecorate %ubo Binding 4
               OpDecorate %samplerposition DescriptorSet 0
               OpDecorate %samplerposition Binding 1
               OpDecorate %inUV Location 0
               OpDecorate %samplerNormal DescriptorSet 0
               OpDecorate %samplerNormal Binding 2
               OpDecorate %samplerAlbedo DescriptorSet 0
               OpDecorate %samplerAlbedo Binding 3
               OpDecorate %outFragColor Location 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
    %uint_35 = OpConstant %uint 35
    %float_1 = OpConstant %float 1
    %uint_36 = OpConstant %uint 36
    %uint_37 = OpConstant %uint 37
  %float_0_5 = OpConstant %float 0.5
    %uint_39 = OpConstant %uint 39
       %bool = OpTypeBool
   %float_n1 = OpConstant %float -1
    %uint_41 = OpConstant %uint 41
        %147 = OpTypeImage %float 2D 0 1 0 1 Unknown
        %152 = OpTypeSampledImage %147
%_ptr_UniformConstant_152 = OpTypePointer UniformConstant %152
%samplerShadowMap = OpVariable %_ptr_UniformConstant_152 UniformConstant
     %uint_8 = OpConstant %uint 8
    %uint_42 = OpConstant %uint 42
    %float_0 = OpConstant %float 0
    %uint_44 = OpConstant %uint 44
 %float_0_25 = OpConstant %float 0.25
    %uint_47 = OpConstant %uint 47
    %uint_52 = OpConstant %uint 52
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
      %int_0 = OpConstant %int 0
      %v3int = OpTypeVector %int 3
    %uint_53 = OpConstant %uint 53
  %float_1_5 = OpConstant %float 1.5
    %uint_54 = OpConstant %uint 54
%_ptr_Function_int = OpTypePointer Function %int
    %uint_55 = OpConstant %uint 55
    %uint_57 = OpConstant %uint 57
    %uint_58 = OpConstant %uint 58
    %uint_59 = OpConstant %uint 59
      %int_1 = OpConstant %int 1
    %uint_61 = OpConstant %uint 61
    %uint_63 = OpConstant %uint 63
    %uint_65 = OpConstant %uint 65
    %uint_66 = OpConstant %uint 66
    %uint_70 = OpConstant %uint 70
    %uint_74 = OpConstant %uint 74
      %int_3 = OpConstant %int 3
    %uint_76 = OpConstant %uint 76
%mat4v4float = OpTypeMatrix %v4float 4
       %true = OpConstantTrue %bool
      %Light = OpTypeStruct %v4float %v4float %v4float %mat4v4float
    %uint_21 = OpConstant %uint 21
     %uint_7 = OpConstant %uint 7
    %uint_22 = OpConstant %uint 22
%_arr_Light_uint_3 = OpTypeArray %Light %uint_3
        %UBO = OpTypeStruct %v4float %_arr_Light_uint_3 %int %int
    %uint_28 = OpConstant %uint 28
    %uint_30 = OpConstant %uint 30
%_ptr_Uniform_UBO = OpTypePointer Uniform %UBO
        %ubo = OpVariable %_ptr_Uniform_UBO Uniform
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
    %uint_80 = OpConstant %uint 80
    %uint_85 = OpConstant %uint 85
    %uint_87 = OpConstant %uint 87
    %uint_93 = OpConstant %uint 93
        %462 = OpTypeImage %float 2D 0 0 0 1 Unknown
        %464 = OpTypeSampledImage %462
%_ptr_UniformConstant_464 = OpTypePointer UniformConstant %464
%samplerposition = OpVariable %_ptr_UniformConstant_464 UniformConstant
%_ptr_Input_v2float = OpTypePointer Input %v2float
       %inUV = OpVariable %_ptr_Input_v2float Input
    %uint_94 = OpConstant %uint 94
%samplerNormal = OpVariable %_ptr_UniformConstant_464 UniformConstant
    %uint_95 = OpConstant %uint 95
%samplerAlbedo = OpVariable %_ptr_UniformConstant_464 UniformConstant
    %uint_98 = OpConstant %uint 98
%_ptr_Uniform_int = OpTypePointer Uniform %int
    %uint_99 = OpConstant %uint 99
   %uint_101 = OpConstant %uint 101
%_ptr_Output_v4float = OpTypePointer Output %v4float
%outFragColor = OpVariable %_ptr_Output_v4float Output
        %531 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%_ptr_Output_float = OpTypePointer Output %float
   %uint_102 = OpConstant %uint 102
   %uint_104 = OpConstant %uint 104
   %uint_105 = OpConstant %uint 105
   %uint_107 = OpConstant %uint 107
   %uint_108 = OpConstant %uint 108
   %uint_110 = OpConstant %uint 110
   %uint_111 = OpConstant %uint 111
   %uint_113 = OpConstant %uint 113
   %uint_114 = OpConstant %uint 114
   %uint_116 = OpConstant %uint 116
   %uint_117 = OpConstant %uint 117
   %uint_121 = OpConstant %uint 121
%float_0_100000001 = OpConstant %float 0.100000001
   %uint_123 = OpConstant %uint 123
   %uint_125 = OpConstant %uint 125
   %uint_128 = OpConstant %uint 128
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
   %uint_130 = OpConstant %uint 130
   %uint_131 = OpConstant %uint 131
   %uint_134 = OpConstant %uint 134
   %uint_135 = OpConstant %uint 135
   %uint_137 = OpConstant %uint 137
%float_0_965925813 = OpConstant %float 0.965925813
   %uint_138 = OpConstant %uint 138
%float_0_906307817 = OpConstant %float 0.906307817
   %uint_139 = OpConstant %uint 139
  %float_100 = OpConstant %float 100
   %uint_142 = OpConstant %uint 142
   %uint_145 = OpConstant %uint 145
   %uint_146 = OpConstant %uint 146
   %uint_147 = OpConstant %uint 147
   %uint_150 = OpConstant %uint 150
   %uint_151 = OpConstant %uint 151
   %uint_154 = OpConstant %uint 154
   %uint_155 = OpConstant %uint 155
   %uint_156 = OpConstant %uint 156
   %float_16 = OpConstant %float 16
  %float_2_5 = OpConstant %float 2.5
   %uint_158 = OpConstant %uint 158
      %int_2 = OpConstant %int 2
   %uint_162 = OpConstant %uint 162
   %uint_164 = OpConstant %uint 164
   %uint_167 = OpConstant %uint 167
     %int_n1 = OpConstant %int -1
       %2170 = OpConstantComposite %v2float %float_0_5 %float_0_5
        %151 = OpExtInst %void %2 DebugInfoNone
         %47 = OpExtInst %void %2 DebugExpression
          %6 = OpExtInst %void %2 DebugTypeFunction %uint_3 %void
         %17 = OpExtInst %void %2 DebugSource %1
         %18 = OpExtInst %void %2 DebugCompilationUnit %uint_1 %uint_4 %17 %uint_2
         %16 = OpExtInst %void %2 DebugFunction %15 %6 %17 %uint_0 %uint_0 %18 %15 %uint_3 %uint_0
         %25 = OpExtInst %void %2 DebugTypeBasic %24 %uint_32 %uint_3 %uint_0
         %27 = OpExtInst %void %2 DebugTypeVector %25 %uint_4
         %31 = OpExtInst %void %2 DebugTypeVector %25 %uint_2
         %34 = OpExtInst %void %2 DebugTypeFunction %uint_3 %25 %27 %25 %31
         %40 = OpExtInst %void %2 DebugFunction %39 %34 %17 %uint_0 %uint_0 %18 %39 %uint_3 %uint_0
         %44 = OpExtInst %void %2 DebugLocalVariable %45 %27 %17 %uint_0 %uint_0 %40 %uint_4 %uint_1
         %48 = OpExtInst %void %2 DebugLocalVariable %49 %25 %17 %uint_0 %uint_0 %40 %uint_4 %uint_2
         %51 = OpExtInst %void %2 DebugLocalVariable %52 %31 %17 %uint_0 %uint_0 %40 %uint_4 %uint_3
         %55 = OpExtInst %void %2 DebugTypeFunction %uint_3 %25 %27 %25
         %60 = OpExtInst %void %2 DebugFunction %59 %55 %17 %uint_0 %uint_0 %18 %59 %uint_3 %uint_0
         %64 = OpExtInst %void %2 DebugLocalVariable %65 %27 %17 %uint_0 %uint_0 %60 %uint_4 %uint_1
         %67 = OpExtInst %void %2 DebugLocalVariable %49 %25 %17 %uint_0 %uint_0 %60 %uint_4 %uint_2
         %70 = OpExtInst %void %2 DebugTypeVector %25 %uint_3
         %73 = OpExtInst %void %2 DebugTypeFunction %uint_3 %70 %70 %70
         %78 = OpExtInst %void %2 DebugFunction %77 %73 %17 %uint_0 %uint_0 %18 %77 %uint_3 %uint_0
         %82 = OpExtInst %void %2 DebugLocalVariable %83 %70 %17 %uint_0 %uint_0 %78 %uint_4 %uint_1
         %85 = OpExtInst %void %2 DebugLocalVariable %86 %70 %17 %uint_0 %uint_0 %78 %uint_4 %uint_2
         %93 = OpExtInst %void %2 DebugLocalVariable %77 %25 %17 %uint_35 %uint_0 %40 %uint_4
         %99 = OpExtInst %void %2 DebugLocalVariable %100 %27 %17 %uint_36 %uint_0 %40 %uint_4
        %139 = OpExtInst %void %2 DebugLexicalBlock %17 %uint_0 %uint_0 %40
        %144 = OpExtInst %void %2 DebugLocalVariable %145 %25 %17 %uint_41 %uint_0 %139 %uint_4
        %153 = OpExtInst %void %2 DebugTypeComposite %154 %uint_0 %17 %uint_41 %uint_0 %18 %155 %151 %uint_3
        %158 = OpExtInst %void %2 DebugGlobalVariable %159 %153 %17 %uint_41 %uint_0 %18 %159 %samplerShadowMap %uint_8
        %191 = OpExtInst %void %2 DebugLexicalBlock %17 %uint_0 %uint_0 %139
        %208 = OpExtInst %void %2 DebugTypeBasic %207 %uint_32 %uint_4 %uint_0
        %210 = OpExtInst %void %2 DebugTypeVector %208 %uint_2
        %213 = OpExtInst %void %2 DebugLocalVariable %214 %210 %17 %uint_52 %uint_0 %60 %uint_4
        %226 = OpExtInst %void %2 DebugLocalVariable %227 %25 %17 %uint_53 %uint_0 %60 %uint_4
        %233 = OpExtInst %void %2 DebugLocalVariable %234 %25 %17 %uint_54 %uint_0 %60 %uint_4
        %246 = OpExtInst %void %2 DebugLocalVariable %247 %25 %17 %uint_55 %uint_0 %60 %uint_4
        %258 = OpExtInst %void %2 DebugLocalVariable %259 %25 %17 %uint_57 %uint_0 %60 %uint_4
        %264 = OpExtInst %void %2 DebugLocalVariable %265 %208 %17 %uint_58 %uint_0 %60 %uint_4
        %270 = OpExtInst %void %2 DebugLocalVariable %271 %208 %17 %uint_59 %uint_0 %60 %uint_4
        %277 = OpExtInst %void %2 DebugLocalVariable %278 %208 %17 %uint_61 %uint_0 %60 %uint_4
        %299 = OpExtInst %void %2 DebugLocalVariable %300 %208 %17 %uint_63 %uint_0 %60 %uint_4
        %363 = OpExtInst %void %2 DebugLocalVariable %364 %208 %17 %uint_74 %uint_0 %78 %uint_4
        %383 = OpExtInst %void %2 DebugLocalVariable %384 %27 %17 %uint_76 %uint_0 %78 %uint_4
        %387 = OpExtInst %void %2 DebugTypeMatrix %27 %uint_4 %true
        %390 = OpExtInst %void %2 DebugTypeMember %391 %27 %17 %uint_21 %uint_7 %uint_0 %uint_0 %uint_3
        %394 = OpExtInst %void %2 DebugTypeMember %391 %27 %17 %uint_21 %uint_7 %uint_0 %uint_0 %uint_3
        %395 = OpExtInst %void %2 DebugTypeMember %391 %27 %17 %uint_21 %uint_7 %uint_0 %uint_0 %uint_3
        %396 = OpExtInst %void %2 DebugTypeMember %397 %387 %17 %uint_22 %uint_7 %uint_0 %uint_0 %uint_3
        %399 = OpExtInst %void %2 DebugTypeComposite %400 %uint_1 %17 %uint_76 %uint_0 %18 %400 %uint_0 %uint_3 %390 %394 %395 %396
        %402 = OpExtInst %void %2 DebugTypeArray %399 %uint_3
        %404 = OpExtInst %void %2 DebugTypeMember %391 %27 %17 %uint_21 %uint_7 %uint_0 %uint_0 %uint_3
        %405 = OpExtInst %void %2 DebugTypeMember %406 %402 %17 %uint_28 %uint_8 %uint_0 %uint_0 %uint_3
        %408 = OpExtInst %void %2 DebugTypeMember %409 %208 %17 %uint_30 %uint_6 %uint_0 %uint_0 %uint_3
        %411 = OpExtInst %void %2 DebugTypeMember %409 %208 %17 %uint_30 %uint_6 %uint_0 %uint_0 %uint_3
        %412 = OpExtInst %void %2 DebugTypeComposite %413 %uint_1 %17 %uint_76 %uint_0 %18 %413 %uint_0 %uint_3 %404 %405 %408 %411
        %416 = OpExtInst %void %2 DebugGlobalVariable %417 %412 %17 %uint_76 %uint_0 %18 %417 %ubo %uint_8
        %431 = OpExtInst %void %2 DebugLocalVariable %259 %25 %17 %uint_80 %uint_0 %78 %uint_4
        %459 = OpExtInst %void %2 DebugLocalVariable %460 %70 %17 %uint_93 %uint_0 %16 %uint_4
        %465 = OpExtInst %void %2 DebugTypeComposite %154 %uint_0 %17 %uint_93 %uint_0 %18 %155 %151 %uint_3
        %468 = OpExtInst %void %2 DebugGlobalVariable %469 %465 %17 %uint_93 %uint_0 %18 %469 %samplerposition %uint_8
        %473 = OpExtInst %void %2 DebugGlobalVariable %474 %31 %17 %uint_93 %uint_0 %18 %474 %inUV %uint_8
        %481 = OpExtInst %void %2 DebugLocalVariable %482 %70 %17 %uint_94 %uint_0 %16 %uint_4
        %485 = OpExtInst %void %2 DebugGlobalVariable %486 %465 %17 %uint_94 %uint_0 %18 %486 %samplerNormal %uint_8
        %494 = OpExtInst %void %2 DebugLocalVariable %495 %27 %17 %uint_95 %uint_0 %16 %uint_4
        %498 = OpExtInst %void %2 DebugGlobalVariable %499 %465 %17 %uint_95 %uint_0 %18 %499 %samplerAlbedo %uint_8
        %512 = OpExtInst %void %2 DebugLexicalBlock %17 %uint_0 %uint_0 %16
        %529 = OpExtInst %void %2 DebugGlobalVariable %530 %27 %17 %uint_101 %uint_0 %18 %530 %outFragColor %uint_8
        %612 = OpExtInst %void %2 DebugLocalVariable %83 %70 %17 %uint_121 %uint_0 %16 %uint_4
        %621 = OpExtInst %void %2 DebugLocalVariable %622 %70 %17 %uint_123 %uint_0 %16 %uint_4
        %629 = OpExtInst %void %2 DebugLocalVariable %364 %208 %17 %uint_125 %uint_0 %16 %uint_4
        %647 = OpExtInst %void %2 DebugLocalVariable %648 %70 %17 %uint_128 %uint_0 %16 %uint_4
        %660 = OpExtInst %void %2 DebugLocalVariable %145 %25 %17 %uint_130 %uint_0 %16 %uint_4
        %671 = OpExtInst %void %2 DebugLocalVariable %672 %70 %17 %uint_134 %uint_0 %16 %uint_4
        %686 = OpExtInst %void %2 DebugLocalVariable %687 %25 %17 %uint_137 %uint_0 %16 %uint_4
        %693 = OpExtInst %void %2 DebugLocalVariable %694 %25 %17 %uint_138 %uint_0 %16 %uint_4
        %700 = OpExtInst %void %2 DebugLocalVariable %701 %25 %17 %uint_139 %uint_0 %16 %uint_4
        %707 = OpExtInst %void %2 DebugLocalVariable %708 %70 %17 %uint_142 %uint_0 %16 %uint_4
        %723 = OpExtInst %void %2 DebugLocalVariable %724 %25 %17 %uint_145 %uint_0 %16 %uint_4
        %732 = OpExtInst %void %2 DebugLocalVariable %733 %25 %17 %uint_146 %uint_0 %16 %uint_4
        %742 = OpExtInst %void %2 DebugLocalVariable %743 %25 %17 %uint_147 %uint_0 %16 %uint_4
        %751 = OpExtInst %void %2 DebugLocalVariable %752 %25 %17 %uint_150 %uint_0 %16 %uint_4
        %761 = OpExtInst %void %2 DebugLocalVariable %762 %70 %17 %uint_151 %uint_0 %16 %uint_4
        %769 = OpExtInst %void %2 DebugLocalVariable %770 %70 %17 %uint_154 %uint_0 %16 %uint_4
        %779 = OpExtInst %void %2 DebugLocalVariable %780 %25 %17 %uint_155 %uint_0 %16 %uint_4
        %789 = OpExtInst %void %2 DebugLocalVariable %790 %70 %17 %uint_156 %uint_0 %16 %uint_4
        %838 = OpExtInst %void %2 DebugLexicalBlock %17 %uint_0 %uint_0 %16
       %1294 = OpExtInst %void %2 DebugInlinedAt %uint_101 %512
       %1375 = OpExtInst %void %2 DebugInlinedAt %uint_80 %78 %1294
       %1552 = OpExtInst %void %2 DebugInlinedAt %uint_65 %60 %1375
       %1674 = OpExtInst %void %2 DebugInlinedAt %uint_164 %838
       %1755 = OpExtInst %void %2 DebugInlinedAt %uint_80 %78 %1674
       %1932 = OpExtInst %void %2 DebugInlinedAt %uint_65 %60 %1755
               OpLine %1 90 11
       %main = OpFunction %void None %5
         %22 = OpLabel
               OpLine %1 33 51
       %1931 = OpVariable %_ptr_Function_float Function
       %1933 = OpVariable %_ptr_Function_v4float Function
               OpLine %1 50 37
       %1754 = OpVariable %_ptr_Function_v2int Function
       %1759 = OpVariable %_ptr_Function_float Function
       %1760 = OpVariable %_ptr_Function_int Function
       %1762 = OpVariable %_ptr_Function_int Function
       %1763 = OpVariable %_ptr_Function_int Function
       %1764 = OpVariable %_ptr_Function_v4float Function
               OpLine %1 73 41
       %1673 = OpVariable %_ptr_Function_int Function
               OpLine %1 33 51
       %1551 = OpVariable %_ptr_Function_float Function
       %1553 = OpVariable %_ptr_Function_v4float Function
               OpLine %1 50 37
       %1374 = OpVariable %_ptr_Function_v2int Function
       %1379 = OpVariable %_ptr_Function_float Function
       %1380 = OpVariable %_ptr_Function_int Function
       %1382 = OpVariable %_ptr_Function_int Function
       %1383 = OpVariable %_ptr_Function_int Function
       %1384 = OpVariable %_ptr_Function_v4float Function
               OpLine %1 73 41
       %1293 = OpVariable %_ptr_Function_int Function
               OpLine %1 90 11
    %fragPos = OpVariable %_ptr_Function_v3float Function
     %normal = OpVariable %_ptr_Function_v3float Function
     %albedo = OpVariable %_ptr_Function_v4float Function
      %param = OpVariable %_ptr_Function_v3float Function
    %param_0 = OpVariable %_ptr_Function_v3float Function
  %fragcolor = OpVariable %_ptr_Function_v3float Function
          %N = OpVariable %_ptr_Function_v3float Function
          %i = OpVariable %_ptr_Function_int Function
          %L = OpVariable %_ptr_Function_v3float Function
       %dist = OpVariable %_ptr_Function_float Function
          %V = OpVariable %_ptr_Function_v3float Function
%lightCosInnerAngle = OpVariable %_ptr_Function_float Function
%lightCosOuterAngle = OpVariable %_ptr_Function_float Function
 %lightRange = OpVariable %_ptr_Function_float Function
        %dir = OpVariable %_ptr_Function_v3float Function
     %cosDir = OpVariable %_ptr_Function_float Function
 %spotEffect = OpVariable %_ptr_Function_float Function
%heightAttenuation = OpVariable %_ptr_Function_float Function
      %NdotL = OpVariable %_ptr_Function_float Function
       %diff = OpVariable %_ptr_Function_v3float Function
          %R = OpVariable %_ptr_Function_v3float Function
      %NdotR = OpVariable %_ptr_Function_float Function
       %spec = OpVariable %_ptr_Function_v3float Function
    %param_1 = OpVariable %_ptr_Function_v3float Function
    %param_2 = OpVariable %_ptr_Function_v3float Function
       %2171 = OpExtInst %void %2 DebugNoLine
               OpSelectionMerge %1288 None
               OpSwitch %uint_0 %1289
       %1289 = OpLabel
               OpLine %1 90 11
        %454 = OpExtInst %void %2 DebugFunctionDefinition %16 %main
       %2172 = OpExtInst %void %2 DebugScope %16
        %456 = OpExtInst %void %2 DebugLine %17 %uint_93 %uint_93 %uint_0 %uint_0
        %461 = OpExtInst %void %2 DebugDeclare %459 %fragPos %47
        %470 = OpLoad %464 %samplerposition
        %475 = OpLoad %v2float %inUV
        %476 = OpImageSampleImplicitLod %v4float %470 %475
        %477 = OpVectorShuffle %v3float %476 %476 0 1 2
               OpStore %fragPos %477
        %478 = OpExtInst %void %2 DebugLine %17 %uint_94 %uint_94 %uint_0 %uint_0
        %483 = OpExtInst %void %2 DebugDeclare %481 %normal %47
        %487 = OpLoad %464 %samplerNormal
        %488 = OpLoad %v2float %inUV
        %489 = OpImageSampleImplicitLod %v4float %487 %488
        %490 = OpVectorShuffle %v3float %489 %489 0 1 2
               OpStore %normal %490
        %491 = OpExtInst %void %2 DebugLine %17 %uint_95 %uint_95 %uint_0 %uint_0
        %496 = OpExtInst %void %2 DebugDeclare %494 %albedo %47
        %500 = OpLoad %464 %samplerAlbedo
        %501 = OpLoad %v2float %inUV
        %502 = OpImageSampleImplicitLod %v4float %500 %501
               OpStore %albedo %502
        %503 = OpExtInst %void %2 DebugLine %17 %uint_98 %uint_98 %uint_0 %uint_0
        %506 = OpAccessChain %_ptr_Uniform_int %ubo %int_3
        %507 = OpLoad %int %506
        %509 = OpSGreaterThan %bool %507 %int_0
       %2173 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %511 None
               OpBranchConditional %509 %510 %511
        %510 = OpLabel
       %2174 = OpExtInst %void %2 DebugScope %512
        %514 = OpExtInst %void %2 DebugLine %17 %uint_99 %uint_99 %uint_0 %uint_0
        %516 = OpAccessChain %_ptr_Uniform_int %ubo %int_3
        %517 = OpLoad %int %516
       %2175 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %523 None
               OpSwitch %517 %523 1 %518 2 %519 3 %520 4 %521 5 %522
        %518 = OpLabel
       %2176 = OpExtInst %void %2 DebugScope %512
        %525 = OpExtInst %void %2 DebugLine %17 %uint_101 %uint_101 %uint_0 %uint_0
               OpStore %param %531
               OpStore %param_0 %477
       %2177 = OpExtInst %void %2 DebugScope %78 %1294
       %1333 = OpExtInst %void %2 DebugLine %17 %uint_0 %uint_0 %uint_0 %uint_0
       %1301 = OpExtInst %void %2 DebugDeclare %82 %param %47
       %1302 = OpExtInst %void %2 DebugDeclare %85 %param_0 %47
       %1335 = OpExtInst %void %2 DebugLine %17 %uint_74 %uint_74 %uint_0 %uint_0
       %1304 = OpExtInst %void %2 DebugDeclare %363 %1293 %47
               OpStore %1293 %int_0
               OpBranch %1305
       %1305 = OpLabel
       %2178 = OpExtInst %void %2 DebugScope %78 %1294
       %1338 = OpExtInst %void %2 DebugLine %17 %uint_74 %uint_74 %uint_0 %uint_0
       %2179 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %1331 %1328 None
               OpBranch %1306
       %1306 = OpLabel
       %2180 = OpExtInst %void %2 DebugScope %78 %1294
       %1340 = OpExtInst %void %2 DebugLine %17 %uint_74 %uint_74 %uint_0 %uint_0
       %1307 = OpLoad %int %1293
       %1308 = OpSLessThan %bool %1307 %int_3
               OpBranchConditional %1308 %1309 %1331
       %1309 = OpLabel
       %2181 = OpExtInst %void %2 DebugScope %78 %1294
       %1344 = OpExtInst %void %2 DebugLine %17 %uint_76 %uint_76 %uint_0 %uint_0
       %1311 = OpLoad %int %1293
       %1312 = OpAccessChain %_ptr_Uniform_mat4v4float %ubo %int_1 %1311 %int_3
       %1313 = OpLoad %mat4v4float %1312
       %1315 = OpCompositeExtract %float %476 0
       %1316 = OpCompositeExtract %float %476 1
       %1317 = OpCompositeExtract %float %476 2
       %1318 = OpCompositeConstruct %v4float %1315 %1316 %1317 %float_1
       %1319 = OpMatrixTimesVector %v4float %1313 %1318
       %2158 = OpExtInst %void %2 DebugValue %383 %1319 %47
       %1356 = OpExtInst %void %2 DebugLine %17 %uint_80 %uint_80 %uint_0 %uint_0
       %1322 = OpConvertSToF %float %1311
       %2164 = OpExtInst %void %2 DebugValue %64 %1319 %47
       %2167 = OpExtInst %void %2 DebugValue %67 %1322 %47
       %2182 = OpExtInst %void %2 DebugScope %60 %1375
       %1463 = OpExtInst %void %2 DebugLine %17 %uint_52 %uint_52 %uint_0 %uint_0
       %1392 = OpExtInst %void %2 DebugDeclare %213 %1374 %47
       %1393 = OpLoad %152 %samplerShadowMap
       %1394 = OpImage %147 %1393
       %1395 = OpImageQuerySizeLod %v3int %1394 %int_0
       %1396 = OpVectorShuffle %v2int %1395 %1395 0 1
               OpStore %1374 %1396
       %2141 = OpExtInst %void %2 DebugLine %17 %uint_53 %uint_53 %uint_0 %uint_0
       %2140 = OpExtInst %void %2 DebugValue %226 %float_1_5 %47
       %1475 = OpExtInst %void %2 DebugLine %17 %uint_54 %uint_54 %uint_0 %uint_0
       %1402 = OpCompositeExtract %int %1395 0
       %1403 = OpConvertSToF %float %1402
       %1404 = OpFDiv %float %float_1_5 %1403
       %2143 = OpExtInst %void %2 DebugValue %233 %1404 %47
       %1483 = OpExtInst %void %2 DebugLine %17 %uint_55 %uint_55 %uint_0 %uint_0
       %1409 = OpCompositeExtract %int %1395 1
       %1410 = OpConvertSToF %float %1409
       %1411 = OpFDiv %float %float_1_5 %1410
       %2146 = OpExtInst %void %2 DebugValue %246 %1411 %47
       %1487 = OpExtInst %void %2 DebugLine %17 %uint_57 %uint_57 %uint_0 %uint_0
       %1412 = OpExtInst %void %2 DebugDeclare %258 %1379 %47
               OpStore %1379 %float_0
       %1489 = OpExtInst %void %2 DebugLine %17 %uint_58 %uint_58 %uint_0 %uint_0
       %1413 = OpExtInst %void %2 DebugDeclare %264 %1380 %47
               OpStore %1380 %int_0
       %2150 = OpExtInst %void %2 DebugLine %17 %uint_59 %uint_59 %uint_0 %uint_0
       %2149 = OpExtInst %void %2 DebugValue %270 %int_1 %47
       %1493 = OpExtInst %void %2 DebugLine %17 %uint_61 %uint_61 %uint_0 %uint_0
       %1415 = OpExtInst %void %2 DebugDeclare %277 %1382 %47
               OpStore %1382 %int_n1
               OpBranch %1418
       %1418 = OpLabel
       %2183 = OpExtInst %void %2 DebugScope %60 %1375
       %1498 = OpExtInst %void %2 DebugLine %17 %uint_61 %uint_61 %uint_0 %uint_0
       %2184 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %1456 %1453 None
               OpBranch %1419
       %1419 = OpLabel
       %2185 = OpExtInst %void %2 DebugScope %60 %1375
       %1500 = OpExtInst %void %2 DebugLine %17 %uint_61 %uint_61 %uint_0 %uint_0
       %1420 = OpLoad %int %1382
       %1422 = OpSLessThanEqual %bool %1420 %int_1
               OpBranchConditional %1422 %1423 %1456
       %1423 = OpLabel
       %2186 = OpExtInst %void %2 DebugScope %60 %1375
       %1504 = OpExtInst %void %2 DebugLine %17 %uint_63 %uint_63 %uint_0 %uint_0
       %1424 = OpExtInst %void %2 DebugDeclare %299 %1383 %47
               OpStore %1383 %int_n1
               OpBranch %1427
       %1427 = OpLabel
       %2187 = OpExtInst %void %2 DebugScope %60 %1375
       %1509 = OpExtInst %void %2 DebugLine %17 %uint_63 %uint_63 %uint_0 %uint_0
       %2188 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %1452 %1449 None
               OpBranch %1428
       %1428 = OpLabel
       %2189 = OpExtInst %void %2 DebugScope %60 %1375
       %1511 = OpExtInst %void %2 DebugLine %17 %uint_63 %uint_63 %uint_0 %uint_0
       %1429 = OpLoad %int %1383
       %1431 = OpSLessThanEqual %bool %1429 %int_1
               OpBranchConditional %1431 %1432 %1452
       %1432 = OpLabel
       %2190 = OpExtInst %void %2 DebugScope %60 %1375
       %1516 = OpExtInst %void %2 DebugLine %17 %uint_65 %uint_65 %uint_0 %uint_0
       %1434 = OpLoad %int %1382
       %1435 = OpConvertSToF %float %1434
       %1436 = OpFMul %float %1404 %1435
       %1438 = OpLoad %int %1383
       %1439 = OpConvertSToF %float %1438
       %1440 = OpFMul %float %1411 %1439
       %1441 = OpCompositeConstruct %v2float %1436 %1440
               OpStore %1384 %1319
       %2152 = OpExtInst %void %2 DebugValue %48 %1322 %47
       %2155 = OpExtInst %void %2 DebugValue %51 %1441 %47
       %2191 = OpExtInst %void %2 DebugScope %40 %1552
       %1613 = OpExtInst %void %2 DebugLine %17 %uint_0 %uint_0 %uint_0 %uint_0
       %1557 = OpExtInst %void %2 DebugDeclare %44 %1384 %47
       %1616 = OpExtInst %void %2 DebugLine %17 %uint_35 %uint_35 %uint_0 %uint_0
       %1561 = OpExtInst %void %2 DebugDeclare %93 %1551 %47
               OpStore %1551 %float_1
       %1618 = OpExtInst %void %2 DebugLine %17 %uint_36 %uint_36 %uint_0 %uint_0
       %1562 = OpExtInst %void %2 DebugDeclare %99 %1553 %47
       %1565 = OpCompositeExtract %float %1319 3
       %1566 = OpCompositeConstruct %v4float %1565 %1565 %1565 %1565
       %1567 = OpFDiv %v4float %1319 %1566
               OpStore %1553 %1567
       %1626 = OpExtInst %void %2 DebugLine %17 %uint_37 %uint_37 %uint_0 %uint_0
       %1569 = OpVectorShuffle %v2float %1567 %1567 0 1
       %1570 = OpVectorTimesScalar %v2float %1569 %float_0_5
       %1572 = OpFAdd %v2float %1570 %2170
       %1574 = OpCompositeExtract %float %1572 0
       %2060 = OpCompositeInsert %v4float %1574 %1567 0
               OpStore %1553 %2060
       %1576 = OpCompositeExtract %float %1572 1
       %2065 = OpCompositeInsert %v4float %1576 %2060 1
               OpStore %1553 %2065
       %1637 = OpExtInst %void %2 DebugLine %17 %uint_39 %uint_39 %uint_0 %uint_0
       %1578 = OpCompositeExtract %float %1567 2
       %1579 = OpFOrdGreaterThan %bool %1578 %float_n1
       %2192 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %1584 None
               OpBranchConditional %1579 %1580 %1584
       %1580 = OpLabel
       %2193 = OpExtInst %void %2 DebugScope %40 %1552
       %2072 = OpExtInst %void %2 DebugLine %17 %uint_39 %uint_39 %uint_0 %uint_0
       %2071 = OpLoad %v4float %1553
       %1582 = OpCompositeExtract %float %2071 2
       %1583 = OpFOrdLessThan %bool %1582 %float_1
               OpBranch %1584
       %1584 = OpLabel
       %1585 = OpPhi %bool %1579 %1432 %1583 %1580
               OpSelectionMerge %1611 None
               OpBranchConditional %1585 %1586 %1611
       %1586 = OpLabel
       %2194 = OpExtInst %void %2 DebugScope %139 %1552
       %1646 = OpExtInst %void %2 DebugLine %17 %uint_41 %uint_41 %uint_0 %uint_0
       %1588 = OpLoad %152 %samplerShadowMap
       %1589 = OpLoad %v4float %1553
       %1590 = OpVectorShuffle %v2float %1589 %1589 0 1
       %1592 = OpFAdd %v2float %1590 %1441
       %1594 = OpCompositeExtract %float %1592 0
       %1595 = OpCompositeExtract %float %1592 1
       %1596 = OpCompositeConstruct %v3float %1594 %1595 %1322
       %1597 = OpImageSampleImplicitLod %v4float %1588 %1596
       %1598 = OpCompositeExtract %float %1597 0
       %2137 = OpExtInst %void %2 DebugValue %144 %1598 %47
       %1659 = OpExtInst %void %2 DebugLine %17 %uint_42 %uint_42 %uint_0 %uint_0
       %1600 = OpCompositeExtract %float %1589 3
       %1601 = OpFOrdGreaterThan %bool %1600 %float_0
       %2195 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %1607 None
               OpBranchConditional %1601 %1602 %1607
       %1602 = OpLabel
       %2196 = OpExtInst %void %2 DebugScope %139 %1552
       %2076 = OpExtInst %void %2 DebugLine %17 %uint_42 %uint_42 %uint_0 %uint_0
       %2075 = OpLoad %v4float %1553
       %1605 = OpCompositeExtract %float %2075 2
       %1606 = OpFOrdLessThan %bool %1598 %1605
               OpBranch %1607
       %1607 = OpLabel
       %1608 = OpPhi %bool %1601 %1586 %1606 %1602
               OpSelectionMerge %1610 None
               OpBranchConditional %1608 %1609 %1610
       %1609 = OpLabel
       %2197 = OpExtInst %void %2 DebugScope %191 %1552
       %1668 = OpExtInst %void %2 DebugLine %17 %uint_44 %uint_44 %uint_0 %uint_0
               OpStore %1551 %float_0_25
               OpBranch %1610
       %1610 = OpLabel
       %2198 = OpExtInst %void %2 DebugNoScope
               OpBranch %1611
       %1611 = OpLabel
       %2199 = OpExtInst %void %2 DebugScope %40 %1552
       %1670 = OpExtInst %void %2 DebugLine %17 %uint_47 %uint_47 %uint_0 %uint_0
       %1612 = OpLoad %float %1551
       %2200 = OpExtInst %void %2 DebugScope %60 %1375
       %1530 = OpExtInst %void %2 DebugLine %17 %uint_65 %uint_65 %uint_0 %uint_0
       %1445 = OpLoad %float %1379
       %1446 = OpFAdd %float %1445 %1612
               OpStore %1379 %1446
       %1533 = OpExtInst %void %2 DebugLine %17 %uint_66 %uint_66 %uint_0 %uint_0
       %1447 = OpLoad %int %1380
       %1448 = OpIAdd %int %1447 %int_1
               OpStore %1380 %1448
               OpBranch %1449
       %1449 = OpLabel
       %2201 = OpExtInst %void %2 DebugScope %60 %1375
       %1537 = OpExtInst %void %2 DebugLine %17 %uint_63 %uint_63 %uint_0 %uint_0
       %1450 = OpLoad %int %1383
       %1451 = OpIAdd %int %1450 %int_1
               OpStore %1383 %1451
               OpBranch %1427
       %1452 = OpLabel
       %2202 = OpExtInst %void %2 DebugNoScope
               OpBranch %1453
       %1453 = OpLabel
       %2203 = OpExtInst %void %2 DebugScope %60 %1375
       %1541 = OpExtInst %void %2 DebugLine %17 %uint_61 %uint_61 %uint_0 %uint_0
       %1454 = OpLoad %int %1382
       %1455 = OpIAdd %int %1454 %int_1
               OpStore %1382 %1455
               OpBranch %1418
       %1456 = OpLabel
       %2204 = OpExtInst %void %2 DebugScope %60 %1375
       %1545 = OpExtInst %void %2 DebugLine %17 %uint_70 %uint_70 %uint_0 %uint_0
       %1457 = OpLoad %float %1379
       %1458 = OpLoad %int %1380
       %1459 = OpConvertSToF %float %1458
       %1460 = OpFDiv %float %1457 %1459
       %2205 = OpExtInst %void %2 DebugScope %78 %1294
       %2162 = OpExtInst %void %2 DebugLine %17 %uint_80 %uint_80 %uint_0 %uint_0
       %2161 = OpExtInst %void %2 DebugValue %431 %1460 %47
       %1363 = OpExtInst %void %2 DebugLine %17 %uint_85 %uint_85 %uint_0 %uint_0
       %1326 = OpLoad %v3float %param
       %1327 = OpVectorTimesScalar %v3float %1326 %1460
               OpStore %param %1327
               OpBranch %1328
       %1328 = OpLabel
       %2206 = OpExtInst %void %2 DebugScope %78 %1294
       %1367 = OpExtInst %void %2 DebugLine %17 %uint_74 %uint_74 %uint_0 %uint_0
       %1329 = OpLoad %int %1293
       %1330 = OpIAdd %int %1329 %int_1
               OpStore %1293 %1330
               OpBranch %1305
       %1331 = OpLabel
       %2207 = OpExtInst %void %2 DebugScope %78 %1294
       %1371 = OpExtInst %void %2 DebugLine %17 %uint_87 %uint_87 %uint_0 %uint_0
       %1332 = OpLoad %v3float %param
       %2208 = OpExtInst %void %2 DebugScope %512
        %884 = OpExtInst %void %2 DebugLine %17 %uint_101 %uint_101 %uint_0 %uint_0
        %537 = OpAccessChain %_ptr_Output_float %outFragColor %uint_0
        %538 = OpCompositeExtract %float %1332 0
               OpStore %537 %538
        %539 = OpAccessChain %_ptr_Output_float %outFragColor %uint_1
        %540 = OpCompositeExtract %float %1332 1
               OpStore %539 %540
        %541 = OpAccessChain %_ptr_Output_float %outFragColor %uint_2
        %542 = OpCompositeExtract %float %1332 2
               OpStore %541 %542
        %543 = OpExtInst %void %2 DebugLine %17 %uint_102 %uint_102 %uint_0 %uint_0
               OpBranch %523
        %519 = OpLabel
       %2209 = OpExtInst %void %2 DebugScope %512
        %895 = OpExtInst %void %2 DebugLine %17 %uint_104 %uint_104 %uint_0 %uint_0
        %550 = OpAccessChain %_ptr_Output_float %outFragColor %uint_0
        %551 = OpCompositeExtract %float %476 0
               OpStore %550 %551
        %552 = OpAccessChain %_ptr_Output_float %outFragColor %uint_1
        %553 = OpCompositeExtract %float %476 1
               OpStore %552 %553
        %554 = OpAccessChain %_ptr_Output_float %outFragColor %uint_2
        %555 = OpCompositeExtract %float %476 2
               OpStore %554 %555
        %556 = OpExtInst %void %2 DebugLine %17 %uint_105 %uint_105 %uint_0 %uint_0
               OpBranch %523
        %520 = OpLabel
       %2210 = OpExtInst %void %2 DebugScope %512
        %906 = OpExtInst %void %2 DebugLine %17 %uint_107 %uint_107 %uint_0 %uint_0
        %563 = OpAccessChain %_ptr_Output_float %outFragColor %uint_0
        %564 = OpCompositeExtract %float %489 0
               OpStore %563 %564
        %565 = OpAccessChain %_ptr_Output_float %outFragColor %uint_1
        %566 = OpCompositeExtract %float %489 1
               OpStore %565 %566
        %567 = OpAccessChain %_ptr_Output_float %outFragColor %uint_2
        %568 = OpCompositeExtract %float %489 2
               OpStore %567 %568
        %569 = OpExtInst %void %2 DebugLine %17 %uint_108 %uint_108 %uint_0 %uint_0
               OpBranch %523
        %521 = OpLabel
       %2211 = OpExtInst %void %2 DebugScope %512
        %918 = OpExtInst %void %2 DebugLine %17 %uint_110 %uint_110 %uint_0 %uint_0
        %577 = OpAccessChain %_ptr_Output_float %outFragColor %uint_0
        %578 = OpCompositeExtract %float %502 0
               OpStore %577 %578
        %579 = OpAccessChain %_ptr_Output_float %outFragColor %uint_1
        %580 = OpCompositeExtract %float %502 1
               OpStore %579 %580
        %581 = OpAccessChain %_ptr_Output_float %outFragColor %uint_2
        %582 = OpCompositeExtract %float %502 2
               OpStore %581 %582
        %583 = OpExtInst %void %2 DebugLine %17 %uint_111 %uint_111 %uint_0 %uint_0
               OpBranch %523
        %522 = OpLabel
       %2212 = OpExtInst %void %2 DebugScope %512
        %930 = OpExtInst %void %2 DebugLine %17 %uint_113 %uint_113 %uint_0 %uint_0
        %591 = OpAccessChain %_ptr_Output_float %outFragColor %uint_0
        %592 = OpCompositeExtract %float %502 3
               OpStore %591 %592
        %593 = OpAccessChain %_ptr_Output_float %outFragColor %uint_1
        %594 = OpCompositeExtract %float %502 3
               OpStore %593 %594
        %595 = OpAccessChain %_ptr_Output_float %outFragColor %uint_2
        %596 = OpCompositeExtract %float %502 3
               OpStore %595 %596
        %597 = OpExtInst %void %2 DebugLine %17 %uint_114 %uint_114 %uint_0 %uint_0
               OpBranch %523
        %523 = OpLabel
       %2213 = OpExtInst %void %2 DebugScope %512
        %602 = OpExtInst %void %2 DebugLine %17 %uint_116 %uint_116 %uint_0 %uint_0
        %604 = OpAccessChain %_ptr_Output_float %outFragColor %uint_3
               OpStore %604 %float_1
        %605 = OpExtInst %void %2 DebugLine %17 %uint_117 %uint_117 %uint_0 %uint_0
               OpBranch %1288
        %511 = OpLabel
       %2214 = OpExtInst %void %2 DebugScope %16
        %609 = OpExtInst %void %2 DebugLine %17 %uint_121 %uint_121 %uint_0 %uint_0
        %613 = OpExtInst %void %2 DebugDeclare %612 %fragcolor %47
        %615 = OpVectorShuffle %v3float %502 %502 0 1 2
        %617 = OpVectorTimesScalar %v3float %615 %float_0_100000001
               OpStore %fragcolor %617
        %618 = OpExtInst %void %2 DebugLine %17 %uint_123 %uint_123 %uint_0 %uint_0
        %623 = OpExtInst %void %2 DebugDeclare %621 %N %47
        %625 = OpExtInst %v3float %3 Normalize %490
               OpStore %N %625
        %626 = OpExtInst %void %2 DebugLine %17 %uint_125 %uint_125 %uint_0 %uint_0
        %630 = OpExtInst %void %2 DebugDeclare %629 %i %47
               OpStore %i %int_0
               OpBranch %631
        %631 = OpLabel
       %2215 = OpExtInst %void %2 DebugScope %16
        %636 = OpExtInst %void %2 DebugLine %17 %uint_125 %uint_125 %uint_0 %uint_0
       %2216 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %633 %634 None
               OpBranch %637
        %637 = OpLabel
       %2217 = OpExtInst %void %2 DebugScope %16
        %639 = OpExtInst %void %2 DebugLine %17 %uint_125 %uint_125 %uint_0 %uint_0
        %640 = OpLoad %int %i
        %642 = OpSLessThan %bool %640 %int_3
               OpBranchConditional %642 %632 %633
        %632 = OpLabel
       %2218 = OpExtInst %void %2 DebugScope %16
        %644 = OpExtInst %void %2 DebugLine %17 %uint_128 %uint_128 %uint_0 %uint_0
        %649 = OpExtInst %void %2 DebugDeclare %647 %L %47
        %650 = OpLoad %int %i
        %652 = OpAccessChain %_ptr_Uniform_v4float %ubo %int_1 %650 %int_0
        %653 = OpLoad %v4float %652
        %654 = OpVectorShuffle %v3float %653 %653 0 1 2
        %656 = OpFSub %v3float %654 %477
               OpStore %L %656
        %657 = OpExtInst %void %2 DebugLine %17 %uint_130 %uint_130 %uint_0 %uint_0
        %661 = OpExtInst %void %2 DebugDeclare %660 %dist %47
        %663 = OpExtInst %float %3 Length %656
               OpStore %dist %663
        %973 = OpExtInst %void %2 DebugLine %17 %uint_131 %uint_131 %uint_0 %uint_0
        %667 = OpExtInst %v3float %3 Normalize %656
               OpStore %L %667
        %668 = OpExtInst %void %2 DebugLine %17 %uint_134 %uint_134 %uint_0 %uint_0
        %673 = OpExtInst %void %2 DebugDeclare %671 %V %47
        %674 = OpAccessChain %_ptr_Uniform_v4float %ubo %int_0
        %675 = OpLoad %v4float %674
        %676 = OpVectorShuffle %v3float %675 %675 0 1 2
        %678 = OpFSub %v3float %676 %477
               OpStore %V %678
        %983 = OpExtInst %void %2 DebugLine %17 %uint_135 %uint_135 %uint_0 %uint_0
        %682 = OpExtInst %v3float %3 Normalize %678
               OpStore %V %682
        %683 = OpExtInst %void %2 DebugLine %17 %uint_137 %uint_137 %uint_0 %uint_0
        %688 = OpExtInst %void %2 DebugDeclare %686 %lightCosInnerAngle %47
               OpStore %lightCosInnerAngle %float_0_965925813
        %690 = OpExtInst %void %2 DebugLine %17 %uint_138 %uint_138 %uint_0 %uint_0
        %695 = OpExtInst %void %2 DebugDeclare %693 %lightCosOuterAngle %47
               OpStore %lightCosOuterAngle %float_0_906307817
        %697 = OpExtInst %void %2 DebugLine %17 %uint_139 %uint_139 %uint_0 %uint_0
        %702 = OpExtInst %void %2 DebugDeclare %700 %lightRange %47
               OpStore %lightRange %float_100
        %704 = OpExtInst %void %2 DebugLine %17 %uint_142 %uint_142 %uint_0 %uint_0
        %709 = OpExtInst %void %2 DebugDeclare %707 %dir %47
        %711 = OpAccessChain %_ptr_Uniform_v4float %ubo %int_1 %650 %int_0
        %712 = OpLoad %v4float %711
        %713 = OpVectorShuffle %v3float %712 %712 0 1 2
        %715 = OpAccessChain %_ptr_Uniform_v4float %ubo %int_1 %650 %int_1
        %716 = OpLoad %v4float %715
        %717 = OpVectorShuffle %v3float %716 %716 0 1 2
        %718 = OpFSub %v3float %713 %717
        %719 = OpExtInst %v3float %3 Normalize %718
               OpStore %dir %719
        %720 = OpExtInst %void %2 DebugLine %17 %uint_145 %uint_145 %uint_0 %uint_0
        %725 = OpExtInst %void %2 DebugDeclare %723 %cosDir %47
        %728 = OpDot %float %667 %719
               OpStore %cosDir %728
        %729 = OpExtInst %void %2 DebugLine %17 %uint_146 %uint_146 %uint_0 %uint_0
        %734 = OpExtInst %void %2 DebugDeclare %732 %spotEffect %47
        %738 = OpExtInst %float %3 SmoothStep %float_0_906307817 %float_0_965925813 %728
               OpStore %spotEffect %738
        %739 = OpExtInst %void %2 DebugLine %17 %uint_147 %uint_147 %uint_0 %uint_0
        %744 = OpExtInst %void %2 DebugDeclare %742 %heightAttenuation %47
        %747 = OpExtInst %float %3 SmoothStep %float_100 %float_0 %663
               OpStore %heightAttenuation %747
        %748 = OpExtInst %void %2 DebugLine %17 %uint_150 %uint_150 %uint_0 %uint_0
        %753 = OpExtInst %void %2 DebugDeclare %751 %NdotL %47
        %756 = OpDot %float %625 %667
        %757 = OpExtInst %float %3 FMax %float_0 %756
               OpStore %NdotL %757
        %758 = OpExtInst %void %2 DebugLine %17 %uint_151 %uint_151 %uint_0 %uint_0
        %763 = OpExtInst %void %2 DebugDeclare %761 %diff %47
        %765 = OpCompositeConstruct %v3float %757 %757 %757
               OpStore %diff %765
        %766 = OpExtInst %void %2 DebugLine %17 %uint_154 %uint_154 %uint_0 %uint_0
        %771 = OpExtInst %void %2 DebugDeclare %769 %R %47
        %773 = OpFNegate %v3float %667
        %775 = OpExtInst %v3float %3 Reflect %773 %625
               OpStore %R %775
        %776 = OpExtInst %void %2 DebugLine %17 %uint_155 %uint_155 %uint_0 %uint_0
        %781 = OpExtInst %void %2 DebugDeclare %779 %NdotR %47
        %784 = OpDot %float %775 %682
        %785 = OpExtInst %float %3 FMax %float_0 %784
               OpStore %NdotR %785
        %786 = OpExtInst %void %2 DebugLine %17 %uint_156 %uint_156 %uint_0 %uint_0
        %791 = OpExtInst %void %2 DebugDeclare %789 %spec %47
        %794 = OpExtInst %float %3 Pow %785 %float_16
        %796 = OpCompositeExtract %float %502 3
        %797 = OpFMul %float %794 %796
        %799 = OpFMul %float %797 %float_2_5
        %800 = OpCompositeConstruct %v3float %799 %799 %799
               OpStore %spec %800
       %1052 = OpExtInst %void %2 DebugLine %17 %uint_158 %uint_158 %uint_0 %uint_0
        %805 = OpFAdd %v3float %765 %800
        %807 = OpVectorTimesScalar %v3float %805 %738
        %809 = OpVectorTimesScalar %v3float %807 %747
        %816 = OpAccessChain %_ptr_Uniform_v4float %ubo %int_1 %650 %int_2
        %817 = OpLoad %v4float %816
        %818 = OpVectorShuffle %v3float %817 %817 0 1 2
        %819 = OpFMul %v3float %809 %818
        %821 = OpVectorShuffle %v3float %502 %502 0 1 2
        %823 = OpLoad %v3float %fragcolor
        %824 = OpExtInst %v3float %3 Fma %819 %821 %823
               OpStore %fragcolor %824
               OpBranch %634
        %634 = OpLabel
       %2219 = OpExtInst %void %2 DebugScope %16
        %826 = OpExtInst %void %2 DebugLine %17 %uint_125 %uint_125 %uint_0 %uint_0
        %827 = OpLoad %int %i
        %828 = OpIAdd %int %827 %int_1
               OpStore %i %828
               OpBranch %631
        %633 = OpLabel
       %2220 = OpExtInst %void %2 DebugScope %16
        %830 = OpExtInst %void %2 DebugLine %17 %uint_162 %uint_162 %uint_0 %uint_0
        %832 = OpAccessChain %_ptr_Uniform_int %ubo %int_2
        %833 = OpLoad %int %832
        %835 = OpSGreaterThan %bool %833 %int_0
       %2221 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %837 None
               OpBranchConditional %835 %836 %837
        %836 = OpLabel
       %2222 = OpExtInst %void %2 DebugScope %838
        %840 = OpExtInst %void %2 DebugLine %17 %uint_164 %uint_164 %uint_0 %uint_0
        %843 = OpLoad %v3float %fragcolor
               OpStore %param_1 %843
               OpStore %param_2 %477
       %2223 = OpExtInst %void %2 DebugScope %78 %1674
       %1713 = OpExtInst %void %2 DebugLine %17 %uint_0 %uint_0 %uint_0 %uint_0
       %1681 = OpExtInst %void %2 DebugDeclare %82 %param_1 %47
       %1682 = OpExtInst %void %2 DebugDeclare %85 %param_2 %47
       %1715 = OpExtInst %void %2 DebugLine %17 %uint_74 %uint_74 %uint_0 %uint_0
       %1684 = OpExtInst %void %2 DebugDeclare %363 %1673 %47
               OpStore %1673 %int_0
               OpBranch %1685
       %1685 = OpLabel
       %2224 = OpExtInst %void %2 DebugScope %78 %1674
       %1718 = OpExtInst %void %2 DebugLine %17 %uint_74 %uint_74 %uint_0 %uint_0
       %2225 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %1711 %1708 None
               OpBranch %1686
       %1686 = OpLabel
       %2226 = OpExtInst %void %2 DebugScope %78 %1674
       %1720 = OpExtInst %void %2 DebugLine %17 %uint_74 %uint_74 %uint_0 %uint_0
       %1687 = OpLoad %int %1673
       %1688 = OpSLessThan %bool %1687 %int_3
               OpBranchConditional %1688 %1689 %1711
       %1689 = OpLabel
       %2227 = OpExtInst %void %2 DebugScope %78 %1674
       %1724 = OpExtInst %void %2 DebugLine %17 %uint_76 %uint_76 %uint_0 %uint_0
       %1691 = OpLoad %int %1673
       %1692 = OpAccessChain %_ptr_Uniform_mat4v4float %ubo %int_1 %1691 %int_3
       %1693 = OpLoad %mat4v4float %1692
       %1695 = OpCompositeExtract %float %476 0
       %1696 = OpCompositeExtract %float %476 1
       %1697 = OpCompositeExtract %float %476 2
       %1698 = OpCompositeConstruct %v4float %1695 %1696 %1697 %float_1
       %1699 = OpMatrixTimesVector %v4float %1693 %1698
       %2125 = OpExtInst %void %2 DebugValue %383 %1699 %47
       %1736 = OpExtInst %void %2 DebugLine %17 %uint_80 %uint_80 %uint_0 %uint_0
       %1702 = OpConvertSToF %float %1691
       %2131 = OpExtInst %void %2 DebugValue %64 %1699 %47
       %2134 = OpExtInst %void %2 DebugValue %67 %1702 %47
       %2228 = OpExtInst %void %2 DebugScope %60 %1755
       %1843 = OpExtInst %void %2 DebugLine %17 %uint_52 %uint_52 %uint_0 %uint_0
       %1772 = OpExtInst %void %2 DebugDeclare %213 %1754 %47
       %1773 = OpLoad %152 %samplerShadowMap
       %1774 = OpImage %147 %1773
       %1775 = OpImageQuerySizeLod %v3int %1774 %int_0
       %1776 = OpVectorShuffle %v2int %1775 %1775 0 1
               OpStore %1754 %1776
       %2108 = OpExtInst %void %2 DebugLine %17 %uint_53 %uint_53 %uint_0 %uint_0
       %2107 = OpExtInst %void %2 DebugValue %226 %float_1_5 %47
       %1855 = OpExtInst %void %2 DebugLine %17 %uint_54 %uint_54 %uint_0 %uint_0
       %1782 = OpCompositeExtract %int %1775 0
       %1783 = OpConvertSToF %float %1782
       %1784 = OpFDiv %float %float_1_5 %1783
       %2110 = OpExtInst %void %2 DebugValue %233 %1784 %47
       %1863 = OpExtInst %void %2 DebugLine %17 %uint_55 %uint_55 %uint_0 %uint_0
       %1789 = OpCompositeExtract %int %1775 1
       %1790 = OpConvertSToF %float %1789
       %1791 = OpFDiv %float %float_1_5 %1790
       %2113 = OpExtInst %void %2 DebugValue %246 %1791 %47
       %1867 = OpExtInst %void %2 DebugLine %17 %uint_57 %uint_57 %uint_0 %uint_0
       %1792 = OpExtInst %void %2 DebugDeclare %258 %1759 %47
               OpStore %1759 %float_0
       %1869 = OpExtInst %void %2 DebugLine %17 %uint_58 %uint_58 %uint_0 %uint_0
       %1793 = OpExtInst %void %2 DebugDeclare %264 %1760 %47
               OpStore %1760 %int_0
       %2117 = OpExtInst %void %2 DebugLine %17 %uint_59 %uint_59 %uint_0 %uint_0
       %2116 = OpExtInst %void %2 DebugValue %270 %int_1 %47
       %1873 = OpExtInst %void %2 DebugLine %17 %uint_61 %uint_61 %uint_0 %uint_0
       %1795 = OpExtInst %void %2 DebugDeclare %277 %1762 %47
               OpStore %1762 %int_n1
               OpBranch %1798
       %1798 = OpLabel
       %2229 = OpExtInst %void %2 DebugScope %60 %1755
       %1878 = OpExtInst %void %2 DebugLine %17 %uint_61 %uint_61 %uint_0 %uint_0
       %2230 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %1836 %1833 None
               OpBranch %1799
       %1799 = OpLabel
       %2231 = OpExtInst %void %2 DebugScope %60 %1755
       %1880 = OpExtInst %void %2 DebugLine %17 %uint_61 %uint_61 %uint_0 %uint_0
       %1800 = OpLoad %int %1762
       %1802 = OpSLessThanEqual %bool %1800 %int_1
               OpBranchConditional %1802 %1803 %1836
       %1803 = OpLabel
       %2232 = OpExtInst %void %2 DebugScope %60 %1755
       %1884 = OpExtInst %void %2 DebugLine %17 %uint_63 %uint_63 %uint_0 %uint_0
       %1804 = OpExtInst %void %2 DebugDeclare %299 %1763 %47
               OpStore %1763 %int_n1
               OpBranch %1807
       %1807 = OpLabel
       %2233 = OpExtInst %void %2 DebugScope %60 %1755
       %1889 = OpExtInst %void %2 DebugLine %17 %uint_63 %uint_63 %uint_0 %uint_0
       %2234 = OpExtInst %void %2 DebugNoScope
               OpLoopMerge %1832 %1829 None
               OpBranch %1808
       %1808 = OpLabel
       %2235 = OpExtInst %void %2 DebugScope %60 %1755
       %1891 = OpExtInst %void %2 DebugLine %17 %uint_63 %uint_63 %uint_0 %uint_0
       %1809 = OpLoad %int %1763
       %1811 = OpSLessThanEqual %bool %1809 %int_1
               OpBranchConditional %1811 %1812 %1832
       %1812 = OpLabel
       %2236 = OpExtInst %void %2 DebugScope %60 %1755
       %1896 = OpExtInst %void %2 DebugLine %17 %uint_65 %uint_65 %uint_0 %uint_0
       %1814 = OpLoad %int %1762
       %1815 = OpConvertSToF %float %1814
       %1816 = OpFMul %float %1784 %1815
       %1818 = OpLoad %int %1763
       %1819 = OpConvertSToF %float %1818
       %1820 = OpFMul %float %1791 %1819
       %1821 = OpCompositeConstruct %v2float %1816 %1820
               OpStore %1764 %1699
       %2119 = OpExtInst %void %2 DebugValue %48 %1702 %47
       %2122 = OpExtInst %void %2 DebugValue %51 %1821 %47
       %2237 = OpExtInst %void %2 DebugScope %40 %1932
       %1993 = OpExtInst %void %2 DebugLine %17 %uint_0 %uint_0 %uint_0 %uint_0
       %1937 = OpExtInst %void %2 DebugDeclare %44 %1764 %47
       %1996 = OpExtInst %void %2 DebugLine %17 %uint_35 %uint_35 %uint_0 %uint_0
       %1941 = OpExtInst %void %2 DebugDeclare %93 %1931 %47
               OpStore %1931 %float_1
       %1998 = OpExtInst %void %2 DebugLine %17 %uint_36 %uint_36 %uint_0 %uint_0
       %1942 = OpExtInst %void %2 DebugDeclare %99 %1933 %47
       %1945 = OpCompositeExtract %float %1699 3
       %1946 = OpCompositeConstruct %v4float %1945 %1945 %1945 %1945
       %1947 = OpFDiv %v4float %1699 %1946
               OpStore %1933 %1947
       %2006 = OpExtInst %void %2 DebugLine %17 %uint_37 %uint_37 %uint_0 %uint_0
       %1949 = OpVectorShuffle %v2float %1947 %1947 0 1
       %1950 = OpVectorTimesScalar %v2float %1949 %float_0_5
       %1952 = OpFAdd %v2float %1950 %2170
       %1954 = OpCompositeExtract %float %1952 0
       %2086 = OpCompositeInsert %v4float %1954 %1947 0
               OpStore %1933 %2086
       %1956 = OpCompositeExtract %float %1952 1
       %2091 = OpCompositeInsert %v4float %1956 %2086 1
               OpStore %1933 %2091
       %2017 = OpExtInst %void %2 DebugLine %17 %uint_39 %uint_39 %uint_0 %uint_0
       %1958 = OpCompositeExtract %float %1947 2
       %1959 = OpFOrdGreaterThan %bool %1958 %float_n1
       %2238 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %1964 None
               OpBranchConditional %1959 %1960 %1964
       %1960 = OpLabel
       %2239 = OpExtInst %void %2 DebugScope %40 %1932
       %2098 = OpExtInst %void %2 DebugLine %17 %uint_39 %uint_39 %uint_0 %uint_0
       %2097 = OpLoad %v4float %1933
       %1962 = OpCompositeExtract %float %2097 2
       %1963 = OpFOrdLessThan %bool %1962 %float_1
               OpBranch %1964
       %1964 = OpLabel
       %1965 = OpPhi %bool %1959 %1812 %1963 %1960
               OpSelectionMerge %1991 None
               OpBranchConditional %1965 %1966 %1991
       %1966 = OpLabel
       %2240 = OpExtInst %void %2 DebugScope %139 %1932
       %2026 = OpExtInst %void %2 DebugLine %17 %uint_41 %uint_41 %uint_0 %uint_0
       %1968 = OpLoad %152 %samplerShadowMap
       %1969 = OpLoad %v4float %1933
       %1970 = OpVectorShuffle %v2float %1969 %1969 0 1
       %1972 = OpFAdd %v2float %1970 %1821
       %1974 = OpCompositeExtract %float %1972 0
       %1975 = OpCompositeExtract %float %1972 1
       %1976 = OpCompositeConstruct %v3float %1974 %1975 %1702
       %1977 = OpImageSampleImplicitLod %v4float %1968 %1976
       %1978 = OpCompositeExtract %float %1977 0
       %2104 = OpExtInst %void %2 DebugValue %144 %1978 %47
       %2039 = OpExtInst %void %2 DebugLine %17 %uint_42 %uint_42 %uint_0 %uint_0
       %1980 = OpCompositeExtract %float %1969 3
       %1981 = OpFOrdGreaterThan %bool %1980 %float_0
       %2241 = OpExtInst %void %2 DebugNoScope
               OpSelectionMerge %1987 None
               OpBranchConditional %1981 %1982 %1987
       %1982 = OpLabel
       %2242 = OpExtInst %void %2 DebugScope %139 %1932
       %2102 = OpExtInst %void %2 DebugLine %17 %uint_42 %uint_42 %uint_0 %uint_0
       %2101 = OpLoad %v4float %1933
       %1985 = OpCompositeExtract %float %2101 2
       %1986 = OpFOrdLessThan %bool %1978 %1985
               OpBranch %1987
       %1987 = OpLabel
       %1988 = OpPhi %bool %1981 %1966 %1986 %1982
               OpSelectionMerge %1990 None
               OpBranchConditional %1988 %1989 %1990
       %1989 = OpLabel
       %2243 = OpExtInst %void %2 DebugScope %191 %1932
       %2048 = OpExtInst %void %2 DebugLine %17 %uint_44 %uint_44 %uint_0 %uint_0
               OpStore %1931 %float_0_25
               OpBranch %1990
       %1990 = OpLabel
       %2244 = OpExtInst %void %2 DebugNoScope
               OpBranch %1991
       %1991 = OpLabel
       %2245 = OpExtInst %void %2 DebugScope %40 %1932
       %2050 = OpExtInst %void %2 DebugLine %17 %uint_47 %uint_47 %uint_0 %uint_0
       %1992 = OpLoad %float %1931
       %2246 = OpExtInst %void %2 DebugScope %60 %1755
       %1910 = OpExtInst %void %2 DebugLine %17 %uint_65 %uint_65 %uint_0 %uint_0
       %1825 = OpLoad %float %1759
       %1826 = OpFAdd %float %1825 %1992
               OpStore %1759 %1826
       %1913 = OpExtInst %void %2 DebugLine %17 %uint_66 %uint_66 %uint_0 %uint_0
       %1827 = OpLoad %int %1760
       %1828 = OpIAdd %int %1827 %int_1
               OpStore %1760 %1828
               OpBranch %1829
       %1829 = OpLabel
       %2247 = OpExtInst %void %2 DebugScope %60 %1755
       %1917 = OpExtInst %void %2 DebugLine %17 %uint_63 %uint_63 %uint_0 %uint_0
       %1830 = OpLoad %int %1763
       %1831 = OpIAdd %int %1830 %int_1
               OpStore %1763 %1831
               OpBranch %1807
       %1832 = OpLabel
       %2248 = OpExtInst %void %2 DebugNoScope
               OpBranch %1833
       %1833 = OpLabel
       %2249 = OpExtInst %void %2 DebugScope %60 %1755
       %1921 = OpExtInst %void %2 DebugLine %17 %uint_61 %uint_61 %uint_0 %uint_0
       %1834 = OpLoad %int %1762
       %1835 = OpIAdd %int %1834 %int_1
               OpStore %1762 %1835
               OpBranch %1798
       %1836 = OpLabel
       %2250 = OpExtInst %void %2 DebugScope %60 %1755
       %1925 = OpExtInst %void %2 DebugLine %17 %uint_70 %uint_70 %uint_0 %uint_0
       %1837 = OpLoad %float %1759
       %1838 = OpLoad %int %1760
       %1839 = OpConvertSToF %float %1838
       %1840 = OpFDiv %float %1837 %1839
       %2251 = OpExtInst %void %2 DebugScope %78 %1674
       %2129 = OpExtInst %void %2 DebugLine %17 %uint_80 %uint_80 %uint_0 %uint_0
       %2128 = OpExtInst %void %2 DebugValue %431 %1840 %47
       %1743 = OpExtInst %void %2 DebugLine %17 %uint_85 %uint_85 %uint_0 %uint_0
       %1706 = OpLoad %v3float %param_1
       %1707 = OpVectorTimesScalar %v3float %1706 %1840
               OpStore %param_1 %1707
               OpBranch %1708
       %1708 = OpLabel
       %2252 = OpExtInst %void %2 DebugScope %78 %1674
       %1747 = OpExtInst %void %2 DebugLine %17 %uint_74 %uint_74 %uint_0 %uint_0
       %1709 = OpLoad %int %1673
       %1710 = OpIAdd %int %1709 %int_1
               OpStore %1673 %1710
               OpBranch %1685
       %1711 = OpLabel
       %2253 = OpExtInst %void %2 DebugScope %78 %1674
       %1751 = OpExtInst %void %2 DebugLine %17 %uint_87 %uint_87 %uint_0 %uint_0
       %1712 = OpLoad %v3float %param_1
       %2254 = OpExtInst %void %2 DebugScope %838
       %1087 = OpExtInst %void %2 DebugLine %17 %uint_164 %uint_164 %uint_0 %uint_0
               OpStore %fragcolor %1712
               OpBranch %837
        %837 = OpLabel
       %2255 = OpExtInst %void %2 DebugScope %16
        %848 = OpExtInst %void %2 DebugLine %17 %uint_167 %uint_167 %uint_0 %uint_0
        %850 = OpLoad %v3float %fragcolor
        %851 = OpCompositeExtract %float %850 0
        %852 = OpCompositeExtract %float %850 1
        %853 = OpCompositeExtract %float %850 2
        %854 = OpCompositeConstruct %v4float %851 %852 %853 %float_1
               OpStore %outFragColor %854
               OpBranch %1288
       %1288 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<BlockMergePass>(text, true);
}

// TODO(greg-lunarg): Add tests to verify handling of these cases:
//
//    More complex control flow
//    Others?

}  // namespace
}  // namespace opt
}  // namespace spvtools
