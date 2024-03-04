{ pkgs,
  lib,
  stdenv
}:

stdenv.mkDerivation rec {
  pname = "nemu";
  version = "2024-03-02";

  src = ./.;

  NEMU_HOME = "/build/nemu";
  nativeBuildInputs = with pkgs; [
    gnumake
    flex
    bison
  ];

  buildInputs = with pkgs; [
    check
    readline
    libllvm
  ];

  configurePhase = ''
    echo pwd=$(pwd)
    make alldefconfig
  '';

  buildPhase = ''
    make
  '';

  checkPhase = ''
    make test
  '';

  installPhase = ''
    mkdir -p $out/bin
    make PREFIX=$out install
  '';

  meta = with lib; {
    description = "NJU EMUlator, a full system x86/mips32/riscv32/riscv64 emulator for teaching";
    homepage = "https://github.com/NJU-ProjectN/nemu.git";
    license = with licenses; [ ];
    maintainers = with maintainers; [ ];
  };
}
