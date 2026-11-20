#include "telemetria_io.hpp"
#include <cmath>      
#include <cstddef>    
#include <fstream>    
#include <iomanip>    
#include <locale>     
#include <sstream>    
#include <string>
#include <iostream>

bool numero_finito(const std::string& texto, double& valor) {
    std::istringstream entrada{texto};
    entrada.imbue(std::locale::classic());
    entrada >> valor;
    if (!entrada) return false;
    entrada >> std::ws;
    return entrada.eof() && std::isfinite(valor);
}

ResultadoInterpretacao interpretar_linha(const std::string& linha) {

    const auto virgula = linha.find(',');
    if (virgula == std::string::npos ||
        linha.find(',', virgula + 1) != std::string::npos) {
        return {false, {}, "estrutura: esperados 2 campos"};
    }

    if (linha.find('"') != std::string::npos) {
        return {false, {}, "aspas fora do formato didatico"};
    }
    const std::string canal = linha.substr(0, virgula);
    const std::string texto_valor = linha.substr(virgula + 1);

    if (canal.empty())
        return {false, {}, "canal vazio"};

    if (canal != "PRESSAO_PNEU" &&
        canal != "TEMP_MOTOR" &&
        canal != "LUZ_AMBIENTE") {
        return {false, {}, "canal desconhecido: " + canal};
    }

   double valor{};
    if (!numero_finito(texto_valor, valor)) {
        return {false, {}, "valor nao e um numero finito completo"};
    }

    // Se passou por todas as checagens, retorna sucesso com a leitura estruturada
    return {true, {canal, valor}, {}};
}

ResultadoCarga carregar_arquivo(const std::string& caminho) {
    std::ifstream entrada{caminho};
    if (!entrada) {
        return {false, false, {}, {"nao abriu: " + caminho}};
    }

    ResultadoCarga resultado;
    resultado.arquivo_aberto = true;
    std::string linha;
    std::size_t numero = 0;

    while (std::getline(entrada, linha)) {
        ++numero;
        if (linha.empty()) continue;

        const auto item = interpretar_linha(linha);
        if (item.ok) {
            resultado.aceitas.push_back(item.leitura);
        } else {
            resultado.erros.push_back(
                "linha " + std::to_string(numero) + ": " + item.erro);
        }
    }

    if (entrada.bad()) {
        resultado.falha_leitura = true;
        resultado.erros.push_back("falha de leitura");
    }
    return resultado;
}

void aplicar_no_kart(Kart& kart, const LeituraValidada& leitura) {
    if (leitura.canal == "PRESSAO_PNEU") {
        kart.atualizarPressao(leitura.valor);
    } else if (leitura.canal == "TEMP_MOTOR") {
        kart.atualizarTemperatura(leitura.valor);
    } else if (leitura.canal == "LUZ_AMBIENTE") {
        kart.atualizarLuminosidade(leitura.valor);
        kart.atualizarFarois();
    }
}

bool escrever_boletim(const std::string& caminho,
                      const Kart& kart, const ResultadoCarga& carga) {
    std::ofstream saida{caminho};
    if (!saida) return false; 
    saida.imbue(std::locale::classic());
    saida << std::setprecision(12);
    saida << "BOLETIM DO COMETA\n";
    saida << "Pressao: " << kart.pressaoPneu() << " kPa - "
          << (kart.pressaoSegura() ? "OK" : "ATENCAO") << '\n';
    saida << "Temperatura: " << kart.temperaturaMotor() << " C - "
          << (kart.temperaturaSegura() ? "OK" : "ATENCAO") << '\n';
    saida << "Luminosidade: " << kart.luminosidade() << " lux - "
          << (kart.faroisLigados() ? "ATENCAO" : "OK") << '\n';
    saida << "Farois: " << (kart.faroisLigados() ? "LIGADOS" : "DESLIGADOS")
          << '\n';
    saida << "Aceitas: " << carga.aceitas.size() << '\n';
    saida << "Recusadas: " << carga.erros.size() << '\n';
    for (const auto& erro : carga.erros) {
        saida << "Erro: " << erro << '\n';
    }
    saida.close();
    return static_cast<bool>(saida);
}
