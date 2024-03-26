{ stdenv, 
  lib,
  cmake,
  SDL2,
  isa ? "native",
  platform ? "NEMU"
}:
stdenv.mkDerivation {
  pname = "abstract-machine";
  version = "2024.02.18";

  src = ./.;

  cmakeFlags =  [
    (lib.cmakeFeature "ISA" isa)
    (lib.cmakeBool "__PLATFORM_${lib.strings.toUpper platform}__" true)
  ];

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [

  ] ++ (if platform=="native" then [ SDL2 ] else [ ]);
}
