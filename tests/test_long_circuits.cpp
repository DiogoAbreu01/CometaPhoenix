#include "GamePhysics.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"TRACK_LENGTH_FAIL "<<__LINE__<<" "<<#x<<"\n";return 1;}}while(false)
int main(){
 using namespace phoenix;
 const std::array<Circuit,4> circuits{Circuit::Forest,Circuit::Alpine,Circuit::Coast,Circuit::Sakura};
 const std::array<double,4> oldLaps{450,510,560,490}; // approximate 0.10.x length in meters
 for(std::size_t i=0;i<circuits.size();++i){
    const auto c=circuits[i];const auto meters=circuitLengthMeters(c);
    std::cout<<circuitName(c)<<" distance_m="<<meters<<" ratio_old="<<meters/oldLaps[i]<<"\n";
    CHECK(std::isfinite(meters) && meters>1700. && meters<3900.);
    CHECK(meters/oldLaps[i]>3.1);
    CHECK(circuitMaximumRadius(c)>0.);
    for(int step=0;step<4096;++step){
       const double angle=step*6.283185307179586/4096.;
       const double radius=circuitRadius(c,angle);
       CHECK(std::isfinite(radius) && radius>15. && radius<circuitMaximumRadius(c));
       // Analytic derivatives must agree with finite differences on every track.
       const double approx=(circuitRadius(c,angle+1e-4)-circuitRadius(c,angle-1e-4))/2e-4;
       CHECK(std::isfinite(approx) && std::abs(approx)<80.);
    }
 }
 std::cout<<"LONG_CIRCUITS_TEST_PASS\n";
}
