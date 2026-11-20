#include "GamePhysics.hpp"
#include "RivalAI.hpp"
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc,char** argv){
 using namespace phoenix;
 int minutes=2;
 if(argc==3 && std::string(argv[1])=="--minutes")minutes=std::atoi(argv[2]);
 if(minutes<1 || minutes>60){std::cerr<<"Invalid duration\n";return 2;}
 constexpr double dt=1.0/60.0;
 for(auto circuit:{Circuit::Forest,Circuit::Alpine,Circuit::Coast}){
  Game game(42);game.setCircuit(circuit);game.setTargetLaps(0);game.setAutopilot(true);
  std::array<RivalAI,2> rivals{};
  rivals[0].setDifficulty(RivalDifficulty::Hard);
  rivals[0].reset(circuit,Weather::Rain,1.0,-.09);
  rivals[1].setDifficulty(RivalDifficulty::Normal);
  rivals[1].reset(circuit,Weather::Rain,.88,-.18);
  double prevWorld=0.;std::uint64_t restarts=0,contacts=0;
  for(int frame=0;frame<minutes*60*60;++frame){
   if(frame%(60*40)==0){
      const auto condition=static_cast<Weather>((frame/(60*40))%5);
      game.setWeather(condition);
      for(auto& r:rivals)r.setWeather(condition);
   }
   game.update({},dt);
   const std::array<GhostPose,2> prior{rivals[0].pose(),rivals[1].pose()};
   const auto& v=game.vehicle();
   for(int i=0;i<2;++i){
      rivals[i].setCompetitors({v.x,v.y,v.angle},prior[1-i]);
      rivals[i].update(dt,0);
   }
   for(int i=0;i<2;++i){
      const auto hit=game.collideRival(rivals[i].pose(),prior[i],dt);
      if(hit.impulseApplied){++contacts;rivals[i].applyContact(hit.closingSpeedMs);}
   }
   const auto& car=game.vehicle();
   const auto& h=game.handling();
   if(!std::isfinite(car.x)||!std::isfinite(car.y)||!std::isfinite(car.speed)||
      !std::isfinite(car.angle)||!std::isfinite(game.worldTime())||
      !std::isfinite(h.tireWear)||!std::isfinite(h.tireTemperatureC)||
      !std::isfinite(h.brakeTemperatureC)||!std::isfinite(h.effectiveGrip)||
      car.speed<-1e-9||car.speed>102.001||h.tireWear<0.||h.tireWear>1.||
      h.effectiveGrip<0.||h.effectiveGrip>1.1){
     std::cerr<<"Invalid physical state frame "<<frame<<" circuit "<<(int)circuit<<'\n';return 1;
   }
   if(game.worldTime()+1e-8<prevWorld){std::cerr<<"Non-monotonic time\n";return 1;}
   prevWorld=game.worldTime();
   if(game.abandoned()){game.restart(42+static_cast<unsigned>(restarts++));prevWorld=0.;}
  }
  std::cout<<"STRESS "<<circuitName(circuit)<<" simulated_minutes="<<minutes<<" contacts="<<contacts
           <<" restarts="<<restarts<<" tires="<<game.handling().tireWear<<" lap="<<game.vehicle().lap<<"\n";
 }
 std::cout<<"STRESS_TESTS_PASS\n";
}
