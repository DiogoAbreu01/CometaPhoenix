<h1 align="center">COMETA PHOENIX</h1>

**Midnight garage. Sakura streets. Native C++.**

*A JDM-flavored racing and repair simulator — an engineering project that actually runs outside the browser.*

[Build & play](#compilar-e-jogar) · [Game systems](#conceitos-da-disciplina) · [Benchmarks](#testes-e-benchmarks) · [CI](.github/workflows/ci.yml)

[![CI](https://github.com/DiogoAbreu01/CometaPhoenix/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/DiogoAbreu01/CometaPhoenix/actions/workflows/ci.yml)
![C++17](https://img.shields.io/badge/C%2B%2B17-101017?style=flat-square&logo=cplusplus&logoColor=ff86ae)
![SDL2 / OpenGL](https://img.shields.io/badge/SDL2_%2F_OpenGL-101017?style=flat-square&logo=opengl&logoColor=ff86ae)

<table>
  <tr>
    <td><img src="docs/screenshots/sakura_race_v010.jpg" alt="In-engine Cometa Phoenix Sakura racing capture" width="430"></td>
    <td><img src="docs/screenshots/jdm_garage_v010.jpg" alt="In-engine Cometa Phoenix JDM garage capture" width="430"></td>
  </tr>
</table>
<sub>Existing project captures, not stock imagery.</sub>


> **DAs / Developer's note:** My childhood racing-game obsession meets C++: weather, vehicle damage, repair strategies and an actual graphics/rendering loop.

**Under the hood:** Native C++17, SDL2, OpenGL 3.3/GLSL, a shared simulation engine, STL algorithms, repair strategy polymorphism, race physics, garage management and reproducible CLI/graphics benchmarks. The physical model is educational, **not a certified vehicle simulator**.

**Build and validation:** Use CMake + CTest ([instructions below](#compilar-e-jogar)); in-game screenshots are already versioned in [docs/screenshots](docs/screenshots). Performance measurements depend on hardware and benchmark settings.

---

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

## 0.9.12 — recordes persistentes
O melhor tempo e a trajetória do fantasma são gravados no diretório de preferências do usuário (SDL2), separados por pista e clima. Não precisam de nuvem. Recordes inválidos são rejeitados, e benchmarks não alteram arquivos pessoais. A mudança do clima no meio da volta invalida a tentativa de recorde.

## 0.9.13 — adversários de tempo
Dois rivais C++17 seguem o traçado com velocidades diferentes e desaceleram no mau tempo. Escolha 0, 1 ou 2 oponentes pressionando K no menu, ou use --rivals 0|1|2. São concorrentes não-colidentes: ainda não possuem desvio de obstáculos ou contato físico. O HUD e o minimapa indicam os participantes.

## 0.9.14 — histórico de corridas e QA

Ao terminar uma corrida ou abandonar por dano crítico, o jogo acrescenta uma linha ao arquivo race_history.csv no mesmo diretório de preferências do SDL2 usado pelos recordes, com circuito, clima, voltas válidas, posição, tempo e danos. O arquivo pode ser aberto em um editor de planilhas. A UI mostra a posição final e o placar durante a prova.

Para realizar uma partida de QA sem alterar seus dados: cometa_game --frames 3600 --no-vsync --data-dir caminho_de_teste --autopilot --rivals 2 --circuit 0 --weather rain --laps 1. O teste deve gerar race_history.csv e record_c0_w1.phx no diretório escolhido. Sem --no-vsync, o jogo mantém o limite do monitor. Benchmarks não gravam arquivos pessoais.

[Captura real da chegada com dois rivais](docs/screenshots/0914-finish.jpg). [Roadmap e pendências técnicas](docs/ROADMAP_1_0.md).

## 0.9.15 — competicao, controle e telemetria

- IA de rivais com tres dificuldades (linha de comando --difficulty easy|normal|hard), adaptacao de curvas e deslocamento de faixa para ultrapassagem. D no menu altera dificuldade e K numero de adversarios.
- Contato kart-kart: hitbox circular simplificada, reacao a impacto, perda de velocidade, penalidade e eventual dano/reboque. Reacao na velocidade dos rivais e cooldown de contato; nao ha deformacao 3D.
- Pneus e freios: temperatura, desgaste e resposta de aderencia/frenagem. Componentes danificados alteram performance. Modelo didatico e aproximado.
- Menu: H apresenta as ultimas cinco corridas, B troca layout WASD/IJKL/setas, N muda as cores dos rivais, -/+ volume. Gamepad SDL2: direcao no stick esquerdo, gatilhos como aceleracao/freio, Start pausa. Dispositivo nao foi testado fisicamente.
- Spray de pneu procedural em chuva/temporal. [Menu](docs/screenshots/0915-menu.jpg), [corrida](docs/screenshots/0915-race.jpg), [oficina](docs/screenshots/0915-garage.jpg).
- Testes: ctest --test-dir build-win --output-on-failure. Opcional: cometa_race_stress_tests --minutes 30 (tempo de simulacao acelerado, nao teste termico/manual).

## 0.9.16 — ajuste de estabilidade
No menu, F liga/desliga a guia de trajetoria; B alterna esquemas de teclado. Colisoes graves entre karts agora exigem velocidade relativa normal de pelo menos 10,5 m/s, reduzindo reparos por contatos leves. Benchmarks emitem RIVAL_HARD_CONTACT para contatos graves.

## 0.9.17 — integridade de voltas em disputa
Contato com rivais em velocidade moderada nao cancela a contagem da volta nem impede a chegada; invalida somente o recorde fantasma daquela volta (nao foi limpa). Dano grave requer maior velocidade de fechamento. Ultrapassagens e pilotos rivais continuam sujeitos ao simulador simplificado de contato.

## 0.10.0 — Sakura Touge / visual JDM

O quarto circuito, Sakura Touge (indice 3), e a pista inicial. O menu foi
redesenhado com fotografia cinematografica tratada, sacadas japonesas e uma
identidade editorial em vermelho cereja, rosa sakura, perola, ameixa e ambar.
HUD e oficina 3D tiveram suas cores e composicoes harmonizadas.
As cerejeiras usam sprites autorais em um shader 2D, junto de petalas animadas,
rio, estrada, lanternas, templo e quatro portais torii.
O kart ganhou pintura Sakura Edition 86 em branco perola e vinho metalizado.

Controles de menu: Enter iniciar, T pista, L voltas, K rivais, D dificuldade,
H historico, B layout do teclado, F guia, N cores alternativas, -/+ volume.
Em corrida: WASD/setas dirigir, C piloto automatico, P pausa, R reiniciar,
M mapa inteiro. A opcao --circuit 3 --overview permite capturar a planta geral.

Ferramentas de arte: tools/make_sakura_canopy.py, tools/make_jdm_kart.py
e tools/make_jdm_menu.py. Os resultados gerados estao inclusos no pacote;
a compilacao e a execucao nao dependem de Python.
O editor de assets requer Pillow e NumPy, alem de fontes do Windows.
Veja docs/ART_DIRECTION_JDM.md para capturas e limitacoes.

## 0.10.1 - Menu JDM navegavel com mouse

Tela principal Sakura Edition agora recebe cliques em Iniciar Corrida, circuito, voltas, rivais, dificuldade, volume, historico, controles, guia, paleta e qualidade. As teclas originais continuam funcionando. Hit testing C++17 usa escala da janela SDL2 e framebuffer HiDPI. Teste deterministico: cometa_menu_interaction_tests.

## 0.11.0 - Kart Feel
- Segurar Espaco/Shift ou botao X no controle ativa derrapagem arcade ao dirigir acima de 22 km/h. Direcao progressiva, perda de aderencia e velocidade, pontos de drift e marcas visuais perto do kart.
- Piloto automatico ignora drift. Este e um recurso de dirigibilidade de kart, nao drift JDM.
- Testes novos: inicio/fim da derrapagem, maior resposta de angulo e reset. 17/17 testes Windows.

## 0.12.0 — World Tour: circuitos longos

Os quatro mapas foram reconstruidos geometricamente, com novos setores e
comprimento real aproximado de 2 a 2,8 km: Floresta 2203 m, Alpes 2539 m,
Litoral 2782 m, Sakura Touge 2472 m. Antes eram centenas de metros.
CPU e shader usam a mesma formula de curvas, com harmonic modes 2,3,4,5,7,9.
O piloto automatico utiliza antecipacao por distancia de pista e passou
doze condicoes de sol/chuva/neve sem colisao com as barreiras.
Minimapa e zoom geral agora se ajustam para as quatro extensoes.

Recordes usam PHOENIX_GHOST_V2 e prefixo record_long_v2, preservando os
arquivos anteriores, que nao correspondem mais a topologia atual.

Capturas reais: docs/screenshots/012-map-0.jpg, 012-map-1.jpg,
012-map-2.jpg e 012-map-3.jpg.

## 0.13.0 — Cup World Tour / Race & Repair

Campeonato de quatro etapas: Sakura, Floresta, Alpes, Litoral.
Uma volta longa por etapa (2,20–2,78 km), dois rivais e posicoes 10/7/4 pontos.
Climas sol, chuva, neve e sol. O mesmo kart carrega desgaste de pneus entre etapas.

No menu, pressione J ou clique em CAMPEONATO para comecar.
Ao completar uma etapa use 1 (conservar materiais, custo zero), 2 (reparo rapido:
1 material, 9 Wh, desgaste -16%) ou 3 (reparo completo: 2 materiais,
24 Wh, desgaste -44%). Orcamento inicial 4 materiais e 90 Wh.
Esc abre o menu, O retorna a corrida livre, J reinicia campeonato.
R reinicia etapa ainda em andamento.

O placar final fica em championship_history.csv, e resultados individuais
em race_history.csv no diretorio SDL2. Arquivos locais sem nuvem.
A campanha atual so continua enquanto o jogo permanece aberto; nao existe
recuperacao da etapa atual apos encerrar o processo.

QA automatizado, sem alterar dados pessoais:
cometa_game --championship --autopilot --auto-service --frames 62000 --no-vsync --data-dir ./qa-cup --quality 0

Em Windows, o executavel completou as quatro provas e gravou os dados:
37 pontos, P1 geral, 0 materiais, 42 Wh restantes. Testes CTest 19/19.
Capturas: docs/screenshots/013-menu.jpg e docs/screenshots/013-service.jpg.
