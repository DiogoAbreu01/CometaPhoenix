// Included by GameApp after the OpenGL API and uniform helpers.
// Asset format: PHXM, vertex/index counts (little endian uint32), 13 floats/vertex, uint32 indices.
#include "ImpactAudio.hpp"
struct Vec3 {float x,y,z;};
Vec3 operator-(Vec3 a,Vec3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec3 cross(Vec3 a,Vec3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
Vec3 unit(Vec3 a){float d=std::sqrt(a.x*a.x+a.y*a.y+a.z*a.z);return {a.x/d,a.y/d,a.z/d};}
void set3(GLApi &gl,GLuint p,const char *k,Vec3 v){auto u=uniform(gl,p,k);if(u>=0)gl.Uniform3f(u,v.x,v.y,v.z);}
class KartMesh {
 GLuint vao{},vbo{},ebo{};GLsizei count{};
 public:
 KartMesh(GLApi &gl,const std::filesystem::path &path){
  std::ifstream f(path,std::ios::binary);char magic[4]{};std::uint32_t nv=0,ni=0;
  f.read(magic,4);f.read(reinterpret_cast<char*>(&nv),4);f.read(reinterpret_cast<char*>(&ni),4);
  if(!f||std::memcmp(magic,"PHXM",4)||nv==0||nv>2000000||ni==0||ni>6000000||ni%3)throw std::runtime_error("Malha invalida");
  std::vector<float> v(std::size_t(nv)*13);std::vector<std::uint32_t> i(ni);
  f.read(reinterpret_cast<char*>(v.data()),v.size()*sizeof(float));f.read(reinterpret_cast<char*>(i.data()),i.size()*sizeof(std::uint32_t));
  if(!f||*std::max_element(i.begin(),i.end())>=nv||!std::all_of(v.begin(),v.end(),[](float n){return std::isfinite(n);}))throw std::runtime_error("Malha truncada ou indice invalido");
  gl.GenVertexArrays(1,&vao);gl.GenBuffers(1,&vbo);gl.GenBuffers(1,&ebo);gl.BindVertexArray(vao);
  gl.BindBuffer(ARRAY,vbo);gl.BufferData(ARRAY,v.size()*sizeof(float),v.data(),STATIC);
  const int sizes[]={3,3,3,2,2};const int offsets[]={0,3,6,9,11};
  for(int a=0;a<5;a++){gl.EnableVertexAttribArray(a);gl.VertexAttribPointer(a,sizes[a],FLOAT,0,13*sizeof(float),reinterpret_cast<void*>(std::size_t(offsets[a])*sizeof(float)));}
  gl.BindBuffer(ELEMENT,ebo);gl.BufferData(ELEMENT,i.size()*sizeof(std::uint32_t),i.data(),STATIC);count=(GLsizei)ni;gl.BindVertexArray(0);
 }
 void draw(GLApi&gl,GLuint p,float orbit,int quality,int w,int h,float t,float damage,float part,float repair){
  const float a=.45f+orbit;Vec3 cam{3.4f*std::cos(a),3.4f*std::sin(a),2.3f};auto forward=unit(Vec3{0,0,.46f}-cam),right=unit(cross(forward,{0,0,1})),up=unit(cross(right,forward));
  gl.Enable(DEPTH);gl.Clear(DEPTH_BIT);gl.UseProgram(p);set3(gl,p,"u_camera",cam);set3(gl,p,"u_forward",forward);set3(gl,p,"u_right",right);set3(gl,p,"u_up",up);
  setf(gl,p,"u_aspect",float(w)/h);setf(gl,p,"u_time",t);setf(gl,p,"u_damage",damage);setf(gl,p,"u_repair",repair);setf(gl,p,"u_part",part);setf(gl,p,"u_quality",float(quality));
  gl.BindVertexArray(vao);gl.DrawElements(TRIANGLES,count,UINT,nullptr);gl.Disable(DEPTH);
 }
 void destroy(GLApi&gl){gl.DeleteBuffers(1,&vbo);gl.DeleteBuffers(1,&ebo);gl.DeleteVertexArrays(1,&vao);}
};
// Original engine synthesis using harmonic wavetables.
class EngineAudio {
 friend struct EngineAudioTest;
 static constexpr int sampleRate=48000, tableSize=2048;
 static constexpr std::uint32_t rpmMask=0xffff, gearMask=7, playingBit=1u<<19, throttleBit=1u<<20, effectsBit=1u<<21;
 static constexpr unsigned gearShift=16;
 static constexpr float idleRpm=1200.f;
 static_assert(std::atomic<std::uint32_t>::is_always_lock_free, "Audio controls must be lock-free");

 std::uint32_t device{};
 unsigned gear{};
 std::atomic<std::uint32_t> controls{1200};
 std::array<float,tableSize> coastTable{},loadedTable{};
 float rpm=idleRpm,gain=0,load=0,phase=0,previousInput=0,previousOutput=0;
 unsigned renderedGear{},shiftCutFrames{};
 ImpactAudio effects;
 bool wasPlaying{};

 void prepare(){
  constexpr float coast[]={.36f,.48f,.08f,.06f};
  constexpr float loaded[]={.26f,.55f,.11f,.21f,.07f,.11f,.04f,.07f};
  for(int i=0;i<tableSize;++i){
   const double angle=6.283185307179586*i/tableSize;
   coastTable[i]=loadedTable[i]=0.f;
   for(int h=0;h<4;++h) coastTable[i]+=coast[h]*float(std::sin(angle*(h+1)));
   for(int h=0;h<8;++h) loadedTable[i]+=loaded[h]*float(std::sin(angle*(h+1)));
  }
  rpm=idleRpm;
  gain=load=phase=previousInput=previousOutput=0.f;
  renderedGear=shiftCutFrames=0;
  wasPlaying=false;
  effects.reset();
 }

 static void callback(void *context,std::uint8_t *stream,int bytes){
  auto &self=*static_cast<EngineAudio*>(context);
  const auto target=self.controls.load(std::memory_order_relaxed);
  const bool playing=(target&playingBit)!=0;
  const bool throttle=(target&throttleBit)!=0;
  const unsigned newGear=(target>>gearShift)&gearMask;
  if(playing&&self.wasPlaying&&newGear>self.renderedGear&&throttle)
   self.shiftCutFrames=sampleRate*55/1000;
  if(!playing||!throttle) self.shiftCutFrames=0;
  self.wasPlaying=playing;
  self.renderedGear=newGear;

  self.effects.beginBlock();
  const int frames=bytes/int(sizeof(float));
  for(int i=0;i<frames;++i){
   const bool cutting=self.shiftCutFrames!=0;
   if(cutting) --self.shiftCutFrames;
   const float targetRpm=float(target&rpmMask);
   const float targetLoad=playing&&throttle&&!cutting?1.f:0.f;
   const float targetGain=!playing?0.f:cutting?.085f:throttle?.32f:.13f;
   self.rpm+=(targetRpm-self.rpm)*(cutting?.0015f:.0008f);
   self.load+=(targetLoad-self.load)*.0014f;
   self.gain+=(targetGain-self.gain)*.002f;
   if(targetLoad==0.f&&self.load<1e-8f) self.load=0.f;
   if(targetGain==0.f&&self.gain<1e-8f) self.gain=0.f;

   // The second harmonic carries the main firing tone. Keep phase continuous across shifts.
   self.phase+=self.rpm*(tableSize/(20.f*sampleRate));
   if(self.phase>=tableSize) self.phase-=tableSize;
   const int index=int(self.phase);
   const int next=(index+1)&(tableSize-1);
   const float fraction=self.phase-index;
   const float coast=self.coastTable[index]+fraction*(self.coastTable[next]-self.coastTable[index]);
   const float loaded=self.loadedTable[index]+fraction*(self.loadedTable[next]-self.loadedTable[index]);
   const float input=(coast+self.load*(loaded-coast))*self.gain;
   // A DC blocker also removes low-frequency bias during pitch and gain transitions.
   const float effect=self.effects.nextSample();
   const float combined=input+((target&effectsBit)?effect:0.f);
   const float output=combined-self.previousInput+.998f*self.previousOutput;
   self.previousInput=combined;
   self.previousOutput=std::abs(output)<1e-12f?0.f:output;
   const float clipped=std::clamp(output,-1.f,1.f);
   std::memcpy(stream+i*sizeof(float),&clipped,sizeof(float));
  }
  const int trailing=bytes%int(sizeof(float));
  if(trailing>0) std::memset(stream+frames*sizeof(float),0,trailing);
 }

 public:
 void start(){
  if(device||SDL_InitSubSystem(0x10)!=0) return;
  prepare();
  SDL_AudioSpec desired{};
  const std::uint16_t endianProbe=1;
  desired.format=*reinterpret_cast<const unsigned char*>(&endianProbe)?0x8120:0x9120;
  desired.freq=sampleRate;
  desired.channels=1;
  desired.samples=512;
  desired.callback=callback;
  desired.userdata=this;
  device=SDL_OpenAudioDevice(nullptr,0,&desired,nullptr,0);
  if(device) SDL_PauseAudioDevice(device,0);
 }

 void update(double speed,bool playing,bool throttle,bool effectsEnabled=true){
  constexpr double upshift[]={22.,36.,51.,68.,85.};
  constexpr double rpmPerKmh[]={385.,225.,153.,116.,92.,77.};
  speed=std::isfinite(speed)?std::clamp(speed,0.,102.):0.;
  if(!playing) gear=0;
  else {
   while(gear<5&&speed>=upshift[gear]) ++gear;
   while(gear>0&&speed<upshift[gear-1]-3.) --gear;
  }
  const double minimum=throttle?2150.:idleRpm;
  const auto targetRpm=std::uint32_t(playing?std::clamp(speed*rpmPerKmh[gear],minimum,8500.):idleRpm);
  controls.store(targetRpm|(gear<<gearShift)|(playing?playingBit:0u)|(throttle?throttleBit:0u)|(effectsEnabled?effectsBit:0u),
                 std::memory_order_relaxed);
 }

 bool available() const noexcept {return device!=0;}
 void triggerEffect(ImpactKind kind,float intensity) noexcept {effects.trigger(kind,intensity);}
 void triggerImpact(float intensity) noexcept {effects.trigger(ImpactKind::Chassis,intensity);}
 void stop(){
  if(!device) return;
  update(0.,false,false);
  // Allow the SDL callback to finish the release before closing the device.
  SDL_Delay(100);
  SDL_CloseAudioDevice(device);
  device=0;
 }
 ~EngineAudio(){stop();}
};
