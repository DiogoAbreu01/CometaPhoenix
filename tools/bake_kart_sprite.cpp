// Offline C++17 orthographic sprite baker. PHXM -> 512x512 RGBA, OpenGL row order.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
using V=std::array<float,13>;
struct RGB {float r{},g{},b{};};
constexpr int S=1024, O=512;
constexpr float scale=S/2.64f;
float sat(float x){return std::clamp(x,0.f,1.f);}
float edge(float ax,float ay,float bx,float by,float x,float y){return (bx-ax)*(y-ay)-(by-ay)*(x-ax);}
int main(int argc,char**argv) try{
 if(argc!=3)throw std::runtime_error("usage: bake_kart_sprite mesh output.rgba");
 std::ifstream f(argv[1],std::ios::binary);char magic[4];std::uint32_t nv{},ni{};
 f.read(magic,4);f.read(reinterpret_cast<char*>(&nv),4);f.read(reinterpret_cast<char*>(&ni),4);
 if(!f||std::memcmp(magic,"PHXM",4)||!nv||nv>2000000||!ni||ni>6000000||ni%3)throw std::runtime_error("bad mesh header");
 std::vector<V> vertices(nv);std::vector<std::uint32_t> indices(ni);
 f.read(reinterpret_cast<char*>(vertices.data()),nv*sizeof(V));f.read(reinterpret_cast<char*>(indices.data()),ni*sizeof(std::uint32_t));
 if(!f||f.peek()!=EOF||*std::max_element(indices.begin(),indices.end())>=nv)throw std::runtime_error("bad mesh data");
 for(const auto&v:vertices)for(float x:v)if(!std::isfinite(x))throw std::runtime_error("nonfinite vertex");
 std::vector<float> z(S*S,-std::numeric_limits<float>::infinity());
 std::vector<RGB> color(S*S);
 for(std::size_t i=0;i<indices.size();i+=3){
  const V&a=vertices[indices[i]],&b=vertices[indices[i+1]],&c=vertices[indices[i+2]];
  float ax=a[0]*scale+S*.5f,ay=a[1]*scale+S*.5f,bx=b[0]*scale+S*.5f,by=b[1]*scale+S*.5f,cx=c[0]*scale+S*.5f,cy=c[1]*scale+S*.5f;
  float area=edge(ax,ay,bx,by,cx,cy);if(std::abs(area)<1e-7f)continue;
  int x0=std::clamp(int(std::floor(std::min({ax,bx,cx}))),0,S-1),x1=std::clamp(int(std::ceil(std::max({ax,bx,cx}))),0,S-1);
  int y0=std::clamp(int(std::floor(std::min({ay,by,cy}))),0,S-1),y1=std::clamp(int(std::ceil(std::max({ay,by,cy}))),0,S-1);
  for(int y=y0;y<=y1;++y)for(int x=x0;x<=x1;++x){
   float u=edge(bx,by,cx,cy,x+.5f,y+.5f)/area,v=edge(cx,cy,ax,ay,x+.5f,y+.5f)/area,w=1-u-v;
   if(std::min({u,v,w})<0)continue;
   auto at=[&](int k){return u*a[k]+v*b[k]+w*c[k];};
   float h=at(2);auto p=y*S+x;if(h<=z[p])continue;z[p]=h;
   float nx=at(3),ny=at(4),nz=at(5),len=std::sqrt(nx*nx+ny*ny+nz*nz);
   if(len>1e-6f){nx/=len;ny/=len;nz/=len;}else nz=1;
   if(nz<0){nx=-nx;ny=-ny;nz=-nz;}
   float diffuse=sat(-.36f*nx+.48f*ny+.8f*nz);
   float metal=sat(a[9]),rough=std::clamp(a[10],.12f,1.f);
   float highlight=std::pow(sat(-.2f*nx+.26f*ny+.94f*nz),4.f+45.f*(1-rough))*(.07f+.20f*metal);
   float ambient=.34f+.55f*diffuse, crevice=.78f+.22f*sat(h/.8f);
   auto shade=[&](int k){float linear=sat(a[k]*ambient*crevice+highlight);return std::pow(linear,.84f);};
   color[p]={shade(6),shade(7),shade(8)};
  }
 }
 std::vector<unsigned char> rgba(O*O*4);std::size_t covered=0;
 for(int y=0;y<O;++y)for(int x=0;x<O;++x){
  RGB sum{};int count=0;
  for(int j=0;j<2;++j)for(int i=0;i<2;++i){auto p=(2*y+j)*S+2*x+i;if(!std::isfinite(z[p]))continue;sum.r+=color[p].r;sum.g+=color[p].g;sum.b+=color[p].b;++count;}
  if(!count)continue;++covered;auto k=4*(y*O+x);
  rgba[k]=static_cast<unsigned char>(std::lround(255*sat(sum.r/count)));
  rgba[k+1]=static_cast<unsigned char>(std::lround(255*sat(sum.g/count)));
  rgba[k+2]=static_cast<unsigned char>(std::lround(255*sat(sum.b/count)));
  rgba[k+3]=static_cast<unsigned char>(std::lround(255.f*count/4.f));
 }
 if(covered<O*O/10||covered>O*O*8/10)throw std::runtime_error("unexpected silhouette");
 std::ofstream out(argv[2],std::ios::binary);out.write(reinterpret_cast<const char*>(rgba.data()),rgba.size());
 if(!out)throw std::runtime_error("write failed");
 std::cout<<"SPRITE_OK "<<nv<<" vertices "<<ni/3<<" triangles "<<covered<<" covered pixels\n";
 return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
