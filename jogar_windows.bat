@echo off
setlocal
cd /d "%~dp0"
if exist "cometa_game.exe" (
  start "" /WAIT "cometa_game.exe"
  goto :done
)
if exist "build-win\Release\cometa_game.exe" (
  start "" /WAIT "build-win\Release\cometa_game.exe"
  goto :done
)
where cmake >nul 2>nul
if errorlevel 1 (
  echo Instale CMake e Visual Studio C++.
  pause
  exit /b 1
)
if not defined VCPKG_ROOT (
  echo Defina VCPKG_ROOT apontando para a instalacao do vcpkg ou baixe o ZIP Windows do GitHub Actions.
  pause
  exit /b 1
)
cmake -S . -B build-win -A x64 "-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DCOMETA_REFERENCIAS=OFF
if errorlevel 1 goto :failed
cmake --build build-win --config Release --parallel 4
if errorlevel 1 goto :failed
cmake --install build-win --config Release --prefix windows-portable
start "" /WAIT "windows-portable\cometa_game.exe"
goto :done
:failed
echo Erro na compilacao. Consulte o console para detalhes.
pause
exit /b 1
:done
endlocal
