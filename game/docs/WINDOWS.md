# Windows x64 — compilação prevista, ainda não homologada
Instalar ferramentas C++ do Visual Studio 2022, CMake e vcpkg com SDL2 (`vcpkg install sdl2:x64-windows`). No PowerShell, com `VCPKG_ROOT` correto:
```
cmake -S . -B build-win -A x64 "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build-win --config Release --parallel 4
ctest --test-dir build-win -C Release --output-on-failure
cmake --install build-win --config Release --prefix windows-portable
.\windows-portable\cometa_game.exe
.\BENCHMARK_WINDOWS.ps1
```
O executável, SDL2.dll, game/assets e game/shaders devem permanecer juntos. O workflow Windows reproduz os passos. Validar OpenGL 3.3 com o driver real. A versão não inclui um .exe Windows pré-compilado nesta rodada.
