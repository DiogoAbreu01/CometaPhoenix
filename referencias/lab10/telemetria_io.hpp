#pragma once
#include "kart2.hpp"
#include <string>
#include <vector>
struct LeituraValidada {
    std::string canal;
    double valor{};
};
struct ResultadoInterpretacao {
    bool ok{false};
    LeituraValidada leitura{};
    std::string erro{};
};
struct ResultadoCarga {
    bool arquivo_aberto{false};
    bool falha_leitura{false};
    std::vector<LeituraValidada> aceitas;
    std::vector<std::string> erros;
};
bool numero_finito(const std::string& texto, double& valor);
ResultadoInterpretacao interpretar_linha(const std::string& linha);
ResultadoCarga carregar_arquivo(const std::string& caminho);
void aplicar_no_kart(Kart& kart, const LeituraValidada& leitura);
bool escrever_boletim(const std::string& caminho,
                      const Kart& kart, const ResultadoCarga& carga);
