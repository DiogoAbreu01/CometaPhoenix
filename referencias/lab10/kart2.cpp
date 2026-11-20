#include "kart2.hpp"

Kart::Kart(const std::string& modelo, const std::string& piloto, 
           const std::string& chassi, const std::string& cor)
    : modelo_(modelo), piloto_(piloto), chassi_(chassi), cor_(cor),
      velocidadeKmh_(0), combustivelL_(0.0), quantidadeVoltas_(0), melhorTempo_(0) {}

std::string Kart::getModelo() const { return modelo_; }
std::string Kart::getPiloto() const { return piloto_; }
std::string Kart::getChassi() const { return chassi_; }
std::string Kart::getCor() const { return cor_; }
int Kart::getVelocidadeKmh() const { return velocidadeKmh_; }
double Kart::getCombustivelL() const { return combustivelL_; }
int Kart::getQuantidadeVoltas() const { return quantidadeVoltas_; }
double Kart::getMelhorTempo() const { return melhorTempo_; }

bool Kart::acelerar(int incremento) {
    if (incremento <= 0) return false;
    
    // candidato
    int candidato = velocidadeKmh_ + incremento;
    
    // regra
    if (candidato > VELOCIDADE_MAX) return false;
    
    // commit
    velocidadeKmh_ = candidato;
    return true;
}

bool Kart::frear(int decremento) {
    if (decremento <= 0) return false;
    
    // candidato
    int candidato = velocidadeKmh_ - decremento;
    
    // regra
    if (candidato < 0) return false;
    
    // commit
    velocidadeKmh_ = candidato;
    return true;
}

bool Kart::abastecer(double quantidade) {
    if (quantidade <= 0.0) return false;
    
    // candidato
    double candidato = combustivelL_ + quantidade;
    
    // regra
    if (candidato > COMBUSTIVEL_MAX) return false;
    
    // commit
    combustivelL_ = candidato;
    return true;
}

// reg volta garant consist
bool Kart::registrarVolta(double tempo) {
    // regra valida antes
    if (tempo <= 0.0) return false;

    // comtt
    quantidadeVoltas_++;

    if (quantidadeVoltas_ == 1 || tempo < melhorTempo_) {
        melhorTempo_ = tempo;
    }

    return true;
}

bool Kart::trocarPiloto(const std::string& novoPiloto) {
    if (novoPiloto.empty()) return false;
    piloto_ = novoPiloto;
    return true;
}

bool Kart::repintar(const std::string& novaCor) {
    if (novaCor.empty()) return false;
    cor_ = novaCor;
    return true;
}
// Consultas e atualizacoes da telemetria do Kart.
double Kart::getCombustivelLitros() const { return getCombustivelL(); }
double Kart::pressaoPneu() const { return pressaoPneu_; }
double Kart::temperaturaMotor() const { return temperaturaMotor_; }
double Kart::luminosidade() const { return luminosidade_; }
bool Kart::pressaoSegura() const { return pressaoPneu_ >= 70.0 && pressaoPneu_ <= 90.0; }
bool Kart::temperaturaSegura() const { return temperaturaMotor_ <= 105.0; }
bool Kart::faroisLigados() const { return faroisLigados_; }
void Kart::atualizarPressao(double valor) { pressaoPneu_ = valor; }
void Kart::atualizarTemperatura(double valor) { temperaturaMotor_ = valor; }
void Kart::atualizarLuminosidade(double valor) { luminosidade_ = valor; }
void Kart::atualizarFarois() { faroisLigados_ = luminosidade_ < 200.0; }
