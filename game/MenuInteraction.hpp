#pragma once
#include <algorithm>
#include <cmath>

namespace phoenix {
enum class MenuAction {
    None, Start, Course, Laps, Rivals, Difficulty, Volume, History,
    Keyboard, Guide, Colors, Quality
};
// Menu screen coordinates are the same as the JDM HUD's reference 1280x720 layout.
// Caller supplies logical SDL window size and actual GL drawable size (HiDPI-safe).
inline MenuAction menuHitTest(int mx,int my,int winW,int winH,int drawW,int drawH) noexcept {
    if(winW<=0||winH<=0||drawW<640||drawH<480||mx<0||my<0||mx>=winW||my>=winH)
        return MenuAction::None;
    const double scale=std::max(.31,std::min({1.42,double(drawW)/1280.,double(drawH)/720.}));
    const double x=double(mx)*drawW/winW/scale-44.;
    const double y=double(my)*drawH/winH/scale-58.;
    if(x<23. || x>501.)return MenuAction::None;
    if(y>=259.&&y<=323.)return MenuAction::Start;
    if(y>=341.&&y<=457.)return MenuAction::Course;
    if(y>=468.&&y<504.)return x<239.?MenuAction::Laps:MenuAction::Rivals;
    if(y>=504.&&y<530.)return x<239.?MenuAction::Difficulty:MenuAction::Volume;
    if(y>=530.&&y<556.)return x<250.?MenuAction::History:x<365.?MenuAction::Keyboard:MenuAction::Guide;
    if(y>=556.&&y<592.)return x<250.?MenuAction::Colors:MenuAction::Quality;
    return MenuAction::None;
}
} // namespace phoenix
