#include "TireDynamics.hpp"

#include <initializer_list>
#include <iostream>
#include <limits>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "Line " << __LINE__ << ": " << #condition << '\n'; return 1; \
} } while (false)

namespace {

bool sameState(const phoenix::TireState& a, const phoenix::TireState& b) {
    return a.temperatureC == b.temperatureC && a.wear == b.wear &&
        a.brakeTempC == b.brakeTempC && a.brakeFade == b.brakeFade &&
        a.gripFactor == b.gripFactor;
}

bool inRange(double value, double low, double high) {
    return value >= low && value <= high;
}

} // namespace

int main() {
    using phoenix::TireDynamics;
    constexpr double dt = 0.05;
    TireDynamics dry, wet;
    dry.reset();
    const auto fresh = dry.state();
    for (int i = 0; i < 1200; ++i) {
        dry.update(dt, 0, 0, 0, 25, 0, 0);
    }
    REQUIRE(sameState(dry.state(), fresh));
    REQUIRE(inRange(dry.grip(), 0.8, 1.1));

    for (int i = 0; i < 200; ++i) {
        dry.update(dt, 180, 1, 1, 25, 0, 0);
        wet.update(dt, 180, 1, 1, 25, 1, 0);
    }
    const auto hot = dry.state();
    REQUIRE(hot.temperatureC > fresh.temperatureC);
    REQUIRE(hot.brakeFade > 0.25);
    REQUIRE(dry.brakingEfficiency() == 1 - hot.brakeFade);
    REQUIRE(wet.state().temperatureC < hot.temperatureC);
    REQUIRE(wet.state().brakeTempC < hot.brakeTempC);
    REQUIRE(wet.grip() < dry.grip());
    for (int i = 0; i < 1200; ++i) {
        dry.update(dt, 0, 0, 0, 25, 0, 0);
    }
    REQUIRE(inRange(dry.state().temperatureC, 25, 25 + 0.05 * (hot.temperatureC - 25)));
    REQUIRE(dry.state().brakeTempC < 40);
    REQUIRE(dry.brakingEfficiency() == 1);

    TireDynamics first, second;
    REQUIRE(first.grip() == fresh.gripFactor);
    for (int i = 0; i < 2000; ++i) {
        const double speed = 40 + i % 231;
        const double lateralG = (i % 41 - 20) / 10.0;
        const double brake = (i % 19) / 18.0;
        const double ambient = i % 61 - 10;
        const double rain = (i % 11) / 10.0;
        const double offroad = (i % 7) / 6.0;
        first.update(dt, speed, lateralG, brake, ambient, rain, offroad);
        second.update(dt, speed, lateralG, brake, ambient, rain, offroad);
        const auto& state = first.state();
        REQUIRE(sameState(state, second.state()));
        REQUIRE(inRange(state.temperatureC, -50, 200));
        REQUIRE(inRange(state.wear, 0, 1));
        REQUIRE(inRange(state.brakeTempC, -50, 1000));
        REQUIRE(inRange(state.brakeFade, 0, 0.75));
        REQUIRE(inRange(first.grip(), 0.15, 1.05));
        REQUIRE(inRange(first.brakingEfficiency(), 0.25, 1));
    }

    first.reset();
    for (int i = 0; i < 1000; ++i) {
        first.update(0.2, 180, 2.5, 1, 25, 0, 0);
    }
    REQUIRE(inRange(first.state().wear, 0.25, 1));
    const double wear = first.state().wear;
    for (int i = 0; i < 3000; ++i) {
        first.update(0.2, 0, 0, 0, 25, 0, 0);
    }
    REQUIRE(first.state().wear == wear);
    REQUIRE(inRange(first.state().temperatureC, 25, 25.001));
    REQUIRE(first.grip() < fresh.gripFactor - 0.1);

    first.reset(60);
    second.reset(-20);
    REQUIRE(first.grip() > fresh.gripFactor);
    REQUIRE(second.grip() < fresh.gripFactor);
    first.reset();
    second.reset();
    first.update(10, 180, 1, 1, 25, 0, 0);
    second.update(0.2, 180, 1, 1, 25, 0, 0);
    const auto capped = first.state();
    REQUIRE(sameState(capped, second.state()));
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    for (double invalidDt : {0.0, -dt, nan, infinity}) {
        first.update(invalidDt, 400, 3, 1, 60, 1, 1);
        REQUIRE(sameState(first.state(), capped));
    }
    first.reset(nan);
    first.update(dt, nan, infinity, nan, -infinity, nan, infinity);
    REQUIRE(sameState(first.state(), fresh));

    std::cout << "Tire dynamics tests passed\n";
}
