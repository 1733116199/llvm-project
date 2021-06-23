//===-- Y86TargetMachine.cpp - Define TargetMachine for Y86 ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Top-level implementation for the Y86 target.
//
//===----------------------------------------------------------------------===//

#include "Y86TargetMachine.h"
#include "Y86.h"
#include "TargetInfo/Y86TargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeY86Target() {
  // Register the target.
  RegisterTargetMachine<Y86TargetMachine> X(getTheY86Target());
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::Static;
  return *RM;
}

static std::string computeDataLayout(const Triple &TT, StringRef CPU,
                                     const TargetOptions &Options) {
  return "e-m:e-p:16:16-i32:16-i64:16-f32:16-f64:16-a:8-n8:16-S16";
}

Y86TargetMachine::Y86TargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         Optional<Reloc::Model> RM,
                                         Optional<CodeModel::Model> CM,
                                         CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(TT, CPU, Options), TT, CPU, FS,
                        Options, getEffectiveRelocModel(RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

Y86TargetMachine::~Y86TargetMachine() {}

namespace {
/// Y86 Code Generator Pass Configuration Options.
class Y86PassConfig : public TargetPassConfig {
public:
  Y86PassConfig(Y86TargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  Y86TargetMachine &getY86TargetMachine() const {
    return getTM<Y86TargetMachine>();
  }

  bool addInstSelector() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *Y86TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new Y86PassConfig(*this, PM);
}

bool Y86PassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createY86ISelDag(getY86TargetMachine(), getOptLevel()));
  return false;
}

void Y86PassConfig::addPreEmitPass() {
  // Must run branch selection immediately preceding the asm printer.
  addPass(createY86BranchSelectionPass(), false);
}
