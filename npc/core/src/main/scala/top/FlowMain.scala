package flow

import scala.reflect.runtime.universe._
import chisel3._
import chisel3.util.experimental.decode.{decoder, TruthTable}
import chisel3.util._
import chisel3.experimental.Trace._
import shapeless.{HNil, ::}
import shapeless.HList
import shapeless.ops.coproduct.Prepend
import chisel3.util.{ BinaryMemoryFile, HexMemoryFile }

import chisel3.experimental.Trace
import scala.collection.IndexedSeqView
import shapeless.Poly1
import flow.components.RamControlInterface

object RV32Inst {
  private val bp = BitPat

  val lui    = this.bp("b???????_?????_?????_???_?????_01101_11")
  val auipc  = this.bp("b???????_?????_?????_???_?????_00101_11")

  val jal    = this.bp("b???????_?????_?????_???_?????_11011_11")
  val jalr   = this.bp("b???????_?????_?????_???_?????_11001_11")
  val beq    = this.bp("b???????_?????_?????_000_?????_11000_11")
  val bne    = this.bp("b???????_?????_?????_001_?????_11000_11")
  val blt    = this.bp("b???????_?????_?????_100_?????_11000_11")
  val bge    = this.bp("b???????_?????_?????_101_?????_11000_11")
  val bltu   = this.bp("b???????_?????_?????_110_?????_11000_11")
  val bgeu   = this.bp("b???????_?????_?????_111_?????_11000_11")

  val lb     = this.bp("b???????_?????_?????_000_?????_00000_11")
  val lh     = this.bp("b???????_?????_?????_001_?????_00000_11")
  val lw     = this.bp("b???????_?????_?????_010_?????_00000_11")
  val lbu    = this.bp("b???????_?????_?????_100_?????_00000_11")
  val lhu    = this.bp("b???????_?????_?????_101_?????_00000_11")
  val sb     = this.bp("b???????_?????_?????_000_?????_01000_11")
  val sh     = this.bp("b???????_?????_?????_001_?????_01000_11")
  val sw     = this.bp("b???????_?????_?????_010_?????_01000_11")

  val addi   = this.bp("b???????_?????_?????_000_?????_00100_11")
  val slti   = this.bp("b???????_?????_?????_010_?????_00100_11")
  val sltiu  = this.bp("b???????_?????_?????_011_?????_00100_11")
  val xori   = this.bp("b???????_?????_?????_100_?????_00100_11")
  val ori    = this.bp("b???????_?????_?????_110_?????_00100_11")
  val andi   = this.bp("b???????_?????_?????_111_?????_00100_11")
  val slli   = this.bp("b0000000_?????_?????_001_?????_00100_11")
  val srli   = this.bp("b0000000_?????_?????_101_?????_00100_11")
  val srai   = this.bp("b0100000_?????_?????_101_?????_00100_11")
  val add    = this.bp("b0000000_?????_?????_000_?????_01100_11")
  val sub    = this.bp("b0100000_?????_?????_000_?????_01100_11")
  val sll    = this.bp("b0000000_?????_?????_001_?????_01100_11")
  val slt    = this.bp("b0000000_?????_?????_010_?????_01100_11")
  val sltu   = this.bp("b0000000_?????_?????_011_?????_01100_11")
  val xor    = this.bp("b0000000_?????_?????_100_?????_01100_11")
  val srl    = this.bp("b0000000_?????_?????_101_?????_01100_11")
  val sra    = this.bp("b0100000_?????_?????_101_?????_01100_11")
  val or     = this.bp("b0000000_?????_?????_110_?????_01100_11")
  val and    = this.bp("b0000000_?????_?????_111_?????_01100_11")

  val ebreak = this.bp("b0000000_00001_00000_000_00000_11100_11")


  val mul    = this.bp("b0000001_?????_?????_000_?????_01100_11")
  val mulh   = this.bp("b0000001_?????_?????_001_?????_01100_11")
  val mulhsu = this.bp("b0000001_?????_?????_010_?????_01100_11")
  val mulhu  = this.bp("b0000001_?????_?????_011_?????_01100_11")
  val div    = this.bp("b0000001_?????_?????_100_?????_01100_11")
  val divu   = this.bp("b0000001_?????_?????_101_?????_01100_11")
  val rem    = this.bp("b0000001_?????_?????_110_?????_01100_11")
  val remu   = this.bp("b0000001_?????_?????_111_?????_01100_11")

  val inv    = this.bp("b???????_?????_?????_???_?????_?????_??")
}

import flow.components.{RegControl, PcControlInterface, ALUControlInterface}
class Control(width: Int) extends RawModule {
  // Helpers
  class WrapList[T](vl: T) { type Type = T; val v = vl}
  object wrap extends Poly1 {
    implicit def default[A] = at[A](Right(_).withLeft[Int])
  }
  def lit(x: Element) = { x.litValue.toInt }
  def toBits(t: dst.Type): BitPat = {
    val list = t.toList
    list.map(e => e match {
      case Right(x) => BitPat(lit(x).U(x.getWidth.W))
      case Left(x) => BitPat.dontCare(x)
    }).reduceLeft(_ ## _)
  }
  val r = Right
  def l[T <: Any](x: T) = x match {
    case x: ChiselEnum => Left(log2Ceil(x.all.length))
    case x: Data => Left(x.getWidth)
    case _ => throw new IllegalArgumentException
  }

  val inst = IO(Input(UInt(width.W)))

  val reg = IO(Flipped(new RegControl))
  val pc = IO(Flipped(new PcControlInterface))
  val alu = IO(Flipped(new ALUControlInterface))
  val ram = IO(Flipped(new RamControlInterface(32)))

  val dst = new WrapList((
    reg.ctrlBindPorts ++
    pc.ctrlBindPorts ++
    alu.ctrlBindPorts ++
    ram.ctrlBindPorts).map(wrap))

  val dstList = dst.v.toList
  val controlWidth = dstList.map(_.toOption.get.getWidth).reduce(_ + _)
  val reversePrefixSum = dstList.scanLeft(0)(_ + _.toOption.get.getWidth)
  val sliceIndex = reversePrefixSum.map(controlWidth - _)
  val slices = sliceIndex.map(_ - 1).zip(sliceIndex.tail)

  import reg.WriteSelect._
  import reg._
  import pc.SrcSelect._
  import pc._
  import alu.OpSelect._
  import alu.SrcASelect._
  import alu.SrcBSelect._
  import pc._
  import RV32Inst._
  val ControlMapping: Array[(BitPat, dst.Type)] = Array(
    //     Regs | writeEnable :: writeSelect :: HNil
    //     PC   | useImmB :: srcSelect :: HNil
    //     Exe  | op :: srcASelect :: srcBSelect :: signExt :: HNil
    //     Mem  | valid :: writeMask :: writeEnable :: HNil

    (lui   , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc)::
              r(aOpAdd)     :: r(aSrcAZero)  :: r(aSrcBImmU) :: r(false.B) ::
              r(false.B)    :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (auipc , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc)::
              r(aOpAdd)     :: r(aSrcAPc)  :: r(aSrcBImmU) :: r(false.B) ::
              r(false.B)    :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    // ---- Control Transfer Instructions ----
    (jal   , (r(true.B)     :: r(rNpc)     ::
              r(false.B)    :: r(pExeOut)  ::
              r(aOpAdd)     :: r(aSrcAPc)  :: r(aSrcBImmJ) :: r(false.B) ::
              r(false.B)    :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (jalr  , (r(true.B)     :: r(rNpc)     ::
              r(false.B)    :: r(pExeOut)    ::
              r(aOpAdd)     :: r(aSrcARs1) :: r(aSrcBImmI) :: r(false.B) ::
              r(false.B)    :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (beq   , (r(false.B)    :: l(WriteSelect) ::
              r(true.B)     :: r(pStaticNpc)     ::
              r(aOpSlt)     :: r(aSrcARs1)  :: r(aSrcBRs2) :: l(Bool())    ::
              r(false.B)    :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (bne   , (r(false.B)    :: l(WriteSelect) ::
              r(true.B)     :: r(pStaticNpc)     ::
              r(aOpSlt)     :: r(aSrcARs1)  :: r(aSrcBRs2) :: l(Bool())    ::
              r(false.B)    :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (blt   , (r(false.B)    :: l(WriteSelect) ::
              r(true.B)     :: r(pStaticNpc)     ::
              r(aOpSlt)     :: r(aSrcARs1)  :: r(aSrcBRs2) :: r(true.B)  ::
              r(false.B)    :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (bge   , (r(false.B)    :: l(WriteSelect) ::
              r(true.B)     :: r(pStaticNpc)     ::
              r(aOpSlt)     :: r(aSrcARs1)  :: r(aSrcBRs2) :: r(true.B)  ::
              r(false.B)    :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (bltu  , (r(false.B)    :: l(WriteSelect)::
              r(true.B)     :: r(pStaticNpc)    ::
              r(aOpSltu)     :: r(aSrcARs1)    :: r(aSrcBRs2) :: r(false.B) ::
              r(false.B)    :: l(UInt(4.W))  :: r(false.B)   :: HNil)),

    (bgeu  , (r(false.B)    :: l(WriteSelect)::
              r(true.B)     :: r(pStaticNpc)    ::
              r(aOpSltu)     :: r(aSrcARs1)    :: r(aSrcBRs2) :: r(false.B) ::
              r(false.B)    :: l(UInt(4.W))  :: r(false.B)   :: HNil)),

    // ---- Memory Access Instructions ----

    (lb    , (r(true.B)     :: r(rMemOut)    ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpAdd)     :: r(aSrcARs1)   :: r(aSrcBImmI) :: l(Bool()) ::
              r(true.B)     :: l(UInt(4.W))  :: r(false.B)   :: HNil)),

    (lh    , (r(true.B)     :: r(rMemOut)    ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpAdd)     :: r(aSrcARs1)   :: r(aSrcBImmI) :: l(Bool()) ::
              r(true.B)     :: l(UInt(4.W))  :: r(false.B)   :: HNil)),

    (lw    , (r(true.B)     :: r(rMemOut)    ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpAdd)     :: r(aSrcARs1)   :: r(aSrcBImmI) :: l(Bool()) ::
              r(true.B)     :: l(UInt(4.W))  :: r(false.B)   :: HNil)),

    (sb    , (r(false.B)    :: l(WriteSelect)::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpAdd)     :: r(aSrcARs1)   :: r(aSrcBImmS) :: l(Bool()) ::
              r(true.B)     :: r(1.U(4.W))   :: r(true.B)   :: HNil)),

    (sh    , (r(false.B)    :: l(WriteSelect)::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpAdd)     :: r(aSrcARs1)   :: r(aSrcBImmS) :: l(Bool()) ::
              r(true.B)     :: r(3.U(4.W))   :: r(true.B)   :: HNil)),

    (sw    , (r(false.B)    :: l(WriteSelect)::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpAdd)     :: r(aSrcARs1)   :: r(aSrcBImmS) :: l(Bool()) ::
              r(true.B)     :: r(15.U(4.W))  :: r(true.B)   :: HNil)),

    // ---- Integer Computational Instructions ---

    (addi  , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpAdd)     :: r(aSrcARs1) :: r(aSrcBImmI) :: l(Bool())    ::
              r(false.B)    :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (slti  , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSlt)     :: r(aSrcARs1) :: r(aSrcBImmI) :: r(true.B)  ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (sltiu , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSltu)     :: r(aSrcARs1) :: r(aSrcBImmI) :: r(false.B) ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (xori  , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpOr)      :: r(aSrcARs1) :: r(aSrcBImmI) :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (ori   , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpXor)     :: r(aSrcARs1) :: r(aSrcBImmI) :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (andi  , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpAnd)     :: r(aSrcARs1) :: r(aSrcBImmI) :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (slli  , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSll)     :: r(aSrcARs1) :: r(aSrcBImmI) :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (srli  , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSrl)     :: r(aSrcARs1) :: r(aSrcBImmI) :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (srai  , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSra)     :: r(aSrcARs1) :: r(aSrcBImmI) :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (add   , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpAdd)     :: r(aSrcARs1) :: r(aSrcBRs2)  :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (sub   , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSub)     :: r(aSrcARs1) :: r(aSrcBRs2)  :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (sll   , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSll)     :: r(aSrcARs1) :: r(aSrcBRs2)  :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (slt   , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSlt)     :: r(aSrcARs1) :: r(aSrcBRs2)  :: r(true.B)  ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (sltu  , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSltu)     :: r(aSrcARs1) :: r(aSrcBRs2)  :: r(false.B) ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (xor   , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpXor)     :: r(aSrcARs1) :: r(aSrcBRs2)  :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (srl   , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSrl)     :: r(aSrcARs1) :: r(aSrcBRs2)  :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (sra   , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpSra)     :: r(aSrcARs1) :: r(aSrcBRs2)  :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (or    , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpOr)      :: r(aSrcARs1) :: r(aSrcBRs2)  :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),

    (and   , (r(true.B)     :: r(rAluOut)  ::
              r(false.B)    :: r(pStaticNpc) ::
              r(aOpAnd)     :: r(aSrcARs1) :: r(aSrcBRs2)  :: l(Bool())    ::
              r(false.B)     :: l(UInt(4.W)):: r(false.B)   :: HNil)),
  )

  val default = BitPat(0.U(controlWidth.W))

//   println(s"ControlMapping = ${ControlMapping.map(it => (it._1 -> toBits(it._2))).foreach(x => println(x._2))}\n")
  val out = decoder(
    inst,
    TruthTable(ControlMapping.map(it => (it._1 -> toBits(it._2))), default))
  val srcList = slices.map(s => out(s._1, s._2))

  assert(out != default)
  println(s"out = $out, default = $default\n")
  println(s"dstList = ${dstList}\n")
  println(s"srcList = ${srcList}\n")
  srcList
    .zip(dstList)
    .foreach({ case (src, dst) =>
      dst.toOption.get := src.asTypeOf(dst.toOption.get)
    })
}

import flow.components.{RegisterFile, ProgramCounter, ALU, RamDpi}
import chisel3.util.experimental.loadMemoryFromFileInline
class Flow extends Module {
  def lit(x: Data) = { x.litValue.toInt }

  val dataType = UInt(32.W)
  val ram = Module(new RamDpi)
  val control = Module(new Control(32))
  val reg = Module(new RegisterFile(dataType, 32, 2))
  val pc = Module(new ProgramCounter(dataType))
  val alu = Module(new ALU(dataType))

  // TODO: Switch to Decoupled and Arbiter later
  ram.io.pc := pc.out
  val inst = ram.io.inst

  dontTouch(reg.control.writeEnable)

  import control.pc.SrcSelect._

  val npc = Wire(dataType)
  npc := pc.out + 4.U
  pc.in.exeOut := alu.out.result
  pc.in.immB := Cat(Fill(20, inst(31)), inst(7), inst(30, 25), inst(11, 8), 0.U(1.W))

  control.inst := inst
  reg.control <> control.reg
  // FIXME: Probably optimizable with bulk connection
  pc.control <> control.pc
  pc.control.useImmB := control.pc.useImmB
  alu.control <> control.alu
  val branchUseSlt = Wire(Bool())
  val branchInvertResult = Wire(Bool())
  branchUseSlt := inst(14)
  branchInvertResult := inst(12)
  val _branchResult = Mux(branchUseSlt, alu.out.result(0), alu.out.eq)
  val branchResult = Mux(branchInvertResult, !_branchResult, _branchResult)
  pc.control.useImmB := control.pc.useImmB && branchResult
  // printf(cf"_branchResult = ${_branchResult}, branchResult = ${branchResult}\n")
  // printf(cf"pcin.useImmB = ${pc.control.useImmB}, control.out.useImmB = ${control.pc.useImmB} \n")

  import control.reg.WriteSelect._
  reg.in.writeData(lit(rAluOut)) := alu.out.result
  // TODO: Read address in load command goes here
  reg.in.writeData(lit(rMemOut)) := ram.io.readData
  reg.in.writeData(lit(rNpc)) := npc

  reg.in.writeAddr := inst(11, 7)
  reg.in.rs(0) := inst(19, 15) // rs1
  reg.in.rs(1) := inst(24, 20) // rs2

  // TODO: Bulk connection here 
  // FIXME: The following 2 line won't compile with bloop
  ram.io.clock := clock
  ram.io.reset := reset
  ram.io.writeAddr := alu.out.result
  ram.io.writeData := reg.out.src(1)
  ram.io.writeMask := control.ram.writeMask
  ram.io.writeEnable := control.ram.writeEnable
  ram.io.valid := control.ram.valid
  ram.io.readAddr := alu.out.result

  import control.alu.SrcASelect._
  import control.alu.SrcBSelect._
  alu.in.a(lit(aSrcARs1)) := reg.out.src(0)
  alu.in.a(lit(aSrcAPc)) := pc.out
  alu.in.a(lit(aSrcAZero)) := 0.U

  alu.in.b(lit(aSrcBRs2)) := reg.out.src(1)
  // alu.in.b(lit(aSrcBImmI)) := inst(31, 20).pad(aSrcBImmI.getWidth)
  alu.in.b(lit(aSrcBImmI)) := Cat(Fill(20, inst(31)), inst(31, 20))
  alu.in.b(lit(aSrcBImmJ)) := Cat(Fill(12, inst(31)), inst(19, 12), inst(20), inst(30, 25), inst(24, 21), 0.U(1.W))
  alu.in.b(lit(aSrcBImmS)) := Cat(Fill(20, inst(31)), inst(31), inst(30, 25), inst(11, 8), inst(7))
  alu.in.b(lit(aSrcBImmU)) := Cat(inst(31, 12), 0.U(12.W))

  Trace.traceName(pc.out)
  dontTouch(control.out)
}
