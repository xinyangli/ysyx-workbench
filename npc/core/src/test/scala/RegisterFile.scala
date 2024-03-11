package flowpc

import chisel3._
import chiseltest._
import org.scalatest.freespec.AnyFreeSpec
import chiseltest.simulator.WriteVcdAnnotation

import flowpc.components._

class RegisterFileSpec extends AnyFreeSpec with ChiselScalatestTester {
  "RegisterFileCore" - {
    "(0) is always 0" - {
      // val reg = new RegisterFileCore(32, UInt(32.W), 2)
      test(new RegisterFileCore(32, UInt(32.W), 2)).withAnnotations(Seq(WriteVcdAnnotation)) { c => 
        c.readPorts(0).addr.poke(0)
        c.readPorts(1).addr.poke(0)
        c.writePort.enable.poke(true)
        c.writePort.addr.poke(0)
        c.writePort.data.poke(0xdeadbeef)

        c.readPorts(0).data.expect(0)
        c.readPorts(1).data.expect(0)
        c.clock.step(1)
        c.readPorts(0).data.expect(0)
        c.readPorts(1).data.expect(0)
      }
    }
  }
}
