{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    nur-xin = {
      url = "git+https://git.xinyang.life/xin/nur.git";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { self, ... }@inputs: with inputs;
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system} //
          { nur.xin = nur-xin.legacyPackages.${system}; };
      in
      {
        devShells.default = with pkgs; mkShell {
          packages = [
            clang-tools
            rnix-lsp

            gdb
            jre

            gtkwave
          ];

          inputsFrom = [ self.packages.${system}.nemu ];
        };

        packages.nemu = with pkgs; stdenv.mkDerivation rec {
          pname = "nemu";
          version = "2024-01-02";

          src = ./.;

          nativeBuildInputs = [
            gnumake
            flex
            bison
          ];

          buildInputs = [
            readline
            libllvm
          ];

          NEMU_HOME = src;

          meta = with lib; {
            description = "NJU EMUlator, a full system x86/mips32/riscv32/riscv64 emulator for teaching";
            homepage = "https://github.com/NJU-ProjectN/nemu.git";
            license = with licenses; [ ];
            maintainers = with maintainers; [ ];
          };
        };
      }
    );
}

