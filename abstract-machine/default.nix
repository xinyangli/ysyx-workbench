{ stdenv,
  lib,
  cmake,
  SDL2,
  isa ? "native",
  platform ? [ ]
}:
stdenv.mkDerivation {
  pname = "abstract-machine";
  version = "2024.06.01";

  src = ./.;

  cmakeFlags =  [
    (lib.cmakeFeature "ISA" isa)
  ] ++ map (p: (lib.cmakeBool "__PLATFORM_${lib.strings.toUpper p}__" true)) platform;

  cmakeBuildType = "Debug";
  dontStrip = true;

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [

  ] ++ (if isa=="native" then [ SDL2 ] else [ ]);

  doCheck = true;
}
