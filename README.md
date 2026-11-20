# Cometa Phoenix — 0.9.11 (candidato nativo)

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

## Audio nativo 0.9.10
O motor usa síntese procedural original em C++17 (não é uma amostra Ferrari). Além da aceleração e troca de marcha, cada tipo de colisão, o processo de reparação e o fim da corrida têm efeitos diferentes. A tecla V controla o som. Consulte `tests/test_engine_audio.cpp`, `tests/test_impact_audio.cpp` e `game/ImpactAudio.hpp`.

## Capturas v0.9.10 (RX 580, renderizacao real)
[Oficina com piso de concreto fotografado](docs/screenshots/0910-garage.jpg) | [Corrida Litoral](docs/screenshots/0910-race.jpg). As imagens nao demonstram que o audio foi ouvido ou que a corrida foi jogada manualmente.

## Capturas da versão 0.9.10
- [Oficina 3D — piso fotográfico](docs/screenshots/0910-garage.jpg)
- [Corrida com chuva — kart MK-III](docs/screenshots/0910-race.jpg)

Audio: engine wavetable original com seis marchas virtuais e SFX procedurais distintos para impactos, reparos e conclusão de corrida; não usa samples de terceiros. Para verificar saída SDL, execute `cometa_game.exe` sem `--benchmark` e observe `AUDIO_DEVICE OPEN`; o dispositivo deve estar ativo, e o teste automatizado não substitui escuta real.

## Fantasma da melhor volta (0.9.11)

Uma volta válida grava a trajetória C++17 na memória (máximo de 4096 amostras); nas voltas seguintes, o fantasma ciano reproduz o recorde com interpolação. A tecla G alterna sua visibilidade. Trocar de circuito ou reiniciar apaga a gravação. Não existe salvamento entre partidas. O fantasma não colide com o kart. [Captura real na RX 580](docs/screenshots/0911-ghost-fullhd.jpg).

## 0.9.12 � recordes persistentes
O melhor tempo e a trajet�ria do fantasma s�o gravados no diret�rio de prefer�ncias do usu�rio (SDL2), separados por pista e clima. N�o precisam de nuvem. Recordes inv�lidos s�o rejeitados, e benchmarks n�o alteram arquivos pessoais. A mudan�a do clima no meio da volta invalida a tentativa de recorde.
