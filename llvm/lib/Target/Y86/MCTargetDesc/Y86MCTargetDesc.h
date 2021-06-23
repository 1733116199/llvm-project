//===-- Y86MCTargetDesc.h - Y86 Target Descriptions -------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86MCTARGETDESC_H
#define LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86MCTARGETDESC_H

#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {
class Target;
class MCAsmBackend;
class MCCodeEmitter;
class MCInstrInfo;
class MCSubtargetInfo;
class MCRegisterInfo;
class MCContext;
class MCTargetOptions;
class MCObjectTargetWriter;
class MCStreamer;
class MCTargetStreamer;

/// Creates a machine code emitter for Y86.
MCCodeEmitter *createY86MCCodeEmitter(const MCInstrInfo &MCII,
                                         const MCRegisterInfo &MRI,
                                         MCContext &Ctx);

MCAsmBackend *createY86MCAsmBackend(const Target &T,
                                       const MCSubtargetInfo &STI,
                                       const MCRegisterInfo &MRI,
                                       const MCTargetOptions &Options);

MCTargetStreamer *
createY86ObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

std::unique_ptr<MCObjectTargetWriter>
createY86ELFObjectWriter(uint8_t OSABI);

} // End llvm namespace

// Defines symbolic names for Y86 registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "Y86GenRegisterInfo.inc"

// Defines symbolic names for the Y86 instructions.
#define GET_INSTRINFO_ENUM
#include "Y86GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "Y86GenSubtargetInfo.inc"

#endif
