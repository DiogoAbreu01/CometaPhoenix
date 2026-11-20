from pathlib import Path
from PIL import Image,ImageDraw,ImageFilter,ImageFont,ImageEnhance
import numpy as np
import math,random
random.seed(40)
p=Path(__file__).resolve().parents[1]
w,h=1280,720
old=p/"game/assets/phoenix_cinematic_menu.rgb"
img=Image.frombytes("RGB",(w,h),old.read_bytes())
# Filmic split-tone: dusk-plum in shadows, amber/pink highlights.
a=np.asarray(img,dtype=np.float32)/255.
lum=(.24*a[:,:,0]+.65*a[:,:,1]+.11*a[:,:,2])
a[:,:,0]=np.clip(a[:,:,0]*1.10+.028*(1-lum)+.024*lum,0,1)
a[:,:,1]=np.clip(a[:,:,1]*.91+.007*lum,0,1)
a[:,:,2]=np.clip(a[:,:,2]*1.05+.050*(1-lum),0,1)
# Gentle rose-gold haze on the upper landscape, dark film vignette on the left for typography.
yy,xx=np.ogrid[:h,:w]
bloom=np.exp(-((xx-935.)**2/(2*350.**2)+(yy-135.)**2/(2*205.**2)))
a+=bloom[:,:,None]*np.array([.080,.030,.054],dtype=np.float32)
vignette=.96-.16*np.maximum(np.abs(xx/w-.56),np.abs(yy/h-.49))
a*=vignette[:,:,None]
img=Image.fromarray(np.uint8(np.clip(a,0,1)*255),"RGB").convert("RGBA")
# Blossoming twigs draped across scene, painterly rather than repeated stock trees.
branch=Image.new("RGBA",(w,h))
b=ImageDraw.Draw(branch,"RGBA")
def twig(x,y,ang,L,depth,width):
    if depth==0:
        # A pink cluster at each natural branch tip, with staggered 5-petal flowers.
        for j in range(random.randint(2,5)):
            fx=x+random.gauss(0,16)
            fy=y+random.gauss(0,12)
            rr=random.uniform(3.5,9.)
            for k in range(5):
                th=k*math.tau/5+random.uniform(-.15,.15)
                cx=fx+math.cos(th)*rr*.55
                cy=fy+math.sin(th)*rr*.55
                b.ellipse((cx-rr*.55,cy-rr*.48,cx+rr*.55,cy+rr*.48),
                          fill=random.choice([(248,174,202,172),(255,203,215,184),(237,145,189,162),(255,226,218,185)]))
            b.ellipse((fx-1.5,fy-1.5,fx+1.5,fy+1.5),fill=(250,198,116,220))
        return
    ex=x+math.cos(ang)*L;ey=y+math.sin(ang)*L
    b.line((x,y,ex,ey),fill=(46,30,49,int(125+depth*15)),width=max(1,int(width)))
    for k in range(2 if depth<=2 else 3):
        twig(ex,ey,ang+random.uniform(-.9,.9),L*random.uniform(.48,.82),depth-1,width*.57)
# Cluster at top-right and one subtle upper-left that don't overrun the text area.
twig(1320,-20,2.47,150,5,15)
twig(1180,-35,2.56,120,4,11)
twig(-22,-32,.65,85,4,7)
branch=branch.filter(ImageFilter.GaussianBlur(.46))
img=Image.alpha_composite(img,branch)
# Floating soft petals, varying scale and depth.
petals=Image.new("RGBA",(w,h))
d=ImageDraw.Draw(petals,"RGBA")
for i in range(185):
    x=random.randrange(0,w)
    y=random.randrange(0,h)
    r=random.uniform(.9,4.2)
    if x<580 and y>170 and random.random()<.83:continue
    tint=random.choice([(246,179,202,150),(255,214,218,147),(241,146,184,135),(252,233,227,145)])
    d.ellipse((x-r,y-r*.48,x+r,y+r*.48),fill=tint)
img=Image.alpha_composite(img,petals)
# Subtle Japanese street light decals + lacquer-red emblem: not a copy of a logo.
deco=Image.new("RGBA",(w,h))
dd=ImageDraw.Draw(deco,"RGBA")
dd.ellipse((1051,69,1220,236),outline=(251,162,182,112),width=2)
dd.ellipse((1064,82,1207,223),outline=(251,183,179,61),width=1)
dd.line((1082,126,1188,126),fill=(232,127,146,95),width=2)
# A torii silhouette in the rising-sun badge.
dd.rounded_rectangle((1110,137,1165,143),3,fill=(252,177,179,163))
dd.polygon([(1105,133),(1169,133),(1175,136),(1099,136)],fill=(248,168,178,185))
dd.rectangle((1114,143,1119,179),fill=(232,131,149,180))
dd.rectangle((1157,143,1162,179),fill=(232,131,149,180))
dd.rectangle((1120,155,1157,158),fill=(248,168,178,150))
# Speed-trail strokes across the lower roadway.
for i in range(4):
    yy=644+i*11
    dd.line((725,yy,1210,yy-125),fill=(241,129,166,int(38-i*7)),width=max(1,4-i))
# Delicate foil stripe along bottom.
dd.rectangle((0,708,w,710),fill=(255,187,161,95))
img=Image.alpha_composite(img,deco)
# Japanese art mark, readable using installed gothic glyphs, restrained.
letter=Image.new("RGBA",(w,h))
ld=ImageDraw.Draw(letter)
font=ImageFont.truetype("C:/Windows/Fonts/YuGothM.ttc",36)
ld.text((1093,242),"桜",font=font,fill=(255,220,224,174))
tiny=ImageFont.truetype("C:/Windows/Fonts/arial.ttf",12)
ld.text((1028,293),"SAKURA / NIGHT RUN",font=tiny,fill=(255,211,214,180),stroke_width=0)
img=Image.alpha_composite(img,letter)
# Add low-frequency bloom without oversharpening the image.
soft=img.convert("RGB").filter(ImageFilter.GaussianBlur(17))
img=Image.blend(img.convert("RGB"),soft,.08)
out=p/"game/assets/phoenix_jdm_menu.rgb"
out.write_bytes(img.tobytes())
prev=p/"docs/screenshots/sakura_jdm_menu_art.jpg"
img.save(prev,quality=90)
print("JDM_MENU_ASSET",out.stat().st_size,"PREVIEW",prev)
