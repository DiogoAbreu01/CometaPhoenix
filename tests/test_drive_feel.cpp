#include "GamePhysics.hpp"
#include <cmath>
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"DRIVE_FEEL_FAIL line "<<__LINE__<<": "<<#x<<"\n";return 1;}}while(false)
int main(){
 phoenix::Game normal(42),drift(42);
 phoenix::Input accelerate{};accelerate.throttle=true;
 normal.setAutopilot(true);drift.setAutopilot(true);
 for(int i=0;i<210;++i){normal.update({},.02);drift.update({},.02);}
 normal.setAutopilot(false);drift.setAutopilot(false);
 CHECK(normal.vehicle().speed>22.);
 phoenix::Input steer{};steer.throttle=true;steer.left=true;
 phoenix::Input handbrake=steer;handbrake.drift=true;
 for(int i=0;i<32;++i){normal.update(steer,.02);drift.update(handbrake,.02);}
 CHECK(drift.driftAmount()>.7);
 CHECK(drift.driftPoints()>0.);
 CHECK(std::isfinite(drift.vehicle().x) && std::isfinite(drift.vehicle().y));
 CHECK(std::abs(drift.vehicle().angle-normal.vehicle().angle)>.02);
 for(int i=0;i<40;++i)drift.update(accelerate,.02);
 CHECK(drift.driftAmount()<.01);
 drift.restart();CHECK(drift.driftPoints()==0. && drift.driftAmount()==0.);
 drift.setAutopilot(true);for(int i=0;i<100;++i)drift.update(handbrake,.02);
 CHECK(drift.driftAmount()==0.);
 std::cout<<"DRIVE_FEEL_PASS\n";
}
