package flow

import flow._

import chisel3._
import chisel3.experimental.Trace._
import chisel3.stage.{ChiselGeneratorAnnotation, DesignAnnotation}
import chisel3.util.experimental.InlineInstance
import circt.stage.ChiselStage
import firrtl.AnnotationSeq
import firrtl.annotations.TargetToken.{Instance, OfModule, Ref}
import java.io.PrintWriter
import scala.io.Source


// TODO: Generate verilator config file

object VerilogMain extends App {
  val opt = CliOptions().parse(args)
  val topName = "Flow"

  val config: Config = opt.configFile match {
    case Some(f) => {
      val source = Source.fromFile(f)
      val jsonString = source.mkString
      source.close()
      io.circe.parser.decode[Config](jsonString) match {
        case Right(x) => x
        case Left(e) => throw e
      }
    }
    case None => Config(traceConfig = TraceConfig(enable = true))
  }

  val annos = (new ChiselStage).execute(
    Array("--target-dir", "/home/xin/repo/ysyx-workbench/npc/build/Flow/vsrc", "--target", "systemverilog", "--split-verilog"),
    Seq(

    ) ++ (if(config.traceConfig.enable) Seq(ChiselGeneratorAnnotation(() => new Flow)) else Seq())
  )

  if(config.traceConfig.enable) {
    val dut = annos.collectFirst { case DesignAnnotation(dut) => dut }.get.asInstanceOf[Flow]

    val verilatorConfigSeq = finalTargetMap(annos)
      .values
      .flatten
      .map(ct =>
        s"""public_flat_rd -module "${ct.tokens.collectFirst {
              case OfModule(m) => m
            }.get}" -var "${ct.tokens.collectFirst { case Ref(r) => r }.get}"
         """ )
    
    val verilatorConfigWriter = new PrintWriter(opt.targetDir.sp
    verilatorConfigWriter.write("`verilator_config")
    for(ct <- verilatorConfigSeq) {
      try {
        verilatorConfigWriter.println(ct)
      } finally {
        verilatorConfigWriter.close()
      }
    }
    
  }
}