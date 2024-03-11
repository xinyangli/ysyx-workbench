package npc

import chisel3._
import chiseltest._
import org.scalatest.freespec.AnyFreeSpec
import chiseltest.simulator.WriteVcdAnnotation

import npc.util._

// class ALUGeneratorSpec extends AnyFreeSpec with ChiselScalatestTester {
//   "With 32 width, " - {
//     val neg = (x: BigInt) => BigInt("FFFFFFFF", 16) - x + 1
//     val not = (x: BigInt) => x ^ BigInt("FFFFFFFF", 16)
//     val mask = BigInt("FFFFFFFF", 16)
//     val oprands: List[(BigInt, BigInt)] = List(
//       (5, 3), (101010, 101010), (0xFFFFFFFCL, 0xFFFFFFFFL), (4264115, 2)
//     )
//     val operations: Map[Int, (BigInt, BigInt) => BigInt] = Map(
//       0 -> ((a: BigInt, b: BigInt) => (a + b) & mask),
//       1 -> ((a: BigInt, b: BigInt) => (a + neg(b)) & mask),
//       2 -> ((a, _) => not(a)),
//       3 -> (_ & _),
//       4 -> (_ | _),
//       5 -> (_ ^ _),
//       6 -> ((a, b) => if (a < b) 1 else 0),
//       7 -> ((a, b) => if (a == b) 1 else 0),
//     )
//     val validate = (c: ALUGenerator[32], op: Int, oprands: List[(BigInt, BigInt)]) => {
//       c.io.op.poke(op.U)
//       oprands.foreach({ case (a, b) =>
//         c.io.a.poke(a.U)
//         c.io.b.poke(b.U)
//         c.io.out.expect(operations(op)(a, b))
//       })
//     }
//     "add should work" in {
//       test(new ALUGenerator(32)) { c => validate(c, 0, oprands) }
//     }
//     "sub should work" - {
//       "with positive result" in {
//         test(new ALUGenerator(32)) { c =>
//           validate(c, 1, oprands.filter({case (a, b) => a >= b}))
//         }
//       }
//       "with negative result" in {
//         test(new ALUGenerator(32)) { c =>
//           validate(c, 1, oprands.filter({case (a, b) => a < b}))
//         }
//       }
//     }
//     "not should work" in {
//       test(new ALUGenerator(32)) { c => validate(c, 2, oprands) }
//     }
//     "and should work" in {
//       test(new ALUGenerator(32)) { c => validate(c, 3, oprands) }
//     }
//     "or should work" in {
//       test(new ALUGenerator(32)) { c => validate(c, 4, oprands) }
//     }
//     "xor should work" in {
//       test(new ALUGenerator(32)) { c => validate(c, 5, oprands) }
//     }
//     "compare should work" in {
//       test(new ALUGenerator(32)) { c => validate(c, 6, oprands) }
//     }
//     "equal should work" in {
//       test(new ALUGenerator(32)) { c => validate(c, 7, oprands) }
//     }
//   }
// }
