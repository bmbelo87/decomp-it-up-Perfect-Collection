/* piu_test <game_dir>: valida todos os formatos e simula autoplay de um step.
 * Nao depende de SDL: e o teste do nucleo (Fase 1). */
#include "piu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

static int has_ext(const char *n, const char *e)
{
    size_t l = strlen(n), k = strlen(e);
    if (l < k) return 0;
    for (size_t i = 0; i < k; i++) { char a = n[l - k + i], b = e[i]; if (a >= 'a' && a <= 'z') a -= 32; if (a != b) return 0; }
    return 1;
}

typedef void (*cb_t)(const char *path, void *u);
static void each(const char *dir, const char *ext, cb_t cb, void *u)
{
    char pat[512], full[600];
#ifdef _WIN32
    WIN32_FIND_DATAA fd;
    snprintf(pat, sizeof pat, "%s/*", dir);
    HANDLE h = FindFirstFileA(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        size_t e = strlen(ext);
        (void)e; if (!has_ext(fd.cFileName, ext)) continue;
        snprintf(full, sizeof full, "%s/%s", dir, fd.cFileName);
        cb(full, u);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
#else
    DIR *d = opendir(dir);
    struct dirent *de;
    (void)pat;
    if (!d) return;
    while ((de = readdir(d))) {
        size_t l = strlen(de->d_name), e = strlen(ext);
        (void)l; (void)e; if (!has_ext(de->d_name, ext)) continue;
        snprintf(full, sizeof full, "%s/%s", dir, de->d_name);
        cb(full, u);
    }
    closedir(d);
#endif
}

static int n_ok, n_bad;
static void t_unpack(const char *p, void *u)
{
    size_t len; int ok = 0;
    uint8_t *b = piu_unpack_file(p, &len, &ok);
    (void)u;
    if (b && ok) n_ok++; else { n_bad++; printf("FALHA unpack %s\n", p); }
    free(b);
}
static void t_dat(const char *p, void *u)
{
    PiuRes r; int bgas = 0;
    (void)u;
    if (piu_res_load(p, &r) || !r.data_adler_ok) { n_bad++; printf("FALHA res %s\n", p); return; }
    for (int i = 0; i < r.count; i++) {
        if (!r.e[i].adler_ok) { n_bad++; printf("FALHA entrada %s/%s\n", p, r.e[i].name); continue; }
        size_t l = strlen(r.e[i].name);
        if (l > 4 && has_ext(r.e[i].name, ".BGA")) {
            PiuBga g;
            int rc = piu_bga_parse(r.e[i].data, r.e[i].size, &g);
            if (rc) { n_bad++; printf("FALHA bga %s/%s rc=%d\n", p, r.e[i].name, rc); } else bgas++;
            piu_bga_free(&g);
        }
    }
    n_ok++;
    piu_res_free(&r);
}
static void t_not(const char *p, void *u)
{
    static PiuChart c;
    (void)u;
    if (piu_chart_load(p, &c)) { n_bad++; printf("FALHA not %s\n", p); } else n_ok++;
}

/* autoplay: bate cada linha com erro 'jitter' (em px de scroll) e soma o resultado */
static void autoplay(const char *path, int jitter)
{
    static PiuChart c;
    if (piu_chart_load(path, &c)) { printf("nao abriu %s\n", path); return; }
    PiuClock ck = { c.bpm[0], c.sync[0], (int)c.n_per_beat };
    PiuWindows w; PiuPlayer p = { 0 };
    p.life = 500;                      /* valor inicial real ainda nao confirmado */
    piu_windows(ck.bpm, &w);
    int notes = 0;
    for (int r = 0; r < PIU_ROWS; r++) {
        int hit = 0;
        for (int col = 0; col < 5; col++) if (c.cell[r * PIU_ROWSTRIDE + col] == '9') hit = 1;
        if (!hit) continue;
        notes++;
        int d = (r * 60) / ck.n + ((r * 7) % (2 * jitter + 1) - jitter);   /* erro pseudo-aleatorio deterministico */
        d = d - (r * 60) / ck.n;
        piu_apply_judgment(&p, piu_classify(&w, d));
    }
    printf("%s '%s' bpm=%.1f N=%d janelas P/G/Go/B=%d/%d/%d/%d notas=%d jitter=%d -> P%d G%d Go%d B%d M%d score=%d combo=%d\n",
           path, c.title, ck.bpm, ck.n, w.t[0], w.t[1], w.t[2], w.t[3], notes, jitter,
           p.cnt[0], p.cnt[1], p.cnt[2], p.cnt[3], p.cnt[4], p.score, p.combo);
}

int main(int argc, char **argv)
{
    const char *g = argc > 1 ? argv[1] : "../game";
    char d[512];
    snprintf(d, sizeof d, "%s/AUDIO", g); n_ok = n_bad = 0; each(d, ".AUD", t_unpack, 0); printf("AUD  ok=%d bad=%d\n", n_ok, n_bad);
    snprintf(d, sizeof d, "%s/TITLE", g); n_ok = n_bad = 0; each(d, ".PNZ", t_unpack, 0); printf("PNZ  ok=%d bad=%d\n", n_ok, n_bad);
    snprintf(d, sizeof d, "%s/BGA", g);   n_ok = n_bad = 0; each(d, ".DAT", t_dat, 0);    printf("DAT  ok=%d bad=%d\n", n_ok, n_bad);
    snprintf(d, sizeof d, "%s/STEP", g);  n_ok = n_bad = 0; each(d, ".NOT", t_not, 0);    printf("NOT  ok=%d bad=%d\n", n_ok, n_bad);
    snprintf(d, sizeof d, "%s/STEP/101_DB.NOT", g);
    autoplay(d, 0); autoplay(d, 6); autoplay(d, 20);
    return 0;
}
