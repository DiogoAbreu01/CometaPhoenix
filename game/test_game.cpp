#include "GamePhysics.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#define CHECK(x) do{if(!(x)){std::cerr<<"ERRO em "<<__LINE__<<": "<<#x<<"\n";std::exit(1);}}while(false)
int main(){using namespace phoenix;
 CHECK(climate(Weather::Rain).grip<climate(Weather::Sunny).grip);
 CHECK(climate(Weather::Snow).grip<climate(Weather::Rain).grip);
 CHECK(climate(Weather::Fog).visibility<climate(Weather::Storm).visibility);
 Game a(42),b(42);a.setAutopilot(true);b.setAutopilot(true);
 for(int i=0;i<200;i++){a.update({},.02);b.update({},.02);} CHECK(a.vehicle().x==b.vehicle().x);CHECK(a.vehicle().speed>0);
 Game sunny(42), snow(42);sunny.setWeather(Weather::Sunny);snow.setWeather(Weather::Snow);
 for(int i=0;i<100;i++){sunny.update({true,false,true,false},.02);snow.update({true,false,true,false},.02);}
 CHECK(std::fabs(sunny.vehicle().angle-snow.vehicle().angle)>.02);
 Game ghost;
 const auto emptyGhost=ghost.ghostPose();
 CHECK(!ghost.hasGhost());CHECK(emptyGhost.x==0. && emptyGhost.y==0. && emptyGhost.angle==0.);
 ghost.setAutopilot(true);
 Vehicle beforeWrap{}, afterWrap{};
 double wrapTime=0.;
 for(int i=0;i<15000 && ghost.vehicle().lap==0;++i){
     CHECK(!ghost.hasGhost());
     const Vehicle previous=ghost.vehicle();
     const double previousTime=ghost.currentLapTime();
     ghost.update({},.02);
     if(previous.angle<3.141592653589793 && ghost.vehicle().angle>3.141592653589793 && wrapTime==0.){
         beforeWrap=previous;
         afterWrap=ghost.vehicle();
         wrapTime=previousTime+.01;
     }
 }
 CHECK(ghost.vehicle().lap==1);CHECK(ghost.hasGhost());CHECK(wrapTime>0.);
 for(int i=0;i<1000 && ghost.currentLapTime()+.2<wrapTime;++i) ghost.update({},.2);
 ghost.update({},wrapTime-ghost.currentLapTime());
 const auto pose=ghost.ghostPose();
 CHECK(std::isfinite(pose.x) && std::isfinite(pose.y) && std::isfinite(pose.angle));
 CHECK(std::abs(std::hypot(pose.x,pose.y)-circuitRadius(ghost.circuit(),std::atan2(pose.y,pose.x)))<.42);
 CHECK(std::abs(pose.x-(beforeWrap.x+afterWrap.x)*.5)<.10);
 CHECK(std::abs(pose.y-(beforeWrap.y+afterWrap.y)*.5)<.10);
 const double expectedAngle=(beforeWrap.angle+afterWrap.angle)*.5;
 CHECK(std::abs(std::atan2(std::sin(pose.angle-expectedAngle),std::cos(pose.angle-expectedAngle)))<.08);
 ghost.setCircuit(Circuit::Coast);CHECK(!ghost.hasGhost());
 Game denseGhost;
 denseGhost.setAutopilot(true);
 for(int i=0;i<180000 && denseGhost.vehicle().lap==0;++i) denseGhost.update({},.002);
 CHECK(denseGhost.hasGhost());CHECK(denseGhost.bestLapTime()>.002*4096);
 const double denseBest=denseGhost.bestLapTime();
 const Vehicle fasterLapStart=denseGhost.vehicle();
 CHECK(std::abs(denseGhost.ghostPose().x-circuitRadius(denseGhost.circuit(),0.))<1e-5);
 for(int i=0;i<15000 && denseGhost.vehicle().lap==1;++i) denseGhost.update({},.02);
 CHECK(denseGhost.vehicle().lap==2);CHECK(denseGhost.hasGhost());
 CHECK(denseGhost.bestLapTime()<denseBest);
 CHECK(denseGhost.bestLapTime()==denseGhost.lastLapTime());
 CHECK(std::abs(denseGhost.ghostPose().x-fasterLapStart.x)<.09);
 CHECK(std::abs(denseGhost.ghostPose().y-fasterLapStart.y)<.09);
 const auto savedTime=denseGhost.bestLapTime();
 const auto file=std::filesystem::temp_directory_path()/"cometa_phoenix_record_test.txt";
 CHECK(denseGhost.saveRecord(file));
 Game restored;CHECK(restored.loadRecord(file));CHECK(restored.hasGhost());
 CHECK(restored.bestLapTime()==savedTime);
 CHECK(std::isfinite(restored.ghostPose().x));
 restored.setCircuit(Circuit::Coast);CHECK(!restored.loadRecord(file));
 {std::ofstream out(file,std::ios::trunc);out<<"PHOENIX_GHOST_V2 0 0 nan 2\n";}
 CHECK(!restored.loadRecord(file));CHECK(!restored.hasGhost());
 std::error_code rmEc;std::filesystem::remove(file,rmEc);
 denseGhost.restart();CHECK(!denseGhost.hasGhost());
 for(auto weather:{Weather::Sunny,Weather::Rain,Weather::Snow}) {
     Game lapTest(42);lapTest.setAutopilot(true);lapTest.setWeather(weather);
     double maxOffset=0.;int steps=0;
     for(;steps<15000 && lapTest.vehicle().lap<1;++steps){
         lapTest.update({},.02);maxOffset=std::max(maxOffset,std::abs(lapTest.trackOffset()));
     }
     std::cout<<"AUTOPILOT "<<weatherName(weather)<<" laps="<<lapTest.vehicle().lap<<" max_offset="<<maxOffset<<" time="<<steps*.02<<" damage="<<lapTest.simulator().danos().tamanho()<<"\n";
     CHECK(lapTest.vehicle().lap>=1);CHECK(maxOffset<.42);CHECK(!lapTest.hasDamage());
     CHECK(lapTest.bestLapTime()>0.);CHECK(std::abs(lapTest.bestLapTime()-steps*.02)<.04);
     CHECK(lapTest.lastLapTime()==lapTest.bestLapTime());CHECK(lapTest.currentLapTime()<.03);
     const auto best=lapTest.bestLapTime();lapTest.update({},.02);CHECK(lapTest.currentLapTime()>.0);CHECK(lapTest.bestLapTime()==best);
     lapTest.restart(42);CHECK(lapTest.vehicle().lap==0);CHECK(lapTest.bestLapTime()==0.);CHECK(lapTest.currentLapTime()==0.);
 }
 for(auto circuit:{Circuit::Alpine,Circuit::Coast,Circuit::Sakura}){
     for(auto weather:{Weather::Sunny,Weather::Rain,Weather::Snow}){
         Game alternate;alternate.setCircuit(circuit);alternate.setAutopilot(true);alternate.setWeather(weather);
         double offset=0;int steps=0;for(;steps<15000&&alternate.vehicle().lap<1;++steps){alternate.update({},.02);offset=std::max(offset,std::abs(alternate.trackOffset()));}
         std::cout<<"CIRCUIT "<<circuitName(circuit)<<" "<<weatherName(weather)<<" lap="<<alternate.vehicle().lap<<" offset="<<offset<<" time="<<steps*.02<<"\n";
         CHECK(alternate.vehicle().lap==1);CHECK(!alternate.hasDamage());CHECK(alternate.bestLapTime()>0.);
     }
 }
 // A mesma velocidade pode causar tipos de dano diferentes, conforme a incidência.
 CHECK(std::string(assessImpact(60,.15,true,true).component)=="carenagem");
 CHECK(std::string(assessImpact(60,.85,true,true).component)=="suspensao");
 CHECK(std::string(assessImpact(60,.45,true,true).component)=="pneu_de");
 CHECK(std::string(assessImpact(60,.45,false,false).component)=="pneu_td");
 CHECK(std::string(assessImpact(90,1.,true,true).component)=="chassi");
 Game tire;tire.requestCollision(65,.45,false,false);CHECK(tire.hasDamage());
 CHECK(tire.simulator().danos().itens().back().componente=="pneu_td");
 CHECK(tire.simulator().danos().itens().back().causa.find("IMPACTO NA RODA")!=std::string::npos);
 for(int i=0;i<3000;++i)tire.update({},.02);
 CHECK(tire.simulator().reparo().tipo==cometa::TipoReparo::SubstituicaoRobotica);
 Game crash;crash.requestCollision(90,1.,true,true);
 for(int i=0;i<2000;++i)crash.update({},.02);
 CHECK(crash.abandoned());CHECK(crash.simulator().componente("chassi").integridade<.55);
 // Race objectives count valid laps and stop at the selected finish.
 Game finish;finish.setTargetLaps(1);finish.setAutopilot(true);
 for(int i=0;i<10000&&!finish.finished();++i)finish.update({},.02);
 CHECK(finish.finished());CHECK(finish.validCompletedLaps()==1);CHECK(finish.vehicle().speed==0.);
 const double finishTime=finish.worldTime();finish.update({},.02);CHECK(finish.worldTime()==finishTime);
 finish.restart();CHECK(!finish.finished());CHECK(finish.validCompletedLaps()==0);
 finish.setTargetLaps(0);for(int i=0;i<12000;++i)finish.update({},.02);
 CHECK(!finish.finished());CHECK(finish.validCompletedLaps()>=1);
 // Dynamic tire and brake states: finite, bounded, stable with normal usage.
 Game thermal;thermal.setAutopilot(true);
 for(int i=0;i<3500;++i)thermal.update({},.02);
 const auto state=thermal.handling();
 CHECK(std::isfinite(state.tireTemperatureC) && state.tireTemperatureC>15. && state.tireTemperatureC<170.);
 CHECK(std::isfinite(state.brakeTemperatureC) && state.brakeTemperatureC>0. && state.brakeTemperatureC<500.);
 CHECK(state.tireWear>0. && state.tireWear<.2);
 CHECK(state.effectiveGrip>.15 && state.effectiveGrip<=1.05);
 thermal.restart();CHECK(thermal.handling().tireWear==0.);
 // Contact impulse and cooldown are verified in tests/test_contacts.cpp.
 // Competitive contact must not cancel a completed circuit; it must reject a ghost record.
 Game competitive;competitive.setTargetLaps(1);competitive.setAutopilot(true);
 for(int i=0;i<100;++i)competitive.update({},.02);
 const auto cv=competitive.vehicle();
 const double ca=cv.angle;
 const GhostPose rivalContact{cv.x+std::cos(ca)*.10,cv.y+std::sin(ca)*.10,ca};
 const auto soft=competitive.collideRival(rivalContact,rivalContact,.02);
 CHECK(soft.overlap && !soft.damaged && soft.closingSpeedMs>2.4);
 CHECK(competitive.validLap());
 for(int i=0;i<15000&&!competitive.finished() && !competitive.abandoned();++i)competitive.update({},.02);
 CHECK(competitive.finished());
 CHECK(competitive.validCompletedLaps()==1);
 CHECK(!competitive.hasGhost() && competitive.bestLapTime()==0.);
 Game far;far.setAutopilot(true);
 for(int i=0;i<30000 && far.lapProgress()<.4;++i)far.update({},.02);
 CHECK(far.lapProgress()>=.4);far.setAutopilot(false);far.requestDamage("carenagem",12);
 for(int i=0;i<220;++i)far.update({},.02);
 CHECK(!far.garage());CHECK(far.simulator().fase()==cometa::Fase::RetornoBox);
 int towSteps=0;while(!far.garage() && towSteps<6000){far.update({},.02);++towSteps;}
 const double pitX=circuitRadius(Circuit::Forest,0)+.76;
 CHECK(far.garage());CHECK(std::hypot(far.vehicle().x-pitX,far.vehicle().y-.12)<1e-7);
 CHECK(far.vehicle().speed==0.);CHECK(far.vehicle().lap==0);CHECK(far.bestLapTime()==0.);
 CHECK(!far.hasGhost());
 std::cout<<"PIT_ARRIVAL elapsed="<<(220+towSteps)*.02<<" m_error="<<std::hypot(far.vehicle().x-pitX,far.vehicle().y-.12)*15<<"\n";
 Game damaged;damaged.requestDamage("carenagem",10);CHECK(damaged.hasDamage());
 for(int i=0;i<800;i++)damaged.update({},.02);
 CHECK(damaged.garage() || damaged.simulator().reparo().tipo!=cometa::TipoReparo::Nenhum);
 Game reset(17),fresh(17);reset.requestDamage("carenagem",12);double loss=reset.simulator().danos().itens().front().deltaIntegridade;reset.restart(17);CHECK(!reset.hasDamage());reset.requestDamage("carenagem",12);fresh.requestDamage("carenagem",12);CHECK(reset.simulator().danos().itens().front().deltaIntegridade==loss);CHECK(reset.simulator().danos().itens().front().deltaIntegridade==fresh.simulator().danos().itens().front().deltaIntegridade);
 Game fracture;fracture.requestDamage("chassi",18);for(int i=0;i<900;i++)fracture.update({},.02);CHECK(fracture.abandoned());CHECK(fracture.simulator().componente("chassi").integridade<.55);
 Game repair;repair.requestDamage("carenagem",12);bool sawGarage=false;for(int i=0;i<1600;i++){repair.update({},.02);sawGarage|=repair.garage();}CHECK(sawGarage);CHECK(!repair.abandoned());CHECK(repair.simulator().fase()==cometa::Fase::Corrida);CHECK(repair.simulator().recursos().agenteMl<35);CHECK(std::abs(repair.trackOffset())<.01);CHECK(repair.vehicle().lap==0);CHECK(repair.bestLapTime()==0.);
 CHECK(repair.simulator().componenteMaisCritico().id==repair.simulator().rankingRisco().front().id);
 CHECK(!repair.hasGhost());CHECK(repair.validCompletedLaps()==0);
 bool threw=false;try{damaged.update({},-1);}catch(const std::invalid_argument&){threw=true;}CHECK(threw);
 std::cout<<"PASSOU: clima dinamico, controle, colisao, regeneraçao, reproducibilidade.\n";
}
