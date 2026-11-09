"""Decifra os arquivos .AUD/.PNZ/.DAT(?) do PIU.EXE (loader 0x289B0).
Formato: u32 H | dados. p[i] = bitrev8(c[i]) ^ KEY[(H+i)&0x3FF]; H == adler32(p) (0x298D1).
KEY = 1024 bytes em 0x12B13C (obj4)."""
import sys,zlib,struct,os
here=os.path.dirname(os.path.abspath(__file__))
KEY=open(os.path.join(here,'../objs_fixed/obj4_00120000.bin'),'rb').read()[0x12b13c-0x120000:][:1024]
REV=bytes(int('{:08b}'.format(i)[::-1],2) for i in range(256))
def unpack(b):
    h=struct.unpack('<I',b[:4])[0];c=b[4:]
    p=bytes(REV[x]^KEY[(h+i)&0x3ff] for i,x in enumerate(c))
    return p,h,zlib.adler32(p)&0xffffffff
if __name__=='__main__':
    for f in sys.argv[1:]:
        p,h,a=unpack(open(f,'rb').read())
        print(f,len(p),'hdr=%08x adler=%08x %s'%(h,a,'OK' if h==a else 'BAD'),p[:16].hex(' '))
