{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, ... }@inputs: with inputs;
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system}; in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            gtkwave
            gdb
            jq
            bear
            clang-tools
            rnix-lsp
            sbt
          ];

          nativeBuildInputs = with pkgs; [
            verilator
            gcc
            python3
            scala
            self.packages.${system}.circt
          ];

          buildInputs = with pkgs; [
            SDL2
            SDL2_image
            jre
          ];

          shellHook = ''
            export NEMU_HOME=/home/xin/repo/ysyx-workbench/nemu
          '';
        };
        
        # This version (1.43.0) of circt does not exist in nixpkgs
        # and Chisel 5.1.0 specifically build against it, so here we are.
        # Ref: https://github.com/NixOS/nixpkgs/blob/b6465c8/pkgs/development/compilers/circt/default.nix
        packages.circt =
          with pkgs;
          let
            pythonEnv = python3.withPackages (ps: [ ps.psutil ]);
          in
          stdenv.mkDerivation rec {
            pname = "circt";
            version = "1.43.0";
            src = fetchFromGitHub {
              owner = "llvm";
              repo = "circt";
              rev = "firtool-${version}";
              sha256 = "sha256-RkjigboswLkLgLkgOGahQLIygCkC3Q9rbVw3LqIzREY=";
              fetchSubmodules = true;
            };

            requiredSystemFeatures = [ "big-parallel" ];

            nativeBuildInputs = [ cmake ninja git pythonEnv ];

            cmakeDir = "../llvm/llvm";
            cmakeFlags = [
              "-DLLVM_ENABLE_BINDINGS=OFF"
              "-DLLVM_ENABLE_OCAMLDOC=OFF"
              "-DLLVM_BUILD_EXAMPLES=OFF"
              "-DLLVM_OPTIMIZED_TABLEGEN=ON"
              "-DLLVM_ENABLE_PROJECTS=mlir"
              "-DLLVM_EXTERNAL_PROJECTS=circt"
              "-DLLVM_EXTERNAL_CIRCT_SOURCE_DIR=.."
              "-DCIRCT_LLHD_SIM_ENABLED=OFF"
            ];

            LIT_FILTER_OUT = if stdenv.cc.isClang then "CIRCT :: Target/ExportSystemC/.*\.mlir" else null;

            preConfigure = ''
              find ./test -name '*.mlir' -exec sed -i 's|/usr/bin/env|${coreutils}/bin/env|g' {} \;
            '';

            installPhase = ''
              runHook preInstall
              mkdir -p $out/bin
              mv bin/{{fir,hls}tool,circt-{as,dis,lsp-server,opt,reduce,translate}} $out/bin
              runHook postInstall
            '';

            doCheck = true;
            checkTarget = "check-circt check-circt-integration";

            meta = {
              description = "Circuit IR compilers and tools";
              homepage = "https://circt.org/";
              license = lib.licenses.asl20;
              maintainers = with lib.maintainers; [ sharzy ];
              platforms = lib.platforms.all;
            };
          };
      }
    );
}

