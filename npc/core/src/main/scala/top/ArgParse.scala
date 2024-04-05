package flow

import scopt.{ OParser, DefaultOEffectSetup }
import java.io.File

case class CliOptions(
    targetDir: File = new File("."),
    configFile: Option[File] = None,
    argsFile: Option[File] = None,
    verilatorConfigFileOut: File = new File("conf.vlt"),
) {
  val builder = OParser.builder[CliOptions]
  val parser =  {
    import builder._
    OParser.sequence(
      programName("flow"),
      help("help"),
      opt[Option[File]]('c', "config")
        .action((x, c) => c.copy(configFile = x))
        .text("JSON Configuration file for generation"),
      opt[Option[File]]("args-file")
        .action((x, c) => c.copy(argsFile = x))
        .text("Passing file content as args when emit"),
      opt[File]('o', "target-dir")
        .action((x, c) => c.copy(targetDir = x))
        .text("Output files relative to this path"),
      opt[File]("out-verilator-conf")
        .action((x, c) => c.copy(verilatorConfigFileOut = x))
        .text("Options needed when simulating with verilator")
    )
  }

  def parse(args: Array[String]): CliOptions = {
    OParser.runParser(parser, args, CliOptions()) match {
      case (result, effects) =>
        OParser.runEffects(effects, new DefaultOEffectSetup {
          // ignore terminate
          override def terminate(exitState: Either[String, Unit]): Unit = ()
        })

        result match {
          case Some(cliOptions: CliOptions) => { return cliOptions }
          case _ => { throw new IllegalArgumentException("Wrong command line argument") }
        }
    }
  }
}
