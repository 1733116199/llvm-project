//===-- Y86ELFObjectWriter.cpp - Y86 ELF Writer ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Y86FixupKinds.h"
#include "MCTargetDesc/Y86MCTargetDesc.h"

#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class Y86ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  Y86ELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(false, OSABI, ELF::EM_Y86,
                              /*HasRelocationAddend*/ true) {}

  ~Y86ELFObjectWriter() override {}

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override {
    // Translate fixup kind to ELF relocation type.
    switch (Fixup.getTargetKind()) {
    case FK_Data_1:                   return ELF::R_Y86_8;
    case FK_Data_2:                   return ELF::R_Y86_16_BYTE;
    case FK_Data_4:                   return ELF::R_Y86_32;
    case Y86::fixup_32:            return ELF::R_Y86_32;
    case Y86::fixup_10_pcrel:      return ELF::R_Y86_10_PCREL;
    case Y86::fixup_16:            return ELF::R_Y86_16;
    case Y86::fixup_16_pcrel:      return ELF::R_Y86_16_PCREL;
    case Y86::fixup_16_byte:       return ELF::R_Y86_16_BYTE;
    case Y86::fixup_16_pcrel_byte: return ELF::R_Y86_16_PCREL_BYTE;
    case Y86::fixup_2x_pcrel:      return ELF::R_Y86_2X_PCREL;
    case Y86::fixup_rl_pcrel:      return ELF::R_Y86_RL_PCREL;
    case Y86::fixup_8:             return ELF::R_Y86_8;
    case Y86::fixup_sym_diff:      return ELF::R_Y86_SYM_DIFF;
    default:
      llvm_unreachable("Invalid fixup kind");
    }
  }
};
} // end of anonymous namespace

std::unique_ptr<MCObjectTargetWriter>
llvm::createY86ELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<Y86ELFObjectWriter>(OSABI);
}
