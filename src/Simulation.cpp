#include "cometa/Simulation.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace cometa {
namespace {
constexpr double PI = 3.14159265358979323846;
constexpr double RAIO_PISTA_M = 65;
constexpr double MASSA_KG = 170; // kart + piloto: hipotese ilustrativa

std::string escaparJson(const std::string& s) {
    std::string saida;
    for (char c : s) {
        switch (c) {
        case '\\': saida += "\\\\"; break;
        case '"': saida += "\\\""; break;
        case '\n': saida += "\\n"; break;
        case '\r': saida += "\\r"; break;
        case '\t': saida += "\\t"; break;
        default: if (static_cast<unsigned char>(c) >= 32) saida += c;
        }
    }
    return saida;
}
std::string escaparCsv(const std::string& s) {
    std::string r = "\"";
    for (char c : s) { if (c == '"') r += '"'; r += c; }
    return r + '"';
}
} // namespace

const char* nome(Fase fase) noexcept {
    switch (fase) {
    case Fase::Corrida: return "CORRIDA";
    case Fase::Alerta: return "ALERTA";
    case Fase::RetornoBox: return "RETORNO_BOX";
    case Fase::Diagnostico: return "DIAGNOSTICO";
    case Fase::Reparando: return "REPARANDO";
    case Fase::Validacao: return "VALIDACAO";
    case Fase::Abandono: return "ABANDONO";
    }
    return "DESCONHECIDA";
}

const char* nome(TipoReparo tipo) noexcept {
    switch (tipo) {
    case TipoReparo::Nenhum: return "NENHUM";
    case TipoReparo::Microcapsulas: return "MICROCAPSULAS";
    case TipoReparo::MemoriaForma: return "MEMORIA_FORMA";
    case TipoReparo::SubstituicaoRobotica: return "SUBSTITUICAO_ROBOTICA";
    }
    return "NENHUM";
}

bool Microcapsulas::serve(const Componente& p) const noexcept {
    return p.tecnologia == TipoReparo::Microcapsulas &&
           p.material == Material::Composito && p.integridade >= 0.45;
}
PlanoReparo Microcapsulas::criarPlano(const Componente& p) const {
    if (!serve(p)) throw std::invalid_argument("microcapsulas nao servem neste dano");
    PlanoReparo r;
    r.tipo = tipo(); r.componente = p.id;
    r.integridadeInicial = p.integridade;
    r.integridadeAlvo = std::min(0.94, p.integridade + (1.0 - p.integridade) * 0.80);
    r.tempoTotalS = 7.5; r.energiaTotalWh = 11.0;
    r.agenteMl = 14.0;
    r.explicacao = "Selagem de microtrincas no composito; cura parcial, nao restaura fadiga.";
    return r;
}

bool MemoriaDeForma::serve(const Componente& p) const noexcept {
    return p.tecnologia == TipoReparo::MemoriaForma &&
           p.material == Material::LigaMemoria && p.integridade >= 0.52;
}
PlanoReparo MemoriaDeForma::criarPlano(const Componente& p) const {
    if (!serve(p)) throw std::invalid_argument("liga de memoria alem do limite recuperavel");
    PlanoReparo r;
    r.tipo = tipo(); r.componente = p.id;
    r.integridadeInicial = p.integridade;
    r.integridadeAlvo = std::min(0.88, p.integridade + (1.0 - p.integridade) * 0.62);
    r.tempoTotalS = 9.0; r.energiaTotalWh = 16.0;
    r.explicacao = "Aquecimento controlado reduz deformacao, mas nao elimina dano por fadiga.";
    return r;
}

bool SubstituicaoRobotica::serve(const Componente& p) const noexcept {
    return !p.estrutural && p.id != "chassi";
}
PlanoReparo SubstituicaoRobotica::criarPlano(const Componente& p) const {
    if (!serve(p)) throw std::invalid_argument("troca de estrutura principal nao autorizada");
    PlanoReparo r;
    r.tipo = tipo(); r.componente = p.id;
    r.integridadeInicial = p.integridade; r.integridadeAlvo = 1.0;
    r.tempoTotalS = 12; r.energiaTotalWh = 24; r.pecas = 1;
    r.explicacao = "Substituicao robotica especulativa: peça nova, nao regeneracao de metal.";
    return r;
}

Simulacao::Simulacao(std::string cenario, std::uint32_t seed)
    : cenario_(std::move(cenario)), rng_(seed) {
    if (cenario_ != "normal" && cenario_ != "impacto_leve" &&
        cenario_ != "impacto_critico" && cenario_ != "sem_recurso" &&
        cenario_ != "fadiga") {
        throw std::invalid_argument("cenario invalido: normal, impacto_leve, impacto_critico, sem_recurso ou fadiga");
    }
    // Catalogo editavel de parametros DIDATICOS: nao sao propriedades certificadas.
    componentes_ = {
        {"chassi", "Chassi tubular", Material::Aco, .00042, 180, 80, 1, 0, 0, 0, 0,
         TipoReparo::Nenhum, 0, .34, 0, true},
        {"carenagem", "Carenagem composita", Material::Composito, .00026, 75, 35, 1, 0, 0, 0, 18,
         TipoReparo::Microcapsulas, .2, .61, .2, false},
        {"suspensao", "Suspensao inteligente", Material::LigaMemoria, .00030, 110, 60, 1, 0, 0, 0, 0,
         TipoReparo::MemoriaForma, -.35, .24, -.1, false},
        {"pneu_de", "Pneu dianteiro esquerdo", Material::Borracha, .00048, 50, 28, 1, 0, 0, 0, 0,
         TipoReparo::SubstituicaoRobotica, -.53, .11, -.29, false},
        {"pneu_dd", "Pneu dianteiro direito", Material::Borracha, .00048, 50, 28, 1, 0, 0, 0, 0,
         TipoReparo::SubstituicaoRobotica, -.53, .11, .29, false},
        {"pneu_te", "Pneu traseiro esquerdo", Material::Borracha, .00048, 50, 28, 1, 0, 0, 0, 0,
         TipoReparo::SubstituicaoRobotica, .56, .11, -.29, false},
        {"pneu_td", "Pneu traseiro direito", Material::Borracha, .00048, 50, 28, 1, 0, 0, 0, 0,
         TipoReparo::SubstituicaoRobotica, .56, .11, .29, false},
        {"freio", "Sistema de freio", Material::MetalMecanico, .00034, 140, 70, 1, 0, 0, 0, 0,
         TipoReparo::SubstituicaoRobotica, -.38, .21, .18, false},
        {"motor", "Motor", Material::MetalMecanico, .00060, 170, 90, 1, 0, 0, 0, 0,
         TipoReparo::SubstituicaoRobotica, .5, .40, 0, false},
    };
    if (cenario_ == "sem_recurso") { recursos_.agenteMl = 0; recursos_.pecasReposicao = 0; }
    capturar();
}

Componente& Simulacao::buscar(const std::string& id) {
    auto it = std::find_if(componentes_.begin(), componentes_.end(),
                           [&](const Componente& p) { return p.id == id; });
    if (it == componentes_.end()) throw std::invalid_argument("componente nao encontrado: " + id);
    return *it;
}
const Componente& Simulacao::componente(const std::string& id) const {
    auto it = std::find_if(componentes_.begin(), componentes_.end(),
                           [&](const Componente& p) { return p.id == id; });
    if (it == componentes_.end()) throw std::invalid_argument("componente nao encontrado: " + id);
    return *it;
}

void Simulacao::registrarDano(Componente& p, const std::string& causa, double perda,
                            double tensao, double probabilidade, double energiaJ) {
    perda = limitar(perda, 0.0, p.integridade);
    if (perda < 1e-6) return;
    p.integridade = limitar(p.integridade - perda, 0.0, 1.0);
    p.tensaoMPa = tensao;
    EventoDano e;
    e.id = ++sequencia_; e.segundo = tempo_; e.componente = p.id;
    e.causa = causa; e.tensaoMPa = tensao; e.limiteMPa = p.limiteMPa;
    e.energiaImpactoJ = energiaJ; e.deltaIntegridade = perda;
    e.probabilidade = probabilidade; e.fatal = p.estrutural && p.integridade < .55;
    eventos_.registrar(e);
    ultimoEvento_ = p.nome + ": " + causa;
    if (fase_ == Fase::Corrida) mudarFase(Fase::Alerta);
    capturar();
}

void Simulacao::aplicarFadiga(Componente& p, double dt) {
    // Dano cumulativo estilo Palmgren-Miner, calibracao arbitraria. N_ref = 1e5 ciclos.
    const double amplitude = p.tensaoMPa * .42;
    const double razao = amplitude / p.fadigaLimiteMPa;
    const double ciclos = dt * (velocidadeKmh_ / 3.6) / 3.5; // irregularidades a cada 3.5 m
    const double incremento = std::pow(razao, 4) * ciclos / 100000.;
    p.fadiga += incremento;
    p.desgaste = limitar(p.desgaste + dt * (p.id.find("pneu_") == 0 ? .00014 : .000025), 0.0, 1.0);
    if (p.fadiga > 1.0 && p.integridade > 0.5) {
        const double probabilidade = limitar((p.fadiga - 1) * .08, 0.0, 0.40);
        std::uniform_real_distribution<double> sorteio(0., 1.);
        if (sorteio(rng_) < probabilidade * dt) {
            registrarDano(p, "fadiga acumulada (modelo probabilistico)", .12,
                          p.tensaoMPa, probabilidade, 0);
        }
    }
}

void Simulacao::calcularCargas(double dt) {
    // Simplificacao: aceleracao lateral v^2/r + vibracao da pista.
    const double v = velocidadeKmh_ / 3.6;
    const double aceleracaoLateral = v * v / RAIO_PISTA_M;
    aceleracaoG_ = aceleracaoLateral / 9.80665;
    const double aceleracaoPista = aceleracaoLateral * .35 + 0.65 * std::abs(std::sin(tempo_ * 4.3));
    const double cargaN = MASSA_KG * aceleracaoPista;
    for (auto& p : componentes_) {
        double fator = p.id == "chassi" ? .18 : p.id == "suspensao" ? .13 : .08;
        // Mpa: F(N)/area(m2)/1e6. Nao equivale a FEA nem a tensao local real.
        p.tensaoMPa = cargaN * fator / p.areaM2 / 1e6;
        aplicarFadiga(p, dt);
    }
    temperaturaC_ = limitar(temperaturaC_ + dt * ((velocidadeKmh_ > 45 ? .13 : -.07)), 30., 120.);
}

void Simulacao::provocarImpacto(const std::string& id, double deltaVelocidadeMs, double duracaoS) {
    if (deltaVelocidadeMs <= 0 || duracaoS <= 0 ||
        !std::isfinite(deltaVelocidadeMs) || !std::isfinite(duracaoS)) {
        throw std::invalid_argument("impulso exige velocidade e duracao positivas e finitas");
    }
    if (fase_ == Fase::Abandono) throw std::logic_error("kart abandonou");
    Componente& p = buscar(id);
    // F = m delta-v / delta-t. Porcao de carga da colisao sobre p: heuristica.
    const double fracao = id == "chassi" ? .85 : id == "carenagem" ? .52 : .40;
    const double forcaN = MASSA_KG * deltaVelocidadeMs / duracaoS * fracao;
    const double tensaoMPa = forcaN / p.areaM2 / 1e6;
    const double razao = tensaoMPa / p.limiteMPa;
    const double energiaJ = .5 * MASSA_KG * deltaVelocidadeMs * deltaVelocidadeMs;
    const double probabilidade = limitar(.12 + .54 * (razao - .40), .02, .99);
    std::uniform_real_distribution<double> sorteio(0.0, 1.0);
    const double r = sorteio(rng_);
    const bool fratura = razao >= 1.35 || r < probabilidade;
    const double perda = fratura
        ? limitar(.19 + .25 * std::max(0., razao - .6) + .08 * r, .20, .88)
        : limitar(.04 + .06 * razao, .02, .16);
    registrarDano(p, fratura ? "colisao: fissura/fratura por tensao" : "colisao: dano superficial",
                  perda, tensaoMPa, probabilidade, energiaJ);
}

void Simulacao::mudarFase(Fase fase) {
    fase_ = fase;
    faseDesde_ = tempo_;
    if (fase == Fase::RetornoBox) { controleBoxExterno_ = false; chegouBoxExterno_ = false; }
}

bool Simulacao::validarSeguro() const {
    // Regras de seguranca adicionais ao limiar de reparabilidade.
    return std::all_of(componentes_.begin(), componentes_.end(), [](const Componente& p) {
        const double minimo = p.estrutural ? .77 : (p.id == "carenagem" ? .72 : .73);
        return p.integridade >= minimo && p.fadiga < 1.25;
    });
}

void Simulacao::diagnosticar() {
    auto risco = rankingRisco();
    const Componente& pior = risco.front();
    std::ostringstream msg;
    msg << pior.nome << " | integridade " << std::fixed << std::setprecision(1)
        << pior.integridade * 100 << "% | pico " << pior.tensaoMPa
        << " MPa / limite " << pior.limiteMPa << " MPa. ";
    const auto& dados = eventos_.itens();
    auto ev = std::find_if(dados.rbegin(), dados.rend(), [&](const EventoDano& e) {
        return e.componente == pior.id;
    });
    if (ev != dados.rend()) msg << "Causa observada: " << ev->causa << ". ";
    msg << "Modelo aproximado, sem elementos finitos; confianca moderada.";
    diagnostico_ = msg.str();
}

bool Simulacao::iniciarReparo() {
    if (fase_ != Fase::Diagnostico) return false;
    const auto ranking = rankingRisco();
    const Componente& p = ranking.front();
    Microcapsulas caps; MemoriaDeForma sma; SubstituicaoRobotica robo;
    const EstrategiaReparo* estrategia = nullptr;
    if (caps.serve(p)) estrategia = &caps;
    else if (sma.serve(p)) estrategia = &sma;
    else if (robo.serve(p)) estrategia = &robo;
    if (!estrategia) {
        diagnostico_ += " IMPOSSIVEL: fratura estrutural nao e autocuravel.";
        mudarFase(Fase::Abandono); capturar(); return false;
    }
    PlanoReparo candidato = estrategia->criarPlano(p);
    if (candidato.energiaTotalWh > recursos_.bateriaWh ||
        candidato.agenteMl > recursos_.agenteMl ||
        candidato.agenteMl > p.microcapsulasMl ||
        candidato.pecas > recursos_.pecasReposicao) {
        diagnostico_ += " IMPOSSIVEL: recurso insuficiente no box.";
        mudarFase(Fase::Abandono); capturar(); return false;
    }
    reparo_ = candidato;
    // Reserva recursos atomicamente no inicio: nao ha duplo gasto.
    recursos_.bateriaWh -= reparo_.energiaTotalWh;
    recursos_.agenteMl -= reparo_.agenteMl;
    recursos_.pecasReposicao -= reparo_.pecas;
    buscar(p.id).microcapsulasMl -= reparo_.agenteMl;
    mudarFase(Fase::Reparando);
    capturar();
    return true;
}

void Simulacao::atualizarReparo(double dt) {
    if (reparo_.tipo == TipoReparo::Nenhum) throw std::logic_error("reparo nao iniciado");
    reparo_.tempoDecorridoS = std::min(reparo_.tempoTotalS, reparo_.tempoDecorridoS + dt);
    const double fracao = reparo_.tempoDecorridoS / reparo_.tempoTotalS;
    Componente& p = buscar(reparo_.componente);
    p.integridade = limitar(reparo_.integridadeInicial +
                           (reparo_.integridadeAlvo - reparo_.integridadeInicial) * fracao,
                           0., 1.);
    if (reparo_.tempoDecorridoS >= reparo_.tempoTotalS - 1e-9) {
        if (reparo_.tipo == TipoReparo::SubstituicaoRobotica) {
            p.desgaste = 0; p.fadiga = 0; // PECA NOVA, nao cura de fadiga antiga
        }
        diagnostico_ += " Reparo finalizado: " + std::string(nome(reparo_.tipo));
        mudarFase(Fase::Validacao);
        capturar();
    }
}

void Simulacao::definirVelocidadeExterna(double kmh) {
    if (!std::isfinite(kmh) || kmh < 0 || kmh > 160) throw std::invalid_argument("velocidade externa invalida");
    controleVelocidadeExterna_ = true;
    velocidadeKmh_ = kmh;
}

void Simulacao::definirChegadaBoxExterna(bool chegou, double kmh) {
    if (fase_ != Fase::RetornoBox) throw std::logic_error("retorno ao box nao iniciado");
    if (!std::isfinite(kmh) || kmh < 0 || kmh > 160) throw std::invalid_argument("velocidade de reboque invalida");
    controleBoxExterno_ = true;
    chegouBoxExterno_ = chegou;
    velocidadeKmh_ = kmh;
}

void Simulacao::avancar(double dt) {
    if (!(dt > 0.0 && dt <= .2) || !std::isfinite(dt))
        throw std::invalid_argument("dt deve estar entre 0 e 0.2 s, finito");
    const Fase anterior = fase_;
    tempo_ += dt;
    if (fase_ == Fase::Corrida) {
        if (!controleVelocidadeExterna_) velocidadeKmh_ = 46 + 7 * std::sin(tempo_ * .26);
        distancia_ += velocidadeKmh_ / 3.6 * dt;
        voltas_ = static_cast<int>(distancia_ / (2 * PI * RAIO_PISTA_M));
        calcularCargas(dt);
        if (!impactoExecutado_ && tempo_ >= 8 && cenario_ != "normal") {
            impactoExecutado_ = true;
            if (cenario_ == "impacto_critico") provocarImpacto("chassi", 15., .013);
            else if (cenario_ == "fadiga") {
                Componente& p = buscar("suspensao");
                p.fadiga = .75;
                registrarDano(p, "fadiga da suspensao + deformacao em curva", .31,
                              p.limiteMPa * 1.06, .38, 0);
            } else provocarImpacto("carenagem", 12.0, .040);
        }
    } else if (fase_ == Fase::Alerta) {
        velocidadeKmh_ = std::max(15., velocidadeKmh_ - 30. * dt);
        if (tempo_ - faseDesde_ >= 1.0) mudarFase(Fase::RetornoBox);
    } else if (fase_ == Fase::RetornoBox) {
        if (!controleBoxExterno_) {
            velocidadeKmh_ = std::max(0., velocidadeKmh_ - 9. * dt);
            distancia_ += velocidadeKmh_ / 3.6 * dt;
        }
        if (controleBoxExterno_ ? chegouBoxExterno_ : tempo_ - faseDesde_ >= 3.) {
            velocidadeKmh_ = 0; mudarFase(Fase::Diagnostico); diagnosticar(); capturar();
        }
    } else if (fase_ == Fase::Diagnostico) {
        if (tempo_ - faseDesde_ >= 1.5) iniciarReparo();
    } else if (fase_ == Fase::Reparando) {
        atualizarReparo(dt);
    } else if (fase_ == Fase::Validacao) {
        if (tempo_ - faseDesde_ >= 1.5) {
            if (validarSeguro()) { diagnostico_ += " Validacao OK: retorno seguro a pista."; mudarFase(Fase::Corrida); }
            else { diagnostico_ += " Validacao FALHOU: integridade/fadiga insegura."; mudarFase(Fase::Abandono); }
            capturar();
        }
    }
    if (fase_ != anterior) capturar();
    if (tempo_ - amostraDesde_ >= .25 - 1e-8) {
        AmostraTelemetria a;
        a.segundo = tempo_; a.pistaRad = std::fmod(distancia_ / RAIO_PISTA_M, 2 * PI);
        a.velocidadeKmh = velocidadeKmh_; a.temperaturaC = temperaturaC_;
        a.aceleracaoG = aceleracaoG_; a.fase = fase_;
        telemetria_.registrar(a);
        amostraDesde_ = tempo_;
        capturar();
    }
}

void Simulacao::simular(double segundos, double dt) {
    if (!std::isfinite(segundos) || segundos < 0 || segundos > 3600)
        throw std::invalid_argument("duracao invalida");
    if (!(dt > 0 && dt <= .2) || !std::isfinite(dt))
        throw std::invalid_argument("dt invalido");
    for (int passos = static_cast<int>(std::ceil(segundos / dt)); passos > 0; --passos)
        avancar(dt);
}

Quadro Simulacao::quadro() const {
    Quadro q;
    q.telemetria.segundo = tempo_;
    q.telemetria.pistaRad = std::fmod(distancia_ / RAIO_PISTA_M, 2 * PI);
    q.telemetria.velocidadeKmh = velocidadeKmh_;
    q.telemetria.temperaturaC = temperaturaC_;
    q.telemetria.aceleracaoG = aceleracaoG_;
    q.telemetria.fase = fase_;
    q.componentes = componentes_; q.recursos = recursos_;
    q.eventos = static_cast<int>(eventos_.tamanho());
    q.ultimoEvento = ultimoEvento_;
    q.diagnostico = diagnostico_;
    q.reparo = reparo_;
    q.volta = voltas_;
    return q;
}

void Simulacao::capturar() {
    Quadro q = quadro();
    if (!quadros_.empty() && std::abs(quadros_.back().telemetria.segundo - tempo_) < 1e-9 &&
        quadros_.back().telemetria.fase == fase_)
        quadros_.back() = std::move(q);
    else quadros_.push_back(std::move(q));
}

std::vector<Componente> Simulacao::rankingRisco() const {
    auto ordenado = componentes_;
    std::sort(ordenado.begin(), ordenado.end(), [](const Componente& a, const Componente& b) {
        const double riscoA = (1 - a.integridade) + .16 * a.fadiga + (a.estrutural ? .03 : 0);
        const double riscoB = (1 - b.integridade) + .16 * b.fadiga + (b.estrutural ? .03 : 0);
        return riscoA > riscoB;
    });
    return ordenado;
}
const Componente& Simulacao::componenteMaisCritico() const {
    auto score=[](const Componente& p){return (1-p.integridade)+.16*p.fadiga+(p.estrutural?.03:0.);};
    return *std::max_element(componentes_.begin(),componentes_.end(),[&](const Componente& a,const Componente& b){return score(a)<score(b);});
}
int Simulacao::numeroCriticos() const {
    return static_cast<int>(std::count_if(componentes_.begin(), componentes_.end(),
        [](const Componente& p) { return p.integridade < .75; }));
}

void Simulacao::exportarCsv(const std::string& caminho) const {
    std::ofstream f(caminho);
    if (!f) throw std::runtime_error("nao foi possivel criar CSV: " + caminho);
    f << "tipo;tempo_s;fase;componente;causa;integridade_perdida;tensao_mpa;limite_mpa;probabilidade;energia_j;velocidade_kmh;temperatura_c\n";
    f << std::fixed << std::setprecision(5);
    for (const auto& a : telemetria_.itens()) {
        f << "telemetria;" << a.segundo << ';' << nome(a.fase)
          << ";;;;;;;;" << a.velocidadeKmh << ';' << a.temperaturaC << '\n';
    }
    for (const auto& e : eventos_.itens()) {
        f << "dano;" << e.segundo << ";;" << escaparCsv(e.componente) << ';'
          << escaparCsv(e.causa) << ';' << e.deltaIntegridade << ';' << e.tensaoMPa
          << ';' << e.limiteMPa << ';' << e.probabilidade << ';' << e.energiaImpactoJ
          << ";;\n";
    }
    if (!f) throw std::runtime_error("falha ao gravar CSV");
}

void Simulacao::exportarJson(const std::string& caminho) const {
    std::ofstream f(caminho);
    if (!f) throw std::runtime_error("nao foi possivel criar JSON: " + caminho);
    f << std::fixed << std::setprecision(5);
    f << "{\"versao\":1,\"cenario\":\"" << escaparJson(cenario_)
      << "\",\"observacao\":\"Propriedades e probabilidades didaticas, nao validadas.\",\"eventos\":[";
    for (std::size_t i = 0; i < eventos_.itens().size(); ++i) {
        const auto& e = eventos_.itens()[i];
        if (i) f << ',';
        f << "{\"id\":" << e.id << ",\"segundo\":" << e.segundo
          << ",\"componente\":\"" << escaparJson(e.componente)
          << "\",\"causa\":\"" << escaparJson(e.causa)
          << "\",\"tensaoMPa\":" << e.tensaoMPa << ",\"limiteMPa\":" << e.limiteMPa
          << ",\"energiaJ\":" << e.energiaImpactoJ << ",\"perda\":" << e.deltaIntegridade
          << ",\"probabilidade\":" << e.probabilidade << '}';
    }
    f << "],\"quadros\":[";
    for (std::size_t i = 0; i < quadros_.size(); ++i) {
        const auto& q = quadros_[i];
        if (i) f << ',';
        f << "{\"t\":" << q.telemetria.segundo << ",\"angulo\":" << q.telemetria.pistaRad
          << ",\"velocidade\":" << q.telemetria.velocidadeKmh
          << ",\"temperatura\":" << q.telemetria.temperaturaC
          << ",\"g\":" << q.telemetria.aceleracaoG << ",\"fase\":\"" << nome(q.telemetria.fase)
          << "\",\"volta\":" << q.volta << ",\"eventos\":" << q.eventos
          << ",\"ultimoEvento\":\"" << escaparJson(q.ultimoEvento)
          << "\",\"diagnostico\":\"" << escaparJson(q.diagnostico)
          << "\",\"reparo\":{\"tipo\":\"" << nome(q.reparo.tipo)
          << "\",\"componente\":\"" << escaparJson(q.reparo.componente)
          << "\",\"decorrido\":" << q.reparo.tempoDecorridoS
          << ",\"total\":" << q.reparo.tempoTotalS << "},\"recursos\":{\"bateriaWh\":"
          << q.recursos.bateriaWh << ",\"agenteMl\":" << q.recursos.agenteMl
          << ",\"pecas\":" << q.recursos.pecasReposicao << "},\"componentes\":[";
        for (std::size_t j = 0; j < q.componentes.size(); ++j) {
            const auto& p = q.componentes[j];
            if (j) f << ',';
            f << "{\"id\":\"" << escaparJson(p.id) << "\",\"nome\":\"" << escaparJson(p.nome)
              << "\",\"integridade\":" << p.integridade << ",\"desgaste\":" << p.desgaste
              << ",\"fadiga\":" << p.fadiga << ",\"tensao\":" << p.tensaoMPa
              << ",\"limite\":" << p.limiteMPa << ",\"x\":" << p.x
              << ",\"y\":" << p.y << ",\"z\":" << p.z << '}';
        }
        f << "]}";
    }
    f << "]}\n";
    if (!f) throw std::runtime_error("falha ao gravar JSON");
}
} // namespace cometa
