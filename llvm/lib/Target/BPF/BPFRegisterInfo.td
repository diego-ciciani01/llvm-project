//===-- BPFRegisterInfo.td - BPF Register defs -------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the BPF register file
//===----------------------------------------------------------------------===//

let Namespace = "BPF" in {
  def sub_32 : SubRegIndex<32>;
}

class Wi<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "BPF";
}

// Registers are identified with 4-bit ID numbers.
// Ri - 64-bit integer registers
class Ri<bits<16> Enc, string n, list<Register> subregs>
  : RegisterWithSubRegs<n, subregs> {
  let HWEncoding = Enc;
  let Namespace = "BPF";
  let SubRegIndices = [sub_32];
}

foreach I = 0-11 in {
  // 32-bit Integer (alias to low part of 64-bit register).
  def W#I  : Wi<I,  "w"#I>,  DwarfRegNum<[I]>;
  // 64-bit Integer registers
  def R#I  : Ri<I,  "r"#I,  [!cast<Wi>("W"#I)]>,  DwarfRegNum<[I]>;
}


// Register classes.
def GPR32 : RegisterClass<"BPF", [i32], 64, (add
  (sequence "W%u", 1, 9),
  W0, // Return value
  W11, // Stack Ptr
  W10  // Frame Ptr
)>;

// Vector registers - AVX 512
def ZMM0  : BPFReg<"zmm0",  [], []>;
def ZMM1  : BPFReg<"zmm1",  [], []>;
def ZMM2  : BPFReg<"zmm2",  [], []>;
def ZMM3  : BPFReg<"zmm3",  [], []>;
def ZMM4  : BPFReg<"zmm4",  [], []>;
def ZMM5  : BPFReg<"zmm5",  [], []>;
def ZMM6  : BPFReg<"zmm6",  [], []>;
def ZMM7  : BPFReg<"zmm7",  [], []>;
def ZMM8  : BPFReg<"zmm8",  [], []>;
def ZMM9  : BPFReg<"zmm9",  [], []>;
def ZMM10 : BPFReg<"zmm10", [], []>;
def ZMM11 : BPFReg<"zmm11", [], []>;
def ZMM12 : BPFReg<"zmm12", [], []>;
def ZMM13 : BPFReg<"zmm13", [], []>;
def ZMM14 : BPFReg<"zmm14", [], []>;
def ZMM15 : BPFReg<"zmm15", [], []>;

def ZMMRC : RegisterClass<"BPF", [v16i32], 512, (
    add ZMM0, ZMM1, ZMM2, ZMM3, ZMM4, ZMM5, ZMM6, ZMM7, ZMM8, ZMM9,
    ZMM10, ZMM11, ZMMM12, ZMM13, ZMM14, ZMMM15)>;
)

def ZMM0Encoding  : DwarfRegNum<[0]>;
def ZMM1Encoding  : DwarfRegNum<[1]>;
def ZMM2Encoding  : DwarfRegNum<[2]>;
def ZMM3Encoding  : DwarfRegNum<[3]>;
def ZMM4Encoding  : DwarfRegNum<[4]>;
def ZMM5Encoding  : DwarfRegNum<[5]>;
def ZMM6Encoding  : DwarfRegNum<[6]>;
def ZMM7Encoding  : DwarfRegNum<[7]>;
def ZMM8Encoding  : DwarfRegNum<[8]>;
def ZMM9Encoding  : DwarfRegNum<[9]>;
def ZMM10Encoding : DwarfRegNum<[10]>;
def ZMM11Encoding : DwarfRegNum<[11]>;
def ZMM12Encoding : DwarfRegNum<[12]>;
def ZMM13Encoding : DwarfRegNum<[13]>;
def ZMM14Encoding : DwarfRegNum<[14]>;
def ZMM15Encoding : DwarfRegNum<[15]>;

def GPR : RegisterClass<"BPF", [i64], 64, (add
  (sequence "R%u", 1, 9),
  R0, // Return value
  R11, // Stack Ptr
  R10  // Frame Ptr
)>;
