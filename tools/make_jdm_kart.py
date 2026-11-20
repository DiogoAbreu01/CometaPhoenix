from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import numpy as np
p=Path(__file__).resolve().parents[1]
old=(p/"game/assets/kart_sprite.rgba").read_bytes()
im=Image.frombytes("RGBA",(512,512),old)
a=np.array(im,dtype=np.float32)
R,G,B,A=(a[:,:,i] for i in range(4))
# Metallic turquoise fairings turn into JDM sakura crimson, with readable bright highlights.
mask=(A>12)&(G>R*1.20)&(B>R*1.18)&(G>30)&(B>29)&(G<B*1.31)&(B<G*1.31)
r=np.clip(G*.82+87,0,245)
g=np.clip(G*.41+23,0,174)
b=np.clip(B*.50+37,0,194)
a[:,:,0]=np.where(mask,r,R)
a[:,:,1]=np.where(mask,g,G)
a[:,:,2]=np.where(mask,b,B)
im=Image.fromarray(np.uint8(a),"RGBA")
d=ImageDraw.Draw(im)
# Small lacquer-red sun insignia and a racing number on the white nose (right-facing kart).
d.ellipse((402,231,432,261),fill=(203,67,100,205))
d.ellipse((410,239,424,253),fill=(254,225,205,235))
font=ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf",31)
d.text((338,258),"86",font=font,fill=(87,39,59,218),stroke_width=1,stroke_fill=(252,225,220,175))
asset=p/"game/assets/kart_jdm_sprite.rgba"
asset.write_bytes(im.tobytes())
im.save(p/"docs/screenshots/kart_jdm_livery.png")
print("JDM_KART_SKIN",asset.stat().st_size,"changed_pixels",int(mask.sum()))
