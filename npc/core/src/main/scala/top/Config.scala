package flow

import io.circe.generic.JsonCodec

// Which group of signals to trace
@JsonCodec case class TraceConfig (
  enable: Boolean = false,
  registers: Array[Int] = Array(),
  mem: Array[(Int, Int)] = Array(),
)

@JsonCodec case class Config(
  // Whether to enable Difftest
  enableDifftest: Boolean = true,
  traceConfig: TraceConfig = TraceConfig(),
)
