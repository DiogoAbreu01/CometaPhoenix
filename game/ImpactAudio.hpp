#pragma once
// Original procedural kart SFX (C++17). One atomic mailbox, no allocations on SDL callback.
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <string_view>

enum class ImpactKind : unsigned { None=0, Scrape=1, Tire=2, Suspension=3, Chassis=4, Repair=5, Finish=6 };

inline ImpactKind classifyImpact(std::string_view part,std::string_view cause) noexcept {
 if(cause.find("RASPAGEM")!=std::string_view::npos) return ImpactKind::Scrape;
 if(part=="chassi") return ImpactKind::Chassis;
 if(part=="suspensao"||part=="freio") return ImpactKind::Suspension;
 if(part.rfind("pneu_",0)==0) return ImpactKind::Tire;
 return ImpactKind::Scrape;
}

class ImpactAudio {
 friend struct ImpactAudioTest;
 static constexpr float rate=48000.f, tau=6.28318530718f;
 std::atomic<std::uint32_t> pending_{0};
 ImpactKind current_{ImpactKind::None};
 std::uint32_t seed_{0x24b0e9d5u};
 int sample_{0}, duration_{0};
 float phase_{0}, envelope_{0}, lowNoise_{0}, strength_{0};
 float randomNoise() noexcept {
   seed_^=seed_<<13;seed_^=seed_>>17;seed_^=seed_<<5;
   return float(seed_&0xffffu)/32767.5f-1.f;
 }
public:
 void reset() noexcept { pending_.store(0,std::memory_order_relaxed); current_=ImpactKind::None; seed_=0x24b0e9d5u; sample_=duration_=0; phase_=envelope_=lowNoise_=strength_=0.f; }
 // Producer: game/SDL event thread. Last event wins when many happen inside one audio block.
 void trigger(ImpactKind type,float severity=0.6f) noexcept {
   if(type==ImpactKind::None)return;
   const auto level=static_cast<std::uint32_t>(std::lround(std::clamp(std::isfinite(severity)?severity:0.f,0.f,1.f)*255.f));
   pending_.store((static_cast<std::uint32_t>(type)<<8)|level,std::memory_order_release);
 }
 // Consumer: once at start of each 512-frame SDL callback, never the game thread.
 void beginBlock() noexcept {
   const auto command=pending_.exchange(0,std::memory_order_acq_rel);
   if(!command)return;
   const auto kind=static_cast<ImpactKind>(command>>8);
   const int lengths[]={0,11000,15000,20000,26000,17000,40000};
   auto id=static_cast<unsigned>(kind);
   if(id==0||id>=7)return;
   current_=kind;sample_=0;duration_=lengths[id];phase_=0.f;envelope_=1.f;
   strength_=.3f+.7f*float(command&255)/255.f;
 }
 // Consumer: original transient sounds. Bounded/no allocation/no block operations.
 float nextSample() noexcept {
   if(current_==ImpactKind::None)return 0.f;
   const float t=float(sample_)/rate, progress=float(sample_)/duration_;
   if(sample_++>=duration_){current_=ImpactKind::None;return 0.f;}
   const float n=randomNoise();
   lowNoise_+=.16f*(n-lowNoise_);
   float hz=0.f, noise=0.f, tonal=0.f, gain=0.f;
   switch(current_){
     case ImpactKind::Scrape:
       hz=330.f-170.f*progress;noise=.58f*n+.22f*lowNoise_;tonal=.16f*std::sin(phase_);gain=.14f;
       break;
     case ImpactKind::Tire:
       hz=850.f-700.f*progress;noise=.18f*n;tonal=.65f*std::sin(phase_)+.22f*std::sin(phase_*2.f);gain=.19f;
       break;
     case ImpactKind::Suspension:
       hz=490.f-115.f*progress;noise=.12f*n;tonal=.58f*std::sin(phase_)+.27f*std::sin(phase_*2.73f);gain=.22f;
       break;
     case ImpactKind::Chassis:
       hz=105.f-80.f*progress;noise=.54f*lowNoise_;tonal=.72f*std::sin(phase_);gain=.27f;
       break;
     case ImpactKind::Repair:
       hz=570.f+440.f*progress+65.f*std::sin(t*38.f);noise=.04f*n;tonal=.75f*std::sin(phase_);gain=.12f;
       break;
     case ImpactKind::Finish:
       hz=progress<.32f?523.25f:progress<.65f?659.25f:783.99f;
       noise=0.f;tonal=.56f*std::sin(phase_)+.17f*std::sin(phase_*2.f);gain=.13f;
       break;
     default: current_=ImpactKind::None;return 0.f;
   }
   phase_+=tau*hz/rate;
   if(phase_>tau)phase_-=tau;
   const float attack=std::min(1.f,t*115.f);
   const float decay=std::exp(-5.8f*progress);
   return std::clamp((tonal+noise)*gain*strength_*attack*decay,-.35f,.35f);
 }
 bool active() const noexcept {return current_!=ImpactKind::None;}
};
