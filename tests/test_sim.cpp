#include "cometa/Simulation.hpp"

#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#define CHECK(expr) do { if (!(expr)) { std::cerr << "FALHOU: " << #expr << " em " << __FILE__ << ':' << __LINE__ << '\n'; std::exit(1); } } while (false)
using namespace cometa;

void testaTemplates() {
    CHECK(limitar<int>(11, 0, 10) == 10);
    CHECK(limitar<double>(-1.5, 0.0, 1.0) == 0.0);
    Historico<EventoDano> d;
    Historico<AmostraTelemetria> t;
    d.registrar({}); t.registrar({});
    CHECK(d.tamanho() == 1 && t.tamanho() == 1);
    bool invalido = false;
    try { (void)limitar(3, 7, 0); } catch (const std::invalid_argument&) { invalido = true; }
    CHECK(invalido);
}
void testaNormal() {
    Simulacao s("normal", 42);
    s.simular(22);
    CHECK(s.fase() == Fase::Corrida);
    CHECK(s.danos().tamanho() == 0);
    CHECK(s.telemetria().tamanho() > 20);
    CHECK(s.componente("chassi").integridade == 1.0);
    CHECK(s.quadro().telemetria.velocidadeKmh > 0);
}
void testaImpactoRegeneracao() {
    Simulacao s("impacto_leve", 42);
    s.simular(11);
    CHECK(s.danos().tamanho() == 1);
    CHECK(s.danos().itens()[0].energiaImpactoJ > 0);
    const auto perda = s.danos().itens()[0].deltaIntegridade;
    CHECK(perda > .25 && perda < .55);
    const double antes = s.componente("carenagem").integridade;
    s.simular(5);
    CHECK(s.fase() == Fase::Reparando);
    CHECK(s.reparo().tipo == TipoReparo::Microcapsulas);
    CHECK(s.recursos().agenteMl < 35);
    CHECK(s.reparo().tempoDecorridoS > 0);
    CHECK(s.componente("carenagem").integridade > antes);
    CHECK(s.componente("carenagem").integridade < 1);
    s.simular(18);
    CHECK(s.fase() == Fase::Corrida);
    CHECK(s.componente("carenagem").integridade > .75);
    CHECK(s.componente("carenagem").integridade < 1);
    CHECK(s.danos().tamanho() == 1);
    CHECK(s.rankingRisco().front().id == "carenagem");
}
void testaChassiCritico() {
    Simulacao s("impacto_critico", 42);
    s.simular(38);
    CHECK(s.fase() == Fase::Abandono);
    CHECK(s.componente("chassi").integridade < .55);
    CHECK(s.reparo().tipo == TipoReparo::Nenhum);
    CHECK(s.diagnostico().find("nao e autocuravel") != std::string::npos);
}
void testaSemRecurso() {
    Simulacao s("sem_recurso", 42);
    s.simular(38);
    CHECK(s.fase() == Fase::Abandono);
    CHECK(s.reparo().tipo == TipoReparo::Nenhum);
    CHECK(s.diagnostico().find("recurso insuficiente") != std::string::npos);
}
void testaMemoriaForma() {
    Simulacao s("fadiga", 42);
    s.simular(32);
    CHECK(s.danos().tamanho() == 1);
    CHECK(s.reparo().tipo == TipoReparo::MemoriaForma);
    CHECK(s.componente("suspensao").fadiga >= .75);
    CHECK(s.componente("suspensao").integridade > .73);
    CHECK(s.componente("suspensao").integridade < 1);
    CHECK(s.fase() == Fase::Corrida);
}
void testaDeterminismo() {
    Simulacao a("impacto_leve", 17), b("impacto_leve", 17);
    a.simular(28); b.simular(28);
    CHECK(a.danos().tamanho() == b.danos().tamanho());
    CHECK(a.danos().itens().front().deltaIntegridade == b.danos().itens().front().deltaIntegridade);
    CHECK(a.componente("carenagem").integridade == b.componente("carenagem").integridade);
}
void testaExcecoes() {
    bool falhou = false;
    try { Simulacao s("inexistente"); } catch (const std::invalid_argument&) { falhou = true; }
    CHECK(falhou);
    Simulacao s;
    falhou = false;
    try { s.avancar(-.1); } catch (const std::invalid_argument&) { falhou = true; }
    CHECK(falhou);
    falhou = false;
    try { s.provocarImpacto("chassi", 4, 0); } catch (const std::invalid_argument&) { falhou = true; }
    CHECK(falhou);
    falhou = false;
    try { s.componente("desconhecido"); } catch (const std::invalid_argument&) { falhou = true; }
    CHECK(falhou);
}
void testaTodosComponentes() {
 Simulacao s("normal",42);
 for(const auto& p:s.componentes()) { CHECK(p.integridade==1.); CHECK(s.componente(p.id).id==p.id); }
 for(const auto& p:s.componentes()) {Simulacao t("normal",42); t.provocarImpacto(p.id,10,.036);CHECK(t.danos().tamanho()==1);CHECK(t.danos().itens()[0].componente==p.id);CHECK(t.componente(p.id).integridade<1);CHECK(t.componenteMaisCritico().id==t.rankingRisco().front().id);}
}
void testaExportacao() {
    Simulacao s("impacto_leve"); s.simular(36);
    s.exportarJson("teste_cometa.json");
    s.exportarCsv("teste_cometa.csv");
    std::ifstream json("teste_cometa.json"), csv("teste_cometa.csv");
    CHECK(json.good() && csv.good());
    std::string js((std::istreambuf_iterator<char>(json)), std::istreambuf_iterator<char>());
    CHECK(js.find("\"quadros\"") != std::string::npos);
    CHECK(js.find("\"REPARANDO\"") != std::string::npos);
    CHECK(js.find("\"micro") == std::string::npos || js.find("MICROCAPSULAS") != std::string::npos);
    std::remove("teste_cometa.json"); std::remove("teste_cometa.csv");
}
int main() {
    testaTemplates(); testaNormal(); testaImpactoRegeneracao(); testaChassiCritico();
    testaSemRecurso(); testaMemoriaForma(); testaDeterminismo(); testaExcecoes(); testaTodosComponentes(); testaExportacao();
    std::cout << "PASSOU: templates/STL, 5 cenarios, reparo progressivo, limites, determinismo, exportacao\n";
}
