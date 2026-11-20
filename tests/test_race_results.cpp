#include "RaceResults.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL line "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(false)
int main(){
 using namespace phoenix;
 std::array<RivalAI,2> rivals{};
 rivals[0].reset(Circuit::Forest,Weather::Rain,1.0,-.09);
 rivals[1].reset(Circuit::Forest,Weather::Rain,.88,-.18);
 Game player(42);player.setTargetLaps(1);player.setWeather(Weather::Rain);
 player.setAutopilot(true);
 CHECK(racePosition(player,rivals,0)==1);
 CHECK(racePosition(player,rivals,2)>=1 && racePosition(player,rivals,2)<=3);
 for(int i=0;i<15000&&!player.finished();++i){
     player.update({},.02);
     for(auto& rival:rivals)rival.update(.02,1);
 }
 CHECK(player.finished());
 auto result=makeRaceOutcome(player,rivals,2);
 CHECK(result.completedLaps==1);
 CHECK(result.targetLaps==1);
 CHECK(result.position>=1 && result.position<=3);
 CHECK(result.bestLapSeconds>0. && result.elapsedSeconds>0.);
 const auto path=std::filesystem::temp_directory_path()/"cometa_phoenix_race_history_test.csv";
 std::error_code ec;std::filesystem::remove(path,ec);
 CHECK(appendRaceHistory(path,result));
 CHECK(appendRaceHistory(path,result));
 std::ifstream in(path);std::string header,row1,row2,trailing;
 CHECK(bool(std::getline(in,header)));
 CHECK(header.find("completed_laps")!=std::string::npos);
 CHECK(bool(std::getline(in,row1)));
 CHECK(bool(std::getline(in,row2)));
 CHECK(row1==row2);
 CHECK(row1.find("PHOENIX_RACE_V1,0,1,1,1,2,")!=std::string::npos);
 CHECK(!bool(std::getline(in,trailing)));
 in.close();
 auto invalid=result;
 invalid.elapsedSeconds=std::numeric_limits<double>::quiet_NaN();
 CHECK(!appendRaceHistory(path,invalid));
 invalid=result;invalid.position=4;
 CHECK(!appendRaceHistory(path,invalid));
 invalid=result;invalid.targetLaps=0;
 CHECK(!appendRaceHistory(path,invalid));
 {std::ofstream corrupt(path,std::ios::trunc);corrupt<<"not our csv\n";}
 CHECK(!appendRaceHistory(path,result));
 std::filesystem::remove(path,ec);
 player.restart();player.requestDamage("chassi",18);
 for(int i=0;i<2000&&!player.abandoned();++i)player.update({},.02);
 CHECK(player.abandoned());
 const auto abandoned=makeRaceOutcome(player,rivals,2);
 CHECK(abandoned.abandoned && abandoned.position==3);
 CHECK(appendRaceHistory(path,abandoned));
 std::filesystem::remove(path,ec);
 std::cout<<"RACE_HISTORY_OK\n";
}
