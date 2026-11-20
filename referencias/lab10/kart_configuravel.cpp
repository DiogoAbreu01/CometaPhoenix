#include "kart_configuravel.hpp"
#include <utility>

bool KartConfiguravel::adicionarSensorExtra(std::unique_ptr<Sensor> sensor) {
    if (!sensor) return false;
    sensoresExtras_.push_back(std::move(sensor));
    return true;
}

std::size_t KartConfiguravel::quantidadeSensoresExtras() const {
    return sensoresExtras_.size();
}
