{
  description = "queue port in cpp";

  inputs = {
    # grab nixpkgs, I use unstable!
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";

    # for 'foreach' system
    utils.url = "github:numtide/flake-utils";

  };

  outputs = { self, nixpkgs, utils }:
    utils.lib.eachSystem [
      "x86_64-linux"
      "aarch64-linux"
      "x86_64-darwin"
      "aarch64-darwin"
    ] (system:
      let
        # packages for the given system
        pkgs = import nixpkgs {
          inherit system;
        };
        cross = pkgs.pkgsCross.aarch64-multiplatform;
      in {
        # on `nix develop`
        devShells.default = cross.mkShell.override { stdenv = cross.clangStdenv; } {
          nativeBuildInputs = [ pkgs.cmake ];

          # puts a nice hook, I like this
          shellHook = ''
            PS1="(dev) $PS1"
            export CMAKE_EXPORT_COMPILE_COMMANDS=1
          '';
        };
      });
}
