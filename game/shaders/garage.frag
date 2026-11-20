#version 330 core
out vec4 FragColor;
uniform vec2 u_resolution;
uniform float u_time,u_orbit,u_damage,u_repair,u_phase,u_quality;
uniform sampler2D u_concretePhoto,u_concreteNormal;
float hash21(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float grid(float v,float w){return 1.-smoothstep(0.,w,abs(fract(v)-.5));}
void main(){
 vec2 uv=(gl_FragCoord.xy-u_resolution*.5)/u_resolution.y;
 float a=.45+u_orbit;
 vec3 ro=vec3(3.4*cos(a),3.4*sin(a),2.3);
 vec3 target=vec3(0.,0.,.46),fwd=normalize(target-ro);
 vec3 right=normalize(cross(fwd,vec3(0,0,1))),up=normalize(cross(right,fwd));
 vec3 rd=normalize(1.275*fwd+uv.x*right+uv.y*up);
 // shadowy hi-tech hangar, soft indirect sky + illuminated industrial panels
 vec3 col=mix(vec3(.030,.025,.048),vec3(.070,.047,.077),clamp(.5+rd.z*.50,0.,1.));
 float wallGrid=max(grid(uv.x*13.+.18,.018),grid(uv.y*9.,.015));
 col+=vec3(.071,.028,.067)*wallGrid*.42;
 float skyline=1.-smoothstep(.09,.3,abs(uv.y-.28));
 col+=vec3(.12,.047,.09)*skyline*.17;
 // projected floor plane (Z=0) in same camera as real GLB model.
 if(rd.z<-.001){
  float t=-ro.z/rd.z;
  vec3 p=ro+rd*t;
  vec2 tile=fract(p.xy*2.15);
  float seam=1.-smoothstep(.010,.025,min(min(tile.x,1.-tile.x),min(tile.y,1.-tile.y)));
  float checker=mod(floor(p.x*2.15)+floor(p.y*2.15),2.);
  vec3 floorcol=mix(vec3(.046,.041,.059),vec3(.060,.052,.067),checker);
  float grain=hash21(floor(p.xy*120.))*.016;
  floorcol+=grain; floorcol-=seam*.020;
  // CC0 scanned concrete: seamless repeat, mipmapped with explicit screen gradients.
  float concreteRough=.78;
  if(u_quality>1.0){
    vec2 materialUV=p.xy*1.55;
    vec2 gradX=dFdx(materialUV),gradY=dFdy(materialUV);
    vec4 scan=textureGrad(u_concretePhoto,materialUV,gradX,gradY);
    vec3 normalMap=textureGrad(u_concreteNormal,materialUV,gradX,gradY).xyz*2.-1.;
    float micro=clamp(.76+.24*dot(normalize(normalMap),normalize(vec3(.4,-.3,.82))),.53,1.1);
    vec3 concrete=pow(max(scan.rgb,vec3(.001)),vec3(2.2));
    floorcol=mix(floorcol,concrete*.29*micro+vec3(.015,.02,.022),.70);
    concreteRough=clamp(scan.a,.2,1.);
  }
  float radius=length(p.xy);
  float platform=1.-smoothstep(1.36,1.45,radius);
  floorcol=mix(floorcol,vec3(.095,.061,.087),platform*.85);
  float ring=1.-smoothstep(.015,.047,abs(radius-1.41));
  floorcol+=vec3(.68,.16,.36)*ring;
  float ring2=1.-smoothstep(.018,.038,abs(radius-1.64));
  floorcol+=vec3(.31,.084,.19)*ring2*.35;
  // Contact shadow under exposed wheels and chassis.
  float shadow=1.-smoothstep(.45,1.40,length(p.xy*vec2(.9,1.05)));
  vec2 wheel=vec2(p.x>0.?p.x-.70:p.x+.80,abs(p.y)-.61);
  float contact=1.-smoothstep(.20,1.12,length(wheel/vec2(.26,.20)));
  float chassis=1.-smoothstep(.42,1.15,length(p.xy/vec2(.85,.48)));
  floorcol*=max(.13,(1.-shadow*.40)*(1.-contact*.81)*(1.-chassis*.45));
  // Reflective shop floor, inexpensive non-RT wet reflection streaks.
  float streak=pow(max(0.,sin(p.y*20.+sin(p.x*7.)*.6)),14.);
  floorcol+=vec3(.15,.060,.11)*streak*(u_quality>1.5?(.11+.24*(1.-concreteRough)):.09)*exp(-radius*.15);
  float scanner=1.-smoothstep(.02,.09,abs(p.x-sin(u_time*.55)*.7));
  floorcol+=vec3(.20,.06,.16)*scanner*.19*step(1.5,u_phase);
  float haze=1.-exp(-t*.065);
  col=mix(floorcol,col,haze);
 }
 // Soft lighting and sci-fi volumetric beams, deliberately no blocky fake car.
 float key=pow(max(0.,1.-length(uv-vec2(.50,.38))),3.);
 col+=vec3(.085,.047,.12)*key;
 float neonEdge=pow(max(0.,1.-abs(uv.x*.85-uv.y*.28-.78)),8.);
 col+=vec3(.091,.043,.09)*neonEdge;
 float corners=length(uv*vec2(1.10,.85));col*=1.-.33*smoothstep(.23,.82,corners);
 if(u_phase>1.5&&u_phase<3.5){
  float pulse=.5+.5*sin(u_time*2.8);
  float hologram=pow(max(0.,1.-abs(uv.y+.35+sin(u_time*.27)*.04)*50.),2.);
  col+=vec3(.20,.067,.15)*hologram*(.18+.28*pulse);
 }
 col=col/(col+.40);col=pow(max(col,0.),vec3(.83));
 FragColor=vec4(col,1.);
}
