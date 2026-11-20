# Operação PHOENIX — checkpoint 20/11/2026

## Escopo e rastreabilidade
Reaberto o PDF ECOP13A_LABS12_14_PROJETO_COMETA_LIVRE (2 páginas) e conferido o inventário dos ZIPs originais e últimos pacotes. Base canônica: núcleo nativo C++ de desktop incluído na entrega web 0.6 + recursos 3D e pista da edição web 0.8. A edição WASM fica como referência histórica, não como segunda entrega. Os arquivos originais não foram alterados. Não houve leitura integral linha a linha de todos os laboratórios.

| Critério (p.2) | Evidência |
|---|---|
| Template em dois tipos | Historico<EventoDano>, Historico<AmostraTelemetria>, limitar<int/double>; testaTemplates |
| STL contêiner + algoritmo | vector + sort, find_if, all_of e max_element, diagnóstico/validação |
| Organização | Simulacao, Game, estratégias de reparo, KartMesh, EngineAudio |
| Criatividade | dano e regeneração com recursos; pista climática; oficina 3D |
| C++17, normal e caso-limite | CTest; cenários normal, fratura crítica, falta de recurso, reset determinístico |
| README breve | README.md, instruções e ferramentas |

## Alterações realizadas
- Remoção dos quatro quadros de cutscene e seu carregamento na inicialização.
- Pista ampliada com fórmula de raio coincidente com shader; piloto automático usa tangente da curva. Mapa em M.
- Mesh PHXM com 32.946 vértices e 61.752 triângulos, derivada do GLB local da v0.8. 1 desenho indexado na oficina. Sprite 2D oriundo da malha. O modelo artístico hospedado do Higgsfield não é o arquivo usado em tempo real.
- Som original V12 aproximado, sintetizado por wavetable em callback SDL C++; giro ajustado à velocidade/marcha. Não é captura real Ferrari e não foi ouvido em hardware. Não há áudio de colisão nesta versão nativa.
- Cache de uniform, buffers de HUD reaproveitados, max_element em lugar de cópia/ordenação por quadro, framebuffer com profundidade em ambas as cenas, quatro perfis, descarte de amostra do sprite fora da área do kart.
- Testes de todos os nove componentes, dano crítico no teclado, retorno após reparo, reset de semente, integridade dos assets e ordenação de risco.

## Testes e limites
Linux x86_64, GCC/CMake Release, SDL2 runtime, Xvfb OpenGL por software. Capturas reais no diretório evidencias; não são capturas do computador do usuário. Benchmark 1280x720: 180 quadros, 30 aquecimento (oficina Ultra: 120/30 após interrupção do primeiro ensaio). glFinish sincroniza antes da medição; FPS/p95/p99 não equivalem a GPU física. Os perfis baixos tiveram comportamentos não monotônicos na oficina: investigar em GPU real. Os parâmetros estruturais são didáticos e a pista/telemetria CLI usam escalas aproximadas. Não é AAA nem ray tracing por hardware.

## Operação remota
Desktop Commander conectado a Windows; Ryzen 5 5500 e RX 580 2048SP reportados. Codex CLI 0.170.0 autenticado; executou revisão independente dos requisitos e checagem read-only de toolchain/ZIP. Resultado: compilador/MSVC/MinGW/clang e SDL2 não encontrados nos caminhos consultados; ZIP dos fontes não localizado em Downloads nem no workspace isolado. TITAN não identificado. Transferência automática entre o sandbox e o PC não disponível pelas ferramentas usadas. Nenhum jogo rodou no PC do usuário; nenhum benchmark dessa GPU; sem compilação Windows. Nenhum repositório Phoenix identificado na busca GitHub; nenhuma publicação efetuada. IC não alterada/iniciada. Nenhum agente fica executando após este checkpoint.

## Próximo gate obrigatório
Copiar ZIP de fonte ao Windows, disponibilizar compilador e SDL2, compilar e executar CTest, rodar BENCHMARK_WINDOWS.ps1 e jogar/manual testar clima, direção, dano, reparo e áudio. Repetir benchmarks e comparar percentis em GPU real antes de declarar otimização ou homologação. Apresentação deliberadamente pendente por solicitação do usuário.
