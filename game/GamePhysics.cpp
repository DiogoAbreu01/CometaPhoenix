#include "GamePhysics.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace phoenix {
namespace {constexpr double PI=3.14159265358979323846;
double targetRadius(double a) {return 4.65 + .62*std::cos(3*a+.3) + .34*std::sin(5*a-.6) + .22*std::cos(7*a+1.2);}
double clamp(double a,double l,double h){return std::max(l,std::min(h,a));}
double normAngle(double a) {return std::atan2(std::sin(a),std::cos(a));}
}
const char* weatherName(Weather w) noexcept {switch(w){case Weather::Sunny:return "SOL";case Weather::Rain:return "CHUVA";case Weather::Snow:return "NEVE";case Weather::Storm:return "TEMPESTADE";case Weather::Fog:return "NEBLINA";}return "?";}
Climate climate(Weather w) noexcept {switch(w){case Weather::Sunny:return {1.,1.,0,28,.02,1};case Weather::Rain:return {.69,.78,.7,17,.23,1.08};case Weather::Snow:return {.39,.74,.64,-4,.11,1.3};case Weather::Storm:return {.51,.49,1.,16,.62,1.18};case Weather::Fog:return {.83,.35,.12,12,.04,1.08};}return {1,1,0,28,0,1};}
Game::Game(std::uint32_t seed):seed_(seed){restart(seed);}
void Game::restart(std::uint32_t seed){seed_=seed;sim_=std::make_unique<cometa::Simulacao>("normal", seed);vehicle_={}; vehicle_.x=targetRadius(0);vehicle_.y=0;vehicle_.angle=PI/2;lastPolarAngle_=0;orbitTurns_=0;worldTime_=0;cinematic_=0;lastImpact_=-100;}
bool Game::garage() const noexcept {auto f=sim_->fase();return f==cometa::Fase::Diagnostico || f==cometa::Fase::Reparando || f==cometa::Fase::Validacao || f==cometa::Fase::Abandono;}
double Game::repairFraction() const noexcept {const auto& r=sim_->reparo();return r.tempoTotalS>0?clamp(r.tempoDecorridoS/r.tempoTotalS,0,1):0;}
double Game::trackOffset() const noexcept {double a=std::atan2(vehicle_.y,vehicle_.x);return std::hypot(vehicle_.x,vehicle_.y)-targetRadius(a);}
double Game::lapProgress() const noexcept {return (std::atan2(vehicle_.y,vehicle_.x)+2*PI)/(2*PI)-std::floor((std::atan2(vehicle_.y,vehicle_.x)+2*PI)/(2*PI));}
void Game::requestDamage(const std::string& part, double severity){if(sim_->fase()!=cometa::Fase::Corrida)return; sim_->provocarImpacto(part,clamp(severity,1.,20.),part=="chassi"?.013:.036);cinematic_=2.6;lastImpact_=worldTime_;}
void Game::update(const Input& in, double dt){
    if(!(dt>0&&dt<=.2) || !std::isfinite(dt))throw std::invalid_argument("passo invalido");
    const auto before=sim_->fase();
    worldTime_+=dt; cinematic_=std::max(0.,cinematic_-dt);
    if(sim_->fase()==cometa::Fase::Corrida){
        Climate cl=climate(weather_);
        double speed=vehicle_.speed;
        bool offroad=std::abs(trackOffset())>.42;
        double grip=cl.grip*(offroad?.52:1.)*(1.-std::min(.45,sim_->componente("pneu_de").desgaste*.45));
        double accel= (in.throttle ? 22.0:0.0) - (in.brake? 35.0:0.0) - (5.0 + speed*.003*cl.drag);
        if(autopilot_){
            const double a=std::atan2(vehicle_.y,vehicle_.x);
            const double r=targetRadius(a), dr=-1.86*std::sin(3*a+.3)+1.70*std::cos(5*a-.6)-1.54*std::sin(7*a+1.2);
            const double ddr=-5.58*std::cos(3*a+.3)-8.50*std::sin(5*a-.6)-10.78*std::cos(7*a+1.2);
            const double k=std::abs(r*r+2*dr*dr-r*ddr)/std::pow(r*r+dr*dr,1.5)/15.;
            const double limit=std::sqrt(4.5*grip/std::max(.008,k))*3.6;
            const double target=std::clamp(limit,18.,68.*std::sqrt(grip));
            accel=std::clamp((target-speed)*2.3,-35.,22.);
        }
        speed=clamp(speed+accel*dt,0.,102.);
        double steer=((in.left?1.:0.)-(in.right?1.:0.));
        if(autopilot_){
            const double a=std::atan2(vehicle_.y,vehicle_.x), look=a+.075+speed*.00065;
            const double r=targetRadius(look);
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
        vehicle_.lap=std::max(0,(int)std::floor(orbitTurns_/(2*PI)));
        // A rigidez da pista gera impactos controlados ao atravessar a defensa.
        double offset=trackOffset();
        if(std::abs(offset)>.63 && worldTime_-lastImpact_>2.2 && speed>14.){
            requestDamage(speed>73.?"suspensao":"carenagem",clamp(speed/5.,5.,15.));
            // Retorna o kart a borda da pista, impedindo disparos repetidos de colisao.
            const double rr=targetRadius(a)+std::copysign(.46,offset);
            vehicle_.x=rr*std::cos(a);vehicle_.y=rr*std::sin(a);
            vehicle_.speed*=.42;
        }
    } else if(sim_->fase()==cometa::Fase::Alerta){
        // Imobilizacao para acionar o reboque, sem atravessar terreno ou dirigir com chassi inseguro.
        vehicle_.speed=std::max(0.,vehicle_.speed-dt*70.);
        vehicle_.lateral=0.;
    } else if(sim_->fase()==cometa::Fase::RetornoBox){
        const double a=std::atan2(vehicle_.y,vehicle_.x);
        const double pitX=targetRadius(0)+.76, pitY=.12;
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
            const double r=targetRadius(a);
            const double dr=-1.86*std::sin(3*a+.3)+1.70*std::cos(5*a-.6)-1.54*std::sin(7*a+1.2);
            const double next=a+step/std::hypot(r,dr);
            const double offset=(std::hypot(ox,oy)-r)*std::max(0.,1.-3.*dt);
            const double nr=targetRadius(next)+offset;
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
       double a=0.;vehicle_.x=targetRadius(a);vehicle_.y=0.;vehicle_.angle=PI/2;vehicle_.speed=0;lastPolarAngle_=a;orbitTurns_=vehicle_.lap*2*PI;lastImpact_=worldTime_;
    }
}
std::string Game::status() const {switch(sim_->fase()) {case cometa::Fase::Corrida:return "PILOTANDO";case cometa::Fase::Alerta:return "IMPACTO DETECTADO";case cometa::Fase::RetornoBox:return "REBOQUE AO BOX";case cometa::Fase::Diagnostico:return "ESCANEANDO ESTRUTURA";case cometa::Fase::Reparando:return "REGENERACAO EM CURSO";case cometa::Fase::Validacao:return "VALIDANDO SEGURANCA";case cometa::Fase::Abandono:return "DANO IRREVERSIVEL";}return "?";}
}
