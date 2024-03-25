{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, ... }@inputs: with inputs;
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        crossPkgs = import nixpkgs {
          localSystem = system;
          crossSystem = {
            config = "riscv32-none-elf";
            gcc = {
              abi = "ilp32";
              arch = "rv32if";
            };
          };
        };
      in
      {
        packages.nemu = pkgs.callPackage ./nemu { am-kernels = self.packages.${system}.am-kernels-cmake; };
        packages.abstract-machine = crossPkgs.callPackage ./abstract-machine { isa = "riscv"; platform = "nemu"; };

        packages.am-kernels-cmake = crossPkgs.stdenv.mkDerivation rec {
          pname = "am-kernels-cmake";
          version = "2024.02.18";

          src = ./am-kernels; 

          nativeBuildInputs = [
            pkgs.cmake
          ];

          cmakeFlags = [
            (pkgs.lib.cmakeFeature "ISA" "riscv")
            (pkgs.lib.cmakeFeature "PLATFORM" "nemu")
            (pkgs.lib.cmakeFeature "CMAKE_INSTALL_DATADIR" "share")
          ];

          buildInputs = [
            # SDL2
            self.packages.${system}.abstract-machine
          ];
        };
        
        devShells.nemu = pkgs.mkShell {
          packages = with pkgs; [
            clang-tools
            gdb
          ];
          inputsFrom = [
            self.packages.${system}.nemu
          ];
        };
      }
    );
}
