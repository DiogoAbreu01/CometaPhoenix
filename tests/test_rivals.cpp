#include "RivalAI.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

#define REQUIRE(condition) do { if (!(condition)) { \
    std::cerr << "Line " << __LINE__ << ": " << #condition << '\n'; return 1; \
} } while (false)

namespace {

bool samePose(phoenix::GhostPose a, phoenix::GhostPose b) {
    return a.x == b.x && a.y == b.y && a.angle == b.angle;
}

phoenix::GhostPose trackPose(phoenix::Circuit circuit, double angle, double offset = 0.0) {
    const double radius = phoenix::circuitRadius(circuit, angle) + offset;
    return {radius * std::cos(angle), radius * std::sin(angle), 0.0};
}

} // namespace

int main() {
    using namespace phoenix;
    constexpr double dt = 1.0 / 60.0;
    constexpr double turn = 6.28318530717958647692;
    for (Circuit circuit : {Circuit::Forest, Circuit::Alpine, Circuit::Coast, Circuit::Sakura}) {
        double sunnyTime = 0.0;
        for (Weather weather : {Weather::Sunny, Weather::Rain, Weather::Snow,
                                Weather::Storm, Weather::Fog}) {
            for (double start : {-0.08, -0.16}) {
                RivalAI first, second, slower;
                first.reset(circuit, weather, 1.0, start);
                second.reset(circuit, weather, 1.0, start);
                slower.reset(circuit, weather, 0.8, start);
                REQUIRE(first.totalProgress() == 0.0 && first.raceTime() == 0.0);
                REQUIRE(std::abs(std::atan2(first.pose().y, first.pose().x) - start) < 1e-12);
                const auto startingPose = first.pose();

                double previousProgress = 0.0;
                int previousLaps = 0;
                bool crossedZero = false;
                for (int frame = 0; frame < 5400 && !slower.finished(1); ++frame) {
                    first.update(dt, 1);
                    second.update(dt, 1);
                    slower.update(dt, 1);
                    const auto pose = first.pose();
                    REQUIRE(samePose(pose, second.pose()));
                    REQUIRE(first.raceTime() == second.raceTime());
                    REQUIRE(first.totalProgress() == second.totalProgress());
                    REQUIRE(first.laps() == second.laps());
                    REQUIRE(std::isfinite(pose.x) && std::isfinite(pose.y) && std::isfinite(pose.angle));
                    REQUIRE(std::isfinite(first.raceTime()) && std::isfinite(first.totalProgress()));
                    const double polar = std::atan2(pose.y, pose.x);
                    REQUIRE(std::abs(std::hypot(pose.x, pose.y) - circuitRadius(circuit, polar)) < 1e-10);
                    if (first.totalProgress() > previousProgress) {
                        const double ahead = polar + 0.001;
                        const double radiusAhead = circuitRadius(circuit, ahead);
                        const double tangent = std::atan2(radiusAhead * std::sin(ahead) - pose.y,
                                                          radiusAhead * std::cos(ahead) - pose.x);
                        REQUIRE(std::abs(std::remainder(pose.angle - tangent, turn)) < 0.004);
                    }
                    REQUIRE(first.laps() >= previousLaps);
                    REQUIRE(first.totalProgress() >= previousProgress);
                    if (!crossedZero && polar >= 0.0) {
                        REQUIRE(first.laps() == 0);
                        crossedZero = true;
                    }
                    previousProgress = first.totalProgress();
                    previousLaps = first.laps();
                }
                REQUIRE(crossedZero);
                REQUIRE(first.finished(1) && slower.finished(1));
                REQUIRE(first.laps() == 1 && first.totalProgress() == turn);
                REQUIRE(samePose(first.pose(), startingPose));
                const double finishTime = first.raceTime();
                first.update(1.0, 1);
                REQUIRE(first.raceTime() == finishTime && first.totalProgress() == turn);
                REQUIRE(first.raceTime() < slower.raceTime() && slower.raceTime() < 90.0);
                if (start == -0.08) {
                    if (weather == Weather::Sunny) {
                        sunnyTime = first.raceTime();
                    } else {
                        REQUIRE(first.raceTime() > sunnyTime);
                    }
                }
            }
        }
    }

    RivalAI free, sunny;
    free.reset(Circuit::Forest, Weather::Sunny, 1.0, -0.16);
    free.update(1.0, 0);
    sunny = free;
    const double pausedTime = free.raceTime();
    const double pausedProgress = free.totalProgress();
    const auto pausedPose = free.pose();
    free.setWeather(Weather::Snow);
    REQUIRE(free.raceTime() == pausedTime && free.totalProgress() == pausedProgress);
    REQUIRE(samePose(free.pose(), pausedPose));
    free.update(1.0, 0);
    sunny.update(1.0, 0);
    REQUIRE(free.totalProgress() > pausedProgress && free.totalProgress() < sunny.totalProgress());
    free.update(150.0, 0);
    REQUIRE(free.laps() >= 2 && !free.finished(0));
    REQUIRE(free.raceTime() == 152.0);

    sunny.update(150.0, 2);
    REQUIRE(sunny.finished(2) && sunny.laps() == 2);
    REQUIRE(sunny.totalProgress() == 2.0 * turn && sunny.raceTime() < 152.0);
    sunny.update(1.0, 0);
    REQUIRE(sunny.totalProgress() > 2.0 * turn && !sunny.finished(0));

    constexpr Circuit circuit = Circuit::Forest;
    for (double passingSide : {-1.0, 1.0}) {
        RivalAI passing, repeat, clear;
        passing.reset(circuit, Weather::Sunny, 1.0, -0.06);
        repeat = clear = passing;
        const auto grid = passing.pose();
        const auto player = trackPose(circuit, 0.015);
        const auto other = trackPose(circuit, 0.015, -passingSide * 0.20);
        clear.setCompetitors(player, other, false);
        double previousOffset = 0.0;
        double peakOffset = 0.0;
        for (int frame = 0; frame < 120; ++frame) {
            passing.setCompetitors(player, other);
            repeat.setCompetitors(player, other);
            passing.update(dt, 1);
            repeat.update(dt, 1);
            clear.update(dt, 1);
            const auto pose = passing.pose();
            REQUIRE(samePose(pose, repeat.pose()));
            REQUIRE(passing.totalProgress() == repeat.totalProgress());
            REQUIRE(passing.raceTime() == repeat.raceTime());
            const double offset = std::hypot(pose.x, pose.y) -
                circuitRadius(circuit, std::atan2(pose.y, pose.x));
            REQUIRE(std::abs(offset) <= 0.23 + 1e-12);
            REQUIRE(std::abs(offset - previousOffset) <= 0.8 * dt + 1e-12);
            peakOffset = std::max(peakOffset, passingSide * offset);
            previousOffset = offset;
            if (frame == 5) {
                REQUIRE(passing.totalProgress() < clear.totalProgress());
            }
        }
        REQUIRE(peakOffset > 0.16);
        REQUIRE(std::abs(previousOffset) < 1e-12);
        REQUIRE(passing.totalProgress() > 0.075);
        passing.update(90.0, 1);
        REQUIRE(passing.finished(1) && samePose(passing.pose(), grid));
    }

    RivalAI easy, normal, hard;
    REQUIRE(normal.difficulty() == RivalDifficulty::Normal);
    easy.setDifficulty(RivalDifficulty::Easy);
    hard.setDifficulty(RivalDifficulty::Hard);
    for (auto* rival : {&easy, &normal, &hard}) {
        rival->reset(circuit, Weather::Rain, 1.0, 0.0);
        rival->update(90.0, 1);
        REQUIRE(rival->finished(1));
    }
    REQUIRE(easy.difficulty() == RivalDifficulty::Easy);
    REQUIRE(hard.difficulty() == RivalDifficulty::Hard);
    REQUIRE(hard.raceTime() < normal.raceTime() && normal.raceTime() < easy.raceTime());
    RivalAI collisionTest,collisionBaseline;
    collisionTest.reset(Circuit::Forest,Weather::Sunny,1.0,0.0);
    collisionBaseline=collisionTest;
    collisionTest.applyContact(9.0);
    collisionTest.update(.8,0);
    collisionBaseline.update(.8,0);
    REQUIRE(collisionTest.totalProgress()<collisionBaseline.totalProgress());
    collisionTest.applyContact(std::numeric_limits<double>::quiet_NaN());
    REQUIRE(std::isfinite(collisionTest.pose().x));
    std::cout << "RivalAI regression tests passed\n";
}
