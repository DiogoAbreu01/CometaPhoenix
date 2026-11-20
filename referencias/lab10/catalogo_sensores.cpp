#include "catalogo_sensores.hpp"
#include <algorithm>  
#include <cmath>     
#include <fstream>   
#include <locale>     
#include <sstream>    
#include <unordered_set> 
#include <utility>   
namespace {
bool numero_finito_completo(const std::string& texto, double& numero) {
    std::istringstream entrada{texto};
    entrada.imbue(std::locale::classic());
    entrada >> numero;
    if (!entrada) return false;
    entrada >> std::ws;
    return entrada.eof() && std::isfinite(numero);
}
bool interpretar_configuracao(const std::string& linha,
                            EspecificacaoSensor& spec, std::string& erro) {
    const auto primeira = linha.find(',');
    if (primeira == std::string::npos) {
        erro = "estrutura: esperados 3 campos";
        return false;
    }
    const auto segunda = linha.find(',', primeira + 1);
    if (segunda == std::string::npos ||
        linha.find(',', segunda + 1) != std::string::npos) {
        erro = "estrutura: esperados 3 campos";
        return false;
    }
    if (linha.find('"') != std::string::npos) {
        erro = "aspas fora do formato didatico";
        return false;
    }

    const std::string tipo = linha.substr(0, primeira);
    const std::string id = linha.substr(primeira + 1,
                                       segunda - primeira - 1);
    const std::string limite_texto = linha.substr(segunda + 1);
    if (tipo.empty() || id.empty()) {
        erro = "tipo ou id vazio";
        return false;
    }
    if (tipo.find_first_of(" \t\r") != std::string::npos ||
        id.find_first_of(" \t\r") != std::string::npos) {
        erro = "tipo ou id contem espacos";
        return false;
    }
    double limite{};
    if (!numero_finito_completo(limite_texto, limite)) {
        erro = "limite nao e numero finito completo";
        return false;
    }
    spec = {tipo, id, limite};
    return true;
}
} // namespace

ResultadoConfiguracao carregar_configuracao(const std::string& caminho) {
    std::ifstream entrada{caminho};
    ResultadoConfiguracao resultado;
    if (!entrada) {
        resultado.erros.push_back("nao abriu: " + caminho);
        return resultado;
    }
    resultado.arquivo_aberto = true;
    std::unordered_set<std::string> ids;
    std::string linha;
    std::size_t numero_linha = 0;
    while (std::getline(entrada, linha)) {
        ++numero_linha;
        if (linha.empty()) continue;
        EspecificacaoSensor spec;
        std::string erro;
        if (!interpretar_configuracao(linha, spec, erro)) {
            resultado.erros.push_back("linha " + std::to_string(numero_linha)
                                      + ": " + erro);
            continue;
        }
        if (!ids.insert(spec.id).second) {
            resultado.erros.push_back("linha " + std::to_string(numero_linha)
                                      + ": id duplicado: " + spec.id);
            continue;
        }
        resultado.especificacoes.push_back(std::move(spec));
    }
    if (entrada.bad()) {
        resultado.falha_leitura = true;
        resultado.erros.push_back("falha grave de leitura");
    }
    return resultado;
}

std::unique_ptr<Sensor> criar_sensor(const EspecificacaoSensor& spec,
                                    std::string& erro) {
    erro.clear();
    if (spec.id.empty()) {
        erro = "identificador vazio";
        return nullptr;
    }
    if (!std::isfinite(spec.limite)) {
        erro = "limite nao finito";
        return nullptr;
    }
    if (spec.tipo == "temperature") {
        if (spec.limite <= -273.15) {
            erro = "limite de temperatura fisicamente impossivel";
            return nullptr;
        }
        return std::make_unique<SensorTemperaturaMotor>(spec.id, 92.0, spec.limite);
    }
    if (spec.tipo == "pressure") {
        if (spec.limite <= 70.0) {
            erro = "maximo de pressao deve ser maior que 70 kPa";
            return nullptr;
        }
        return std::make_unique<SensorPressaoPneu>(spec.id, 82.0, 70.0, spec.limite);
    }
    erro = "tipo de sensor nao suportado: " + spec.tipo;
    return nullptr;
}

bool instalar_sensores(KartConfiguravel& configuracao,
                      const std::vector<EspecificacaoSensor>& specs,
                      std::vector<EntradaCatalogo>& catalogo,
                      std::string& erro) {
    erro.clear();
    if (configuracao.quantidadeSensoresExtras() != 0 || !catalogo.empty()) {
        erro = "instalacao exige configuracao e catalogo inicialmente vazios";
        return false;
    }
    for (const auto& spec : specs) {
        auto dono = criar_sensor(spec, erro);
        if (!dono) {
            erro = "sensor " + spec.id + ": " + erro;
            return false;
        }
        const Sensor* observador = dono.get();
        if (!configuracao.adicionarSensorExtra(std::move(dono))) {
            erro = "nao foi possivel instalar sensor " + spec.id;
            return false;
        }
        catalogo.push_back({observador, spec.tipo,
                            spec.tipo == "temperature" ? "C" : "kPa"});
    }
    return true;
}

bool escrever_catalogo(const std::string& caminho,
                      const std::vector<EntradaCatalogo>& catalogo) {
    for (const auto& item : catalogo) {
        if (item.observador == nullptr) return false;
    }
    std::ofstream saida{caminho};
    if (!saida) return false;

    std::vector<const EntradaCatalogo*> ordenados;
    ordenados.reserve(catalogo.size());
    for (const auto& item : catalogo) ordenados.push_back(&item);
    std::sort(ordenados.begin(), ordenados.end(),
              [](const EntradaCatalogo* a, const EntradaCatalogo* b) {
                  return a->observador->identificador() < b->observador->identificador();
              });

    saida << "id,type,unit\n";
    for (const auto* item : ordenados) {
        saida << item->observador->identificador() << ','
              << item->tipo << ',' << item->unidade << '\n';
    }
    saida.close();
    return static_cast<bool>(saida);
}
