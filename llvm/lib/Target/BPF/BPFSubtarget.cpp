//===-- BPFSubtarget.cpp - BPF Subtarget Information ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the BPF specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "BPFSubtarget.h"
#include "BPF.h"
#include "BPFTargetMachine.h"
#include "GISel/BPFCallLowering.h"
#include "GISel/BPFLegalizerInfo.h"
#include "GISel/BPFRegisterBankInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/ScheduleDAG.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"

using namespace llvm;

// The generic BPF CPU has no SIMD instruction latency model. This is a
// deliberately narrow, opt-in probe to test whether prioritizing long
// SIMD MUL def-use chains changes the machine schedule. Zero means off.
static cl::opt<unsigned> BPFSIMDMulDepLatency(
    "bpf-simd-mul-dep-latency", cl::Hidden, cl::init(0),
    cl::desc("Experimental latency in cycles for SIMD multiply data edges "
             "in the BPF machine scheduler (0 disables)"));

// Optional logging to prove that SIMD data dependencies are intercepted.
static cl::opt<bool> BPFSIMDMulDepTrace(
    "bpf-simd-mul-dep-trace", cl::Hidden, cl::init(false),
    cl::desc("Print each SIMD multiply dependency latency adjustment"));

#define DEBUG_TYPE "bpf-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "BPFGenSubtargetInfo.inc"

static cl::opt<bool> Disable_ldsx("disable-ldsx", cl::Hidden, cl::init(false),
  cl::desc("Disable ldsx insns"));
static cl::opt<bool> Disable_movsx("disable-movsx", cl::Hidden, cl::init(false),
  cl::desc("Disable movsx insns"));
static cl::opt<bool> Disable_bswap("disable-bswap", cl::Hidden, cl::init(false),
  cl::desc("Disable bswap insns"));
static cl::opt<bool> Disable_sdiv_smod("disable-sdiv-smod", cl::Hidden,
  cl::init(false), cl::desc("Disable sdiv/smod insns"));
static cl::opt<bool> Disable_gotol("disable-gotol", cl::Hidden, cl::init(false),
  cl::desc("Disable gotol insn"));
static cl::opt<bool>
    Disable_StoreImm("disable-storeimm", cl::Hidden, cl::init(false),
                     cl::desc("Disable BPF_ST (immediate store) insn"));
static cl::opt<bool> Disable_load_acq_store_rel(
    "disable-load-acq-store-rel", cl::Hidden, cl::init(false),
    cl::desc("Disable load-acquire and store-release insns"));
static cl::opt<bool> Disable_gotox("disable-gotox", cl::Hidden, cl::init(false),
                                   cl::desc("Disable gotox insn"));

void BPFSubtarget::adjustSchedDependency(
    SUnit *Def, int /*DefOpIdx*/, SUnit * /*Use*/, int /*UseOpIdx*/,
    SDep &Dep, const TargetSchedModel * /*SchedModel*/) const {

    if (BPFSIMDMulDepTrace) {
  errs() << "BPF_SCHED_HOOK";

  if (Def) {
    if (Def->isInstr() && Def->getInstr()) {
      errs() << " type=MachineInstr"
             << " opcode=" << Def->getInstr()->getOpcode();
    } else if (!Def->isInstr() && Def->getNode()) {
      const SDNode *N = Def->getNode();

      errs() << " type=SDNode"
             << " is_machine=" << N->isMachineOpcode();

      if (N->isMachineOpcode())
        errs() << " opcode=" << N->getMachineOpcode();
      else
        errs() << " opcode=" << N->getOpcode();
    }
  }

  errs() << " kind=" << static_cast<unsigned>(Dep.getKind())
         << " latency=" << Dep.getLatency() << '\n';
}
  // Only modify true register data dependencies of native SIMD multiplies.
  // Never touch order/memory/anti/output edges or scalar instructions.
  if (!BPFSIMDMulDepLatency || Dep.getKind() != SDep::Data || !Def )
    return;

  unsigned Opcode;

  if (Def->isInstr() && Def->getInstr()) {
      Opcode = Def->getInstr()->getOpcode();
  } else if (!Def->isInstr() && Def->getNode() &&
               Def->getNode()->isMachineOpcode()) {
      Opcode = Def->getNode()->getMachineOpcode();
  } else {
      return;
    }

  if (Opcode != BPF::SIMD_MUL3 && Opcode != BPF::SIMD_MUL)
    return;

  // This is a scheduling hint, not an inserted delay. Preserve any stronger
  // latency supplied by a future scheduling model.
  const unsigned OldLatency = Dep.getLatency();
  if (BPFSIMDMulDepLatency > OldLatency)
    Dep.setLatency(BPFSIMDMulDepLatency);

  if (BPFSIMDMulDepTrace)
    errs() << "BPF_SIMD_MUL_EDGE opcode="
           << (Opcode == BPF::SIMD_MUL3 ? "SIMD_MUL3" : "SIMD_MUL")
           << " latency=" << OldLatency << "->" << Dep.getLatency()
           << '\n';
}

void BPFSubtarget::anchor() {}

BPFSubtarget &BPFSubtarget::initializeSubtargetDependencies(StringRef CPU,
                                                            StringRef FS) {
  initializeEnvironment();
  initSubtargetFeatures(CPU, FS);
  ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);
  return *this;
}

void BPFSubtarget::initializeEnvironment() {
  HasJmpExt = false;
  HasJmp32 = false;
  HasAlu32 = false;
  UseDwarfRIS = false;
  HasLdsx = false;
  HasMovsx = false;
  HasBswap = false;
  HasSdivSmod = false;
  HasGotol = false;
  HasStoreImm = false;
  HasLoadAcqStoreRel = false;
  HasGotox = false;
  AllowsMisalignedMemAccess = false;
}

void BPFSubtarget::initSubtargetFeatures(StringRef CPU, StringRef FS) {
  if (CPU.empty())
    CPU = "v3";
  if (CPU == "probe")
    CPU = sys::detail::getHostCPUNameForBPF();
  if (CPU == "generic" || CPU == "v1")
    return;
  if (CPU == "v2") {
    HasJmpExt = true;
    return;
  }
  if (CPU == "v3") {
    HasJmpExt = true;
    HasJmp32 = true;
    HasAlu32 = true;
    return;
  }
  if (CPU == "v4") {
    HasJmpExt = true;
    HasJmp32 = true;
    HasAlu32 = true;
    HasLdsx = !Disable_ldsx;
    HasMovsx = !Disable_movsx;
    HasBswap = !Disable_bswap;
    HasSdivSmod = !Disable_sdiv_smod;
    HasGotol = !Disable_gotol;
    HasStoreImm = !Disable_StoreImm;
    HasLoadAcqStoreRel = !Disable_load_acq_store_rel;
    HasGotox = !Disable_gotox;
    return;
  }
}

BPFSubtarget::BPFSubtarget(const Triple &TT, const std::string &CPU,
                           const std::string &FS, const TargetMachine &TM)
    : BPFGenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      InstrInfo(initializeSubtargetDependencies(CPU, FS)), FrameLowering(*this),
      TLInfo(TM, *this) {
  IsLittleEndian = TT.isLittleEndian();

  CallLoweringInfo.reset(new BPFCallLowering(*getTargetLowering()));
  Legalizer.reset(new BPFLegalizerInfo(*this));
  auto *RBI = new BPFRegisterBankInfo(*getRegisterInfo());
  RegBankInfo.reset(RBI);

  InstSelector.reset(createBPFInstructionSelector(
      *static_cast<const BPFTargetMachine *>(&TM), *this, *RBI));
}

const CallLowering *BPFSubtarget::getCallLowering() const {
  return CallLoweringInfo.get();
}

InstructionSelector *BPFSubtarget::getInstructionSelector() const {
  return InstSelector.get();
}

const LegalizerInfo *BPFSubtarget::getLegalizerInfo() const {
  return Legalizer.get();
}

const RegisterBankInfo *BPFSubtarget::getRegBankInfo() const {
  return RegBankInfo.get();
}
