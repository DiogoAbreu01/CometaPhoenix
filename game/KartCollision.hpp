#pragma once
#include "GamePhysics.hpp"
#include <algorithm>
#include <cmath>

namespace phoenix {
struct ContactResolution {
    bool overlap{};
    double nx{},ny{},penetrationUnits{},closingSpeedMs{};
};

// Top-down swept approximation: two convex kart footprints as discs.
// World units are 15 meters; speed in km/h. Does not allocate.
inline ContactResolution detectKartContact(const Vehicle& player,const GhostPose& rival,
                                           const GhostPose& previousRival,double dt) noexcept {
    if(!(dt>0. && dt<=.2) || !std::isfinite(dt))return {};
    constexpr double minDistance=.174;  // combined approx footprint, 2.61 m
    const double dx=player.x-rival.x,dy=player.y-rival.y;
    const double dist=std::hypot(dx,dy);
    if(!std::isfinite(dist) || dist>=minDistance)return {};
    const double nx=dist>1e-7?dx/dist:std::cos(player.angle+1.57);
    const double ny=dist>1e-7?dy/dist:std::sin(player.angle+1.57);
    const double playerVx=std::cos(player.angle)*player.speed/3.6;
    const double playerVy=std::sin(player.angle)*player.speed/3.6;
    const double rivalVx=(rival.x-previousRival.x)*15./dt;
    const double rivalVy=(rival.y-previousRival.y)*15./dt;
    const double closing=std::max(0.,(rivalVx-playerVx)*nx+(rivalVy-playerVy)*ny);
    return {true,nx,ny,minDistance-dist,std::clamp(closing,0.,30.)};
}
} // namespace phoenix
