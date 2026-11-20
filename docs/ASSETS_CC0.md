# Materiais fotografados — licenças

Asphalt 02, de Rob Tuytel (Poly Haven), CC0. Página: https://polyhaven.com/a/asphalt_02 ; licença: https://polyhaven.com/license . Mapas Diffuse, Rough e Normal GL 1K obtidos pela API oficial https://api.polyhaven.com/files/asphalt_02, conferidos por tamanho e MD5 de origem. Difuso fa19772d4817754c3efab708c651f5a8; rugosidade ec80a2929797c1b3ffc121c4f665585a; normal 338da8de636ea36133170578cac82e8e. Redimensionados para 512x512: RGB difuso + A rugosidade em asphalt_photo.rgba, normal RGB em asphalt_normal.rgb. Imagens lidas de cima para baixo. Dados locais, sem dependência de rede no jogo. Mipmaps e repetição em OpenGL, amostragem com gradientes explícitos para bordas e escala. 1.75 MiB em disco. Não é uma digitalização de toda a pista ou do kart.

## Concrete Floor 02 — piso fotográfico da oficina

- Autor: Poly Haven (material **Concrete Floor 02**), licença CC0: https://polyhaven.com/a/concrete_floor_02 e https://polyhaven.com/license
- Origem: https://api.polyhaven.com/files/concrete_floor_02 . Maps `Diffuse`, `Rough` e `nor_gl`, JPG de 1K.
- Verificação MD5 (fornecido na API): Diffuse `1cfee53d2f82fa59726ab7ae6a2e8c09`, Rough `653d60d0db1872a49138455cf31c8326`, nor_gl `c2065fd81f571f4e39163ea5661128fb`.
- Conversão offline para 512×512: `game/assets/concrete_photo.rgba` (difuso RGB + rugosidade no alfa) e `concrete_normal.rgb` (normal GL). Bytes RGBA/RGB, repetição e mipmaps por OpenGL 3.3. Materiais fazem parte do pacote offline; jogo continua C++17 nativo.
- Efeito: textura fotográfica microestruturada sob a plataforma, mantendo marcas de piso, sombra de contato e iluminação artificial. Não é reflexo por ray tracing nem reconstrução fotográfica da oficina inteira.
