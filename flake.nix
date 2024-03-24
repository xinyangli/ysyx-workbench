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
        packages.nemu = pkgs.callPackage ./nemu { am-kernels = self.packages.${system}.am-kernels-rv32; };

        packages.abstract-machine = crossPkgs.stdenv.mkDerivation rec {
          pname = "abstract-machine";
          version = "2024.02.18";

          src = ./abstract-machine;

          nativeBuildInputs = [
            pkgs.cmake
          ];

          buildInputs = [
            # SDL2
          ];
          cmakeFlags =  [
            (pkgs.lib.cmakeFeature "ISA" "riscv")
            (pkgs.lib.cmakeBool "__PLATFORM_NEMU__" true)
          ];
        };

        packages.am-kernels-cmake = crossPkgs.stdenv.mkDerivation rec {
          pname = "am-kernels-cmake";
          version = "2024.02.19";

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

        packages.am-kernels = pkgs.stdenv.mkDerivation rec {
          pname = "am-kernels";
          version = "2024.02.18";

          buildInputs = with pkgs; [
            SDL2
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
            rm -r build/native/src build/native/tests
            cp -r build/native/* $out/bin/
          '';
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
            mkdir -p $out/bin
            cp build/riscv32-nemu/*.bin $out/share/images
            cp build/riscv32-nemu/*.txt $out/share/dump
            cp build/riscv32-nemu/*.elf $out/bin
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

        devShells.abstract-machine-shell = pkgs.mkShell {
          packages = with pkgs; [
            clang-tools
            cmake
            check
            pkg-config
            SDL2
          ];
        };
      }
    );
}
