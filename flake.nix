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
        { glslang
        , lib
        , llvmPackages_15 
        , SDL2
        , vulkan-headers 
        , vulkan-loader 
        , vulkan-validation-layers
        , clangFlags ? []
        , logLevel ? "NONE" # one of the following: DEBUG, INFO, WARN, ERROR, NONE
        }:

        llvmPackages_15.libcxxStdenv.mkDerivation {
          pname = "vk-renderer";
          version = "2023-10-14";

          src = ./.;

          buildInputs = [
            glslang
            SDL2
            vulkan-headers
            vulkan-loader
            vulkan-validation-layers
          ];

          buildPhase = ''
            clang++ src/*.cpp -o vk-renderer ${lib.concatStringsSep " " [
              "-std=c++20"
              "-fexperimental-library"
              "-fuse-ld=gold"
              "-DLOG_LEVEL=${logLevel}"
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
        debug = pkgs.callPackage vk-renderer {
          logLevel = "DEBUG";
          clangFlags = [ 
            "-DENABLE_VALIDATION_LAYERS=true"
          ];
        };
        release = pkgs.callPackage vk-renderer {
          logLevel = "NONE";
          clangFlags = [ 
            "-O3"
            "-flto"
          ];
        };
        default = self.packages.${system}.debug;
      });

      app = forEachSystem (pkgs: system: {
        debug = {
          type = "app";
          program = "${self.packages.${system}.debug}/bin/vk-renderer";
        };
        release = {
          type = "app";
          program = "${self.packages.${system}.release}/bin/vk-renderer";
        };
        default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/vk-renderer";
        };
      });

      devShells = forEachSystem (pkgs: system: {
        default = pkgs.mkShell {
          packages = with pkgs; [
            glibc
            glslang
            llvmPackages_15.libcxx
            SDL2
            vulkan-headers
            vulkan-loader
            vulkan-validation-layers
          ];

          shellHook = with pkgs; ''
            export CPATH=$CPATH:${llvmPackages_15.libcxx.dev}/include/c++/v1:${glibc.dev}/include
            export CPLUS_INCLUDE_PATH=$CPATH
            export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${llvmPackages_15.libcxx}/lib:${llvmPackages_15.libcxxabi}/lib
          '';
        };
      });
    };
}
