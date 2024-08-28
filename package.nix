{ lib
, clangStdenv
, cmake
, ninja
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
    "^framework.*"
    "CMakeLists.txt"
  ];

  nativeBuildInputs = [ cmake ninja ];
  buildInputs = [
    libGL
    glfw
    glew-egl
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

