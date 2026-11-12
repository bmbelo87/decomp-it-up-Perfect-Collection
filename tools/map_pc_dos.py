"""Tabela de correspondencia PC (pumpy.exe desempacotado) -> DOS (PIU.EXE) via strings em comum.
Para cada funcao do PC que referencia uma string, acha a mesma string no ELF Linux (R5/v3.52, mesmos enderecos func_XXXXX do DOS) e as funcoes que a usam."""
import re,bisect,subprocess,struct,sys
S="C:/Users/bruno/AppData/Local/Temp/claude/4476bbab4032e5bd/ab79d3c0-b0c2-4f1f-b0ce-8f84be14a0b1/scratchpad/"
pc=open('E:/06_pc/pumpy_unpacked.exe','rb').read()
elf=open('E:/06_pc/game/piu','rb').read()
# ELF: vaddr<->offset (2 LOADs)
loads=[(0x08048000,0,0xca9ec),(0x08113000,0xcb000,0x90877a)]
def v2o(v):
    for va,off,sz in loads:
        if va<=v<va+sz: return off+v-va
def o2v(o):
    for va,off,sz in loads:
        if off<=o<off+sz: return va+o-off
# funcoes do ELF
nm=subprocess.run(['nm','E:/06_pc/game/piu'],capture_output=True,text=True).stdout
fs=sorted((int(l.split()[0],16),l.split()[2]) for l in nm.split('\n') if len(l.split())==3 and l.split()[1] in 'tT' and l.split()[2].startswith('func_'))
fa=[a for a,_ in fs]
def efunc(v):
    i=bisect.bisect_right(fa,v)-1; return fs[i][1] if i>=0 else '?'
# refs a enderecos no asm do ELF
asm=open(S+'elf.asm').read().split('\n')
cur=None; erefs={}
for l in asm:
    m=re.match(r'([0-9a-f]+) <(.+)>:',l)
    if m: cur=m.group(2); continue
    for m in re.finditer(r'0x(8[0-9a-f]{6})\b',l):
        erefs.setdefault(int(m.group(1),16),set()).add(cur)
# strings do ELF: texto -> vaddr
def find_all(blob,s):
    out=[];i=-1
    b=b'\0'+s.encode()+b'\0'
    while True:
        i=blob.find(b,i+1)
        if i<0: break
        out.append(i+1)
    return out
rows=[]
for line in open('scratch/pc_func_strings.txt'):
    a,_,rest=line.rstrip('\n').partition(': ')
    if re.search(r'libpng|zlib|chunk|Bad |Mesa|runtime|heap|_onexit|stdio|lowio|IHDR|PLTE|IDAT|tEXt|PNG|compress|Decompress|NULL|row|pHYs|bit length|distance|Out of Memory|CRC',rest): continue
    hits={}
    for s in rest.split(' | '):
        if len(s)<5: continue
        for o in find_all(elf,s):
            v=o2v(o)
            for f in erefs.get(v,()): hits.setdefault(f,[]).append(s)
    rows.append((a,rest,hits))
with open('scratch/map_pc_dos.txt','w') as fo:
    for a,rest,hits in rows:
        h='; '.join(f'{f} [{",".join(x[:18] for x in v[:3])}]' for f,v in sorted(hits.items(),key=lambda kv:-len(kv[1]))[:4])
        fo.write(f'PC {a}: {rest[:90]}\n     -> ELF/DOS: {h or "(nenhuma - novo/diferente)"}\n')
print(len(rows),sum(1 for r in rows if r[2]))
