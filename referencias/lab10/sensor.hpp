#ifndef SENSOR_HPP
#define SENSOR_HPP

#include <string>

class Sensor {
    std::string id_;
    double leitura_;
public:
    Sensor(const std::string& id, double leitura);
    virtual ~Sensor() = default;
    const std::string& identificador() const;
    double leituraBruta() const;
    virtual bool requerAtencao() const = 0;
};

class SensorTemperaturaMotor : public Sensor {
    double limite_;
public:
    SensorTemperaturaMotor(const std::string& id, double leitura, double limite);
    bool requerAtencao() const override;
};

class SensorPressaoPneu : public Sensor {
    double minimo_;
    double maximo_;
public:
    SensorPressaoPneu(const std::string& id, double leitura, double minimo, double maximo);
    bool requerAtencao() const override;
};

void displayStatus(const Sensor& sensor);

#endif
