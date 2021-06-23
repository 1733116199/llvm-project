add_llvm_component_library(LLVMY86Info
  Y86TargetInfo.cpp

  LINK_COMPONENTS
  Support

  ADD_TO_COMPONENT
  Y86
  )
