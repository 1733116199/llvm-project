//===-- Y86TargetInfo.cpp - Y86 Target Implementation ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/Y86TargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheY86Target() {
  static Target TheY86Target;
  return TheY86Target;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeY86TargetInfo() {
  RegisterTarget<Triple::y86> X(getTheY86Target(), "y86",
                                   "Y86 [experimental]", "Y86");
}
