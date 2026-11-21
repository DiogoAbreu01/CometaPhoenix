#pragma once

#include <algorithm>
#include <cmath>

namespace phoenix {

struct TireState {
    double temperatureC{}, wear{}, brakeTempC{}, brakeFade{}, gripFactor{};
};

class TireDynamics {
public:
    void reset(double ambientC = 25) noexcept {
        const double ambient = bounded(ambientC, -50, 60, 25);
        state_ = {ambient, 0, ambient, 0, gripAt(ambient, 0, 0, 0)};
    }

    void update(double dt, double speedKmh, double lateralG, double brakeInput,
                double ambientC, double rainAmount, double offroad) noexcept {
        if (!std::isfinite(dt) || dt <= 0) {
            return;
        }
        dt = std::min(dt, 0.2);
        const double speed = bounded(speedKmh, 0, 400) / 180;
        const double cornering = std::abs(bounded(lateralG, -3, 3));
        const double brake = bounded(brakeInput, 0, 1);
        const double ambient = bounded(ambientC, -50, 60, 25);
        const double rain = bounded(rainAmount, 0, 1);
        const double dirt = bounded(offroad, 0, 1);

        const double tireHeat = speed * (3.5 * cornering * cornering + 5 * brake);
        const double tireCooling = 0.055 + 0.02 * speed + 0.12 * rain;
        state_.temperatureC = std::clamp(state_.temperatureC + dt *
            (tireHeat - tireCooling * (state_.temperatureC - ambient)), -50.0, 200.0);

        // Combined cornering and braking demand serve as a proxy for slip.
        const double slip = std::max(0.0, cornering + 0.8 * brake + 0.4 * dirt - 0.8);
        state_.wear = std::min(1.0, state_.wear + dt * (0.000025 * speed + 0.00025 * speed * slip * slip));

        const double brakeHeat = 105 * speed * brake;
        const double brakeCooling = 0.065 + 0.025 * speed + 0.1 * rain;
        state_.brakeTempC = std::clamp(state_.brakeTempC + dt *
            (brakeHeat - brakeCooling * (state_.brakeTempC - ambient)), -50.0, 1000.0);
        state_.brakeFade = 0.75 * std::clamp((state_.brakeTempC - 350) / 450, 0.0, 1.0);
        state_.gripFactor = gripAt(state_.temperatureC, state_.wear, rain, dirt);
    }

    void seedWear(double wear) noexcept {
        if(!std::isfinite(wear))return;
        state_.wear=std::clamp(wear,0.,1.);
        state_.gripFactor=gripAt(state_.temperatureC,state_.wear,0.,0.);
    }
    const TireState& state() const noexcept { return state_; }
    double grip() const noexcept { return state_.gripFactor; }
    double brakingEfficiency() const noexcept { return 1 - state_.brakeFade; }

private:
    static double bounded(double value, double lo, double hi, double fallback = 0) noexcept {
        return std::isfinite(value) ? std::clamp(value, lo, hi) : fallback;
    }

    static double gripAt(double temperature, double wear, double rain, double dirt) noexcept {
        constexpr double optimumC = 75;
        const double thermalLoss = temperature < optimumC
            ? 0.003 * (optimumC - temperature)
            : 0.005 * (temperature - optimumC);
        return std::clamp((1.05 - thermalLoss) * (1 - 0.45 * wear) *
            (1 - 0.30 * rain) * (1 - 0.35 * dirt), 0.15, 1.05);
    }

    TireState state_{25, 0, 25, 0, gripAt(25, 0, 0, 0)};
};

} // namespace phoenix
