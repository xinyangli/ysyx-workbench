ThisBuild / scalaVersion     := "2.13.12"
ThisBuild / version          := "0.1.0"


val chiselVersion = "6.2.0"

lazy val root = (project in file("."))
  .settings(
    name := "flow",
    libraryDependencies ++= Seq(
      "org.chipsalliance" %% "chisel" % chiselVersion,
      "edu.berkeley.cs" %% "chiseltest" % "6.0.0" % "test",
      "com.chuusai" %% "shapeless" % "2.3.3"
    ),
    scalacOptions ++= Seq(
      "-language:reflectiveCalls",
      "-deprecation",
      "-feature",
      "-Xcheckinit",
      "-Ymacro-annotations",
    ),
    addCompilerPlugin("org.chipsalliance" % "chisel-plugin" % chiselVersion cross CrossVersion.full),
  )
