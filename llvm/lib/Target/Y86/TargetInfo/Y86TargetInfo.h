//===-- Y86TargetInfo.h - Y86 Target Implementation -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_TARGETINFO_Y86TARGETINFO_H
#define LLVM_LIB_TARGET_Y86_TARGETINFO_Y86TARGETINFO_H

namespace llvm {

class Target;

Target &getTheY86Target();

} // namespace llvm

#endif // LLVM_LIB_TARGET_Y86_TARGETINFO_Y86TARGETINFO_H
