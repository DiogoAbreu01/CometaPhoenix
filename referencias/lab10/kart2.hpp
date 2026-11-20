#ifndef KART_HPP
#define KART_HPP

#include <string>

class Kart {
private:
    std::string modelo_;
    std::string piloto_;
    std::string chassi_;
    std::string cor_;
    int velocidadeKmh_;
    double combustivelL_;
    int quantidadeVoltas_;
    double melhorTempo_;
    double pressaoPneu_{82.0};
    double temperaturaMotor_{92.0};
    double luminosidade_{700.0};
    bool faroisLigados_{false};

public:
    // limites da classe
    static constexpr int VELOCIDADE_MAX = 80;
    static constexpr double COMBUSTIVEL_MAX = 10.0;

    Kart(const std::string& modelo, const std::string& piloto, 
         const std::string& chassi, const std::string& cor);

    // gtters
    std::string getModelo() const;
    std::string getPiloto() const;
    std::string getChassi() const;
    std::string getCor() const;
    int getVelocidadeKmh() const;
    double getCombustivelL() const;
    double getCombustivelLitros() const;
    double pressaoPneu() const;
    double temperaturaMotor() const;
    double luminosidade() const;
    bool pressaoSegura() const;
    bool temperaturaSegura() const;
    bool faroisLigados() const;
    void atualizarPressao(double valor);
    void atualizarTemperatura(double valor);
    void atualizarLuminosidade(double valor);
    void atualizarFarois();
    int getQuantidadeVoltas() const;
    double getMelhorTempo() const;

    // valida
    bool acelerar(int incremento);
    bool frear(int decremento);
    bool abastecer(double quantidade);
    bool registrarVolta(double tempo);
    bool trocarPiloto(const std::string& novoPiloto);
    bool repintar(const std::string& novaCor);
};

#endif