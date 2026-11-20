# Cometa Phoenix — operação 0.9 (candidato nativo)

Evolução do Kart Cometa, ECOP13A / LAB12–LAB14. Corrida 2D com cinco climas e oficina 3D somente para diagnóstico, regeneração e abandono. Motor e interface em C++17, SDL2 + OpenGL 3.3; GLSL para GPU. Sem navegador, JavaScript ou cutscenes. Não é um simulador estrutural validado nem uma versão final homologada no Windows.

## Conceitos da disciplina
- `include/cometa/Simulation.hpp`: `Historico<T>` armazena `EventoDano` e `AmostraTelemetria`; `limitar<T>` generaliza limites.
- `src/Simulation.cpp`: `std::vector`, `std::sort`, `std::find_if`, `std::all_of`, `std::max_element` e estratégias polimórficas `EstrategiaReparo`. A validação avalia todas as peças, não apenas a última reparada.
- `game/GamePhysics.cpp`: dirigibilidade, clima e máquina de estados compartilhada com o motor CLI. `game/GameApp.cpp` e `NativeExtras.hpp`: renderização, HUD, malha, áudio.
- Melhoria: desgaste, diagnóstico e regeneração com recursos finitos; regeneração parcial e rejeição de fratura crítica. Mecânica e tecnologias são aproximações didáticas.

## Compilar e jogar
Linux (Debian/Ubuntu, exemplo): `sudo apt install build-essential cmake libsdl2-dev`. Driver OpenGL 3.3.
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
./build/cometa_game
```
Windows x64 (a validar): Visual Studio 2022 com Desktop development with C++, CMake, vcpkg (`vcpkg install sdl2:x64-windows`). Definir `VCPKG_ROOT`, executar `jogar_windows.bat`. Ver `game/docs/WINDOWS.md` e workflow de compilação.

Enter inicia; WASD/setas pilotam; 1–5 clima; C piloto automático; X impacto; Z fratura crítica; R reinicia; P pausa; Q qualidade; M mapa; V som; F11 tela cheia; Esc menu/sair. Cenários CLI: `./build/cometa_phoenix --cenario normal --duracao 22` ou `--cenario impacto_critico --duracao 38`.

## Verificar e medir
`cometa_game --benchmark --frames 300 --autopilot --weather rain --quality 1`; usar `--garage` para medir oficina. FPS médio após aquecimento, p50/p95/p99 em ms. Leia `docs/OPERACAO_PHOENIX.md` e `evidencias/`.

## Fontes e ferramentas
Enunciado Projeto Cometa Livre (UNIFEI), laboratórios originais, docs C++/STL, SDL2/OpenGL; ChatGPT e Codex para revisão e implementação assistida; Higgsfield para referência artística; malha local derivada da versão web 0.8 e som V12 procedural original. A apresentação final só deve ser produzida após homologação e compreensão do código pelo aluno.
