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
