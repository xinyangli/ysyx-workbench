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
            gnumake
            bison
            flex
            ncurses
            readline
            libllvm
          ];

          shellHook = ''
            export NEMU_HOME=/home/xin/repo/ysyx-workbench/nemu
            export AM_HOME=/home/xin/repo/ysyx-workbench/abstract-machine
          '';
        };
      }
    );
}
