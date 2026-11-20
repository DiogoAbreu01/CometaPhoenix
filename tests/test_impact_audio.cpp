#include "../game/ImpactAudio.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string_view>
struct ImpactAudioTest {
 static int run() {
  int failures=0;
  auto check=[&](bool ok, const char* name){if(!ok){++failures;std::cerr<<"FAIL "<<name<<'\n';}};
  check(classifyImpact("carenagem","RASPAGEM LATERAL")==ImpactKind::Scrape,"scrape");
  check(classifyImpact("pneu_dd","IMPACTO NA RODA")==ImpactKind::Tire,"tire");
  check(classifyImpact("suspensao","IMPACTO FRONTAL")==ImpactKind::Suspension,"suspension");
  check(classifyImpact("chassi","IMPACTO ESTRUTURAL")==ImpactKind::Chassis,"chassis");
  check(classifyImpact("motor","REGENERACAO")==ImpactKind::Scrape,"fallback");
  std::array<double,6> energy{};
  for(unsigned type=1;type<=6;++type){
    ImpactAudio fx;
    check(fx.nextSample()==0.f,"initial silence");
    fx.trigger(static_cast<ImpactKind>(type),1.f);
    fx.beginBlock();
    double e=0.,mean=0.;float peak=0.;
    constexpr int N=48000;
    for(int i=0;i<N;++i){
       if(i%512==0)fx.beginBlock();
       auto x=fx.nextSample();
       check(std::isfinite(x) && std::abs(x)<=.35f,"bounded sample");
       e+=double(x)*x;mean+=x;peak=std::max(peak,std::abs(x));
    }
    energy[type-1]=e/N;
    check(peak>.015f,"audible amplitude");
    check(std::abs(mean/N)<.02,"DC offset");
    check(!fx.active() && fx.nextSample()==0.f,"releases to silence");
    std::cout<<"SFX "<<type<<" RMS="<<std::sqrt(energy[type-1])<<" peak="<<peak<<'\n';
  }
  ImpactAudio m;
  m.trigger(ImpactKind::Scrape,.5f);
  m.trigger(ImpactKind::Chassis,1.f);
  m.beginBlock();
  check(m.active(),"event mailbox");
  m.trigger(ImpactKind::Repair,std::numeric_limits<float>::quiet_NaN());
  m.beginBlock();
  float first=m.nextSample();
  check(std::isfinite(first),"NaN level safe");
  check(std::abs(energy[3]-energy[0])>.000005,"different collision timbres");
  if(failures)std::cerr<<failures<<" assertions failed\n";
  else std::cout<<"IMPACT_AUDIO_OK\n";
  return failures?1:0;
 }
};
int main(){return ImpactAudioTest::run();}
