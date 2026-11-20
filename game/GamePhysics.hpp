#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "cometa/Simulation.hpp"

namespace phoenix {
enum class Circuit { Forest, Alpine, Coast };
const char* circuitName(Circuit circuit) noexcept;
double circuitRadius(Circuit circuit,double angle) noexcept;
enum class Weather { Sunny, Rain, Snow, Storm, Fog };
const char* weatherName(Weather weather) noexcept;
struct ImpactAssessment { const char* component; const char* kind; double deltaVelocityMs; double durationS; };
ImpactAssessment assessImpact(double speedKmh,double incidence,bool left,bool front) noexcept;
struct Input { bool throttle{}, brake{}, left{}, right{}, reset{}; };
struct Vehicle { double x{1.92}, y{0}, angle{1.5707963267948966}, speed{}, lateral{}; int lap{}; };
struct GhostPose {double x{},y{},angle{};};
struct Climate { double grip{}, visibility{}, precipitation{}, temperature{}, wind{}, drag{}; };
Climate climate(Weather w) noexcept;
struct Particle { float x{},y{},life{},size{}; };
class Game {
public:
    explicit Game(std::uint32_t seed=42);
    void update(const Input& input, double dt);
    void setCircuit(Circuit circuit);
    Circuit circuit() const noexcept { return circuit_; }
    void setWeather(Weather w) noexcept { weather_ = w; }
    void restart(std::uint32_t seed = 42);
    void requestDamage(const std::string& component="carenagem", double severity=10);
    void requestCollision(double speedKmh,double incidence,bool left,bool front);
    Weather weather() const noexcept { return weather_; }
    const Vehicle& vehicle() const noexcept { return vehicle_; }
    const cometa::Simulacao& simulator() const noexcept { return *sim_; }
    double worldTime() const noexcept { return worldTime_; }
    double cinematic() const noexcept { return cinematic_; }
    bool garage() const noexcept;
    bool hasDamage() const noexcept { return sim_->danos().tamanho()>0; }
    bool abandoned() const noexcept { return sim_->fase() == cometa::Fase::Abandono; }
    double repairFraction() const noexcept;
    double trackOffset() const noexcept;
    double lapProgress() const noexcept;
    double currentLapTime() const noexcept { return currentLapTime_; }
    double lastLapTime() const noexcept { return lastLapTime_; }
    double bestLapTime() const noexcept { return bestLapTime_; }
    bool hasGhost() const noexcept;
    GhostPose ghostPose() const noexcept;
    bool validLap() const noexcept { return validLap_; }
    int validCompletedLaps() const noexcept { return validCompletedLaps_; }
    int targetLaps() const noexcept { return targetLaps_; }
    void setTargetLaps(int laps);
    bool finished() const noexcept { return targetLaps_>0 && validCompletedLaps_>=targetLaps_; }
    void setAutopilot(bool b) noexcept { autopilot_=b; }
    bool autopilot() const noexcept { return autopilot_; }
    std::string status() const;
private:
    struct GhostSample { GhostPose pose; double time; };
    static constexpr std::size_t maxGhostSamples_ = 4096;
    void startGhostLap();
    void recordGhostSample(bool finish);
    std::vector<GhostSample> ghostLap_, bestGhostLap_;
    double ghostSampleInterval_{};
    std::unique_ptr<cometa::Simulacao> sim_;
    Vehicle vehicle_{};
    Weather weather_{Weather::Sunny};
    Circuit circuit_{Circuit::Forest};
    std::uint32_t seed_{42};
    double worldTime_{0}, cinematic_{0}, lastImpact_{-100};
    bool autopilot_{false};
    double lastPolarAngle_{0}, orbitTurns_{0};
    double currentLapTime_{}, lastLapTime_{}, bestLapTime_{};
    bool validLap_{true};
    int validCompletedLaps_{0}, targetLaps_{3};
};
}
