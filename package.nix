{ lib
, clangStdenv
, cmake
, ninja
, libGL
, glfw
, glew
, glm
, target
}:
clangStdenv.mkDerivation {
  name = target;
  allowSubstitutes = false;

  src = lib.sourceByRegex ./. [
    "^labs.*"
    "^framework.*"
    "CMakeLists.txt"
  ];

  nativeBuildInputs = [ cmake ninja ];
  buildInputs = [
    libGL
    (glfw.overrideAttrs (oldAttrs: {
      cmakeFlags = oldAttrs.cmakeFlags ++ [
        "-DGLFW_BUILD_WAYLAND=OFF"
      ];
    }))

    glew
    glm
  ];

  configurePhase = /* bash */ ''
    cmake -G Ninja .
  '';

  buildPhase = /* bash */ ''
    ninja ${target}
  '';

  installPhase = /* bash */ ''
    mkdir -p $out/bin
    mv bin/* $out/bin
  '';
}

