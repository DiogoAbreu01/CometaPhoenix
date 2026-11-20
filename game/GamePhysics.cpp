#include "GamePhysics.hpp"
#include "KartCollision.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <iomanip>
#include <locale>
#include <system_error>
namespace phoenix {
namespace {constexpr double PI=3.14159265358979323846;
struct Shape {double base,a3,a5,a7;};
Shape shape(Circuit c) noexcept {
    switch(c){case Circuit::Alpine:return {5.35,.94,.22,.12};case Circuit::Coast:return {5.85,.32,.58,.08};default:return {4.65,.62,.34,.22};}
}
double derivative(Circuit c,double a) noexcept {auto s=shape(c);return -3*s.a3*std::sin(3*a+.3)+5*s.a5*std::cos(5*a-.6)-7*s.a7*std::sin(7*a+1.2);}
double secondDerivative(Circuit c,double a) noexcept {auto s=shape(c);return -9*s.a3*std::cos(3*a+.3)-25*s.a5*std::sin(5*a-.6)-49*s.a7*std::cos(7*a+1.2);}
double clamp(double a,double l,double h){return std::max(l,std::min(h,a));}
double normAngle(double a) {return std::atan2(std::sin(a),std::cos(a));}
}
const char* circuitName(Circuit c) noexcept {switch(c){case Circuit::Alpine:return "ALPES";case Circuit::Coast:return "LITORAL";default:return "FLORESTA";}}
double circuitRadius(Circuit c,double a) noexcept {auto s=shape(c);return s.base+s.a3*std::cos(3*a+.3)+s.a5*std::sin(5*a-.6)+s.a7*std::cos(7*a+1.2);}
void Game::setCircuit(Circuit c){circuit_=c;restart(seed_);}
const char* weatherName(Weather w) noexcept {switch(w){case Weather::Sunny:return "SOL";case Weather::Rain:return "CHUVA";case Weather::Snow:return "NEVE";case Weather::Storm:return "TEMPESTADE";case Weather::Fog:return "NEBLINA";}return "?";}
Climate climate(Weather w) noexcept {switch(w){case Weather::Sunny:return {1.,1.,0,28,.02,1};case Weather::Rain:return {.69,.78,.7,17,.23,1.08};case Weather::Snow:return {.39,.74,.64,-4,.11,1.3};case Weather::Storm:return {.51,.49,1.,16,.62,1.18};case Weather::Fog:return {.83,.35,.12,12,.04,1.08};}return {1,1,0,28,0,1};}
ImpactAssessment assessImpact(double speedKmh,double incidence,bool left,bool front) noexcept {
    const double speed=std::isfinite(speedKmh)?std::clamp(speedKmh,0.,102.):0.;
    const double angle=std::isfinite(incidence)?std::clamp(incidence,0.,1.):0.;
    const double normalMs=speed/3.6*angle;
    const double severity=std::clamp(normalMs,1.,20.);
    if(normalMs>=17.)return {"chassi","IMPACTO ESTRUTURAL",severity,.013};
    if(angle>=.72 && front){
        if(normalMs>=10.)return {"suspensao","IMPACTO FRONTAL",severity,.023};
        return {"carenagem","IMPACTO FRONTAL LEVE",severity,.040};
    }
    if(angle<.25)return {"carenagem","RASPAGEM LATERAL",std::clamp(speed/3.6*.16,1.,6.),.060};
    return {front?(left?"pneu_de":"pneu_dd"):(left?"pneu_te":"pneu_td"),"IMPACTO NA RODA",severity,.030};
}
Game::Game(std::uint32_t seed):seed_(seed){restart(seed);}
void Game::setTargetLaps(int laps){targetLaps_=std::clamp(laps,0,99);restart(seed_);}
void Game::restart(std::uint32_t seed){
    seed_=seed;sim_=std::make_unique<cometa::Simulacao>("normal", seed);vehicle_={};
    vehicle_.x=circuitRadius(circuit_,0);vehicle_.y=0;vehicle_.angle=PI/2;
    lastPolarAngle_=0;orbitTurns_=0;worldTime_=0;cinematic_=0;lastImpact_=-100;lastKartContact_=-100;
    handling_={};
    currentLapTime_=lastLapTime_=bestLapTime_=0.;validLap_=true;cleanLap_=true;validCompletedLaps_=0;
    tireModel_.reset(climate(weather_).temperature);
    ghostLap_.reserve(maxGhostSamples_);
    bestGhostLap_.reserve(maxGhostSamples_);
    bestGhostLap_.clear();
    startGhostLap();
}
bool Game::garage() const noexcept {auto f=sim_->fase();return f==cometa::Fase::Diagnostico || f==cometa::Fase::Reparando || f==cometa::Fase::Validacao || f==cometa::Fase::Abandono;}
bool Game::hasGhost() const noexcept { return !bestGhostLap_.empty(); }

GhostPose Game::ghostPose() const noexcept {
    if (!hasGhost()) return {};
    const auto next = std::lower_bound(bestGhostLap_.begin(), bestGhostLap_.end(), currentLapTime_,
        [](const GhostSample& sample, double time) { return sample.time < time; });
    if (next == bestGhostLap_.begin()) return next->pose;
    if (next == bestGhostLap_.end()) return bestGhostLap_.back().pose;
    const auto& previous = *(next - 1);
    const double fraction = (currentLapTime_ - previous.time) / (next->time - previous.time);
    return {
        previous.pose.x + fraction * (next->pose.x - previous.pose.x),
        previous.pose.y + fraction * (next->pose.y - previous.pose.y),
        normAngle(previous.pose.angle + fraction * normAngle(next->pose.angle - previous.pose.angle))
    };
}

bool Game::saveRecord(const std::filesystem::path& file) const {
    if(file.empty() || !hasGhost() || !std::isfinite(bestLapTime_) || bestLapTime_<=0. ||
       bestGhostLap_.size()<2 || bestGhostLap_.size()>maxGhostSamples_)return false;
    std::error_code ec;
    if(!file.parent_path().empty())std::filesystem::create_directories(file.parent_path(),ec);
    if(ec)return false;
    const auto temp=file.string()+".tmp";
    {
        std::ofstream out(temp,std::ios::trunc);
        if(!out)return false;
        out.imbue(std::locale::classic());
        out<<std::setprecision(17);
        out<<"PHOENIX_GHOST_V1 "<<static_cast<int>(circuit_)<<' '<<static_cast<int>(weather_)
           <<' '<<bestLapTime_<<' '<<bestGhostLap_.size()<<'\n';
        for(const auto& sample:bestGhostLap_)out<<sample.time<<' '<<sample.pose.x<<' '
           <<sample.pose.y<<' '<<sample.pose.angle<<'\n';
        out.flush();
        if(!out)return false;
    }
    std::filesystem::rename(temp,file,ec);
    if(ec){
        std::filesystem::remove(file,ec);
        ec.clear();
        std::filesystem::rename(temp,file,ec);
    }
    if(ec){std::filesystem::remove(temp,ec);return false;}
    return true;
}
bool Game::loadRecord(const std::filesystem::path& file) {
    bestGhostLap_.clear();bestLapTime_=0.;
    if(file.empty())return false;
    std::error_code ec;
    const auto size=std::filesystem::file_size(file,ec);
    if(ec || size==0 || size>1024*1024)return false;
    std::ifstream in(file);
    if(!in)return false;
    in.imbue(std::locale::classic());
    std::string magic;int circuit=-1,weather=-1;
    double best=0.;std::size_t count=0;
    if(!(in>>magic>>circuit>>weather>>best>>count) ||
       magic!="PHOENIX_GHOST_V1" || circuit!=static_cast<int>(circuit_) ||
       weather!=static_cast<int>(weather_) || !std::isfinite(best) ||
       best<1. || best>36000. || count<2 || count>maxGhostSamples_)return false;
    std::vector<GhostSample> samples;
    samples.reserve(count);
    for(std::size_t i=0;i<count;++i){
        GhostSample s{};
        if(!(in>>s.time>>s.pose.x>>s.pose.y>>s.pose.angle) ||
           !std::isfinite(s.time)||!std::isfinite(s.pose.x)||
           !std::isfinite(s.pose.y)||!std::isfinite(s.pose.angle)||
           s.time<0.||s.time>best+.01||
           (i==0 && s.time>1e-5) ||
           (i>0 && s.time<=samples.back().time) ||
           std::abs(s.pose.x)>12.||std::abs(s.pose.y)>12.||
           std::abs(s.pose.angle)>PI+.001 ||
           std::abs(std::hypot(s.pose.x,s.pose.y)-circuitRadius(circuit_,std::atan2(s.pose.y,s.pose.x)))>.7)
            return false;
        samples.push_back(s);
    }
    std::string trailing;
    if(in>>trailing)return false;
    if(std::abs(samples.back().time-best)>.02)return false;
    bestGhostLap_.swap(samples);
    bestLapTime_=best;
    return true;
}
void Game::startGhostLap() {
    ghostLap_.clear();
    ghostSampleInterval_ = 0.;
    recordGhostSample(true);
}

void Game::recordGhostSample(bool finish) {
    if (!ghostLap_.empty()) {
        const double elapsed = currentLapTime_ - ghostLap_.back().time;
        if (elapsed <= 0. || (!finish && elapsed < ghostSampleInterval_)) return;
    }
    if (ghostLap_.size() == maxGhostSamples_) {
        // Keep the start and thin the history so the full lap still fits.
        for (std::size_t i = 1; i < maxGhostSamples_ / 2; ++i)
            ghostLap_[i] = ghostLap_[2 * i];
        ghostLap_.resize(maxGhostSamples_ / 2);
        ghostSampleInterval_ = std::max(ghostSampleInterval_ * 2.,
            currentLapTime_ / static_cast<double>(ghostLap_.size()));
    }
    ghostLap_.push_back({{vehicle_.x, vehicle_.y, normAngle(vehicle_.angle)}, currentLapTime_});
}
double Game::repairFraction() const noexcept {const auto& r=sim_->reparo();return r.tempoTotalS>0?clamp(r.tempoDecorridoS/r.tempoTotalS,0,1):0;}
double Game::trackOffset() const noexcept {double a=std::atan2(vehicle_.y,vehicle_.x);return std::hypot(vehicle_.x,vehicle_.y)-circuitRadius(circuit_,a);}
double Game::lapProgress() const noexcept {return (std::atan2(vehicle_.y,vehicle_.x)+2*PI)/(2*PI)-std::floor((std::atan2(vehicle_.y,vehicle_.x)+2*PI)/(2*PI));}
void Game::requestDamage(const std::string& part, double severity){if(finished()||sim_->fase()!=cometa::Fase::Corrida)return; validLap_=false; sim_->provocarImpacto(part,clamp(severity,1.,20.),part=="chassi"?.013:.036);cinematic_=2.6;lastImpact_=worldTime_;}
void Game::requestCollision(double speedKmh,double incidence,bool left,bool front){
    if(!std::isfinite(speedKmh)||!std::isfinite(incidence)||speedKmh<=0.)
        throw std::invalid_argument("impacto invalido");
    if(finished()||sim_->fase()!=cometa::Fase::Corrida)return;
    auto hit=assessImpact(speedKmh,incidence,left,front);
    validLap_=false;sim_->provocarImpacto(hit.component,hit.deltaVelocityMs,hit.durationS,hit.kind);
    cinematic_=2.6;lastImpact_=worldTime_;
}
KartContactEvent Game::collideRival(const GhostPose& rival,const GhostPose& previousRival,double dt){
    if(finished() || sim_->fase()!=cometa::Fase::Corrida)return {};
    const auto contact=detectKartContact(vehicle_,rival,previousRival,dt);
    if(!contact.overlap)return {};
    // Shift out of overlap, without mutating the opponent's deterministic racing AI.
    vehicle_.x+=contact.nx*contact.penetrationUnits;
    vehicle_.y+=contact.ny*contact.penetrationUnits;
    const double angle=std::atan2(vehicle_.y,vehicle_.x);
    const double rr=circuitRadius(circuit_,angle);
    const double off=std::hypot(vehicle_.x,vehicle_.y)-rr;
    if(std::abs(off)>.50){
        vehicle_.x=(rr+std::copysign(.50,off))*std::cos(angle);
        vehicle_.y=(rr+std::copysign(.50,off))*std::sin(angle);
    }
    const double incoming=contact.closingSpeedMs;
    if(worldTime_-lastKartContact_<.55)return {true,false,incoming};
    lastKartContact_=worldTime_;
    vehicle_.speed*=std::clamp(1.-.07*incoming,.26,1.);
    const double side=std::cos(vehicle_.angle)*contact.ny-std::sin(vehicle_.angle)*contact.nx;
    vehicle_.angle=normAngle(vehicle_.angle+std::clamp(side*incoming*.014,-.28,.28));
    vehicle_.lateral+=side*incoming/9.80665;
    if(incoming>2.4)cleanLap_=false;
    if(incoming>=15.0 && worldTime_-lastImpact_>2.2) {
        const bool left=side>0.;
        const bool front=std::cos(vehicle_.angle)*(-contact.nx)+std::sin(vehicle_.angle)*(-contact.ny)>.25;
        requestCollision(std::clamp(incoming*3.6,1.,102.),.95,left,front);
        return {true,true,incoming,true};
    }
    return {true,false,incoming,true};
}
void Game::update(const Input& in, double dt){
    if(!(dt>0&&dt<=.2) || !std::isfinite(dt))throw std::invalid_argument("passo invalido");
    if(finished())return;
    const auto before=sim_->fase();
    worldTime_+=dt; cinematic_=std::max(0.,cinematic_-dt);
    if(sim_->fase()==cometa::Fase::Corrida){
        Climate cl=climate(weather_);
        double speed=vehicle_.speed;
        bool offroad=std::abs(trackOffset())>.42;
        const double tireHealth=(sim_->componente("pneu_de").integridade+
            sim_->componente("pneu_dd").integridade+
            sim_->componente("pneu_te").integridade+
            sim_->componente("pneu_td").integridade)*.25;
        tireModel_.update(dt,speed,vehicle_.lateral,in.brake?1.:0.,
                          cl.temperature,cl.precipitation,offroad?1.:0.);
        const auto tire=tireModel_.state();
        handling_.tireTemperatureC=tire.temperatureC;
        handling_.brakeTemperatureC=tire.brakeTempC;
        handling_.tireWear=tire.wear;
        const double healthGrip=std::clamp(.57+.43*tireHealth,.45,1.);
        handling_.effectiveGrip=std::clamp(tireModel_.grip()*(.45+.55*cl.grip)*
                                          healthGrip,.16,1.05);
        handling_.brakeEffectiveness=std::clamp(sim_->componente("freio").integridade*
                                              tireModel_.brakingEfficiency(),.35,1.);
        double grip=handling_.effectiveGrip;
        const double engineEfficiency=std::clamp(sim_->componente("motor").integridade,.40,1.);
        double accel= (in.throttle ? 22.0*engineEfficiency:0.0) -
            (in.brake? 35.0*handling_.brakeEffectiveness:0.0) -
            (5.0 + speed*.003*cl.drag);
        if(autopilot_){
            const double a=std::atan2(vehicle_.y,vehicle_.x);
            const double r=circuitRadius(circuit_,a), dr=derivative(circuit_,a);
            const double ddr=secondDerivative(circuit_,a);
            const double k=std::abs(r*r+2*dr*dr-r*ddr)/std::pow(r*r+dr*dr,1.5)/15.;
            const double limit=std::sqrt(4.5*grip/std::max(.008,k))*3.6;
            const double caution=cl.grip<.5?.78:cl.grip<.6?.89:1.;
            const double target=std::clamp(limit*caution,14.,68.*std::sqrt(grip)*caution);
            accel=std::clamp((target-speed)*2.3,-35.*handling_.brakeEffectiveness,22.*engineEfficiency);
        }
        speed=clamp(speed+accel*dt,0.,102.);
        double steer=((in.left?1.:0.)-(in.right?1.:0.));
        if(autopilot_){
            const double a=std::atan2(vehicle_.y,vehicle_.x), look=a+.075+speed*.00065;
            const double r=circuitRadius(circuit_,look);
            const double desired=std::atan2(r*std::sin(look)-vehicle_.y,r*std::cos(look)-vehicle_.x);
            steer=clamp(normAngle(desired-vehicle_.angle)*3.2,-1.,1.);
        }
        double turn= (0.8+speed*.014) * grip * steer * dt;
        vehicle_.lateral= turn/dt * speed/3.6 /9.80665;
        vehicle_.angle+=turn;
        // A unidade de pista equivale a ~15 m; posicao atualizada com integracao semiexplicita.
        vehicle_.x+=std::cos(vehicle_.angle)*(speed/3.6)/15.0*dt;
        vehicle_.y+=std::sin(vehicle_.angle)*(speed/3.6)/15.0*dt;
        vehicle_.speed=speed;
        double a=std::atan2(vehicle_.y,vehicle_.x);
        double delta=normAngle(a-lastPolarAngle_);
        orbitTurns_+=delta;lastPolarAngle_=a;
        currentLapTime_+=dt;
        if(offroad || std::abs(trackOffset())>.42)validLap_=false;
        const int completed=std::max(vehicle_.lap,(int)std::floor(orbitTurns_/(2*PI)));
        if(validLap_ && cleanLap_)recordGhostSample(completed>vehicle_.lap);
        if(completed>vehicle_.lap){
            lastLapTime_=validLap_?currentLapTime_:0.;
            if(lastLapTime_>0.)++validCompletedLaps_;
            if(lastLapTime_>0. && cleanLap_ && (bestLapTime_==0. || lastLapTime_<bestLapTime_)){
                bestLapTime_=lastLapTime_;
                bestGhostLap_.swap(ghostLap_);
            }
            vehicle_.lap=completed;currentLapTime_=0.;validLap_=true;cleanLap_=true;
            startGhostLap();
            if(finished()){vehicle_.speed=0.;vehicle_.lateral=0.;return;}
        }
        // A rigidez da pista gera impactos controlados ao atravessar a defensa.
        double offset=trackOffset();
        if(std::abs(offset)>.63 && worldTime_-lastImpact_>2.2 && speed>14.){
            // Normal aproximada da defesa radial; incidência determina a energia normal e a peça atingida.
            const double r=circuitRadius(circuit_,a),dr=derivative(circuit_,a);
            const double length=std::hypot(r,dr),sign=std::copysign(1.,offset);
            const double nx=sign*(r*std::cos(a)+dr*std::sin(a))/length;
            const double ny=sign*(r*std::sin(a)-dr*std::cos(a))/length;
            const double hx=std::cos(vehicle_.angle),hy=std::sin(vehicle_.angle);
            const double projection=hx*nx+hy*ny;
            const double incidence=std::abs(projection);
            const bool left=(-hy*nx+hx*ny)>0.;
            requestCollision(speed,incidence,left,projection>.32);
            // Retorna o kart a borda da pista, impedindo disparos repetidos de colisao.
            const double rr=circuitRadius(circuit_,a)+std::copysign(.46,offset);
            vehicle_.x=rr*std::cos(a);vehicle_.y=rr*std::sin(a);
            vehicle_.speed*=.42;
        }
    } else if(sim_->fase()==cometa::Fase::Alerta){
        // Imobilizacao para acionar o reboque, sem atravessar terreno ou dirigir com chassi inseguro.
        vehicle_.speed=std::max(0.,vehicle_.speed-dt*70.);
        vehicle_.lateral=0.;
    } else if(sim_->fase()==cometa::Fase::RetornoBox){
        const double a=std::atan2(vehicle_.y,vehicle_.x);
        const double pitX=circuitRadius(circuit_,0)+.76, pitY=.12;
        const bool approach=std::abs(a)<.12;
        const double target=approach?24.:45.;
        vehicle_.speed+=std::clamp(target-vehicle_.speed,-30.*dt,20.*dt);
        const double step=(vehicle_.speed/3.6)/15.*dt;
        const double ox=vehicle_.x, oy=vehicle_.y;
        bool arrived=false;
        if(approach){
            const double dx=pitX-ox,dy=pitY-oy,dist=std::hypot(dx,dy);
            if(dist<=step+1e-8){vehicle_.x=pitX;vehicle_.y=pitY;vehicle_.speed=0;arrived=true;}
            else {vehicle_.x+=step*dx/dist;vehicle_.y+=step*dy/dist;}
        } else {
            // Transporte assistido segue o circuito no sentido de corrida ate a entrada do box.
            const double r=circuitRadius(circuit_,a);
            const double dr=derivative(circuit_,a);
            const double next=a+step/std::hypot(r,dr);
            const double offset=(std::hypot(ox,oy)-r)*std::max(0.,1.-3.*dt);
            const double nr=circuitRadius(circuit_,next)+offset;
            vehicle_.x=nr*std::cos(next);vehicle_.y=nr*std::sin(next);
        }
        if(std::hypot(vehicle_.x-ox,vehicle_.y-oy)>1e-8)vehicle_.angle=std::atan2(vehicle_.y-oy,vehicle_.x-ox);
        vehicle_.lateral=0.;
        sim_->definirChegadaBoxExterna(arrived,vehicle_.speed);
    } else {vehicle_.speed=0;}
    if (sim_->fase()==cometa::Fase::Corrida) sim_->definirVelocidadeExterna(vehicle_.speed);
    sim_->avancar(dt);
    if(before!=cometa::Fase::Corrida && sim_->fase()==cometa::Fase::Corrida){
       // apos conclusao do box, reposiciona na entrada da pista evitando salto de colisao
       double a=0.;vehicle_.x=circuitRadius(circuit_,a);vehicle_.y=0.;vehicle_.angle=PI/2;vehicle_.speed=0;lastPolarAngle_=a;orbitTurns_=vehicle_.lap*2*PI;lastImpact_=worldTime_;currentLapTime_=0.;validLap_=true;cleanLap_=true;
       startGhostLap();
    }
}
std::string Game::status() const {if(finished())return "CORRIDA CONCLUIDA";switch(sim_->fase()) {case cometa::Fase::Corrida:return "PILOTANDO";case cometa::Fase::Alerta:return "IMPACTO DETECTADO";case cometa::Fase::RetornoBox:return "REBOQUE AO BOX";case cometa::Fase::Diagnostico:return "ESCANEANDO ESTRUTURA";case cometa::Fase::Reparando:return "REGENERACAO EM CURSO";case cometa::Fase::Validacao:return "VALIDANDO SEGURANCA";case cometa::Fase::Abandono:return "DANO IRREVERSIVEL";}return "?";}
}
