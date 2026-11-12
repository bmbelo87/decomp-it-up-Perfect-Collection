"""Extrai BGA/*.dat da build PC (pumpy.exe). Container 'RES\0' ver 3: magic, ver, datasize, count, 2 u32; dir (28 B/entrada) e dados com a mesma cifra XOR do Arcade."""
import sys,struct,zlib,os
def xor(b):
    o=bytearray(b);k=0xD2
    for i in range(len(o)): o[i]^=k;k=(k+0x95)&255
    return bytes(o)
def read_res(path):
    d=open(path,'rb').read()
    magic,ver,dsz,cnt,h4,h5=struct.unpack('<4sIIIII',d[:24])
    assert magic==b'RES\0',magic
    dirb=xor(d[24:24+28*cnt]);data=d[24+28*cnt:24+28*cnt+dsz]
    out=[]
    for i in range(cnt):
        n,sz,off,a=struct.unpack('<16sIII',dirb[i*28:i*28+28])
        p=xor(data[off:off+sz]);out.append((n.split(b'\0')[0].decode('latin1'),p,(zlib.adler32(p)&0xffffffff)==a))
    return ver,out
if __name__=='__main__':
    src,dst=sys.argv[1],sys.argv[2]; bad=0
    for f in sorted(os.listdir(src)):
        if not f.lower().endswith('.dat'): continue
        try: ver,o=read_res(os.path.join(src,f))
        except Exception as e: print(f,'ERRO',e); bad+=1; continue
        od=os.path.join(dst,os.path.splitext(f)[0]); os.makedirs(od,exist_ok=True)
        for n,p,ok in o:
            open(os.path.join(od,n),'wb').write(p)
            if not ok: print(f,n,'adler diferente'); bad+=1
        print(f,'ver',ver,len(o),'itens')
    print('problemas:',bad)
