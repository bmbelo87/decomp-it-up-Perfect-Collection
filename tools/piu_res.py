"""Container BGA/*.DAT ("RES\0") do PIU.EXE (res_load 0x29A45).
Header 0x18: magic 'RES\0', ver u32, count u32, datasize u32, u32 (?), adler32 dos dados (cifrados).
Diretorio: count * 28 bytes, cifra XOR(0xD2,+0x95) (0x29949): name[16], size u32, offset u32, adler32 u32.
Dados: datasize bytes; cada arquivo em data[offset:offset+size], mesma cifra XOR, adler32 do plaintext."""
import sys,struct,zlib,os
def xor(b):
    o=bytearray(b);k=0xD2
    for i in range(len(o)): o[i]^=k;k=(k+0x95)&255
    return bytes(o)
def read_res(path):
    d=open(path,'rb').read()
    magic,ver,cnt,dsz,_,adl=struct.unpack('<4sIIIII',d[:24])
    assert magic==b'RES\0',magic
    dirb=xor(d[24:24+28*cnt]);data=d[24+28*cnt:24+28*cnt+dsz]
    ok=(zlib.adler32(data)&0xffffffff)==adl  # 0x29B1C: adler sobre data como lido (cifrado)
    out=[]
    for i in range(cnt):
        n,sz,off,a=struct.unpack('<16sIII',dirb[i*28:i*28+28])
        p=xor(data[off:off+sz]);out.append((n.split(b'\0')[0].decode('latin1'),p,(zlib.adler32(p)&0xffffffff)==a))
    return out,ok
if __name__=='__main__':
    for f in sys.argv[1:]:
        o,ok=read_res(f);print(f,len(o),'data_adler',ok,[ (n,len(p),g) for n,p,g in o][:4])
