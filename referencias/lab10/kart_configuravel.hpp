#ifndef KART_CONFIGURAVEL_HPP
#define KART_CONFIGURAVEL_HPP

#include "kart2.hpp"
#include "sensor.hpp"
#include <memory>
#include <vector>

class KartConfiguravel : public Kart {
    std::vector<std::unique_ptr<Sensor>> sensoresExtras_;
public:
    using Kart::Kart;
    bool adicionarSensorExtra(std::unique_ptr<Sensor> sensor);
    std::size_t quantidadeSensoresExtras() const;
};

#endif
