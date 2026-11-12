import sys,os,zlib,io
try:
    from PIL import Image, ImageChops
except ImportError:
    Image=None
sys.path.insert(0,'tools')
import piu_res, piu_res_pc
A="E:/06_pc/game/BGA"; P="E:/Pump-it-Up-Perfect-Collection_Win_EN_Disc-Image/PUMPY_0416/Bga"
da={f.upper():f for f in os.listdir(A) if f.lower().endswith('.dat')}
dp={f.upper():f for f in os.listdir(P) if f.lower().endswith('.dat')}
print("DAT so no ARCADE (ausentes no PC):",sorted(set(da)-set(dp)))
print("DAT so no PC (ausentes no Arcade):",sorted(set(dp)-set(da)))
same=[];diff=[]
rep=[]
for k in sorted(set(da)&set(dp)):
    ea={n.upper():p for n,p,_ in piu_res.read_res(os.path.join(A,da[k]))[0]}
    ep={n.upper():p for n,p,_ in piu_res_pc.read_res(os.path.join(P,dp[k]))[1]}
    onlya=sorted(set(ea)-set(ep)); onlyp=sorted(set(ep)-set(ea))
    chg=[];recomp=[]
    for n in sorted(set(ea)&set(ep)):
        if ea[n]==ep[n]: continue
        if n.endswith('.PNG') and Image:
            try:
                ia=Image.open(io.BytesIO(ea[n])).convert('RGBA'); ib=Image.open(io.BytesIO(ep[n])).convert('RGBA')
                if ia.size==ib.size:
                    mx=max(x[1] for x in ImageChops.difference(ia,ib).getextrema())
                    if mx<=8: recomp.append(n); continue          # so ruido/quantizacao (PC reduz a profundidade de cor)
                    n=n+f' [dif max {mx}]'
                n=n+('' if ia.size==ib.size else f' [{ia.size[0]}x{ia.size[1]}->{ib.size[0]}x{ib.size[1]}]')
            except Exception: pass
        elif n.endswith(('.SPR','.SP2','.BGA')) and n[-4:]!='.BGA' and ea[n].split()==ep[n].split(): recomp.append(n); continue
        chg.append(n)
    if onlya or onlyp or chg: rep.append((k,len(ea),len(ep),onlya,onlyp,chg,len(recomp)))
    else: same.append(k)
print("DATs com conteudo identico:",len(same),same)
print("DATs com diferenca:",len(rep))
for k,na,np_,oa,op,c,rc in rep:
    print(f"\n{k}: arcade {na} itens, PC {np_} itens | so arcade {len(oa)} | so PC {len(op)} | conteudo diferente {len(c)} | so ruido<=8/recompressao {rc}")
    if oa: print("  so Arcade:",", ".join(oa))
    if op: print("  so PC:",", ".join(op))
    if c: print("  diferentes:",", ".join(c),"")
