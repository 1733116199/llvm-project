add_llvm_component_library(LLVMY86Disassembler
  Y86Disassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  Y86Info
  Support

  ADD_TO_COMPONENT
  Y86
  )
