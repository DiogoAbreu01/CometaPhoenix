#version 330 core
out vec4 FragColor;
uniform vec2 u_resolution;
uniform float u_time;
uniform vec2 u_camera;
uniform vec2 u_car;
uniform float u_heading;
uniform vec2 u_ghostPos;
uniform float u_ghostHeading;
uniform float u_hasGhost;
uniform float u_rivalCount;
uniform float u_accessible;
uniform vec2 u_rival0,u_rival1;
uniform float u_rivalAngle0,u_rivalAngle1;
uniform float u_zoom;
uniform float u_weather;
uniform float u_speed;
uniform float u_flash;
uniform float u_integrity;
uniform vec2 u_damagePoint;
uniform float u_quality;
uniform float u_racingAssist;
uniform float u_cinematic;
uniform float u_box;
uniform float u_night;
uniform float u_circuit;
uniform sampler2D u_kartSprite;
uniform sampler2D u_sakuraSprite;
uniform sampler2D u_asphaltPhoto,u_asphaltNormal;
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
float radius(float a){vec4 s=u_circuit<.5?vec4(4.65,.62,.34,.22):u_circuit<1.5?vec4(5.35,.94,.22,.12):(u_circuit<2.5?vec4(5.85,.32,.58,.08):vec4(5.15,.67,.29,.14));return s.x+s.y*cos(3.*a+.3)+s.z*sin(5.*a-.6)+s.w*cos(7.*a+1.2);}
// Filter periodic markings toward their mean when the pattern is subpixel.
float marking(float phase,float threshold,float pixelAngle,float frequency){
  float width=clamp(pixelAngle*frequency,.04,1.);
  return mix(smoothstep(threshold-width,threshold+width,cos(phase)),
             acos(threshold)/PI,smoothstep(.6,1.,width));
}
// SAKURA TOUGE: hanami grove and red lacquer shrine, hand-authored GPU scene.
vec3 sakuraGarden(vec2 p,float d,vec3 col){
    float roadside=smoothstep(.53,.90,abs(d));
    if(roadside<=.001)return col;
    // Stone walkway, lotus pond and vermilion torii landmark in the central garden.
    vec2 temple=p-vec2(-.58,.35);
    float stone=1.-smoothstep(.55,.72,abs(temple.y+.08));
    stone*=1.-smoothstep(1.16,1.46,abs(temple.x));
    float slabs=step(.51,fract(temple.x*5.+.05));
    col=mix(col,mix(vec3(.30,.27,.32),vec3(.41,.39,.40),slabs),stone*roadside*.72);
    vec2 q=temple-vec2(.0,.10);
    float shrine=1.-smoothstep(.015,.046,sdBox(q,vec2(.39,.27)));
    col=mix(col,vec3(.48,.13,.18),shrine*roadside);
    float eave=1.-smoothstep(.013,.035,sdBox(q+vec2(.0,.30),vec2(.48,.055)));
    float eave2=1.-smoothstep(.013,.035,sdBox(q-vec2(.0,.30),vec2(.46,.050)));
    col=mix(col,vec3(.91,.37,.31),(eave+eave2)*roadside*.88);
    float ridge=(1.-smoothstep(.007,.025,abs(q.x)))*(1.-smoothstep(.2,.3,abs(q.y)));
    col=mix(col,vec3(.94,.72,.45),ridge*roadside*.7);
    // Lanterns and straight stone steps by the shrine.
    for(int i=0;i<4;++i){
       vec2 lp=temple-vec2(-.87+float(i)*.57,-.67);
       float halo=1.-smoothstep(.055,.19,length(lp));
       float core=1.-smoothstep(.012,.041,length(lp));
       col+=vec3(.4,.21,.11)*halo*roadside*.25;
       col=mix(col,vec3(1.,.60,.35),core*roadside);
    }
    // Blossoms: overlapping lobes, warm shadows, micro-highlights, deterministic placement.
    if(u_quality<.5)return col;
    vec2 cell=floor(p*1.62);
    for(int ix=-1;ix<=1;++ix)for(int iy=-1;iy<=1;++iy){
        vec2 id=cell+vec2(float(ix),float(iy));
        float seed=hash21(id+7.2);
        if(seed<.17)continue;
        vec2 c=(id+vec2(hash21(id+11.2),hash21(id+27.8))*.89)/1.62;
        vec2 delta=p-c;
        float rr=.18+.12*hash21(id+4.5);
        float l=length(delta);
        if(l>rr+.06)continue;
        float shade=1.-smoothstep(rr*.72,rr*1.23,length(delta+vec2(.065,-.065)));
        col*=1.-shade*.17*roadside;
        vec2 uv=.5+delta/(rr*2.);
        vec4 blossoms=texture(u_sakuraSprite,uv);
        float tree=blossoms.a*roadside;
        vec3 ink=mix(vec3(.86,.78,.91),vec3(1.03,.93,.97),seed);
        col=mix(col,blossoms.rgb*ink,tree*.98);
    }
    // Roadside vermilion torii gates and warm lanterns mark the touge curves.
    for(int gateId=0;gateId<4;++gateId){
       float a=.52+float(gateId)*1.56;
       float r=radius(a)+.86;
       vec2 center=r*vec2(cos(a),sin(a));
       vec2 q=rot(-a)*(p-center)*2.0;
       if(length(q)>.75)continue;
       float beam=1.-smoothstep(.025,.053,sdBox(q-vec2(.0,.10),vec2(.38,.050)));
       float beam2=1.-smoothstep(.018,.035,sdBox(q-vec2(.0,.19),vec2(.46,.033)));
       float pillar1=1.-smoothstep(.012,.032,sdBox(q-vec2(-.29,-.08),vec2(.040,.23)));
       float pillar2=1.-smoothstep(.012,.032,sdBox(q-vec2(.29,-.08),vec2(.040,.23)));
       float wood=max(max(beam,beam2),max(pillar1,pillar2));
       float shadow=1.-smoothstep(.20,.55,length(q+vec2(.055,-.055)));
       col*=1.-shadow*.22*roadside;
       col=mix(col,vec3(.78,.17,.21),wood*roadside);
       float gilt=(1.-smoothstep(.008,.020,abs(q.y-.18)))*(1.-smoothstep(.20,.23,abs(q.x)));
       col=mix(col,vec3(.99,.74,.40),gilt*roadside*.75);
       // Lantern halos beside the gate.
       for(int j=0;j<2;++j){
          vec2 lantern=q-vec2(j==0?-.53:.53,-.08);
          float halo=1.-smoothstep(.07,.20,length(lantern));
          float flame=1.-smoothstep(.018,.045,length(lantern));
          col+=vec3(.23,.095,.047)*halo*.52*roadside;
          col=mix(col,vec3(1.,.68,.38),flame*roadside);
       }
    }
    return col;
}
vec3 sakuraPetals(vec2 screen,vec3 col,float t){
 if(u_quality<1.5)return col;
 // Light, sparse screen-space petals drifting; no expensive particle buffers.
 for(int layer=0;layer<2;++layer){
    float L=float(layer);
    vec2 flow=screen*vec2(14.+L*5.,8.+L*3.)+vec2(t*(.11+L*.05),-t*(.27+L*.09));
    vec2 cell=floor(flow),f=fract(flow);
    float seed=hash21(cell+L*39.);
    vec2 center=vec2(hash21(cell+7.+L),hash21(cell+15.+L));
    vec2 diff=(f-center)*vec2(1.0,1.4);
    diff.x+=sin(t+seed*6.)*.075;
    float petal=(1.-smoothstep(.035,.090,length(diff)))*step(.91,seed);
    col=mix(col,vec3(.99,.78,.82),petal*.68);
 }
 return col;
}
vec3 baseCol(vec2 p,float d,float angle){
  float pixel=max(u_zoom/u_resolution.y,.0001);
  float edgeAA=max(fwidth(d)*.5,.004);
  float pixelAngle=pixel/max(length(p),.1);
  float outer=abs(d);
  float terrain=noise(p*5.);
  vec3 ground;
  if(u_circuit>2.5){
    float bamboo=noise(p*vec2(3.1,5.4)),scree=noise(p*12.);
    ground=mix(vec3(.062,.112,.135),vec3(.16,.23,.21),terrain);
    ground+=vec3(.05,.035,.028)*smoothstep(.70,.86,bamboo)*.8;
    float stream=abs(p.x-(2.5+.51*sin(p.y*.94)+.25*sin(p.y*1.77)));
    float water=1.-smoothstep(.23,.42,stream);
    vec3 lake=mix(vec3(.11,.24,.30),vec3(.26,.39,.43),noise(p*4.+u_time*.04));
    lake+=.06*sin(p.y*12.+u_time*.3)*vec3(.44,.42,.44);
    ground=mix(ground,lake,water*.8);
    float stone=smoothstep(.57,.86,scree)*smoothstep(.65,1.6,abs(d));
    ground=mix(ground,vec3(.28,.26,.28),stone*.23);
  }else if(u_circuit>1.5){
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

  bool japan=u_circuit>2.5;
  float road=1.-smoothstep(.44-edgeAA,.44+edgeAA,outer);
  vec3 col=ground;
  vec2 tileUV=p*5.;
  vec2 tileDX=dFdx(tileUV),tileDY=dFdy(tileUV);
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
    // CC0 photo material: diffuse RGB, roughness alpha, independently sampled GL normal.
    vec4 photo=textureGrad(u_asphaltPhoto,tileUV,tileDX,tileDY);
    vec3 realColor=pow(max(photo.rgb,vec3(.001)),vec3(2.2));
    vec3 realNormal=textureGrad(u_asphaltNormal,tileUV,tileDX,tileDY).xyz*2.-1.;
    float lit=clamp(.82+.27*dot(realNormal,normalize(vec3(-.5,.6,.55))),.6,1.12);
    asphalt=mix(asphalt,realColor*.82*lit+vec3(.018),japan?.51:.83);
    if(japan) asphalt=mix(asphalt,vec3(.15,.14,.18),.31);
    // Rain and storm share the wet surface. Snow and fog use their own effects.
    bool storm=u_weather>2.5 && u_weather<3.5;
    if((u_weather>.5 && u_weather<1.5) || storm){
      float basin=noise(p*vec2(8.,16.)+vec2(.2,2.));
      float wetEdge=1.-smoothstep(.31,.42,outer);
      float puddle=smoothstep(storm?.53:.63,storm?.66:.75,basin)*wetEdge;
      float sky=smoothstep(.12,.83,terrain);
      asphalt=mix(asphalt*.79,mix(vec3(.030,.049,.068),vec3(.13,.18,.22),sky),puddle*mix(.88,.60,photo.a));
    }
    col=mix(ground,asphalt,road);
  }
  float stripe=1.-smoothstep(.013-edgeAA,.013+edgeAA,abs(outer-.336));
  col=mix(col,japan?vec3(.94,.79,.74):vec3(.78,.82,.71),stripe*(japan?.72:.64));
  float curb=1.-smoothstep(.023-edgeAA,.023+edgeAA,abs(outer-.442));
  float red=marking(angle*144.,0.,pixelAngle,144.);
  float curbFace=mix(.74,1.,smoothstep(.423,.443,outer));
  col=mix(col,mix(japan?vec3(.99,.87,.82):vec3(.91,.90,.82),japan?vec3(.78,.28,.38):vec3(.70,.080,.075),red)*curbFace,curb*.97);
  float mid=1.-smoothstep(.011-edgeAA,.011+edgeAA,outer);
  float dash=marking(angle*82.,.16,pixelAngle,82.);
  col=mix(col,japan?vec3(.98,.75,.49):vec3(.88,.76,.40),mid*dash*(japan?.79:.67));
  if(u_weather>1.5&&u_weather<2.5){
   float snow=noise(p*19.);float ss=smoothstep(.4,.67,snow)*.78;
   col=mix(col,vec3(.71,.81,.89),ss*(smoothstep(.31,.46,abs(d))));
   col=mix(col,vec3(.2,.25,.29),ss*.08);
  }
  col=forestDetails(p,col,d);
  if(japan) col=sakuraGarden(p,d,col);
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
  col=mix(col,mix(japan?vec3(.35,.32,.41):vec3(.18,.27,.30),japan?vec3(.93,.77,.73):vec3(.80,.88,.86),barrierStripe),barrier*.78);
  float gravel=smoothstep(.46,.50,outer)*(1.-smoothstep(.75,.85,outer));
  col=mix(col,vec3(.13,.14,.145)+terrain*.038,gravel*.63);
  float beacon=1.-smoothstep(.008,.020,length(vec2(outer-.592,mod(angle*40.+.5,1.)-.5)));
  col+=(japan?vec3(.91,.24,.42):vec3(.025,.72,.64))*beacon*step(1.5,u_quality)*.82;
  // Marcas de frenagem concentradas nas entradas das curvas.
  float braking=smoothstep(.38,.82,abs(sin(3.*angle+.3)))*smoothstep(.03,.14,abs(d));
  float skid=pow(max(0.,sin(angle*71.+d*14.)),30.);
  col*=1.-braking*skid*.08*(1.-smoothstep(.32,.43,outer));
  // Optional accessible racing guide: safe center-line cue with amber apex warnings.
  if(u_racingAssist>.5 && outer<.42){
    float line=1.-smoothstep(.012,.030,abs(d+.09));
    float apex=smoothstep(.64,.88,abs(sin(3.*angle+.3)));
    col=mix(col,mix(japan?vec3(.92,.46,.59):vec3(.83,.50,.62),vec3(.93,.56,.08),apex),line*.51);
  }
  // Sombra direcional das muretas: reforco de profundidade visual.
  float castShadow=1.-smoothstep(.030,.10,abs(outer-.62));
  col*=1.-castShadow*.12;
  return col;
}
float wheel(vec2 q,vec2 c){return sdBox(q-c,vec2(.053,.041));}
// Kart em 2D: textura top-down derivada da malha 3D de 53 mil triangulos.
// Uma unica amostra de textura substitui multiplos SDF de quadrados.
vec3 tyreSpray(vec2 p,vec3 under){
  if(u_quality<2.5 || u_speed<17.)return under;
  vec2 delta=p-u_car;
  if(dot(delta,delta)>.52)return under;
  vec2 forward=vec2(cos(u_heading),sin(u_heading));
  float rear=-dot(delta,forward);
  float side=abs(dot(delta,vec2(-forward.y,forward.x)));
  float band=smoothstep(.10,.20,rear)*(1.-smoothstep(.43,.67,rear))*
             (1.-smoothstep(.05,.23,side));
  if(band<.02)return under;
  float wet=(u_weather>.5&&u_weather<3.5)?1.:.32;
  float flakes=noise(p*68.-forward*u_time*13.);
  vec3 mist=u_weather>1.5&&u_weather<2.5?vec3(.67,.75,.84):vec3(.34,.43,.44);
  return mix(under,mist,band*(.12+.18*flakes)*wet);
}
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

// Two procedural C++17 rivals. Sprite is sampled only near each kart.
vec3 racingRival(vec2 p,vec3 ground,vec2 pos,float heading,vec3 tint){
 vec2 delta=p-pos;
 if(dot(delta,delta)>.21)return ground;
 vec2 q=rot(-heading)*delta;
 vec2 uv=vec2(.5+q.x*1.8868,.5+q.y*1.8868);
 if(any(lessThan(uv,vec2(0.))) || any(greaterThan(uv,vec2(1.))))return ground;
 vec4 sprite=texture(u_kartSprite,uv);
 vec3 paint=mix(sprite.rgb,tint,.60);
 float shadow=1.-smoothstep(.12,.26,length(q));
 return mix(mix(ground,vec3(.01,.01,.02),.22*shadow),paint,sprite.a*.92);
}
// Best-lap replay ghost: translucent, non-colliding and sampled from the MK-III sprite.
vec3 replayGhost(vec2 p,vec3 under){
 if(u_hasGhost<.5)return under;
 vec2 delta=p-u_ghostPos;
 if(dot(delta,delta)>.20)return under;
 vec2 q=rot(-u_ghostHeading)*delta;
 vec2 uv=vec2(.5+q.x*1.8868,.5+q.y*1.8868);
 if(any(lessThan(uv,vec2(0.)))||any(greaterThan(uv,vec2(1.))))return under;
 vec4 s=texture(u_kartSprite,uv);
 float pulse=.90+.10*sin(u_time*4.7);
 vec3 tint=mix(s.rgb,vec3(.99,.70,.85),.74);
 return mix(under,tint,s.a*.36*pulse);
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
  if(u_rivalCount>.5)col=racingRival(world,col,u_rival0,u_rivalAngle0,mix(vec3(.30,.44,.76),vec3(.12,.68,.53),u_accessible));
  if(u_rivalCount>1.5)col=racingRival(world,col,u_rival1,u_rivalAngle1,vec3(.98,.70,.37));
  col=replayGhost(world,col);
  // Spray de pneu limitado a regiao atras do kart (chuva e tempestade).
  if(u_quality>1.5 && u_speed>10. &&
     ((u_weather>.5 && u_weather<1.5) || (u_weather>2.5 && u_weather<3.5))){
     vec2 delta=world-u_car;
     if(dot(delta,delta)<.8){
        vec2 back=rot(-u_heading)*delta;
        float trail=smoothstep(.1,.19,-back.x)*(1.-smoothstep(.4,.82,-back.x));
        float width=1.-smoothstep(.05,.27,abs(back.y));
        float particulate=noise(world*83.+vec2(u_time*4.,-u_time*3.));
        float spray=trail*width*(.1+.25*particulate)*(u_speed/100.);
        col=mix(col,vec3(.56,.7,.76),clamp(spray,0.,.25));
     }
  }
  col=tyreSpray(world,col);
  col=kart(world,col,d);
  if(u_circuit>2.5) col=sakuraPetals(gl_FragCoord.xy/u_resolution.y,col,u_time);
  // Luz cinematografica variavel, vinheta, haze e reflexo de lente.
  float cloud=noise(world*.6+vec2(u_time*.014,-u_time*.009));
  float shadow=.90+.15*cloud;
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
  float vignette=1.-.42*pow(length(norm*vec2(1.16,1.)),2.4);
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
