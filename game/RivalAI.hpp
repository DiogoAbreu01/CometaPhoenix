#pragma once

#include "GamePhysics.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace phoenix {

enum class RivalDifficulty { Easy, Normal, Hard };

class RivalAI {
public:
    void reset(Circuit circuit, Weather weather, double speedFactor, double startingAngle) {
        circuit_ = circuit;
        speedFactor_ = std::isfinite(speedFactor) ? speedFactor : 1.0;
        startingAngle_ = std::isfinite(startingAngle) ? std::remainder(startingAngle, turn_) : 0.0;
        progress_ = 0.0;
        raceTime_ = 0.0;
        laneOffset_ = 0.0;
        competitorsEnabled_ = false;
        setWeather(weather);
        speed_ = cornerSpeed(startingAngle_);
    }

    void applyContact(double closingSpeedMs) noexcept {
        if(!std::isfinite(closingSpeedMs)||closingSpeedMs<=0.)return;
        speed_=std::max(.5,speed_*(1.-std::clamp(closingSpeedMs*.034,.04,.48)));
    }
    void setWeather(Weather weather) {
        grip_ = climate(weather).grip;
    }

    void setDifficulty(RivalDifficulty difficulty) noexcept { difficulty_ = difficulty; }
    RivalDifficulty difficulty() const noexcept { return difficulty_; }

    void setCompetitors(GhostPose player, GhostPose other, bool enabled = true) noexcept {
        competitors_ = {player, other};
        competitorsEnabled_ = enabled;
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
            const double remaining = finishProgress - progress_;
            const auto traffic = nearbyTraffic(angle);
            // Leave room to merge back onto the grid line before crossing the finish.
            const double targetLane = remaining < 0.20 ? 0.0 : preferredLane(traffic);
            laneOffset_ += std::clamp(targetLane - laneOffset_, -0.8 * step, 0.8 * step);
            const double cornerTarget = cornerSpeed(angle);
            double targetSpeed = cornerTarget;
            for (const auto& competitor : traffic) {
                if (competitor.ahead && std::abs(competitor.offset - laneOffset_) < clearance_) {
                    const double speedScale = std::clamp(competitor.distance / detectionRange_, 0.45, 1.0);
                    targetSpeed = std::min(targetSpeed, cornerTarget * speedScale);
                }
            }
            speed_ += std::clamp(targetSpeed - speed_, -12.0 * step, 5.0 * step);
            const double distance = speed_ * step;
            // Midpoint integration of d(angle)/dt = speed / (ds/d(angle)).
            const double midpoint = angle + 0.5 * distance / metersPerRadian(angle);
            const double advance = distance / metersPerRadian(midpoint);
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
    static constexpr double laneLimit_ = 0.23;
    static constexpr double clearance_ = 0.16;
    static constexpr double detectionRange_ = 0.7;

    struct NearbyCompetitor {
        double distance{};
        double offset{1.0};
        bool ahead{};
    };

    std::array<NearbyCompetitor, 2> nearbyTraffic(double angle) const noexcept {
        std::array<NearbyCompetitor, 2> traffic{};
        if (!competitorsEnabled_) {
            return traffic;
        }
        const GhostPose position = trackPoint(angle);
        for (std::size_t i = 0; i < competitors_.size(); ++i) {
            const auto& competitor = competitors_[i];
            const double distance = std::hypot(competitor.x - position.x, competitor.y - position.y);
            if (!(distance < detectionRange_)) {
                continue;
            }
            const double polar = std::atan2(competitor.y, competitor.x);
            const double ahead = std::remainder(polar - angle, turn_);
            // Hold the passing lane while the other car is alongside or just behind.
            if (ahead > 0.0 || (laneOffset_ != 0.0 && distance < 0.25)) {
                traffic[i] = {distance, std::hypot(competitor.x, competitor.y) -
                                        circuitRadius(circuit_, polar), ahead > 0.0};
            }
        }
        return traffic;
    }

    static double laneClearance(const std::array<NearbyCompetitor, 2>& traffic,
                                double lane) noexcept {
        return std::min(std::abs(traffic[0].offset - lane), std::abs(traffic[1].offset - lane));
    }

    static double preferredLane(const std::array<NearbyCompetitor, 2>& traffic) noexcept {
        if (laneClearance(traffic, 0.0) >= clearance_) {
            return 0.0;
        }
        return laneClearance(traffic, laneLimit_) >= laneClearance(traffic, -laneLimit_)
            ? laneLimit_ : -laneLimit_;
    }

    double cruiseSpeed() const noexcept {
        double pace = 1.0;
        switch (difficulty_) {
        case RivalDifficulty::Easy: pace = 0.85; break;
        case RivalDifficulty::Normal: break;
        case RivalDifficulty::Hard: pace = 1.15; break;
        }
        return std::clamp(55.0 * (0.55 + 0.45 * grip_) * speedFactor_, 30.0, 65.0)
            * pace / 3.6;
    }

    double cornerSpeed(double angle) const noexcept {
        const double radius = circuitRadius(circuit_, angle) + laneOffset_;
        const double before = circuitRadius(circuit_, angle - sampleAngle_) + laneOffset_;
        const double after = circuitRadius(circuit_, angle + sampleAngle_) + laneOffset_;
        const double slope = (after - before) / (2.0 * sampleAngle_);
        const double second = (after - 2.0 * radius + before) / (sampleAngle_ * sampleAngle_);
        const double length = std::hypot(radius, slope);
        const double curvature = std::abs(radius * radius + 2.0 * slope * slope - radius * second)
            / (metersPerTrackUnit_ * length * length * length);
        const double cruise = cruiseSpeed();
        // Blend cruise pace with the grip-limited corner speed.
        return cruise / std::sqrt(1.0 + curvature * cruise * cruise / (22.0 * grip_));
    }

    double trackAngle() const noexcept {
        return startingAngle_ + std::fmod(progress_, turn_);
    }

    GhostPose trackPoint(double angle) const noexcept {
        const double radius = circuitRadius(circuit_, angle) + laneOffset_;
        return {radius * std::cos(angle), radius * std::sin(angle), 0.0};
    }

    double metersPerRadian(double angle) const noexcept {
        const double radius = circuitRadius(circuit_, angle) + laneOffset_;
        const double slope = (circuitRadius(circuit_, angle + sampleAngle_) -
                              circuitRadius(circuit_, angle - sampleAngle_)) / (2.0 * sampleAngle_);
        return metersPerTrackUnit_ * std::hypot(radius, slope);
    }

    Circuit circuit_{Circuit::Forest};
    RivalDifficulty difficulty_{RivalDifficulty::Normal};
    std::array<GhostPose, 2> competitors_{};
    bool competitorsEnabled_{};
    double grip_{1.0};
    double laneOffset_{};
    double speedFactor_{1.0};
    double speed_{55.0 / 3.6};
    double startingAngle_{};
    double progress_{};
    double raceTime_{};
};

} // namespace phoenix
