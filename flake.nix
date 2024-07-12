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
              mini-gdbstub = nur-xin.legacyPackages.${system}.mini-gdbstub;
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
              end-of-file-fixer.enable = true;
              cmake-format.enable = true;
              clang-format = {
                enable = true;
                types_or = pkgs.lib.mkForce [ "c" "c++" ];
              };
            };
          };
        };

        packages.nemu = pkgs.callPackage ./nemu { am-kernels = self.packages.${system}.am-kernels-nemu; };
        packages.nemu-lib = pkgs.callPackage ./nemu { am-kernels = self.packages.${system}.am-kernels-nemu; defconfig = "riscv32-lib_defconfig"; };
        packages.abstract-machine = crossPkgs.callPackage ./abstract-machine { isa = "riscv"; platform = [ "nemu" "npc" ]; };
        packages.abstract-machine-native = pkgs.callPackage ./abstract-machine { isa = "native"; };

        packages.am-kernels-npc = crossPkgs.callPackage ./am-kernels { abstract-machine = self.packages.${system}.abstract-machine; arch = "riscv-npc"; };
        packages.am-kernels-nemu = crossPkgs.callPackage ./am-kernels { abstract-machine = self.packages.${system}.abstract-machine; arch = "riscv-nemu"; };

        # FIXME: native cannot compile due to abstract machine
        packages.am-kernels-native = crossPkgs.callPackage ./am-kernels { abstract-machine = self.packages.${system}.abstract-machine-native; arch = "native"; };

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
          NEMU_IMAGES_PATH = self.packages.${system}.am-kernels-nemu + "/share/am-kernels";
        };

        devShells.npc = with pkgs; mkShell.override { stdenv = ccacheStdenv; } {
          inherit (self.checks.${system}.pre-commit-check) shellHook;
          CHISEL_FIRTOOL_PATH = "${nixpkgs-circt162.legacyPackages.${system}.circt}/bin";
          NPC_IMAGES_DIR="${self.packages.${system}.am-kernels-npc}/share/am-kernels";
          packages = [
            clang-tools
            cmake
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
            flex
            bison
            verilator
            self.packages.${system}.am-kernels-npc
          ];

          buildInputs = [
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
