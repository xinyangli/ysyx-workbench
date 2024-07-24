{ pkgs,
  lib,
  stdenv,
  dtc,
  mini-gdbstub,
  am-kernels ? null,
  defconfig ? "alldefconfig",
}:

stdenv.mkDerivation rec {
  pname = "nemu";
  version = "2024-03-02";

  src = ./.;

  nativeBuildInputs = with pkgs; [
    gnumake
    pkg-config
    flex
    bison
    dtc
  ];

  buildInputs = with pkgs; [
    readline
    libllvm
    mini-gdbstub
  ];

  checkInputs = [
    pkgs.check
    am-kernels
  ];

  configurePhase = ''
    export NEMU_HOME=$(pwd)
    make ${defconfig}
  '';

  buildPhase = ''
    make
  '';

  doCheck = (am-kernels != null);
  checkPhase = if doCheck then ''
    export NEMU_IMAGES_PATH=${am-kernels}/share/am-kernels
    make test
  '' else "";

  installPhase = ''
    mkdir -p $out/bin
    mkdir -p $out/lib
    make PREFIX=$out install
  '';

  shellHook = ''
    export NEMU_HOME=$(pwd)
  '';

  meta = with lib; {
    description = "NJU EMUlator, a full system x86/mips32/riscv32/riscv64 emulator for teaching";
    homepage = "https://github.com/NJU-ProjectN/nemu.git";
    license = with licenses; [ ];
    maintainers = with maintainers; [ ];
  };
}
