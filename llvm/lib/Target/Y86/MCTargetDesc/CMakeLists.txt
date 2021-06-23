add_llvm_component_library(LLVMY86Desc
  Y86AsmBackend.cpp
  Y86ELFObjectWriter.cpp
  Y86ELFStreamer.cpp
  Y86InstPrinter.cpp
  Y86MCAsmInfo.cpp
  Y86MCCodeEmitter.cpp
  Y86MCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  Y86Info
  Support

  ADD_TO_COMPONENT
  Y86
  )
