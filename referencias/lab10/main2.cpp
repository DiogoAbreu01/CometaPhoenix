#include <iostream>
#include "kart2.hpp"
#include "telemetria_io.hpp"
#include "catalogo_sensores.hpp" 

void testarEDescrever(const std::string& acao, bool aceitou, bool preservou) {
    std::cout << "Ação: " << acao 
              << " | Resultado: " << (aceitou ? "ACEITO" : "REJEITADO")
              << " | Estado Integro: " << (preservou ? "SIM" : "NÃO (ERRO)") << "\n";
}

void display(const Kart& kart) {
    std::cout << "=== TELEMETRIA DO COMETA ===\n\n";
    std::cout << "PNEUS\n";
    std::cout << "Pressao ............ " << kart.pressaoPneu() << " kPa\n";
    std::cout << "Estado ............. " << (kart.pressaoSegura() ? "OK" : "ALERTA") << "\n\n";

    std::cout << "MOTOR\n";
    std::cout << "Temperatura ........ " << kart.temperaturaMotor() << " C\n";
    std::cout << "Estado ............. " << (kart.temperaturaSegura() ? "OK" : "ALERTA") << "\n\n";

    std::cout << "AMBIENTE\n";
    std::cout << "Luminosidade ....... " << kart.luminosidade() << " lux\n";
    std::cout << "Faróis ............. " << (kart.faroisLigados() ? "LIGADOS" : "desligados") << "\n\n";

    std::cout << "KART\n";
    std::cout << "Velocidade ......... " << kart.getVelocidadeKmh() << " km/h\n\n";
}

int main() {
    Kart mario{"Cometa", "Mario", "CH-001", "vermelho"};

    std::cout << " INICIANDO SEQUÊNCIA DE TESTES KART MARIO \n\n";

    mario.acelerar(50);

    int velAntes = mario.getVelocidadeKmh();
    bool okAcel = mario.acelerar(100); 
    int velDepois = mario.getVelocidadeKmh();
    testarEDescrever("Acelerar +100", okAcel, (!okAcel && velAntes == velDepois));

    mario.frear(30); 

    const int velAntesFreio = mario.getVelocidadeKmh();
    const bool okFreio = mario.frear(30); 
    const int velDepoisFreio = mario.getVelocidadeKmh();
    testarEDescrever("Frear -30", okFreio, (!okFreio && velAntesFreio == velDepoisFreio));

    mario.abastecer(5.0); 

    double combAntes = mario.getCombustivelLitros();
    bool okComb = mario.abastecer(8.0); 
    double combDepois = mario.getCombustivelLitros();
    testarEDescrever("Abastecer +8.0L", okComb, (!okComb && combAntes == combDepois));

    mario.registrarVolta(61.8);

    int voltasAntes = mario.getQuantidadeVoltas();
    double tempoAntes = mario.getMelhorTempo();
    bool okVolta = mario.registrarVolta(-2.0); 
    int voltasDepois = mario.getQuantidadeVoltas();
    double tempoDepois = mario.getMelhorTempo();

    bool voltaPreservada = (!okVolta && voltasAntes == voltasDepois && tempoAntes == tempoDepois);
    testarEDescrever("Registrar Volta -2.0s", okVolta, voltaPreservada);

    std::cout << "\n ESTADO FINAL DO KART \n";
    std::cout << "Piloto: " << mario.getPiloto() << "\n";
    std::cout << "Velocidade: " << mario.getVelocidadeKmh() << " km/h\n";
    std::cout << "Combustível: " << mario.getCombustivelLitros() << " L\n";
    std::cout << "Voltas Completadas: " << mario.getQuantidadeVoltas() << "\n";
    std::cout << "Melhor Tempo: " << mario.getMelhorTempo() << " s\n\n";

    std::cout << "========================================\n";
    std::cout << " INICIANDO TESTES DE TELEMETRIA \n";
    std::cout << "========================================\n\n";

    Kart cometa{"Cometa", "Mario", "CH-001", "vermelho"};
    cometa.acelerar(50);

    std::cout << "LEITURA INICIAL\n";
    std::cout << "Pressao ............ " << cometa.pressaoPneu() << " kPa\n";
    std::cout << "Estado ............. " << (cometa.pressaoSegura() ? "OK" : "ALERTA") << "\n";
    std::cout << "Temperatura ........ " << cometa.temperaturaMotor() << " C\n";
    std::cout << "Estado ............. " << (cometa.temperaturaSegura() ? "OK" : "ALERTA") << "\n";
    std::cout << "Luminosidade ....... " << cometa.luminosidade() << " lux\n";
    std::cout << "Faróis ............. " << (cometa.faroisLigados() ? "LIGADOS" : "desligados") << "\n";
    std::cout << "Velocidade ......... " << cometa.getVelocidadeKmh() << " km/h\n\n";

    cometa.atualizarPressao(62.0);
    cometa.atualizarTemperatura(108.0);
    cometa.atualizarLuminosidade(120.0);
    cometa.atualizarFarois();

    display(cometa);

    std::cout << "\n========== LAB09: LEITURA DE ARQUIVO ==========\n";
    const ResultadoCarga carga = carregar_arquivo("dados/telemetria_cometa.csv");
    if (!carga.arquivo_aberto || carga.falha_leitura) {
        for (const auto& erro : carga.erros) std::cerr << erro << '\n';
        return 2;
    }

    Kart kartTreino{"Cometa", "Mario", "CH-001", "vermelho"};
    for (const auto& leitura : carga.aceitas) {
        aplicar_no_kart(kartTreino, leitura);
    }

    if (!escrever_boletim("boletim_cometa.txt", kartTreino, carga)) {
        std::cerr << "Nao foi possivel escrever boletim_cometa.txt\n";
        return 3;
    }

    std::cout << "Leituras aceitas: " << carga.aceitas.size()
              << " | Recusadas: " << carga.erros.size() << '\n';
    for (const auto& erro : carga.erros) std::cout << erro << '\n';
    std::cout << "Boletim gerado: boletim_cometa.txt\n";
    std::cout << "\n========== LAB10: CONFIGURACAO DE SENSORES ==========\n";
    const ResultadoConfiguracao entradaConfig =
        carregar_configuracao("dados/configuracao_cometa.csv");
    if (!entradaConfig.pronta()) {
        for (const auto& erro : entradaConfig.erros) std::cerr << erro << '\n';
        return 10;
    }

    KartConfiguravel configuracao{"Cometa", "Mario", "CH-001", "vermelho"};
    std::vector<EntradaCatalogo> catalogo;
    std::string erroInstalacao;
    if (!instalar_sensores(configuracao, entradaConfig.especificacoes,
                          catalogo, erroInstalacao)) {
        std::cerr << erroInstalacao << '\n';
        return 11;
    }
    for (const auto& item : catalogo) displayStatus(*item.observador);
    if (!escrever_catalogo("catalogo_cometa.csv", catalogo)) {
        std::cerr << "Nao foi possivel escrever catalogo_cometa.csv\n";
        return 12;
    }
    std::cout << "LAB10 instalados=" << configuracao.quantidadeSensoresExtras() << '\n';
    std::cout << "catalogo=catalogo_cometa.csv\n";
    return 0;
}