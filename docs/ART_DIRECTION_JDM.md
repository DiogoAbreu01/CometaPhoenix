# Sakura Edition — Art Direction

Visao: corrida independente em serra japonesa, clima de hanami e JDM noturno.
Paleta: indigo #101326, ameixa #251c37, flor #f5a0b7, cereja #b74765,
perola #f7eee8, ambar #ffc482 e pinheiros #183b37.

## Capturas reais no OpenGL

- [Menu JDM](screenshots/jdm_menu_v010.jpg)
- [Corrida Sakura Touge](screenshots/sakura_race_v010.jpg)
- [Mapa completo](screenshots/sakura_overview_v010.jpg)
- [Garagem 3D](screenshots/jdm_garage_v010.jpg)
- [Arte da capa sem UI](screenshots/sakura_jdm_menu_art.jpg)
- [Canopy floral](screenshots/sakura_canopy.png)
- [Kart Sakura Edition](screenshots/kart_jdm_livery.png)

## Implementacao

Menu RGB, textura canopy e livery kart RGBA gerados a partir de fontes
originais do projeto. Estrada e paisagem sao desenhadas em GLSL OpenGL 3.3,
com detalhes dos cenarios distribuídos por coordenadas de mundo.
Todos os assets ficam no repositorio, assim como o gerador Python/Pillow.

## Limites

O jogo continua com karts em visao de cima. Os tres circuitos antigos
mantiveram a topologia, mas compartilham o novo front-end e iluminacao.
Evolucoes futuras: refazer a geometria dos mapas antigos e criar hairpins
mais extensos, testar com jogadores e, se aprovado, desenvolver carros JDM.
