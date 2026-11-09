/* Tempo, scroll e julgamento. Fonte: docs/FUNCTION_MAP.md Atualizacoes 6 e 8 */
#include "piu.h"
#include <math.h>

int32_t piu_c14(const PiuClock *c, int32_t pos)               /* 0x250FA: fild; fmul bpm; frndint; fistp */
{
    int32_t off = (c->sync * 240) / 100;                       /* 0x23EF9: idiv 100 */
    return (int32_t)nearbyintf((float)(pos - off) * c->bpm);
}

int32_t piu_c18(const PiuClock *c, int32_t pos) { return piu_c14(c, pos) / 240; }   /* 0x2511F */

int piu_row_now(const PiuClock *c, int32_t pos, int flag10)    /* 0x23F1A..0x23F7C */
{
    int32_t off = (c->sync * 240) / 100;
    int32_t x = (int32_t)nearbyintf((float)(pos - off) * c->bpm / 240.0f);
    if (!flag10) x -= 0xD;
    int32_t r = (x * c->n) / 60 + 1;
    if (r < 0 || r >= PIU_ROWS) r = 0;
    return r;
}

void piu_windows(float b, PiuWindows *w)                       /* 0x24BA6..0x24C44 */
{
    static const float M[4] = { 8, 16, 24, 32 }, C[4] = { 4, 5, 6, 7 };
    for (int i = 0; i < 4; i++) w->t[i] = (int32_t)nearbyintf((b * M[i] / 120.0f + C[i]) * 0.5f);
}

int piu_classify(const PiuWindows *w, int d)                   /* 0x24086..0x240AA */
{
    if (d < 0) d = -d;
    if (d <= w->t[0]) return PIU_PERFECT;
    if (d <= w->t[1]) return PIU_GREAT;
    if (d <= w->t[2]) return PIU_GOOD;
    return PIU_BAD;
}

void piu_apply_judgment(PiuPlayer *p, int g)                   /* 0x23229 / jump table 0x23215 */
{
    switch (g) {
    case PIU_PERFECT:
        p->cnt[0]++; p->combo++; p->miss_streak = 0; p->score += 1000;
        p->life += (p->life_rate * 12) / 1000; p->life_rate += 20;
        if (p->combo > 3) p->score += 1000;
        break;
    case PIU_GREAT:
        p->cnt[1]++; p->combo++; p->miss_streak = 0; p->score += 500;
        p->life += (p->life_rate * 10) / 1000; p->life_rate += 16;
        if (p->combo > 3) p->score += 1000;
        break;
    case PIU_GOOD:
        p->cnt[2]++; p->combo = 0; p->miss_streak = 0;
        break;
    case PIU_BAD:
        p->cnt[3]++; p->combo = 0; p->miss_streak = 0; p->life -= 0x32; p->life_rate += p->modifier / 2;
        break;
    case PIU_MISS:
        p->cnt[4]++; p->combo = 0; p->miss_streak++;
        p->life += (-500 * p->life) / 2000 - 20; p->life_rate += p->modifier;
        break;
    }
}
