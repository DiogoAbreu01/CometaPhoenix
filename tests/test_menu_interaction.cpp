#include "MenuInteraction.hpp"
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"MENU TEST FAIL line "<<__LINE__<<"\n";return 1;}}while(false)
int main(){
 using namespace phoenix;
 // At 1280x720 the UI scale is 1, panel origin is (44,58).
 CHECK(menuHitTest(100,350,1280,720,1280,720)==MenuAction::Start);
 CHECK(menuHitTest(100,455,1280,720,1280,720)==MenuAction::Course);
 CHECK(menuHitTest(120,540,1280,720,1280,720)==MenuAction::Laps);
 CHECK(menuHitTest(400,540,1280,720,1280,720)==MenuAction::Rivals);
 CHECK(menuHitTest(120,575,1280,720,1280,720)==MenuAction::Difficulty);
 CHECK(menuHitTest(380,575,1280,720,1280,720)==MenuAction::Volume);
 CHECK(menuHitTest(120,597,1280,720,1280,720)==MenuAction::History);
 CHECK(menuHitTest(340,597,1280,720,1280,720)==MenuAction::Keyboard);
 CHECK(menuHitTest(430,597,1280,720,1280,720)==MenuAction::Guide);
 CHECK(menuHitTest(140,640,1280,720,1280,720)==MenuAction::Colors);
 CHECK(menuHitTest(450,640,1280,720,1280,720)==MenuAction::Quality);
 CHECK(menuHitTest(135,665,1280,720,1280,720)==MenuAction::Championship);
 CHECK(menuHitTest(1200,350,1280,720,1280,720)==MenuAction::None);
 // Scale to a 1920x1080 drawable.
 CHECK(menuHitTest(150,525,1920,1080,1920,1080)==MenuAction::Start);
 // 1280x720 logical window, 2560x1440 high-DPI framebuffer; clamp 1.42.
 // Choose a point originally inside the 1.42-scaled start button.
 CHECK(menuHitTest(80,250,1280,720,2560,1440)==MenuAction::Start);
 CHECK(menuHitTest(10,2,1280,720,1280,720)==MenuAction::None);
 CHECK(menuHitTest(-1,250,1280,720,1280,720)==MenuAction::None);
 CHECK(menuHitTest(50,250,0,720,1280,720)==MenuAction::None);
 std::cout<<"JDM_MENU_POINTER_TESTS_PASS\n";
}
