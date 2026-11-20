#include "sensor.hpp"
#include <iostream>
#include <utility>

Sensor::Sensor(const std::string& id, double leitura)
    : id_(id), leitura_(leitura) {}

const std::string& Sensor::identificador() const { return id_; }
double Sensor::leituraBruta() const { return leitura_; }

SensorTemperaturaMotor::SensorTemperaturaMotor(const std::string& id, double leitura, double limite)
    : Sensor(id, leitura), limite_(limite) {}

bool SensorTemperaturaMotor::requerAtencao() const { return leituraBruta() > limite_; }

SensorPressaoPneu::SensorPressaoPneu(const std::string& id, double leitura, double minimo, double maximo)
    : Sensor(id, leitura), minimo_(minimo), maximo_(maximo) {}

bool SensorPressaoPneu::requerAtencao() const {
    return leituraBruta() < minimo_ || leituraBruta() > maximo_;
}

void displayStatus(const Sensor& sensor) {
    std::cout << sensor.identificador() << " | Leitura: " << sensor.leituraBruta()
              << " | " << (sensor.requerAtencao() ? "ATENCAO" : "OK") << '\n';
}
