#version 330 core
out vec4 FragColor;
uniform vec2 u_resolution;
uniform float u_time;
uniform vec2 u_camera;
uniform vec2 u_car;
uniform float u_heading;
uniform float u_zoom;
uniform float u_weather;
uniform float u_speed;
uniform float u_flash;
uniform float u_integrity;
uniform vec2 u_damagePoint;
uniform float u_quality;
uniform float u_cinematic;
uniform float u_box;
uniform float u_night;
uniform float u_circuit;
uniform sampler2D u_kartSprite;
#define PI 3.14159265359
float hash21(vec2 p){p=fract(p*vec2(123.34,456.21)); p+=dot(p,p+45.32);return fract(p.x*p.y);}
float hash11(float p){return fract(sin(p*127.1)*43758.5453123);}
float noise(vec2 p){vec2 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);return mix(mix(hash21(i),hash21(i+vec2(1,0)),f.x),mix(hash21(i+vec2(0,1)),hash21(i+vec2(1,1)),f.x),f.y);}
// Copa de arvores e detalhes de floresta gerados na GPU apenas em qualidade alta.
// Cada fragmento usa no maximo 4 celulas; nao ha objetos CPU por arvore.
vec3 forestDetails(vec2 p,vec3 base,float distanceToTrack){
 if(u_quality<1.5)return base;
 float woods=smoothstep(.51,.68,abs(distanceToTrack));
 if(woods<=.02)return base;
 vec2 pos=p*3.05,cell=floor(pos);vec3 outC=base;
 for(int ox=0;ox<2;ox++)for(int oy=0;oy<2;oy++){
   vec2 id=cell+vec2(float(ox)-.5,float(oy)-.5);
   vec2 randomPos=vec2(hash21(id+17.1),hash21(id+68.3));
   vec2 q=p-(id+randomPos)/3.05;
   float dist=length(q),h=hash21(id+5.6);
   float crown=.12+.06*h;
   float shape=sin(atan(q.y,q.x)*9.+h*6.)*.007+sin(atan(q.y,q.x)*15.)*.004;
   float shadow=(1.-smoothstep(crown*.74,crown*1.45,length(q+vec2(.028,-.035))))*.29;
   outC*=1.-shadow*woods;
   float tree=(1.-smoothstep(crown-.013,crown+.017,dist+shape))*woods*step(.19,h);
   if(u_circuit>1.5 && distanceToTrack>0.) continue;
   vec3 leaves=mix(vec3(.012,.055,.054),vec3(.075,.17,.098),hash21(id+33.));
   leaves*=.73+.31*smoothstep(-.5,.55,normalize(q+vec2(.002)).x*.67+normalize(q+vec2(.002)).y*.33);
   leaves+=noise(q*128.)*.028;
   outC=mix(outC,leaves,tree);
 }
 return outC;
}
float sdBox(vec2 p,vec2 b){vec2 q=abs(p)-b;return length(max(q,0.))+min(max(q.x,q.y),0.);}
mat2 rot(float a){return mat2(cos(a),sin(a),-sin(a),cos(a));}
float radius(float a){vec4 s=u_circuit<.5?vec4(4.65,.62,.34,.22):u_circuit<1.5?vec4(5.35,.94,.22,.12):vec4(5.85,.32,.58,.08);return s.x+s.y*cos(3.*a+.3)+s.z*sin(5.*a-.6)+s.w*cos(7.*a+1.2);}
float track(vec2 p){return length(p)-radius(atan(p.y,p.x));}
vec3 baseCol(vec2 p,float d){
  vec2 g=p*7.;float n=noise(g*3.4),fine=noise(g*15.);
  vec3 grass=mix(vec3(.027,.083,.078),vec3(.042,.15,.115),n);
  grass+=.022*sin(p.x*27.+n*8.)*sin(p.y*25.);
  float forest= smoothstep(5.6,7.2,length(p));
  grass=mix(grass,vec3(.014,.051,.059),forest*.7);
  if(u_circuit>1.5){grass=mix(vec3(.22,.17,.10),vec3(.39,.33,.19),n);float coast=smoothstep(.8,1.8,d);grass=mix(grass,vec3(.018,.16,.21)+vec3(.035,.08,.09)*noise(p*12.),coast);}else if(u_circuit>.5){grass=mix(vec3(.08,.10,.11),vec3(.24,.26,.25),n);grass=mix(grass,vec3(.66,.72,.76),smoothstep(.6,1.3,abs(d))*.75);}
  float roadNoise=noise(p*95.)*.035 + noise(p*5.)*.031;
  vec3 asphalt=vec3(.053,.069,.086)+roadNoise;
  asphalt=mix(asphalt,vec3(.09,.115,.13),.16+max(0.,sin(p.x*12.))*0.1);
  float aa=.010;
  vec3 col=mix(grass,asphalt,1.-smoothstep(.43,.445,abs(d)));
  float stripe=1.-smoothstep(.012,.026,abs(abs(d)-.336));
  col=mix(col,vec3(.78,.82,.71),stripe*.6);
  // Linhas alternadas vermelhas e brancas nas zebras
  float k=mod(floor(atan(p.y,p.x)*47.+p.x*9.),2.);
  float curb=(1.-smoothstep(.008,.019,abs(abs(d)-.442)));
  col=mix(col,mix(vec3(.97,.96,.87),vec3(.86,.14,.16),k),curb*.95);
  float mid=(1.-smoothstep(.01,.022,abs(d)));
  float dash=step(.55,fract(atan(p.y,p.x)*13.));
  col=mix(col,vec3(.88,.76,.40),mid*dash*.67);
  // Pista molhada: brilho especular e reflexos alongados.
  float wet=step(.5,u_weather)*(.45+ .25*noise(p*11.));
  float puddle=smoothstep(.64,.76,noise(p*9. +vec2(.2,2.)))*wet;
  col=mix(col,vec3(.035,.083,.126)+.12*pow(max(0.,sin(u_time*.7+p.x*7.)),8.),puddle*(1.-smoothstep(.30,.39,abs(d))));
  if(wet>.1 && u_quality>=2.){
    float reflection=pow(max(.0,sin((p.x+2.*p.y)*14.+noise(p*12.)*4.)),20.);
    float microglint=noise(p*85.+vec2(u_time*.025,0.));
    col+=vec3(.15,.19,.18)*(reflection*.31+pow(microglint,17.)*.09)*wet*(1.-smoothstep(.32,.44,abs(d)));
  }
  if(u_weather>1.5&&u_weather<2.5){
   float snow=noise(p*19.);float ss=smoothstep(.4,.67,snow)*.78;
   col=mix(col,vec3(.71,.81,.89),ss*(smoothstep(.31,.46,abs(d))));
   col=mix(col,vec3(.2,.25,.29),ss*.08);
  }
  // Faixa de partida / chegada no ponto leste.
  float grid=1.-smoothstep(.04,.07,abs(p.y));
  float start=(1.-smoothstep(.09,.11,abs(p.x-radius(0.))))*grid*(1.-smoothstep(.24,.34,abs(d)));
  float checks=mod(floor((p.x-radius(0.))/.032)+floor((p.y+.32)/.034),2.);
  col=mix(col,mix(vec3(.87),vec3(.085,.11,.13),checks),start*.95);
  // Box com faixa separada na parte leste.
  float pit=1.-smoothstep(.028,.04,abs(p.y-.0));
  col=mix(col,vec3(.12,.57,.55),pit*smoothstep(radius(0.)+.25,radius(0.)+.40,p.x)*(1.-smoothstep(radius(0.)+1.6,radius(0.)+1.75,p.x))*.8);
  vec2 boxp=p-vec2(radius(0.)+.76,.12);
  float pitPad=1.-smoothstep(.1,.13,sdBox(boxp,vec2(.24,.24)));
  col=mix(col,vec3(.1,.18,.21),pitPad);
  float hazard=step(.5,fract((boxp.x+boxp.y)*29.));
  float edge=smoothstep(.205,.235,max(abs(boxp.x),abs(boxp.y)))*(1.-smoothstep(.25,.27,max(abs(boxp.x),abs(boxp.y))));
  col=mix(col,mix(vec3(.96,.71,.13),vec3(.08,.09,.1),hazard),edge*.96);
  col=forestDetails(p,col,d);
  // Illuminated pit-strip and asphalt lane microstructure.
  float pitGlow=smoothstep(radius(0.)+.23,radius(0.)+.42,p.x)*(1.-smoothstep(radius(0.)+1.69,radius(0.)+1.91,p.x));
  float neon=1.-smoothstep(.006,.018,abs(p.y-.23));
  col+=vec3(.018,.23,.18)*neon*pitGlow*step(1.5,u_quality);

  // Elementos da pista longa: muretas, linha de escape, barreiras e refletores laterais.
  float angle=atan(p.y,p.x);
  float outer=abs(d);
  float barrier=1.-smoothstep(.017,.035,abs(outer-.58));
  float barrierStripe=step(.52,fract(angle*76.));
  col=mix(col,mix(vec3(.18,.27,.30),vec3(.80,.88,.86),barrierStripe),barrier*.78);
  float gravel=smoothstep(.46,.50,outer)*(1.-smoothstep(.75,.85,outer));
  float stones=pow(noise(p*64.),2.)*.055+noise(p*15.)*.023;
  col=mix(col,vec3(.13,.14,.145)+stones,gravel*.63);
  float beacon=1.-smoothstep(.008,.020,length(vec2(outer-.592,mod(angle*40.+.5,1.)-.5)));
  col+=vec3(.025,.72,.64)*beacon*step(1.5,u_quality)*.82;
  // Marcas de frenagem concentradas nas entradas das curvas.
  float braking=smoothstep(.38,.82,abs(sin(3.*angle+.3)))*smoothstep(.03,.14,abs(d));
  float skid=pow(max(0.,sin(angle*71.+d*14.)),30.);
  col*=1.-braking*skid*.08*(1.-smoothstep(.32,.43,outer));
  // Sombra direcional das muretas: reforco de profundidade visual.
  float castShadow=1.-smoothstep(.030,.10,abs(outer-.62));
  col*=1.-castShadow*.12;
  // Direcao de iluminacao difusa + relevo finissimo do asfalto.
  if(u_quality>1.5){
    float h0=noise(p*82.),hx=noise(p*82.+vec2(.019,0.)),hy=noise(p*82.+vec2(0.,.019));
    vec2 microNormal=vec2(hx-h0,hy-h0);
    float facing=clamp(.7+dot(microNormal,normalize(vec2(-.7,.6)))*.32,.52,1.25);
    float surface=1.-smoothstep(.39,.45,abs(d));
    col*=mix(1.,facing,surface*.7);
  }
  return col;
}
float wheel(vec2 q,vec2 c){return sdBox(q-c,vec2(.053,.041));}
// Kart em 2D: textura top-down derivada da malha 3D de 53 mil triangulos.
// Uma unica amostra de textura substitui multiplos SDF de quadrados.
vec3 kart(vec2 p,vec3 under,float d){
 vec2 delta=p-u_car;
 if(dot(delta,delta)>.24)return under;
 vec2 q=rot(-u_heading)*delta;
 float shadow=1.-smoothstep(.06,.23,sdBox(q+vec2(.018,-.016),vec2(.205,.12)));
 vec3 col=mix(under,vec3(.008,.021,.028),shadow*.65);
 vec2 texcoord=vec2(.5+q.x*1.8868,.5+q.y*1.8868);
 vec4 sprite=texture(u_kartSprite,texcoord);
 float inside=step(0.,texcoord.x)*step(texcoord.x,1.)*step(0.,texcoord.y)*step(texcoord.y,1.);
 col=mix(col,sprite.rgb,sprite.a*inside);
 float damage=1.-u_integrity;
 if(damage>.05){float wounds=1.-smoothstep(.025,.095,length(q-u_damagePoint));
  float scar=abs(sin((q.x+q.y*.7)*235.+noise(q*80.)*2.));
  col=mix(col,vec3(.95,.14,.06),wounds*damage*(.45+.45*scar)*sprite.a*inside);
 }
 float glow=pow(max(0.,1.-length(q-vec2(.21,0.))/.14),3.);
 col+=vec3(.02,.14,.13)*glow*(u_quality>1.5?1.:.45);
 return col;
}

float rain(vec2 uv,float t,float snow){
  float g=0.;
  for(int k=0;k<3;k++){
   if(float(k)>u_quality+.51)break;
   float sc=36.+float(k)*27.;vec2 id=floor(uv*sc+vec2(float(k)*17.));
   float a=hash21(id+float(k)*12.);
   vec2 p=fract(uv*sc+vec2(float(k)*17.));
   vec2 pos=vec2(a,fract(a*13.1+t*(snow>.5?.23:3.8)*(1.+float(k)*.3)));
   if(snow>0.5){g+=(1.-smoothstep(.015,.047,length(p-pos)))*(.7-float(k)*.14);}
   else {vec2 v=vec2((p.x-pos.x)*1.5,(p.y-pos.y)*.15+(p.x-pos.x)*.45);g+=(1.-smoothstep(.01,.03,length(v)))*.25;}
  }
  return g;
}
void main(){
  vec2 pix=gl_FragCoord.xy;
  vec2 norm=(pix-u_resolution*.5)/u_resolution.y;
  vec2 world=norm*u_zoom+u_camera;
  float d=track(world);
  vec3 col=baseCol(world,d);
  col=kart(world,col,d);
  float grassBlade=sin(world.x*480.)*sin(world.y*400.);
  col+=.005*grassBlade;
  // Luz cinematografica variavel, vinheta, haze e reflexo de lente.
  float cloud=noise(world*.6+vec2(u_time*.014,-u_time*.009));
  float shadow=.82+.22*cloud;
  col*=shadow;
  float dark=(u_weather>2.5 && u_weather<3.5)? .61:1.;
  if(u_weather>3.5) {col=mix(col,vec3(.24,.33,.39),.29+.17*(1.-smoothstep(0.,1.3,length(world-u_car))));}
  col*=dark;
  float lightning=(u_weather>2.5&&u_weather<3.5)?pow(max(0.,sin(u_time*2.17)*sin(u_time*.93)),70.)*.9:0.;
  col+=lightning*vec3(.5,.67,.94);
  // Precipitacao em coordenadas de tela; sol continua sem particulas.
  vec2 scr=pix/u_resolution.y;
  if(u_weather>.5&&u_weather<3.5){float p=rain(scr,u_time,u_weather>1.5&&u_weather<2.5?1.:0.);col+=p*((u_weather>1.5&&u_weather<2.5)?vec3(.68,.8,.9):vec3(.27,.41,.52));}
  if(u_weather<.5){float sundir=max(0.,1.-length(norm-vec2(.6,.85))*.8);col+=pow(sundir,7.)*vec3(.055,.07,.035);}
  if(u_flash>0.){col=mix(col,vec3(.87,.36,.22),u_flash*.27);}
  float vignette=1.-.6*pow(length(norm*vec2(1.16,1.)),2.4);
  col*=vignette;
  float grain=hash21(pix+fract(u_time*12.))-.5;
  col+=grain*.009;
  if(u_quality>2.5){
    // Contraste filmico e bloom barato: glow baseado na luminancia local.
    float lum=dot(col,vec3(.21,.72,.07));
    col+=max(0.,lum-.23)*.10*vec3(.16,.32,.29);
    col=col*(2.51*col+.03)/(col*(2.43*col+.59)+.14);
  }
  col=pow(max(col,0.),vec3(.94));
  FragColor=vec4(col,1.);
}
