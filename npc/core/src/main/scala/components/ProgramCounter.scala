package flow.components
import chisel3._
import chisel3.util.{Valid, log2Ceil}
import chisel3.util.MuxLookup
import shapeless.{HNil, ::}

class PcControlInterface extends Bundle {
  object SrcSelect extends ChiselEnum {
    val pStaticNpc, pBranchResult = Value
  }

  val srcSelect = Input(SrcSelect())

  type CtrlTypes = SrcSelect.Type :: HNil
  def ctrlBindPorts: CtrlTypes = {
    srcSelect :: HNil
  }
}

class ProgramCounter[T <: Data](tpe: T) extends Module {

  val control = IO(new PcControlInterface)
  val in = IO(new Bundle {
    val pcSrcs = Input(Vec(control.SrcSelect.all.length, tpe))
  })
  val out = IO(Output(tpe))

  private val pc = RegInit(0x80000000L.U)

  pc := in.pcSrcs(control.srcSelect.asUInt)
  out := pc
}

object ProgramCounter {
  def apply[T <: Data](tpe: T): ProgramCounter[T] = {
    val pc = Module(new ProgramCounter(tpe))
    pc
  }
}
