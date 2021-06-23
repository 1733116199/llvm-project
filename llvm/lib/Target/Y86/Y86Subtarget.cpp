//===-- Y86Subtarget.cpp - Y86 Subtarget Information ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Y86 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "Y86Subtarget.h"
#include "Y86.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "y86-subtarget"

static cl::opt<Y86Subtarget::HWMultEnum>
HWMultModeOption("mhwmult", cl::Hidden,
           cl::desc("Hardware multiplier use mode for Y86"),
           cl::init(Y86Subtarget::NoHWMult),
           cl::values(
             clEnumValN(Y86Subtarget::NoHWMult, "none",
                "Do not use hardware multiplier"),
             clEnumValN(Y86Subtarget::HWMult16, "16bit",
                "Use 16-bit hardware multiplier"),
             clEnumValN(Y86Subtarget::HWMult32, "32bit",
                "Use 32-bit hardware multiplier"),
             clEnumValN(Y86Subtarget::HWMultF5, "f5series",
                "Use F5 series hardware multiplier")));

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "Y86GenSubtargetInfo.inc"

void Y86Subtarget::anchor() { }

Y86Subtarget &
Y86Subtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS) {
  ExtendedInsts = false;
  HWMultMode = NoHWMult;

  StringRef CPUName = CPU;
  if (CPUName.empty())
    CPUName = "y86";

  ParseSubtargetFeatures(CPUName, /*TuneCPU*/ CPUName, FS);

  if (HWMultModeOption != NoHWMult)
    HWMultMode = HWMultModeOption;

  return *this;
}

Y86Subtarget::Y86Subtarget(const Triple &TT, const std::string &CPU,
                                 const std::string &FS, const TargetMachine &TM)
    : Y86GenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS), FrameLowering(),
      InstrInfo(initializeSubtargetDependencies(CPU, FS)), TLInfo(TM, *this) {}
