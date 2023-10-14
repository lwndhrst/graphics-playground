{
  inputs = {
    nixpkgs = {
      url = "github:nixos/nixpkgs/nixos-unstable";
    };
  };

  outputs = { self, nixpkgs }:
    let 
      forAllSystems = function:
        nixpkgs.lib.genAttrs [
          "x86_64-linux"
        ] (system: function nixpkgs.legacyPackages.${system});

      vk-renderer =
        { lib
        , llvmPackages_16 
        , compiler-flags ? []
        }:

        llvmPackages_16.stdenv.mkDerivation {
          pname = "vk-renderer";
          version = "2023-10-14";

          src = ./src;

          buildInputs = [
          ];

          buildPhase = ''
            clang++ main.cpp -o vk-renderer ${lib.concatStringsSep " " compiler-flags}
          '';

          installPhase = ''
            mkdir -p $out
            cp vk-renderer $out
          '';
        };

    in {
      packages = forAllSystems (pkgs: {
        default = pkgs.callPackage vk-renderer {};
        release = pkgs.callPackage vk-renderer {
          compiler-flags = [ "-O3" ];
        };
      });

      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          buildInputs = with pkgs; [
          ];

          LD_LIBRARY_PATH = with pkgs; lib.makeLibraryPath [
          ];
        };
      });
    };
}
