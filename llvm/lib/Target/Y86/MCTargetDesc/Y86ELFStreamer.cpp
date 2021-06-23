//===-- Y86ELFStreamer.cpp - Y86 ELF Target Streamer Methods --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Y86 specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "Y86MCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"

using namespace llvm;

namespace llvm {

class Y86TargetELFStreamer : public MCTargetStreamer {
public:
  MCELFStreamer &getStreamer();
  Y86TargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);
};

// This part is for ELF object output.
Y86TargetELFStreamer::Y86TargetELFStreamer(MCStreamer &S,
                                                 const MCSubtargetInfo &STI)
    : MCTargetStreamer(S) {
  MCAssembler &MCA = getStreamer().getAssembler();
  unsigned EFlags = MCA.getELFHeaderEFlags();
  MCA.setELFHeaderEFlags(EFlags);

  // Emit build attributes section according to
  // Y86 EABI (slaa534.pdf, part 13).
  MCSection *AttributeSection = getStreamer().getContext().getELFSection(
      ".Y86.attributes", ELF::SHT_Y86_ATTRIBUTES, 0);
  Streamer.SwitchSection(AttributeSection);

  // Format version.
  Streamer.emitInt8(0x41);
  // Subsection length.
  Streamer.emitInt32(22);
  // Vendor name string, zero-terminated.
  Streamer.emitBytes("mspabi");
  Streamer.emitInt8(0);

  // Attribute vector scope tag. 1 stands for the entire file.
  Streamer.emitInt8(1);
  // Attribute vector length.
  Streamer.emitInt32(11);
  // OFBA_MSPABI_Tag_ISA(4) = 1, Y86
  Streamer.emitInt8(4);
  Streamer.emitInt8(1);
  // OFBA_MSPABI_Tag_Code_Model(6) = 1, Small
  Streamer.emitInt8(6);
  Streamer.emitInt8(1);
  // OFBA_MSPABI_Tag_Data_Model(8) = 1, Small
  Streamer.emitInt8(8);
  Streamer.emitInt8(1);
}

MCELFStreamer &Y86TargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}

MCTargetStreamer *
createY86ObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();
  if (TT.isOSBinFormatELF())
    return new Y86TargetELFStreamer(S, STI);
  return nullptr;
}

} // namespace llvm
