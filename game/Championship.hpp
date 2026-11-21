#pragma once
#include "RaceResults.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <locale>
#include <system_error>

namespace phoenix {

enum class ServiceChoice { Conserve=1, Quick=2, Full=3 };

class Championship {
public:
 static constexpr int stages=4;
 void start() noexcept {
    active_=true;finished_=betweenStages_=false;stage_=0;
    points_={};stagePositions_={};bestTimes_={};wins_={};
    materials_=4;battery_=90.;carryWear_=0.;totalContacts_=0;
 }
 bool active() const noexcept {return active_;}
 bool finished() const noexcept {return active_&&finished_;}
 bool betweenStages() const noexcept {return active_&&betweenStages_;}
 int stage() const noexcept {return stage_;}
 Circuit course() const noexcept {return courses_[static_cast<std::size_t>(stage_)];}
 Circuit nextCourse() const noexcept {return courses_[static_cast<std::size_t>(std::min(stage_+1,stages-1))];}
 Weather stageWeather() const noexcept {
    constexpr std::array<Weather,4> weather{Weather::Sunny,Weather::Rain,Weather::Snow,Weather::Sunny};
    return weather[static_cast<std::size_t>(stage_)];
 }
 const std::array<int,3>& points() const noexcept {return points_;}
 int materials() const noexcept {return materials_;}
 double battery() const noexcept {return battery_;}
 double wear() const noexcept {return carryWear_;}
 unsigned totalContacts() const noexcept {return totalContacts_;}
 std::array<int,4> stagePositions() const noexcept {return stagePositions_;}
 int overallPosition() const noexcept {
    int standing=1;
    for(int i=1;i<3;++i)
        standing+=points_[i]>points_[0]||(points_[i]==points_[0]&&wins_[i]>wins_[0]);
    return standing;
 }
 bool recordStage(const RaceOutcome& race,const std::array<RivalAI,2>& rivals,double wear) noexcept {
    if(!active_||finished_||betweenStages_||race.circuit!=course()||
       race.targetLaps!=1||race.rivals!=2||
       (!race.abandoned&&race.completedLaps<1)||
       race.position<1||race.position>3||
       !std::isfinite(wear)||wear<0.||wear>1.||
       !std::isfinite(race.elapsedSeconds)||race.elapsedSeconds<=0.)return false;
    const int playerPos=race.abandoned?3:race.position;
    const auto ahead=[&](){
       if(rivals[0].finished(1)!=rivals[1].finished(1))return rivals[0].finished(1);
       if(rivals[0].finished(1)&&rivals[1].finished(1))
           return rivals[0].raceTime()<=rivals[1].raceTime();
       return rivals[0].totalProgress()>=rivals[1].totalProgress();
    };
    const int otherBest=playerPos==1?2:1;
    const int otherLast=playerPos==3?2:3;
    const std::array<int,3> places{playerPos,
        ahead()?otherBest:otherLast,ahead()?otherLast:otherBest};
    constexpr std::array<int,4> award{0,10,7,4};
    for(int i=0;i<3;++i){
       points_[i]+=award[static_cast<std::size_t>(places[i])];
       if(places[i]==1)++wins_[i];
    }
    stagePositions_[static_cast<std::size_t>(stage_)]=playerPos;
    bestTimes_[static_cast<std::size_t>(stage_)]=race.abandoned?0.:race.elapsedSeconds;
    totalContacts_+=static_cast<unsigned>(std::min<std::size_t>(race.damageEvents,500));
    // Racing wears tires; only between-stage service can restore them.
    carryWear_=std::clamp(std::max(carryWear_,wear)+.10+
               .025*std::min<std::size_t>(race.damageEvents,8),0.,.95);
    if(stage_==stages-1)finished_=true;
    else betweenStages_=true;
    return true;
 }
 bool service(ServiceChoice choice) noexcept {
    if(!betweenStages_||finished_)return false;
    int units=0;double wattHours=0.,restored=0.;
    switch(choice){
      case ServiceChoice::Conserve:break;
      case ServiceChoice::Quick:units=1;wattHours=9.;restored=.16;break;
      case ServiceChoice::Full:units=2;wattHours=24.;restored=.44;break;
      default:return false;
    }
    if(units>materials_||wattHours>battery_)return false;
    materials_-=units;battery_-=wattHours;
    carryWear_=std::max(0.,carryWear_-restored);
    ++stage_;
    betweenStages_=false;
    return true;
 }
 bool appendSummary(const std::filesystem::path& path) const {
    if(!finished_||path.empty())return false;
    try{
       std::error_code ec;
       if(!path.parent_path().empty())std::filesystem::create_directories(path.parent_path(),ec);
       if(ec)return false;
       bool blank=true;
       if(std::filesystem::exists(path,ec)&&!ec){
          const auto sz=std::filesystem::file_size(path,ec);
          if(ec||sz>1024*1024)return false;
          blank=sz==0;
          if(!blank){
             std::ifstream in(path);
             std::string line;
             if(!std::getline(in,line)||line!="format,position,player_points,rival1_points,rival2_points,materials,battery,wear,contacts")return false;
          }
       }
       std::ofstream out(path,std::ios::app);
       if(!out)return false;
       out.imbue(std::locale::classic());
       if(blank)out<<"format,position,player_points,rival1_points,rival2_points,materials,battery,wear,contacts\n";
       out<<std::fixed<<std::setprecision(3)<<"PHOENIX_CUP_V1,"<<overallPosition()
          <<','<<points_[0]<<','<<points_[1]<<','<<points_[2]<<','<<materials_
          <<','<<battery_<<','<<carryWear_<<','<<totalContacts_<<'\n';
       out.flush();
       return bool(out);
    }catch(...){return false;}
 }
private:
 static constexpr std::array<Circuit,4> courses_{
   Circuit::Sakura,Circuit::Forest,Circuit::Alpine,Circuit::Coast
 };
 bool active_{},finished_{},betweenStages_{};
 int stage_{};
 std::array<int,3> points_{},wins_{};
 std::array<int,4> stagePositions_{};
 std::array<double,4> bestTimes_{};
 int materials_{4};double battery_{90.},carryWear_{};
 unsigned totalContacts_{};
};

} // namespace phoenix
