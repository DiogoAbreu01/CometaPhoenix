#include "GamePhysics.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>
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
 Game far;far.setAutopilot(true);
 for(int i=0;i<30000 && far.lapProgress()<.4;++i)far.update({},.02);
 CHECK(far.lapProgress()>=.4);far.setAutopilot(false);far.requestDamage("carenagem",12);
 for(int i=0;i<220;++i)far.update({},.02);
 CHECK(!far.garage());CHECK(far.simulator().fase()==cometa::Fase::RetornoBox);
 int towSteps=0;while(!far.garage() && towSteps<6000){far.update({},.02);++towSteps;}
 const double pitX=4.65+.62*std::cos(.3)+.34*std::sin(-.6)+.22*std::cos(1.2)+.76;
 CHECK(far.garage());CHECK(std::hypot(far.vehicle().x-pitX,far.vehicle().y-.12)<1e-7);
 CHECK(far.vehicle().speed==0.);CHECK(far.vehicle().lap==0);CHECK(far.bestLapTime()==0.);
 std::cout<<"PIT_ARRIVAL elapsed="<<(220+towSteps)*.02<<" m_error="<<std::hypot(far.vehicle().x-pitX,far.vehicle().y-.12)*15<<"\n";
 Game damaged;damaged.requestDamage("carenagem",10);CHECK(damaged.hasDamage());
 for(int i=0;i<800;i++)damaged.update({},.02);
 CHECK(damaged.garage() || damaged.simulator().reparo().tipo!=cometa::TipoReparo::Nenhum);
 Game reset(17),fresh(17);reset.requestDamage("carenagem",12);double loss=reset.simulator().danos().itens().front().deltaIntegridade;reset.restart(17);CHECK(!reset.hasDamage());reset.requestDamage("carenagem",12);fresh.requestDamage("carenagem",12);CHECK(reset.simulator().danos().itens().front().deltaIntegridade==loss);CHECK(reset.simulator().danos().itens().front().deltaIntegridade==fresh.simulator().danos().itens().front().deltaIntegridade);
 Game fracture;fracture.requestDamage("chassi",18);for(int i=0;i<900;i++)fracture.update({},.02);CHECK(fracture.abandoned());CHECK(fracture.simulator().componente("chassi").integridade<.55);
 Game repair;repair.requestDamage("carenagem",12);bool sawGarage=false;for(int i=0;i<1600;i++){repair.update({},.02);sawGarage|=repair.garage();}CHECK(sawGarage);CHECK(!repair.abandoned());CHECK(repair.simulator().fase()==cometa::Fase::Corrida);CHECK(repair.simulator().recursos().agenteMl<35);CHECK(std::abs(repair.trackOffset())<.01);CHECK(repair.vehicle().lap==0);CHECK(repair.bestLapTime()==0.);
 CHECK(repair.simulator().componenteMaisCritico().id==repair.simulator().rankingRisco().front().id);
 bool threw=false;try{damaged.update({},-1);}catch(const std::invalid_argument&){threw=true;}CHECK(threw);
 std::cout<<"PASSOU: clima dinamico, controle, colisao, regeneraçao, reproducibilidade.\n";
}
