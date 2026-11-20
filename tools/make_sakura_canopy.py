from pathlib import Path
from PIL import Image,ImageDraw,ImageFilter
import math,random
random.seed(20261120)
p=Path(__file__).resolve().parents[1]
W=512
im=Image.new("RGBA",(W,W))
# Warm directional cast shadow and a transparent cloud of individual flower clusters.
shadow=Image.new("RGBA",(W,W))
sd=ImageDraw.Draw(shadow,"RGBA")
for i in range(115):
    th=random.random()*math.tau; rr=math.sqrt(random.random())*170
    x=256+math.cos(th)*rr+8;y=256+math.sin(th)*rr+15
    rad=random.uniform(19,36)
    sd.ellipse((x-rad,y-rad,x+rad,y+rad),fill=(37,28,42,30))
shadow=shadow.filter(ImageFilter.GaussianBlur(15))
im.alpha_composite(shadow)
layer=Image.new("RGBA",(W,W))
d=ImageDraw.Draw(layer,"RGBA")
# Small exposed branch structure, carefully tucked beneath the blossoms.
d.ellipse((243,247,269,273),fill=(63,41,48,225))
for k in range(16):
    a=k*math.tau/16+random.uniform(-.15,.15)
    length=random.uniform(95,177)
    x=256+math.cos(a)*length
    y=256+math.sin(a)*length
    d.line((256,256,x,y),fill=(91,59,71,148),width=5)
    for j in range(3):
       t=random.uniform(.4,.95)
       bx=256+math.cos(a)*length*t;by=256+math.sin(a)*length*t
       branchA=a+random.choice((-1,1))*random.uniform(.35,.65)
       d.line((bx,by,bx+math.cos(branchA)*42,by+math.sin(branchA)*42),fill=(109,61,80,110),width=2)
# Canopy lobes with dark burgundy depth, not a single uniform blob.
for i in range(110):
    a=random.uniform(-math.pi,math.pi)
    rr=math.sqrt(random.random())*182
    x=256+rr*math.cos(a);y=256+rr*math.sin(a)
    rad=random.uniform(14,35)
    d.ellipse((x-rad,y-rad*.82,x+rad,y+rad*.82),fill=random.choice([
        (131,74,105,120),(174,96,130,117),(194,119,151,120),(99,62,91,100)
    ]))
# Hundreds of five-petal blossom clusters, in soft cream, mauve, candyfloss pink.
palette=[(252,211,215),(250,183,199),(240,155,189),(227,141,175),
         (255,225,214),(243,194,207),(227,169,203),(244,210,221)]
for i in range(770):
    a=random.random()*math.tau
    rr=math.sqrt(random.random())*194
    x=256+math.cos(a)*rr;y=256+math.sin(a)*rr
    if rr>176 and random.random() < (rr-176)/35:continue
    col=random.choice(palette)
    radius=random.uniform(2.6,7.2)
    angle=random.random()*math.tau
    for petal in range(5):
        th=angle+petal*math.tau/5
        px=x+math.cos(th)*radius*.65;py=y+math.sin(th)*radius*.65
        d.ellipse((px-radius*.75,py-radius*.65,px+radius*.75,py+radius*.65),
                  fill=(*col,random.randrange(170,246)))
    d.ellipse((x-1.3,y-1.3,x+1.3,y+1.3),fill=(225,164,104,218))
# Sparkles and bright sun-kissed flower tips, almost like illustrated watercolors.
for i in range(160):
    a=random.random()*math.tau; rr=math.sqrt(random.random())*191
    x=256+math.cos(a)*rr;y=256+math.sin(a)*rr
    rad=random.uniform(.7,2.3)
    d.ellipse((x-rad,y-rad,x+rad,y+rad),fill=(255,246,232,random.randrange(95,197)))
im.alpha_composite(layer)
im=im.filter(ImageFilter.GaussianBlur(.40))
# Transparent canvas; all individual blossoms retained in raw RGBA texture.
out=p/"game/assets/sakura_canopy.rgba"
out.write_bytes(im.tobytes())
im.save(p/"docs/screenshots/sakura_canopy.png")
opaque=sum(1 for a in im.getchannel("A").getdata() if a>100)
print("CANOPY_PIXELS",opaque,"ASSET_BYTES",out.stat().st_size)
