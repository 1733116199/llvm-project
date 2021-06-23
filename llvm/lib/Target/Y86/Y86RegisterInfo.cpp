//===-- Y86RegisterInfo.cpp - Y86 Register Information --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Y86 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "Y86RegisterInfo.h"
#include "Y86.h"
#include "Y86MachineFunctionInfo.h"
#include "Y86TargetMachine.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

#define DEBUG_TYPE "y86-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "Y86GenRegisterInfo.inc"

// FIXME: Provide proper call frame setup / destroy opcodes.
Y86RegisterInfo::Y86RegisterInfo()
  : Y86GenRegisterInfo(Y86::PC) {}

const MCPhysReg*
Y86RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  const Y86FrameLowering *TFI = getFrameLowering(*MF);
  const Function* F = &MF->getFunction();
  static const MCPhysReg CalleeSavedRegs[] = {
    Y86::R4, Y86::R5, Y86::R6, Y86::R7,
    Y86::R8, Y86::R9, Y86::R10,
    0
  };
  static const MCPhysReg CalleeSavedRegsFP[] = {
    Y86::R5, Y86::R6, Y86::R7,
    Y86::R8, Y86::R9, Y86::R10,
    0
  };
  static const MCPhysReg CalleeSavedRegsIntr[] = {
    Y86::R4,  Y86::R5,  Y86::R6,  Y86::R7,
    Y86::R8,  Y86::R9,  Y86::R10, Y86::R11,
    Y86::R12, Y86::R13, Y86::R14, Y86::R15,
    0
  };
  static const MCPhysReg CalleeSavedRegsIntrFP[] = {
    Y86::R5,  Y86::R6,  Y86::R7,
    Y86::R8,  Y86::R9,  Y86::R10, Y86::R11,
    Y86::R12, Y86::R13, Y86::R14, Y86::R15,
    0
  };

  if (TFI->hasFP(*MF))
    return (F->getCallingConv() == CallingConv::Y86_INTR ?
            CalleeSavedRegsIntrFP : CalleeSavedRegsFP);
  else
    return (F->getCallingConv() == CallingConv::Y86_INTR ?
            CalleeSavedRegsIntr : CalleeSavedRegs);

}

BitVector Y86RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const Y86FrameLowering *TFI = getFrameLowering(MF);

  // Mark 4 special registers with subregisters as reserved.
  Reserved.set(Y86::PCB);
  Reserved.set(Y86::SPB);
  Reserved.set(Y86::SRB);
  Reserved.set(Y86::CGB);
  Reserved.set(Y86::PC);
  Reserved.set(Y86::SP);
  Reserved.set(Y86::SR);
  Reserved.set(Y86::CG);

  // Mark frame pointer as reserved if needed.
  if (TFI->hasFP(MF)) {
    Reserved.set(Y86::R4B);
    Reserved.set(Y86::R4);
  }

  return Reserved;
}

const TargetRegisterClass *
Y86RegisterInfo::getPointerRegClass(const MachineFunction &MF, unsigned Kind)
                                                                         const {
  return &Y86::GR16RegClass;
}

void
Y86RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                        int SPAdj, unsigned FIOperandNum,
                                        RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const Y86FrameLowering *TFI = getFrameLowering(MF);
  DebugLoc dl = MI.getDebugLoc();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();

  unsigned BasePtr = (TFI->hasFP(MF) ? Y86::R4 : Y86::SP);
  int Offset = MF.getFrameInfo().getObjectOffset(FrameIndex);

  // Skip the saved PC
  Offset += 2;

  if (!TFI->hasFP(MF))
    Offset += MF.getFrameInfo().getStackSize();
  else
    Offset += 2; // Skip the saved FP

  // Fold imm into offset
  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  if (MI.getOpcode() == Y86::ADDframe) {
    // This is actually "load effective address" of the stack slot
    // instruction. We have only two-address instructions, thus we need to
    // expand it into mov + add
    const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

    MI.setDesc(TII.get(Y86::MOV16rr));
    MI.getOperand(FIOperandNum).ChangeToRegister(BasePtr, false);

    if (Offset == 0)
      return;

    // We need to materialize the offset via add instruction.
    Register DstReg = MI.getOperand(0).getReg();
    if (Offset < 0)
      BuildMI(MBB, std::next(II), dl, TII.get(Y86::SUB16ri), DstReg)
        .addReg(DstReg).addImm(-Offset);
    else
      BuildMI(MBB, std::next(II), dl, TII.get(Y86::ADD16ri), DstReg)
        .addReg(DstReg).addImm(Offset);

    return;
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(BasePtr, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
}

Register Y86RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const Y86FrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? Y86::R4 : Y86::SP;
}
