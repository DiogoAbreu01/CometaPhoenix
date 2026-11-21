# World Tour / Race & Repair — 0.13.0

## Como jogar
- No menu: J ou clique em CAMPEONATO.
- Etapas: Sakura Touge, Floresta, Alpes e Litoral, uma volta cada.
- Pontos por prova: 10/7/4 (colocacoes 1–3), vitorias desempatem.
- Climas: sol, chuva, neve, sol.
- Entre as etapas: 1 conservar; 2 reparo rapido (1 material, 9 Wh);
  3 reparo total (2 materiais, 24 Wh).
- Estoque inicial: 4 materiais, bateria 90 Wh.
- Desgaste acumulado passa da etapa anterior; servico restaura parte dele.
- ESC abre menu, O retorna a corrida livre, J reinicia campeonato.
- Finalizado: resultado gravado em championship_history.csv.
- Nao ha salvamento da campanha em andamento ao sair.

## Validacao do executavel Windows
Corrida simulada acelerada, com dispositivo de audio aberto e pasta QA:
- Sakura 139,55 s; P1; 10 pontos.
- Floresta 155,375 s; P1; 20 pontos totais.
- Alpes 265,192 s; P2; 27 pontos totais.
- Litoral 160,425 s; P1; 37 pontos totais.
- Geral P1; 37/31/16 pontos; 0 materiais; 42 Wh; desgaste 20,3%.
- 19/19 CTest Windows aprovados.
- Execucao E2E das quatro provas em 17,48s reais (simulacao acelerada).
- Resultados individuais e temporada salvos corretamente.

## Capturas reais
[Menu de selecao](screenshots/013-menu.jpg)
[Tela de reparos entre provas](screenshots/013-service.jpg)

## Limites
Kart racer indie 2D com oficina 3D, fisica arcade. Gamepad manual,
sessao longa em tempo real e acessibilidade precisam de homologacao final.
O campeonato ainda nao salva progresso intermediario.
