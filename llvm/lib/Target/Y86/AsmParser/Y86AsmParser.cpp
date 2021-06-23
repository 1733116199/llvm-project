//===- Y86AsmParser.cpp - Parse Y86 assembly to MCInst instructions -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Y86.h"
#include "Y86RegisterInfo.h"
#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "TargetInfo/Y86TargetInfo.h"

#include "llvm/ADT/APInt.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"

#define DEBUG_TYPE "y86-asm-parser"

using namespace llvm;

namespace {

/// Parses Y86 assembly from a stream.
class Y86AsmParser : public MCTargetAsmParser {
  const MCSubtargetInfo &STI;
  MCAsmParser &Parser;
  const MCRegisterInfo *MRI;

  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) override;
  OperandMatchResultTy tryParseRegister(unsigned &RegNo, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool ParseDirective(AsmToken DirectiveID) override;
  bool ParseDirectiveRefSym(AsmToken DirectiveID);

  unsigned validateTargetOperandClass(MCParsedAsmOperand &Op,
                                      unsigned Kind) override;

  bool parseJccInstruction(ParseInstructionInfo &Info, StringRef Name,
                           SMLoc NameLoc, OperandVector &Operands);

  bool ParseOperand(OperandVector &Operands);

  bool ParseLiteralValues(unsigned Size, SMLoc L);

  MCAsmParser &getParser() const { return Parser; }
  MCAsmLexer &getLexer() const { return Parser.getLexer(); }

  /// @name Auto-generated Matcher Functions
  /// {

#define GET_ASSEMBLER_HEADER
#include "Y86GenAsmMatcher.inc"

  /// }

public:
  Y86AsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                  const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, STI, MII), STI(STI), Parser(Parser) {
    MCAsmParserExtension::Initialize(Parser);
    MRI = getContext().getRegisterInfo();

    setAvailableFeatures(ComputeAvailableFeatures(STI.getFeatureBits()));
  }
};

/// A parsed Y86 assembly operand.
class Y86Operand : public MCParsedAsmOperand {
  typedef MCParsedAsmOperand Base;

  enum KindTy {
    k_Imm,
    k_Reg,
    k_Tok,
    k_Mem,
    k_IndReg,
    k_PostIndReg
  } Kind;

  struct Memory {
    unsigned Reg;
    const MCExpr *Offset;
  };
  union {
    const MCExpr *Imm;
    unsigned      Reg;
    StringRef     Tok;
    Memory        Mem;
  };

  SMLoc Start, End;

public:
  Y86Operand(StringRef Tok, SMLoc const &S)
      : Base(), Kind(k_Tok), Tok(Tok), Start(S), End(S) {}
  Y86Operand(KindTy Kind, unsigned Reg, SMLoc const &S, SMLoc const &E)
      : Base(), Kind(Kind), Reg(Reg), Start(S), End(E) {}
  Y86Operand(MCExpr const *Imm, SMLoc const &S, SMLoc const &E)
      : Base(), Kind(k_Imm), Imm(Imm), Start(S), End(E) {}
  Y86Operand(unsigned Reg, MCExpr const *Expr, SMLoc const &S, SMLoc const &E)
      : Base(), Kind(k_Mem), Mem({Reg, Expr}), Start(S), End(E) {}

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert((Kind == k_Reg || Kind == k_IndReg || Kind == k_PostIndReg) &&
        "Unexpected operand kind");
    assert(N == 1 && "Invalid number of operands!");

    Inst.addOperand(MCOperand::createReg(Reg));
  }

  void addExprOperand(MCInst &Inst, const MCExpr *Expr) const {
    // Add as immediate when possible
    if (!Expr)
      Inst.addOperand(MCOperand::createImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(Kind == k_Imm && "Unexpected operand kind");
    assert(N == 1 && "Invalid number of operands!");

    addExprOperand(Inst, Imm);
  }

  void addMemOperands(MCInst &Inst, unsigned N) const {
    assert(Kind == k_Mem && "Unexpected operand kind");
    assert(N == 2 && "Invalid number of operands");

    Inst.addOperand(MCOperand::createReg(Mem.Reg));
    addExprOperand(Inst, Mem.Offset);
  }

  bool isReg()   const override { return Kind == k_Reg; }
  bool isImm()   const override { return Kind == k_Imm; }
  bool isToken() const override { return Kind == k_Tok; }
  bool isMem()   const override { return Kind == k_Mem; }
  bool isIndReg()         const { return Kind == k_IndReg; }
  bool isPostIndReg()     const { return Kind == k_PostIndReg; }

  bool isCGImm() const {
    if (Kind != k_Imm)
      return false;

    int64_t Val;
    if (!Imm->evaluateAsAbsolute(Val))
      return false;
    
    if (Val == 0 || Val == 1 || Val == 2 || Val == 4 || Val == 8 || Val == -1)
      return true;

    return false;
  }

  StringRef getToken() const {
    assert(Kind == k_Tok && "Invalid access!");
    return Tok;
  }

  unsigned getReg() const override {
    assert(Kind == k_Reg && "Invalid access!");
    return Reg;
  }

  void setReg(unsigned RegNo) {
    assert(Kind == k_Reg && "Invalid access!");
    Reg = RegNo;
  }

  static std::unique_ptr<Y86Operand> CreateToken(StringRef Str, SMLoc S) {
    return std::make_unique<Y86Operand>(Str, S);
  }

  static std::unique_ptr<Y86Operand> CreateReg(unsigned RegNum, SMLoc S,
                                                  SMLoc E) {
    return std::make_unique<Y86Operand>(k_Reg, RegNum, S, E);
  }

  static std::unique_ptr<Y86Operand> CreateImm(const MCExpr *Val, SMLoc S,
                                                  SMLoc E) {
    return std::make_unique<Y86Operand>(Val, S, E);
  }

  static std::unique_ptr<Y86Operand> CreateMem(unsigned RegNum,
                                                  const MCExpr *Val,
                                                  SMLoc S, SMLoc E) {
    return std::make_unique<Y86Operand>(RegNum, Val, S, E);
  }

  static std::unique_ptr<Y86Operand> CreateIndReg(unsigned RegNum, SMLoc S,
                                                  SMLoc E) {
    return std::make_unique<Y86Operand>(k_IndReg, RegNum, S, E);
  }

  static std::unique_ptr<Y86Operand> CreatePostIndReg(unsigned RegNum, SMLoc S,
                                                  SMLoc E) {
    return std::make_unique<Y86Operand>(k_PostIndReg, RegNum, S, E);
  }

  SMLoc getStartLoc() const override { return Start; }
  SMLoc getEndLoc() const override { return End; }

  void print(raw_ostream &O) const override {
    switch (Kind) {
    case k_Tok:
      O << "Token " << Tok;
      break;
    case k_Reg:
      O << "Register " << Reg;
      break;
    case k_Imm:
      O << "Immediate " << *Imm;
      break;
    case k_Mem:
      O << "Memory ";
      O << *Mem.Offset << "(" << Reg << ")";
      break;
    case k_IndReg:
      O << "RegInd " << Reg;
      break;
    case k_PostIndReg:
      O << "PostInc " << Reg;
      break;
    }
  }
};
} // end anonymous namespace

bool Y86AsmParser::MatchAndEmitInstruction(SMLoc Loc, unsigned &Opcode,
                                              OperandVector &Operands,
                                              MCStreamer &Out,
                                              uint64_t &ErrorInfo,
                                              bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned MatchResult =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);

  switch (MatchResult) {
  case Match_Success:
    Inst.setLoc(Loc);
    Out.emitInstruction(Inst, STI);
    return false;
  case Match_MnemonicFail:
    return Error(Loc, "invalid instruction mnemonic");
  case Match_InvalidOperand: {
    SMLoc ErrorLoc = Loc;
    if (ErrorInfo != ~0U) {
      if (ErrorInfo >= Operands.size())
        return Error(ErrorLoc, "too few operands for instruction");

      ErrorLoc = ((Y86Operand &)*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = Loc;
    }
    return Error(ErrorLoc, "invalid operand for instruction");
  }
  default:
    return true;
  }
}

// Auto-generated by TableGen
static unsigned MatchRegisterName(StringRef Name);
static unsigned MatchRegisterAltName(StringRef Name);

bool Y86AsmParser::ParseRegister(unsigned &RegNo, SMLoc &StartLoc,
                                    SMLoc &EndLoc) {
  switch (tryParseRegister(RegNo, StartLoc, EndLoc)) {
  case MatchOperand_ParseFail:
    return Error(StartLoc, "invalid register name");
  case MatchOperand_Success:
    return false;
  case MatchOperand_NoMatch:
    return true;
  }

  llvm_unreachable("unknown match result type");
}

OperandMatchResultTy Y86AsmParser::tryParseRegister(unsigned &RegNo,
                                                       SMLoc &StartLoc,
                                                       SMLoc &EndLoc) {
  if (getLexer().getKind() == AsmToken::Identifier) {
    auto Name = getLexer().getTok().getIdentifier().lower();
    RegNo = MatchRegisterName(Name);
    if (RegNo == Y86::NoRegister) {
      RegNo = MatchRegisterAltName(Name);
      if (RegNo == Y86::NoRegister)
        return MatchOperand_NoMatch;
    }

    AsmToken const &T = getParser().getTok();
    StartLoc = T.getLoc();
    EndLoc = T.getEndLoc();
    getLexer().Lex(); // eat register token

    return MatchOperand_Success;
  }

  return MatchOperand_ParseFail;
}

bool Y86AsmParser::parseJccInstruction(ParseInstructionInfo &Info,
                                          StringRef Name, SMLoc NameLoc,
                                          OperandVector &Operands) {
  if (!Name.startswith_lower("j"))
    return true;

  auto CC = Name.drop_front().lower();
  unsigned CondCode;
  if (CC == "ne" || CC == "nz")
    CondCode = Y86CC::COND_NE;
  else if (CC == "eq" || CC == "z")
    CondCode = Y86CC::COND_E;
  else if (CC == "lo" || CC == "nc")
    CondCode = Y86CC::COND_LO;
  else if (CC == "hs" || CC == "c")
    CondCode = Y86CC::COND_HS;
  else if (CC == "n")
    CondCode = Y86CC::COND_N;
  else if (CC == "ge")
    CondCode = Y86CC::COND_GE;
  else if (CC == "l")
    CondCode = Y86CC::COND_L;
  else if (CC == "mp")
    CondCode = Y86CC::COND_NONE;
  else
    return Error(NameLoc, "unknown instruction");

  if (CondCode == (unsigned)Y86CC::COND_NONE)
    Operands.push_back(Y86Operand::CreateToken("jmp", NameLoc));
  else {
    Operands.push_back(Y86Operand::CreateToken("j", NameLoc));
    const MCExpr *CCode = MCConstantExpr::create(CondCode, getContext());
    Operands.push_back(Y86Operand::CreateImm(CCode, SMLoc(), SMLoc()));
  }

  // Skip optional '$' sign.
  if (getLexer().getKind() == AsmToken::Dollar)
    getLexer().Lex(); // Eat '$'

  const MCExpr *Val;
  SMLoc ExprLoc = getLexer().getLoc();
  if (getParser().parseExpression(Val))
    return Error(ExprLoc, "expected expression operand");

  int64_t Res;
  if (Val->evaluateAsAbsolute(Res))
    if (Res < -512 || Res > 511)
      return Error(ExprLoc, "invalid jump offset");

  Operands.push_back(Y86Operand::CreateImm(Val, ExprLoc,
    getLexer().getLoc()));

  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLexer().getLoc();
    getParser().eatToEndOfStatement();
    return Error(Loc, "unexpected token");
  }

  getParser().Lex(); // Consume the EndOfStatement.
  return false;
}

bool Y86AsmParser::ParseInstruction(ParseInstructionInfo &Info,
                                       StringRef Name, SMLoc NameLoc,
                                       OperandVector &Operands) {
  // Drop .w suffix
  if (Name.endswith_lower(".w"))
    Name = Name.drop_back(2);

  if (!parseJccInstruction(Info, Name, NameLoc, Operands))
    return false;

  // First operand is instruction mnemonic
  Operands.push_back(Y86Operand::CreateToken(Name, NameLoc));

  // If there are no more operands, then finish
  if (getLexer().is(AsmToken::EndOfStatement))
    return false;

  // Parse first operand
  if (ParseOperand(Operands))
    return true;

  // Parse second operand if any
  if (getLexer().is(AsmToken::Comma)) {
    getLexer().Lex(); // Eat ','
    if (ParseOperand(Operands))
      return true;
  }

  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLexer().getLoc();
    getParser().eatToEndOfStatement();
    return Error(Loc, "unexpected token");
  }

  getParser().Lex(); // Consume the EndOfStatement.
  return false;
}

bool Y86AsmParser::ParseDirectiveRefSym(AsmToken DirectiveID) {
    StringRef Name;
    if (getParser().parseIdentifier(Name))
      return TokError("expected identifier in directive");

    MCSymbol *Sym = getContext().getOrCreateSymbol(Name);
    getStreamer().emitSymbolAttribute(Sym, MCSA_Global);
    return false;
}

bool Y86AsmParser::ParseDirective(AsmToken DirectiveID) {
  StringRef IDVal = DirectiveID.getIdentifier();
  if (IDVal.lower() == ".long") {
    ParseLiteralValues(4, DirectiveID.getLoc());
  } else if (IDVal.lower() == ".word" || IDVal.lower() == ".short") {
    ParseLiteralValues(2, DirectiveID.getLoc());
  } else if (IDVal.lower() == ".byte") {
    ParseLiteralValues(1, DirectiveID.getLoc());
  } else if (IDVal.lower() == ".refsym") {
    return ParseDirectiveRefSym(DirectiveID);
  }
  return true;
}

bool Y86AsmParser::ParseOperand(OperandVector &Operands) {
  switch (getLexer().getKind()) {
    default: return true;
    case AsmToken::Identifier: {
      // try rN
      unsigned RegNo;
      SMLoc StartLoc, EndLoc;
      if (!ParseRegister(RegNo, StartLoc, EndLoc)) {
        Operands.push_back(Y86Operand::CreateReg(RegNo, StartLoc, EndLoc));
        return false;
      }
      LLVM_FALLTHROUGH;
    }
    case AsmToken::Integer:
    case AsmToken::Plus:
    case AsmToken::Minus: {
      SMLoc StartLoc = getParser().getTok().getLoc();
      const MCExpr *Val;
      // Try constexpr[(rN)]
      if (!getParser().parseExpression(Val)) {
        unsigned RegNo = Y86::PC;
        SMLoc EndLoc = getParser().getTok().getLoc();
        // Try (rN)
        if (getLexer().getKind() == AsmToken::LParen) {
          getLexer().Lex(); // Eat '('
          SMLoc RegStartLoc;
          if (ParseRegister(RegNo, RegStartLoc, EndLoc))
            return true;
          if (getLexer().getKind() != AsmToken::RParen)
            return true;
          EndLoc = getParser().getTok().getEndLoc();
          getLexer().Lex(); // Eat ')'
        }
        Operands.push_back(Y86Operand::CreateMem(RegNo, Val, StartLoc,
          EndLoc));
        return false;
      }
      return true;
    }
    case AsmToken::Amp: {
      // Try &constexpr
      SMLoc StartLoc = getParser().getTok().getLoc();
      getLexer().Lex(); // Eat '&'
      const MCExpr *Val;
      if (!getParser().parseExpression(Val)) {
        SMLoc EndLoc = getParser().getTok().getLoc();
        Operands.push_back(Y86Operand::CreateMem(Y86::SR, Val, StartLoc,
          EndLoc));
        return false;
      }
      return true;
    }
    case AsmToken::At: {
      // Try @rN[+]
      SMLoc StartLoc = getParser().getTok().getLoc();
      getLexer().Lex(); // Eat '@'
      unsigned RegNo;
      SMLoc RegStartLoc, EndLoc;
      if (ParseRegister(RegNo, RegStartLoc, EndLoc))
        return true;
      if (getLexer().getKind() == AsmToken::Plus) {
        Operands.push_back(Y86Operand::CreatePostIndReg(RegNo, StartLoc, EndLoc));
        getLexer().Lex(); // Eat '+'
        return false;
      }
      if (Operands.size() > 1) // Emulate @rd in destination position as 0(rd)
        Operands.push_back(Y86Operand::CreateMem(RegNo,
            MCConstantExpr::create(0, getContext()), StartLoc, EndLoc));
      else
        Operands.push_back(Y86Operand::CreateIndReg(RegNo, StartLoc, EndLoc));
      return false;
    }
    case AsmToken::Hash:
      // Try #constexpr
      SMLoc StartLoc = getParser().getTok().getLoc();
      getLexer().Lex(); // Eat '#'
      const MCExpr *Val;
      if (!getParser().parseExpression(Val)) {
        SMLoc EndLoc = getParser().getTok().getLoc();
        Operands.push_back(Y86Operand::CreateImm(Val, StartLoc, EndLoc));
        return false;
      }
      return true;
  }
}

bool Y86AsmParser::ParseLiteralValues(unsigned Size, SMLoc L) {
  auto parseOne = [&]() -> bool {
    const MCExpr *Value;
    if (getParser().parseExpression(Value))
      return true;
    getParser().getStreamer().emitValue(Value, Size, L);
    return false;
  };
  return (parseMany(parseOne));
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeY86AsmParser() {
  RegisterMCAsmParser<Y86AsmParser> X(getTheY86Target());
}

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "Y86GenAsmMatcher.inc"

static unsigned convertGR16ToGR8(unsigned Reg) {
  switch (Reg) {
  default:
    llvm_unreachable("Unknown GR16 register");
  case Y86::PC:  return Y86::PCB;
  case Y86::SP:  return Y86::SPB;
  case Y86::SR:  return Y86::SRB;
  case Y86::CG:  return Y86::CGB;
  case Y86::R4:  return Y86::R4B;
  case Y86::R5:  return Y86::R5B;
  case Y86::R6:  return Y86::R6B;
  case Y86::R7:  return Y86::R7B;
  case Y86::R8:  return Y86::R8B;
  case Y86::R9:  return Y86::R9B;
  case Y86::R10: return Y86::R10B;
  case Y86::R11: return Y86::R11B;
  case Y86::R12: return Y86::R12B;
  case Y86::R13: return Y86::R13B;
  case Y86::R14: return Y86::R14B;
  case Y86::R15: return Y86::R15B;
  }
}

unsigned Y86AsmParser::validateTargetOperandClass(MCParsedAsmOperand &AsmOp,
                                                     unsigned Kind) {
  Y86Operand &Op = static_cast<Y86Operand &>(AsmOp);

  if (!Op.isReg())
    return Match_InvalidOperand;

  unsigned Reg = Op.getReg();
  bool isGR16 =
      Y86MCRegisterClasses[Y86::GR16RegClassID].contains(Reg);

  if (isGR16 && (Kind == MCK_GR8)) {
    Op.setReg(convertGR16ToGR8(Reg));
    return Match_Success;
  }

  return Match_InvalidOperand;
}
