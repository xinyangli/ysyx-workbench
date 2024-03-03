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
            abi = "ilp32";
          };
        };
      in
      {
        packages.nemu = pkgs.callPackage ./nemu {};
        
        devShells.am-kernels = crossPkgs.mkShell {
          inputsFrom = [
            self.packages.${system}.nemu
          ];
          packages = [
            pkgs.stdenv.cc
            pkgs.readline
            pkgs.libllvm
          ];
          shellHook = ''
            export PROJECT_ROOT=/home/xin/repo/ysyx-workbench
            export AM_HOME=$PROJECT_ROOT/abstract-machine;
            export NEMU_HOME=$PROJECT_ROOT/nemu;
          '';
        };
      }
    );
}

