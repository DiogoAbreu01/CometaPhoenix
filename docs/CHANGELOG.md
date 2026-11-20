# PHOENIX — histórico de entregas

## 0.9.3 — cronometragem de voltas
- HUD com tempo atual e melhor volta válida; contador não regride ao andar em sentido contrário.
- Saída da pista ou impacto invalida a tentativa; reparação reinicia a cronometragem sem somar volta pelo reboque. A volta percorrida pode ser contada sem gerar recorde válido.
- Testes de regressão: volta completa em sol, chuva e neve, reset de cronômetro e nenhuma volta falsa no box.
- Windows Release no Ryzen 5 5500 / RX 580 2048SP: 8/8 CTest aprovados. Smoke de corrida com 270 quadros medidos após 30 de aquecimento, Full HD, Ultra, chuva, sem VSync/áudio: 289,6 FPS; p95 3,53 ms, pico 40,82 ms (captura no último quadro). Não é teste de estabilidade prolongada.
- Linux: revalidação desta revisão pendente.
- BlueLab: fila exclusiva PHOENIX; Qwen local respondeu, mas recomendações imprecisas foram descartadas. Codex engineer bloqueado pela política; alterações implementadas via Desktop Commander. TITAN ainda não validado. IC e fila original preservadas.

## 0.9.2
Base nativa C++17, correção do piloto automático e chegada física ao box. Linux e Windows previamente compilados e testados. Tag `v0.9.2` preservada.

Pendências: novas pistas, assets mais realistas, impactos diferenciados, testes prolongados, apresentação acadêmica após homologação.

## 0.9.8 — corrida com objetivo, novos circuitos, materiais e impactos
- Três circuitos selecionáveis (T no menu): Floresta, Alpes, Litoral. Traçados analíticos próprios compartilhados por física e minimapa, replicados no shader; seis novos testes de volta completa, total nove combinações circuito/clima.
- Colisões por incidência normal e velocidade: raspagem, frontal leve, suspensão, roda individual, chassi crítico. Causa detalhada no histórico; localização visual no kart. Modelo heurístico didático, não crash test físico.
- Codex via fila isolada BlueLab, tarefa 5 concluída: materiais, clima e antialiasing dos shaders. Texturas reais de asfalto (difuso, roughness e normal, 512px mipmapped) com licença e origem em ASSETS_CC0.md. Malha 3D anterior mantida.
- L no menu seleciona 1, 3, 5 voltas válidas ou livre; fim da corrida, tempo total, reinício. `--laps N` seleciona objetivo para testes. Teste de contagem, término, reset e modo livre.
- Windows Release RX580: CTest 8/8. Full HD Ultra, chuva: Floresta 3600 frames, 3570 medidos, 425,6 FPS (inclui cena pós-chegada), 1 volta concluída sem danos, 41,89 s simulados. Litoral 7200 frames, 7170 medidos, 532,53 FPS, p95 2,126 ms, p99 2,185 ms, 2 voltas/120 s simulados sem danos, 13,63 s de execução real. Sem áudio e VSync, passo fixo. Estes números não representam teste de temperatura prolongado. Capturas e logs no diretório de evidências Windows. Atualização da verificação gráfica Linux no workflow; resultado ainda não confirmado nesta revisão.
- Infraestrutura isolada: TITAN 35B IQ2_XXS passou smoke test real, mas consulta técnica posterior interrompida por RAM. Não é requisito do executável. Codex operacional; IC preservada.
- Pendências: mesh 3D de maior fidelidade, áudio e avaliação manual/estabilidade prolongada, verificar CI Linux da versão, apresentação após homologação. Não declarar 1.0 final.

## 0.9.9 — MK-III: geometria mecânica e vista superior
- Codex, tarefa 8 da fila exclusiva BlueLab: gerador C++17 `tools/upgrade_kart.cpp` adiciona quatro rotores ventilados, quatro cubos com 20 fixadores, quatro molas com colares, 13 aletas de motor e dois ajustadores de cintos. 55.994 vértices, 92.924 triângulos (+31.172), preservando a malha original, limites e formato PHXM. Regeração conferida byte a byte pelo agente.
- Gerador C++17 `tools/bake_kart_sprite.cpp`: rasterização ortográfica com buffer de profundidade, normais interpoladas, iluminação e supersampling 2x para sprite RGBA 512x512. 106.635 pixels cobertos. Vista em corrida atualizada a partir da malha nova, sem custo de geometria adicional nessa cena.
- Oficina: sombras de contato sob as rodas, reflexos de estúdio aproximados, dano visual menos invasivo, seleção individual de pneus por quadrante, rotação A/D. Não é uma reconstrução exata da prancha conceitual nem renderização fotorrealista.
- Windows Release RX580 2048SP, Full HD Ultra: 8/8 CTest, teste reforçado de normais, limite de triângulos e transparência. Oficina 240 quadros (210 medidos), 661,197 FPS, p95 3,1805 ms, p99 3,4687 ms; corrida Litoral chuva 1200 quadros (1170 medidos), 536,003 FPS, p95 2,1307 ms, p99 2,2239 ms. Ensaios curtos, sem áudio/VSync, sem screenshot durante as medições reportadas. Sem teste térmico/manual completo.
- Capturas reais reduzidas em `docs/screenshots/099-garage-final.jpg` e `099-race.jpg`; originais no PC. Linux desta revisão, áudio e homologação final pendentes. TITAN 35B não acionado nesta rodada por instabilidade anterior; Codex/BlueLab efetivamente utilizados. IC preservada.

Para atualizar a vista superior após regenerar a malha (Linux ou toolchain C++17 equivalente no Windows):
```sh
c++ -std=c++17 -O2 tools/bake_kart_sprite.cpp -o build/bake_kart_sprite
./build/bake_kart_sprite game/assets/kart.mesh build/kart_sprite.rgba
cp build/kart_sprite.rgba game/assets/kart_sprite.rgba
```

## 0.9.10 — áudio C++17 e piso da oficina (20/11/2026)
- Síntese do motor revisada: harmônicos por wavetable, rotação em RPM, seis relações virtuais, corte breve na troca de marchas, desaceleração contínua, bloqueio de DC e limite de amplitude. Áudio procedural original; **não** é gravação de Ferrari.
- Novo módulo `game/ImpactAudio.hpp`: seis efeitos sintetizados em tempo real sem alocações na callback SDL (raspagem, pneu, suspensão, chassi, regeneração e chegada). Caixa postal atômica evita concorrência entre lógica e áudio; mute interrompe a reprodução dos efeitos.
- `tests/test_engine_audio.cpp` valida marcha, idle, aceleração, transições, saturação e mute. `tests/test_impact_audio.cpp` valida classificação, forma de onda, RMS, limites de amplitude, fim dos seis sons e segurança numérica.
- Oficina: textura CC0 **Concrete Floor 02** da Poly Haven, com mapas difuso, roughness e normal em 512 px, dentro do pacote offline. Fonte e licença em `docs/ASSETS_CC0.md`.
- Windows nativo com SDL2 e RX 580: 10/10 CTest, execução com áudio habilitado em corrida e garagem, shaders carregados sem falhas. Litoral Full HD Ultra chuva, 900 quadros, 870 medidos sem áudio/VSync: 534,40 FPS, p95 2,137 ms, p99 2,301 ms. Medição de garagem separada e Linux ainda em validação.
- Trabalho anterior da fila BlueLab/Codex (motor e concreto) foi preservado, revisado e integrado com efeitos por peça. IC intocada. Captura e benchmark não validam semelhança acústica com motor real nem substituem audição humana.

**Validação final da 0.9.10:** oficina isolada em 240 quadros (210 medidos), Full HD Ultra: 774,15 FPS; p95 1,738 ms, p99 1,776 ms. Litoral com chuva em 7200 quadros (7170 medidos), 120 s simulados: 532,63 FPS; p95 2,137 ms, p99 2,219 ms, máximo 2,822 ms, duas voltas, zero danos. Motor e efeito de garagem executaram também com dispositivo SDL de áudio habilitado (caminho do jogo sem `--benchmark`), sem erros reportados. Capturas reais: `docs/screenshots/0910-garage.jpg` e `0910-race.jpg`. A escuta subjetiva, compatibilidade Linux da revisão e partidas manuais prolongadas permanecem pendentes.

### Validacao adicional da 0.9.10 — audio e GPU
- Integração dos seis efeitos procedurais por componente, transição de reparo e chegada; corrigida saturação do filtro durante o impacto. Diagnóstico SDL confirma `AUDIO_DEVICE OPEN` no Windows conectado, mas não equivale a audição humana.
- 10/10 CTest Release aprovados após o ajuste. Execução de 180 quadros com áudio habilitado passou.
- RX 580 2048SP, Full HD Ultra, 360 quadros por cenário (330 medidos após 30 de aquecimento), sem VSync/áudio: Floresta 430,55 FPS (p95 2,552 ms); Alpes 419,42 FPS (p95 2,617 ms); Litoral 522,83 FPS (p95 2,160 ms); oficina 722,07 FPS (p95 1,622 ms). Ensaios curtos e não garantem estabilidade prolongada.
- Capturas reais: `docs/screenshots/0910-garage.jpg` e `0910-race.jpg`. Testes Linux e homologação final permanecem pendentes.

## 0.9.11 — fantasma da melhor volta
- Codex via BlueLab (tarefa 10), C++17: gravação da melhor volta válida em buffers limitados a 4096 poses, interpolação de posição e rotação, sem alocação por quadro na reprodução; corridas com dano ou saída de pista não geram fantasma. Circuito/reinício apagam o registro.
- Shader de corrida mostra o MK-III translúcido azul-ciano na trajetória gravada, tecla G alterna a exibição, HUD informa quando existe recorde. Não gera colisões. Fantasma armazenado apenas em memória nesta revisão.
- Windows Release, RX 580 2048SP, Full HD Ultra chuva, corrida com piloto automático: 3300 quadros (3270 medidos), uma volta concluída e fantasma presente após o primeiro recorde, zero dano; 430,291 FPS médios medidos, P95 2,514 ms, P99 2,581 ms. Sem VSync e som, 7,8 segundos reais; não é teste térmico prolongado. CTest 10/10.
- Captura real: docs/screenshots/0911-ghost-fullhd.jpg. Linux CI da revisão, validação de controles manuais e homologação final pendentes. TITAN 35B não foi executado nesta etapa por instabilidade anterior em consultas; BlueLab/Codex efetivamente utilizados. IC intocada.

## 0.9.12 — rodada 1: recordes no disco
Arquivo PHOENIX_GHOST_V1 com versão, circuito, clima, tempo, posições e ângulos; limite de 4096 amostras e 1 MiB, validação estrita e escrita via temporário. Persistência nativa C++17, diretório SDL_GetPrefPath; reload ao reiniciar, mudar pista/clima ou número de voltas. Benchmarks não gravam dados do usuário. CTest 10/10 Windows.

## 0.9.13 — rodada 2: rivais C++17
Codex BlueLab tarefa 11 criou RivalAI.hpp e testes determinísticos de distância de arco, três pistas, cinco climas e limitação de velocidade. Integração gráfica de dois karts coloridos, mini mapa, seleção via K no menu e --rivals N. 11/11 CTest Windows. RX580 Full HD Ultra chuva 5400 quadros (5370 medidos): 525,457 FPS, p95 2,1615ms, sem danos, 90s simulados, rivais chegaram a ~2,01 e 1,76 voltas. Rivais não colidem nem executam estratégias ofensivas. Captura real em docs/screenshots/0913-rivals.jpg.

## 0.9.14 — rodada 3: resultados + verificação completa
- Cabeçalho C++17 RaceResults.hpp: cálculo único de colocação, validação de dados e histórico CSV append-only (máximo 8 MiB). Registra conclusão ou abandono, pista, clima, voltas, rivais, posição, tempo, melhor volta e número de impactos.
- No HUD há colocação da corrida, e a tela de chegada exibe a classificação. Flags --data-dir e --no-vsync permitem QA real com arquivos isolados da configuração pessoal.
- CTest Windows Release: 12/12, incluindo simulador de adversários e teste de escrita/arquivo corrompido. Teste end-to-end via executável real, com áudio: 3600 quadros sem VSync em 720p; foram gravados race_history.csv (164 B) e record_c0_w1.phx (~186 kB) em pasta de QA. Resultado 1 volta válida em 41,892 s, 3ª posição com dois rivais, 0 danos.
- Smoke da chegada em RX580 Full HD Ultra, 3600 quadros (3570 medidos), chuva, dois rivais: 420,374 FPS, p95 2,5724 ms, p99 2,6292 ms. Teste acelerado sem VSync; não é partida longa/manual. Captura real docs/screenshots/0914-finish.jpg.
- Documento ROADMAP_1_0.md lista requisitos finais e prioridades futuras. Linux e Windows CI desta versão ainda precisam ser confirmados após o push. IC permaneceu intocada.

## 0.9.15 — melhorias de competicao, fisica e UX

- Codex/BlueLab tarefa 12: trafego, curvas por grip, ultrapassagem com deslocamento lateral de ate 0,23 unidade e dificuldade facil/normal/dificil.
- KartCollision: aproximacao de colisoes disco/disco e eventos de dano do jogador; rivais perdem velocidade na batida. Testes deterministas.
- TireDynamics: calor de pneus e freios, desgaste, fading de frenagem, grip e integridade de componentes. Piloto automatico ajustado para neve, sem regressao de trajetoria.
- Teclas de controle, volume ajustavel, gamepad SDL2, esquema de cor alternativo, HUD de temperatura/desgaste, historico de resultados e spray grafico local.
- Windows 15/15 CTest. Teste de estresse C++17: 30 minutos simulados em cada pista sem NaNs. Nao substitui sessao humana real nem benchmarking termico.
- RX580, 1080p Ultra, 360 quadros, captura: corrida chuva com rivais 313,574 FPS, menu 310,714, oficina 522,282 (medias medidas). Capturas reais em docs/screenshots/0915-*.jpg.
- Limitacoes: IA e contato simplificados, hardware gamepad nao testado, nenhuma prova de estabilidade termica longa, qualidade grafica ainda nao fotorrealista. Linux/Windows CI desta revisao pendente.
