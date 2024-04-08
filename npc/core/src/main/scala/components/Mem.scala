package flow.components

import chisel3._
import chisel3.util.HasBlackBoxPath
import chisel3.util.HasBlackBoxResource
import chisel3.util.log2Ceil
import chisel3.experimental.noPrefix
import scala.collection.SeqMap
import flow.components
import shapeless.{HNil, ::}

class RamControlInterface(addrWidth: Int) extends Bundle {
  val valid = Input(Bool())
  val writeMask = Input(UInt((addrWidth / 8).W))
  val writeEnable = Input(Bool())
  def ctrlBindPorts = {
    valid :: writeMask :: writeEnable :: HNil
  }

}

/* FIXME: Extends here might not be the best solution.
 *        We need a way to merge two bundles together
 */
class RamInterface[T <: Data](tpe: T, addrWidth: Int) extends RamControlInterface(addrWidth) {
  val writeAddr = Input(UInt(addrWidth.W))
  val writeData = Input(tpe)
  val readAddr = Input(UInt(addrWidth.W))
  val readData = Output(tpe)
  val pc = Input(UInt(addrWidth.W))
  val inst = Output(tpe)
}

class RamDpi extends BlackBox with HasBlackBoxResource {
  val io = IO((new RamInterface(UInt(32.W), 32)))
  addResource("/RamDpi.v")
}
