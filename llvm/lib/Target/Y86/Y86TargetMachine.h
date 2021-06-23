//===-- Y86TargetMachine.h - Define TargetMachine for Y86 -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Y86 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_LIB_TARGET_Y86_Y86TARGETMACHINE_H
#define LLVM_LIB_TARGET_Y86_Y86TARGETMACHINE_H

#include "Y86Subtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class StringRef;

/// Y86TargetMachine
///
class Y86TargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  Y86Subtarget        Subtarget;

public:
  Y86TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                      CodeGenOpt::Level OL, bool JIT);
  ~Y86TargetMachine() override;

  const Y86Subtarget *getSubtargetImpl(const Function &F) const override {
    return &Subtarget;
  }
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
}; // Y86TargetMachine.

} // end namespace llvm

#endif
