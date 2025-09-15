{
  inputs = { };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };

    in {
      devShell.${system} = pkgs.mkShell {
        name = "graphics-playground";

        packages = with pkgs; [
          cmake
          glslang
          ninja
          pkg-config

          clang-tools
          gdb
          glsl_analyzer

          vulkan-headers
          vulkan-loader
          vulkan-utility-libraries
          vulkan-validation-layers

          # wayland dependencies for SDL
          libffi
          libGL
          libxkbcommon
          wayland
          wayland-protocols
          wayland-scanner
        ];

        shellHook = ''
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.libxkbcommon}/lib
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.wayland}/lib
        '';
      };
    };
}
