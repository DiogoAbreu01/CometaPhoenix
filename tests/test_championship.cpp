#include "Championship.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#define CHECK(c) do{if(!(c)){std::cerr<<"CUP_FAIL "<<__LINE__<<" "<<#c<<"\n";return 1;}}while(false)
int main(){
 using namespace phoenix;
 Championship cup;
 CHECK(!cup.active() && !cup.betweenStages());
 CHECK(!cup.service(ServiceChoice::Conserve));
 cup.start();
 CHECK(cup.active() && cup.course()==Circuit::Sakura);
 CHECK(cup.materials()==4 && cup.battery()==90.);
 std::array<RivalAI,2> rivals{};
 for(int stage=0;stage<4;++stage){
    Game player(42);
    player.setCircuit(cup.course());player.setWeather(Weather::Sunny);
    player.setTargetLaps(1);player.setAutopilot(true);
    for(int i=0;i<2;++i){
       rivals[i].setDifficulty(i?RivalDifficulty::Easy:RivalDifficulty::Normal);
       rivals[i].reset(cup.course(),Weather::Sunny,i?0.87:1.,-.09*(i+1));
    }
    player.seedTireWear(cup.wear());
    CHECK(std::abs(player.handling().tireWear-cup.wear())<1e-12);
    int frames=0;
    while(!player.finished()&&!player.abandoned()&&frames<30000){
       player.update({},.02);
       for(auto& rival:rivals)rival.update(.02,1);
       ++frames;
    }
    CHECK(player.finished() && !player.abandoned());
    RaceOutcome outcome=makeRaceOutcome(player,rivals,2);
    CHECK(outcome.completedLaps==1 && outcome.targetLaps==1);
    CHECK(cup.recordStage(outcome,rivals,player.handling().tireWear));
    CHECK(!cup.recordStage(outcome,rivals,player.handling().tireWear));
    CHECK(cup.points()[0]>0 && cup.wear()>.02);
    if(stage==0){
        CHECK(cup.betweenStages() && !cup.finished());
        CHECK(cup.nextCourse()==Circuit::Forest);
        const double before=cup.wear();
        CHECK(cup.service(ServiceChoice::Quick));
        CHECK(cup.materials()==3 && cup.battery()==81.);
        CHECK(cup.wear()<before);
        CHECK(!cup.service(ServiceChoice::Conserve));
    }else if(stage==1){
        CHECK(cup.service(ServiceChoice::Full));
        CHECK(cup.materials()==1 && cup.battery()==57.);
    }else if(stage==2){
        CHECK(!cup.service(ServiceChoice::Full)); // material guard, no partial mutation
        CHECK(cup.materials()==1 && cup.battery()==57.);
        CHECK(cup.service(ServiceChoice::Conserve));
    }else{
        CHECK(cup.finished() && !cup.betweenStages());
        CHECK(!cup.service(ServiceChoice::Conserve));
    }
 }
 CHECK(cup.stage()==3 && cup.finished());
 CHECK(cup.points()[0]>=16);
 CHECK(cup.overallPosition()>=1 && cup.overallPosition()<=3);
 const auto path=std::filesystem::temp_directory_path()/"phoenix_cup_013_test.csv";
 std::error_code ec;std::filesystem::remove(path,ec);
 CHECK(cup.appendSummary(path));
 std::ifstream in(path);std::string header,row;
 CHECK(bool(std::getline(in,header))&&header.find("player_points")!=std::string::npos);
 CHECK(bool(std::getline(in,row))&&row.find("PHOENIX_CUP_V1")!=std::string::npos);
 in.close();std::filesystem::remove(path,ec);
 std::cout<<"CUP_013_PASS points="<<cup.points()[0]<<"/"<<cup.points()[1]<<"/"<<cup.points()[2]
          <<" result=P"<<cup.overallPosition()<<" materials="<<cup.materials()
          <<" battery="<<cup.battery()<<" wear="<<cup.wear()<<"\n";
}
