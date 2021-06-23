//===-- Y86MCTargetDesc.cpp - Y86 Target Descriptions ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Y86 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "Y86MCTargetDesc.h"
#include "Y86InstPrinter.h"
#include "Y86MCAsmInfo.h"
#include "TargetInfo/Y86TargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "Y86GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "Y86GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "Y86GenRegisterInfo.inc"

static MCInstrInfo *createY86MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitY86MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createY86MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitY86MCRegisterInfo(X, Y86::PC);
  return X;
}

static MCSubtargetInfo *
createY86MCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createY86MCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createY86MCInstPrinter(const Triple &T,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new Y86InstPrinter(MAI, MII, MRI);
  return nullptr;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeY86TargetMC() {
  Target &T = getTheY86Target();

  RegisterMCAsmInfo<Y86MCAsmInfo> X(T);
  TargetRegistry::RegisterMCInstrInfo(T, createY86MCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(T, createY86MCRegisterInfo);
  TargetRegistry::RegisterMCSubtargetInfo(T, createY86MCSubtargetInfo);
  TargetRegistry::RegisterMCInstPrinter(T, createY86MCInstPrinter);
  TargetRegistry::RegisterMCCodeEmitter(T, createY86MCCodeEmitter);
  TargetRegistry::RegisterMCAsmBackend(T, createY86MCAsmBackend);
  TargetRegistry::RegisterObjectTargetStreamer(
      T, createY86ObjectTargetStreamer);
}
