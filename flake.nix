{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    nixpkgs-circt162.url = "github:NixOS/nixpkgs/7995cae3ad60e3d6931283d650d7f43d31aaa5c7";
    flake-utils.url = "github:numtide/flake-utils";
    pre-commit-hooks = {
      url = "github:cachix/pre-commit-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    nur-xin = {
      url = "git+https://git.xinyang.life/xin/nur.git";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { self, ... }@inputs: with inputs;
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
          overlays = [
            (self: super: {
              nvboard = nur-xin.legacyPackages.${system}.nvboard;
            })
          ];
        };
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
        checks = {
          pre-commit-check = pre-commit-hooks.lib.${system}.run {
            src = ./.;
            hooks = {
              trim-trailing-whitespace.enable = true;
              clang-format = {
                enable = true;
                types_or = pkgs.lib.mkForce [ "c" "c++" ];
              };
            };
          };
        };
        packages.nemu = pkgs.callPackage ./nemu { am-kernels = self.packages.${system}.am-kernels; };
        packages.abstract-machine = crossPkgs.callPackage ./abstract-machine { isa = "riscv"; platform = "nemu"; };

        packages.am-kernels = crossPkgs.stdenv.mkDerivation rec {
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

        devShells.npc = with pkgs; mkShell {
          inherit (self.checks.${system}.pre-commit-check) shellHook;
          CHISEL_FIRTOOL_PATH = "${nixpkgs-circt162.legacyPackages.${system}.circt}/bin";
          packages = [
            clang-tools
            cmake
            ninja
            coursier
            espresso
            bloop

            gdb
            jre

            gtkwave
          ];

          nativeBuildInputs = [
            cmake
            sbt
            nvboard
            nixpkgs-circt162.legacyPackages.${system}.circt
            yosys
            cli11
          ];

          buildInputs = [
            verilator
            nvboard
          ] ++ self.checks.${system}.pre-commit-check.enabledPackages;

          # DIFFTEST_LIB = ;
        };
      }
    );
}
