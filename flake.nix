{
  inputs = {
    nixpkgs = {
      url = "github:nixos/nixpkgs/nixos-unstable";
    };
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };

    in {
      packages.${system}.default = pkgs.llvmPackages_16.stdenv.mkDerivation {
        name = "vk-renderer";
        version = "2023-10-14";

        src = "${self}";

        buildInputs = with pkgs; [
          
        ];

        buildPhase = ''
          clang++ src/main.cpp -o vk-renderer
        '';

        installPhase = ''
          mkdir -p $out
          cp vk-renderer $out
        '';
      };
    };
}
