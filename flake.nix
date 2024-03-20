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
        packages.nemu = pkgs.callPackage ./nemu { am-kernels = self.packages.${system}.am-kernels-rv32; };

        packages.am-kernels = pkgs.stdenv.mkDerivation rec {
          pname = "am-kernels";
          version = "2024.02.18";

          buildInputs = [
            pkgs.SDL2
          ];

          src = pkgs.fetchFromGitHub {
            owner = "NJU-ProjectN";
            repo = "am-kernels";
            rev = "bb725d6f8223dd7de831c3b692e8c4531e9d01af";
            hash = "sha256-ZHdrw28TN8cMvhhzM469OV7cp0Yp+8yao855HP4+P4A=";
          };

          AM_HOME = pkgs.fetchFromGitHub {
            owner = "xinyangli";
            repo = "abstract-machine";
            rev = "788595aac61c6b2f3b78ca8aa7d08dc33911bca4";
            hash = "sha256-YvWHIBP9tz3HL2TyibftvvQrpkWUDPnviCF4oyLmdjg=";
          };

          ARCH = "native";

          patchPhase = ''
            sed -i 's/\/bin\/echo/echo/' tests/cpu-tests/Makefile
          '';

          buildPhase = ''
            AS=$CC make -C tests/cpu-tests BUILD_DIR=$(pwd)/build ARCH=$ARCH
          '';

          installPhase = ''
            mkdir -p $out/bin
            ls build/
            cp build/native/*.elf $out/bin/
          '';

          # dontFixup = true;
        };

        packages.am-kernels-rv32 = crossPkgs.stdenv.mkDerivation rec {
          pname = "am-kernels-rv32";
          version = "2024.02.18";

          src = pkgs.fetchFromGitHub {
            owner = "NJU-ProjectN";
            repo = "am-kernels";
            rev = "bb725d6f8223dd7de831c3b692e8c4531e9d01af";
            hash = "sha256-ZHdrw28TN8cMvhhzM469OV7cp0Yp+8yao855HP4+P4A=";
          };

          AM_HOME = pkgs.fetchFromGitHub {
            owner = "xinyangli";
            repo = "abstract-machine";
            rev = "788595aac61c6b2f3b78ca8aa7d08dc33911bca4";
            hash = "sha256-YvWHIBP9tz3HL2TyibftvvQrpkWUDPnviCF4oyLmdjg=";
          };

          ARCH = "riscv32-nemu";

          patchPhase = ''
            sed -i 's/\/bin\/echo/echo/' tests/cpu-tests/Makefile
          '';

          buildPhase = ''
            AS=$CC make -C tests/cpu-tests BUILD_DIR=$(pwd)/build ARCH=$ARCH
          '';

          installPhase = ''
            mkdir -p $out/share/images $out/share/dump
            cp build/riscv32-nemu/*.bin $out/share/images
            cp build/riscv32-nemu/*.txt $out/share/dump
          '';

          dontFixup = true;
        };

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            gdb
          ] ++ builtins.attrValues self.packages.${system};
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

