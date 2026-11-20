// Included by GameApp after the OpenGL API and uniform helpers.
// Asset format: PHXM, vertex/index counts (little endian uint32), 13 floats/vertex, uint32 indices.
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
// Original V12-like wavetable synthesis; no external samples. Audio callback allocates nothing.
class EngineAudio {
 std::uint32_t device{};std::array<float,4096> table{};std::atomic<float> wantedRpm{1000},wantedGain{0};float rpm=1000,gain=0,phase=0;int rate=48000;
 static void callback(void *context,std::uint8_t *stream,int bytes){auto &self=*static_cast<EngineAudio*>(context);auto out=reinterpret_cast<float*>(stream);int n=bytes/int(sizeof(float));
  float rr=self.wantedRpm.load(std::memory_order_relaxed),gg=self.wantedGain.load(std::memory_order_relaxed);
  for(int i=0;i<n;i++) {self.rpm+=(rr-self.rpm)*.0007f;self.gain+=(gg-self.gain)*.0012f;self.phase+=self.rpm*.1f*4096.f/self.rate;if(self.phase>=4096)self.phase-=4096;int k=int(self.phase);float f=self.phase-k;out[i]=(self.table[k]*(1-f)+self.table[(k+1)&4095]*f)*self.gain;}
 }
 public:
 void start(){for(int i=0;i<4096;i++){double a=6.283185307179586*i/4096;double v=0;for(int h=1;h<=12;h++)v+=std::sin(a*h)/(std::pow(h,.87))* .12;table[i]=float(std::tanh(v*2));}
  if(SDL_InitSubSystem(0x10)!=0) return;
  SDL_AudioSpec desired{},actual{};desired.freq=48000;desired.format=0x8120;desired.channels=1;desired.samples=512;desired.callback=callback;desired.userdata=this;device=SDL_OpenAudioDevice(nullptr,0,&desired,&actual,0);if(device){rate=actual.freq;SDL_PauseAudioDevice(device,0);}}
 void update(double speed,bool playing,bool throttle){int gear=std::min(5,int(speed/20));float rpmTarget=1200.f+float(speed-gear*20)*340.f;wantedRpm.store(playing?rpmTarget:1000.f,std::memory_order_relaxed);wantedGain.store(playing?(throttle?.28f:.12f):0.f,std::memory_order_relaxed);}
 void stop(){if(device){SDL_CloseAudioDevice(device);device=0;}}
 ~EngineAudio(){stop();}
};
