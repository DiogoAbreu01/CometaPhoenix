#ifndef ECOP13A_LAB10_CATALOGO_SENSORES_HPP
#define ECOP13A_LAB10_CATALOGO_SENSORES_HPP

#include "kart_configuravel.hpp"
#include <memory>
#include <string>
#include <vector>

struct EspecificacaoSensor {
    std::string tipo;         
    std::string id;           
    double limite{};          
};

struct ResultadoConfiguracao {
    bool arquivo_aberto{false};
    bool falha_leitura{false};
    std::vector<EspecificacaoSensor> especificacoes;
    std::vector<std::string> erros;

    bool pronta() const {
        return arquivo_aberto && !falha_leitura && erros.empty();
    }
};

struct EntradaCatalogo {
    const Sensor* observador{nullptr};
    std::string tipo;
    std::string unidade;
};

ResultadoConfiguracao carregar_configuracao(const std::string& caminho);

std::unique_ptr<Sensor> criar_sensor(const EspecificacaoSensor& spec,
                                    std::string& erro);

bool instalar_sensores(KartConfiguravel& configuracao,
                      const std::vector<EspecificacaoSensor>& specs,
                      std::vector<EntradaCatalogo>& catalogo,
                      std::string& erro);

bool escrever_catalogo(const std::string& caminho,
                      const std::vector<EntradaCatalogo>& catalogo);

#endif
