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
          shader-slang
          tracy
          valgrind

          vulkan-headers
          vulkan-loader
          vulkan-utility-libraries
          vulkan-validation-layers

          # SDL wayland
          libdecor
          libffi
          libGL
          libxkbcommon
          wayland
          wayland-protocols
          wayland-scanner

          # SDL x11
          xorg.libX11
          xorg.libXcursor
          xorg.libXext
          xorg.libXfixes
          xorg.libXi
          xorg.libXrandr
          xorg.libXScrnSaver
          xorg.libXtst
          xorg.libxcb

          # Tracy
          dbus
        ];

        shellHook = ''
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.vulkan-loader}/lib

          # SDL wayland
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.libdecor}/lib
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.libxkbcommon}/lib
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.wayland}/lib

          # SDL x11
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libX11}/lib
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXcursor}/lib
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXext}/lib
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXfixes}/lib
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXi}/lib
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXrandr}/lib
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXScrnSaver}/lib
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.xorg.libXtst}/lib
        '';
      };
    };
}
