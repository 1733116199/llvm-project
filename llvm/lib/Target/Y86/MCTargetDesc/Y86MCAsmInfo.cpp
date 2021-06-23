//===-- Y86MCAsmInfo.cpp - Y86 asm properties -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the Y86MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "Y86MCAsmInfo.h"
using namespace llvm;

void Y86MCAsmInfo::anchor() { }

Y86MCAsmInfo::Y86MCAsmInfo(const Triple &TT,
                                 const MCTargetOptions &Options) {
  CodePointerSize = CalleeSaveStackSlotSize = 2;

  CommentString = ";";
  SeparatorString = "{";

  AlignmentIsInBytes = false;
  UsesELFSectionDirectiveForBSS = true;

  SupportsDebugInformation = true;
}
