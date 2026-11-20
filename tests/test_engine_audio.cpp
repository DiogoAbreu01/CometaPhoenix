// Reuse the native translation unit for its private SDL and OpenGL declarations.
// The test never opens a window or an audio device.
#define main cometa_game_entry_point
#include "../game/GameApp.cpp"
#undef main

namespace {
struct EngineAudioTest {
    struct Metrics {
        double rms{}, mean{};
        float peak{}, maxStep{};
        int risingCrossings{};
    };

    EngineAudio engine;
    float previous{};
    bool ok=true;

    EngineAudioTest() { engine.prepare(); }

    Metrics render(int blocks) {
        std::array<float,512> samples{};
        Metrics metrics;
        for(int block=0;block<blocks;++block) {
            EngineAudio::callback(&engine,reinterpret_cast<std::uint8_t*>(samples.data()),
                                  int(sizeof(samples)));
            for(float sample:samples) {
                ok=ok&&std::isfinite(sample)&&std::abs(sample)<=1.f;
                metrics.rms+=double(sample)*sample;
                metrics.mean+=sample;
                metrics.peak=std::max(metrics.peak,std::abs(sample));
                metrics.maxStep=std::max(metrics.maxStep,std::abs(sample-previous));
                metrics.risingCrossings+=previous<=0.f&&sample>0.f;
                previous=sample;
            }
        }
        const double count=blocks*samples.size();
        metrics.rms=std::sqrt(metrics.rms/count);
        metrics.mean/=count;
        return metrics;
    }

    void check(bool condition,const char* description) {
        if(!condition) std::cerr<<description<<'\n';
        ok=ok&&condition;
    }

    bool run() {
        check(render(2).peak==0.f,"Initial mute must be silent");
        engine.update(0.,true,false);
        render(20);
        // At 1,200 RPM the 60 Hz base tone completes 64 cycles in 100 blocks.
        const auto idle=render(100);
        check(idle.rms>.03&&idle.rms<.08&&std::abs(idle.mean)<.0002,"Idle RMS or DC");

        engine.update(0.,true,true);
        render(20);
        const auto rev=render(94);
        check(rev.rms>2.*idle.rms&&rev.risingCrossings>idle.risingCrossings,
              "Stationary throttle must raise level and pitch");

        engine.update(21.9,true,true);
        render(20);
        const auto before=render(94);
        engine.update(22.,true,true);
        const auto shift=render(5);
        render(20);
        const auto after=render(94);
        check(shift.rms<before.rms*.7&&after.rms>shift.rms*1.5,"Upshift cut and recovery");
        check(after.risingCrossings<before.risingCrossings*.8,"Upshift must lower pitch");
        check(shift.maxStep<.16f&&std::abs(after.mean)<.0002,"Upshift discontinuity or DC");

        engine.update(21.9,true,true);
        check(render(5).rms>after.rms*.9,"Shift threshold must have hysteresis");
        engine.update(21.9,true,false);
        const auto release=render(5);
        render(20);
        const auto coast=render(94);
        check(coast.rms<after.rms*.5&&coast.rms>.03&&release.maxStep<.16f,
              "Throttle release must settle to a smoother, quieter coast");

        engine.update(std::numeric_limits<double>::max(),true,true);
        render(20);
        const auto fast=render(94);
        engine.update(std::numeric_limits<double>::quiet_NaN(),true,false);
        render(20);
        check(std::abs(render(94).rms-idle.rms)<.001,"Non-finite speed must fall back to idle");
        engine.update(0.,false,false);
        check(render(1).peak>.001f,"Mute must release gradually");
        render(40);
        check(render(2).peak==0.f,"Released engine must settle to silence");

        engine.update(38.,true,false);
        render(30);
        const auto quiet=render(12);
        engine.triggerImpact(.95f);
        const auto transient=render(5);
        render(45);
        const auto tail=render(12);
        std::cout<<"Impact quiet peak="<<quiet.peak<<" transient peak="<<transient.peak<<" tail rms="<<tail.rms<<" transient rms="<<transient.rms<<"\n";
        check(transient.peak>quiet.peak+.035f&&transient.peak<1.f,
              "Crash transient must be audible but remain below clipping");
        check(tail.rms<transient.rms*.7,"Crash decay must settle");
        engine.update(0.,false,false);
        render(40);
        check(render(3).peak==0.f,"Crash effect must not linger when muted");
        std::cout<<"RMS idle="<<idle.rms<<" rev="<<rev.rms<<" before="<<before.rms
                 <<" shift="<<shift.rms<<" after="<<after.rms<<" coast="<<coast.rms
                 <<" high-speed peak="<<fast.peak<<" shift max step="<<shift.maxStep
                 <<" mean idle="<<idle.mean<<" mean after shift="<<after.mean<<'\n';
        return ok;
    }
};
}

int main() {
    return EngineAudioTest{}.run()?0:1;
}
