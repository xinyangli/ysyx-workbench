package flowpc

import chisel3._
import chiseltest._
import org.scalatest.freespec.AnyFreeSpec
import chiseltest.simulator.WriteVcdAnnotation

import chisel3.util.{SRAM}

import flowpc.components._
class RegisterFileSpec extends AnyFreeSpec with ChiselScalatestTester {
  "RegisterFileCore" - {
    "register 0 is always 0" in {
      test(new RegisterFileCore(32, UInt(32.W), 2)) { c =>
          c.readPorts(0).addr.poke(0)
          c.readPorts(1).addr.poke(0)
          c.writePort.enable.poke(true)
          c.writePort.addr.poke(0)
          c.writePort.data.poke(0x1234)

          c.readPorts(0).data.expect(0)
          c.readPorts(1).data.expect(0)
          c.clock.step(2)
          c.readPorts(0).data.expect(0)
          c.readPorts(1).data.expect(0)
        }
    }
    "register other than 0 can be written" in {
      test(new RegisterFileCore(32, UInt(32.W), 2)) { c =>
          import scala.util.Random
          val r = new Random()
          for (i <- 1 until 32) {
            val v = r.nextLong() & 0xFFFFFFFFL
            c.readPorts(0).addr.poke(i)
            c.writePort.enable.poke(true)
            c.writePort.addr.poke(i)
            c.writePort.data.poke(v)

            c.clock.step(1)
            c.readPorts(0).data.expect(v)
          }
        }
    }
  }
  "RegisterInterface" - {
    class Top extends Module {
      val io = RegisterFile(32, UInt(32.W), 2, 2)
    }
    "worked" in {
      test(new Top) { c =>
        // import c.io.control.WriteSelect._
        // c.io.control.writeEnable.poke(true)
        // c.io.control.writeSelect.poke(rAluOut)
        // c.io.data.write.addr.poke(1)
        // c.io.data.write.data(rAluOut.asUInt).poke(0xcdef)
        // c.io.data.read(0).rs.poke(1)
        // c.clock.step(1)
        // c.io.data.read(0).src.expect(0xcdef)
      }
    }
  }
}
