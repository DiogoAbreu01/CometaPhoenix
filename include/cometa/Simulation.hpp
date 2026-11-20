#pragma once

#include <cstdint>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace cometa {

// Inspirado nos templates e na abordagem de classes dos LABs originais.
template<class T>
class Historico {
public:
    void registrar(const T& item) { itens_.push_back(item); }
    const std::vector<T>& itens() const noexcept { return itens_; }
    std::size_t tamanho() const noexcept { return itens_.size(); }
private:
    std::vector<T> itens_;
};

template<class T>
T limitar(T valor, T minimo, T maximo) {
    if (maximo < minimo) throw std::invalid_argument("faixa invertida");
    return valor < minimo ? minimo : (maximo < valor ? maximo : valor);
}

enum class Fase { Corrida, Alerta, RetornoBox, Diagnostico, Reparando, Validacao, Abandono };
enum class Material { Aco, Composito, Borracha, LigaMemoria, MetalMecanico };
enum class TipoReparo { Nenhum, Microcapsulas, MemoriaForma, SubstituicaoRobotica };

const char* nome(Fase fase) noexcept;
const char* nome(TipoReparo reparo) noexcept;

struct Componente {
    std::string id;
    std::string nome;
    Material material{};
    double areaM2{};           // area resistente equivalente, m^2 (aproximacao)
    double limiteMPa{};        // tensao admissivel parametrica, MPa
    double fadigaLimiteMPa{};  // resistencia de referencia a ciclos, MPa
    double integridade{1.0};
    double desgaste{};         // 0..1, sem recuperacao por selagem
    double fadiga{};           // acumulacao (Miner heuristico)
    double tensaoMPa{};
    double microcapsulasMl{};  // estoque local disponivel no componente
    TipoReparo tecnologia{TipoReparo::Nenhum};
    double x{}, y{}, z{};      // posicao na maquete do kart, metros
    bool estrutural{false};
};

struct EventoDano {
    int id{};
    double segundo{};
    std::string componente;
    std::string causa;
    double tensaoMPa{};
    double limiteMPa{};
    double energiaImpactoJ{};
    double deltaIntegridade{};
    double probabilidade{};
    bool fatal{};
};

struct AmostraTelemetria {
    double segundo{};
    double pistaRad{};
    double velocidadeKmh{};
    double temperaturaC{};
    double aceleracaoG{};
    Fase fase{Fase::Corrida};
};

struct Recursos {
    double bateriaWh{110};
    double agenteMl{35};
    int pecasReposicao{1};
};

struct PlanoReparo {
    TipoReparo tipo{TipoReparo::Nenhum};
    std::string componente;
    double tempoTotalS{};
    double tempoDecorridoS{};
    double energiaTotalWh{};
    double agenteMl{};
    int pecas{};
    double integridadeInicial{};
    double integridadeAlvo{};
    std::string explicacao;
};

class EstrategiaReparo {
public:
    virtual ~EstrategiaReparo() = default;
    virtual TipoReparo tipo() const noexcept = 0;
    virtual bool serve(const Componente& p) const noexcept = 0;
    virtual PlanoReparo criarPlano(const Componente& p) const = 0;
};

class Microcapsulas final : public EstrategiaReparo {
public:
    TipoReparo tipo() const noexcept override { return TipoReparo::Microcapsulas; }
    bool serve(const Componente& p) const noexcept override;
    PlanoReparo criarPlano(const Componente& p) const override;
};

class MemoriaDeForma final : public EstrategiaReparo {
public:
    TipoReparo tipo() const noexcept override { return TipoReparo::MemoriaForma; }
    bool serve(const Componente& p) const noexcept override;
    PlanoReparo criarPlano(const Componente& p) const override;
};

class SubstituicaoRobotica final : public EstrategiaReparo {
public:
    TipoReparo tipo() const noexcept override { return TipoReparo::SubstituicaoRobotica; }
    bool serve(const Componente& p) const noexcept override;
    PlanoReparo criarPlano(const Componente& p) const override;
};

struct Quadro {
    AmostraTelemetria telemetria;
    std::vector<Componente> componentes;
    Recursos recursos;
    int eventos{};
    std::string ultimoEvento;
    std::string diagnostico;
    PlanoReparo reparo;
    int volta{};
};

class Simulacao {
public:
    explicit Simulacao(std::string cenario = "impacto_leve", std::uint32_t seed = 42);
    void avancar(double dt);
    // Integra controle do jogador sem alterar os cenarios autonomos dos laboratorios.
    void definirVelocidadeExterna(double kmh);
    // No jogo, aguarda a chegada fisica do reboque ao box; nos labs, conserva o fluxo temporal.
    void definirChegadaBoxExterna(bool chegou, double kmh);
    void simular(double segundos, double dt = 0.05);
    void provocarImpacto(const std::string& id, double deltaVelocidadeMs, double duracaoS);
    bool iniciarReparo();
    const std::vector<Componente>& componentes() const noexcept { return componentes_; }
    const Historico<EventoDano>& danos() const noexcept { return eventos_; }
    const Historico<AmostraTelemetria>& telemetria() const noexcept { return telemetria_; }
    const Recursos& recursos() const noexcept { return recursos_; }
    const PlanoReparo& reparo() const noexcept { return reparo_; }
    const std::string& diagnostico() const noexcept { return diagnostico_; }
    Fase fase() const noexcept { return fase_; }
    double tempo() const noexcept { return tempo_; }
    int voltas() const noexcept { return voltas_; }
    Quadro quadro() const;
    const std::vector<Quadro>& quadros() const noexcept { return quadros_; }
    const Componente& componente(const std::string& id) const;
    std::vector<Componente> rankingRisco() const;
    const Componente& componenteMaisCritico() const;
    int numeroCriticos() const;
    void exportarCsv(const std::string& caminho) const;
    void exportarJson(const std::string& caminho) const;

private:
    std::vector<Componente> componentes_;
    Historico<EventoDano> eventos_;
    Historico<AmostraTelemetria> telemetria_;
    Recursos recursos_;
    Fase fase_{Fase::Corrida};
    std::string cenario_;
    std::mt19937 rng_;
    double tempo_{};
    double faseDesde_{};
    double distancia_{};
    double temperaturaC_{78};
    double velocidadeKmh_{44};
    bool controleVelocidadeExterna_{false};
    bool controleBoxExterno_{false}, chegouBoxExterno_{false};
    double aceleracaoG_{};
    bool impactoExecutado_{};
    int voltas_{};
    int sequencia_{};
    std::string diagnostico_{"Aguardando leituras"};
    std::string ultimoEvento_{};
    PlanoReparo reparo_{};
    std::vector<Quadro> quadros_;
    double amostraDesde_{};
    int ultimaAmostra_{};
    void calcularCargas(double dt);
    void aplicarFadiga(Componente& p, double dt);
    void registrarDano(Componente& p, const std::string& causa, double perda,
                       double tensao, double probabilidade, double energiaJ);
    void diagnosticar();
    Componente& buscar(const std::string& id);
    void atualizarReparo(double dt);
    bool validarSeguro() const;
    void mudarFase(Fase fase);
    void capturar();
};

} // namespace cometa
