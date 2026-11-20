// COMETA PHOENIX - Jogo nativo SDL2/OpenGL em C++17.
// SDL2 e apenas a camada de janela/entrada; simulacao, UI e shaders sao proprios.
#include "GamePhysics.hpp"
#include "RivalAI.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <numeric>
#include <limits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdio>
#include <cstddef>
#include <chrono>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

extern "C" {
struct SDL_Window;
int SDL_Init(std::uint32_t);
void SDL_Quit();
SDL_Window* SDL_CreateWindow(const char*,int,int,int,int,std::uint32_t);
void SDL_DestroyWindow(SDL_Window*);
int SDL_GL_SetAttribute(int,int);
void* SDL_GL_CreateContext(SDL_Window*);
void SDL_GL_DeleteContext(void*);
int SDL_GL_SetSwapInterval(int);
void SDL_GL_SwapWindow(SDL_Window*);
int SDL_PollEvent(void*);
const std::uint8_t* SDL_GetKeyboardState(int*);
void SDL_GetWindowSize(SDL_Window*,int*,int*);
void SDL_GL_GetDrawableSize(SDL_Window*,int*,int*);
int SDL_SetWindowFullscreen(SDL_Window*,std::uint32_t);
void* SDL_GL_GetProcAddress(const char*);
std::uint32_t SDL_GetTicks();
void SDL_Delay(std::uint32_t);
const char* SDL_GetError();
char* SDL_GetPrefPath(const char*,const char*);
void SDL_free(void*);
struct SDL_AudioSpec {int freq; std::uint16_t format; std::uint8_t channels,silence; std::uint16_t samples,padding; std::uint32_t size; void (*callback)(void*,std::uint8_t*,int); void* userdata;};
std::uint32_t SDL_OpenAudioDevice(const char*,int,const SDL_AudioSpec*,SDL_AudioSpec*,int);
void SDL_PauseAudioDevice(std::uint32_t,int);
void SDL_CloseAudioDevice(std::uint32_t);
int SDL_InitSubSystem(std::uint32_t);

}
namespace {
using GLuint=unsigned int;using GLenum=unsigned int;using GLint=int;using GLsizei=int;using GLsizeiptr=std::ptrdiff_t;using GLfloat=float;using GLchar=char;using GLboolean=unsigned char;
constexpr GLenum DEPTH=0x0B71,DEPTH_BIT=0x0100,ELEMENT=0x8893,STATIC=0x88E4,UINT=0x1405, VS=0x8B31,FS=0x8B30,COMPILE=0x8B81,LINK=0x8B82,ARRAY=0x8892,DYNAMIC=0x88E8,FLOAT=0x1406,TRIANGLES=0x0004,COLOR=0x4000,BLEND=0x0BE2,SRC_ALPHA=0x0302,ONE_MINUS_SRC_ALPHA=0x0303,RGBA=0x1908,UNSIGNED_BYTE=0x1401,RGB=0x1907,RED=0x1903,
TEXTURE_2D=0x0DE1,TEX_MIN=0x2801,TEX_MAG=0x2800,TEX_WRAP_S=0x2802,TEX_WRAP_T=0x2803,LINEAR=0x2601,CLAMP=0x812F,FRAMEBUFFER=0x8D40,COLOR_ATTACHMENT0=0x8CE0,FRAMEBUFFER_COMPLETE=0x8CD5,TEXTURE0=0x84C0;
#define GLFUN(name,ret,...) ret (*name)(__VA_ARGS__){};
struct GLApi{
 GLFUN(CreateShader,GLuint,GLenum) GLFUN(ShaderSource,void,GLuint,GLsizei,const GLchar* const*,const GLint*) GLFUN(CompileShader,void,GLuint)
 GLFUN(GetShaderiv,void,GLuint,GLenum,GLint*) GLFUN(GetShaderInfoLog,void,GLuint,GLsizei,GLsizei*,GLchar*) GLFUN(DeleteShader,void,GLuint)
 GLuint (*CreateProgram)(){}; GLFUN(AttachShader,void,GLuint,GLuint) GLFUN(LinkProgram,void,GLuint) GLFUN(GetProgramiv,void,GLuint,GLenum,GLint*)
 GLFUN(GetProgramInfoLog,void,GLuint,GLsizei,GLsizei*,GLchar*) GLFUN(UseProgram,void,GLuint) GLFUN(DeleteProgram,void,GLuint)
 GLFUN(Uniform3f,void,GLint,GLfloat,GLfloat,GLfloat) GLFUN(DrawElements,void,GLenum,GLsizei,GLenum,const void*) GLFUN(DeleteBuffers,void,GLsizei,const GLuint*) void (*Finish)(){};
 GLFUN(GetUniformLocation,GLint,GLuint,const GLchar*) GLFUN(Uniform1f,void,GLint,GLfloat) GLFUN(Uniform2f,void,GLint,GLfloat,GLfloat) GLFUN(Uniform1i,void,GLint,GLint)
 GLFUN(GenVertexArrays,void,GLsizei,GLuint*) GLFUN(BindVertexArray,void,GLuint) GLFUN(DeleteVertexArrays,void,GLsizei,const GLuint*)
 GLFUN(GenBuffers,void,GLsizei,GLuint*) GLFUN(BindBuffer,void,GLenum,GLuint) GLFUN(BufferData,void,GLenum,GLsizeiptr,const void*,GLenum)
 GLFUN(EnableVertexAttribArray,void,GLuint) GLFUN(VertexAttribPointer,void,GLuint,GLint,GLenum,GLboolean,GLsizei,const void*)
 GLFUN(Viewport,void,GLint,GLint,GLsizei,GLsizei) GLFUN(DrawArrays,void,GLenum,GLint,GLsizei)
 GLFUN(ClearColor,void,GLfloat,GLfloat,GLfloat,GLfloat) GLFUN(Clear,void,GLenum)
 GLFUN(Enable,void,GLenum) GLFUN(Disable,void,GLenum) GLFUN(BlendFunc,void,GLenum,GLenum)
 GLFUN(ReadPixels,void,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,void*) GLFUN(GetString,const unsigned char*,GLenum)
 GLFUN(GenTextures,void,GLsizei,GLuint*) GLFUN(DeleteTextures,void,GLsizei,const GLuint*) GLFUN(BindTexture,void,GLenum,GLuint)
 GLFUN(TexImage2D,void,GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const void*)
 GLFUN(TexParameteri,void,GLenum,GLenum,GLint) GLFUN(ActiveTexture,void,GLenum) GLFUN(GenerateMipmap,void,GLenum)
 GLFUN(GenFramebuffers,void,GLsizei,GLuint*) GLFUN(BindFramebuffer,void,GLenum,GLuint) GLFUN(DeleteFramebuffers,void,GLsizei,const GLuint*)
 GLFUN(GenRenderbuffers,void,GLsizei,GLuint*) GLFUN(BindRenderbuffer,void,GLenum,GLuint) GLFUN(RenderbufferStorage,void,GLenum,GLenum,GLsizei,GLsizei) GLFUN(FramebufferRenderbuffer,void,GLenum,GLenum,GLenum,GLuint) GLFUN(DeleteRenderbuffers,void,GLsizei,const GLuint*)
 GLFUN(FramebufferTexture2D,void,GLenum,GLenum,GLenum,GLuint,GLint) GLFUN(CheckFramebufferStatus,GLenum,GLenum)
 void load(){
 #define LOAD(n) n=reinterpret_cast<decltype(n)>(SDL_GL_GetProcAddress("gl" #n));if(!n)throw std::runtime_error("OpenGL nao suporta gl" #n);
 LOAD(CreateShader)LOAD(ShaderSource)LOAD(CompileShader)LOAD(GetShaderiv)LOAD(GetShaderInfoLog)LOAD(DeleteShader)
 LOAD(CreateProgram)LOAD(AttachShader)LOAD(LinkProgram)LOAD(GetProgramiv)LOAD(GetProgramInfoLog)LOAD(UseProgram)LOAD(DeleteProgram)
 LOAD(Uniform3f)LOAD(DrawElements)LOAD(DeleteBuffers)LOAD(Finish)
 LOAD(GetUniformLocation)LOAD(Uniform1f)LOAD(Uniform2f)LOAD(Uniform1i)LOAD(GenVertexArrays)LOAD(BindVertexArray)LOAD(DeleteVertexArrays)
 LOAD(GenBuffers)LOAD(BindBuffer)LOAD(BufferData)LOAD(EnableVertexAttribArray)LOAD(VertexAttribPointer)LOAD(Viewport)LOAD(DrawArrays)
 LOAD(ClearColor)LOAD(Clear)LOAD(Enable)LOAD(Disable)LOAD(BlendFunc)LOAD(ReadPixels)LOAD(GetString)
 LOAD(GenTextures)LOAD(DeleteTextures)LOAD(BindTexture)LOAD(TexImage2D)LOAD(TexParameteri)LOAD(ActiveTexture)LOAD(GenerateMipmap)
 LOAD(GenRenderbuffers)LOAD(BindRenderbuffer)LOAD(RenderbufferStorage)LOAD(FramebufferRenderbuffer)LOAD(DeleteRenderbuffers)
 LOAD(GenFramebuffers)LOAD(BindFramebuffer)LOAD(DeleteFramebuffers)LOAD(FramebufferTexture2D)LOAD(CheckFramebufferStatus)
 #undef LOAD
 }
};
#undef GLFUN
std::string readText(const std::string& file){std::ifstream f(file);if(!f)throw std::runtime_error("Nao foi possivel abrir shader: "+file);return std::string((std::istreambuf_iterator<char>(f)),{});}
GLuint shader(GLApi&gl,GLenum type,const std::string&code){GLuint s=gl.CreateShader(type);const char*p=code.c_str();gl.ShaderSource(s,1,&p,nullptr);gl.CompileShader(s);int ok=0;gl.GetShaderiv(s,COMPILE,&ok);if(!ok){char buf[8192]{};gl.GetShaderInfoLog(s,sizeof(buf),nullptr,buf);throw std::runtime_error(std::string("Shader GLSL invalido: ")+buf);}return s;}
GLuint program(GLApi&gl,const std::string&v,const std::string&f){auto vs=shader(gl,VS,v),fs=shader(gl,FS,f);GLuint p=gl.CreateProgram();gl.AttachShader(p,vs);gl.AttachShader(p,fs);gl.LinkProgram(p);int ok=0;gl.GetProgramiv(p,LINK,&ok);gl.DeleteShader(vs);gl.DeleteShader(fs);if(!ok){char buf[8192]{};gl.GetProgramInfoLog(p,sizeof(buf),nullptr,buf);throw std::runtime_error(std::string("Link GLSL: ")+buf);}return p;}
const std::string FULLSCREEN_VERTEX=R"GLSL(#version 330 core
out vec2 uv;
void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);uv=p;gl_Position=vec4(p*2.-1.,0,1);}
)GLSL";
const std::string UI_VERTEX=R"GLSL(#version 330 core
layout(location=0) in vec2 xy;layout(location=1) in vec4 color;
out vec4 tint;uniform vec2 res;
void main(){tint=color;gl_Position=vec4(xy.x/res.x*2.-1.,1.-xy.y/res.y*2.,0.,1.);}
)GLSL";
const std::string UI_FRAGMENT=R"GLSL(#version 330 core
in vec4 tint;out vec4 FragColor;void main(){FragColor=tint;}
)GLSL";
const std::string FONT_VERTEX=R"GLSL(#version 330 core
layout(location=0) in vec2 xy;
layout(location=1) in vec2 texUV;
layout(location=2) in vec4 color;
out vec2 uv;out vec4 tint;uniform vec2 res;
void main(){uv=texUV;tint=color;gl_Position=vec4(xy.x/res.x*2.-1.,1.-xy.y/res.y*2.,0.,1.);}
)GLSL";
const std::string FONT_FRAGMENT=R"GLSL(#version 330 core
in vec2 uv;in vec4 tint;out vec4 FragColor;uniform sampler2D atlas;
void main(){float a=texture(atlas,uv).r;FragColor=vec4(tint.rgb,tint.a*a);}
)GLSL";
const std::string MENU_FRAGMENT=R"GLSL(#version 330 core
in vec2 uv;out vec4 FragColor;uniform sampler2D imageTex;uniform float u_time;
void main(){vec2 tc=vec2(uv.x,1.-uv.y);vec3 c=texture(imageTex,tc).rgb;
float left=smoothstep(.00,.70,uv.x);c*=mix(.36,.88,left);
c+=vec3(.0,.015,.02)*(.5+.5*sin(u_time*.7));
c=pow(c,vec3(.98));FragColor=vec4(c,1.);}
)GLSL";
const std::string UPSCALE_FRAGMENT=R"GLSL(#version 330 core
in vec2 uv;out vec4 FragColor;uniform sampler2D sceneTex;uniform vec2 sourceRes;uniform float sharpen;
void main(){vec2 tc=uv;vec3 col=texture(sceneTex,tc).rgb;
if(sharpen>.01){vec2 p=1./sourceRes;
vec3 blur=(texture(sceneTex,tc+vec2(p.x,0.)).rgb+texture(sceneTex,tc-vec2(p.x,0.)).rgb+
texture(sceneTex,tc+vec2(0.,p.y)).rgb+texture(sceneTex,tc-vec2(0.,p.y)).rgb)*.25;
col=clamp(col+(col-blur)*sharpen,0.,1.);}
FragColor=vec4(col,1.);}
)GLSL";
std::vector<unsigned char> readBinary(const std::filesystem::path& file,std::size_t expected){
 std::ifstream f(file,std::ios::binary);if(!f)throw std::runtime_error("Asset ausente: "+file.string());
 std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)),{});
 if(bytes.size()!=expected)throw std::runtime_error("Tamanho invalido de asset: "+file.string());
 return bytes;
}
GLuint loadTex(GLApi&gl,int w,int h,GLenum fmt,const unsigned char* data,bool tile=false){
 GLuint t=0;gl.GenTextures(1,&t);gl.BindTexture(TEXTURE_2D,t);gl.TexImage2D(TEXTURE_2D,0,(GLint)fmt,w,h,0,fmt,UNSIGNED_BYTE,data);
 gl.TexParameteri(TEXTURE_2D,TEX_MIN,LINEAR);gl.TexParameteri(TEXTURE_2D,TEX_MAG,LINEAR);
 gl.TexParameteri(TEXTURE_2D,TEX_WRAP_S,tile?0x2901:CLAMP);gl.TexParameteri(TEXTURE_2D,TEX_WRAP_T,tile?0x2901:CLAMP);if(tile){gl.GenerateMipmap(TEXTURE_2D);gl.TexParameteri(TEXTURE_2D,TEX_MIN,0x2703);}return t;
}
struct RenderTarget{
 GLuint texture{},framebuffer{},depth{};int w{},h{};
 void reset(GLApi&gl){if(framebuffer)gl.DeleteFramebuffers(1,&framebuffer);if(texture)gl.DeleteTextures(1,&texture);if(depth)gl.DeleteRenderbuffers(1,&depth);framebuffer=texture=depth=0;w=h=0;}
 void resize(GLApi&gl,int W,int H){if(w==W&&h==H)return;reset(gl);w=W;h=H;
   texture=loadTex(gl,w,h,RGBA,nullptr);gl.GenFramebuffers(1,&framebuffer);gl.BindFramebuffer(FRAMEBUFFER,framebuffer);
   gl.FramebufferTexture2D(FRAMEBUFFER,COLOR_ATTACHMENT0,TEXTURE_2D,texture,0);
   gl.GenRenderbuffers(1,&depth);gl.BindRenderbuffer(0x8D41,depth);gl.RenderbufferStorage(0x8D41,0x81A6,w,h);gl.FramebufferRenderbuffer(FRAMEBUFFER,0x8D00,0x8D41,depth);
   if(gl.CheckFramebufferStatus(FRAMEBUFFER)!=FRAMEBUFFER_COMPLETE)throw std::runtime_error("Framebuffer incompleto");
   gl.BindFramebuffer(FRAMEBUFFER,0);
 }
};
struct Color {float r{},g{},b{},a{1.};};
const Color WHITE{.93f,.96f,.96f,1},MUTED{.47f,.64f,.68f,1},CYAN{.27f,.98f,.83f,1},WARN{1.f,.7f,.25f,1},BAD{1.f,.30f,.23f,1};
struct UI {
 std::vector<float> verts,letterVerts;
 void tri(float x,float y,Color c){verts.insert(verts.end(),{x,y,c.r,c.g,c.b,c.a});}
 void rect(float x,float y,float w,float h,Color c){if(w<=0||h<=0)return;tri(x,y,c);tri(x+w,y,c);tri(x+w,y+h,c);tri(x,y,c);tri(x+w,y+h,c);tri(x,y+h,c);}
 void line(float x,float y,float x2,float y2,float wide,Color c){
   float dx=x2-x,dy=y2-y,len=std::sqrt(dx*dx+dy*dy);if(len<.001f)return;
   float ox=-dy/len*wide*.5f,oy=dx/len*wide*.5f;
   tri(x+ox,y+oy,c);tri(x2+ox,y2+oy,c);tri(x2-ox,y2-oy,c);
   tri(x+ox,y+oy,c);tri(x2-ox,y2-oy,c);tri(x-ox,y-oy,c);
 }
 void ring(float cx,float cy,float r,float thick,float start,float span,Color c,int n=48){
   for(int i=0;i<n;i++){
      float a=start+span*i/n,b=start+span*(i+1)/n;
      float ca=std::cos(a),sa=std::sin(a),cb=std::cos(b),sb=std::sin(b);
      line(cx+r*ca,cy+r*sa,cx+r*cb,cy+r*sb,thick,c);
   }
 }
 void letter(float x,float y,float wid,float hei,float u0,float v0,float u1,float v1,Color c){
   auto pt=[&](float px,float py,float u,float v){letterVerts.insert(letterVerts.end(),{px,py,u,v,c.r,c.g,c.b,c.a});};
   pt(x,y,u0,v0);pt(x+wid,y,u1,v0);pt(x+wid,y+hei,u1,v1);
   pt(x,y,u0,v0);pt(x+wid,y+hei,u1,v1);pt(x,y+hei,u0,v1);
 }
 void text(const std::string& txt,float x,float y,float px,Color c){
    const float start=x;
    for(unsigned char ch:txt){if(ch=='\n'){y+=px*9.f;x=start;continue;}if(ch<32||ch>126){x+=px*6;continue;}
      const int idx=(int)ch-32;const float U0=(idx%16)*48.f/768.f,V0=(idx/16)*60.f/360.f;
      letter(x,y,5.4f*px,7.3f*px,U0,V0,U0+48.f/768.f,V0+60.f/360.f,c);
      x+=px*6.0f;
    }
 }
};
std::string intStr(int n){return std::to_string(n);}
std::string fmt(double d,int precision=0){if(!std::isfinite(d))return "0";int p=std::max(0,std::min(2,precision));char buf[64];std::snprintf(buf,sizeof(buf),"%.*f",p,d);return buf;}
GLint uniform(GLApi&gl,GLuint p,const char*k){static std::unordered_map<GLuint,std::unordered_map<std::string,GLint>> cache;auto &m=cache[p];auto i=m.find(k);if(i!=m.end())return i->second;return m.emplace(k,gl.GetUniformLocation(p,k)).first->second;}
void setf(GLApi& gl,GLuint p,const char*k,float f){int u=uniform(gl,p,k);if(u>=0)gl.Uniform1f(u,f);}
void set2(GLApi& gl,GLuint p,const char*k,float x,float y){int u=uniform(gl,p,k);if(u>=0)gl.Uniform2f(u,x,y);}
#include "NativeExtras.hpp"
float partId(const std::string& s){if(s=="chassi")return 1;if(s=="carenagem")return 2;if(s=="suspensao")return 3;if(s=="pneu_de")return 11;if(s=="pneu_dd")return 12;if(s=="pneu_te")return 13;if(s=="pneu_td")return 14;if(s.find("pneu_")==0)return 4;if(s=="motor")return 5;if(s=="freio")return 6;return 3;}
void drawHUD(UI & ui,const phoenix::Game &game,const std::array<phoenix::RivalAI,2>& rivals,int rivalCount,int w,int h,int quality,bool paused,double fps,bool ghostVisible){
 auto &sim=game.simulator();auto &veh=game.vehicle();bool shop=game.garage();float S=std::max(.30f,std::min({1.7f,(float)w/1280.f,(float)h/720.f}));
 ui.rect(0,0,(float)w,82*S,{.018f,.037f,.053f,.91f});ui.rect(0,79*S,(float)w,2*S,CYAN);
 ui.text("COMETA",30*S,19*S,4*S,WHITE);ui.text("PHOENIX",205*S,19*S,4*S,CYAN);
 ui.text("REGENERATIVE RACING LAB / C++17",31*S,53*S,1.5f*S,MUTED);
 ui.text(shop?"GARAGEM 3D / DIAGNOSTICO":"PISTA 2D / CONTROLE EM TEMPO REAL",std::max(30.f,(float)w-445*S),29*S,2*S,WHITE);
 if(!shop){
  ui.rect(25*S,103*S,277*S,194*S,{.012f,.029f,.041f,.83f});ui.rect(25*S,103*S,3*S,194*S,CYAN);
  ui.text("TELEMETRIA",44*S,119*S,2.8f*S,CYAN);
  ui.text("VELOCIDADE",44*S,164*S,2*S,MUTED);ui.text(fmt(veh.speed),44*S,187*S,5.4f*S,WHITE);ui.text("KM/H",165*S,207*S,2*S,CYAN);
  ui.text("ADERENCIA",44*S,260*S,1.9f*S,MUTED);
  ui.rect(153*S,259*S,117*S,8*S,{.12f,.22f,.24f,1});ui.rect(153*S,259*S,117*S*(float)phoenix::climate(game.weather()).grip,8*S,CYAN);
  if(rivalCount>0){
    int position=1;
    const double playerProg=game.vehicle().lap+game.lapProgress();
    for(int i=0;i<rivalCount;++i){
      if(game.finished())position+=rivals[i].finished(game.targetLaps()) && rivals[i].raceTime()<game.worldTime();
      else position+=rivals[i].totalProgress()/6.283185307179586>playerProg;
    }
    ui.text("POSICAO "+intStr(position)+"/"+intStr(rivalCount+1),44*S,284*S,1.6f*S,WARN);
  }
  ui.rect((float)w-267*S,103*S,242*S,193*S,{.013f,.031f,.046f,.88f});
  ui.text("CONDICAO",(float)w-246*S,118*S,2.4f*S,MUTED);
  ui.text(phoenix::weatherName(game.weather()),(float)w-245*S,155*S,2.9f*S,CYAN);
  ui.text("VOLTA "+intStr(game.finished()?game.validCompletedLaps():game.validCompletedLaps()+1)+(game.targetLaps()>0?"/"+intStr(game.targetLaps()):" LIVRE"),(float)w-245*S,201*S,2.2f*S,WHITE);
  ui.text("TEMPO "+fmt(game.currentLapTime(),2)+" S",(float)w-245*S,229*S,1.7f*S,game.validLap()?WHITE:BAD);
  ui.text("MELHOR "+(game.bestLapTime()>0.?fmt(game.bestLapTime(),2)+" S":std::string("--")),(float)w-245*S,259*S,1.7f*S,CYAN);
  if(game.hasGhost())ui.text(ghostVisible?"FANTASMA ON":"FANTASMA OFF",(float)w-245*S,282*S,1.45f*S,ghostVisible?CYAN:MUTED);
  // Conta-giros circular estilo HUD de corridas; um unico VBO por frame.
  float cx=(float)w-132*S,cy=(float)h-187*S;
  ui.ring(cx,cy,66*S,4*S,2.45f,4.58f,{.14f,.25f,.28f,.85f});
  ui.ring(cx,cy,66*S,6*S,2.45f,4.58f*std::min(1.f,(float)(veh.speed/102.)),CYAN);
  ui.text(fmt(veh.speed,0),cx-48*S,cy-12*S,5.0f*S,WHITE);
  ui.text("KM/H",cx-28*S,cy+30*S,1.6f*S,MUTED);
  // Mini mapa top-down com o kart ativo.
  const float mx=(float)w-107*S,my=365*S;
  ui.rect(mx-76*S,my-59*S,152*S,134*S,{.012f,.033f,.041f,.83f});
  for(int i=0;i<46;i++){
    float a=6.2831853f*i/46.,b=6.2831853f*(i+1)/46.;
    float ra=(float)phoenix::circuitRadius(game.circuit(),a)*8*S,rb=(float)phoenix::circuitRadius(game.circuit(),b)*8*S;
    ui.line(mx+ra*std::cos(a),my+ra*.7f*std::sin(a),mx+rb*std::cos(b),my+rb*.7f*std::sin(b),6*S,{.30f,.48f,.49f,.9f});
  }
  float ang=std::atan2((float)veh.y,(float)veh.x),ra=(float)phoenix::circuitRadius(game.circuit(),ang)*8*S;
  ui.rect(mx+ra*std::cos(ang)-4*S,my+ra*.7f*std::sin(ang)-4*S,8*S,8*S,CYAN);
  for(int i=0;i<rivalCount;++i){
     const auto other=rivals[i].pose();
     ui.rect(mx+float(other.x)*8*S-3*S,my+float(other.y)*5.6f*S-3*S,6*S,6*S,i==0?BAD:WARN);
  }
  ui.text("MAPA",mx-62*S,my-55*S,1.5f*S,MUTED);
  if(game.cinematic()>0){ui.rect(0,(float)h*.46f,(float)w,90*S,{.31f,.024f,.02f,.6f});ui.text("ALERTA DE COLISAO",(float)w*.24f,(float)h*.48f,4.5f*S,BAD);}
 }else{
  ui.rect(22*S,104*S,328*S,261*S,{.01f,.03f,.042f,.86f});ui.rect(22*S,104*S,3*S,261*S,CYAN);
  ui.text("DIAGNOSTICO ESTRUTURAL",43*S,123*S,2.1f*S,CYAN);
  {
   const auto& p=sim.componenteMaisCritico();
   ui.text("PECA: "+p.id,44*S,169*S,2.1f*S,WHITE);
   ui.text("INTEGRIDADE: "+fmt(p.integridade*100,1)+"%",44*S,205*S,2.1f*S,(p.integridade<.75?BAD:CYAN));
   ui.text("TENSAO: "+fmt(p.tensaoMPa,1)+" MPA",44*S,238*S,2.1f*S,WHITE);
   ui.text("FADIGA: "+fmt(p.fadiga,2),44*S,271*S,2.1f*S,MUTED);
  }
  ui.text("REGEN: "+fmt(game.repairFraction()*100,0)+"%",44*S,309*S,2.1f*S,CYAN);
  ui.rect(44*S,335*S,275*S,10*S,{.13f,.2f,.23f,1});ui.rect(44*S,335*S,(float)(275*S*game.repairFraction()),10*S,CYAN);
  ui.rect((float)w-285*S,105*S,265*S,125*S,{.01f,.031f,.043f,.90f});
  ui.text("RECURSOS BOX",(float)w-266*S,123*S,2.4f*S,CYAN);
  ui.text("ENERGIA "+fmt(sim.recursos().bateriaWh,0)+" WH",(float)w-266*S,164*S,2*S,WHITE);
  ui.text("SELANTE "+fmt(sim.recursos().agenteMl,0)+" ML",(float)w-266*S,195*S,2*S,WHITE);
 }
 ui.rect(0,(float)h-73*S,(float)w,73*S,{.014f,.036f,.05f,.93f});
 ui.rect(0,(float)h-73*S,(float)w,2*S,CYAN);
 ui.text("STATUS: "+game.status(),28*S,(float)h-55*S,1.95f*S,sim.fase()==cometa::Fase::Abandono?BAD:CYAN);
 ui.text(shop?"A/D OU SETAS: GIRAR CAMERA  |  P PAUSA  |  Q GFX  |  R REINICIA":"WASD/SETAS DIRIGIR  |  1-5 CLIMA  |  X DANO  |  R REINICIA  |  P PAUSA  |  Q GFX  |  M MAPA  |  G FANTASMA  |  V SOM",28*S,(float)h-25*S,1.55f*S,MUTED);
 ui.text("FPS "+fmt(fps,0),std::max(0.f,(float)w-255*S),(float)h-52*S,1.7f*S,CYAN);
 ui.text("GFX "+intStr(quality),std::max(0.f,(float)w-116*S),(float)h-52*S,1.8f*S,WARN);
 if(game.finished()){ui.rect(0,(float)h*.39f,(float)w,126*S,{.008f,.05f,.046f,.94f});ui.text("CORRIDA CONCLUIDA",(float)w*.22f,(float)h*.41f,4.4f*S,CYAN);ui.text("TOTAL "+fmt(game.worldTime(),2)+" S  |  R REINICIAR",(float)w*.23f,(float)h*.49f,2.1f*S,WHITE);}
 else if(paused){ui.rect(0,(float)h*.46f,(float)w,70*S,{.005f,.012f,.021f,.82f});ui.text("PAUSADO",(float)w*.38f,(float)h*.48f,4.8f*S,WHITE);}
}
void drawMenu(UI &ui,int w,int h,int quality,phoenix::Circuit circuit,int laps,int rivals){
 const float S=std::max(.28f,std::min({1.4f,(float)w/1280.f,(float)h/720.f}));
 ui.rect(0,0,(float)w,15*S,{.012f,.08f,.09f,.82f});
 ui.rect(35*S,120*S,7*S,245*S,CYAN);
 ui.text("COMETA",65*S,143*S,9*S,WHITE);
 ui.text("PHOENIX",65*S,220*S,9*S,CYAN);
 ui.text("REGENERATIVE RACING",70*S,321*S,2.5f*S,WHITE);
 ui.rect(65*S,383*S,445*S,64*S,{.013f,.24f,.25f,.83f});
 ui.text("[ ENTER ] INICIAR CORRIDA",81*S,399*S,2.9f*S,WHITE);
 ui.rect(65*S,466*S,445*S,118*S,{.008f,.02f,.03f,.84f});
 ui.text(std::string("[ T ] CIRCUITO: ")+phoenix::circuitName(circuit),84*S,482*S,2*S,WHITE);
 ui.text("[ L ] VOLTAS: "+(laps?intStr(laps):std::string("LIVRE")),84*S,518*S,2*S,MUTED);
 ui.text("[ K ] RIVAIS: "+intStr(rivals)+"   F11 TELA CHEIA   M MAPA",84*S,554*S,1.8f*S,MUTED);
 ui.rect(0,(float)h-60*S,(float)w,60*S,{.009f,.023f,.034f,.89f});
 ui.text("COMETA PHOENIX / C++17 + OPENGL 3.3",34*S,(float)h-39*S,2*S,MUTED);
 ui.text("PERFIL "+intStr(quality),std::max(20.f,(float)w-175*S),(float)h-39*S,2*S,CYAN);
}

void screenshot(GLApi&gl,const std::string&name,int w,int h){
 std::vector<unsigned char> pixels(std::size_t(w)*h*4);gl.ReadPixels(0,0,w,h,RGBA,UNSIGNED_BYTE,pixels.data());
 std::ofstream f(name,std::ios::binary);if(!f)throw std::runtime_error("Sem permissao para salvar screenshot");f<<"P6\n"<<w<<" "<<h<<"\n255\n";for(int y=h-1;y>=0;--y)for(int x=0;x<w;x++){const auto*p=&pixels[4*(std::size_t(y)*w+x)];f.write(reinterpret_cast<const char*>(p),3);}std::cout<<"SCREENSHOT "<<name<<"\n";
}
} // namespace
int main(int argc,char**argv){
 try{
  std::string shot;int maxFrames=0;int initialWeather=0;bool autoPilot=false,forceGarage=false,forceMenu=false,benchmark=false;int initialCircuit=0,initialLaps=3,initialRivals=2;int initialQuality=2, initialWidth=1280, initialHeight=720;
  for(int i=1;i<argc;i++){std::string a=argv[i];if(a=="--screenshot"&&i+1<argc)shot=argv[++i];else if(a=="--frames"&&i+1<argc)maxFrames=std::stoi(argv[++i]);else if(a=="--autopilot")autoPilot=true;else if(a=="--garage")forceGarage=true;else if(a=="--menu")forceMenu=true;else if(a=="--benchmark")benchmark=true;else if(a=="--width"&&i+1<argc)initialWidth=std::clamp(std::stoi(argv[++i]),640,3840);else if(a=="--height"&&i+1<argc)initialHeight=std::clamp(std::stoi(argv[++i]),480,2160);else if(a=="--laps"&&i+1<argc)initialLaps=std::clamp(std::stoi(argv[++i]),0,99);else if(a=="--circuit"&&i+1<argc)initialCircuit=std::clamp(std::stoi(argv[++i]),0,2);else if(a=="--rivals"&&i+1<argc)initialRivals=std::clamp(std::stoi(argv[++i]),0,2);else if(a=="--quality"&&i+1<argc)initialQuality=std::max(0,std::min(3,std::stoi(argv[++i])));else if(a=="--weather"&&i+1<argc){std::string v=argv[++i];initialWeather=v=="rain"?1:v=="snow"?2:v=="storm"?3:v=="fog"?4:0;}}
  if(benchmark&&maxFrames==0)maxFrames=300;
  if(SDL_Init(0x00000020 | 0x00004000)!=0)throw std::runtime_error(SDL_GetError());
  SDL_GL_SetAttribute(17,3);SDL_GL_SetAttribute(18,3);SDL_GL_SetAttribute(21,1);SDL_GL_SetAttribute(5,1);SDL_GL_SetAttribute(6,24);
  SDL_Window* window=SDL_CreateWindow("COMETA PHOENIX | REGENERATIVE RACING",0x2FFF0000,0x2FFF0000,initialWidth,initialHeight,0x00000002|0x00000004|0x00000020);
  if(!window)throw std::runtime_error(SDL_GetError());
  void*ctx=SDL_GL_CreateContext(window);if(!ctx)throw std::runtime_error(SDL_GetError());
  SDL_GL_SetSwapInterval(benchmark?0:1);
  GLApi gl;gl.load();
  if(benchmark){int dw=0,dh=0;SDL_GL_GetDrawableSize(window,&dw,&dh);
    std::cout<<"OPENGL vendor="<<gl.GetString(0x1F00)<<" renderer="<<gl.GetString(0x1F01)<<" version="<<gl.GetString(0x1F02)<<" drawable="<<dw<<"x"<<dh<<"\n";}
  std::string dir=COMETA_GAME_SHADER_DIR;
  // Pasta instalada tem prioridade em distribuicoes portateis Windows/Linux.
  const auto exe=std::filesystem::absolute(argv[0]).parent_path();
  for(const auto &candidate:{exe/"game/shaders",exe.parent_path()/"game/shaders",std::filesystem::current_path()/"game/shaders",std::filesystem::path(COMETA_GAME_SHADER_DIR)}){
     if(std::filesystem::exists(candidate/"racing.frag")){dir=candidate.string();break;}
  }
  GLuint race=program(gl,FULLSCREEN_VERTEX,readText(dir+"/racing.frag"));
  GLuint garage=program(gl,FULLSCREEN_VERTEX,readText(dir+"/garage.frag"));
  GLuint uiProg=program(gl,UI_VERTEX,UI_FRAGMENT);
  GLuint fontProg=program(gl,FONT_VERTEX,FONT_FRAGMENT);
  GLuint menuProg=program(gl,FULLSCREEN_VERTEX,MENU_FRAGMENT);
  GLuint meshProg=program(gl,readText(dir+"/kart.vert"),readText(dir+"/kart.frag"));
  GLuint upProg=program(gl,FULLSCREEN_VERTEX,UPSCALE_FRAGMENT);
  // A busca e sempre relativa ao executavel ou ao projeto para rodar de qualquer pasta.
  std::filesystem::path assetDir=std::filesystem::path(dir).parent_path()/"assets";
  if(!std::filesystem::exists(assetDir/"phoenix_ui_atlas.r8"))
      throw std::runtime_error("Assets do jogo ausentes: "+assetDir.string());
  auto fontBytes=readBinary(assetDir/"phoenix_ui_atlas.r8",768*360);
  GLuint fontTexture=loadTex(gl,768,360,RED,fontBytes.data());
  auto menuBytes=readBinary(assetDir/"phoenix_cinematic_menu.rgb",1280*720*3);
  GLuint menuTexture=loadTex(gl,1280,720,RGB,menuBytes.data());
  auto sprite=readBinary(assetDir/"kart_sprite.rgba",512*512*4);
  GLuint spriteTexture=loadTex(gl,512,512,RGBA,sprite.data());
  const auto asphaltPixels=readBinary(assetDir/"asphalt_photo.rgba",512*512*4);
  const auto asphaltNormals=readBinary(assetDir/"asphalt_normal.rgb",512*512*3);
  GLuint asphaltPhoto=loadTex(gl,512,512,RGBA,asphaltPixels.data(),true);
  GLuint asphaltNormal=loadTex(gl,512,512,RGB,asphaltNormals.data(),true);
  const auto concretePixels=readBinary(assetDir/"concrete_photo.rgba",512*512*4);
  const auto concreteNormals=readBinary(assetDir/"concrete_normal.rgb",512*512*3);
  GLuint concretePhoto=loadTex(gl,512,512,RGBA,concretePixels.data(),true);
  GLuint concreteNormal=loadTex(gl,512,512,RGB,concreteNormals.data(),true);
  KartMesh mesh(gl,assetDir/"kart.mesh");
  EngineAudio engineAudio;
  if(!benchmark){engineAudio.start();std::cout<<"AUDIO_DEVICE "<<(engineAudio.available()?"OPEN":"UNAVAILABLE")<<"\n";
   if(!engineAudio.available())std::cerr<<"Audio indisponivel: "<<SDL_GetError()<<"\n";}
  RenderTarget scaledScene;
  GLuint quadVAO=0,uiVAO=0,uiVBO=0,fontVAO=0,fontVBO=0;
  gl.GenVertexArrays(1,&quadVAO);gl.GenVertexArrays(1,&uiVAO);gl.GenBuffers(1,&uiVBO);
  gl.GenVertexArrays(1,&fontVAO);gl.GenBuffers(1,&fontVBO);
  gl.BindVertexArray(uiVAO);gl.BindBuffer(ARRAY,uiVBO);gl.EnableVertexAttribArray(0);gl.VertexAttribPointer(0,2,FLOAT,0,6*sizeof(float),nullptr);gl.EnableVertexAttribArray(1);gl.VertexAttribPointer(1,4,FLOAT,0,6*sizeof(float),reinterpret_cast<const void*>(2*sizeof(float)));
  gl.BindVertexArray(fontVAO);gl.BindBuffer(ARRAY,fontVBO);gl.EnableVertexAttribArray(0);gl.VertexAttribPointer(0,2,FLOAT,0,8*sizeof(float),nullptr);gl.EnableVertexAttribArray(1);gl.VertexAttribPointer(1,2,FLOAT,0,8*sizeof(float),reinterpret_cast<const void*>(2*sizeof(float)));gl.EnableVertexAttribArray(2);gl.VertexAttribPointer(2,4,FLOAT,0,8*sizeof(float),reinterpret_cast<const void*>(4*sizeof(float)));
  phoenix::Game game(42);game.setCircuit((phoenix::Circuit)initialCircuit);game.setTargetLaps(initialLaps);game.setWeather((phoenix::Weather)initialWeather);game.setAutopilot(autoPilot);
  std::filesystem::path recordDir;
  if(!benchmark){
    if(char* pref=SDL_GetPrefPath("Cometa","Phoenix")){recordDir=pref;SDL_free(pref);}
  }
  auto recordFile=[&](){return recordDir/("record_c"+std::to_string((int)game.circuit())+"_w"+std::to_string((int)game.weather())+".phx");};
  double recordedBest=0.;
  auto reloadRecord=[&](){
    if(!recordDir.empty())game.loadRecord(recordFile());
    recordedBest=game.bestLapTime();
  };
  reloadRecord();
  std::array<phoenix::RivalAI,2> rivals{};
  int rivalCount=initialRivals;
  auto resetRivals=[&](){
    for(int i=0;i<2;++i)rivals[i].reset(game.circuit(),game.weather(),i==0?1.0:.88,-.09*(i+1));
  };
  resetRivals();
  if(forceGarage){game.requestDamage("carenagem",12);for(int i=0;i<150;i++)game.update({},.05);}
  std::size_t knownDamageEvents=game.simulator().danos().tamanho();
  auto knownPhase=game.simulator().fase();
  bool knownFinished=game.finished();
  bool running=true,paused=false,menuOpen=forceMenu||(maxFrames==0&&!forceGarage),fullscreen=false,mapOpen=false,muted=false,ghostVisible=true;
  int quality=initialQuality,frame=0; UI ui;ui.verts.reserve(30000);ui.letterVerts.reserve(14000);int measuredRace=0,measuredGarage=0,measuredMenu=0;std::vector<double> frameMs;frameMs.reserve(maxFrames>0?maxFrames:0);double fps=60.,fpsWindow=0.;int fpsFrames=0;
  float orbit=0,smx=(float)game.vehicle().x,smy=(float)game.vehicle().y;
  std::uint32_t ticks=SDL_GetTicks();double accumulator=0;
  const auto benchStart=std::chrono::steady_clock::now(); const int warmup=benchmark?std::min(30,maxFrames/4):0;
  while(running){
   auto frameStart=std::chrono::steady_clock::now();
   alignas(16) unsigned char ev[128]{};
   while(SDL_PollEvent(ev)){
    std::uint32_t type{};std::memcpy(&type,ev,sizeof(type));
    if(type==0x100)running=false;
    if(type==0x300&&ev[13]==0){int k=0;std::memcpy(&k,ev+20,sizeof(k));
     if(k==27){if(menuOpen)running=false;else {menuOpen=true;paused=false;}}
     if(k==13||k==1073741912){menuOpen=false;}
     if(k==1073741892){fullscreen=!fullscreen;SDL_SetWindowFullscreen(window,fullscreen?0x00001001u:0u);}
     if(k=='q')quality=(quality+1)%4;
     if(k=='l' && menuOpen){const int n=game.targetLaps();game.setTargetLaps(n==0?1:n==1?3:n==3?5:0);reloadRecord();resetRivals();smx=(float)game.vehicle().x;smy=(float)game.vehicle().y;}
     if(k=='k' && menuOpen){rivalCount=(rivalCount+1)%3;resetRivals();}
     if(k=='t' && menuOpen){game.setCircuit(static_cast<phoenix::Circuit>((static_cast<int>(game.circuit())+1)%3));reloadRecord();resetRivals();smx=(float)game.vehicle().x;smy=(float)game.vehicle().y;}
     if(!menuOpen){
       if(k=='m')mapOpen=!mapOpen;
       if(k=='g')ghostVisible=!ghostVisible;
       if(k=='v')muted=!muted;
       if(k=='p')paused=!paused;
       if(k=='r'){game.restart();reloadRecord();resetRivals();smx=(float)game.vehicle().x;smy=(float)game.vehicle().y;}
       if(k=='c')game.setAutopilot(!game.autopilot());
       if(k=='x')game.requestDamage("carenagem",10.);
       if(k=='z')game.requestDamage("chassi",18.);
       if(k>='1'&&k<='5'){game.setWeather((phoenix::Weather)(k-'1'));reloadRecord();for(auto& bot:rivals)bot.setWeather(game.weather());}
     }
    }
   }
   const auto*kb=SDL_GetKeyboardState(nullptr);
   phoenix::Input in{};in.throttle=kb[26]||kb[82];in.brake=kb[22]||kb[81];in.left=kb[4]||kb[80];in.right=kb[7]||kb[79];
   std::uint32_t now=SDL_GetTicks();double dt=std::min(.075,(now-ticks)*.001);ticks=now;
   if(maxFrames>0)dt=1.0/60.0;
   if(!paused && !menuOpen){accumulator+=dt;int steps=0;while(accumulator>=1./120.&&steps<12){game.update(in,1./120.);if(!game.finished())for(int i=0;i<rivalCount;++i)rivals[i].update(1./120.,game.targetLaps());accumulator-=1./120.;++steps;}if(steps==12)accumulator=0;if(game.garage()&&!benchmark){float direction=(in.right?1.f:0.f)-(in.left?1.f:0.f);orbit+=(float)dt*(direction!=0?direction*1.45f:.22f);}else orbit+=(float)dt*.27f;}
   if(!benchmark && !recordDir.empty() && game.bestLapTime()>0. &&
       (recordedBest==0. || game.bestLapTime()<recordedBest-.000001)){
     const bool saved=game.saveRecord(recordFile());
     recordedBest=game.bestLapTime();
     if(!saved)std::cerr<<"Aviso: falha ao salvar recorde em "<<recordFile().string()<<"\n";
   }
   smx+=(float)(game.vehicle().x-smx)*std::min(1.0,dt*3.4);smy+=(float)(game.vehicle().y-smy)*std::min(1.0,dt*3.4);
   int w=0,h=0;SDL_GL_GetDrawableSize(window,&w,&h);w=std::max(320,w);h=std::max(240,h);
   // Qualidade: renderizacao a .65/.82/1.0/1.0 da resolucao nativa;
   // UI permanece sempre a resolucao total e nao perde nitidez.
   const float scale=quality==0?.65f:quality==1?.82f:1.f;
   const bool lowRes=scale<.999f && !menuOpen;
   const int rw=lowRes?std::max(240,int(w*scale)):w,rh=lowRes?std::max(180,int(h*scale)):h;
   if(lowRes){scaledScene.resize(gl,rw,rh);gl.BindFramebuffer(FRAMEBUFFER,scaledScene.framebuffer);}
   else gl.BindFramebuffer(FRAMEBUFFER,0);
   gl.Viewport(0,0,rw,rh);gl.ClearColor(.01f,.02f,.03f,1.f);gl.Clear(COLOR);
   GLuint p=0;bool shop=game.garage();
   if(menuOpen){p=menuProg;gl.UseProgram(p);gl.ActiveTexture(TEXTURE0);gl.BindTexture(TEXTURE_2D,menuTexture);gl.Uniform1i(gl.GetUniformLocation(p,"imageTex"),0);setf(gl,p,"u_time",(float)game.worldTime());}
   else {
     p=shop?garage:race;gl.UseProgram(p);
     set2(gl,p,"u_resolution",(float)rw,(float)rh);setf(gl,p,"u_time",(float)game.worldTime());setf(gl,p,"u_quality",(float)quality);
     if(shop){const auto &rank=game.simulator().componenteMaisCritico();float part=partId(rank.id);float harm=(float)(1.-rank.integridade);
       setf(gl,p,"u_orbit",orbit);setf(gl,p,"u_repair",(float)game.repairFraction());setf(gl,p,"u_damage",harm);setf(gl,p,"u_part",part);
       setf(gl,p,"u_phase",game.simulator().fase()==cometa::Fase::Reparando?2.f:game.simulator().fase()==cometa::Fase::Diagnostico?1.f:3.f);
       gl.ActiveTexture(TEXTURE0+3);gl.BindTexture(TEXTURE_2D,concretePhoto);gl.Uniform1i(uniform(gl,p,"u_concretePhoto"),3);
       gl.ActiveTexture(TEXTURE0+4);gl.BindTexture(TEXTURE_2D,concreteNormal);gl.Uniform1i(uniform(gl,p,"u_concreteNormal"),4);
       gl.ActiveTexture(TEXTURE0);
     }else{
       const auto &ranking=game.simulator().componenteMaisCritico();set2(gl,p,"u_camera",mapOpen?0.f:smx,mapOpen?0.f:smy);set2(gl,p,"u_car",(float)game.vehicle().x,(float)game.vehicle().y);
       setf(gl,p,"u_heading",(float)game.vehicle().angle);setf(gl,p,"u_zoom",mapOpen?17.f:2.85f);
       const auto gh=game.ghostPose();
       setf(gl,p,"u_hasGhost",game.hasGhost()&&ghostVisible?1.f:0.f);
       setf(gl,p,"u_rivalCount",(float)rivalCount);
       for(int i=0;i<2;++i){
           const auto other=rivals[i].pose();
           set2(gl,p,i==0?"u_rival0":"u_rival1",(float)other.x,(float)other.y);
           setf(gl,p,i==0?"u_rivalAngle0":"u_rivalAngle1",(float)other.angle);
       }set2(gl,p,"u_ghostPos",(float)gh.x,(float)gh.y);setf(gl,p,"u_ghostHeading",(float)gh.angle);
       gl.ActiveTexture(TEXTURE0);gl.BindTexture(TEXTURE_2D,spriteTexture);gl.Uniform1i(uniform(gl,p,"u_kartSprite"),0);
       gl.ActiveTexture(TEXTURE0+1);gl.BindTexture(TEXTURE_2D,asphaltPhoto);gl.Uniform1i(uniform(gl,p,"u_asphaltPhoto"),1);
       gl.ActiveTexture(TEXTURE0+2);gl.BindTexture(TEXTURE_2D,asphaltNormal);gl.Uniform1i(uniform(gl,p,"u_asphaltNormal"),2);
       gl.ActiveTexture(TEXTURE0);
       setf(gl,p,"u_weather",(float)(int)game.weather());setf(gl,p,"u_speed",(float)game.vehicle().speed);
       setf(gl,p,"u_flash",0.f);setf(gl,p,"u_integrity",(float)ranking.integridade);
       const auto& id=ranking.id;
       float dx=0.f,dy=0.f;
       if(id=="carenagem")dx=.12f; else if(id=="motor")dx=-.14f;else if(id=="suspensao"||id=="freio")dx=.13f;
       else if(id.find("pneu_")==0){dx=id[5]=='d'?.13f:-.13f;dy=id[6]=='e'?.10f:-.10f;}
       set2(gl,p,"u_damagePoint",dx,dy);
       setf(gl,p,"u_cinematic",0.f);setf(gl,p,"u_box",0);setf(gl,p,"u_night",0);setf(gl,p,"u_circuit",(float)(int)game.circuit());
     }
   }
   gl.BindVertexArray(quadVAO);gl.DrawArrays(TRIANGLES,0,3);
   if(shop&&!menuOpen){const auto &part=game.simulator().componenteMaisCritico();mesh.draw(gl,meshProg,orbit,quality,rw,rh,(float)game.worldTime(),(float)(1.-part.integridade),partId(part.id),(float)game.repairFraction());}
   if(lowRes){
      gl.BindFramebuffer(FRAMEBUFFER,0);gl.Viewport(0,0,w,h);gl.UseProgram(upProg);
      gl.ActiveTexture(TEXTURE0);gl.BindTexture(TEXTURE_2D,scaledScene.texture);
      gl.Uniform1i(gl.GetUniformLocation(upProg,"sceneTex"),0);
      set2(gl,upProg,"sourceRes",(float)rw,(float)rh);
      setf(gl,upProg,"sharpen",quality==0?.14f:.23f);
      gl.DrawArrays(TRIANGLES,0,3);
   }
   ui.verts.clear();ui.letterVerts.clear();
   if(menuOpen)drawMenu(ui,w,h,quality,game.circuit(),game.targetLaps(),rivalCount); else drawHUD(ui,game,rivals,rivalCount,w,h,quality,paused,fps,ghostVisible);
   gl.Enable(BLEND);gl.BlendFunc(SRC_ALPHA,ONE_MINUS_SRC_ALPHA);
   gl.UseProgram(uiProg);set2(gl,uiProg,"res",(float)w,(float)h);
   gl.BindVertexArray(uiVAO);gl.BindBuffer(ARRAY,uiVBO);
   gl.BufferData(ARRAY,(GLsizeiptr)(ui.verts.size()*sizeof(float)),ui.verts.data(),DYNAMIC);
   gl.DrawArrays(TRIANGLES,0,(GLsizei)(ui.verts.size()/6));
   gl.UseProgram(fontProg);set2(gl,fontProg,"res",(float)w,(float)h);
   gl.ActiveTexture(TEXTURE0);gl.BindTexture(TEXTURE_2D,fontTexture);
   gl.Uniform1i(gl.GetUniformLocation(fontProg,"atlas"),0);
   gl.BindVertexArray(fontVAO);gl.BindBuffer(ARRAY,fontVBO);
   gl.BufferData(ARRAY,(GLsizeiptr)(ui.letterVerts.size()*sizeof(float)),ui.letterVerts.data(),DYNAMIC);
   gl.DrawArrays(TRIANGLES,0,(GLsizei)(ui.letterVerts.size()/8));gl.Disable(BLEND);
   if(!shot.empty()&&(frame==std::max(1,maxFrames)-1)){screenshot(gl,shot,w,h);running=false;}
   if(benchmark)gl.Finish();
   SDL_GL_SwapWindow(window);
   if(benchmark&&frame>=warmup){frameMs.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-frameStart).count());if(menuOpen)++measuredMenu;else if(shop)++measuredGarage;else ++measuredRace;}
   const auto damageEvents=game.simulator().danos().tamanho();
   if(damageEvents>knownDamageEvents && !benchmark && !muted && !menuOpen){
    const auto& affected=game.simulator().danos().itens().back().componente;
    const float strength=affected=="chassi"?.98f:affected=="suspensao"?.80f:affected.find("pneu_")==0?.65f:.46f;
    const auto& event=game.simulator().danos().itens().back();
    engineAudio.triggerEffect(classifyImpact(event.componente,event.causa),strength);
   }
   knownDamageEvents=damageEvents;
   auto currentPhase=game.simulator().fase();
   if(!benchmark&&!muted&&!menuOpen&&!paused&&currentPhase!=knownPhase&&currentPhase==cometa::Fase::Reparando)
      engineAudio.triggerEffect(ImpactKind::Repair,.8f);
   knownPhase=currentPhase;
   if(!benchmark&&!muted&&!menuOpen&&!paused&&game.finished()&&!knownFinished)
      engineAudio.triggerEffect(ImpactKind::Finish,.85f);
   knownFinished=game.finished();
   engineAudio.update(game.vehicle().speed,!menuOpen&&!paused&&!shop&&!muted&&!game.finished(),in.throttle||game.autopilot(),!menuOpen&&!paused&&!muted&&!benchmark);
   fpsWindow+=dt;fpsFrames++;if(fpsWindow>=.5){fps=fpsFrames/fpsWindow;fpsFrames=0;fpsWindow=0.;}
   frame++;if(maxFrames>0&&frame>=maxFrames)running=false;
   if(maxFrames==0)SDL_Delay(1);
  }
  if(benchmark){
    double wall=std::chrono::duration<double>(std::chrono::steady_clock::now()-benchStart).count();
    std::cout<<"BENCHMARK frames="<<frame<<" seconds="<<wall<<" avg_fps="<<(wall>0?frame/wall:0.)
             <<" ms_per_frame="<<(frame>0?wall*1000./frame:0.)<<" quality="<<quality<<"\n";std::cout<<"SCENE_COUNTS race="<<measuredRace<<" garage="<<measuredGarage<<" menu="<<measuredMenu<<"\n";
    std::cout<<"RIVALS active="<<rivalCount;for(int i=0;i<rivalCount;++i)std::cout<<" ai"<<i<<"_lap="<<rivals[i].laps()<<"_progress="<<rivals[i].totalProgress()/6.283185307179586;std::cout<<"\n";
  std::cout<<"GAMEPLAY laps="<<game.validCompletedLaps()<<" traversed="<<game.vehicle().lap<<" elapsed="<<game.worldTime()<<" damage_events="<<game.simulator().danos().tamanho()<<" finished="<<game.finished()<<"\n";
    if(!frameMs.empty()){double mean=std::accumulate(frameMs.begin(),frameMs.end(),0.)/frameMs.size();std::cout<<"MEASURED frames="<<frameMs.size()<<" warmup="<<warmup<<" fps="<<1000./mean<<" mean_ms="<<mean<<"\n";std::sort(frameMs.begin(),frameMs.end());auto pct=[&](double p){return frameMs[(std::size_t)((frameMs.size()-1)*p)];};std::cout<<"FRAME_MS p50="<<pct(.5)<<" p95="<<pct(.95)<<" p99="<<pct(.99)<<" max="<<frameMs.back()<<"\n";}
  }
  scaledScene.reset(gl);gl.DeleteTextures(1,&fontTexture);gl.DeleteTextures(1,&menuTexture);gl.DeleteTextures(1,&spriteTexture);gl.DeleteTextures(1,&asphaltPhoto);gl.DeleteTextures(1,&asphaltNormal);gl.DeleteTextures(1,&concretePhoto);gl.DeleteTextures(1,&concreteNormal);engineAudio.stop();mesh.destroy(gl);
  gl.DeleteBuffers(1,&uiVBO);gl.DeleteBuffers(1,&fontVBO);gl.DeleteVertexArrays(1,&quadVAO);gl.DeleteVertexArrays(1,&uiVAO);gl.DeleteVertexArrays(1,&fontVAO);
  gl.DeleteProgram(race);gl.DeleteProgram(garage);gl.DeleteProgram(uiProg);gl.DeleteProgram(fontProg);gl.DeleteProgram(menuProg);gl.DeleteProgram(meshProg);gl.DeleteProgram(upProg);
  SDL_GL_DeleteContext(ctx);SDL_DestroyWindow(window);SDL_Quit();
  return 0;
 }catch(const std::exception&e){std::cerr<<"COMETA PHOENIX ERROR: "<<e.what()<<'\n';return 1;}
}
