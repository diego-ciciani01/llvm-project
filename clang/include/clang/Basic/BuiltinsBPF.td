//===--- BuiltinsBPF.td - BPF Builtin function database ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "clang/Basic/BuiltinsBase.td"

// Get record field information
def PreserveFieldInfo : TargetBuiltin {
  let Spellings = ["__builtin_preserve_field_info"];
  let Attributes = [CustomTypeChecking];
  let Prototype = "unsigned int(...)";
}

// Get BTF type id
def BtfTypeID : TargetBuiltin {
  let Spellings = ["__builtin_btf_type_id"];
  let Attributes = [CustomTypeChecking];
  let Prototype = "long unsigned int(...)";
}

// Get type information
def PreserveTypeInfo : TargetBuiltin {
  let Spellings = ["__builtin_preserve_type_info"];
  let Attributes = [CustomTypeChecking];
  let Prototype = "long unsigned int(...)";
}

// Preserve enum value
def PreserveEnumValue : TargetBuiltin {
  let Spellings = ["__builtin_preserve_enum_value"];
  let Attributes = [CustomTypeChecking];
  let Prototype = "long int(...)";
}

def simdLoad : TargetBuiltin {
  let Spellings = ["__builtin_bpf_simd_load"];
  let Attributes = [NoThrow];
  let Prototype = "_Vector<16, int>(void *)";
}

def simdStore : TargetBuiltin {
  let Spellings = ["__builtin_bpf_simd_store"];
  let Attributes = [NoThrow];
  let Prototype = "void(void *, _Vector<16, int>)";
}

def simdAdd : TargetBuiltin {
  let Spellings = ["__builtin_bpf_simd_add"];
  let Attributes = [NoThrow, Const];
  let Prototype = "_Vector<16, int>(_Vector<16, int>, _Vector<16, int>)";
}

def simdXor : TargetBuiltin {
  let Spellings = ["__builtin_bpf_simd_xor"];
  let Attributes = [NoThrow, Const];
  let Prototype = "_Vector<16, int>(_Vector<16, int>, _Vector<16, int>)";
}
