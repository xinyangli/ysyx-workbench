package flow.components

import chisel3._
import chisel3.util.HasBlackBoxPath
import chisel3.util.HasBlackBoxResource
import chisel3.util.log2Ceil
import chisel3.experimental.noPrefix
import scala.collection.SeqMap
import javax.swing.plaf.synth.SynthRadioButtonMenuItemUI

class RamInterface[T <: Data](tpe: T, addrWidth: Int) extends Bundle {
  val valid = Input(Bool())
  val writeEnable = Input(Bool())
  val writeAddr = Input(UInt(addrWidth.W))
  val writeData = Input(tpe)
  val writeMask = Input(UInt((addrWidth / 8).W))
  val readAddr = Input(UInt(addrWidth.W))
  val readData = Output(tpe)
}

class RamDpi extends BlackBox with HasBlackBoxResource {
  val io = IO(new RamInterface(UInt(32.W), 32))
  addResource("/RamDpi.v")
}
