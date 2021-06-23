add_llvm_component_library(LLVMY86AsmParser
  Y86AsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  Y86Desc
  Y86Info
  Support

  ADD_TO_COMPONENT
  Y86
)
