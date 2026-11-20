# Cometa Phoenix — 0.9.9 (candidato nativo)

Projeto ECOP13A / LAB12–LAB14. Corrida 2D com três circuitos e cinco condições climáticas, oficina 3D com diagnóstico, regeneração e abandono. Aplicativo e motor C++17 com SDL2/OpenGL 3.3 e shaders GLSL. Não depende de navegador nem JavaScript. Modelo físico didático, não certificado.

## Conceitos da disciplina
- `include/cometa/Simulation.hpp`: `Historico<T>` utilizado com `EventoDano` e `AmostraTelemetria`; função genérica `limitar<T>`.
- `src/Simulation.cpp`: contêineres e algoritmos STL (`vector`, `sort`, `find_if`, `all_of`, `max_element`); polimorfismo com `EstrategiaReparo`.
- `game/GamePhysics.cpp`: física de corrida, circuitos, clima, colisão direcional, tempos e voltas válidas, reboque até o box. Compartilha o motor de simulação com o CLI.
- `game/GameApp.cpp` e `NativeExtras.hpp`: renderização, HUD, malha indexada e áudio V12 sintetizado. Recursos de oficina finitos, fraturas estruturais críticas podem impedir retorno.

## Compilar e jogar
Linux (Debian/Ubuntu): `sudo apt install build-essential cmake libsdl2-dev`. Driver OpenGL 3.3.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
./build/cometa_game
```

Windows x64: Visual Studio 2022 + CMake + vcpkg (`sdl2:x64-windows`) ou LLVM/MinGW com SDL2. Ver `game/docs/WINDOWS.md`. A instalação portátil pode ser executada diretamente na pasta que contém `cometa_game.exe`, SDL2 e assets.

### Controles
No menu: Enter iniciar; T alternar Floresta/Alpes/Litoral; L alternar 1, 3, 5 voltas ou modo livre. Durante a corrida: WASD/setas, 1–5 clima, C piloto automático, X dano de teste, Z dano crítico, R reiniciar, P pausa, Q perfil gráfico, M mapa, V som, F11 tela cheia, Esc menu/sair. Apenas voltas válidas contam para terminar; sair da pista ou bater invalida a tentativa. No box, a cronometragem da tentativa é reiniciada.

## Testes e benchmarks
`cometa_game --benchmark --frames 7200 --autopilot --circuit 2 --weather rain --quality 3 --width 1920 --height 1080 --laps 0`. Benchmarks usam passo fixo, VSync e áudio desligados. `--garage` mede a oficina; `--screenshot caminho.ppm` captura imagem real do framebuffer. Não confundir tempo simulado com duração real do teste. CLI: `cometa_phoenix --cenario normal --duracao 22` ou `--cenario impacto_critico --duracao 38`. Ver `docs/CHANGELOG.md` e `docs/ASSETS_CC0.md`.

## Fontes e IA
Enunciado Projeto Cometa Livre (UNIFEI), laboratórios originais, documentação C++/STL, SDL2 e OpenGL. ChatGPT, Codex e BlueLab para implementação e revisão; Higgsfield para referência artística. Texturas fotográficas CC0 Poly Haven; kart derivado da malha local anterior; som original sintetizado em C++. Apresentação acadêmica final após homologação e compreensão do código pelo aluno. Não há alegação de fotorrealismo ou reprodução de gravação Ferrari.

## MK-III: capturas reais

[Oficina 3D](docs/screenshots/099-garage-final.jpg) | [Corrida no Litoral](docs/screenshots/099-race.jpg). Modelo com 92.924 triangulos; geradores C++17 em tools/. A geometria original permanece no historico do Git.
