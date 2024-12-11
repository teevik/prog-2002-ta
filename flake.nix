{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs = inputs@{ flake-parts, ... }: flake-parts.lib.mkFlake { inherit inputs; } {
    systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];

    perSystem = { pkgs, lib, ... }:
      let
        package = pkgs.callPackage ./package.nix;
        makePackages = targets: lib.genAttrs targets (target: package { target = target; });
      in
      {
        packages = makePackages [ "lab1" "lab2" "lab3" ];

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            cmake
            clang-tools

            glfw
            glm
            glew
            stb
          ];

          LD_LIBRARY_PATH = with pkgs;
            lib.makeLibraryPath [
              wayland
            ];
        };
      };
  };
}
