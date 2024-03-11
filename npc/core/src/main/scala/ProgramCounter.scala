package flow.components
import chisel3._
import chisel3.util.{Valid, log2Ceil}
import chisel3.util.MuxLookup
import shapeless.{HNil, ::}

class ProgramCounter[T <: Data](tpe: T) extends Module {
  object SrcSelect extends ChiselEnum {
    val pStaticNpc, pBranchResult = Value
  }

  val control = IO(new Bundle {
    val srcSelect = Input(SrcSelect())
  })
  val in = IO(new Bundle {
    val pcSrcs = Input(Vec(SrcSelect.all.length, tpe))
  })
  val out = Output(tpe)

  out := in.pcSrcs(control.srcSelect.asUInt)

  type ctrlTypes = SrcSelect.Type :: HNil
  def ctrlBindPorts: ctrlTypes = {
    control.srcSelect :: HNil
  }
}

object ProgramCounter {
  def apply[T <: Data](tpe: T): ProgramCounter[T] = {
    val pc = new ProgramCounter(tpe)
    pc
  }
}
