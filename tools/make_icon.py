"""Gera icon.ico (varios tamanhos) a partir de icon.png, usado por src/piu.rc (icone do piu.exe e da janela SDL)."""
import os
from PIL import Image
root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
im = Image.open(os.path.join(root, 'icon.png')).convert('RGBA')
side = max(im.size)
sq = Image.new('RGBA', (side, side), (0, 0, 0, 0)); sq.paste(im, ((side - im.width) // 2, (side - im.height) // 2))
sq.save(os.path.join(root, 'icon.ico'), sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
print('icon.ico gerado')
