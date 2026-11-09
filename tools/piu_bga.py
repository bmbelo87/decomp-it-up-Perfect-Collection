"""Parser de .BGA (script de layers), .SPR/.SP2 (texto) do PIU.EXE.
BGA (loader 0x2628F, render 0x25FA5 / 0x2643C):
  hdr 16: 'BGA\0', u32 a(=BPM? @obj+4), u32 b(@obj+8), u32 c(@obj+0)
  50 layers: name[64] (sprite .spr), u32 N, N * key(44):
    s16 time(frame), s16 active(0 => fim), f32 x, f32 y, f32 pivx, f32 pivy, f32 scale, f32 rot, f32 r,g,b,a
  render: interpola linear entre keys i-1 e i por t=(f-t0)/(t1-t0); antes da 1a key nao desenha.
"""
import struct,sys,glob
sys.path.insert(0,'tools')
def parse_bga(b):
    assert b[:4]==b'BGA\0'
    a,bb,c=struct.unpack('<III',b[4:16]);o=16;layers=[]
    for k in range(50):
        if o>=len(b):break  # arquivo pode ter < 50 layers (fread curto, resto zero)
        name=b[o:o+64].split(b'\0')[0].decode('latin1');n,=struct.unpack('<I',b[o+64:o+68]);o+=68
        keys=[struct.unpack('<hhffffffffff',b[o+44*i:o+44*i+44]) for i in range(n)];o+=44*n
        layers.append((name,keys))
    return (a,bb,c),layers,o==len(b)
def parse_spr(b):
    L=[l.strip() for l in b.decode('latin1').replace('\r','').split('\n') if l.strip()]
    t=L[0].split();typ=t[1].upper();args=[int(x) for x in t[2:] if x.lstrip('-').isdigit()]
    num=int(L[1].split()[-1]);ents=[]  # 2 arquivos omitem a palavra NUM
    for l in L[2:2+num]:
        p=l.split()
        if len(p)==11:ents.append((p[2],*[int(x) for x in p[3:11]],p[1]))  # SP2: T tag file x y w h u v w h (src provavelmente u,v,w,h)
        else:ents.append((p[1],*[int(x) for x in p[2:10]]))  # SPR: T file x y w h u0 v0 u1 v1
    return typ,args,ents
if __name__=='__main__':
    from piu_res import read_res
    ok=bad=0
    for f in glob.glob('../game/BGA/*.DAT'):
        for n,p,g in read_res(f)[0]:
            if n.upper().endswith('.BGA'):
                h,l,full=parse_bga(p);ok+=full;bad+=not full
            elif n.upper().endswith(('.SPR','.SP2')):parse_spr(p)
    print('bga ok',ok,'bad',bad)
