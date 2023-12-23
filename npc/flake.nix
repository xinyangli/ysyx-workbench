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
            verilator
            gtkwave
            gcc
            bear
          ];
          shellHook = ''
            export NEMU_HOME=/home/xin/repo/ysyx-workbench/nemu
          '';
        };
      }
    );
}
