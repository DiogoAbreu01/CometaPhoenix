# PHOENIX — lacunas reais para a versão 1.0

Estado verificado: Windows C++17, OpenGL 3.3 / SDL2, três circuitos, cinco condições climáticas, oficina 3D, dez módulos de regressão, 12 testes no CTest, fantasma e recorde persistentes, dois rivais procedurais e resultados locais.

## P0 — validação antes de chamar o jogo de pronto
- Confirmar as Actions Linux e Windows da revisão corrente, incluindo shaders reais com Mesa no Linux e bundle Windows com SDL2.
- Fazer sessões manuais de 30–60 minutos, usando teclado real e áudio audível, com rodadas nos três circuitos e cinco climas. Testar mudança de resolução, tela cheia, pausa, volta inválida, colisão, reparo e término.
- Revisar a UX de menu, navegação, controle, mensagens de abandono e retorno à pista; incluir indicação visível de onde ficam os recordes e o histórico.
- Avaliar p95/p99 do tempo de quadro em hardware modesto, especialmente entradas do box, trocas de câmera, tela cheia, chuva e oficina com malha 3D.
- Testar builds em duas distribuições Linux com drivers reais (Mesa e GPU dedicada) e no Windows MSVC, sem depender apenas de execução CI.
- Documentar os controles e preparar apresentação acadêmica somente depois que o jogo estiver homologado.

## P1 — prioridade de produto
- Rivais atualmente seguem a linha central com velocidades fixas por clima; implementar tomada de decisão, ultrapassagem e colisões físicas reais entre karts. Não alegar IA competitiva avançada antes disso.
- Criar dificuldade ajustável, grid de largada equilibrado, classificação completa, penalidades/tempo perdido no box e feedback de progresso da corrida.
- Melhorar física do veículo: aderência variável por superfície e estado de cada pneu, frenagem, transferência de massa, temperatura, desgaste e calibração com telemetria.
- Polir artes: carroceria e cockpit 3D, faróis, partículas, modelos de pista/vegetação, sombras de contato coerentes, iluminação e chuva; a renderização ainda não é fotorrealista.
- Melhorar som procedimental: validar a resposta ao acelerador, deslizamento e frenagem em escuta real e no Linux, sem alegar ser gravação de Ferrari.
- Adicionar menu de resultados anteriores e recordes filtrados por pista/clima, opção de controle e acessibilidade, gamepad e controles reconfiguráveis.

## P2 — expansão
- Replay de corrida inteira e ghost exportável/importável com checksum e versionamento.
- Mais pistas e variantes (noturno e piso com composições diferentes), AI com diferentes perfis, campeonato e progressão.
- Telemetria e benchmark com gráficos e captura de frame time por cena, além de QA automatizado de gameplay.

## Limites atuais
- O motor de corrida é em C++17. O render usa GLSL (shader OpenGL); SDL2 cuida apenas de entrada, áudio e janela.
- O fantasma é persistido por pista e clima, mas não há interface para listar/excluir recordes.
- Rivais não colidem com o jogador. O resultado CSV é append-only e só contém corridas concluídas ou abandonadas.
- Benchmarks sem VSync não medem experiência normal a 60 Hz; CTest e smoke não substituem jogo manual.
- TITAN 35B IQ2_XXS passou smoke test anteriormente, mas sua estabilidade em consultas técnicas permanece limitada pela memória. A liberação do projeto não depende dele.
