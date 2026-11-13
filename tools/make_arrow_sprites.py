"""Extrai as 5 setas do Arcade (BGA/00.DAT -> ARROW01.PNG / ARROW03.PNG) para docs/img/arrows/{UL,UR,C,DL,DR}.png.
Recorte 54x54 do quadro de animacao em que a seta esta toda colorida: UL/UR = ARROW01 (x 8, y 78); DL/DR = ARROW01 (x 148, y 148, azul cheio);
centro = ARROW03 (x 8, y 148, pes laranja cheios). Giros como em draw_arrow() de src/game.c. UR espelha em x, DL espelha em y, DR espelha em x e y. Uso: python tools/make_arrow_sprites.py <pasta do jogo> [saida]"""
import sys, io, os
sys.path.insert(0, os.path.dirname(__file__))
from piu_res import read_res
from PIL import Image
game = sys.argv[1]; out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(__file__), '..', 'docs', 'img', 'arrows')
os.makedirs(out, exist_ok=True)
files = {n: p for n, p, _ in read_res(os.path.join(game, 'BGA', '00.DAT'))[0]}
def crop(name, y, x=8): return Image.open(io.BytesIO(files[name])).convert('RGBA').crop((x, y, x + 54, y + 54))
FX, FY = Image.FLIP_LEFT_RIGHT, Image.FLIP_TOP_BOTTOM
a_red, a_blue, a_c = crop('ARROW01.PNG', 78), crop('ARROW01.PNG', 148, 148), crop('ARROW03.PNG', 148)      # quadros com todas as cores preenchidas
sp = {'UL': a_red, 'UR': a_red.transpose(FX), 'C': a_c, 'DL': a_blue.transpose(FY), 'DR': a_blue.transpose(FX).transpose(FY)}
for k, im in sp.items(): im.save(os.path.join(out, k + '.png'))
print('ok', list(sp))
