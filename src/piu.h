/* PiuReconstructed - nucleo (sem dependencias). Fonte: docs/FUNCTION_MAP.md, docs/NOT_FORMAT.md */
#ifndef PIU_H
#define PIU_H
#include <stddef.h>
#include <stdint.h>

/* --- checksum / cifras (0x298D1, 0x29949, 0x2896A) --- */
uint32_t piu_adler32(const uint8_t *p, size_t n);
void     piu_xor_d2(uint8_t *p, size_t n);                 /* 0x29949: key=0xD2, +0x95 */
/* .AUD/.PNZ: devolve buffer malloc'ado (plain) ou NULL; *len = tamanho; adler_ok informa checksum */
extern int piu_use_pc_key;
uint8_t *piu_unpack_file(const char *path, size_t *len, int *adler_ok);

/* --- container RES (BGA\*.DAT, 0x29A45) --- */
typedef struct { char name[17]; uint8_t *data; uint32_t size; int adler_ok; } PiuResEntry;
typedef struct { int count; PiuResEntry *e; int data_adler_ok; } PiuRes;
int  piu_res_load(const char *path, PiuRes *r);
void piu_res_free(PiuRes *r);
const PiuResEntry *piu_res_find(const PiuRes *r, const char *name); /* case-insensitive */

/* --- .NOT (0x21361/0x21251) --- */
#define PIU_ROWS 2048
#define PIU_ROWSTRIDE 14
typedef struct {
    uint32_t count; float bpm[4]; int32_t sync[4]; int32_t third[4]; uint32_t n_per_beat; uint32_t h3c, h40;
    char title[0x44];
    uint8_t cell[PIU_ROWS*PIU_ROWSTRIDE];   /* 0x15594C: '0' vazio, '9' nota pendente (apos loader) */
} PiuChart;
int piu_chart_load(const char *path, PiuChart *c);

/* --- BGA/SPR (0x2628F, 0x25FA5) --- */
typedef struct { int16_t time, active; float x,y,px,py,scale,rot,r,g,b,a; } PiuBgaKey;
typedef struct { char name[65]; uint32_t n; PiuBgaKey *k; } PiuBgaLayer;
typedef struct { uint32_t h[3]; int nlayers; PiuBgaLayer layer[50]; } PiuBga;
int  piu_bga_parse(const uint8_t *b, size_t len, PiuBga *g);
void piu_bga_free(PiuBga *g);
/* interpola o layer no frame f; devolve 0 (nada a desenhar) / 1 (ok) / 2 (fim) */
int  piu_bga_eval(const PiuBgaLayer *l, int f, PiuBgaKey *out);

/* --- tempo/scroll (0x25064..0x25121, 0x23ED4, 0x22840) --- */
#define PIU_TICKS_PER_SEC 240
typedef struct { float bpm; int32_t sync; int n; } PiuClock;
int32_t piu_c14(const PiuClock *c, int32_t pos);           /* round((pos - sync*240/100)*bpm) */
int32_t piu_c18(const PiuClock *c, int32_t pos);           /* c14/240 */
int     piu_row_now(const PiuClock *c, int32_t pos, int flag10); /* linha julgavel 0x3D0BD4 */

/* --- julgamento (0x23229, janela 0x24BA6, 0x24057) --- */
enum { PIU_PERFECT='5', PIU_GREAT='6', PIU_GOOD='7', PIU_BAD='8', PIU_MISS='9' };
typedef struct { int32_t t[4]; } PiuWindows;               /* Perfect, Great, Good, Bad */
void piu_windows(float bpm, PiuWindows *w);
int  piu_classify(const PiuWindows *w, int dist);          /* dist = |row*60/N - c18| */
typedef struct { int32_t score, life, life_rate, combo, miss_streak; int16_t cnt[5]; int32_t modifier; } PiuPlayer;
void piu_apply_judgment(PiuPlayer *p, int grade);
#endif
