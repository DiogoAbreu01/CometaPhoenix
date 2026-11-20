#version 330 core
in vec3 v_normal,v_world,v_view;
out vec4 FragColor;
flat in vec3 u_color;
flat in vec2 u_material;
flat in float u_group,u_finish;
uniform float u_time,u_damage,u_repair,u_part,u_quality;
const float PI=3.14159265359;
float sat(float a){return clamp(a,0.0,1.0);}
float distributionGGX(vec3 n,vec3 h,float rough){
 float a=rough*rough,a2=a*a,ndh=max(dot(n,h),0.0);
 float ndh2=ndh*ndh;
 float den=1.0-ndh2+ndh2*a2;
 return a2/max(PI*den*den,1e-8);
}
float geomSchlick(float ndv,float rough){float r=(rough+1.0),k=(r*r)/8.0;return ndv/(ndv*(1.0-k)+k+0.0001);}
vec3 fresnel(float cosTheta,vec3 f0){return f0+(1.0-f0)*pow(1.0-cosTheta,5.0);}
float filteredWave(float phase,float footprint){
 return sin(phase)*(1.0-smoothstep(.65,2.8,footprint));
}
vec3 directLight(vec3 n,vec3 v,vec3 l,vec3 radiance,vec3 base,float rough,float metal,
                 vec2 coat){
 vec3 h=normalize(v+l);
 float ndl=max(dot(n,l),0.0),ndv=max(dot(n,v),0.01);
 if(ndl<=0.0)return vec3(0.0);
 float vdh=sat(dot(h,v));
 vec3 f0=mix(vec3(0.045),base,metal),F=fresnel(vdh,f0);
 float N=distributionGGX(n,h,rough),G=geomSchlick(ndv,rough)*geomSchlick(ndl,rough);
 float denom=4.0*ndv*ndl+0.001;
 vec3 spec=N*G*F/denom;
 vec3 diffuse=(vec3(1.0)-F)*(1.0-metal)*base/PI;
 vec3 lobes=diffuse+spec;
 if(coat.x>0.0){
   float fresnelCoat=coat.x*(.04+.96*pow(1.0-vdh,5.0));
   // A neutral dielectric layer sits over the colored base lobe.
   float geometry=geomSchlick(ndv,coat.y)*geomSchlick(ndl,coat.y);
   float highlight=distributionGGX(n,h,coat.y)*geometry*fresnelCoat/denom;
   lobes=lobes*(1.0-fresnelCoat)+highlight;
 }
 return lobes*radiance*ndl;
}
void main(){
 vec3 n=normalize(v_normal);if(!gl_FrontFacing)n=-n;
 vec3 v=normalize(v_view);
 float metal=sat(u_material.x),rough=clamp(u_material.y,.10,1.0);
 vec3 base=u_color;
 // Compute derivatives before branching on the per-face finish.
 vec3 footprint=fwidth(v_world);
 float normalFootprint=length(fwidth(n));
 vec2 coat=vec2(0.0);
 if(u_finish>1.5&&u_finish<2.5){
   vec3 warp=vec3(310.0,255.0,78.0),weft=vec3(-255.0,310.0,96.0);
   float weave=filteredWave(dot(v_world,warp),dot(footprint,abs(warp)))
              *filteredWave(dot(v_world,weft),dot(footprint,abs(weft)));
   base*=1.0+.15*weave;
   rough=clamp(rough+.09*weave,.36,.82);
 }
 if(u_finish>2.5&&u_finish<3.5){
   vec3 axisA=vec3(310.0,157.0,191.0),axisB=vec3(-79.0,251.0,183.0);
   float grain=filteredWave(dot(v_world,axisA),dot(footprint,abs(axisA)))
              *filteredWave(dot(v_world,axisB),dot(footprint,abs(axisB)));
   base*=.51+.09*grain;
   rough=.91+.04*grain;
 }
 if(u_finish>4.5&&u_finish<5.5){
   coat=vec2(.68,clamp(.26+.50*normalFootprint,.26,.55));
   float pearl=pow(1.0-max(dot(n,v),0.0),3.0);
   base=mix(base,base+vec3(.02,.10,.10),pearl*.55);
 }
 vec3 col=base*(.17+.12*max(0.0,n.z))*(1.0-metal*.55);
 vec3 l1=normalize(vec3(1.5,-2.8,4.0)-v_world),l2=normalize(vec3(-2.5,2.2,3.5)-v_world);
 vec3 l3=normalize(vec3(1.6,2.8,2.8)-v_world);
 col+=directLight(n,v,l1,vec3(3.5,4.7,4.7),base,rough,metal,coat);
 col+=directLight(n,v,l2,vec3(3.0,1.9,1.3),base,rough,metal,coat);
 col+=directLight(n,v,l3,vec3(.8,1.8,2.9),base,rough,metal,coat);
 // Original studio light-strip reflections, wide and roughness-dependent.
 if(u_quality>1.5){
   vec3 refl=reflect(-v,n);
   float barA=pow(sat(dot(refl,normalize(vec3(-.35,-.62,.70)))),mix(125.0,18.0,rough));
   float barB=pow(sat(dot(refl,normalize(vec3(.71,.39,.50)))),mix(150.0,19.0,rough));
   col+=(vec3(.65,1.0,1.0)*barA+vec3(1.0,.48,.22)*barB)*mix(.22,.9,metal);
 }
 float affected=1.0-step(.49,abs(u_group-u_part));
 float broken=sat(u_damage*affected);
 float scan=.5+.5*sin(v_world.x*83.0+v_world.y*59.0+u_time*10.0);
 col=mix(col,vec3(1.25,.025,.012),broken*(.48+.22*scan));
 // Micro-scratches and fading scar are localized only to the damaged component.
 float rim=pow(1.0-max(dot(n,v),0.0),4.0);
 if(u_finish>6.5)col+=vec3(.0,.22,.23);
 col+=(vec3(.024,.11,.13)*rim)*(u_finish>6.5?2.3:.36);
 // ACES approximation maintains detailed highlights without white clipping.
 vec3 mapped=(col*(2.51*col+.03))/(col*(2.43*col+.59)+.14);
 mapped=clamp(mapped,0.0,1.0);
 FragColor=vec4(pow(mapped,vec3(1.0/2.2)),1.0);
}