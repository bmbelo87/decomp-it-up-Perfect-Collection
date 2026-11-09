import glob,sys
base0=0x10000
img=bytearray()
for f in sorted(glob.glob('objs/obj*_*.bin')):
    b=int(f.split('_')[-1][:8],16); data=open(f,'rb').read()
    off=b-base0
    if len(img)<off: img+=bytes(off-len(img))
    img[off:off+len(data)]=data
open('piu_flat_0x10000.bin','wb').write(img); print(hex(len(img)))
