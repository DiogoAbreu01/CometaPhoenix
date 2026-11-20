# Modelo físico e unidades — tudo **ilustrativo**

Nenhuma das aproximações abaixo substitui dinâmica multicorpos, elementos finitos ou medição experimental. Dados de resistência são parâmetros da simulação, **não** valores recomendados para um kart físico.

## Carga lateral durante corrida

- Velocidade: `v = velocidade_kmh / 3.6` em m/s.
- Pista circular de raio equivalente `R = 65 m`; aceleração lateral `a = v²/R` em m/s².
- Vibração ilustrativa: `a_pista = 0.35 a + 0.65 abs(sin(4.3 t))`.
- Força aplicada (distribuição heurística): `F = massa * a_pista`, usando `massa = 170 kg`.
- Tensão média por peça `σ = (F * fracao_de_carga / area_efetiva) / 1e6` em MPa.

Essa tensão equivalente **não captura concentração local**, flambagem, contato entre peças nem soldas. Exigir inspeção de engenharia para usar valores fora da simulação.

## Colisão

- Mudança de velocidade `Δv` e duração do impacto `Δt` são escolhidas pelo cenário.
- Força equivalente: `F_impacto = massa * Δv / Δt` (N).
- `σ_impacto = F_impacto * fracao_local / area_efetiva / 1e6` (MPa).
- Energia cinética equivalente dissipada: `E = 0.5 * massa * Δv²` (J), **não a energia necessariamente absorvida pela peça selecionada**.
- Razão `r=σ_impacto/σ_admissivel`.
- Probabilidade heurística da falha: `p = limitar(0.12 + 0.54*(r - 0.40), 0.02, 0.99)`.
- Para `r >= 1.35`, o cenário considera fratura; abaixo desse valor, sorteio `std::mt19937` pode produzir dano superficial ou fissura. Magnitudes de dano também seguem uma regra heurística e a seed.

Sem calibração, **esses números não podem prever dano real**.

## Fadiga e desgaste

A cada `dt`, estimamos amplitude `σa ≈ 0.42σ`, ciclos por irregularidade `ciclos = dt * v / 3.5` e incremento de dano `ΔD = (σa / σlimite_fadiga)^4 * ciclos / 100000`.

É um indicador de desgaste acumulado **inspirado** na ideia de Palmgren–Miner; **não** representa uma curva S–N obtida em laboratório. Quando o indicador fica alto, cresce uma probabilidade de dano. O cenário `fadiga` injeta uma deformação deliberadamente aos 8 s para demonstração.

## Reparo e validação

1. Escolher mecanismo compatível com material, modo de dano e integridade mínima.
2. Confirmar e reservar energia/selante/peça de reposição.
3. Atualizar a integridade gradualmente ao longo de `tempoTotalS`; a cura da carenagem é parcial (teto 94%).
4. Não zerar fadiga/desgaste em selagem nem em liga com memória. Na **substituição**, peça nova começa sem fadiga acumulada.
5. Validar integridade `>= 0.77` (chassi), `>= 0.72` (carenagem) ou `>= 0.73` (demais) e fadiga `< 1.25` antes de voltar à pista.

Os limites de segurança acima são somente regras internas da aplicação. **Não são limites normativos nem adequados para autorizar uso físico do kart.**

## Regeneração não é mágica

Microcápsulas podem inspirar selagem de pequenas fissuras em compósitos. Ligas com memória de forma recuperam deformações dentro de limites, mas não restauram automaticamente vida em fadiga nem soldam rupturas. Substituição robótica (aqui, especulativa) é troca de peça. A fratura do chassi principal leva ao abandono nesta versão.
