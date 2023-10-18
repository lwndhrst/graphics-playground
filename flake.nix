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
        { compilerFlags ? []
        , glslang
        , lib
        , llvmPackages_16 
        , logLevel ? "INFO" # One of the following: DEBUG, INFO, WARN, ERROR, NONE
        , SDL2
        , vulkan-headers 
        , vulkan-loader 
        }:

        llvmPackages_16.stdenv.mkDerivation {
          pname = "vk-renderer";
          version = "2023-10-14";

          src = ./.;

          buildInputs = [
            glslang
            SDL2
            vulkan-headers
            vulkan-loader
          ];

          buildPhase = ''
            clang++ src/*.cpp -o vk-renderer \
              -DLOG_LEVEL=${logLevel} \
              -lSDL2 \
              -lvulkan \
              ${lib.concatStringsSep " " compilerFlags}
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
          compilerFlags = [ "-O2" ];
          logLevel = "NONE";
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
          ];

          LD_LIBRARY_PATH = with pkgs; lib.makeLibraryPath [];
        };
      });
    };
}
