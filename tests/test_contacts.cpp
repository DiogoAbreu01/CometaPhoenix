#include "KartCollision.hpp"
#include <iostream>
#include <cmath>
#define CHECK(x) do{if(!(x)){std::cerr<<"CONTACT FAIL "<<__LINE__<<": "<<#x<<"\n";return 1;}}while(false)
int main(){
 using namespace phoenix;
 Vehicle player{};player.x=5.;player.y=0.;player.angle=1.57079632679;player.speed=48.;
 GhostPose same{5.,.1,1.57};
 auto a=detectKartContact(player,same,{5.,.09,1.57},.02);
 CHECK(a.overlap);CHECK(a.penetrationUnits>.01);CHECK(std::abs(std::hypot(a.nx,a.ny)-1.)<1e-10);
 CHECK(a.closingSpeedMs>0.);
 CHECK(!detectKartContact(player,{6.,3.,0.},{6.,3.,0.},.02).overlap);
 CHECK(!detectKartContact(player,same,same,-1.).overlap);
 CHECK(!detectKartContact(player,same,same,0.).overlap);
 CHECK(!detectKartContact(player,same,same,1.).overlap);
 auto b=detectKartContact(player,same,{5.,.09,1.57},.02);
 CHECK(a.closingSpeedMs==b.closingSpeedMs);
 CHECK(a.nx==b.nx && a.ny==b.ny);
 player.speed=0.;auto c=detectKartContact(player,{5.,0.,0.},{5.,0.,0.},.02);
 CHECK(c.overlap && std::isfinite(c.nx));
 Game game;
 auto pv=game.vehicle();
 auto gentle=game.collideRival({pv.x,pv.y+.09,pv.angle},{pv.x,pv.y+.09,pv.angle},.02);
 CHECK(gentle.overlap && !gentle.damaged);
 CHECK(!game.hasDamage());
 CHECK(std::hypot(game.vehicle().x-pv.x,game.vehicle().y-pv.y)>.02);
 game.restart();
 pv=game.vehicle();
 // A high-speed kart catches the player from behind, creating a repair event.
 const auto hard=game.collideRival({pv.x,pv.y-.08,pv.angle},
                                  {pv.x,pv.y-.18,pv.angle},.02);
 CHECK(hard.overlap && hard.closingSpeedMs>=6.0 && hard.damaged);
 CHECK(game.hasDamage());
 Game noContact;
 CHECK(!noContact.collideRival({22.,20.,0.},{22.,20.,0.},.02).overlap);
 std::cout<<"CONTACT_DETECTION_OK\n";
}
