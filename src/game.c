/* PiuReconstructed - jogo jogavel (SDL2 + OpenGL 1.x imediato, como o original Glide/Mesa).
 * Logica de tempo/julgamento: piu_play.c (confirmada no assembly). Telas de menu/resultado
 * sao reconstrucao funcional (o original nao foi totalmente decompilado nessas partes). */
#define SDL_MAIN_HANDLED
#include <SDL.h>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>
#ifndef _WIN32
#define strtok_s strtok_r      /* MSVC/C11 anexo K -> POSIX */
#endif
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#include "piu.h"
#include "font8x8_basic.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

static char g_dir[260] = "../game";
static SDL_Window *g_win;
static int g_winw = 800, g_winh = 600;

static void logf_(const char *f, ...) { va_list a; va_start(a, f); vfprintf(stderr, f, a); va_end(a); }

/* ------------------------------------------------------------------ util */
static uint8_t *slurp(const char *path, size_t *n)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = (uint8_t *)malloc(sz ? sz : 1);
    if (fread(b, 1, sz, f) != (size_t)sz) { free(b); fclose(f); return NULL; }
    fclose(f); *n = (size_t)sz; return b;
}
static int ieq(const char *a, const char *b)
{
    for (; *a && *b; a++, b++) {
        char x = *a, y = *b;
        if (x >= 'A' && x <= 'Z') x += 32;
        if (y >= 'A' && y <= 'Z') y += 32;
        if (x != y) return 0;
    }
    return *a == *b;
}
static int has_ext(const char *n, const char *e)
{
    size_t l = strlen(n), k = strlen(e);
    return l >= k && ieq(n + l - k, e);
}
static void path_of(char *out, size_t sz, const char *fmt, ...)
{
    char t[300]; va_list a; va_start(a, fmt); vsnprintf(t, sizeof t, fmt, a); va_end(a);
    snprintf(out, sz, "%s/%s", g_dir, t);
}

/* ------------------------------------------------------------------ PNG -> GL */
static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static int paeth(int a, int b, int c)
{
    int p = a + b - c, pa = abs(p - a), pb = abs(p - b), pc = abs(p - c);
    return (pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c);
}
static uint8_t *png_decode(const uint8_t *f, size_t sz, int *W, int *H)
{
    static const uint8_t sig[8] = { 0x89, 'P', 'N', 'G', 13, 10, 26, 10 };
    if (sz < 33 || memcmp(f, sig, 8)) return NULL;
    uint32_t w = 0, h = 0; int depth = 0, ct = 0; uint8_t *pal = NULL; size_t pall = 0, trl = 0; uint8_t *trns = NULL;
    size_t idat = 0, off = 8;
    while (off + 12 <= sz) {
        uint32_t len = be32(f + off); const uint8_t *t = f + off + 4; size_t d = off + 8;
        if (d + len > sz) break;
        if (!memcmp(t, "IHDR", 4)) { w = be32(f + d); h = be32(f + d + 4); depth = f[d + 8]; ct = f[d + 9]; }
        else if (!memcmp(t, "PLTE", 4)) { pal = (uint8_t *)malloc(len); memcpy(pal, f + d, len); pall = len; }
        else if (!memcmp(t, "tRNS", 4)) { trns = (uint8_t *)malloc(len); memcpy(trns, f + d, len); trl = len; }
        else if (!memcmp(t, "IDAT", 4)) idat += len;
        else if (!memcmp(t, "IEND", 4)) break;
        off = d + len + 4;
    }
    int ch = ct == 0 ? 1 : ct == 2 ? 3 : ct == 3 ? 1 : ct == 4 ? 2 : ct == 6 ? 4 : 0;
    if (!w || !h || !idat || !ch || depth != 8) { free(pal); free(trns); return NULL; }
    uint8_t *comp = (uint8_t *)malloc(idat); size_t ci = 0; off = 8;
    while (off + 12 <= sz) {
        uint32_t len = be32(f + off); const uint8_t *t = f + off + 4; size_t d = off + 8;
        if (!memcmp(t, "IDAT", 4)) { memcpy(comp + ci, f + d, len); ci += len; }
        if (!memcmp(t, "IEND", 4)) break;
        off = d + len + 4;
    }
    uLongf rawsz = (uLongf)(((size_t)w * ch + 1) * h);
    uint8_t *raw = (uint8_t *)malloc(rawsz);
    if (uncompress(raw, &rawsz, comp, (uLong)idat) != Z_OK) { free(comp); free(raw); free(pal); free(trns); return NULL; }
    free(comp);
    size_t rs = (size_t)w * ch;
    uint8_t *out = (uint8_t *)malloc((size_t)w * h * 4), *cur = (uint8_t *)malloc(rs), *prev = (uint8_t *)calloc(1, rs);
    const uint8_t *src = raw;
    for (uint32_t y = 0; y < h; y++) {
        int ft = *src++;
        for (size_t i = 0; i < rs; i++) {
            int L = i >= (size_t)ch ? cur[i - ch] : 0, U = prev[i], LU = i >= (size_t)ch ? prev[i - ch] : 0, v;
            switch (ft) {
            case 1: v = src[i] + L; break; case 2: v = src[i] + U; break;
            case 3: v = src[i] + ((L + U) >> 1); break; case 4: v = src[i] + paeth(L, U, LU); break;
            default: v = src[i];
            }
            cur[i] = (uint8_t)v;
        }
        uint8_t *dst = out + (size_t)y * w * 4;
        for (uint32_t x = 0; x < w; x++) {
            const uint8_t *s = cur + (size_t)x * ch; uint8_t r, g, b, a = 255;
            switch (ct) {
            case 0: r = g = b = s[0]; break;
            case 2: r = s[0]; g = s[1]; b = s[2]; break;
            case 3: if ((size_t)s[0] * 3 + 2 < pall) { r = pal[s[0] * 3]; g = pal[s[0] * 3 + 1]; b = pal[s[0] * 3 + 2]; } else r = g = b = 0;
                    if (trns && s[0] < trl) a = trns[s[0]]; break;
            case 4: r = g = b = s[0]; a = s[1]; break;
            default: r = s[0]; g = s[1]; b = s[2]; a = s[3];
            }
            dst[x * 4] = r; dst[x * 4 + 1] = g; dst[x * 4 + 2] = b; dst[x * 4 + 3] = a;
        }
        memcpy(prev, cur, rs); src += rs;
    }
    free(cur); free(prev); free(raw); free(pal); free(trns);
    *W = (int)w; *H = (int)h; return out;
}
typedef struct { GLuint id; int w, h; } Tex;
static Tex tex_make(const uint8_t *rgba, int w, int h)
{
    Tex t = { 0, w, h };
    glGenTextures(1, &t.id); glBindTexture(GL_TEXTURE_2D, t.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    return t;
}
static Tex tex_from_png(const uint8_t *b, size_t n)
{
    int w, h; uint8_t *p = png_decode(b, n, &w, &h); Tex t = { 0, 0, 0 };
    if (p) { t = tex_make(p, w, h); free(p); }
    return t;
}

/* ------------------------------------------------------------------ desenho */
static void quad(const Tex *t, float x, float y, float w, float h, float u0, float v0, float u1, float v1)
{
    if (t) { glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, t->id); } else glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v0); glVertex2f(x, y);
    glTexCoord2f(u1, v0); glVertex2f(x + w, y);
    glTexCoord2f(u1, v1); glVertex2f(x + w, y + h);
    glTexCoord2f(u0, v1); glVertex2f(x, y + h);
    glEnd();
}
static Tex g_font;
static void font_init(void)
{
    uint8_t *px = (uint8_t *)calloc(128 * 64 * 4, 1);
    for (int c = 0; c < 128; c++) for (int r = 0; r < 8; r++) for (int b = 0; b < 8; b++) {
        int on = (font8x8_basic[c][r] >> b) & 1;
        int x = (c % 16) * 8 + b, y = (c / 16) * 8 + r; uint8_t *p = px + (y * 128 + x) * 4;
        p[0] = p[1] = p[2] = 255; p[3] = on ? 255 : 0;
    }
    g_font = tex_make(px, 128, 64); free(px);
    glBindTexture(GL_TEXTURE_2D, g_font.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
static void text(float x, float y, float sc, const char *s)
{
    for (; *s; s++, x += 8 * sc) {
        unsigned c = (unsigned char)*s; if (c > 127) c = '?';
        float u = (c % 16) / 16.0f, v = (c / 16) / 8.0f;
        quad(&g_font, x, y, 8 * sc, 8 * sc, u, v, u + 1 / 16.0f, v + 1 / 8.0f);
    }
}
static void textc(float cx, float y, float sc, const char *s) { text(cx - strlen(s) * 4 * sc, y, sc, s); }
static void textf(float x, float y, float sc, const char *f, ...)
{
    char b[256]; va_list a; va_start(a, f); vsnprintf(b, sizeof b, f, a); va_end(a); text(x, y, sc, b);
}

/* ------------------------------------------------------------------ RES + sprites */
typedef struct { char tag[10]; char file[28]; float x, y, w, h, u0, v0, u1, v1; } SprTile;
typedef struct { int type, a, n, pb, pc; SprTile *t; } Spr;      /* pattern a b c: a = ordem do indice (0 coluna-maior), pb = linhas, pc = colunas (FUN_2F4EF) */
typedef struct { PiuRes res; Tex *tex; char **texname; int ntex; } ResSet;

static int rs_open(ResSet *r, const char *path)
{
    memset(r, 0, sizeof *r);
    return piu_res_load(path, &r->res);
}
static void rs_close(ResSet *r)
{
    for (int i = 0; i < r->ntex; i++) { if (r->tex[i].id) glDeleteTextures(1, &r->tex[i].id); free(r->texname[i]); }
    free(r->tex); free(r->texname); piu_res_free(&r->res); memset(r, 0, sizeof *r);
}
/* "xxx.tga" -> entrada "XXX.PNG" do RES (textura 256x256 do jogo original) */
static Tex *rs_tex(ResSet *r, const char *name)
{
    char key[40]; snprintf(key, sizeof key, "%s", name);
    char *dot = strrchr(key, '.'); if (dot) *dot = 0;
    for (int i = 0; i < r->ntex; i++) if (ieq(r->texname[i], key)) return r->tex[i].id ? &r->tex[i] : NULL;
    char fn[48]; snprintf(fn, sizeof fn, "%s.PNG", key);
    const PiuResEntry *e = piu_res_find(&r->res, fn);
    Tex t = { 0, 0, 0 };
    if (e) t = tex_from_png(e->data, e->size);
    r->tex = (Tex *)realloc(r->tex, (r->ntex + 1) * sizeof(Tex));
    r->texname = (char **)realloc(r->texname, (r->ntex + 1) * sizeof(char *));
    r->tex[r->ntex] = t; r->texname[r->ntex] = strdup(key); r->ntex++;
    return t.id ? &r->tex[r->ntex - 1] : NULL;
}
static Spr *spr_parse(const uint8_t *b, size_t len)
{
    char *s = (char *)malloc(len + 1); memcpy(s, b, len); s[len] = 0;
    Spr *sp = (Spr *)calloc(1, sizeof *sp);
    char *save = NULL; int ln = 0; int sp2 = 0;
    int ntl = 0; for (size_t k = 1; k + 1 < len; k++) if (b[k] == 'T' && b[k + 1] == ' ' && (b[k - 1] == '\n')) ntl++;
    for (char *l = strtok_s(s, "\r\n", &save); l; l = strtok_s(NULL, "\r\n", &save)) {
        while (*l == ' ' || *l == '\t') l++;
        if (!*l) continue;
        if (ln == 0) {
            if (strstr(l, "ANI") || strstr(l, "ani")) sp->type = 2;
            else if (strstr(l, "PATTERN") || strstr(l, "pattern") || strstr(l, "pattren")) {
                sp->type = 3;
                const char *q = strstr(l, "PATTERN"); if (!q) q = strstr(l, "pattern"); if (!q) q = strstr(l, "pattren");
                long long na = 0; int nb = 0, nc = 0; if (sscanf(q + 7, "%lld %d %d", &na, &nb, &nc) == 3) { sp->a = na != 0; sp->pb = nb; sp->pc = nc; }
            }
            else sp->type = 0;
            sp->a = 0; ln++; continue;
        }
        if (ln == 1) { char *p = l; while (*p && (*p < '0' || *p > '9')) p++; sp->n = atoi(p);
            if (ntl > sp->n) sp->n = ntl;      /* NUM do ARROW542 diz 43 mas ha 45 tiles */
            sp->t = (SprTile *)calloc(sp->n + 1, sizeof(SprTile)); ln++; continue; }
        if (ln - 2 >= sp->n) break;
        char tok[11][48]; int nt = sscanf(l, "%47s %47s %47s %47s %47s %47s %47s %47s %47s %47s %47s",
            tok[0], tok[1], tok[2], tok[3], tok[4], tok[5], tok[6], tok[7], tok[8], tok[9], tok[10]);
        SprTile *t = &sp->t[ln - 2]; ln++;
        int o = 1;
        if (nt >= 11) { snprintf(t->tag, sizeof t->tag, "%s", tok[1]); o = 2; sp2 = 1; }
        snprintf(t->file, sizeof t->file, "%s", tok[o]);
        float v[8]; for (int i = 0; i < 8; i++) v[i] = (float)atoi(tok[o + 1 + i]);
        t->x = v[0]; t->y = v[1]; t->w = v[2]; t->h = v[3];
        if (nt >= 11) { t->u0 = v[4]; t->v0 = v[5]; t->u1 = v[4] + v[6]; t->v1 = v[5] + v[7]; }
        else { t->u0 = v[4]; t->v0 = v[5]; t->u1 = v[6]; t->v1 = v[7]; }
    }
    (void)sp2; free(s); return sp;
}
static void spr_free(Spr *s) { if (s) { free(s->t); free(s); } }
static const SprTile *sp2_find(const Spr *s, const char *tag)
{
    for (int i = 0; s && i < s->n; i++) if (!strncmp(s->t[i].tag, tag, 6)) return &s->t[i];
    return NULL;
}
static void draw_tile(ResSet *rs, const SprTile *t, float ox, float oy)
{
    Tex *tx = rs_tex(rs, t->file); if (!tx) return;
    if (!t->w && !t->h) { quad(tx, ox, oy, (float)tx->w, (float)tx->h, 0, 0, 1, 1); return; }   /* layer .tga puro */
    quad(tx, ox + t->x, oy + t->y, t->w, t->h, t->u0 / tx->w, t->v0 / tx->h, t->u1 / tx->w, t->v1 / tx->h);
}

/* ------------------------------------------------------------------ BGA */
typedef struct { PiuBga g; ResSet rs; Spr *spr[50]; int ok; } BgaRun;
static void bga_unload(BgaRun *b)
{
    if (!b->ok) return;
    for (int i = 0; i < 50; i++) spr_free(b->spr[i]);
    piu_bga_free(&b->g); rs_close(&b->rs); memset(b, 0, sizeof *b);
}
/* name = "81", "01", "082"...: abre BGA/<name>.DAT e a entrada <name>.BGA (0x2628F usa o mesmo nome) */
static int bga_load_name(BgaRun *b, const char *name)
{
    char p[300]; memset(b, 0, sizeof *b);
    path_of(p, sizeof p, "BGA/%s.DAT", name);
    if (rs_open(&b->rs, p)) return 0;
    char n[40]; snprintf(n, sizeof n, "%s.BGA", name);
    const PiuResEntry *e = piu_res_find(&b->rs.res, n);
    if (!e || piu_bga_parse(e->data, e->size, &b->g) < 0) { rs_close(&b->rs); return 0; }
    for (int i = 0; i < b->g.nlayers; i++) {
        const char *nm = b->g.layer[i].name; if (!*nm || !b->g.layer[i].n) continue;
        if (has_ext(nm, ".spr")) {
            const PiuResEntry *se = piu_res_find(&b->rs.res, nm);
            if (se) b->spr[i] = spr_parse(se->data, se->size);
        } else {   /* layer direto em .tga: textura desenhada em tamanho nativo (antes: esticada 640x480 - errado) */
            Spr *s = (Spr *)calloc(1, sizeof *s); s->n = 1; s->t = (SprTile *)calloc(2, sizeof(SprTile));
            snprintf(s->t[0].file, sizeof s->t[0].file, "%s", nm);
            /* w=h=0: tamanho nativo da textura (pivos das chaves = centro da textura: 128 p/ 256x256, 64 p/ 128x128, 2,64 p/ 8x128) */ b->spr[i] = s;
        }
    }
    b->ok = 1; return 1;
}
static int bga_load(BgaRun *b, int id) { char n[16]; snprintf(n, sizeof n, "%03d", id); return bga_load_name(b, n); }
static void bga_draw_n(BgaRun *b, int frame, int only)   /* only >= 0: so a camada `only` (FUN_0002647A); -1: todas (FUN_0002643C) */
{
    if (!b->ok) return;
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 0; i < b->g.nlayers; i++) {
        const PiuBgaLayer *l = &b->g.layer[i]; PiuBgaKey k;
        if (only >= 0 && i != only) continue;
        if (!b->spr[i] || piu_bga_eval(l, frame, &k) != 1) continue;
        float frac = 0;
        for (uint32_t j = 1; j < l->n; j++) if (l->k[j].time > frame) {
            int d = l->k[j].time - l->k[j - 1].time; frac = d ? (float)(frame - l->k[j - 1].time) / d : 0; break; }
        glPushMatrix();
        glTranslatef(k.x, k.y, 0); glTranslatef(k.px, k.py, 0); glRotatef(-k.rot, 0, 0, 1);
        glScalef(k.scale, k.scale, 1); glTranslatef(-k.px, -k.py, 0);
        glColor4f(k.r, k.g, k.b, k.a);
        const Spr *s = b->spr[i];
        if (s->type == 2 && s->n) { int f = (int)(frac * s->n); if (f >= s->n) f = s->n - 1; draw_tile(&b->rs, &s->t[f], 0, 0); }
        else if (s->type == 3 && s->n) {
            /* FUN_2EFAD tipo 3: grade pc colunas x pb linhas, passo = (x+w, y+h) do tile 0; indice = (a ? linha*pc+col : pb*col+linha + deslocamento) % n, deslocamento = (int)(n*(1-frac)) */
            const SprTile *t0 = &s->t[0]; float sx = t0->x + t0->w, sy = t0->y + t0->h; int sh = (int)(s->n * (1.0f - frac));
            for (int r = 0; r < s->pb || (!s->pb && r < 1); r++) for (int c = 0; c < s->pc || (!s->pc && c < 1); c++) {
                int idx = ((s->a ? r * s->pc + c : s->pb * c + r) + sh) % s->n; if (idx < 0) idx += s->n;
                draw_tile(&b->rs, &s->t[idx], c * sx, r * sy);
            }
        }
        else for (int t = s->n - 1; t >= 0; t--) draw_tile(&b->rs, &s->t[t], 0, 0);      /* FUN_2EFAD tipo 1: do ultimo tile ao primeiro */
        glPopMatrix();
    }
    glColor4f(1, 1, 1, 1);
}
static void bga_draw(BgaRun *b, int frame) { bga_draw_n(b, frame, -1); }

/* ------------------------------------------------------------------ audio */
/* ---- efeitos sonoros (SFX). No PIU.EXE original saem de um chip de som (FUN_00030fac); aqui usam os WAV de game/EFFECTS.
 * Ordem = tabela effects_file_names do ELF Linux (indice = voz de PIU.BIN, 35 entradas). */
static const char *const SFX_NAMES[35] = { "01-1","01-11","2-1","3-2","4-2","5-1","6-2","7-1","8-1","9-5","9-A","9-B","9-C","9-D","9-F","TIME_LIMIT",
    "10-2","13-1","12-1","14","START1","START2","START3","RANK_A","RANK_B","RANK_C","RANK_D","RANK_F","NOTGOOD1","NOTGOOD2","NOTGOOD3","NOTGOOD4","GOODJOB1","GOODJOB2","GOODJOB3" };
enum { SFX_COIN = 21, SFX_JOIN = 3, SFX_MOVE = 16, SFX_ENTER = 4, SFX_RANK_A = 23 };
typedef struct { int16_t *pcm; uint32_t frames; } SfxBuf;
typedef struct { const SfxBuf *b; uint32_t pos; } SfxVoice;
static SfxBuf g_sfx[35]; static SfxVoice g_sv[8];
typedef struct { int16_t *pcm; uint64_t frames; volatile uint64_t pos; int playing; uint32_t cb_ticks; uint64_t cb_pos; } Music;
static Music g_mus; static SDL_AudioDeviceID g_adev; static int g_arate = 44100;
static double g_alat_ms, g_aoff_ms;     /* latencia do buffer de audio (cb_pos e o fim do bloco ja entregue ao dispositivo) e ajuste manual (-aoff MS) */
static void SDLCALL audio_cb(void *u, Uint8 *st, int len)
{
    (void)u; int16_t *o = (int16_t *)st; int n = len / 4;
    for (int i = 0; i < n; i++) {
        if (g_mus.playing && g_mus.pcm && g_mus.pos < g_mus.frames) { o[2 * i] = g_mus.pcm[2 * g_mus.pos]; o[2 * i + 1] = g_mus.pcm[2 * g_mus.pos + 1]; g_mus.pos++; }
        else o[2 * i] = o[2 * i + 1] = 0;
        for (int v = 0; v < 8; v++) if (g_sv[v].b) {
            const SfxBuf *b = g_sv[v].b; int a = o[2 * i] + b->pcm[2 * g_sv[v].pos], c = o[2 * i + 1] + b->pcm[2 * g_sv[v].pos + 1];
            o[2 * i] = (int16_t)(a > 32767 ? 32767 : a < -32768 ? -32768 : a); o[2 * i + 1] = (int16_t)(c > 32767 ? 32767 : c < -32768 ? -32768 : c);
            if (++g_sv[v].pos >= b->frames) g_sv[v].b = NULL;
        }
    }
    g_mus.cb_pos = g_mus.pos; g_mus.cb_ticks = SDL_GetTicks();
}
static void sfx_load(void);
static void audio_init(void)
{
    SDL_AudioSpec w, h; SDL_zero(w); w.freq = 44100; w.format = AUDIO_S16SYS; w.channels = 2; w.samples = 1024; w.callback = audio_cb;
    g_adev = SDL_OpenAudioDevice(NULL, 0, &w, &h, 0);
    if (g_adev) { g_arate = h.freq; g_alat_ms = 2000.0 * h.samples / h.freq; sfx_load(); SDL_PauseAudioDevice(g_adev, 0); } else logf_("sem audio: %s\n", SDL_GetError());
}
static void sfx_load(void)
{
    for (int k = 0; k < 35; k++) {
        char p[300]; path_of(p, sizeof p, "EFFECTS/%s.WAV", SFX_NAMES[k]);
        FILE *f = fopen(p, "rb"); if (!f) continue;
        fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
        uint8_t *d = (uint8_t *)malloc(n); if (!d || fread(d, 1, n, f) != (size_t)n) { free(d); fclose(f); continue; } fclose(f);
        int ch = 0, bits = 0; uint32_t rate = 0, dl = 0; const uint8_t *pd = NULL; long o = 12;
        while (o + 8 <= n) {
            uint32_t sz; memcpy(&sz, d + o + 4, 4);
            if (!memcmp(d + o, "fmt ", 4)) { uint16_t c2, b2; memcpy(&c2, d + o + 10, 2); memcpy(&rate, d + o + 12, 4); memcpy(&b2, d + o + 22, 2); ch = c2; bits = b2; }
            else if (!memcmp(d + o, "data", 4)) { pd = d + o + 8; dl = sz; if (o + 8 + (long)dl > n) dl = (uint32_t)(n - o - 8); break; }
            o += 8 + sz + (sz & 1);
        }
        if (pd && rate && (ch == 1 || ch == 2) && (bits == 8 || bits == 16)) {
            uint32_t sf = dl / (ch * bits / 8), of = (uint32_t)((uint64_t)sf * g_arate / rate);
            int16_t *out = (int16_t *)malloc((size_t)of * 4 + 4);
            for (uint32_t i = 0; i < of; i++) {
                uint32_t si = (uint32_t)((uint64_t)i * rate / g_arate); if (si >= sf) si = sf - 1;
                for (int c = 0; c < 2; c++) {
                    int cc = ch == 2 ? c : 0; int v;
                    if (bits == 8) v = ((int)pd[si * ch + cc] - 128) << 8; else { int16_t t; memcpy(&t, pd + (si * ch + cc) * 2, 2); v = t; }
                    out[2 * i + c] = (int16_t)v;
                }
            }
            g_sfx[k].pcm = out; g_sfx[k].frames = of;
        }
        free(d);
    }
}
/* equivale a FUN_00030fac(0, idx, canal): um novo efeito no mesmo canal corta o anterior */
static void sfx_play(int idx, int ch)
{
    if (!g_adev || idx < 0 || idx >= 35 || !g_sfx[idx].pcm || ch < 0 || ch > 7) return;
    SDL_LockAudioDevice(g_adev); g_sv[ch].b = &g_sfx[idx]; g_sv[ch].pos = 0; SDL_UnlockAudioDevice(g_adev);
}
static int music_load_pfx(const char *pfx, int id)
{
    free(g_mus.pcm); memset((void *)&g_mus, 0, sizeof g_mus);
    char p[300]; path_of(p, sizeof p, "AUDIO/%s%03d.AUD", pfx, id);
    size_t len; int ok = 0; uint8_t *mp3 = piu_unpack_file(p, &len, &ok);
    if (!mp3) return 0;
    drmp3_config cfg = { 2, (drmp3_uint32)g_arate }; drmp3_uint64 frames = 0;
    drmp3_int16 *pcm = drmp3_open_memory_and_read_pcm_frames_s16(mp3, len, &cfg, &frames, NULL);
    free(mp3);
    if (!pcm) return 0;
    g_mus.pcm = pcm; g_mus.frames = frames; logf_("musica %s%03d: %.1f s (adler %s)\n", pfx, id, (double)frames / g_arate, ok ? "ok" : "BAD"); return 1;
}
static int music_load(int id) { return music_load_pfx("", id); }
static uint32_t g_t0;     /* inicio (SDL ticks) quando nao ha audio */
static void music_start(void) { g_mus.pos = 0; g_mus.cb_pos = 0; g_mus.cb_ticks = SDL_GetTicks(); g_mus.playing = 1; g_t0 = SDL_GetTicks(); }
static void music_stop(void) { g_mus.playing = 0; }
/* posicao em ticks de 240 Hz (0x205B4) */
static int32_t music_pos(void)
{
    double ms;
    if (g_mus.pcm && g_adev) {
        ms = (double)g_mus.cb_pos * 1000.0 / g_arate + (SDL_GetTicks() - g_mus.cb_ticks);
        double maxms = (double)g_mus.pos * 1000.0 / g_arate; if (ms > maxms) ms = maxms;
        ms -= g_alat_ms + g_aoff_ms; if (ms < 0) ms = 0;      /* posicao ouvida = posicao entregue - latencia (2 blocos) */
    } else ms = SDL_GetTicks() - g_t0;
    return (int32_t)(ms * 240.0 / 1000.0);
}

/* ------------------------------------------------------------------ musicas / charts */
typedef struct { int id; float bpm; char title[64]; } Song;
static Song g_songs[256]; static int g_nsongs;
static const char *DIFFS[7] = { "E1", "H1", "H3", "E2", "H2", "H4", "DB" };
static const char *DIFFN[7] = { "EASY", "HARD", "CRAZY", "EASY DBL", "HARD DBL", "CRAZY DBL", "DOUBLE" };
typedef struct { int id, lv, song, vis; } SEnt;      /* vis = byte 0 da entrada do ELF (0x86D4D5A): so entradas com vis!=0 entram na roda (func_2b70b) */
static SEnt g_sec[9][128]; static int g_nsec[9];      /* STAGE.CFG: secoes MODE (posicionais: EASY HARD DOUBLE REMIX REMIXDB CRAZY REMIXCZ MENU END), linhas "S nivel id" */
static void songs_load(void)
{
    char p[300]; size_t n; path_of(p, sizeof p, "STAGE.CFG");
    uint8_t *b = slurp(p, &n); if (!b) return;
    char *s = (char *)realloc(b, n + 1); s[n] = 0; char *sv = NULL;
    for (char *l = strtok_s(s, "\r\n", &sv); l; l = strtok_s(NULL, "\r\n", &sv)) {
        int id; float bpm; char t[64];
        static int sec = -1; int lv; while (*l == ' ' || *l == '	') l++;      /* linhas das secoes comecam com TAB */
        if (!strncmp(l, "MODE", 4)) { sec++; continue; }
        if (sec >= 0 && sec < 9 && l[0] == 'S' && sscanf(l, "S %d %d", &lv, &id) == 2 && g_nsec[sec] < 128) { SEnt *e = &g_sec[sec][g_nsec[sec]++]; e->id = id; e->lv = lv; e->song = -1; e->vis = 1; continue; }
        if (l[0] == 'T' && l[1] == ' ' && sscanf(l, "T %d:%f:%63[^\r\n]", &id, &bpm, t) == 3 && g_nsongs < 256) {
            Song *so = &g_songs[g_nsongs++]; so->id = id; so->bpm = bpm;
            for (char *c = t; *c; c++) if ((unsigned char)*c > 126) *c = '?';
            snprintf(so->title, sizeof so->title, "%s", t);
        }
    }
    free(s);
    for (int a = 0; a < 9; a++) for (int i = 0; i < g_nsec[a]; i++) for (int j = 0; j < g_nsongs; j++) if (g_songs[j].id == g_sec[a][i].id) g_sec[a][i].song = j;
}
static int chart_path(char *out, size_t sz, int id, int d)
{
    char a[300]; const char *cand[2] = { DIFFS[d], NULL }; char low[4] = { (char)(DIFFS[d][0] + 32), DIFFS[d][1], 0 };
    cand[1] = low;
    for (int i = 0; i < 2; i++) {
        path_of(a, sizeof a, "STEP/%03d_%s.NOT", id, cand[i]);
        FILE *f = fopen(a, "rb"); if (f) { fclose(f); snprintf(out, sz, "%s", a); return 1; }
        path_of(a, sizeof a, "STEP/%03d_%s.not", id, cand[i]);
        f = fopen(a, "rb"); if (f) { fclose(f); snprintf(out, sz, "%s", a); return 1; }
    }
    return 0;
}

/* ------------------------------------------------------------------ jogo */
enum { S_PLAY, S_MODE, S_MCONF, S_WHEEL, S_LOAD, S_END, S_WLOAD, S_IDLE, S_LOGO, S_INTRO, S_TITLE, S_VIEW };   /* original: DAT_00153710 = 0 idle, 0xD/0xE logo, 0x10/0x11 intro, 0x13/0x14 title, 0xB selecao */
static int g_state = S_IDLE;
static ResSet g_arrow;       /* BGA/00.DAT */
static Spr *g_sp541, *g_sp542;
static Tex *g_back;          /* fundo da selecao */
static int g_sel = 0, g_dsel = 0, g_speed = 1, g_sbase = 1;     /* velocidade 1..8 (padrao 1X; os codigos 2X/4X/8X escolhem 2/4/8) */
static Tex g_backtex; static int g_backid = -1;

static PiuChart g_chart;
static PiuPlayer g_ps[2];            /* [1] so no couple (modos de 2 jogadores: passos E2/H2/H4 com 10 colunas, cada jogador em 5) */
#define g_pl g_ps[0]
static int g_two;
static PiuWindows g_win_;
static int g_dbl, g_song, g_diff;
static BgaRun g_bga;
static int g_lrp[2], g_f1p[2], g_prevrow, g_fail, g_maxcombo, g_endpos;
static int g_pressed[10]; static uint32_t g_flashuntil[10], g_pt[10], g_ft[10];     /* g_pt: instante do toque (tiles p + pulso), g_ft: instante do acerto (tiles f) */
static int g_sflag;
static Spr *g_spd[5];            /* SP2 por direcao: DL ARROW542, UL 541, centro 545, UR 543, DR 544 (func_221a4: 0x83C7226/83C9FAA/83C907E/83C8152/83C62FA) */
static int g_lg[2]; static uint32_t g_lgt[2]; static int g_mcp[2];       /* g_mcp: maior combo por jogador (tela de resultado) */
static int g_notes_total;

static int slot_for(int32_t pos)       /* 0x23E69: troca de BPM por tempo (centesimos) */
{
    int cs = (int32_t)((int64_t)pos * 100 / 240);
    if (g_chart.third[0] == 0) return 0;
    if (cs < g_chart.third[0]) return 0;
    if (cs < g_chart.third[1]) return 1;
    return 2;
}
static PiuClock clock_for(int slot)
{
    PiuClock c = { g_chart.bpm[slot], g_chart.sync[slot], (int)g_chart.n_per_beat };
    return c;
}
static int row_max(int row, int c0, int c1)     /* 0x22F21/0x22F5D/0x22F9C */
{
    int m = '0'; if (row < 0 || row >= PIU_ROWS) return m;
    for (int c = c0; c < c1; c++) if (g_chart.cell[row * PIU_ROWSTRIDE + c] > m) m = g_chart.cell[row * PIU_ROWSTRIDE + c];
    return m;
}
static void judge_grade(int p, int g)
{
    PiuPlayer *P = &g_ps[p];
    piu_apply_judgment(P, g);
    if (P->life > 1000) P->life = 1000;
    if (P->life < 0) P->life = 0;
    if (P->combo > g_maxcombo) g_maxcombo = P->combo;
    if (P->combo > g_mcp[p]) g_mcp[p] = P->combo;
    g_lg[p] = g; g_lgt[p] = SDL_GetTicks();
}
static int g_pos_cache, g_slot_cache;
/* Codigos (modifiers): palavras de flags por jogador (ELF 0x83CAEAA P1 / 0x83C62C6 P2) e flags de modo altas (0x81206D2 bits 0x1000 SYNCHRO, 0x2000 COUPLE, 0x4000 UNION).
 * Persistem durante o credito (zerados no fim de jogo/atract); o codigo 55555555 zera tudo. Bits por jogador: 0x02 2X, 0x04 4X, 0x08 8X, 0x10 VANISH, 0x20 MIRROR, 0x40 RANDOM VELOCITY,
 * 0x80 NONSTEP, 0x100 RANDOM VANISH, 0x200 RANDOM STEP (nomes pela lista do usuario; confirmado so o desvio de bits no ELF). */
static int g_mod[2], g_mfx;
static void mods_clear(void) { g_mod[0] = g_mod[1] = 0; g_mfx = 0; }
static int g_w_sec, g_m_sel;
static void start_song(int di)
{
    Song *s = &g_songs[g_sel]; char p[300];
    if (!chart_path(p, sizeof p, s->id, di) || piu_chart_load(p, &g_chart)) return;
    logf_("start_song: musica %d passo %s (%s) sflag=%d modo=%d secao=%d\n", s->id, DIFFS[di], p, g_sflag, g_m_sel, g_w_sec);
    {   /* efeitos dos codigos: velocidade 2X/4X/8X e MIRROR (inverte as colunas de cada metade; double inverte as 10). Demais ainda nao aplicados. */
        int pl = ((g_sflag & 3) == 2) ? 1 : 0, m = g_mod[pl];
        g_speed = (m & 2) ? 2 : (m & 4) ? 4 : (m & 8) ? 8 : g_sbase;
        /* RANDOM STEP (0x200): permutacao aleatoria das 5 colunas de cada metade (inferido: o original embaralha os passos) */
        for (int h = 0; h < 2; h++) {
            int mm = (di >= 3 && di <= 5) ? g_mod[h] : m, dbl2 = di == 6;
            if (!(mm & 0x200) || (h && !(di >= 3 && di <= 5) && !dbl2) ) continue;
            if (dbl2 && h) break;
            int n = dbl2 ? 10 : 5, base = dbl2 ? 0 : h * 5, perm[10];
            for (int i = 0; i < n; i++) perm[i] = i;
            for (int i = n - 1; i > 0; i--) { int j = rand() % (i + 1), t2 = perm[i]; perm[i] = perm[j]; perm[j] = t2; }
            for (int r = 0; r < PIU_ROWS; r++) { uint8_t *c = g_chart.cell + r * PIU_ROWSTRIDE + base, o[10]; memcpy(o, c, n); for (int i = 0; i < n; i++) c[perm[i]] = o[i]; }
        }
        for (int h = 0; h < 2; h++) {
            int mm = (di >= 3 && di <= 5) ? g_mod[h] : m;
            if (!(mm & 0x20)) continue;
            int dbl = di == 6;
            if (dbl && h) break;
            if (!dbl && !(di >= 3 && di <= 5) && h) break;
            for (int r = 0; r < PIU_ROWS; r++) {
                uint8_t *c = g_chart.cell + r * PIU_ROWSTRIDE;
                if (dbl) { for (int i = 0; i < 5; i++) { uint8_t t = c[i]; c[i] = c[9 - i]; c[9 - i] = t; } }
                else { uint8_t *q = c + h * 5; uint8_t t = q[0]; q[0] = q[4]; q[4] = t; t = q[1]; q[1] = q[3]; q[3] = t; }
            }
        }
    }
    g_song = s->id; g_diff = di; g_dbl = di == 6; g_two = di >= 3 && di <= 5;      /* DB = double (1 jogador, 10 colunas); E2/H2/H4 = couple */
    /* (antes: notas nas colunas 5-9 ligavam g_dbl. Errado: 514_H1/403_H1/505_H3 trazem as duas metades e o jogo de 1 jogador so usa as colunas 0-4.) */
    memset(g_ps, 0, sizeof g_ps);
    for (int i = 0; i < 2; i++) { g_ps[i].life = 500; g_ps[i].life_rate = 500; g_ps[i].modifier = -700; }     /* 0x24AC0..0x24B2D */
    piu_windows(g_chart.bpm[0], &g_win_);
    g_mcp[0] = g_mcp[1] = 0; g_lrp[0] = g_lrp[1] = 0; g_prevrow = 0; g_f1p[0] = g_f1p[1] = 0; g_fail = 0; g_maxcombo = 0; g_lg[0] = g_lg[1] = 0;
    g_notes_total = 0;
    for (int r = 0; r < PIU_ROWS; r++) if (row_max(r, 0, (g_two || g_dbl) ? 10 : 5) == '9') g_notes_total++;
    memset(g_pressed, 0, sizeof g_pressed); memset(g_flashuntil, 0, sizeof g_flashuntil); for (int i = 0; i < 10; i++) g_pt[i] = g_ft[i] = 0x80000000u;
    bga_unload(&g_bga); bga_load(&g_bga, s->id);
    music_load(s->id);
    g_endpos = 0; for (int r = PIU_ROWS - 1; r >= 0; r--) if (row_max(r, 0, 14) > '0') { g_endpos = r; break; }
    music_start(); g_state = S_PLAY;
}
static void on_press(int col)       /* 0x24100..0x242B7 */
{
    if (col < 0 || col > 9) return;
    g_pressed[col] = 1; g_flashuntil[col] = SDL_GetTicks() + 120; g_pt[col] = SDL_GetTicks();
    int32_t pos = music_pos(); int slot = slot_for(pos); PiuClock ck = clock_for(slot);
    int row = piu_row_now(&ck, pos, 0); if (row < g_prevrow) row = g_prevrow;
    int pl = g_two ? col / 5 : 0, lo = g_two ? pl * 5 : 0, hi = g_two ? lo + 5 : (g_dbl ? 10 : 5);
    if (ck.n > 2 && row_max(row, lo, hi) < '9') row++;                   /* 0x23FAD */
    if (row < g_lrp[pl]) row = g_lrp[pl];
    uint8_t *cell = &g_chart.cell[row * PIU_ROWSTRIDE + col];
    if (*cell != '9') return;
    int d = (row * 60) / ck.n - piu_c18(&ck, pos);
    *cell = (uint8_t)piu_classify(&g_win_, d);
    if (*cell >= '5' && *cell <= '7') g_ft[col] = SDL_GetTicks();       /* tiles f1..f8 (func_22502); quais julgamentos disparam = inferido */
    int m = row_max(row, lo, hi);
    if (row > g_lrp[pl] && m >= '5' && m <= '8') { judge_grade(pl, m); g_f1p[pl] = 1; g_lrp[pl] = row; }
}
/* ------------------------------------------------------------------ fim da musica: STAGE CLEARED / FAILED / BREAK, DANCE GRADE, GAME OVER (83.DAT)
 * FUN_24DE6 decide a fase (DAT 0x83C62A6) e FUN_255D9 as executa, cada uma tocando quadros do 83.BGA com contador de quadros (0x83C627A, 60 Hz):
 *   1 FAILED   quadro c        (c ate 180)  -> 3      2 CLEARED  quadro 360 + c (ate 240) -> resultado (estado 5, FUN_257CF)
 *   4 BREAK    quadro 1800 + c (ate 240)    -> 3      3 GAME OVER quadro 2280 + c (ate 120) -> attract
 * Resultado: quadro 600 + c%480; sai com c > 400 (ou c > 250 e botao). */
enum { PH_FAIL = 1, PH_CLEAR = 2, PH_OVER = 3, PH_BREAK = 4, PH_RESULT = 7 };
static BgaRun g_eb; static int g_ph, g_pc, g_endkey, g_stage, g_fl; static float g_stlife[4];
static int svc_break_on(void);
static BgaRun g_ab, g_ab2;
static const SprTile *sp2_find_x(const Spr *s, const char *tag);
static void tile_up(ResSet *rs, const SprTile *t, float ox, float oy);
static void goto_idle(void);
static void end_start(int ph) { if (ph == PH_BREAK) music_stop(); if (!g_eb.ok) bga_load_name(&g_eb, "83"); g_ph = ph; g_pc = 0; g_endkey = 0; g_state = S_END; }
static void wheel_enter(void);
static void end_over(void) { end_start(PH_OVER); }
static float end_life(void) { float m = g_ps[0].life / 1000.0f; if (g_two && g_ps[1].life / 1000.0f > m) m = g_ps[1].life / 1000.0f; return m; }
static void end_next(void)             /* FUN_2568A: proximo estagio ou fim de jogo */
{
    float lf = end_life(); if (g_stage >= 0 && g_stage < 4) g_stlife[g_stage] = lf;
    int st = ++g_stage, cont = 0;
    if (g_fl == 0x100 || g_fl == 0x200 || g_fl == 0x800) cont = st < 2;                                 /* REMIX / REMIXDB / REMIXCZ: 2 estagios */
    else if (g_fl == 0x40) cont = st < 3;                                                                  /* BATALHA */
    else if (g_fl == 0x80) cont = st < 3 || (st == 3 && g_stlife[0] >= 1.0f && g_stlife[1] >= 0.9f && g_stlife[2] >= 0.9f);   /* DOUBLE */
    else cont = st < 3 || (st == 3 && g_stlife[0] >= 0.9f && g_stlife[1] >= 0.9f && g_stlife[2] >= 0.9f);                /* EASY / HARD / CRAZY: 3 + extra */
    if (cont) {
        static const char *const BG[7] = { "091", "091", "092", "094", "093", "095", "096" };
        bga_unload(&g_eb); bga_unload(&g_ab); bga_unload(&g_ab2); bga_load_name(&g_ab, "098"); bga_load_name(&g_ab2, BG[g_m_sel]); wheel_enter();
    } else end_over();
}
static void end_step(void)
{
    int c = ++g_pc;
    switch (g_ph) {
    case PH_FAIL: if (c > 180) end_over(); break;
    case PH_BREAK: if (c > 240) end_over(); break;
    case PH_CLEAR: if (c > 240) { g_ph = PH_RESULT; g_pc = 0; } break;
    case PH_OVER: if (c > 120) { g_stage = 0; g_sflag = 0; mods_clear(); goto_idle(); } break;
    case PH_RESULT:
        if (c >= 30 && c <= 55 && c % 5 == 0) sfx_play(8, 1);                    /* 0x25D82: idx 8 canal 1 a cada numero */
        if (c == 70) sfx_play(9, 2);                                              /* aparece a nota: idx 9 canal 2 */
        if (c == 80 && g_fl != 0x40) {                                            /* voz da nota (RANK_A..F, idx 0x17..0x1B, canal 6) pela vida */
            float f = end_life(); int idx = f >= 0.9f ? 0x17 : f >= 0.8f ? 0x18 : f >= 0.7f ? 0x19 : f >= 0.6f ? 0x1a : 0x1b; sfx_play(idx, 6); }
        if (c > 400 || (c > 250 && g_endkey)) end_next();
        break;
    }
}
static void res_digits(float x, float y, int v, int n)       /* FUN_21F81/21E9F: n digitos de event.tga (celulas 24x40, v0 = 192), da direita p/ esquerda, passo -22; y para cima */
{
    Tex *t = rs_tex(&g_arrow, "event.tga"); if (!t) return;
    if (v < 0) v = 0;
    for (int i = 0; i < n; i++) { int d = v % 10; v /= 10; float u = d * 24.f; quad(t, x, y, 24, 40, u / t->w, (192.f + 40) / t->h, (u + 24) / t->w, 192.f / t->h); x -= 22; }
}
static void draw_end(void)
{
    int frame = g_ph == PH_FAIL ? g_pc : g_ph == PH_CLEAR ? 360 + g_pc : g_ph == PH_BREAK ? 1800 + g_pc : g_ph == PH_OVER ? 2280 + g_pc : 600 + g_pc % 480;
    glColor4f(1, 1, 1, 1); bga_draw(&g_eb, frame);
    if (g_ph != PH_RESULT) return;
    float fade = g_pc <= 0x17c ? 1.0f : (400 - g_pc) / 20.0f; if (fade < 0) fade = 0;
    glPushMatrix(); glTranslatef(0, 480, 0); glScalef(1, -1, 1);
    const SprTile *t;
    for (int p = 0; p < (g_two ? 2 : 1); p++) {
        const PiuPlayer *P = &g_ps[p]; int side = g_two ? p : ((g_sflag & 3) == 2 ? 1 : 0);
        float cx = side ? 488.f : 176.f, sx = side ? 588.f : 160.f, gx = side ? 522.f : 118.f;
        glColor4f(1, 1, 1, fade);
        if (g_pc > 10) res_digits(sx, 16, P->score, 7);
        if (g_pc > 30) res_digits(cx, 343, P->cnt[0], 3);
        if (g_pc > 35) res_digits(cx, 293, P->cnt[1], 3);
        if (g_pc > 40) res_digits(cx, 243, P->cnt[2], 3);
        if (g_pc > 45) res_digits(cx, 193, P->cnt[3], 3);
        if (g_pc > 50) res_digits(cx, 143, P->cnt[4], 3);
        if (g_pc > 55) res_digits(cx, 78, g_mcp[p], 3);
        if (g_pc > 70) {
            float f = P->life / 1000.0f;
            const char *gn = (f >= 1.0f && P->cnt[4] == 0) ? "grade_s" : f >= 0.9f ? "grade_a" : f >= 0.8f ? "grade_b" : f >= 0.7f ? "grade_c" : f >= 0.6f ? "grade_d" : "grade_f";
            glColor4f(1, 1, 1, fade * 0.5f);
            if ((t = sp2_find_x(g_sp542, gn))) { glPushMatrix(); glTranslatef(gx, 228, 0); tile_up(&g_arrow, t, 0, 0); glPopMatrix(); }
        }
    }
    glPopMatrix(); glColor4f(1, 1, 1, 1);
}
static void play_update(void)
{
    int32_t pos = music_pos(); int slot = slot_for(pos); PiuClock ck = clock_for(slot);
    int row = piu_row_now(&ck, pos, 0); if (row < g_prevrow) row = g_prevrow;
    if (row != g_prevrow && row > 0) {                                  /* 0x243E8: linha passou */
        int pr = row - 1;
        for (int pl = 0; pl < (g_two ? 2 : 1); pl++) {
            int lo = g_two ? pl * 5 : 0, hi = g_two ? lo + 5 : (g_dbl ? 10 : 5);
            if (!g_f1p[pl] && row_max(pr, lo, hi) != '0') {
                if (row_max(pr, lo, hi) == '9') judge_grade(pl, PIU_MISS);
            }
            if (g_lrp[pl] < row) g_f1p[pl] = 0;
        }
        g_prevrow = row;
    }
    g_pos_cache = pos; g_slot_cache = slot;
    {   /* STAGE BREAK (FUN_24DE6 0x24F50/0x24F9A): todos os jogadores com vida < 1 (com STAGE BREAK ligado, ~7 MISS seguidos) ou serie de MISS > 50 (51 MISS) */
        int na = g_two ? 2 : 1, allzero = 1, allstreak = 1;
        for (int p = 0; p < na; p++) { if (g_ps[p].life >= 1) allzero = 0; if (g_ps[p].miss_streak <= 50) allstreak = 0; }
        if ((svc_break_on() && allzero) || allstreak) { sfx_play(7, 7); end_start(PH_BREAK); return; }       /* idx 7 canal 7 */
    }
    int endrow = g_endpos + 8;
    if ((row > endrow && pos > 240) || (g_mus.pcm && g_mus.pos >= g_mus.frames && row > g_endpos)) {
        /* fim da musica (0x2505A): com STAGE BREAK ligado, CLEARED se a vida de algum jogador >= 0.6, senao FAILED; desligado: sempre CLEARED (inferido) */
        end_start(!svc_break_on() || end_life() >= 0.6f ? PH_CLEAR : PH_FAIL);
    }
}

/* setas: sheet = seta UL; col0 DL(flipY) col1 UL col2 centro col3 UR(flipX) col4 DR(flip XY) */
static void draw_arrow(int col, float cx, float cy, int frame, int kind)   /* kind 0 receptor,1 seta */
{
    int c5 = col % 5; const char *file; float u, v; float x0 = 8 + 70 * (frame % 3);
    if (c5 == 2) { file = "arrow03.tga"; u = x0; v = kind ? 148 : 78; }
    else { file = "arrow01.tga"; u = x0; v = kind ? ((c5 == 1 || c5 == 3) ? 78 : 148) : 8; }
    Tex *t = rs_tex(&g_arrow, file); if (!t) return;
    float u0 = u / t->w, u1 = (u + 54) / t->w, v0 = v / t->h, v1 = (v + 54) / t->h;
    if (c5 == 3 || c5 == 4) { float tmp = u0; u0 = u1; u1 = tmp; }
    if (c5 == 0 || c5 == 4) { float tmp = v0; v0 = v1; v1 = tmp; }
    quad(t, cx - 27, cy - 27, 54, 54, u0, v0, u1, v1);
}
static const SprTile *sp2_find_x(const Spr *s, const char *tag)           /* nome exato (sp2_find compara so 6 letras: gauge_b/gauge__/gauge_D colidem) */
{
    for (int i = 0; s && i < s->n; i++) if (!strcmp(s->t[i].tag, tag)) return &s->t[i];
    return NULL;
}
static void tile_up(ResSet *rs, const SprTile *t, float ox, float oy)         /* tile em coordenadas y-para-cima (dentro de glScalef(1,-1,1)): v invertido */
{
    Tex *tx = rs_tex(rs, t->file); if (!tx) return;
    /* meio texel para dentro: com GL_LINEAR (janela em escala) a borda do recorte puxava o texel do sprite vizinho da folha (linha sob o PRESS CENTER STEP) */
    float hu = 0.5f / tx->w, hv = 0.5f / tx->h;
    quad(tx, ox + t->x, oy + t->y, t->w, t->h, t->u0 / tx->w + hu, t->v1 / tx->h - hv, t->u1 / tx->w - hu, t->v0 / tx->h + hv);
}
/* Icones dos modificadores (FUN_21915 = func_21915, chamada pela roda/modo e pela gameplay; FUN_21885 = translate + sprite do ARROW541 + translate de volta).
 * x = 4 (P1) / 600 (P2), y para cima; sprites do ARROW541 (indice do original = tile - 1): speed 0x1B accel (y 240) | random step 0x32 raccel (240), vanish 0x1A (200) | random vanish 0x23 v_rand (200),
 * mirror 0x19 (160), random velocity 0x20 random (120), nonstep 0x1F nonstp (80); flags de modo: 0x1000 sync 0x18 e 0x2000 couple 0x1E em (4, 280), so sem batalha.
 * Em double so o jogador que entrou mostra (0x21A5B). Cor branca. */
static void draw_mods(void)
{
    static const struct { int m; const char *tag; float y; } L[] = {
        { 0x00e, "accel_", 240 }, { 0x200, "raccel", 240 }, { 0x010, "vanish", 200 }, { 0x100, "v_rand", 200 },
        { 0x020, "mirror", 160 }, { 0x040, "random", 120 }, { 0x080, "nonstp", 80 } };
    if (!g_sp541) return;
    glColor4f(1, 1, 1, 1);
    glPushMatrix(); glTranslatef(0, 480, 0); glScalef(1, -1, 1);
    if (g_mfx & 0x1000) { const SprTile *t = sp2_find_x(g_sp541, "sync__"); if (t) tile_up(&g_arrow, t, 4, 280); }
    if (g_mfx & 0x2000) { const SprTile *t = sp2_find_x(g_sp541, "couple"); if (t) tile_up(&g_arrow, t, 4, 280); }
    for (int p = 0; p < 2; p++) {
        if (!((g_sflag >> p) & 1) && g_state != S_PLAY) continue;
        if (g_state == S_PLAY && !g_two && p != (((g_sflag & 3) == 2) ? 1 : 0)) continue;
        for (unsigned i = 0; i < sizeof L / sizeof *L; i++) if (g_mod[p] & L[i].m) {
            const SprTile *t = sp2_find_x(g_sp541, L[i].tag); if (t) tile_up(&g_arrow, t, p ? 600.f : 4.f, L[i].y); }
    }
    glPopMatrix();
}
static void draw_play(void)
{
    int32_t pos = g_pos_cache; PiuClock ck = clock_for(g_slot_cache);
    int frame = pos / 4; if (frame < 0) frame = 0;
    glColor4f(1, 1, 1, 1); bga_draw(&g_bga, frame);
    glColor4f(1, 1, 1, 1);
    float c14 = (float)piu_c14(&ck, pos); int ncols = (g_dbl || g_two) ? 10 : 5;
    /* Campo (FUN_24DE6 -> func_22674/226ad/226f9, translate (x0, 378) em y-para-cima): P1 x0=28, P2 x0=348; double 66 e 311. Colunas a 48 px; receptores em (x0+35+48k, 413 de baixo = 67 do topo).
     * Notas: dy = (60*r/n - c14/240) * velocidade para baixo da linha (func_22840: t*v/240 - linha*v em y-para-cima). Camadas: receptor b1/b2 (221a4), p1..p8 (22386), f1..f8 (22502), notas n1..n6 (22840). */
    const float recY = 67.0f; uint32_t now = SDL_GetTicks();
    int beat60 = (int)(c14 / 240.0f); if (beat60 < 0) beat60 = 0;
    const char *btag = ((beat60 + 6000) % 60) >= 45 ? "b2" : "b1";                   /* c6292 = (resto%60)/45 */
    char ntag[4]; snprintf(ntag, sizeof ntag, "n%d", ((beat60 + 24000) % 60) / 10 + 1);   /* ebp = resto/10 + 2 (indice do tile) */
    static const float PULSE[8] = { 1.0f, .97f, .94f, .91f, .88f, .85f, .72f, .69f };    /* 0x81206EA */
    float x0s[2] = { 28.f, 348.f }; if (g_dbl) { x0s[0] = 66.f; x0s[1] = 311.f; } else if ((g_sflag & 3) == 2) x0s[0] = 348.f;
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /* Ordem de desenho (func_221a4): DL, centro, DR, UR, UL - cada receptor e um sprite separado, entao Z/S/C ficam atras e Q/E na frente. Tres passadas como o original:
     * receptores b1/b2 (221a4), depois p1..p8 (22386), depois f1..f8 (22502). */
    static const int ORD[5] = { 0, 2, 4, 3, 1 };
    for (int pass = 0; pass < 3; pass++)
    for (int k = 0; k < ncols; k++) {
        int col = (k / 5) * 5 + ORD[k % 5];
        Spr *sp = g_spd[col % 5]; if (!sp) continue;
        float x0 = x0s[col / 5] + 48.f * (col % 5), cx = x0 + 35.f;
        int pc = (int)((now - g_pt[col]) * 60 / 1000), fc = (int)((now - g_ft[col]) * 60 / 1000);
        const SprTile *t;
        glColor4f(1, 1, 1, 1);
        if (pass == 0) { t = sp2_find(sp, btag); if (t) { glPushMatrix(); glTranslatef(cx, recY, 0); if (pc < 8) glScalef(PULSE[pc], PULSE[pc], 1); glColor4f(1, 1, 1, .95f); draw_tile(&g_arrow, t, 0, 0); glPopMatrix(); } }
        else if (pass == 1) { if (pc >= 0 && pc < 16) { char tg[4]; snprintf(tg, sizeof tg, "p%d", pc / 2 + 1); if ((t = sp2_find(sp, tg))) draw_tile(&g_arrow, t, x0, recY - 35.f); } }
        else { if (fc >= 0 && fc < 24) { char tg[4]; snprintf(tg, sizeof tg, "f%d", fc / 3 + 1); if ((t = sp2_find(sp, tg))) draw_tile(&g_arrow, t, x0, recY - 35.f); } }
    }
    for (int k = 0; k < ncols; k++) {
        int col = (k / 5) * 5 + ORD[k % 5];
        Spr *sp = g_spd[col % 5]; const SprTile *t = sp ? sp2_find(sp, ntag) : NULL; if (!t) continue;
        float x0 = x0s[col / 5] + 48.f * (col % 5);
        int md = g_mod[g_two ? col / 5 : ((g_sflag & 3) == 2 ? 1 : 0)];
        float spd = (float)g_speed;
        if (md & 0x40) spd *= 1.0f + 0.5f * (1.0f + sinf(c14 / 240.0f * 0.35f)) * 1.5f;      /* RANDOM VELOCITY (inferido): velocidade varia ao longo da musica */
        for (int r = 0; r < PIU_ROWS; r++) {
            float y = recY + ((60.0f * r) / ck.n - c14 / 240.0f) * spd;
            if (y < -60) continue;
            if (y > 540) break;
            if (md & 0x80) continue;                                                                     /* NONSTEP: setas nao aparecem */
            if (md & 0x10) { if (y < 300) continue; }                                                    /* VANISH: somem no meio da tela (inferido) */
            else if (md & 0x100) { if (y < 180 + (float)((r * 2654435761u >> 24) % 140)) continue; }     /* RANDOM VANISH: limite aleatorio por seta (inferido) */
            if (g_chart.cell[r * PIU_ROWSTRIDE + col] == '9') draw_tile(&g_arrow, t, x0, y - 35.f);
        }
    }
    /* HUD (FUN_24DE6 -> func_22fd8 julgamento/combo, func_23a4b vida, func_21e47 + func_21885 score). Coordenadas do jogo em y-para-cima (como o original). */
    {
        Spr *s541 = g_sp541, *s542 = g_sp542; const SprTile *t; uint32_t fr = now * 60 / 1000;      /* contador de quadros a 60 Hz (0x83C627A) */
        int base_slot = (g_sflag & 3) == 2 ? 1 : 0;
        glColor4f(1, 1, 1, 1);
        glPushMatrix(); glTranslatef(0, 480, 0); glScalef(1, -1, 1);
        for (int hp = 0; hp < (g_two ? 2 : 1); hp++) {
        const PiuPlayer *P = &g_ps[hp]; int slot = g_two ? hp : base_slot;
        /* vida: gauge_b (moldura) em (32|352, 445); preenchimento gauge__ por faixas de 6 px: n=(int)(v*256), q=n/6; P1 revela x in [253-6q,256], P2 x in [0,6q+2]. v = vida/1000.
         * Perigo (vida<180): gauge_D amarelo piscando por tras (so com a opcao 0x86D6FC0 no original; aqui sempre) */
        {
            float v = P->life / 1000.0f; if (v < 0) v = 0; if (v > 1) v = 1;
            int q = (int)(v * 256) / 6; float gx = slot ? 352.f : 32.f;
            glPushMatrix(); glTranslatef(gx, 445, 0);
            if ((t = sp2_find_x(s542, "gauge_b"))) tile_up(&g_arrow, t, 0, 0);
            if (P->life < 180 && (fr & 1) == 0 && (t = sp2_find_x(s542, "gauge_D"))) { glColor4f(1, 1, 0, 1); tile_up(&g_arrow, t, 0, 0); glColor4f(1, 1, 1, 1); }
            if ((fr & 1) && (t = sp2_find_x(s542, "gauge__"))) {      /* preenchimento so em quadros impares (c627a % 2 != 0): pisca a 30 Hz */
                Tex *tx = rs_tex(&g_arrow, t->file);
                if (tx) {
                    if (!slot) { float xa = (float)(253 - 6 * q); quad(tx, xa, 0, 256 - xa, 32, (t->u0 + xa) / tx->w, (t->v0 + 32) / tx->h, (t->u0 + 256) / tx->w, t->v0 / tx->h); }
                    else { float xb = (float)(6 * q + 3);      /* P2 = espelho do P1: faixa [0, 256-(253-6q)] com a textura invertida em x (vermelho na borda de dentro) */
                        quad(tx, 0, 0, xb, 32, (t->u0 + 256) / tx->w, (t->v0 + 32) / tx->h, (t->u0 + 256 - xb) / tx->w, t->v0 / tx->h); }
                }
            }
            glPopMatrix();
        }
        /* score: 7 digitos amarelos (font.tga, celulas 32x39, v0 = 0.53125 + (d/8)*0.15234375), de tras para a frente, passo -24, em (176|573, 16); rotulo score_ em (14|560, 52) */
        {
            Tex *ft = rs_tex(&g_arrow, "font.tga");
            if (ft) {
                int n = P->score; if (n < 0) n = 0;
                glPushMatrix(); glTranslatef(slot ? 573.f : 176.f, 16, 0);
                for (int i = 0; i < 7; i++) {
                    int d = n % 10; n /= 10; float u = (d % 8) * 0.125f, vt = (d / 8) * 0.15234375f + 0.53125f;
                    quad(ft, 0, 0, 32, 39, u, vt + 0.15234375f, u + 0.125f, vt); glTranslatef(-24, 0, 0);
                }
                glPopMatrix();
            }
            if ((t = sp2_find_x(s541, "score_"))) tile_up(&g_arrow, t, slot ? 560.f : 14.f, 52);
        }
        /* julgamento (func_22fd8): 25 quadros; escala de entrada tabela 0x81206EA+0x34 (idx 25-t para t>10); tile = perfec/great_/good__/bad___/miss__ pelo ultimo julgamento ('5'..'9').
         * Combo: 3 digitos de event.tga (celulas 32x48, v0 = .375 + (d/8)*.1875) a 70 px abaixo, passo -26, a partir de +10; branco se combo>=4, depois vermelho (1,0.2,0.2) se serie de MISS>=4 (continua a esquerda). */
        {
            int jt = 25 - (int)((now - g_lgt[hp]) * 60 / 1000);
            if (g_lg[hp] >= '5' && g_lg[hp] <= '9' && jt > 0) {
                static const float JS[17] = { 1.52f, 1.43f, 1.35f, 1.28f, 1.21f, 1.15f, 1.1f, 1.06f, 1.03f, 1.01f, .99f, .98f, .97f, .98f, .99f, 1.0f, 1.0f };
                static const char *const JN[5] = { "perfec", "great_", "good__", "bad___", "miss__" };
                float sc = jt > 10 ? JS[25 - jt] : 1.0f;
                glPushMatrix(); glTranslatef(g_dbl ? 320.f : slot ? 480.f : 160.f, 240, 0); glScalef(sc, sc, 1);
                glColor4f(1, 1, 1, 1);
                if ((t = sp2_find_x(s542, JN[g_lg[hp] - '5']))) tile_up(&g_arrow, t, 0, 0);
                glTranslatef(0, -70, 0);
                Tex *et = rs_tex(&g_arrow, "event.tga");
                for (int pass = 0; pass < 2 && et; pass++) {
                    int n = pass == 0 ? P->combo : P->miss_streak; if (n < 4) continue;
                    if (pass == 1) glColor4f(1, 0.2f, 0.2f, 1);   /* glColor3f(push 0.2, 0.2, 1.0) = (r,g,b) (1.0, 0.2, 0.2): vermelho */
                    glTranslatef(10, 0, 0);
                    for (int i = 0; i < 3; i++) {
                        int d = n % 10; n /= 10; float u = (d % 8) * 0.125f, vt = (d / 8) * 0.1875f + 0.375f;
                        quad(et, 0, 0, 32, 48, u, vt + 0.1875f, u + 0.125f, vt); glTranslatef(-26, 0, 0);
                    }
                }
                glPopMatrix(); glColor4f(1, 1, 1, 1);
            }
        }
        }
        glPopMatrix();
    }
}
/* ------------------------------------------------------------------ fluxo "attract" do PIU.EXE
 * Laco principal FUN_00020F37 (maquina de estados em DAT_00153710):
 *   idle  (0x20E7A): se ha credito -> titulo; senao n=cnt++%4: 0 -> logo(BGA/81.DAT), 1 -> intro(BGA/01.DAT), 3 -> FUN_2D14D (nao reconstruido)
 *   logo  (0x285AB/0x2863A): 500 quadros e volta ao idle (ou titulo se entrar credito)
 *   intro (0x28442/0x28559): ate 0x3B1 (945) quadros, depois carrega BGA/82.DAT -> titulo (0x284C8)
 *   title (0x284E0/0x286A9): 82.DAT; START com credito; 60 quadros depois -> selecao (0x2B763); 0x707 quadros sem acao -> idle
 * O contador DAT_003d651c e o quadro do BGA (incrementa 1 por quadro a 60 Hz, depois FUN_2643C desenha as 50 camadas). */
static BgaRun g_ab;
static int g_service, g_cli_free;
static int g_coins, g_cpc = 1, g_freeplay, g_att, g_cnt, g_bgaf, g_sflag, g_scnt, g_start1, g_start2;
static uint32_t g_tick;
static int cred_avail(void) { return g_freeplay ? 99 : g_coins / g_cpc; }       /* 0x204BA / 0x2047F */
static int svc_demo_on(void);
static void goto_idle(void) { music_stop(); bga_unload(&g_ab); g_state = S_IDLE; }              /* 0x2861B / 0x28695 */
static void enter_title(void)                                                      /* 0x284E0 */
{
    music_stop();
    bga_unload(&g_ab); bga_load_name(&g_ab, "82");
    g_cnt = 0; g_scnt = 0; g_sflag = 0; g_stage = 0; mods_clear(); g_state = S_TITLE;
}
static void mode_enter(void);
static void enter_select(void) { music_stop(); mode_enter(); if (music_load(2)) music_start(); }   /* 0x2BB5E: FUN_294B9 + AUDIO\002.AUD incondicional + BGA 099 */
static int g_auto1;      /* so para self-test: -autostart1 pressiona START do P1 a cada quadro */
static void attract_step(void)
{
    if (g_auto1) g_start1 = 1;
    switch (g_state) {
    case S_IDLE:
        for (int guard = 0; guard < 4 && g_state == S_IDLE; guard++) {
            if (cred_avail()) { enter_title(); break; }
            int n = g_att % 4; g_att++;
            if (n == 0) { bga_load_name(&g_ab, "81"); g_cnt = 0; g_state = S_LOGO; }
            else if (n == 1) { music_stop(); bga_load_name(&g_ab, "01"); g_cnt = 0; g_state = S_INTRO;
                if (svc_demo_on() && music_load(1)) music_start(); }   /* 0x28442: FUN_294B9 (para) e, se DAT_006e1918 (DEMO SOUND), AUDIO.AUD (FUN_29263 EAX=1 + FUN_2949F) */
        }
        break;
    case S_LOGO:   if (g_cnt < 500 && !cred_avail()) { g_bgaf = g_cnt; g_cnt++; } else goto_idle(); break;      /* 0x2863A: EDX = contador antes do ++ */
    case S_INTRO:  if (!cred_avail() && g_cnt < 0x3B1) { g_bgaf = g_cnt; g_cnt++; } else enter_title(); break;  /* 0x28559 */
    case S_TITLE:
        if (g_sflag & 3) g_scnt++;
        if (g_scnt > 0x3C || (g_sflag & 3) == 3) { enter_select(); break; }
        g_bgaf = g_cnt % 180;                 /* 0x2871A: IDIV 0xB4, EDX (resto) vai para FUN_2643C */
        g_cnt++;
        if (g_cnt > 0x707 && !cred_avail() && !(g_sflag & 3)) goto_idle();
        if (cred_avail()) {
            if (!(g_sflag & 1) && g_start1) { g_coins -= g_freeplay ? 0 : g_cpc; g_sflag |= 1; sfx_play(SFX_JOIN, 2); }
            else if (!(g_sflag & 2) && g_start2) { g_coins -= g_freeplay ? 0 : g_cpc; g_sflag |= 2; sfx_play(SFX_JOIN, 2); }
        }
        break;
    }
    g_start1 = g_start2 = 0; g_tick++;
}
/* texto da FONT.PNG do 00.DAT (retangulos medidos na imagem 256x256); posicoes na tela sao inferidas */
static void font_blit(float x, float y, float sx0, float sy0, float sx1, float sy1, float a)
{
    Tex *tx = rs_tex(&g_arrow, "FONT"); if (!tx) return;
    glColor4f(1, 1, 1, a);
    quad(tx, x, y, sx1 - sx0, sy1 - sy0, sx0 / tx->w, sy0 / tx->h, sx1 / tx->w, sy1 / tx->h);
    glColor4f(1, 1, 1, 1);
}
static void font_number(float x, float y, int v)
{
    char s[16]; snprintf(s, sizeof s, "%d", v);
    for (const char *p = s; *p; p++, x += 12) font_blit(x, y, 16 * (*p - '0'), 96, 16 * (*p - '0') + 16, 112, 1);
}
static void title_digits(float x, float y, int v)          /* FUN_000200E3 (func_200e3): digitos de FONT (16x16, u=(d%16)/16, v=96+(d/16)*16), da direita para a esquerda, passo -12, ao menos 1; y para cima */
{
    Tex *ft = rs_tex(&g_arrow, "font.tga"); if (!ft) return;
    do {
        int d = v % 10; v /= 10; float u0 = (d % 16) * 16.f, v0 = 96.f + (d / 16) * 16.f;
        quad(ft, x, y, 16, 16, u0 / ft->w, (v0 + 16) / ft->h, (u0 + 16) / ft->w, v0 / ft->h); x -= 12;
    } while (v);
}
static void draw_title_overlay(void)            /* FUN_00020134 (func_20134): creditos / free play e PRESS CENTER STEP / INSERT COIN, em y-para-cima */
{
    int t = (int)(g_tick % 200);
    float blink = fabsf((float)(t - 100)) * 0.01f + 0.2f;   /* FUN_000C7BD0: |(t%200-100)|*0.01 + 0.2 */
    const SprTile *tl;
    glColor4f(1, 1, 1, 1);
    glPushMatrix(); glTranslatef(0, 480, 0); glScalef(1, -1, 1);
    if (g_freeplay) {
        if ((tl = sp2_find_x(g_sp542, "freeply"))) { glPushMatrix(); glTranslatef(320, 13, 0); tile_up(&g_arrow, tl, 0, 0); glPopMatrix(); }      /* 0x83C7B86 em (0x140, 0x0D) */
    } else {
        /* sem free play: credit(s) em (263, 22) + creditos em (360, 25); com moeda parcial: credit em (239, 22), creditos (338,25), '(' (354,21), restante (358,25), '/' (372,21),
         * moedas por credito (388|378, 25) e ')' (400|390, 21). Tiles credit/left__/right_/slash_ do ARROW541. */
        int cpc = g_cpc ? g_cpc : 1, q = g_coins / cpc, r = g_coins % cpc;
        if (r == 0) {
            if ((tl = sp2_find_x(g_sp541, "credit"))) tile_up(&g_arrow, tl, 263, 22);
            title_digits(360, 25, q);
        } else {
            if ((tl = sp2_find_x(g_sp541, "credit"))) tile_up(&g_arrow, tl, 239, 22);
            title_digits(338, 25, q);
            if ((tl = sp2_find_x(g_sp541, "left__"))) tile_up(&g_arrow, tl, 354, 21);
            title_digits(358, 25, r);
            if ((tl = sp2_find_x(g_sp541, "slash_"))) tile_up(&g_arrow, tl, 372, 21);
            title_digits(cpc >= 10 ? 388 : 378, 25, cpc);
            if ((tl = sp2_find_x(g_sp541, "right_"))) tile_up(&g_arrow, tl, cpc >= 10 ? 400 : 390, 21);
        }
    }
    for (int p = 0; p < 2; p++) {
        if (g_sflag & (1 << p)) continue;
        /* FUN_21885(x, 0x16, tile): press_ (0x25) em x=14 / 408; insert (0x24) em x=48 / 424 */
        glColor4f(1, 1, 1, blink);
        if (cred_avail()) { if ((tl = sp2_find_x(g_sp541, "press_"))) tile_up(&g_arrow, tl, p ? 408.f : 14.f, 22); }
        else if ((tl = sp2_find_x(g_sp541, "insert"))) tile_up(&g_arrow, tl, p ? 424.f : 48.f, 22);
    }
    glPopMatrix(); glColor4f(1, 1, 1, 1);
}
/* ------------------------------------------------------------------ menu de SERVICE (PIU.EXE: FUN_0002e4d4 despacha DAT_0012cb80 = tela 0..8)
 * Botoes originais: TEST = DAT_00153718==0x10000 (mover), SERVICE = 0x20000 (selecionar/sair). Aqui: F2 = TEST, F1 = SERVICE.
 * Telas: 0 menu (2d847) 1 I/O TEST (2d954) 2 EEPROM TEST (2db18) 3 SCREEN TEST (2dc59) 4 GAME OPTION (2dcdc) 5 COIN OPTION (2dfc0)
 *        6 SOUND TEST (2e167) 7 DATE (2e2bc) 8 EXIT (2e450). Texto em y-para-cima (coords do jogo); cursor DAT_0012cb84 (menu) / DAT_0012cb88 (linha). */
typedef struct { uint8_t mode, level, stage, lang, demo, help, coin; } SvcCfg;
static SvcCfg g_cfg = { 0, 1, 2, 0, 1, 0, 1 };      /* DEFAULT SETTING (0x2D6B5): 6e1914=0 6e1915=1 6e1916=2 6e1917=0 6e1918=0 6e1919=0; coin padrao 1 (inferido) */
static int g_svs, g_svcur, g_svrow, g_svlock_ok, g_svlock_err, g_svsnd_a, g_svsnd_e, g_svdate_y = 2000, g_svdate_m = 11, g_svdate_d = 1, g_svio;
static int svc_demo_on(void) { return g_cfg.demo; }
static int svc_break_on(void) { return g_cfg.stage != 0; }      /* opcao STAGE BREAK do GAME OPTION (OFF = 0) */
static void svc_apply(void) { g_cpc = g_cfg.coin ? g_cfg.coin : 1; g_freeplay = g_cfg.coin == 0 || g_cli_free; }
static void svc_save(void)
{
    char pth[300]; path_of(pth, sizeof pth, "PIU_EEPROM.BIN"); FILE *f = fopen(pth, "wb");
    if (f) { fwrite(&g_cfg, sizeof g_cfg, 1, f); fclose(f); }
}
static void svc_load(void)
{
    char pth[300]; path_of(pth, sizeof pth, "PIU_EEPROM.BIN"); FILE *f = fopen(pth, "rb");
    if (f) { SvcCfg c; if (fread(&c, sizeof c, 1, f) == 1 && c.level < 3 && c.stage < 5 && c.lang < 3 && c.coin <= 10) g_cfg = c; fclose(f); }
    svc_apply();
}
static void svc_color(int on, int green)
{
    if (green) glColor4f(0, 1, 0, 1); else if (on) glColor4f(1, 0, 0, 1); else glColor4f(1, 1, 1, 1);     /* 0x12CBB0 / 0x12CB98 / 0x12CBE0 */
}
static void svc_text(float x, float y, const char *s) { text(x, 480 - y - 8, 1, s); }   /* avanco de 8 px/char: "Dec 18 2000" em x=0x224 ocupa ate 636 */                       /* FUN_00026658(x,y,str) */
static const char *const SVC_MAIN[8] = { "I/O TEST", "EEPROM TEST", "SCREEN TEST", "GAME OPTION", "COIN OPTION", "SOUND TEST", "DATE", "EXIT" };
static const char *const SVC_GOPT[9] = { "GAME MODE", "LEVEL", "STAGE BREAK", "LANGUAGE", "DEMO SOUND", "SHOW HELP", "DEFAULT SETTING", "SAVE AND EXIT", "EXIT" };
static const char *const SVC_COPT[4] = { "COIN SETTING", "DEFAULT SETTING", "SAVE AND EXIT", "EXIT" };
static const char *const SVC_SND[3] = { "AUDIO", "EFFECT SOUND", "EXIT" };
static const char *const SVC_DATE[4] = { "YEAR", "MONTH", "DAY", "EXIT" };
static void svc_open(void) { g_service = 1; g_svs = 0; g_svcur = 0; g_svrow = 0; }
/* F1 (pedido do usuario, empirico): entrar no SERVICO fecha o que estiver ativo (gameplay, roda, titulo, fim de fase...): para a musica, descarrega os BGAs e zera
 * jogadores/modificadores/estagio, como um reset. Ao sair do servico volta ao estado 0 (attract): com g_att=0 comeca pelo 81.DAT (logo/insert coin) e, com credito/FREE PLAY, pelo titulo. */
static void svc_halt(void) { music_stop(); bga_unload(&g_eb); bga_unload(&g_ab); bga_unload(&g_ab2); g_sflag = 0; g_stage = 0; g_start1 = g_start2 = 0; mods_clear(); g_att = 0; g_state = S_IDLE; }
static void svc_leave(void) { g_service = 0; music_stop(); bga_unload(&g_ab); bga_unload(&g_ab2); g_sflag = 0; g_stage = 0; mods_clear(); g_att = 0; g_state = S_IDLE; }
static int svc_audio_id(int v) { return v < 3 ? v + 1 : g_songs[(v - 3) % g_nsongs].id; }     /* inferido: 001-003 e depois as musicas */
static void svc_press(int test, int serv)      /* uma vez por aperto (DAT_00153718 e consumido a cada quadro no original) */
{
    static const int NROWS[9] = { 0, 0, 0, 0, 9, 4, 3, 4, 0 };
    if (!test && !serv) return;
    if (g_svs == 0) {
        if (test) { if (++g_svcur > 7) g_svcur = 0; }
        if (serv) { g_svrow = 0; g_svs = g_svcur + 1; }
        return;
    }
    if (g_svs >= 1 && g_svs <= 3) { if (serv) g_svs = 0; return; }
    if (test) { music_stop(); if (++g_svrow >= NROWS[g_svs]) g_svrow = 0; }
    if (!serv) return;
    if (g_svs == 4) {
        switch (g_svrow) {
        case 0: g_cfg.mode = !g_cfg.mode; break;
        case 1: if (++g_cfg.level > 2) g_cfg.level = 0; break;
        case 2: if (++g_cfg.stage > 4) g_cfg.stage = 0; break;
        case 3: if (++g_cfg.lang > 2) g_cfg.lang = 0; break;
        case 4: g_cfg.demo = !g_cfg.demo; break;
        case 5: g_cfg.help = !g_cfg.help; break;
        case 6: { uint8_t c = g_cfg.coin; g_cfg.mode = 0; g_cfg.level = 1; g_cfg.stage = 2; g_cfg.lang = 0; g_cfg.demo = 0; g_cfg.help = 0; g_cfg.coin = c; } break;
        case 7: svc_save(); g_svs = 0; break;
        case 8: g_svs = 0; break;
        }
    } else if (g_svs == 5) {
        switch (g_svrow) {
        case 0: if (++g_cfg.coin > 10) g_cfg.coin = 0; break;          /* 0 = FREE */
        case 1: g_cfg.coin = 1; break;
        case 2: svc_save(); g_svs = 0; break;
        case 3: g_svs = 0; break;
        }
        svc_apply();
    } else if (g_svs == 6) {
        if (g_svrow == 0) { if (music_load(svc_audio_id(g_svsnd_a))) music_start(); g_svsnd_a++; }
        else if (g_svrow == 1) { sfx_play(g_svsnd_e % 35, 0); g_svsnd_e++; }
        else { music_stop(); g_svs = 0; }
    } else if (g_svs == 7) {
        switch (g_svrow) {
        case 0: g_svdate_y = 2000; break;
        case 1: if (++g_svdate_m > 12) g_svdate_m = 11; break;        /* 0x2E2BC: so 11 e 12 */
        case 2: ++g_svdate_d; if (g_svdate_d > (g_svdate_m == 11 ? 30 : 31)) g_svdate_d = 1; break;
        case 3: svc_save(); g_svs = 0; break;
        }
    }
}
static void svc_draw(void)
{
    char b[48];
    glDisable(GL_TEXTURE_2D); glColor4f(0, 0, 0, 1); glBegin(GL_QUADS); glVertex2f(0, 0); glVertex2f(640, 0); glVertex2f(640, 480); glVertex2f(0, 480); glEnd();
    glColor4f(1, 1, 1, 1);
    if (g_svs == 8) { svc_leave(); return; }
    if (g_svs == 0) {
        svc_color(0, 0); svc_text(0x114, 0x1c2, "GAME SETTING");
        for (int i = 0; i < 8; i++) { svc_color(i == g_svcur, 0); svc_text(0x114, 0x160 - i * 0x14, SVC_MAIN[i]); }
        g_svlock_ok++; svc_color(0, 0); snprintf(b, sizeof b, "Lock OK:%d Err:%d", g_svlock_ok, g_svlock_err); svc_text(0x114, 0x98, b);
    } else if (g_svs == 1) {
        const char *L[4] = { "1.TEST  BUTTON", "2.SERVICE BUTTON", "3.CLEAR BUTTON", "4.COIN" }; int y[4] = { 400, 0x17c, 0x168, 0x154 };
        svc_color(0, 0); svc_text(0x114, 0x1c2, "I/O TEST");
        for (int i = 0; i < 4; i++) { svc_color(g_svio & (1 << i), 0); svc_text(0x104, y[i], L[i]); }
    } else if (g_svs == 2) {
        svc_color(0, 0); svc_text(0x114, 0x1c2, "EEPROM TEST");
        svc_text(0x78, 0x154, "WRITE  ..."); svc_text(0x78, 300, "READ   ..."); svc_text(0x78, 0x104, "RESULT ...");
        svc_text(0xdc, 0x154, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345"); svc_text(0xdc, 300, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345");
        svc_color(0, 1); svc_text(0xdc, 0x104, "SUCCESS");
    } else if (g_svs == 3) {
        static const float C[8][3] = { {1,1,1},{1,1,0},{0,1,1},{0,1,0},{1,0,1},{1,0,0},{0,0,1},{0.5f,0.5f,0.5f} };      /* padrao inferido (0x2D194/0x2D259 nao lidos) */
        for (int i = 0; i < 8; i++) { glColor4f(C[i][0], C[i][1], C[i][2], 1); glBegin(GL_QUADS); glVertex2f(i * 80.f, 100); glVertex2f(i * 80.f + 80, 100); glVertex2f(i * 80.f + 80, 400); glVertex2f(i * 80.f, 400); glEnd(); }
        svc_color(0, 0); svc_text(0x114, 0x1c2, "SCREEN TEST");
    } else if (g_svs == 4) {
        static const char *const LV[3] = { "1.EASY", "2.NORMAL", "3.HARD" }, *const LG[3] = { "ENGLISH", "ENGLISH", "CHINESE" }, *const OO[2] = { "OFF", "ON" };
        svc_color(0, 0); svc_text(0x114, 0x1c2, "GAME OPTION");
        for (int i = 0; i < 9; i++) {
            int y = 0x160 - i * 0x14, on = i == g_svrow, gr = 0; const char *v = NULL;
            svc_color(on, 0); svc_text(0xb4, y, SVC_GOPT[i]);
            switch (i) {
            case 0: v = g_cfg.mode ? "EVENT" : "NORMAL"; gr = !g_cfg.mode; break;
            case 1: v = LV[g_cfg.level]; gr = g_cfg.level == 1; break;
            case 2: if (!g_cfg.stage) v = "OFF"; else { snprintf(b, sizeof b, "%d STAGE", g_cfg.stage); v = b; } gr = g_cfg.stage == 2; break;
            case 3: v = LG[g_cfg.lang]; gr = g_cfg.lang == 0; break;
            case 4: v = OO[g_cfg.demo]; gr = !g_cfg.demo; break;
            case 5: v = OO[g_cfg.help]; gr = !g_cfg.help; break;
            }
            if (v) { svc_color(on, gr); svc_text(0x194, y, v); }
        }
    } else if (g_svs == 5) {
        svc_color(0, 0); svc_text(0x114, 0x1c2, "COIN OPTION");
        for (int i = 0; i < 4; i++) {
            int y = 0x160 - i * 0x14; svc_color(i == g_svrow, 0); svc_text(0xb4, y, SVC_COPT[i]);
            if (i == 0) { if (!g_cfg.coin) snprintf(b, sizeof b, "FREE"); else snprintf(b, sizeof b, "1 CREDIT / %2d COINS", g_cfg.coin); svc_text(0x174, y, b); }
        }
    } else if (g_svs == 6) {
        svc_color(0, 0); svc_text(0x114, 0x1c2, "SOUND TEST");
        for (int i = 0; i < 3; i++) {
            int y = 0x160 - i * 0x14; svc_color(i == g_svrow, 0); svc_text(0xb4, y, SVC_SND[i]);
            if (i == 0) { snprintf(b, sizeof b, "#%02d", g_svsnd_a % 100); svc_text(0x174, y, b); }
            if (i == 1) { snprintf(b, sizeof b, "#%02d", g_svsnd_e % 35); svc_text(0x174, 0x14c, b); }
        }
    } else if (g_svs == 7) {
        svc_color(0, 0); svc_text(0x114, 0x1c2, "DATE SETTING");
        for (int i = 0; i < 4; i++) {
            int y = 0x160 - i * 0x14; svc_color(i == g_svrow, 0); svc_text(0xb4, y, SVC_DATE[i]);
            b[0] = 0;
            if (i == 0) snprintf(b, sizeof b, "%4d", g_svdate_y); else if (i == 1) snprintf(b, sizeof b, "%2d", g_svdate_m); else if (i == 2) snprintf(b, sizeof b, "%2d", g_svdate_d);
            if (b[0]) svc_text(0x174, y, b);
        }
    }
    svc_color(0, 0);
    svc_text(4, 4, "PUMP IT UP (R5/v3.52)"); svc_text(0x224, 4, "Dec 18 2000");
    if (g_svs < 1 || g_svs > 3) { svc_text(0xec, 0x24, "MOVE   - TEST    BUTTON"); svc_text(0xec, 0x10, "SELECT - SERVICE BUTTON"); }
    else svc_text(0xec, 0x10, "EXIT   - SERVICE BUTTON");
    glColor4f(1, 1, 1, 1);
}
/* ------------------------------------------------------------------ escolha de MODO (estado 0xB fase 2: FUN_0002BB5E init, FUN_0002BC14, FUN_0002BD93 laco)
 * BGA 099 + barras de titulo da 90.DAT (BAR01..14: 14 folhas x 8 faixas de 32 px; tabela 0x12C3A1). Lista = STAGE.CFG "MODE MENU": ids 2..6
 * (DOUBLE, NORMAL, HARD, REMIX, CRAZY); com 2 jogadores o item 0 vira id 1 (BATTLE) - FUN_2BC14 grava DAT_006E0EE8 hi=1. */
static const int BAR_TBL[14][8] = {
    { 101, 102, 103, 104, 105, 106, 107, 108 }, { 109, 110, 111, 112, 113, 114, 115, 201 }, { 202, 203, 204, 205, 206, 207, 208, 210 },
    { 211, 212, 213, 214, 215, 216, 217, 218 }, { 226, 301, 302, 303, 304, 305, 306, 307 }, { 308, 309, 310, 311, 312, 313, 314, 315 },
    { 316, 317, 318, 319, 320, 321, 328, 329 }, { 401, 402, 403, 404, 405, 406, 407, 408 }, { 409, 410, 411, 412, 413, 414, 116, 117 },
    { 118, 119, 219, 220, 221, 223, 224, 224 }, { 225, 322, 323, 324, 325, 326, 330, 327 }, { 415, 501, 502, 503, 504, 505, 506, 507 },
    { 508, 509, 510, 511, 512, 513, 514, 515 }, { 516, 517, 1, 2, 3, 4, 5, 6 } };
static ResSet g_bars; static int g_bars_ok; static BgaRun g_ab2;      /* g_ab2: fundo colorido 091..096 da fase seguinte */
static int g_m_frame, g_m_anim, g_m_dir = -1, g_m_prev = -1, g_m_cur = 1, g_m_sel, g_m_t0;
static int g_mode_flag;                                     /* DAT_0012B028 |= ... (0x10,0x20,0x100,0x80,0x40,0x400) */
static int mode_id(int k) { return k == 0 ? (((g_sflag & 3) == 3) ? 1 : 2) : k + 2; }       /* DAT_006E0EE8 >> 16 */
static void mode_enter(void)
{
    bga_unload(&g_ab); bga_load_name(&g_ab, "099");
    if (!g_bars_ok) { char p[300]; path_of(p, sizeof p, "BGA/90.DAT"); g_bars_ok = rs_open(&g_bars, p) == 0; }
    g_m_frame = g_m_anim = 0; g_m_dir = g_m_prev = -1; g_m_cur = 1; g_m_sel = 0; g_m_t0 = 0; g_state = S_MODE;
}
static void mode_press(int d)      /* DAT_00153718 bits: 8 -> dir 0, 0x10 -> dir 1, 1 -> dir 2, 2 -> dir 3 (so aceito com anim > 3) */
{
    if (g_m_anim > 3) { g_m_prev = g_m_dir; g_m_dir = d; g_m_anim = 0; sfx_play(SFX_MOVE, 4); }
}
static void mode_confirm(void) { if (g_m_dir != 1) g_m_dir = 0; g_m_prev = g_m_dir; }   /* centro: confirma o item apontado (padrao: o da esquerda) */
static int mode_to_sel(int dir)    /* 0x2C328 / 0x2C3DA: item (dir + cur) -> DAT_006E1778 */
{
    switch (dir + g_m_cur) { case 0: return ((g_sflag & 3) == 3) ? 5 : 4; case 1: return 1; case 2: return 2; case 3: return 3; case 4: return 6; }
    return 0;
}
static void mode_step(void)
{
    int remain = 30 - g_m_t0 / 60;
    if (g_m_anim == 0) {
        if (g_m_dir == 2) { if (--g_m_cur < 0) { g_m_cur = 0; g_m_anim = 0x16; } }
        else if (g_m_dir == 3) { if (++g_m_cur >= 4) { g_m_cur = 3; g_m_anim = 0x16; } }       /* DAT_006E178C - 1 = 4 */
    }
    int dir01 = g_m_dir == 0 || g_m_dir == 1;
    if (dir01) g_m_sel = mode_to_sel(g_m_dir);
    if (!((g_m_prev == g_m_dir && dir01) || remain < 1)) { g_m_anim++; g_m_frame++; g_m_t0++; return; }
    if (!dir01) g_m_dir = 0;
    g_m_sel = mode_to_sel(g_m_dir);
    { static const int F[7] = { 0, 0x10, 0x20, 0x100, 0x80, 0x40, 0x400 }; g_mode_flag |= F[g_m_sel]; }
    sfx_play(SFX_ENTER, 3);                                    /* FUN_2BD93 0x2C4A1: FUN_30FAC idx 4 canal 3 */
    bga_unload(&g_ab); bga_load_name(&g_ab, "097"); g_m_anim = 0; g_state = S_MCONF;      /* fase 3 (FUN_2C4AB): BGA 097 mostra o modo escolhido */
}
static const int MCONF_BASE[7] = { 0, 0, 120, 360, 240, 480, 600 };     /* 0x12CB25: quadro inicial na 097.BGA por DAT_006E1778 */
static void wheel_enter(void);
static void mconf_step(void)
{
    if (++g_m_anim < 0x1e) return;                              /* FUN_2C4AB: ao chegar a 30 quadros: FUN_294B9 e fase 4 */
    music_stop(); bga_unload(&g_ab); bga_unload(&g_ab2);
    static const char *const BG[7] = { "091", "091", "092", "094", "093", "095", "096" };      /* FUN_2C63F 0x2C623: modo -> DAT de fundo */
    bga_load_name(&g_ab, "098"); bga_load_name(&g_ab2, BG[g_m_sel]);
    wheel_enter();                                              /* fase 4 -> fase 5 (FUN_2C63F -> FUN_2C840): roda de musicas */
}
static void draw_mconf(void) { bga_draw(&g_ab, g_m_anim + MCONF_BASE[g_m_sel]); }
static void bar_cell(int id, int hl)                         /* FUN_2AFFF / FUN_2B04E: acha (folha, faixa) na tabela e desenha rotacionado 45 graus */
{
    for (int r = 0; r < 14; r++) for (int j = 0; j < 8; j++) if (BAR_TBL[r][j] == id) {
        char nm[16]; snprintf(nm, sizeof nm, "BAR%02d", r + 1);
        Tex *tx = g_bars_ok ? rs_tex(&g_bars, nm) : NULL; if (!tx) return;
        float v0 = j * 0.125f, v1 = (j + 1) * 0.125f;
        glPushMatrix(); glRotatef(45, 0, 0, 1);
        if (hl) quad(tx, -97, -16, 194, 32, 0.1130f, v1, 0.875f, v0); else quad(tx, 30, 0, 194, 32, 0.1130f, v1, 0.875f, v0);   /* v invertido: y para cima (FUN_2AE66) */
        glPopMatrix(); return;
    }
}
static void mode_items(void)                                 /* FUN_0002B192 */
{
    glColor4f(1, 1, 1, 1);
    glPushMatrix(); glTranslatef(0, 480, 0); glScalef(1, -1, 1);        /* estas rotinas desenham com y para cima (480 - y) */
    glPushMatrix(); glTranslatef(213, 218, 0);
    glPushMatrix();
    for (int i = 0; i < 8; i++) { int k = g_m_cur + i; if (k >= 0 && k < 5) bar_cell(mode_id(k), 0); glTranslatef(66, 0, 0); }
    glPopMatrix(); glPushMatrix();
    for (int i = -1; i > -9; i--) { int k = g_m_cur + i; glTranslatef(-66, 0, 0); if (k >= 0 && k < 5) bar_cell(mode_id(k), 0); }
    glPopMatrix(); glPopMatrix(); glPopMatrix();
}
static void mode_pulse(int id, int an)                       /* FUN_0002B459 */
{
    int p = (an / 2) % 50;
    glPushMatrix(); glTranslatef(0, 480, 0); glScalef(1, -1, 1); glTranslatef(g_m_dir == 0 ? 292.f : 358.f, 319.f, 0); glColor4f(1, 1, 1, 1); bar_cell(id, 1);
    static const float S[3][3] = { { 150, 30, 60 }, { 100, 20, 40 }, { 50, 10, 20 } };
    for (int q = 0; q < 3; q++) if (p < (int)S[q][2]) {
        float f = p / S[q][0] + 1; glPushMatrix(); glScalef(f, f, 1); glColor4f(1, 1, 1, 1 - p / S[q][1]); bar_cell(id, 1); glPopMatrix(); }
    glPopMatrix(); glColor4f(1, 1, 1, 1);
}
static void orig_text(float x, float y, const char *str)       /* FUN_00026658 / FUN_00026521: glifos 8x15 de FONT.PNG (32 por linha, celula 8x15, ASCII-0x20, espaco nao desenha), passo 8, y para cima */
{
    Tex *ft = rs_tex(&g_arrow, "font.tga"); if (!ft) return;
    glPushMatrix(); glTranslatef(0, 480, 0); glScalef(1, -1, 1);
    for (const char *q = str; *q; q++, x += 8) {
        int c = (unsigned char)*q; if (c <= 0x20 || c > 0x7f) continue;
        c -= 0x20; float u = (c % 32) * 8.f, v = (c / 32) * 15.f;
        quad(ft, x, y, 8, 15, u / ft->w, (v + 15) / ft->h, (u + 8) / ft->w, v / ft->h);
    }
    glPopMatrix();
}
static void orig_time(BgaRun *b, int n)                       /* FUN_0002A672(584, 429, n): 2 digitos de 02.TGA (celulas 48x40, 5 por linha, v0 = 162 + (d/5)*40), da direita p/ esquerda, passo -40 */
{
    Tex *t = b->ok ? rs_tex(&b->rs, "02.tga") : NULL; if (!t) return;
    if (n < 0) n = 0;
    glPushMatrix(); glTranslatef(0, 480, 0); glScalef(1, -1, 1); glColor4f(1, 1, 1, 1);
    for (int i = 0; i < 2; i++) {
        int d = n % 10; n /= 10; float u = (d % 5) * 48.f, v = 162.f + (d / 5) * 40.f;
        quad(t, 584.f - 40 * i, 429, 48, 40, u / t->w, (v + 40) / t->h, (u + 48) / t->w, v / t->h);
    }
    glPopMatrix();
}
static void draw_mode(void)
{
    int remain = 30 - g_m_t0 / 60, local = 0;
    static const int HL[7] = { 0, 3, 1, 2, 4, 6, 5 };          /* DAT_006E1778 -> camada (0x2BDDA..) */
    bga_draw_n(&g_ab, 600 + g_m_frame % 90, HL[g_m_sel]);       /* camada de destaque do modo no quadro 600..689 */
    bga_draw(&g_ab, 30);
    orig_time(&g_ab, remain);                                    /* FUN_0002A672 */
    glColor4f(1, 1, 1, 1);
    /* func_2B958: textos de cada jogador acima dos circulos SHIFT, camadas do 099.BGA (mt01..mt08, MT10/MT11) no quadro base + anim%30:
     * P1: ja entrou -> camada 13 (apontando: 14, base 420) base 360; sem credito 11 base 240; com credito 12 base 300. P2: 17 (18 apontando) base 360 / 15 base 240 / 16 base 300.
     * (ramo double 0x280: camadas 19/20 base 510 - nao tratado) */
    {
        int n30 = g_m_anim % 30, aim = g_m_dir != -1;
        for (int pl = 0; pl < 2; pl++) {
            int joined = (g_sflag >> pl) & 1, layer, base;
            if (joined) { layer = pl ? (aim ? 18 : 17) : (aim ? 14 : 13); base = aim ? 420 : 360; }
            else if (cred_avail()) { layer = pl ? 16 : 12; base = 300; }
            else { layer = pl ? 15 : 11; base = 240; }
            bga_draw_n(&g_ab, base + n30, layer);
        }
    }
    glColor4f(1, 1, 1, 1);
    for (int e = 2; e <= 3; e++) {                             /* setas SHIFT: camadas 0x1B/0x1C e 0x1E/0x1F no quadro 90 + anim */
        int c = (g_m_dir == e && g_m_anim < 0x3c) ? g_m_anim : 0;
        if (e == 2) { bga_draw_n(&g_ab, c + 90, 0x1b); bga_draw_n(&g_ab, c + 90, 0x1c); } else { bga_draw_n(&g_ab, c + 90, 0x1e); bga_draw_n(&g_ab, c + 90, 0x1f); }
    }
    if (g_m_dir == 2 && g_m_anim < 0x16) local = g_m_anim * 3 - 0x42;
    if (g_m_dir == 3 && g_m_anim < 0x16) local = -g_m_anim * 3 + 0x42;
    glPushMatrix(); glColor4f(1, 1, 1, 1); glTranslatef((float)local, 0, 0);
    mode_items();
    if (g_m_dir == 0 || g_m_dir == 1) mode_pulse(g_m_dir + g_m_cur + 2, g_m_anim);
    glTranslatef((float)(-(g_m_cur - 1) * 66), 0, 0);
    bga_draw(&g_ab, 150);
    glPopMatrix();
    if (g_m_dir == 0) bga_draw_n(&g_ab, g_m_frame % 30, 0x2e); else if (g_m_dir == 1) bga_draw_n(&g_ab, g_m_frame % 30, 0x2f);
}
/* ------------------------------------------------------------------ roda de musicas (estado 0xB fase 5: FUN_0002C840)
 * Mesma logica de entrada da escolha de modo: bits 8/0x10 apontam o cartaz esquerdo/direito (dir 0/1; repetir confirma), bits 1/2 rolam a roda
 * de 2 em 2 (dir 2/3, 22 quadros). Lista = ids de musica (DAT_006E0EE8 >> 16; aqui todas as do STAGE.CFG). Cada id e uma faixa de BARxx (FUN_2AFFF,
 * as mesmas da escolha de modo) e um recorte de capa CDxx.PNG da 90.DAT (FUN_2AE17/FUN_2AD1B: folhas 2x2 de 128x96, tabela 0x12C5D1). */
static const int CD_TBL[30][4] = {
    { 101, 102, 103, 104 }, { 105, 106, 107, 108 }, { 109, 110, 111, 328 }, { 113, 114, 115, 116 }, { 117, 118, 119, 201 }, { 202, 203, 204, 205 },
    { 206, 207, 208, 210 }, { 211, 212, 213, 214 }, { 215, 216, 217, 218 }, { 219, 220, 221, 223 }, { 224, 225, 301, 302 }, { 303, 304, 305, 306 },
    { 307, 308, 309, 310 }, { 311, 312, 313, 314 }, { 315, 316, 317, 318 }, { 319, 320, 321, 322 }, { 323, 324, 325, 326 }, { 327, 401, 402, 403 },
    { 404, 405, 406, 407 }, { 408, 409, 410, 411 }, { 412, 413, 414, 415 }, { 501, 502, 503, 504 }, { 505, 506, 507, 508 }, { 509, 510, 511, 512 },
    { 513, 514, 515, 516 }, { 517, 0, 0, 0 },
    /* linhas 26-29 (ELF 0x8121C7B/DOS idem; confirmado): ids em posicoes especificas e folha reaproveitada (CD_SHEET) */
    { 0, 226, 0, 0 }, { 0, 112, 0, 0 }, { 0, 0, 0, 329 }, { 0, 330, 0, 0 } };
static const int CD_SHEET[30] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 26, 26, 6, 11 };
static int g_w_list[256], g_w_n, g_w_pos, g_w_sec;
static int w_song(int k);
static void wheel_list(void);
static int w_idx(int k) { return g_w_n ? (((g_w_pos + k) % g_w_n) + g_w_n) % g_w_n : 0; }      /* FUN_2B09D: indice circular */
static void wheel_list(void)                                 /* FUN_2B70B: copia a secao g_w_sec do STAGE.CFG para a lista da roda */
{
    g_w_n = 0; for (int i = 0; i < g_nsec[g_w_sec] && g_w_n < 256; i++) if (g_sec[g_w_sec][i].song >= 0 && g_sec[g_w_sec][i].vis) g_w_list[g_w_n++] = i;
}
/* Codigos secretos (ELF piu: func_2a72b = FUN_2A72B, chamada por FUN_2B958 a cada toque do pad): as 8 ultimas teclas (bits 1 DL, 2 DR, 4 centro, 8 UL, 0x10 UR)
 * sao comparadas com 15 sequencias (tabela 0x81219AA do ELF / DOS equivalente). Com o modo REMIX ligado (flag 0x100):
 *   DL UL DL C UR DR UR C   (func_2a88a) e so 1 jogador  -> REMIXDB (flag 0x200, secao 4)
 *   C DL C DR C DL C DR     (func_2a8c4) e fase 6 (roda) -> REMIXCZ (flag 0x800, secao 6) */
static uint8_t g_hist[2][8];
static void wheel_enter(void);
static void code_press(int pl, int bit)                          /* FUN_2A72B (P1) / FUN_2A973 (P2): tabela 0x81219AA, 16 x 8 teclas */
{
    static const uint8_t T[16][8] = {                            /* DL=1 DR=2 C=4 UL=8 UR=0x10 */
        {4,4,4,4,4,4,4,4}, {4,4,0x10,8,4,2,1,4}, {4,4,1,4,0x10,2,4,8}, {4,4,1,4,2,4,4,4}, {4,4,2,4,4,8,4,4}, {4,4,8,4,4,4,0x10,4},
        {4,1,4,2,4,0x10,4,8}, {8,0x10,4,8,0x10,4,8,0x10}, {4,4,8,0x10,8,0x10,4,4}, {4,4,0x10,8,0x10,8,4,4}, {1,2,4,8,2,1,4,0x10},
        {8,0x10,4,1,2,4,8,0x10}, {8,0x10,2,4,1,4,2,1}, {1,8,1,4,0x10,2,0x10,4}, {4,1,4,2,4,1,4,2}, {0x10,4,1,0x10,8,4,2,8} };
    uint8_t *h = g_hist[pl]; memmove(h, h + 1, 7); h[7] = (uint8_t)bit;
    int m0 = g_mod[pl], f0 = g_mfx, k;
    for (k = 0; k < 16; k++) if (!memcmp(h, T[k], 8)) break;
    if (k > 14) return;
    int *m = &g_mod[pl];
    switch (k) {
    case 0: g_mfx = 0; *m = 0; break;                                                  /* cancelar */
    case 1: *m &= ~0x100; *m ^= 0x10; break;                                           /* 0x2A79D */
    case 2: *m ^= 0x20; break;
    case 3: *m &= ~0x206; *m ^= 8; break;
    case 4: *m &= ~0x20a; *m ^= 4; break;
    case 5: *m &= ~0x20c; *m ^= 2; break;
    case 6: *m &= ~0x100; *m ^= 0x40; break;
    case 7: *m ^= 0x80; break;
    case 8: g_mfx &= ~0x2000; g_mfx ^= 0x1000; break;
    case 9: g_mfx &= ~0x1000; g_mfx ^= 0x2000; break;
    case 10: g_mfx ^= 0x4000; break;
    case 11: *m &= ~0x50; *m ^= 0x100; break;
    case 12: *m &= ~0x0e; *m ^= 0x200; break;
    case 13: case 14:                                                                  /* so com REMIX (flag 0x100), na roda de musicas */
        if (g_state != S_WHEEL || g_m_sel != 3) break;
        if (k == 13 && (g_sflag & 3) != 3) { g_w_sec = 4; wheel_list(); sfx_play(1, 2); }
        else if (k == 14) { g_w_sec = 6; wheel_list(); sfx_play(1, 2); }
        break;
    }
    memset(h, 0, 8);                                                                   /* FUN_2A6C9 */
    if (*m != m0 || g_mfx != f0) sfx_play(1, 2);                                       /* 0x2A91B: ac_sound_play_effect(0,2,1) -> idx 1 canal 2 (inferido) */
}
static void code_key(int k)                                      /* teclas do pad -> bits; letras = P1, teclado numerico = P2 */
{
    int pl = (k >= SDLK_KP_1 && k <= SDLK_KP_9) ? 1 : 0, b = 0;
    if (k == SDLK_z || k == SDLK_KP_1) b = 1; else if (k == SDLK_c || k == SDLK_KP_3) b = 2; else if (k == SDLK_s || k == SDLK_KP_5) b = 4;
    else if (k == SDLK_q || k == SDLK_KP_7) b = 8; else if (k == SDLK_e || k == SDLK_KP_9) b = 0x10;
    if (b && (g_sflag >> pl & 1)) code_press(pl, b);                     /* so jogador que entrou */
}
static void wheel_begin(void)
{
    /* FUN_2B695 (chamada por func_2b763/func_2c63f so com DAT 0x81206CE == 0, i.e. 1o estagio do credito): torna todas as entradas visiveis de novo. Confirmado no ELF. */
    if (g_stage < 1) for (int a = 0; a < 9; a++) for (int i = 0; i < g_nsec[a]; i++) g_sec[a][i].vis = 1;
    memset(g_hist, 0, sizeof g_hist);
    /* FUN_2C63F: DAT_006E1784 = secao do STAGE.CFG pelo modo (flags 0x10 EASY, 0x20 HARD, 0x80 DOUBLE, 0x100 REMIX, 0x400 CRAZY; BATTLE 0x40 depende de DAT_0012B024: 0 EASY)
     * REMIXDB (0x200) e REMIXCZ (0x800) existem no codigo mas quem liga essas flags nao foi achado: inferido = REMIX jogado em double / crazy. */
    { static const int SECOF[7] = { 0, 0, 1, 3, 2, 0, 5 }; g_w_sec = SECOF[g_m_sel]; }
    wheel_list();
    g_w_pos = 0; g_m_frame = g_m_anim = 0; g_m_dir = g_m_prev = -1; g_m_t0 = 0; g_state = S_WHEEL;     /* 0x2C63F: 17A4=0, 1794=1788=-1, 179C=0, fase 5 */
}
/* Tela LOAD n (FUN_2C63F, 0x2C689..0x2C6A2): apos o DANCE GRADE, ao voltar a roda de musicas, TITLE\LOAD%d.PNZ (n = estagios jogados, DAT 0x81206CE) via FUN_2A533 (mesma rotina da T%03d).
 * LOAD1 e um anuncio com data 2000.12.27 (FUN_2D3DF: data > 20001227 -> usa LOAD4 no lugar), LOAD2 Banya, LOAD3 "TRY BONUS STAGE", LOAD4 "TRY NEXT STAGE". Duracao inferida (~1,5 s). */
static Tex g_ldtex; static int g_ldn, g_ldt;
static void wheel_enter(void)
{
    music_stop();                                                      /* FUN_2C63F chama func_294b9 (parar musica) ao entrar na roda */
    int n = g_stage; if (n < 1) { wheel_begin(); return; }            /* 0x2C663: DAT 0x81206CE (estagios jogados) == 0 -> sem tela LOAD (FUN_2B695, vindo do modo) */
    if (n == 1) { time_t tt = time(NULL); struct tm *lt = localtime(&tt); if (lt && (lt->tm_year + 1900) * 10000 + (lt->tm_mon + 1) * 100 + lt->tm_mday > 20001227) n = 4; }   /* FUN_2D3DF: apos 27/12/2000 o anuncio (LOAD1) e trocado por LOAD4 */
    if (n > 4) n = 4;
    if (g_ldtex.id) { glDeleteTextures(1, &g_ldtex.id); g_ldtex.id = 0; }
    char p[300]; size_t len; int ok; uint8_t *b;
    path_of(p, sizeof p, "TITLE/LOAD%d.PNZ", n); b = piu_unpack_file(p, &len, &ok);
    if (!b) { path_of(p, sizeof p, "TITLE/Load%d.pnz", n); b = piu_unpack_file(p, &len, &ok); }
    if (!b) { wheel_begin(); return; }
    g_ldtex = tex_from_png(b, len); free(b); g_ldn = n; g_ldt = 0; g_state = S_WLOAD;
}
static void wload_step(void) { if (++g_ldt >= 90) wheel_begin(); }
static void draw_wload(void) { glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT); glColor4f(1, 1, 1, 1); if (g_ldtex.id) quad(&g_ldtex, 0, 0, 640, 480, 0, 0, 1, 1); }
static int w_song(int k) { return g_sec[g_w_sec][g_w_list[w_idx(k)]].song; }
static void cover_cell(int id)                               /* FUN_2AE17 + FUN_2AD1B: quadro 200x142 centrado, recorte (j%2, j/2) da folha */
{
    for (int r = 0; r < 30; r++) for (int j = 0; j < 4; j++) if (CD_TBL[r][j] == id) {
        char nm[16]; snprintf(nm, sizeof nm, "CD%02d", CD_SHEET[r]);
        Tex *tx = g_bars_ok ? rs_tex(&g_bars, nm) : NULL; if (!tx) return;
        int c = j % 2, w = j / 2;
        quad(tx, -100, -71, 200, 142, c * 0.5f, (w + 1) * 0.375f, (c + 1) * 0.5f, w * 0.375f);
        return;
    }
}
static void cover_slot(int side, int an)                     /* FUN_0002B266: capa em x=161|481, y=320 (y para cima) + 3 pulsos ao apontar */
{
    int p = (an / 2) % 50, id = g_songs[w_song(side)].id;
    glPushMatrix(); glTranslatef(0, 480, 0); glScalef(1, -1, 1); glTranslatef(side == 0 ? 161.f : 481.f, 320.f, 0);
    glColor4f(1, 1, 1, 1); cover_cell(id);
    if (p) {
        static const float S[3][3] = { { 150, 30, 60 }, { 100, 20, 40 }, { 50, 10, 20 } };       /* 0x1205B3..0x1205C7 */
        for (int q = 0; q < 3; q++) if (p < (int)S[q][2]) {
            float f = p / S[q][0] + 1; glPushMatrix(); glScalef(f, f, 1); glColor4f(1, 1, 1, 1 - p / S[q][1]); cover_cell(id); glPopMatrix(); }
    }
    glPopMatrix(); glColor4f(1, 1, 1, 1);
}
static void wheel_items(void)                                /* FUN_0002B0DD: como mode_items, 8 a direita e 8 a esquerda, indice circular */
{
    glColor4f(1, 1, 1, 1);
    glPushMatrix(); glTranslatef(0, 480, 0); glScalef(1, -1, 1);
    glPushMatrix(); glTranslatef(213, 218, 0);
    glPushMatrix();
    for (int i = 0; i < 8; i++) { bar_cell(g_songs[w_song(i)].id, 0); glTranslatef(66, 0, 0); }
    glPopMatrix(); glPushMatrix();
    for (int i = -1; i > -9; i--) { glTranslatef(-66, 0, 0); bar_cell(g_songs[w_song(i)].id, 0); }
    glPopMatrix(); glPopMatrix(); glPopMatrix();
}
static int g_load_di, g_load_t;
static void load_back(int id);
static void wheel_step(void)
{
    int remain = 30 - g_m_t0 / 60;
    if (g_m_anim == 0) {
        if (g_m_dir == 2) { g_w_pos -= 2; if (g_w_pos < 0) g_w_pos += g_w_n; }
        else if (g_m_dir == 3) { g_w_pos += 2; if (g_w_pos >= g_w_n) g_w_pos -= g_w_n; }
    }
    if (!((g_m_prev == g_m_dir && g_m_prev != -1 && g_m_dir < 2) || remain < 1)) { g_m_anim++; g_m_frame++; g_m_t0++; return; }
    if (g_m_dir == -1 || g_m_dir > 1) g_m_dir = 0;
    music_stop();                                     /* fim do preview (Dxxx.AUD) ao escolher */
    { SEnt *cur = &g_sec[g_w_sec][g_w_list[w_idx(g_m_dir)]];      /* FUN_2B6BC(secao atual, id) em func_2cdfa: esconde da roda a musica escolhida (so nesta secao) ate o proximo credito */
      for (int i = 0; i < g_nsec[g_w_sec]; i++) if (g_sec[g_w_sec][i].id == cur->id) g_sec[g_w_sec][i].vis = 0; }
    g_sel = w_song(g_m_dir);                          /* DAT_006E17A0 = lado escolhido; fase 7 (nao lida ainda): lista antiga de dificuldade */
    sfx_play(SFX_ENTER, 3);                                    /* 0x2CDE3: FUN_30FAC idx 4 canal 3 */
    g_m_frame = 0;
    /* Fase 7 (FUN_2CDFA): escolhe o arquivo de passos pelas flags de modo e entra direto na gameplay ("run %s %s %d"). Nao ha outra tela de selecao.
     * Flags: EASY 0x10, HARD 0x20, REMIX 0x100, DOUBLE 0x80, CRAZY 0x400, REMIXDB 0x200 (secao 4), REMIXCZ 0x800 (secao 6). 1P: 0x100|0x20 -> H1, 0x400|0x800 -> H3, resto E1;
     * 2P ((flag&3)==3): 0x100|0x20 -> H2, 0x400|0x800 -> H4, resto E2; DOUBLE/REMIXDB (0x280) -> DB. (ramo 0x4000 de batalha nao tratado.) */
    { static const int F[7] = { 0, 0x10, 0x20, 0x100, 0x80, 0x40, 0x400 };
      int fl = F[g_m_sel], two = (g_sflag & 3) == 3, di;
      if (g_m_sel == 3 && g_w_sec == 4) fl = 0x200; else if (g_w_sec == 6) fl = 0x800;
      if (fl & 0x280) di = 6;
      else if (fl == 0x100 || fl == 0x20) di = two ? 4 : 1;
      else if (fl == 0x400 || fl == 0x800) di = two ? 5 : 2;
      else di = two ? 3 : 0;
      g_fl = fl; bga_unload(&g_ab); bga_unload(&g_ab2); g_load_di = di; g_load_t = 0; g_state = S_LOAD; }       /* tela de loading (T%03d.PNZ), depois start_song */
}
static void load_step(void)            /* FUN_2A533: carrega e desenha TITLE\T%03d.PNZ da musica; depois FUN_2058F inicia o jogo. Duracao: empirico (~1,5 s) */
{
    if (++g_load_t < 90) return;
    start_song(g_load_di);                                   /* a tela antiga de selecao (S_SELECT) foi desativada */
    if (g_state == S_LOAD) goto_idle();                      /* sem o arquivo de passos: volta ao attract */
}
static void draw_load(void)
{
    load_back(g_songs[g_sel].id);
    glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT); glColor4f(1, 1, 1, 1);
    if (g_backtex.id) quad(&g_backtex, 0, 0, 640, 480, 0, 0, 1, 1);
}
static void draw_wheel(void)
{
    int remain = 30 - g_m_t0 / 60, local = 0;
    glColor4f(1, 1, 1, 1);
    glPushMatrix(); glTranslatef(320, 240, 0); glRotatef(g_m_frame * 0.06f, 0, 0, 1); glTranslatef(-320, -240, 0);   /* fundo colorido gira devagar */
    bga_draw(&g_ab2, (g_m_frame * 2) % 90);
    glPopMatrix();
    bga_draw(&g_ab, 0);
    for (int e = 2; e <= 3; e++) {                             /* setas SHIFT: camadas 0x1B/0x1C e 0x1E/0x1F no quadro 90 + anim */
        int c = ((g_m_dir == e && g_m_anim < 0x5a) ? g_m_anim : 0) + 0x5a;
        if (e == 2) { bga_draw_n(&g_ab, c, 0x1b); bga_draw_n(&g_ab, c, 0x1c); } else { bga_draw_n(&g_ab, c, 0x1e); bga_draw_n(&g_ab, c, 0x1f); }
    }
    if (g_m_dir == 2 && g_m_anim < 0x16) local = g_m_anim * 6 - 0x84;
    if (g_m_dir == 3 && g_m_anim < 0x16) local = -g_m_anim * 6 + 0x84;
    glPushMatrix(); glTranslatef((float)local, 0, 0); wheel_items(); glPopMatrix();
    if (g_m_dir == 0 || g_m_dir == 1) {
        bga_draw_n(&g_ab, 0x96, 0x23); bga_draw_n(&g_ab, 0x96, 0x2b); bga_draw_n(&g_ab, 0x96, 0x2c);
        cover_slot(0, 0); cover_slot(1, 0); cover_slot(g_m_dir, g_m_anim);
        glColor4f(1, 1, 1, 1);
        { char b[32]; snprintf(b, sizeof b, "%5.1f", g_songs[w_song(g_m_dir)].bpm); orig_text(0x128, 0x63, b);      /* FUN_2C5CA: "%5.1f" em (0x128, 0x63), y para cima */
          snprintf(b, sizeof b, "%d", g_sec[g_w_sec][g_w_list[w_idx(g_m_dir)]].lv); orig_text(316.f - 4.f * (float)strlen(b), 0x46, b); }      /* centralizado no mesmo eixo do BPM (296 + 5*8/2 = 316) para 1 ou 2 digitos */      /* nivel, "%d" em (0x130,0x46) (inferido da fase anterior) */
    }
    orig_time(&g_ab, remain);                                      /* FUN_0002A672 */
    glColor4f(1, 1, 1, 1);
}
static void draw_attract(void)
{
    glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
    if (g_ab.ok) bga_draw(&g_ab, g_bgaf);
    if (g_state == S_TITLE) {
        draw_title_overlay();
        /* 0x28756..0x287B2: printf("%d%02d", 3, 0x34) em (601,458) preto e (600,459) branco -> "352" */
        for (int pass = 0; pass < 2; pass++) {
            Tex *tx = rs_tex(&g_arrow, "FONT"); if (!tx) break;
            const char *s = "352"; float x = pass ? 600 : 601, y = pass ? 459 : 458; glColor4f(pass, pass, pass, 1);
            for (; *s; s++, x += 10) quad(tx, x, y, 16, 16, (16 * (*s - '0')) / (float)tx->w, 96 / (float)tx->h, (16 * (*s - '0') + 16) / (float)tx->w, 112 / (float)tx->h);
        }
        glColor4f(1, 1, 1, 1);
    }
}

static void load_back(int id)
{
    if (g_backid == id) return;
    if (g_backtex.id) { glDeleteTextures(1, &g_backtex.id); g_backtex.id = 0; }
    g_backid = id;
    char p[300]; path_of(p, sizeof p, "TITLE/T%03d.PNZ", id);
    size_t len; int ok; uint8_t *b = piu_unpack_file(p, &len, &ok);
    if (b) { g_backtex = tex_from_png(b, len); free(b); }
}
static int key_to_col(SDL_Keycode k, int dbl)
{
    switch (k) {
    case SDLK_z: return 0; case SDLK_q: return 1; case SDLK_s: return 2; case SDLK_e: return 3; case SDLK_c: return 4;
    case SDLK_KP_1: return dbl ? 5 : 0; case SDLK_KP_7: return dbl ? 6 : 1; case SDLK_KP_5: return dbl ? 7 : 2;
    case SDLK_KP_9: return dbl ? 8 : 3; case SDLK_KP_3: return dbl ? 9 : 4;   /* P2 = numpad (ELF: player2keys 0x147,0x149,0x145,0x141,0x143 = KP7,KP9,KP5,KP1,KP3) */
    }
    return -1;
}
static void set_view(void)
{
    int w = g_winw, h = g_winh; float a = 640.0f / 480.0f; int vw = w, vh = (int)(w / a);
    if (vh > h) { vh = h; vw = (int)(h * a); }
    glViewport((w - vw) / 2, (h - vh) / 2, vw, vh);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0, 640, 480, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

int main(int argc, char **argv)
{
#ifdef _WIN32
    {   /* sem janela de console (debug) junto com o jogo: libera o console se o stderr for ele proprio (duplo clique/jogar.bat). Com redirecao (testes/CI) ou -debug, mantem. */
        extern __declspec(dllimport) void *__stdcall GetStdHandle(unsigned long);
        extern __declspec(dllimport) unsigned long __stdcall GetFileType(void *);
        extern __declspec(dllimport) int __stdcall FreeConsole(void);
        int dbg = 0; for (int i = 1; i < argc; i++) if (!strcmp(argv[i], "-debug")) dbg = 1;
        if (!dbg && GetFileType(GetStdHandle((unsigned long)-12)) == 2 /* FILE_TYPE_CHAR */) FreeConsole();
    }
#endif
    int end_test = 0, end_testc = 0; int wheel_start = 0, wheel_dir = -1, conf_start = 0, mode_start = 0, svc_start = 0, autostart = -1, autodiff = 1, autotest = 0, testsec = 6; const char *shotpath = NULL; int noauto = 0, skip = 0; const char *viewname = NULL;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-game") && i + 1 < argc) snprintf(g_dir, sizeof g_dir, "%s", argv[++i]);
        else if (!strcmp(argv[i], "-song") && i + 1 < argc) autostart = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-diff") && i + 1 < argc) autodiff = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-selftest") && i + 1 < argc) { autotest = 1; testsec = atoi(argv[++i]); }
        else if (!strcmp(argv[i], "-noauto")) noauto = 1;
        else if (!strcmp(argv[i], "-skip")) skip = 1;
        else if (!strcmp(argv[i], "-conf") && i + 1 < argc) conf_start = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-mode") && i + 1 < argc) mode_start = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-mod") && i + 1 < argc) { g_mod[0] = (int)strtol(argv[++i], NULL, 0); g_mod[1] = g_mod[0]; g_mfx = 0x2000; }       /* teste: modificadores de P1/P2 */
        else if (!strcmp(argv[i], "-wheel") && i + 1 < argc) wheel_start = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-wdir") && i + 1 < argc) wheel_dir = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-service") && i + 1 < argc) svc_start = atoi(argv[++i]) + 1;
        else if (!strcmp(argv[i], "-view") && i + 1 < argc) viewname = argv[++i];
        else if (!strcmp(argv[i], "-frame") && i + 1 < argc) g_bgaf = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-aoff") && i + 1 < argc) g_aoff_ms = atof(argv[++i]);
        else if (!strcmp(argv[i], "-end") && i + 1 < argc) end_test = atoi(argv[++i]);          /* teste: abre direto a fase do fim (1 FAILED 2 CLEARED 3 OVER 4 BREAK 7 RESULT) com dados de exemplo */
        else if (!strcmp(argv[i], "-endc") && i + 1 < argc) end_testc = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-autostart1")) g_auto1 = 1;
        else if (!strcmp(argv[i], "-freeplay")) { g_freeplay = 1; g_cli_free = 1; }
        else if (!strcmp(argv[i], "-coins") && i + 1 < argc) g_coins = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-shot") && i + 1 < argc) shotpath = argv[++i];
    }
    { char t[320]; snprintf(t, sizeof t, "%s/STAGE.CFG", g_dir); FILE *f = fopen(t, "rb");
      if (f) fclose(f); else { char *bp = SDL_GetBasePath(); if (bp) { snprintf(g_dir, sizeof g_dir, "%s../game", bp); SDL_free(bp); } } }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) { logf_("SDL: %s\n", SDL_GetError()); return 1; }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    g_win = SDL_CreateWindow("Pump It Up (PIU.EXE reconstruido)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_winw, g_winh, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!g_win) { logf_("janela: %s\n", SDL_GetError()); return 1; }
    SDL_GLContext ctx = SDL_GL_CreateContext(g_win); SDL_GL_SetSwapInterval(1);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDisable(GL_DEPTH_TEST);
    font_init(); audio_init(); songs_load(); svc_load(); if (conf_start) { g_m_sel = conf_start; bga_load_name(&g_ab, "097"); g_m_anim = 0; g_state = S_MCONF; }
    if (mode_start) { g_sflag = mode_start == 2 ? 3 : 1; mode_enter(); }
    if (end_test) { g_ps[0].score = 1234567; g_ps[0].life = 940; g_ps[0].cnt[0] = 120; g_ps[0].cnt[1] = 45; g_ps[0].cnt[2] = 12; g_ps[0].cnt[3] = 3; g_ps[0].cnt[4] = 7; g_mcp[0] = 88; g_fl = 0x20; g_m_sel = 2; g_sflag = 1; end_start(end_test); g_pc = end_testc; }
    if (wheel_start) {                                          /* teste: abre direto a roda (-wheel MODO [-wdir 0|1|2|3]) */
        static const char *const BG[7] = { "091", "091", "092", "094", "093", "095", "096" };
        char bp[300]; path_of(bp, sizeof bp, "BGA/90.DAT"); g_bars_ok = rs_open(&g_bars, bp) == 0;
        g_m_sel = wheel_start; bga_load_name(&g_ab, "098"); bga_load_name(&g_ab2, BG[g_m_sel]); wheel_enter();
        if (wheel_dir >= 0) { g_m_dir = wheel_dir; g_m_anim = 10; }
    }
    if (svc_start) { svc_open(); g_svs = svc_start - 1; g_svcur = g_svs ? g_svs - 1 : 0; }
    char p[300]; path_of(p, sizeof p, "BGA/00.DAT");
    if (rs_open(&g_arrow, p)) { logf_("BGA/00.DAT nao encontrado em %s\n", g_dir); }
    else { const PiuResEntry *e = piu_res_find(&g_arrow.res, "ARROW541.SP2"); if (e) g_sp541 = spr_parse(e->data, e->size); e = piu_res_find(&g_arrow.res, "ARROW542.SP2"); if (e) g_sp542 = spr_parse(e->data, e->size);
        g_spd[0] = g_sp542; g_spd[1] = g_sp541; const char *N3[3] = { "ARROW545.SP2", "ARROW543.SP2", "ARROW544.SP2" }; for (int k = 0; k < 3; k++) { e = piu_res_find(&g_arrow.res, N3[k]); if (e) g_spd[2 + k] = spr_parse(e->data, e->size); } }
    if (!g_nsongs) { logf_("STAGE.CFG sem musicas (%s)\n", g_dir); return 1; }
    if (viewname) { bga_load_name(&g_ab, viewname); g_state = S_VIEW; }
    if (autostart >= 0) { for (int i = 0; i < g_nsongs; i++) if (g_songs[i].id == autostart) g_sel = i; start_song(autodiff); }
    uint32_t t_end = autotest ? SDL_GetTicks() + testsec * 1000 : 0; int running = 1;
    double acc = 0; uint32_t t_prev = SDL_GetTicks();
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) SDL_GetWindowSize(g_win, &g_winw, &g_winh);
            else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
                SDL_Keycode k = e.key.keysym.sym;
                if (k == SDLK_F11) SDL_SetWindowFullscreen(g_win, (SDL_GetWindowFlags(g_win) & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                if (k == SDLK_x && (e.key.keysym.mod & KMOD_ALT)) running = 0;      /* 0x20F37: Alt+X sai */
                /* Teclas de arcade (pedido do usuario): F1 service, F2 botao 2 (test/next), F3 coin clear, F4 moeda */
                if (k == SDLK_F1) { if (!g_service) { svc_halt(); svc_open(); } else svc_press(0, 1); g_svio |= 2; }       /* SERVICE (0x20000) */
                else if (k == SDLK_F2) { if (g_service) svc_press(1, 0); g_svio |= 1; }                      /* TEST (0x10000) */
                else if (k == SDLK_F3) { g_coins = 0; g_svio |= 4; }
                else if (k == SDLK_F4 || k == SDLK_F5 || (0 /* '5' agora e centro do P2 */)) { if (!g_freeplay) { g_coins++; sfx_play(SFX_COIN, 0); } g_svio |= 8; }   /* 0x20470: FUN_30fac idx 0x15 */   /* 0x20424: moeda */
                if (g_service) { if (k == SDLK_ESCAPE) svc_leave(); continue; }
                if (g_state >= S_IDLE) {
                    if (k == SDLK_ESCAPE) { bga_unload(&g_ab); g_state = S_IDLE; }     /* 0x20F37: ESC -> estado 0 */
                    else if (k == SDLK_s || k == SDLK_RETURN || k == SDLK_SPACE) g_start1 = 1;        /* PRESS CENTER STEP: S = centro do P1 */
                    else if (k == SDLK_KP_5 || k == SDLK_KP_ENTER || k == SDLK_KP_PLUS) g_start2 = 1;   /* centro do P2 = Num 5 */
                } else if (g_state == S_MCONF) {
                    if (k == SDLK_ESCAPE) { music_stop(); bga_unload(&g_ab); g_state = S_IDLE; }
                } else if (g_state == S_WHEEL) {
                    if (k == SDLK_ESCAPE) { music_stop(); bga_unload(&g_ab); bga_unload(&g_ab2); g_state = S_IDLE; }
                    code_key(k);
                    if (k == SDLK_ESCAPE) {}
                    else if (k == SDLK_z || k == SDLK_KP_1 || k == SDLK_LEFT) mode_press(2);        /* mesmas teclas da escolha de modo */
                    else if (k == SDLK_c || k == SDLK_KP_3 || k == SDLK_RIGHT) mode_press(3);
                    else if (k == SDLK_q || k == SDLK_KP_7 || k == SDLK_e || k == SDLK_KP_9) {
                        int d = (k == SDLK_q || k == SDLK_KP_7) ? 0 : 1, old = g_m_dir; mode_press(d);
                        if (g_m_dir == d && g_m_anim == 0 && old != d && music_load_pfx("D", g_songs[w_song(d)].id)) music_start();   /* UL/UR: preview Dxxx.AUD do cartaz apontado (empirico) */
                    }
                    else if (k == SDLK_RETURN || k == SDLK_SPACE) mode_confirm();
                } else if (g_state == S_MODE) {
                    code_key(k);
                    if (k == SDLK_ESCAPE) { music_stop(); bga_unload(&g_ab); g_state = S_IDLE; }
                    /* PIU.EXE FUN_2BD93: bits 1/2 rolam a lista (dir 2/3), bits 8/0x10 apontam cur+0/cur+1 (dir 0/1; repetir entra); centro nao e lido.
                     * Bits do pad confirmados na pratica pelo usuario: 1/2 = baixo-esq/baixo-dir (azuis Z/C), 8/0x10 = cima-esq/cima-dir (Q/E).
                     * (A tabela de teclas do ELF Linux nao serve para isso: e o mapeamento do porte, nao dos bits do DOS.) */
                    else if (k == SDLK_z || k == SDLK_KP_1 || k == SDLK_LEFT) mode_press(2);
                    else if (k == SDLK_c || k == SDLK_KP_3 || k == SDLK_RIGHT) mode_press(3);
                    else if (k == SDLK_q || k == SDLK_KP_7) mode_press(0);
                    else if (k == SDLK_e || k == SDLK_KP_9) mode_press(1);
                    else if (k == SDLK_RETURN || k == SDLK_SPACE) mode_confirm();       /* atalho extra (nao e do original) */
                } else if (g_state == S_PLAY) {
                    if (k == SDLK_ESCAPE) { music_stop(); g_state = S_IDLE; }
                    else on_press(key_to_col(k, g_dbl || g_two));
                } else if (g_state == S_END) {
                    if (k == SDLK_ESCAPE) { g_stage = 0; g_sflag = 0; mods_clear(); goto_idle(); }
                    else if (k == SDLK_s || k == SDLK_KP_5 || k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_SPACE) g_endkey = 1;
                }
            }
        }
        if (g_state == S_PLAY) {
            if (autotest && !noauto) {   /* autoplay do self-test: acerta toda nota na hora */
                int32_t pos = music_pos(); int slot = slot_for(pos); PiuClock ck = clock_for(slot);
                int row = piu_row_now(&ck, pos, 0); if (row < g_prevrow) row = g_prevrow;
                for (int c = 0; c < 10; c++) if (g_chart.cell[row * PIU_ROWSTRIDE + c] == '9' && abs((row * 60) / ck.n - piu_c18(&ck, pos)) <= 3) on_press(c);
            }
            play_update();
        }
        { uint32_t now = SDL_GetTicks(); acc += now - t_prev; t_prev = now; if (acc > 250) acc = 250;
          if (g_service) acc = 0; while (acc >= 1000.0 / 60.0) { acc -= 1000.0 / 60.0; if (g_state >= S_IDLE) attract_step(); else if (g_state == S_MODE) mode_step(); else if (g_state == S_MCONF) mconf_step(); else if (g_state == S_WHEEL) wheel_step(); else if (g_state == S_LOAD) load_step(); else if (g_state == S_WLOAD) wload_step(); else if (g_state == S_END) end_step(); } }
        set_view(); glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT); glColor4f(1, 1, 1, 1);
        if (g_state >= S_IDLE) draw_attract();
        else if (g_state == S_MODE) draw_mode(); else if (g_state == S_MCONF) draw_mconf(); else if (g_state == S_WHEEL) draw_wheel(); else if (g_state == S_LOAD) draw_load(); else if (g_state == S_WLOAD) draw_wload(); else if (g_state == S_END) draw_end(); else if (g_state == S_PLAY) draw_play();
        if (g_state == S_MODE || g_state == S_WHEEL || g_state == S_PLAY) draw_mods();
        if (g_service) svc_draw();
        if (autotest && shotpath && SDL_GetTicks() + 150 > t_end) {
            uint8_t *px = (uint8_t *)malloc((size_t)g_winw * g_winh * 3); FILE *f = fopen(shotpath, "wb");
            glReadPixels(0, 0, g_winw, g_winh, GL_RGB, GL_UNSIGNED_BYTE, px);
            fprintf(f, "P6 %d %d 255 ", g_winw, g_winh);
            for (int y = g_winh - 1; y >= 0; y--) fwrite(px + (size_t)y * g_winw * 3, 3, g_winw, f);
            fclose(f); free(px); shotpath = NULL;
        }
        SDL_GL_SwapWindow(g_win);
        if (autotest && SDL_GetTicks() > t_end) {
            logf_("selftest: estado=%d pos=%d row=%d score=%d combo=%d P%d G%d Go%d B%d M%d\n", g_state, g_pos_cache, g_prevrow, g_pl.score, g_pl.combo, g_pl.cnt[0], g_pl.cnt[1], g_pl.cnt[2], g_pl.cnt[3], g_pl.cnt[4]);
            running = 0;
        }
    }
    music_stop(); SDL_GL_DeleteContext(ctx); SDL_DestroyWindow(g_win); SDL_Quit();
    (void)g_sp541; (void)g_sp542; (void)g_back; (void)sp2_find;
    return 0;
}
