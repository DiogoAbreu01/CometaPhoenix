#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <iostream>
#define REQUIRE(x) do { if(!(x)){ std::cerr<<"Asset invalido: "<<#x<<'\n'; return 1; }} while(false)
int main(int argc,char** argv){
 REQUIRE(argc==2);std::filesystem::path dir(argv[1]);
 std::ifstream f(dir/"kart.mesh",std::ios::binary);char tag[4]{};std::uint32_t v{},i{};
 f.read(tag,4);f.read(reinterpret_cast<char*>(&v),4);f.read(reinterpret_cast<char*>(&i),4);
 REQUIRE(f&&std::memcmp(tag,"PHXM",4)==0&&v>0&&v<2000000&&i>0&&i<6000000&&i%3==0);
 std::vector<float> vs(std::size_t(v)*13);std::vector<std::uint32_t> is(i);
 f.read(reinterpret_cast<char*>(vs.data()),vs.size()*sizeof(float));f.read(reinterpret_cast<char*>(is.data()),is.size()*sizeof(std::uint32_t));
 REQUIRE(f&&f.peek()==std::ifstream::traits_type::eof());
 REQUIRE(*std::max_element(is.begin(),is.end())<v);
 REQUIRE(std::all_of(vs.begin(),vs.end(),[](float x){return std::isfinite(x);}));
 REQUIRE(std::filesystem::file_size(dir/"kart_sprite.rgba")==512*512*4);
 REQUIRE(std::filesystem::file_size(dir/"asphalt_photo.rgba")==512*512*4);
 REQUIRE(std::filesystem::file_size(dir/"asphalt_normal.rgb")==512*512*3);
 REQUIRE(std::filesystem::file_size(dir/"phoenix_ui_atlas.r8")==768*360);
 REQUIRE(std::filesystem::file_size(dir/"phoenix_cinematic_menu.rgb")==1280*720*3);
 std::cout<<"Malha: "<<v<<" vertices; "<<i/3<<" triangulos. Assets corretos.\n";
}
