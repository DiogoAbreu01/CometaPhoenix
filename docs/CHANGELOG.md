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
