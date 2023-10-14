{
  inputs = {
    nixpkgs = {
      url = "github:nixos/nixpkgs/nixos-unstable";
    };
  };

  outputs = { self, nixpkgs }:
    let 
      vk-renderer =
        { llvmPackages_16 
        , ...
        }:

        llvmPackages_16.stdenv.mkDerivation {
          pname = "vk-renderer";
          version = "2023-10-14";

          src = ./src;

          buildInputs = [ ];

          buildPhase = ''
            clang++ main.cpp -o vk-renderer
          '';

          installPhase = ''
            mkdir -p $out
            cp vk-renderer $out
          '';
        };

    in {
      packages."x86_64-linux".default = nixpkgs.legacyPackages."x86_64-linux".callPackage vk-renderer {};
    };
}
