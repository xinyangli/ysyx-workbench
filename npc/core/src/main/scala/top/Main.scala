package top

import flow._
import circt.stage.ChiselStage
import chisel3._
import chisel3.stage.{ ChiselGeneratorAnnotation, DesignAnnotation }
import chisel3.experimental.Trace.finalTarget
import firrtl.AnnotationSeq
import firrtl.annotations.TargetToken



object VerilogMain extends App {
  val topName = "Flow"
def verilatorTemplate(data: Seq[Data], annos: AnnotationSeq): String = {
      val vpiNames = data.flatMap(finalTarget(annos)).map { ct =>
        s"""TOP.${ct.circuit}.${ct.path.map { case (TargetToken.Instance(i), _) => i }.mkString(".")}.${ct.tokens.collectFirst {
          case TargetToken.Ref(r) => r
        }.get}"""
      }
      s"""
         |#include "V${topName}.h"
         |#include "verilated_vpi.h"
         |#include <memory>
         |#include <verilated.h>
         |
         |int vpiGetInt(const char name[]) {
         |  vpiHandle vh1 = vpi_handle_by_name((PLI_BYTE8 *)name, NULL);
         |  if (!vh1)
         |    vl_fatal(__FILE__, __LINE__, "sim_main", "No handle found");
         |  s_vpi_value v;
         |  v.format = vpiIntVal;
         |  vpi_get_value(vh1, &v);
         |  return v.value.integer;
         |}
         |
         |int main(int argc, char **argv) {
         |  const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
         |  contextp->commandArgs(argc, argv);
         |  const std::unique_ptr<V$topName> top{new V$topName{contextp.get(), "TOP"}};
         |  top->reset = 0;
         |  top->clock = 0;
         |  int a_b = 1;
         |  top->i_a_b = a_b;
         |  bool started = false;
         |  int ticks = 20;
         |  while (ticks--) {
         |    contextp->timeInc(1);
         |    top->clock = !top->clock;
         |    if (!top->clock) {
         |      if (contextp->time() > 1 && contextp->time() < 10) {
         |        top->reset = 1;
         |      } else {
         |        top->reset = 0;
         |        started = true;
         |      }
         |      a_b = a_b ? 0 : 1;
         |      top->i_a_b = a_b;
         |    }
         |    top->eval();
         |    VerilatedVpi::callValueCbs();
         |    if (started && !top->clock) {
         |      const int i = top->i_a_b;
         |      const int o = vpiGetInt("${vpiNames.head}");
         |      if (i == o)
         |        vl_fatal(__FILE__, __LINE__, "sim_main", "${vpiNames.head} should be the old value of Module1.i_a_b");
         |      printf("${vpiNames.head}=%d Module1.m0.o_a_b=%d\\n", i, o);
         |    }
         |  }
         |  top->final();
         |  return 0;
         |}
         |""".stripMargin
    }
  // Config(enableDifftest = true)
  val annos = (new ChiselStage).execute(
    Array("--target-dir", "/tmp", "--target", "systemverilog"),
    Seq(
      ChiselGeneratorAnnotation(() => new Flow)
    )
  )
  val dut = annos.collectFirst { case DesignAnnotation(dut) => dut }.get.asInstanceOf[Flow]
  println(verilatorTemplate(Seq(dut.reg.control.writeEnable), annos))
  println(finalTarget(annos)(dut.reg.control.writeEnable).head)
}