import struct,sys,os
src=sys.argv[1]; out=sys.argv[2]; os.makedirs(out,exist_ok=True)
d=open(src,'rb').read()
le=struct.unpack_from('<I',d,0x3c)[0]
assert d[le:le+2]==b'LE'
def u32(o):return struct.unpack_from('<I',d,le+o)[0]
def u16(o):return struct.unpack_from('<H',d,le+o)[0]
flags=u32(0x10);npages=u32(0x14);eip_obj=u32(0x18);eip=u32(0x1c);esp_obj=u32(0x20);esp=u32(0x24)
pgsz=u32(0x28);lastpg=u32(0x2c);fixsz=u32(0x30);objtab=u32(0x40);nobj=u32(0x44);pgmap=u32(0x48)
fixpgtab=u32(0x68);fixrec=u32(0x6c);impmod=u32(0x70);nmod=u32(0x74);impproc=u32(0x78);datapg=u32(0x80)
bias=0
# DOS/32A-style: data pages offset is relative to file start (+bias of MZ stub)
print(f"flags={flags:#x} npages={npages} pgsz={pgsz} lastpg={lastpg} nobj={nobj} entry=obj{eip_obj}+{eip:#x} stack=obj{esp_obj}+{esp:#x} datapg={datapg:#x}")
objs=[]
for i in range(nobj):
    vs,base,fl,pi,np_,_=struct.unpack_from('<IIIIII',d,le+objtab+i*24)
    objs.append((vs,base,fl,pi,np_)); print(f"obj{i+1}: size={vs:#x} base={base:#x} flags={fl:#x} pageidx={pi} pages={np_}")
def pgoff(p): # 1-based
    return datapg+(p-1)*pgsz
buf=bytearray()
for i,(vs,base,fl,pi,np_) in enumerate(objs):
    data=bytearray()
    for p in range(pi,pi+np_):
        e=pgoff(p); data+=d[e:e+pgsz]
    data=data[:vs]+bytes(max(0,vs-len(data)))
    open(f"{out}/obj{i+1}_{base:08x}.bin",'wb').write(data)
# fixup page table
fp=[struct.unpack_from('<I',d,le+fixpgtab+4*i)[0] for i in range(npages+1)]
print("fixup records bytes:",fp[-1], "first nonzero pages:",sum(1 for i in range(npages) if fp[i+1]>fp[i]))
# imports
mods=[];o=le+impmod
for i in range(nmod):
    n=d[o];mods.append(d[o+1:o+1+n].decode());o+=1+n
print("import modules:",mods)
