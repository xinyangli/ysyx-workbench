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

  outputs = { self, flake-utils, nixpkgs, nixpkgs-circt162, pre-commit-hooks, nur-xin }@inputs:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
          overlays = [
            (self: super: {
              nvboard = nur-xin.legacyPackages.${system}.nvboard;
              mini-gdbstub = nur-xin.legacyPackages.${system}.mini-gdbstub;
            })
          ];
        };

        rv32CrossConfig = import nixpkgs {
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
              end-of-file-fixer.enable = true;
              cmake-format.enable = true;
              clang-format = {
                enable = true;
                types_or = pkgs.lib.mkForce [ "c" "c++" ];
              };
            };
          };
        };

        packages = rec {
          abstract-machine = pkgs.callPackage ./abstract-machine { isa = "native"; };
          nemu = pkgs.callPackage ./nemu { };
          nemu-lib = pkgs.callPackage ./nemu { };
          am-kernels = pkgs.callPackage ./am-kernels { abstract-machine = abstract-machine; arch = "native"; };

          rv32Cross = rec {
            abstract-machine = rv32CrossConfig.callPackage ./abstract-machine { isa = "riscv"; platform = [ "nemu" "npc" ]; };
            am-kernels-npc = rv32CrossConfig.callPackage ./am-kernels { inherit abstract-machine; arch = "riscv-npc"; };
            am-kernels-nemu = rv32CrossConfig.callPackage ./am-kernels { inherit abstract-machine; arch = "riscv-nemu"; };
            am-kernels = pkgs.callPackage ./am-kernels { abstract-machine = abstract-machine; arch = "riscv"; };
          };
        };

        devShells.nemu = pkgs.mkShell {
          packages = with pkgs; [
            clang-tools
            gdb
            SDL2
            gnumake
            pkg-config
            flex
            bison
            dtc

            readline
            libllvm
            mini-gdbstub
          ];
          inputsFrom = [
            self.packages.${system}.nemu
          ];
          NEMU_HOME = "/home/xin/repo/ysyx-workbench/nemu";
          NEMU_IMAGES_PATH = self.packages.${system}.rv32Cross.am-kernels-nemu + "/share/am-kernels";
        };

        devShells.npc = pkgs.mkShell.override { stdenv = pkgs.ccacheStdenv; } {
          inherit (self.checks.${system}.pre-commit-check) shellHook;
          CHISEL_FIRTOOL_PATH = "${nixpkgs-circt162.legacyPackages.${system}.circt}/bin";
          NPC_IMAGES_DIR="${self.packages.${system}.am-kernels-npc}/share/am-kernels";
          packages = with pkgs; [
            clang-tools
            cmake
            coursier
            espresso
            bloop

            gdb
            jre

            gtkwave
          ];

          nativeBuildInputs = with pkgs; [
            cmake
            sbt
            nvboard
            nixpkgs-circt162.legacyPackages.${system}.circt
            yosys
            cli11
            flex
            bison
            verilator
            self.packages.${system}.am-kernels-npc
          ];

          buildInputs = with pkgs; [
            nvboard
            openssl
            libllvm
            libxml2
            readline
            mini-gdbstub
          ] ++ self.checks.${system}.pre-commit-check.enabledPackages;

          cmakeFlags = [
            "-DDIFFTEST_LIB:string=${self.packages.${system}.nemu-lib}/lib/riscv32-nemu-interpreter-so"
          ];
        };
      }
    );
}
