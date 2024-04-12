ThisBuild / scalaVersion     := "2.13.12"
ThisBuild / version          := "0.1.0"


val chiselVersion = "6.2.0"
val circeVersion  = "0.14.1"

lazy val root = (project in file("."))
  .settings(
    name := "flow",
    libraryDependencies ++= Seq(
      "org.chipsalliance" %% "chisel" % chiselVersion,
      "edu.berkeley.cs" %% "chiseltest" % "6.0.0" % "test",
      "com.chuusai" %% "shapeless" % "2.3.3",
      "com.github.scopt" %% "scopt" % "4.1.0",
    ) ++ Seq(
      "io.circe" %% "circe-core",
      "io.circe" %% "circe-generic",
      "io.circe" %% "circe-parser"
    ).map(_ % circeVersion),
    scalacOptions ++= Seq(
      "-language:reflectiveCalls",
      "-deprecation",
      "-feature",
      "-Xcheckinit",
      "-Ymacro-annotations",
    ),
    addCompilerPlugin("org.chipsalliance" % "chisel-plugin" % chiselVersion cross CrossVersion.full),
    addSbtPlugin("ch.epfl.scala" % "sbt-bloop" % "1.5.17")
  )