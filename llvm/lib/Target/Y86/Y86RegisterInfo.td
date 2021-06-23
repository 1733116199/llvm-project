//===-- Y86RegisterInfo.td - Y86 Register defs -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the Y86 register file
//===----------------------------------------------------------------------===//

class Y86Reg<bits<4> num, string n, list<string> alt = []> : Register<n> {
  field bits<4> Num = num;
  let Namespace = "Y86";
  let HWEncoding{3-0} = num;
  let AltNames = alt;
  let DwarfNumbers = [num];
}

class Y86RegWithSubregs<bits<4> num, string n, list<Register> subregs,
                           list<string> alt = []> 
  : RegisterWithSubRegs<n, subregs> {
  field bits<4> Num = num;
  let Namespace = "Y86";
  let HWEncoding{3-0} = num;
  let AltNames = alt;
  let DwarfNumbers = [num];
}

//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//

def PCB  : Y86Reg<0,  "r0", ["pc"]>;
def SPB  : Y86Reg<1,  "r1", ["sp"]>;
def SRB  : Y86Reg<2,  "r2", ["sr"]>;
def CGB  : Y86Reg<3,  "r3", ["cg"]>;
def R4B  : Y86Reg<4,  "r4", ["fp"]>;
def R5B  : Y86Reg<5,  "r5">;
def R6B  : Y86Reg<6,  "r6">;
def R7B  : Y86Reg<7,  "r7">;
def R8B  : Y86Reg<8,  "r8">;
def R9B  : Y86Reg<9,  "r9">;
def R10B : Y86Reg<10, "r10">;
def R11B : Y86Reg<11, "r11">;
def R12B : Y86Reg<12, "r12">;
def R13B : Y86Reg<13, "r13">;
def R14B : Y86Reg<14, "r14">;
def R15B : Y86Reg<15, "r15">;

def subreg_8bit : SubRegIndex<8> { let Namespace = "Y86"; }

let SubRegIndices = [subreg_8bit] in {
def PC  : Y86RegWithSubregs<0,  "r0",  [PCB], ["pc"]>;
def SP  : Y86RegWithSubregs<1,  "r1",  [SPB], ["sp"]>;
def SR  : Y86RegWithSubregs<2,  "r2",  [SRB], ["sr"]>;
def CG  : Y86RegWithSubregs<3,  "r3",  [CGB], ["cg"]>;
def R4  : Y86RegWithSubregs<4,  "r4",  [R4B], ["fp"]>;
def R5  : Y86RegWithSubregs<5,  "r5",  [R5B]>;
def R6  : Y86RegWithSubregs<6,  "r6",  [R6B]>;
def R7  : Y86RegWithSubregs<7,  "r7",  [R7B]>;
def R8  : Y86RegWithSubregs<8,  "r8",  [R8B]>;
def R9  : Y86RegWithSubregs<9,  "r9",  [R9B]>;
def R10 : Y86RegWithSubregs<10, "r10", [R10B]>;
def R11 : Y86RegWithSubregs<11, "r11", [R11B]>;
def R12 : Y86RegWithSubregs<12, "r12", [R12B]>;
def R13 : Y86RegWithSubregs<13, "r13", [R13B]>;
def R14 : Y86RegWithSubregs<14, "r14", [R14B]>;
def R15 : Y86RegWithSubregs<15, "r15", [R15B]>;
}

def GR8 : RegisterClass<"Y86", [i8], 8,
   // Volatile registers
  (add R12B, R13B, R14B, R15B, R11B, R10B, R9B, R8B, R7B, R6B, R5B,
   // Frame pointer, sometimes allocable
   R4B,
   // Volatile, but not allocable
   PCB, SPB, SRB, CGB)>;

def GR16 : RegisterClass<"Y86", [i16], 16,
   // Volatile registers
  (add R12, R13, R14, R15, R11, R10, R9, R8, R7, R6, R5,
   // Frame pointer, sometimes allocable
   R4,
   // Volatile, but not allocable
   PC, SP, SR, CG)>;
