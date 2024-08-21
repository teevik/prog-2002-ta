{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs = inputs@{ flake-parts, ... }: flake-parts.lib.mkFlake { inherit inputs; } {
    systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];

    perSystem = { pkgs, ... }: {
      devShells.default = pkgs.mkShell {
        packages = with pkgs; [
          pkg-config

          # cmake and cplusplus
          cmake
          ninja
          # clang

          # x11
          xorg.libX11
          xorg.libXrandr
          xorg.libXinerama
          xorg.libXcursor
          xorg.libXi

          # opengl
          libGL

          wayland
          wayland-scanner
          libxkbcommon
          clang-tools
        ];

        LD_LIBRARY_PATH = with pkgs;
          lib.makeLibraryPath [
            wayland
            libGL
            libxkbcommon
          ];
      };
    };
  };
}
