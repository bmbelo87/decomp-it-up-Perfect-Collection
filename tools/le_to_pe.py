# Wrap LE objects (DOS4GW/Watcom) into a synthetic PE32 so tools that only accept PE can load them.
# VA preserved: ImageBase=0x10000, section RVA = LEbase-0x10000. obj1 (16 bytes @0x10000) overlaps headers; dropped.
import struct,glob
IB=0x10000;SA=0x1000;FA=0x200
ENTRY_VA=0x20000+0xdc004
secs=[]
for f in sorted(glob.glob('objs_fixed/obj*_*.bin')):
    import os; n=int(os.path.basename(f)[3]); b=int(f.split('_')[-1][:8],16)
    if n==1: continue
    d=open(f,'rb').read()
    flags={2:0x60000020|0x40000000|0x80000000,3:0xC0000040,4:0xC0000040}[n]  # obj2 code RWX (self-modifying risk), others data RW
    secs.append((f'.o{n}',b-IB,len(d),d.rstrip(b'\0') or b'\0',flags))
hdr_size=0x400
off=hdr_size;out=[]
for name,rva,vs,raw,fl in secs:
    rs=(len(raw)+FA-1)//FA*FA; out.append((name,rva,vs,off,rs,raw,fl)); off+=rs
img_size=max(r+(v+SA-1)//SA*SA for _,r,v,*_ in out)
dos=bytearray(0x80);dos[0:2]=b'MZ';struct.pack_into('<I',dos,0x3c,0x80)
pe=b'PE\0\0'+struct.pack('<HHIIIHH',0x14c,len(out),0,0,0,0xE0,0x0103)
opt=struct.pack('<HBBIIIIIIIIIHHHHHHIIIIHHIIIIII',0x10b,6,0,out[0][4],0,0,ENTRY_VA-IB,out[0][1],out[-1][1],IB,SA,FA,4,0,0,0,4,0,0,img_size,hdr_size,0,3,0,0x100000,0x1000,0x100000,0x1000,0,16)
opt+=bytes(16*8)
sh=b''
for name,rva,vs,fo,rs,raw,fl in out:
    sh+=struct.pack('<8sIIIIIIHHI',name.encode(),vs,rva,rs,fo,0,0,0,0,fl)
hdr=bytes(dos)+pe+opt+sh;hdr+=bytes(hdr_size-len(hdr))
with open('piu_le.exe','wb') as f:
    f.write(hdr)
    for name,rva,vs,fo,rs,raw,fl in out: f.write(raw+bytes(rs-len(raw)))
print('ok',[(s[0],hex(s[1]+IB),hex(s[2])) for s in out],hex(ENTRY_VA))
