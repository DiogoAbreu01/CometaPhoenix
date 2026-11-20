#pragma once

#include "GamePhysics.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace phoenix {

class RivalAI {
public:
    void reset(Circuit circuit, Weather weather, double speedFactor, double startingAngle) {
        circuit_ = circuit;
        speedFactor_ = std::isfinite(speedFactor) ? speedFactor : 1.0;
        startingAngle_ = std::isfinite(startingAngle) ? std::remainder(startingAngle, turn_) : 0.0;
        progress_ = 0.0;
        raceTime_ = 0.0;
        setWeather(weather);
    }

    void setWeather(Weather weather) {
        const double weatherSpeed = 55.0 * (0.55 + 0.45 * climate(weather).grip);
        speed_ = std::clamp(weatherSpeed * speedFactor_, 30.0, 65.0) / 3.6;
    }

    void update(double dt, int targetLaps) {
        if (!std::isfinite(dt) || dt <= 0.0 || finished(targetLaps)) {
            return;
        }

        const double finishProgress = targetLaps > 0
            ? targetLaps * turn_ : std::numeric_limits<double>::infinity();
        for (double elapsed = 0.0; elapsed < dt;) {
            const double step = std::min(dt - elapsed, 1.0 / 120.0);
            const double angle = trackAngle();
            const double distance = speed_ * step;
            // Midpoint integration of d(angle)/dt = speed / (ds/d(angle)).
            const double midpoint = angle + 0.5 * distance / metersPerRadian(angle);
            const double advance = distance / metersPerRadian(midpoint);
            const double remaining = finishProgress - progress_;
            if (advance >= remaining) {
                progress_ = finishProgress;
                raceTime_ += elapsed + step * (remaining / advance);
                return;
            }
            progress_ += advance;
            elapsed += step;
        }
        raceTime_ += dt;
    }

    GhostPose pose() const noexcept {
        const double angle = trackAngle();
        GhostPose center = trackPoint(angle);
        const GhostPose before = trackPoint(angle - sampleAngle_);
        const GhostPose after = trackPoint(angle + sampleAngle_);
        center.angle = std::atan2(after.y - before.y, after.x - before.x);
        return center;
    }

    int laps() const noexcept {
        const int maxLaps = std::numeric_limits<int>::max();
        const int estimate = static_cast<int>(std::min(progress_ / turn_, double(maxLaps)));
        // Use the same threshold as finished() at floating-point lap boundaries.
        if (estimate < maxLaps && finished(estimate + 1)) {
            return estimate + 1;
        }
        return estimate > 0 && !finished(estimate) ? estimate - 1 : estimate;
    }

    bool finished(int targetLaps) const noexcept {
        return targetLaps > 0 && progress_ >= targetLaps * turn_;
    }

    double raceTime() const noexcept { return raceTime_; }

    // Radians traveled from the grid position, including completed laps.
    double totalProgress() const noexcept { return progress_; }

private:
    static constexpr double turn_ = 6.28318530717958647692;
    static constexpr double sampleAngle_ = 0.0001;
    static constexpr double metersPerTrackUnit_ = 15.0; // GamePhysics world scale

    double trackAngle() const noexcept {
        return startingAngle_ + std::fmod(progress_, turn_);
    }

    GhostPose trackPoint(double angle) const noexcept {
        const double radius = circuitRadius(circuit_, angle);
        return {radius * std::cos(angle), radius * std::sin(angle), 0.0};
    }

    double metersPerRadian(double angle) const noexcept {
        const double radius = circuitRadius(circuit_, angle);
        const double slope = (circuitRadius(circuit_, angle + sampleAngle_) -
                              circuitRadius(circuit_, angle - sampleAngle_)) / (2.0 * sampleAngle_);
        return metersPerTrackUnit_ * std::hypot(radius, slope);
    }

    Circuit circuit_{Circuit::Forest};
    double speedFactor_{1.0};
    double speed_{55.0 / 3.6};
    double startingAngle_{};
    double progress_{};
    double raceTime_{};
};

} // namespace phoenix
