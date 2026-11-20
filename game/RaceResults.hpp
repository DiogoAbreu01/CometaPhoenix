#pragma once
#include "GamePhysics.hpp"
#include "RivalAI.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <locale>
#include <string>
#include <system_error>

namespace phoenix {
constexpr double racePi2=6.2831853071795864769;

inline int racePosition(const Game& game,const std::array<RivalAI,2>& rivals,int active) noexcept {
    active=std::clamp(active,0,2);
    if(game.abandoned())return active+1;
    int position=1;
    if(game.finished()) {
        for(int i=0;i<active;++i)
            position+=rivals[i].finished(game.targetLaps()) &&
                      rivals[i].raceTime()<game.worldTime();
    }else{
        const double progress=game.vehicle().lap+game.lapProgress();
        for(int i=0;i<active;++i)
            position+=rivals[i].totalProgress()/racePi2>progress;
    }
    return position;
}
struct RaceOutcome {
    Circuit circuit{};
    Weather weather{};
    int completedLaps{},targetLaps{},rivals{},position{};
    double elapsedSeconds{},bestLapSeconds{};
    std::size_t damageEvents{};
    bool abandoned{};
};
inline RaceOutcome makeRaceOutcome(const Game& game,const std::array<RivalAI,2>& rivals,int active) noexcept {
    return {game.circuit(),game.weather(),game.validCompletedLaps(),game.targetLaps(),
            std::clamp(active,0,2),racePosition(game,rivals,active),
            game.worldTime(),game.bestLapTime(),game.simulator().danos().tamanho(),game.abandoned()};
}
inline bool appendRaceHistory(const std::filesystem::path& path,const RaceOutcome& result) {
    static constexpr const char* header="format,circuit,weather,completed_laps,target_laps,rivals,position,elapsed_s,best_lap_s,damage_events,status";
    if(path.empty() || static_cast<int>(result.circuit)<0 || static_cast<int>(result.circuit)>2 ||
       static_cast<int>(result.weather)<0 || static_cast<int>(result.weather)>4 ||
       result.targetLaps<0 || result.targetLaps>99 ||
       result.completedLaps<0 || result.rivals<0 || result.rivals>2 ||
       result.position<1 || result.position>result.rivals+1 ||
       !std::isfinite(result.elapsedSeconds) || result.elapsedSeconds<0. ||
       !std::isfinite(result.bestLapSeconds) || result.bestLapSeconds<0. ||
       result.damageEvents>10000000 || (result.targetLaps==0 && !result.abandoned))
        return false;
    try {
        std::error_code ec;
        if(!path.parent_path().empty())std::filesystem::create_directories(path.parent_path(),ec);
        if(ec)return false;
        bool isNew=true;
        if(std::filesystem::exists(path,ec) && !ec){
            auto n=std::filesystem::file_size(path,ec);
            if(ec || n>8*1024*1024)return false;
            isNew=n==0;
            if(!isNew){
                std::ifstream in(path);
                std::string line;
                if(!std::getline(in,line) || line!=header)return false;
            }
        }
        std::ofstream out(path,std::ios::app);
        if(!out)return false;
        out.imbue(std::locale::classic());
        if(isNew)out<<header<<'\n';
        out<<std::fixed<<std::setprecision(3)
           <<"PHOENIX_RACE_V1,"<<static_cast<int>(result.circuit)<<','
           <<static_cast<int>(result.weather)<<','<<result.completedLaps<<','
           <<result.targetLaps<<','<<result.rivals<<','<<result.position<<','
           <<result.elapsedSeconds<<','<<result.bestLapSeconds<<','
           <<result.damageEvents<<','<<(result.abandoned?"ABANDONO":"CONCLUIDA")<<'\n';
        out.flush();
        return bool(out);
    } catch(...) {return false;}
}
} // namespace phoenix
