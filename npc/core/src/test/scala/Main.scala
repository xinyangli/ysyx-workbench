package flow

import chisel3._
import chiseltest._
import org.scalatest.freespec.AnyFreeSpec
import chiseltest.simulator.WriteVcdAnnotation

import flow.Flow

class RV32CPUSpec extends AnyFreeSpec with ChiselScalatestTester {
  "MemoryFile" - {
    "correctly load" in {
      import chisel3.util.{SRAM, SRAMInterface, HexMemoryFile}
      class UserMem extends Module {
        val io = IO(new SRAMInterface(1024, UInt(32.W), 1, 1, 0))
        val memoryFile = HexMemoryFile("../resource/addi.txt") 
        io :<>= SRAM(
          size = 1024,
          tpe = UInt(32.W),
          numReadPorts = 1,
          numWritePorts = 1,
          numReadwritePorts = 0,
          memoryFile = memoryFile
        )
        
        val read = io.readPorts(0).data
        printf(cf"memoryFile=$memoryFile, readPort=$read%x\n")
      }
      test(new UserMem).withAnnotations(Seq(WriteVcdAnnotation)) { c =>
        c.io.readPorts(0).enable.poke(true.B)
        c.io.writePorts(0).enable.poke(false.B)
        c.io.writePorts(0).address.poke(0.U)
        c.io.writePorts(0).data.poke(0.U)
        for (i <- 0 until 32) {
          c.io.readPorts(0).address.poke(i.U)
          c.clock.step(1)
        }
      }
    }
  }
  "should compile" in {
    test(new Flow) { c =>
      c.clock.step(1)
    }
  }

}
