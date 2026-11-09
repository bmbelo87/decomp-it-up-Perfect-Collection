# Apply LE fixup records to dumped objects -> objs_fixed/ (flat VA-linked)
import struct,sys,os,collections
src='../game/PIU.EXE'; d=open(src,'rb').read()
le=struct.unpack_from('<I',d,0x3c)[0]
u32=lambda o:struct.unpack_from('<I',d,le+o)[0]
npages=u32(0x14);pgsz=u32(0x28);objtab=u32(0x40);nobj=u32(0x44);fpt=u32(0x68);frec=u32(0x6c);datapg=u32(0x80)
objs=[struct.unpack_from('<IIIIII',d,le+objtab+i*24) for i in range(nobj)]
data=[bytearray(open(f'objs/obj{i+1}_{o[1]:08x}.bin','rb').read()) for i,o in enumerate(objs)]
def page_obj(p):
    for i,o in enumerate(objs):
        if o[3]<=p<o[3]+o[4]: return i,(p-o[3])*pgsz
fp=[u32(fpt-0x0+4*i)-0 for i in range(npages+1)]
stat=collections.Counter();bad=0
for p in range(1,npages+1):
    s,e=fp[p-1],fp[p]; o=le+frec+s; end=le+frec+e
    oi,poff=page_obj(p)
    while o<end:
        st,fl=d[o],d[o+1]; o+=2
        if st&0x20: cnt=d[o]; o+=1; srcs=None
        else: so=struct.unpack_from('<h',d,o)[0]; o+=2; srcs=[so]
        tt=fl&3
        if tt!=0: raise SystemExit(f'unsupported target type {tt} at page {p}')
        if fl&0x40: tobj=struct.unpack_from('<H',d,o)[0]; o+=2
        else: tobj=d[o]; o+=1
        if (st&0xF)!=2:
            if fl&0x10: toff=struct.unpack_from('<I',d,o)[0]; o+=4
            else: toff=struct.unpack_from('<H',d,o)[0]; o+=2
        else: toff=0
        add=0
        if fl&4:
            if fl&0x20: add=struct.unpack_from('<I',d,o)[0]; o+=4
            else: add=struct.unpack_from('<H',d,o)[0]; o+=2
        if srcs is None:
            srcs=list(struct.unpack_from(f'<{cnt}h',d,o)); o+=2*cnt
        tgt=objs[tobj-1][1]+toff+add
        for so in srcs:
            a=poff+so  # offset within this object
            k=st&0xF
            stat[k]+=1
            if a<0: bad+=1; continue   # spills into previous page's object: handled when src page differs
            if k==7: struct.pack_into('<I',data[oi],a,tgt&0xFFFFFFFF)
            elif k==8: struct.pack_into('<i',data[oi],a,tgt-(objs[oi][1]+a+4))
            elif k==5: struct.pack_into('<H',data[oi],a,tgt&0xFFFF)
            elif k==6: struct.pack_into('<IH',data[oi],a,tgt,0x10)
            elif k==2: struct.pack_into('<H',data[oi],a,tobj)
            else: bad+=1
print('fixup src types:',dict(stat),'skipped:',bad)
os.makedirs('objs_fixed',exist_ok=True)
for i,o in enumerate(objs): open(f'objs_fixed/obj{i+1}_{o[1]:08x}.bin','wb').write(data[i])
