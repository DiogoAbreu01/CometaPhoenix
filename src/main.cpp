#include "cometa/Simulation.hpp"
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    try {
        std::string cenario = "impacto_leve";
        std::string json, csv;
        std::uint32_t seed = 42;
        double duracao = 40;
        for (int i = 1; i < argc; ++i) {
            const std::string a = argv[i];
            if (a == "--ajuda" || a == "--help") {
                std::cout << "Cometa Phoenix C++17\n--cenario normal|impacto_leve|impacto_critico|sem_recurso|fadiga\n"
                          << "--seed INTEIRO --duracao SEGUNDOS --json ARQUIVO --csv ARQUIVO\n";
                return 0;
            }
            if (a == "--headless") continue;
            if (i + 1 >= argc) throw std::invalid_argument("falta valor para " + a);
            const std::string v = argv[++i];
            if (a == "--cenario") cenario = v;
            else if (a == "--seed") { const auto n = std::stoull(v); if (n > UINT32_MAX) throw std::out_of_range("seed"); seed = static_cast<std::uint32_t>(n); }
            else if (a == "--duracao") duracao = std::stod(v);
            else if (a == "--json") json = v;
            else if (a == "--csv") csv = v;
            else throw std::invalid_argument("opcao desconhecida: " + a);
        }
        cometa::Simulacao sim(cenario, seed);
        sim.simular(duracao);
        if (!json.empty()) sim.exportarJson(json);
        if (!csv.empty()) sim.exportarCsv(csv);
        std::cout << "COMETA PHOENIX | cenario: " << cenario << " | seed: " << seed << '\n';
        std::cout << "Tempo: " << std::fixed << std::setprecision(1) << sim.tempo() << " s"
                  << " | Fase: " << cometa::nome(sim.fase()) << " | Voltas: " << sim.voltas() << '\n';
        std::cout << "Eventos: " << sim.danos().tamanho() << " | Criticos: " << sim.numeroCriticos()
                  << " | Bateria: " << sim.recursos().bateriaWh << " Wh\n";
        for (const auto& e : sim.danos().itens())
            std::cout << "[" << e.segundo << "s] " << e.componente << " -> " << e.causa
                      << " | " << e.tensaoMPa << "/" << e.limiteMPa << " MPa"
                      << " | dano " << e.deltaIntegridade * 100 << "%\n";
        std::cout << "Diagnostico: " << sim.diagnostico() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "ERRO: " << e.what() << '\n';
        return 2;
    }
}
