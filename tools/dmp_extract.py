"""Le um minidump do Gerenciador de Tarefas e reconstroi o PE de um modulo a partir da memoria. uso: dmp_extract.py arq.DMP modulo.exe saida.exe"""
import struct,sys
def parse(path):
    d=open(path,'rb').read()
    sig,ver,ns,rva=struct.unpack('<4sIII',d[:16]); assert sig==b'MDMP'
    st={}
    for i in range(ns):
        t,sz,r=struct.unpack('<III',d[rva+12*i:rva+12*i+12]); st.setdefault(t,[]).append((sz,r))
    return d,st
def modules(d,st):
    sz,r=st[4][0]; n,=struct.unpack('<I',d[r:r+4]); out=[]
    for i in range(n):
        base,size,chk,ts,nr=struct.unpack('<QIIII',d[r+4+108*i:r+4+108*i+24])
        l,=struct.unpack('<I',d[nr:nr+4]); out.append((d[nr+4:nr+4+l].decode('utf-16le'),base,size))
    return out
def ranges(d,st):
    out=[]
    if 9 in st:
        sz,r=st[9][0]; n,base=struct.unpack('<QQ',d[r:r+16]); o=base
        for i in range(n):
            a,s=struct.unpack('<QQ',d[r+16+16*i:r+32+16*i]); out.append((a,o,s)); o+=s
    if 5 in st:
        sz,r=st[5][0]; n,=struct.unpack('<I',d[r:r+4])
        for i in range(n):
            a,s,rv=struct.unpack('<QII',d[r+4+16*i:r+20+16*i]); out.append((a,rv,s))
    return out
def read(d,rg,addr,n):
    b=bytearray(n)
    for a,o,s in rg:
        lo=max(a,addr);hi=min(a+s,addr+n)
        if lo<hi: b[lo-addr:hi-addr]=d[o+lo-a:o+hi-a]
    return bytes(b)
if __name__=='__main__':
    d,st=parse(sys.argv[1]); print('streams',sorted(st))
    ms=modules(d,st)
    for m in ms: print(hex(m[1]),hex(m[2]),m[0])
    rg=ranges(d,st); print('ranges',len(rg),'bytes',sum(r[2] for r in rg))
    if len(sys.argv)>3:
        name=sys.argv[2].lower(); m=[x for x in ms if x[0].lower().endswith(name)][0]
        img=bytearray(read(d,rg,m[1],m[2]))
        pe=struct.unpack('<I',img[0x3c:0x40])[0]
        n=struct.unpack('<H',img[pe+6:pe+8])[0]; so=struct.unpack('<H',img[pe+20:pe+22])[0]
        o=pe+24+so
        for i in range(n):   # raw = virtual (imagem ja mapeada)
            vs,va=struct.unpack('<II',img[o+i*40+8:o+i*40+16])
            struct.pack_into('<II',img,o+i*40+16,(vs+0xfff)&~0xfff,va)
        open(sys.argv[3],'wb').write(img); print('gravado',sys.argv[3],len(img))
