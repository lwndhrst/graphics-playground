{
  inputs = {
    nixpkgs = {
      url = "github:nixos/nixpkgs/nixos-unstable";
    };
  };

  outputs = { self, nixpkgs }:
    let 
      forEachSystem = function:
        nixpkgs.lib.genAttrs [
          "x86_64-linux"
        ] (system: function nixpkgs.legacyPackages.${system});

      vk-renderer =
        { lib
        , llvmPackages_16 
        , compilerFlags ? []
        }:

        llvmPackages_16.stdenv.mkDerivation {
          pname = "vk-renderer";
          version = "2023-10-14";

          src = ./.;

          buildInputs = [];

          buildPhase = ''
            clang++ src/main.cpp -o vk-renderer ${lib.concatStringsSep " " compilerFlags}
          '';

          installPhase = ''
            mkdir -p $out/bin
            cp vk-renderer $out/bin
          '';
        };

    in {
      packages = forEachSystem (pkgs: {
        default = pkgs.callPackage vk-renderer {};
        release = pkgs.callPackage vk-renderer {
          compilerFlags = [ "-O3" ];
        };
      });

      app = forEachSystem (pkgs: {
        default = {
          type = "app";
          program = "${pkgs.callPackage vk-renderer {}}/bin/vk-renderer";
        };
        release = {
          type = "app";
          program = "${pkgs.callPackage vk-renderer {
            compilerFlags = [ "-O3" ];
          }}/bin/vk-renderer";
        };
      });

      devShells = forEachSystem (pkgs: {
        default = pkgs.mkShell {
          buildInputs = [];

          LD_LIBRARY_PATH = with pkgs; lib.makeLibraryPath [];
        };
      });
    };
}
