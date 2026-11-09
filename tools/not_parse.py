# Parser de STEP/*.NOT (PIU.EXE DOS build Dec 18 2000). Layout inferido do loader 0x21361 / decoder 0x29949 / expand 0x21251.
import struct,sys,glob,os,collections
HDR=0x88
def decode(b):
    out=bytearray(b);k=0xD2
    for i in range(len(out)): out[i]^=k;k=(k+0x95)&0xFF
    return out
def parse(path):
    d=open(path,'rb').read()
    cnt=struct.unpack_from('<I',d,0)[0]
    bpm=struct.unpack_from('<f',d,8)[0]
    title=d[0x44:0x88].split(b'\0')[0].decode('latin1')
    body=decode(d[HDR:])
    recs=[struct.unpack_from('<HH',body,i*4) for i in range(len(body)//4)]
    return dict(count=cnt,size_ok=(len(d)==HDR+4*cnt),bpm=bpm,hdr=d[:HDR],title=title,recs=recs)
if __name__=='__main__':
    ok=bad=0;maxrow=0;masks=collections.Counter();mono=0;badnames=[]
    for f in sorted(glob.glob('../game/STEP/*.NOT')+glob.glob('../game/STEP/*.not')):
        p=parse(f)
        if p['size_ok']:ok+=1
        else: bad+=1;badnames.append(os.path.basename(f))
        rows=[r for r,m in p['recs']]
        if rows==sorted(rows):mono+=1
        maxrow=max(maxrow,max(rows,default=0))
        for r,m in p['recs']: masks[m&0x7]+=1
    print('files ok/bad',ok,bad,'rows monotonic',mono,'max row',maxrow,'(table size 0x800=2048)')
    print('bad:',badnames[:10])
    p=parse('../game/STEP/101_DB.NOT'); print(p['title'],p['bpm'],p['count']); print([(r,f'{m:016b}') for r,m in p['recs'][:12]])
