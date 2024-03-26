package top

import flow._
import circt.stage.ChiselStage
import firrtl.options.Shell

object VerilogMain extends App {
  Config(enableDifftest = true)
  chisel3.emitVerilog(new Flow, args=args)
}