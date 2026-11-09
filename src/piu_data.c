/* Leitores de formatos: cifras, RES, NOT, BGA. Ver docs/FUNCTION_MAP.md (Atualizacoes 8-10) e docs/NOT_FORMAT.md */
#include "piu.h"
#include "piu_key.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t piu_adler32(const uint8_t *p, size_t n)             /* 0x298D1 com seed 1 */
{
    uint32_t a = 1, b = 0;
    for (size_t i = 0; i < n; i++) { a = (a + p[i]) % 0xFFF1; b = (b + a) % 0xFFF1; }
    return (b << 16) + a;
}

void piu_xor_d2(uint8_t *p, size_t n)
{
    uint8_t k = 0xD2;
    for (size_t i = 0; i < n; i++) { p[i] ^= k; k = (uint8_t)(k + 0x95); }
}

static uint8_t *slurp(const char *path, size_t *n)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *b = (uint8_t *)malloc(sz ? sz : 1);
    if (!b) { fclose(f); return NULL; }
    if (fread(b, 1, sz, f) != (size_t)sz) { free(b); fclose(f); return NULL; }
    fclose(f);
    *n = (size_t)sz;
    return b;
}

static uint8_t bitrev8(uint8_t v)                              /* 0x28934 */
{
    uint8_t r = 0;
    for (int i = 0; i < 8; i++) if (v & (0x80 >> i)) r |= (uint8_t)(1 << i);
    return r;
}

uint8_t *piu_unpack_file(const char *path, size_t *len, int *adler_ok)
{
    size_t n;
    uint8_t *b = slurp(path, &n);
    if (!b || n < 4) { free(b); return NULL; }
    uint32_t h;
    memcpy(&h, b, 4);
    size_t m = n - 4;
    for (size_t i = 0; i < m; i++) b[i] = bitrev8(b[4 + i]) ^ PIU_KEY[(h + i) & 0x3FF];   /* 0x2896A */
    if (adler_ok) *adler_ok = (piu_adler32(b, m) == h);
    *len = m;
    return (uint8_t *)realloc(b, m ? m : 1);
}

/* ---- RES ---- */
int piu_res_load(const char *path, PiuRes *r)
{
    memset(r, 0, sizeof *r);
    size_t n;
    uint8_t *d = slurp(path, &n);
    if (!d || n < 24) { free(d); return -1; }
    uint32_t magic, ver, cnt, dsz, unk, adl;
    memcpy(&magic, d, 4); memcpy(&ver, d + 4, 4); memcpy(&cnt, d + 8, 4);
    memcpy(&dsz, d + 12, 4); memcpy(&unk, d + 16, 4); memcpy(&adl, d + 20, 4);
    if (magic != 0x00534552u || (size_t)24 + 28u * cnt + dsz > n) { free(d); return -2; }
    uint8_t *dir = (uint8_t *)malloc(28u * cnt + 1);
    memcpy(dir, d + 24, 28u * cnt);
    piu_xor_d2(dir, 28u * cnt);
    uint8_t *data = d + 24 + 28u * cnt;
    r->data_adler_ok = (piu_adler32(data, dsz) == adl);        /* adler sobre dados ainda cifrados (0x29B1C) */
    r->count = (int)cnt;
    r->e = (PiuResEntry *)calloc(cnt ? cnt : 1, sizeof(PiuResEntry));
    for (uint32_t i = 0; i < cnt; i++) {
        uint8_t *q = dir + 28 * i;
        uint32_t sz, off, a;
        memcpy(&sz, q + 16, 4); memcpy(&off, q + 20, 4); memcpy(&a, q + 24, 4);
        memcpy(r->e[i].name, q, 16);
        r->e[i].name[16] = 0;
        if ((uint64_t)off + sz > dsz) continue;
        r->e[i].data = (uint8_t *)malloc(sz ? sz : 1);
        memcpy(r->e[i].data, data + off, sz);
        piu_xor_d2(r->e[i].data, sz);
        r->e[i].size = sz;
        r->e[i].adler_ok = (piu_adler32(r->e[i].data, sz) == a);
    }
    free(dir);
    free(d);
    return 0;
}

void piu_res_free(PiuRes *r)
{
    for (int i = 0; i < r->count; i++) free(r->e[i].data);
    free(r->e);
    memset(r, 0, sizeof *r);
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

const PiuResEntry *piu_res_find(const PiuRes *r, const char *name)
{
    for (int i = 0; i < r->count; i++) if (ieq(r->e[i].name, name)) return &r->e[i];
    return NULL;
}

/* ---- NOT ---- */
int piu_chart_load(const char *path, PiuChart *c)
{
    size_t n;
    uint8_t *b = slurp(path, &n);
    if (!b || n < 0x88) { free(b); return -1; }
    memset(c, 0, sizeof *c);
    memcpy(&c->count, b, 4);
    memcpy(c->bpm, b + 8, 16);
    memcpy(c->sync, b + 0x18, 16);
    memcpy(c->third, b + 0x28, 16);
    memcpy(&c->n_per_beat, b + 0x38, 4);
    memcpy(&c->h3c, b + 0x3C, 4);
    memcpy(&c->h40, b + 0x40, 4);
    memcpy(c->title, b + 0x44, 0x44 - 1);
    if (n < 0x88 + 4u * c->count) { free(b); return -2; }
    piu_xor_d2(b + 0x88, 4u * c->count);                       /* 0x29949 */
    memset(c->cell, '0', sizeof c->cell);
    for (uint32_t i = 0; i < c->count; i++) {                  /* 0x21251 */
        uint16_t row, mask;
        memcpy(&row, b + 0x88 + 4 * i, 2);
        memcpy(&mask, b + 0x88 + 4 * i + 2, 2);
        if (row >= PIU_ROWS) continue;
        for (int bit = 15; bit >= 3; bit--)
            if (mask & (1u << bit)) c->cell[row * PIU_ROWSTRIDE + (15 - bit)] = '1';
    }
    for (int r = 0; r < PIU_ROWS; r++)                         /* 0x21361: '1' -> '9' nas colunas 0..9 */
        for (int col = 0; col < 10; col++) {
            uint8_t *p = &c->cell[r * PIU_ROWSTRIDE + col];
            if (*p == '1') *p = '9';
        }
    free(b);
    return 0;
}

/* ---- BGA ---- */
int piu_bga_parse(const uint8_t *b, size_t len, PiuBga *g)
{
    memset(g, 0, sizeof *g);
    if (len < 16 || memcmp(b, "BGA\0", 4)) return -1;
    memcpy(g->h, b + 4, 12);
    size_t o = 16;
    for (int k = 0; k < 50 && o + 68 <= len; k++) {
        PiuBgaLayer *l = &g->layer[k];
        memcpy(l->name, b + o, 64);
        l->name[64] = 0;
        memcpy(&l->n, b + o + 64, 4);
        o += 68;
        if (o + 44ull * l->n > len) return -2;
        l->k = (PiuBgaKey *)calloc(l->n ? l->n : 1, sizeof(PiuBgaKey));
        for (uint32_t i = 0; i < l->n; i++) {
            const uint8_t *q = b + o + 44 * i;
            PiuBgaKey *t = &l->k[i];
            memcpy(&t->time, q, 2);
            memcpy(&t->active, q + 2, 2);
            memcpy(&t->x, q + 4, 4 * 10);
        }
        o += 44ull * l->n;
        g->nlayers = k + 1;
    }
    return o == len ? 0 : 1;
}

void piu_bga_free(PiuBga *g)
{
    for (int i = 0; i < 50; i++) free(g->layer[i].k);
    memset(g, 0, sizeof *g);
}

static float lerpf(float a, float b, float t) { return a + (b - a) * t; }   /* 0x25F88 */

int piu_bga_eval(const PiuBgaLayer *l, int f, PiuBgaKey *out)               /* 0x25FA5 */
{
    if (!l->n || f < l->k[0].time) return 0;
    for (uint32_t i = 0; i < l->n; i++) {
        if (l->k[i].time <= f) continue;
        if (i == 0) { if (!l->k[0].active) return 0; *out = l->k[0]; return 1; }
        const PiuBgaKey *p = &l->k[i - 1], *c = &l->k[i];
        if (!p->active) return 0;
        float t = (float)(f - p->time) / (float)(c->time - p->time);
        out->time = (int16_t)f; out->active = p->active;
        out->x = lerpf(p->x, c->x, t);       out->y = lerpf(p->y, c->y, t);
        out->px = p->px;                     out->py = p->py;       /* 0x260D9..: o pivo NAO e interpolado (usa o da chave anterior) */
        out->scale = lerpf(p->scale, c->scale, t); out->rot = lerpf(p->rot, c->rot, t);
        out->r = lerpf(p->r, c->r, t);       out->g = lerpf(p->g, c->g, t);
        out->b = lerpf(p->b, c->b, t);       out->a = lerpf(p->a, c->a, t);
        return 1;
    }
    return 2;
}
