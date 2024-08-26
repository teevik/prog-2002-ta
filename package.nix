{ lib
, clangStdenv
, cmake
, libGL
, glfw
, glew-egl
, glm
, target
}:
clangStdenv.mkDerivation {
  name = target;
  allowSubstitutes = false;

  src = lib.sourceByRegex ./. [
    "^labs.*"
    "CMakeLists.txt"
  ];

  nativeBuildInputs = [ cmake ];
  buildInputs = [
    libGL
    glfw
    glew-egl
    glm
  ];

  configurePhase = /* bash */ ''
    cmake .
  '';

  buildPhase = /* bash */ ''
    make ${target}
  '';

  installPhase = /* bash */ ''
    mkdir -p $out/bin
    mv bin/* $out/bin
  '';
}

