generate-compile-commands:
  mkdir -p build
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build
  cp build/compile_commands.json .
