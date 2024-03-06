{ pkgs,
  lib,
  stdenv,
  am-kernels
}:

stdenv.mkDerivation rec {
  pname = "nemu";
  version = "2024-03-02";

  src = ./.;

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

  checkInputs = [
    pkgs.check
    am-kernels
  ];

  IMAGES_PATH = "${am-kernels}/share/images";

  configurePhase = ''
    export NEMU_HOME=$(pwd)
    make alldefconfig
  '';

  buildPhase = ''
    make
  '';

  doCheck = true;
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
