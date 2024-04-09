package flow.components
import chisel3._
import chisel3.util.{Valid, log2Ceil}
import chisel3.util.MuxLookup
import shapeless.{HNil, ::}

class PcControlInterface extends Bundle {
  object SrcSelect extends ChiselEnum {
    val pStaticNpc, pExeOut = Value
  }

  val useImmB = Input(Bool())
  val srcSelect = Input(SrcSelect())

  def ctrlBindPorts = {
    useImmB :: srcSelect :: HNil
  }
}

class ProgramCounter[T <: UInt](tpe: T) extends Module {

  val control = IO(new PcControlInterface)
  val in = IO(new Bundle {
    val immB = Input(tpe)
    val exeOut = Input(tpe)
  })
  val out = IO(Output(tpe))

  private val pc_reg = RegInit(0x80000000L.U)

//   pc := in.pcSrcs(control.srcSelect.asUInt)
  import control.SrcSelect._
  when( control.useImmB === true.B ) {
    pc_reg := pc_reg + in.immB
  }. elsewhen( control.srcSelect === pStaticNpc) {
    pc_reg := pc_reg + 4.U
  }. elsewhen( control.srcSelect === pExeOut) {
    pc_reg := in.exeOut
  }
  out := pc_reg
}

object ProgramCounter {
  def apply[T <: UInt](tpe: T): ProgramCounter[T] = {
    val pc = Module(new ProgramCounter(tpe))
    pc
  }
}
