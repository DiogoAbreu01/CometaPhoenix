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
float noise(vec2 p){vec2 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);return mix(mix(hash21(i),hash21(i+vec2(1,0)),f.x),mix(hash21(i+vec2(0,1)),hash21(i+vec2(1,1)),f.x),f.y);}
// Copa de arvores e detalhes de floresta gerados na GPU apenas em qualidade alta.
// Cada fragmento usa no maximo 4 celulas; nao ha objetos CPU por arvore.
vec3 forestDetails(vec2 p,vec3 base,float distanceToTrack){
 if(u_quality<1.5 || (u_circuit>1.5 && distanceToTrack>0.))return base;
 float woods=smoothstep(.51,.68,abs(distanceToTrack));
 if(woods<=.02)return base;
 vec2 pos=p*3.05,cell=floor(pos);vec3 outC=base;
 for(int ox=0;ox<2;ox++)for(int oy=0;oy<2;oy++){
   vec2 id=cell+vec2(float(ox)-.5,float(oy)-.5);
   vec2 randomPos=vec2(hash21(id+17.1),hash21(id+68.3));
   vec2 q=p-(id+randomPos)/3.05;
   float dist=length(q),h=hash21(id+5.6);
   float crown=.12+.06*h;
   float azimuth=atan(q.y,q.x);
   float shape=sin(azimuth*9.+h*6.)*.007+sin(azimuth*15.)*.004;
   float shadow=(1.-smoothstep(crown*.74,crown*1.45,length(q+vec2(.028,-.035))))*.29;
   outC*=1.-shadow*woods;
   float tree=(1.-smoothstep(crown-.013,crown+.017,dist+shape))*woods*step(.19,h);
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
// Filter periodic markings toward their mean when the pattern is subpixel.
float marking(float phase,float threshold,float pixelAngle,float frequency){
  float width=clamp(pixelAngle*frequency,.04,1.);
  return mix(smoothstep(threshold-width,threshold+width,cos(phase)),
             acos(threshold)/PI,smoothstep(.6,1.,width));
}
vec3 baseCol(vec2 p,float d,float angle){
  float pixel=max(u_zoom/u_resolution.y,.0001);
  float edgeAA=max(fwidth(d)*.5,.004);
  float pixelAngle=pixel/max(length(p),.1);
  float outer=abs(d);
  float terrain=noise(p*5.);
  vec3 ground;
  if(u_circuit>1.5){
    float shore=d+(terrain-.5)*.13;
    ground=mix(vec3(.25,.20,.13),vec3(.43,.35,.22),terrain);
    if(shore>.80){
      float depth=smoothstep(.84,1.85,shore);
      float waves=sin(25.*dot(p,vec2(.44,.90))-u_time*1.2+terrain*3.);
      waves*=1.-smoothstep(.02,.09,pixel);
      vec3 water=mix(vec3(.059,.29,.29),vec3(.023,.13,.22),depth);
      water+=vec3(.018,.044,.049)*waves;
      float foam=(1.-smoothstep(.02,.10,abs(shore-.94+waves*.024)));
      water=mix(water,vec3(.51,.70,.67),foam*.33);
      ground=mix(ground,water,smoothstep(.80,1.00,shore));
    }
  }else if(u_circuit>.5){
    float scree=noise(p*22.);
    vec3 rock=mix(vec3(.10,.12,.13),vec3(.30,.27,.23),terrain);
    float snowline=smoothstep(.8,1.65,outer+(terrain-.5)*.75);
    ground=mix(rock,vec3(.59,.66,.72),snowline*mix(.50,.92,scree));
  }else{
    ground=mix(vec3(.027,.083,.078),vec3(.042,.15,.115),terrain);
    ground=mix(ground,vec3(.014,.051,.059),smoothstep(5.6,7.2,length(p))*.7);
  }

  float road=1.-smoothstep(.44-edgeAA,.44+edgeAA,outer);
  vec3 col=ground;
  if(road>0.){
    vec3 asphalt=mix(vec3(.057,.064,.071),vec3(.102,.109,.115),terrain);
    float grainFade=1.-smoothstep(.29,.72,pixel*66.);
    if(grainFade>0.){
      vec2 cells=p*66.;
      float seed=hash21(floor(cells));
      vec2 stone=fract(cells)-mix(vec2(.29),vec2(.71),vec2(seed,fract(seed*19.)));
      float radius=.08+.15*seed;
      float grainAA=max(.04,pixel*33.);
      float chip=1.-smoothstep(radius-grainAA,radius+grainAA,length(stone));
      asphalt+=grainFade*(chip*mix(-.020,.058,fract(seed*37.))-.004);
    }
    // Rain and storm share the wet surface. Snow and fog use their own effects.
    bool storm=u_weather>2.5 && u_weather<3.5;
    if((u_weather>.5 && u_weather<1.5) || storm){
      float basin=noise(p*vec2(8.,16.)+vec2(.2,2.));
      float wetEdge=1.-smoothstep(.31,.42,outer);
      float puddle=smoothstep(storm?.53:.63,storm?.66:.75,basin)*wetEdge;
      float sky=smoothstep(.12,.83,terrain);
      asphalt=mix(asphalt*.79,mix(vec3(.030,.049,.068),vec3(.13,.18,.22),sky),puddle*.84);
    }
    col=mix(ground,asphalt,road);
  }
  float stripe=1.-smoothstep(.013-edgeAA,.013+edgeAA,abs(outer-.336));
  col=mix(col,vec3(.78,.82,.71),stripe*.64);
  float curb=1.-smoothstep(.023-edgeAA,.023+edgeAA,abs(outer-.442));
  float red=marking(angle*144.,0.,pixelAngle,144.);
  float curbFace=mix(.74,1.,smoothstep(.423,.443,outer));
  col=mix(col,mix(vec3(.91,.90,.82),vec3(.70,.080,.075),red)*curbFace,curb*.97);
  float mid=1.-smoothstep(.011-edgeAA,.011+edgeAA,outer);
  float dash=marking(angle*82.,.16,pixelAngle,82.);
  col=mix(col,vec3(.88,.76,.40),mid*dash*.67);
  if(u_weather>1.5&&u_weather<2.5){
   float snow=noise(p*19.);float ss=smoothstep(.4,.67,snow)*.78;
   col=mix(col,vec3(.71,.81,.89),ss*(smoothstep(.31,.46,abs(d))));
   col=mix(col,vec3(.2,.25,.29),ss*.08);
  }
  col=forestDetails(p,col,d);
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
  // Illuminated pit-strip and asphalt lane microstructure.
  float pitGlow=smoothstep(radius(0.)+.23,radius(0.)+.42,p.x)*(1.-smoothstep(radius(0.)+1.69,radius(0.)+1.91,p.x));
  float neon=1.-smoothstep(.006,.018,abs(p.y-.23));
  col+=vec3(.018,.23,.18)*neon*pitGlow*step(1.5,u_quality);

  // Elementos da pista longa: muretas, linha de escape, barreiras e refletores laterais.
  float barrier=1.-smoothstep(.017,.035,abs(outer-.58));
  float barrierStripe=marking(angle*478.,.06,pixelAngle,478.);
  col=mix(col,mix(vec3(.18,.27,.30),vec3(.80,.88,.86),barrierStripe),barrier*.78);
  float gravel=smoothstep(.46,.50,outer)*(1.-smoothstep(.75,.85,outer));
  col=mix(col,vec3(.13,.14,.145)+terrain*.038,gravel*.63);
  float beacon=1.-smoothstep(.008,.020,length(vec2(outer-.592,mod(angle*40.+.5,1.)-.5)));
  col+=vec3(.025,.72,.64)*beacon*step(1.5,u_quality)*.82;
  // Marcas de frenagem concentradas nas entradas das curvas.
  float braking=smoothstep(.38,.82,abs(sin(3.*angle+.3)))*smoothstep(.03,.14,abs(d));
  float skid=pow(max(0.,sin(angle*71.+d*14.)),30.);
  col*=1.-braking*skid*.08*(1.-smoothstep(.32,.43,outer));
  // Sombra direcional das muretas: reforco de profundidade visual.
  float castShadow=1.-smoothstep(.030,.10,abs(outer-.62));
  col*=1.-castShadow*.12;
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
  float angle=atan(world.y,world.x);
  float d=length(world)-radius(angle);
  vec3 col=baseCol(world,d,angle);
  col=kart(world,col,d);
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
