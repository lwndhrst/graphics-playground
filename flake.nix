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
        ] (system: function nixpkgs.legacyPackages.${system} system);

      vk-renderer =
        { binutils
        , glslang
        , lib
        , llvmPackages_15 
        , SDL2
        , vulkan-headers 
        , vulkan-loader 
        , vulkan-validation-layers
        , clangFlags ? []
        , logLevel ? "INFO" # one of the following: DEBUG, INFO, WARN, ERROR, NONE
        }:

        llvmPackages_15.stdenv.mkDerivation {
          pname = "vk-renderer";
          version = "2023-10-14";

          src = ./.;

          buildInputs = [
            binutils
            glslang
            SDL2
            vulkan-headers
            vulkan-loader
            vulkan-validation-layers
          ];

          buildPhase = ''
            clang++ src/*.cpp -o vk-renderer ${lib.concatStringsSep " " [
              "-DLOG_LEVEL=${logLevel}"
              "-fuse-ld=gold"
              "-lSDL2"
              "-lvulkan"
            ]} ${lib.concatStringsSep " " clangFlags}
          '';

          installPhase = ''
            mkdir -p $out/bin
            cp vk-renderer $out/bin
          '';
        };

    in {
      packages = forEachSystem (pkgs: system: {
        default = pkgs.callPackage vk-renderer {};
        debug = pkgs.callPackage vk-renderer {
          logLevel = "DEBUG";
        };
        release = pkgs.callPackage vk-renderer {
          logLevel = "NONE";
          clangFlags = [ 
            "-O3"
            "-flto"
          ];
        };
      });

      app = forEachSystem (pkgs: system: {
        default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/vk-renderer";
        };
        debug = {
          type = "app";
          program = "${self.packages.${system}.debug}/bin/vk-renderer";
        };
        release = {
          type = "app";
          program = "${self.packages.${system}.release}/bin/vk-renderer";
        };
      });

      devShells = forEachSystem (pkgs: system: {
        default = pkgs.mkShell {
          packages = with pkgs; [
            glslang
            SDL2
            vulkan-headers
            vulkan-loader
            vulkan-validation-layers
          ];

          LD_LIBRARY_PATH = with pkgs; lib.makeLibraryPath [];
        };
      });
    };
}
