/*
 * FightPort 4 1.3.0 - Ports & Mods
 * One launcher for the PC ports of EA SPORTS UFC, UFC 2, UFC 3, UFC 4 and UFC 5, with the look of
 * FightPort 4 (logo, circuit background, game cards, VIDEO / GAMEPLAY options, gradient PLAY).
 *  - UFC 1-4: options are written to %LOCALAPPDATA%\UFCnNative\sacredfire.ini (the ports' own file),
 *    the port is started as games\UFCn\UFCn.exe --play.
 *  - UFC 5: options are written to games\UFC5\UFC5.ini, the port is started as UFC5.exe --play.
 * The whole window is drawn by hand (no themed controls) except the two text fields.
 * Win32 + GDI/GDI+, no dependencies beyond Windows itself. GPL-2.0-or-later.
 */
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <initguid.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <math.h>
#ifdef _MSC_VER
#include <intrin.h>
static inline int __get_cpuid_count(unsigned int leaf, unsigned int subleaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx) {
    int cpuInfo[4];
    __cpuidex(cpuInfo, (int)leaf, (int)subleaf);
    *eax = (unsigned int)cpuInfo[0];
    *ebx = (unsigned int)cpuInfo[1];
    *ecx = (unsigned int)cpuInfo[2];
    *edx = (unsigned int)cpuInfo[3];
    return 1;
}
static inline int __get_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx) {
    return __get_cpuid_count(leaf, 0, eax, ebx, ecx, edx);
}
#else
#include <cpuid.h>
#endif
#include <tlhelp32.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "pkg_engine.h"

#define APP_VER L"1.3.0"
#define WM_APP_EXITED (WM_APP + 1)
#define WM_APP_FROZEN (WM_APP + 2)
#define WM_APP_AUTOCFG (WM_APP + 3)

/* ---------------------------------------------------------------- GDI+ flat API (subset) */
typedef struct { UINT32 GdiplusVersion; void *DebugEventCallback; BOOL SuppressBackgroundThread; BOOL SuppressExternalCodecs; } GdiplusStartupInput;
typedef void GpImage; typedef void GpGraphics; typedef void GpBrush; typedef void GpPen; typedef void GpPath;
int WINAPI GdiplusStartup(ULONG_PTR *, const GdiplusStartupInput *, void *);
int WINAPI GdipCreateBitmapFromStream(IStream *, GpImage **);
int WINAPI GdipLoadImageFromFile(const WCHAR *, GpImage **);
int WINAPI GdipGetImageWidth(GpImage *, UINT *);
int WINAPI GdipGetImageHeight(GpImage *, UINT *);
int WINAPI GdipCreateFromHDC(HDC, GpGraphics **);
int WINAPI GdipSetInterpolationMode(GpGraphics *, int);
int WINAPI GdipSetSmoothingMode(GpGraphics *, int);
int WINAPI GdipSetPixelOffsetMode(GpGraphics *, int);
int WINAPI GdipDrawImageRectRectI(GpGraphics *, GpImage *, INT, INT, INT, INT, INT, INT, INT, INT, int, void *, void *, void *);
int WINAPI GdipDrawImageRectI(GpGraphics *, GpImage *, INT, INT, INT, INT);
int WINAPI GdipCreateSolidFill(UINT32, GpBrush **);
int WINAPI GdipCreateLineBrushI(const POINT *, const POINT *, UINT32, UINT32, int, GpBrush **);
int WINAPI GdipFillRectangleI(GpGraphics *, GpBrush *, INT, INT, INT, INT);
int WINAPI GdipDeleteBrush(GpBrush *);
int WINAPI GdipCreatePen1(UINT32, float, int, GpPen **);
int WINAPI GdipCreatePen2(GpBrush *, float, int, GpPen **);
int WINAPI GdipDeletePen(GpPen *);
int WINAPI GdipCreatePath(int, GpPath **);
int WINAPI GdipDeletePath(GpPath *);
int WINAPI GdipAddPathArcI(GpPath *, INT, INT, INT, INT, float, float);
int WINAPI GdipAddPathLineI(GpPath *, INT, INT, INT, INT);
int WINAPI GdipClosePathFigure(GpPath *);
int WINAPI GdipFillPath(GpGraphics *, GpBrush *, GpPath *);
int WINAPI GdipDrawPath(GpGraphics *, GpPen *, GpPath *);
int WINAPI GdipFillEllipseI(GpGraphics *, GpBrush *, INT, INT, INT, INT);
int WINAPI GdipDrawLineI(GpGraphics *, GpPen *, INT, INT, INT, INT);
int WINAPI GdipSetClipPath(GpGraphics *, GpPath *, int);
int WINAPI GdipResetClip(GpGraphics *);
int WINAPI GdipDeleteGraphics(GpGraphics *);
int WINAPI GdipDisposeImage(GpImage *);

/* ---------------------------------------------------------------- look (16:9 Harmonized Theme) */
#define A(c, a) (((UINT32)(a) << 24) | ((UINT32)GetRValue(c) << 16) | ((UINT32)GetGValue(c) << 8) | (UINT32)GetBValue(c))
#define C_BG      RGB(4, 7, 18)
#define C_TEXT    RGB(240, 243, 250)
#define C_SUB     RGB(142, 156, 182)
#define C_MUTED   RGB(95, 108, 130)
#define C_BLUE    RGB(32, 96, 216)
#define C_GOLD    RGB(245, 212, 32)
#define C_FIELD   RGB(10, 14, 26)
#define C_FIELDB  RGB(36, 48, 76)
#define C_CARD    RGB(8, 12, 24)
#define C_CARDB   RGB(32, 42, 68)
#define C_GOOD    RGB(60, 205, 125)
#define C_WARN    RGB(240, 168, 30)
#define C_ORANGE  C_WARN
#define C_BAD     RGB(240, 68, 68)

/* 16:9 Standard Coordinates */
#define BASE_W 1280
#define BASE_H 720
#define X0 624
#define X1 1256
#define UFC4_FOLDER_NAME L"EA_Sports_UFC_4"

/* ---------------------------------------------------------------- strings (PT-BR / EN) */
enum { S_READY, S_PICK, S_NOPORT, S_RUNNING, S_GAME, S_VIDEO, S_GAMEPLAY, S_BROWSE, S_PORT, S_CLICKCHANGE, S_ADV, S_LOGS, S_PLAY, S_STOP,
       S_ABOUT, S_RES, S_WIN, S_FPS, S_PRESENT, S_UPSCALE, S_SHARP, S_LANG, S_VOLUME, S_PLAYER, S_FULL, S_NEO, S_SKIP, S_LOWLAT, S_ULTRA,
       S_NATIVE, S_PS5PORT, S_PROFILE, S_SHADERS, S_OVERLAY, S_X3D, S_VIB, S_FASTSH, S_ECO, S_PRIO, S_LPAGES, S_BINDLESS, S_PICKGAME,
       S_PICKPORT, S_NOTFOUND, S_STARTING, S_CLOSED, S_CLOSEDERR, S_CANTSTART, S_ULTRAON, S_CHOOSEFIRST, S_SESSION, S_NOLOGS, S_SHTHREADS, S_SPKVOL, S_CALIB, S_STAGEPREP, S_FRAMEGEN, S_RES5, S_P60, S_PCROWD, S_PBLUR, S_PARSEC, S_COUNT };
static const wchar_t *TXT[2][S_COUNT] = {
  { L"Pronto para jogar", L"Escolha a pasta do jogo", L"Port não encontrado", L"Em execução", L"JOGO", L"VÍDEO", L"JOGABILIDADE", L"Procurar",
    L"Port", L"clique para trocar", L"Avançado", L"Logs", L"JOGAR", L"PARAR",
    L"Sobre", L"Resolução do jogo", L"Tamanho da janela", L"Limite de FPS", L"Modo de apresentação", L"Upscaling", L"Nitidez",
    L"Idioma do jogo", L"Volume", L"Nome do jogador", L"Tela cheia", L"Modo PS4 Pro", L"Pular vídeos de introdução", L"Baixa latência (co-op)", L"Ultra desempenho (PC fraco)",
    L"Port nativo para PC", L"Port para PC (PS5)", L"Perfil de desempenho", L"Shaders", L"FPS na tela", L"Modo X3D (Ryzen)", L"Vibração do controle",
    L"Compilar shaders rápido", L"Sem economia de energia", L"Prioridade alta", L"Large pages", L"Texturas bindless (exp.)", L"Escolha a pasta do jogo %s",
    L"Escolha a pasta do port do %s (a que tem o %s)", L"%s não foi encontrado nessa pasta.", L"Iniciando o %s...", L"O jogo foi fechado.",
    L"O jogo fechou com um erro (código %lu). Abra os logs para ver os detalhes.", L"Não foi possível iniciar o %s (erro %lu).",
    L"Ultra desempenho ligado: 720p, modo PS4, 30 FPS, FSR 1.0.", L"Escolha a pasta do jogo primeiro.", L"Última sessão: %s.", L"Ainda não há logs: jogue uma vez primeiro.", L"Threads de compilação", L"Alto-falante do controle", L"Calibrar FPS sozinho", L"Preparação paralela (exp.)", L"Geração de quadros", L"Resolução (modo janela)" , L"FPS da luta (patch)", L"Torcida", L"Motion blur", L"Modo Parsec (host online)"},
  { L"Ready to play", L"Choose your game folder", L"Port not found", L"Running", L"GAME", L"VIDEO", L"GAMEPLAY", L"Browse",
    L"Port", L"click to change", L"Advanced", L"Logs", L"PLAY", L"STOP",
    L"About", L"Game resolution", L"Window size", L"Frame rate limit", L"Present mode", L"Upscaling", L"Sharpening",
    L"Game language", L"Volume", L"Player name", L"Fullscreen", L"PS4 Pro mode", L"Skip intro videos", L"Low input latency (co-op)", L"Ultra performance (weak PCs)",
    L"Native PC port", L"PC port (PS5)", L"Performance profile", L"Shaders", L"FPS on screen", L"X3D mode (Ryzen)", L"Controller vibration",
    L"Fast shader compiling", L"No Windows power saving", L"High priority", L"Large pages", L"Bindless textures (exp.)", L"Choose the %s game folder",
    L"Choose the folder of the %s port (the one with %s)", L"%s was not found in that folder.", L"Starting %s...", L"The game closed.",
    L"The game closed with an error (code %lu). Open the logs for details.", L"Could not start %s (error %lu).",
    L"Ultra performance is on: 720p, PS4 mode, 30 FPS, FSR 1.0.", L"Choose the game folder first.", L"Last session: %s.", L"No logs yet: play once first.", L"Shader compile threads", L"Controller speaker", L"Self-calibrate FPS", L"Parallel preparation (exp.)", L"Frame generation", L"Resolution (windowed)" , L"Fight FPS (patch)", L"Crowd", L"Motion blur", L"Parsec mode (online host)"},
};
static int g_lang = 0;   /* 0 = PT-BR, 1 = EN */
#define T(k) TXT[g_lang][k]

/* ---------------------------------------------------------------- games */
#define NGAMES 1
typedef struct { const wchar_t *id, *name, *exe, *data; BOOL ps5; } Game;
static const Game G[NGAMES] = {
    {L"UFC4", L"UFC 4", L"UFC4.exe", L"UFC4Native", FALSE},
};

/* ---------------------------------------------------------------- option tables */
typedef struct { const wchar_t *pt, *en; int value; } Opt;
#define O(pt, en, v) {pt, en, v}
static const Opt RES4[] = {O(L"1280x720", L"1280x720", 0), O(L"1920x1080", L"1920x1080", 1), O(L"2560x1440 (2K, experimental)", L"2560x1440 (2K, experimental)", 2), O(L"3840x2160 (4K, experimental)", L"3840x2160 (4K, experimental)", 3)};
static const Opt WIN4[] = {O(L"Igual ao jogo", L"Same as the game", 0), O(L"1920x1080 (Full HD)", L"1920x1080 (Full HD)", 1), O(L"2560x1440 (2K)", L"2560x1440 (2K)", 2), O(L"3840x2160 (4K)", L"3840x2160 (4K)", 3)};
static const Opt FPS4[] = {O(L"30 FPS (PS4)", L"30 FPS (PS4)", 30), O(L"60 FPS", L"60 FPS", 60), O(L"90 FPS", L"90 FPS", 90), O(L"120 FPS", L"120 FPS", 120), O(L"144 FPS", L"144 FPS", 144), O(L"200 FPS", L"200 FPS", 200), O(L"240 FPS", L"240 FPS", 240), O(L"Sem limite", L"Unlimited", 0)};
static const Opt PRES4[] = {O(L"Mailbox", L"Mailbox", 0), O(L"VSync (FIFO)", L"VSync (FIFO)", 1), O(L"Imediato (sem limite, pode rasgar)", L"Immediate (uncapped, may tear)", 2)};
static const Opt UPS4[] = {O(L"Desligado (bilinear)", L"Off (bilinear)", 0), O(L"AMD FSR 1.0", L"AMD FSR 1.0", 1)};
static const Opt SHP4[] = {O(L"Desligada", L"Off", 0), O(L"Baixa", L"Low", 1), O(L"Média", L"Medium", 2), O(L"Alta", L"High", 3)};
static const Opt LANG4[] = {O(L"Inglês (EUA)", L"English (US)", 1), O(L"Francês", L"French", 2), O(L"Alemão", L"German", 4), O(L"Italiano", L"Italian", 5),
    O(L"Espanhol (Espanha)", L"Spanish (Spain)", 3), O(L"Espanhol (América Latina)", L"Spanish (Latin America)", 20), O(L"Português (Brasil)", L"Portuguese (Brazil)", 17),
    O(L"Português (Portugal)", L"Portuguese (Portugal)", 7), O(L"Polonês", L"Polish", 16), O(L"Russo", L"Russian", 8), O(L"Japonês", L"Japanese", 0), O(L"Chinês (Tradicional)", L"Chinese (Traditional)", 10)};
static const Opt VOL4[] = {O(L"50%", L"50%", 50), O(L"75%", L"75%", 75), O(L"100%", L"100%", 100), O(L"125%", L"125%", 125), O(L"150%", L"150%", 150), O(L"200%", L"200%", 200), O(L"300%", L"300%", 300)};
static const Opt P60[] = {O(L"30 FPS original (opera até limite de FPS)", L"30 FPS original (up to FPS limit)", 0), O(L"60+ FPS (patch: pálpebras/bocas soltas)", L"60+ FPS (patch: loose eyelids/mouths)", 1)};
static const Opt PCROWD[] = {O(L"Ligada (original)", L"On (original)", 0), O(L"Desligada (+5 a 10 FPS)", L"Off (+5 to 10 FPS)", 1)};
static const Opt PBLUR[] = {O(L"Ligado (original)", L"On (original)", 0), O(L"Desligado (imagem mais nítida)", L"Off (sharper image)", 1)};

#define N(a) (int)(sizeof(a) / sizeof(a[0]))

/* a "field" of the panel: a dropdown or a switch bound to one key of the port's settings */
typedef struct { int label; const wchar_t *key; const Opt *opts; int nopts; int def; BOOL perf; } Field;
static const Field VID4P[9] = {{S_RES, L"resolution", RES4, N(RES4), 1, FALSE}, {S_WIN, L"display", WIN4, N(WIN4), 0, FALSE}, {S_FPS, L"fps_limit", FPS4, N(FPS4), 60, FALSE},
                               {S_PRESENT, L"present_mode", PRES4, N(PRES4), 0, FALSE}, {S_UPSCALE, L"upscaler", UPS4, N(UPS4), 0, FALSE}, {S_SHARP, L"sharpness", SHP4, N(SHP4), 2, FALSE},
                               {S_P60, L"patch:60fps", P60, N(P60), 0, FALSE}, {S_PCROWD, L"patch:crowd", PCROWD, N(PCROWD), 0, FALSE}, {S_PBLUR, L"patch:blur", PBLUR, N(PBLUR), 0, FALSE}};
static const Field GP4[2] = {{S_LANG, L"language", LANG4, N(LANG4), 17, FALSE}, {S_VOLUME, L"volume", VOL4, N(VOL4), 100, FALSE}};
static const Field SW4[6] = {{S_FULL, L"fullscreen", NULL, 0, 1, FALSE}, {S_NEO, L"neo", NULL, 0, 0, FALSE}, {S_SKIP, L"skip_movies", NULL, 0, 1, FALSE}, {S_LOWLAT, L"low_latency", NULL, 0, 0, FALSE}, {S_ULTRA, L"ultra", NULL, 0, 0, FALSE},
                             {S_PARSEC, L"parsec", NULL, 0, 0, FALSE}};

/* ---------------------------------------------------------------- state */
static HINSTANCE g_inst;
static HWND g_wnd, g_editGame, g_editPlayer;
static int g_dpi = 96, g_sel = 0, g_hot = -1;
static int g_navTab = 0;   /* 0 = Settings, 1 = PKG Extractor */
static PkgInfo g_pkgBase;
static BOOL g_haveBase = FALSE;
static PkgInfo g_pkgPatch;
static BOOL g_havePatch = FALSE;
static BOOL g_autoDeletePkg = FALSE;
static GpImage *g_imgPkgIcon = NULL;
static int g_pkgExtractPct = 0;
static wchar_t g_pkgExtractStatus[256] = L"";
static wchar_t g_lastExtractedDir[MAX_PATH] = L"";
static volatile BOOL g_pkgExtracting = FALSE;
static volatile BOOL g_pkgExtractCancel = FALSE;
static HANDLE g_pkgThread = NULL;
static HFONT g_fTitle, g_fCard, g_fText, g_fSmall, g_fSection, g_fPlay, g_fBtn, g_fFoot, g_fTabBig, g_fTabNorm;
static HFONT g_fSplashPct = NULL, g_fSplashStatus = NULL;

/* Splash loading screen (0 to 100% in 2.0 seconds) */
static BOOL g_splashActive = TRUE;
static ULONGLONG g_splashStart = 0;
static float g_splashMouseX = 0.0f, g_splashMouseY = 0.0f;
static GpImage *g_imgSplashBg = NULL, *g_imgSplashFg = NULL;
#define SPLASH_DURATION_MS 2000
#define SPLASH_NUM_PARTICLES 45

typedef struct {
    float x, y;
    float size;
    float speedX, speedY;
    float alpha;
    UINT32 color;
} SplashParticle;

static SplashParticle g_splashParticles[SPLASH_NUM_PARTICLES];
static BOOL g_splashParticlesInit = FALSE;
static HBRUSH g_brField;
static wchar_t g_exeDir[MAX_PATH], g_pIni[MAX_PATH], g_msg[512];
static COLORREF g_msgColor = C_SUB;
static ULONG_PTR g_gdip;
static GpImage *g_imgBgConfig = NULL, *g_imgBgExtra = NULL, *g_imgBg = NULL, *g_imgLogo = NULL;
static HBITMAP g_bgCache = NULL; static int g_bgW = 0, g_bgH = 0, g_bgTab = -1;
static HANDLE g_job, g_proc; static int g_procGame = -1;
static wchar_t g_ver[NGAMES][48];

#define SC(x) MulDiv((x), g_dpi, 96)

/* hit regions, rebuilt on every paint */
enum { 
    H_CARD = 0, H_DROP = 10, H_SWITCH = 30, H_BROWSE = 50, H_PORT, H_ADV, H_LOGS, H_SAVE = 60, H_SHORTCUT = 61, H_PLAY = 62, H_ABOUT, H_EN, H_PT,
    H_TAB_CONFIG = 70, H_TAB_PKG = 71,
    H_PKG_BROWSE_ALL = 72, H_PKG_BROWSE_BASE = 73, H_PKG_BROWSE_PATCH = 74,
    H_PKG_EXTRACT_GAME = 75, H_PKG_EXTRACT_CUSTOM = 76, H_PKG_VIEW_SFO = 77, H_PKG_CANCEL = 78,
    H_PKG_CHECK_DELETE = 79, H_PKG_CLEAR = 80,
    H_ICON_START = 100,
    H_ICON_CLOSE = 130,
    H_ICON_CREATE_SHORTCUT = 131,
    H_ICON_CHANGE_SHORTCUT = 132,
    H_ICON_MODAL_BG = 133,
    H_THEMED_MODAL_BG = 250,
    H_THEMED_MODAL_CLOSE = 251,
    H_THEMED_MODAL_BTN1 = 252,
    H_THEMED_MODAL_BTN2 = 253
};

typedef enum {
    THEMED_MODAL_OK = 0,
    THEMED_MODAL_OKCANCEL = 1,
    THEMED_MODAL_YESNO = 2
} ThemedModalButtons;

typedef enum {
    THEMED_ICON_INFO = 0,
    THEMED_ICON_QUESTION = 1,
    THEMED_ICON_WARN = 2,
    THEMED_ICON_ERROR = 3
} ThemedModalIcon;

static BOOL g_themedModalActive = FALSE;
static wchar_t g_themedModalTitle[128] = L"";
static wchar_t g_themedModalText[2048] = L"";
static ThemedModalButtons g_themedModalButtons = THEMED_MODAL_OK;
static ThemedModalIcon g_themedModalIcon = THEMED_ICON_INFO;
static const wchar_t *g_themedModalBtn1Label = NULL;
static const wchar_t *g_themedModalBtn2Label = NULL;
static int g_themedModalResult = 0;
typedef struct { RECT r; int id; } Hit;
static Hit g_hits[256]; static int g_nhits;
static void AddHit(int l, int t, int r, int b, int id) { if (g_nhits < 256) { SetRect(&g_hits[g_nhits].r, l, t, r, b); g_hits[g_nhits++].id = id; } }
static int HitAt(int x, int y) { POINT p = {x, y}; for (int i = g_nhits - 1; i >= 0; i--) if (PtInRect(&g_hits[i].r, p)) return g_hits[i].id; return -1; }

/* ---------------------------------------------------------------- small helpers */
static BOOL FileExists(const wchar_t *p) { DWORD a = GetFileAttributesW(p); return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY); }
static BOOL DirExists(const wchar_t *p) { DWORD a = GetFileAttributesW(p); return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY); }
static void Join(wchar_t *out, const wchar_t *a, const wchar_t *b) { _snwprintf(out, MAX_PATH, L"%s\\%s", a, b); out[MAX_PATH - 1] = 0; }
static void StripSlash(wchar_t *p) { size_t n = wcslen(p); while (n > 3 && (p[n - 1] == L'\\' || p[n - 1] == L'/')) p[--n] = 0; }
static void Msg(COLORREF c, const wchar_t *fmt, ...)
{
    va_list ap; va_start(ap, fmt); _vsnwprintf(g_msg, 512, fmt, ap); va_end(ap); g_msg[511] = 0; g_msgColor = c;
    if (g_wnd) InvalidateRect(g_wnd, NULL, FALSE);
}

static void PortDir(int g, wchar_t *out)
{
    wchar_t k[32]; _snwprintf(k, 32, L"port_%s", G[g].id);
    GetPrivateProfileStringW(L"FightPort4", k, L"", out, MAX_PATH, g_pIni);
    if (out[0] && DirExists(out)) return;
    wchar_t t[MAX_PATH]; Join(t, g_exeDir, L"games"); Join(out, t, G[g].id);
}
static void PortExe(int g, wchar_t *out) { wchar_t d[MAX_PATH]; PortDir(g, d); Join(out, d, G[g].exe); }

/* ---------------------------------------------------------------- settings files */
/* UFC 1-4: %LOCALAPPDATA%\UFCnNative\sacredfire.ini - UTF-8 with BOM, CRLF, key=value, unknown keys kept */
#define MAXL 128
typedef struct { wchar_t k[48], v[MAX_PATH]; } KV;
static KV g_kv[MAXL]; static int g_nkv, g_kvGame = -1;

static void SfPath(int g, wchar_t *out) { wchar_t la[MAX_PATH] = L""; GetEnvironmentVariableW(L"LOCALAPPDATA", la, MAX_PATH); wchar_t d[MAX_PATH]; Join(d, la, G[g].data); Join(out, d, L"sacredfire.ini"); }
static void SfLoad(int g)
{
    g_nkv = 0; g_kvGame = g;
    wchar_t p[MAX_PATH]; SfPath(g, p);
    HANDLE f = CreateFileW(p, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return;
    DWORD sz = GetFileSize(f, NULL); if (sz > 256 * 1024) sz = 256 * 1024;
    char *b = (char *)HeapAlloc(GetProcessHeap(), 0, sz + 1); DWORD n = 0; ReadFile(f, b, sz, &n, NULL); CloseHandle(f); b[n] = 0;
    char *s = b; if ((unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF) s += 3;
    while (*s && g_nkv < MAXL) {
        char *e = s; while (*e && *e != '\n') e++;
        char save = *e; *e = 0; char *cr = strchr(s, '\r'); if (cr) *cr = 0;
        char *eq = strchr(s, '=');
        if (eq && eq != s) { *eq = 0; MultiByteToWideChar(CP_UTF8, 0, s, -1, g_kv[g_nkv].k, 48); MultiByteToWideChar(CP_UTF8, 0, eq + 1, -1, g_kv[g_nkv].v, MAX_PATH); g_nkv++; }
        *e = save; s = *e ? e + 1 : e;
    }
    HeapFree(GetProcessHeap(), 0, b);
}
static void SfSave(void)
{
    if (g_kvGame < 0) return;
    wchar_t p[MAX_PATH], d[MAX_PATH]; SfPath(g_kvGame, p); wcscpy(d, p); *wcsrchr(d, L'\\') = 0; CreateDirectoryW(d, NULL);
    HANDLE f = CreateFileW(p, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return;
    DWORD w; WriteFile(f, "\xEF\xBB\xBF", 3, &w, NULL);
    for (int i = 0; i < g_nkv; i++) {
        wchar_t line[MAX_PATH + 64]; _snwprintf(line, MAX_PATH + 64, L"%s=%s\r\n", g_kv[i].k, g_kv[i].v);
        char u[(MAX_PATH + 64) * 3]; int n = WideCharToMultiByte(CP_UTF8, 0, line, -1, u, sizeof(u), NULL, NULL);
        if (n > 1) WriteFile(f, u, n - 1, &w, NULL);
    }
    CloseHandle(f);
}
static KV *SfFind(const wchar_t *k) { for (int i = 0; i < g_nkv; i++) if (!wcscmp(g_kv[i].k, k)) return &g_kv[i]; return NULL; }

static void GetStr(int g, const wchar_t *k, wchar_t *out, int n, const wchar_t *def)
{
    if (g_kvGame != g) SfLoad(g);
    KV *kv = SfFind(k); wcsncpy(out, kv ? kv->v : def, n); out[n - 1] = 0;
}
/* community patch switches: one "kind" covers every version's entry in patches\UFCn-community.xml */
static BOOL PatchKindMatch(const char *kind, const char *name, size_t n)
{
    char nm[160]; if (n >= sizeof(nm)) n = sizeof(nm) - 1; memcpy(nm, name, n); nm[n] = 0;
    if (!strcmp(kind, "60fps")) return !strcmp(nm, "60+ FPS");
    if (!strcmp(kind, "crowd")) return !strncmp(nm, "Disable Crowd", 13);
    if (!strcmp(kind, "blur")) return strstr(nm, "Motion Blur") != NULL;
    if (!strcmp(kind, "skin0")) return !strncmp(nm, "Skinning Method 0", 17);
    return FALSE;
}
static void PortDir(int g, wchar_t *out);
static void PatchXmlPath(int g, wchar_t *out)
{
    wchar_t d[MAX_PATH], f[64]; PortDir(g, d); _snwprintf(f, 64, L"patches\\%s-community.xml", G[g].id);
    _snwprintf(out, MAX_PATH, L"%s\\%s", d, f); out[MAX_PATH - 1] = 0;
}
/* get (set < 0): 1 when every matching entry is on, -1 when the game has no such patch. set 0/1: rewrites the XML. */
static int PatchState(int g, const wchar_t *kindW, int set)
{
    char kind[16]; WideCharToMultiByte(CP_UTF8, 0, kindW, -1, kind, 16, NULL, NULL);
    wchar_t path[MAX_PATH]; PatchXmlPath(g, path);
    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return -1;
    DWORD sz = GetFileSize(h, NULL), got = 0; char *buf = (char *)malloc(sz + 1);
    if (!buf || sz > (8u << 20)) { CloseHandle(h); free(buf); return -1; }
    ReadFile(h, buf, sz, &got, NULL); CloseHandle(h); buf[got] = 0;
    int found = 0, on = 0; BOOL changed = FALSE;
    for (char *m = buf; (m = strstr(m, "<Metadata ")) != NULL; m++) {
        char *end = strchr(m, '>'); if (!end) break;
        char *nm = strstr(m, " Name=\""); if (!nm || nm > end) continue; nm += 7;
        char *nq = strchr(nm, '"'); if (!nq || nq > end) continue;
        if (!PatchKindMatch(kind, nm, (size_t)(nq - nm))) continue;
        char *en = strstr(m, "isEnabled=\""); if (!en || en > end) continue;
        found++; BOOL isOn = !strncmp(en + 11, "true\"", 5); on += isOn;
        if (set >= 0 && isOn != (set != 0)) {   /* "true" and "false" differ in length: rebuild the buffer */
            size_t pre = (size_t)(en + 11 - buf), oldLen = isOn ? 4 : 5; const char *nv = set ? "true" : "false";
            size_t rest = strlen(en + 11 + oldLen), nl = strlen(nv);
            char *nb = (char *)malloc(pre + nl + rest + 1); if (!nb) break;
            memcpy(nb, buf, pre); memcpy(nb + pre, nv, nl); memcpy(nb + pre + nl, en + 11 + oldLen, rest + 1);
            m = nb + (m - buf); free(buf); buf = nb; changed = TRUE;
        }
    }
    if (changed) {
        h = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
        if (h != INVALID_HANDLE_VALUE) { DWORD wr; WriteFile(h, buf, (DWORD)strlen(buf), &wr, NULL); CloseHandle(h); }
    }
    free(buf);
    if (!found) return -1;
    return set >= 0 ? set : on == found;
}
static int GetInt(int g, const wchar_t *k, int def)
{
    if (!wcsncmp(k, L"patch:", 6)) { int v = PatchState(g, k + 6, -1); return v < 0 ? def : v; }
    if (!wcscmp(k, L"parsec")) { wchar_t pk[32]; _snwprintf(pk, 32, L"parsec_%s", G[g].id); return GetPrivateProfileIntW(L"FightPort4", pk, def, g_pIni); }
    if (!wcscmp(k, L"ultra")) { wchar_t pk[32]; _snwprintf(pk, 32, L"ultra_%s", G[g].id); return GetPrivateProfileIntW(L"FightPort4", pk, def, g_pIni); }
    wchar_t b[32], d[16]; _snwprintf(d, 16, L"%d", def); GetStr(g, k, b, 32, d); return b[0] ? _wtoi(b) : def; }
static void PutStr(int g, const wchar_t *k, const wchar_t *v)
{
    if (g_kvGame != g) SfLoad(g);
    KV *kv = SfFind(k);
    if (!kv && g_nkv < MAXL) { kv = &g_kv[g_nkv++]; wcsncpy(kv->k, k, 47); kv->k[47] = 0; }
    if (kv) { wcsncpy(kv->v, v, MAX_PATH - 1); kv->v[MAX_PATH - 1] = 0; }
    SfSave();
}
static void PutInt(int g, const wchar_t *k, int v)
{
    if (!wcsncmp(k, L"patch:", 6)) { PatchState(g, k + 6, v); return; }
    if (!wcscmp(k, L"parsec") || !wcscmp(k, L"ultra")) {
        wchar_t pk[32], val[16];
        _snwprintf(pk, 32, L"%s_%s", k, G[g].id);
        _snwprintf(val, 16, L"%d", v);
        WritePrivateProfileStringW(L"FightPort4", pk, v ? val : NULL, g_pIni);
    }
    wchar_t b[16]; _snwprintf(b, 16, L"%d", v); PutStr(g, k, b);
}

/* ---------------------------------------------------------------- game folder */
static BOOL HasEboot(const wchar_t *d) { wchar_t p[MAX_PATH]; Join(p, d, L"eboot.bin"); return FileExists(p); }
static BOOL GameOk(const wchar_t *d)
{
    if (!d[0] || !DirExists(d)) return FALSE;
    if (HasEboot(d)) return TRUE;
    wchar_t pat[MAX_PATH], c[MAX_PATH]; Join(pat, d, L"*"); WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd); BOOL ok = FALSE;
    if (h == INVALID_HANDLE_VALUE) return FALSE;
    do { if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && fd.cFileName[0] != L'.') { Join(c, d, fd.cFileName); if (HasEboot(c)) { ok = TRUE; break; } } } while (FindNextFileW(h, &fd));
    FindClose(h); return ok;
}
static BOOL PortOk(int g) { wchar_t e[MAX_PATH]; PortExe(g, e); return FileExists(e); }
static int Status(int g)   /* S_READY, S_PICK, S_NOPORT, S_RUNNING */
{
    if (g_proc && g_procGame == g) return S_RUNNING;
    if (!PortOk(g)) return S_NOPORT;
    wchar_t p[MAX_PATH]; GetStr(g, L"game", p, MAX_PATH, L"");
    return GameOk(p) ? S_READY : S_PICK;
}

static void ReadVersion(int g)
{
    wchar_t d[MAX_PATH], p[MAX_PATH]; PortDir(g, d); Join(p, d, L"VERSION"); g_ver[g][0] = 0;
    HANDLE f = CreateFileW(p, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return;
    char b[64]; DWORD n = 0; ReadFile(f, b, 63, &n, NULL); CloseHandle(f); b[n] = 0;
    char *e = b; while (*e && *e != '\r' && *e != '\n') e++; *e = 0;
    MultiByteToWideChar(CP_UTF8, 0, b, -1, g_ver[g], 48);
}

/* ---------------------------------------------------------------- images */
static GpImage *LoadPngRes(int id)
{
    HRSRC r = FindResourceW(g_inst, MAKEINTRESOURCEW(id), (LPCWSTR)RT_RCDATA); if (!r) return NULL;
    DWORD sz = SizeofResource(g_inst, r); void *p = LockResource(LoadResource(g_inst, r)); if (!p) return NULL;
    IStream *s = SHCreateMemStream((const BYTE *)p, sz); if (!s) return NULL;
    GpImage *img = NULL; GdipCreateBitmapFromStream(s, &img); s->lpVtbl->Release(s); return img;
}
static GpImage *LoadImageFileOrRes(const wchar_t *fileName, int resId)
{
    GpImage *img = NULL;
    wchar_t path[MAX_PATH];
    Join(path, g_exeDir, fileName);
    if (FileExists(path)) {
        if (GdipLoadImageFromFile(path, &img) == 0 && img) return img;
    }
    if (FileExists(fileName)) {
        if (GdipLoadImageFromFile(fileName, &img) == 0 && img) return img;
    }
    const wchar_t *subDirs[] = { L"src\\png", L"png", NULL };
    for (int d = 0; subDirs[d]; d++) {
        wchar_t dPath[MAX_PATH];
        Join(dPath, g_exeDir, subDirs[d]);
        Join(path, dPath, fileName);
        if (FileExists(path) && GdipLoadImageFromFile(path, &img) == 0 && img) return img;
    }
    if (wcsstr(fileName, L"config") != NULL || wcsstr(fileName, L"Config") != NULL) {
        const wchar_t *cnames[] = {
            L"background_menu_configurações.png",
            L"Background_menu_configurações.png",
            L"background_config.png",
            NULL
        };
        for (int i = 0; cnames[i]; i++) {
            Join(path, g_exeDir, cnames[i]);
            if (FileExists(path) && GdipLoadImageFromFile(path, &img) == 0 && img) return img;
        }
        wchar_t pat[MAX_PATH]; Join(pat, g_exeDir, L"*config*.png");
        WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            Join(path, g_exeDir, fd.cFileName);
            FindClose(h);
            if (GdipLoadImageFromFile(path, &img) == 0 && img) return img;
        }
    } else if (wcsstr(fileName, L"extra") != NULL || wcsstr(fileName, L"Extra") != NULL) {
        const wchar_t *enames[] = {
            L"background_menu_extrações.png",
            L"Background_menu_extrações.png",
            L"background_extra.png",
            NULL
        };
        for (int i = 0; enames[i]; i++) {
            Join(path, g_exeDir, enames[i]);
            if (FileExists(path) && GdipLoadImageFromFile(path, &img) == 0 && img) return img;
        }
        wchar_t pat[MAX_PATH]; Join(pat, g_exeDir, L"*extra*.png");
        WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            Join(path, g_exeDir, fd.cFileName);
            FindClose(h);
            if (GdipLoadImageFromFile(path, &img) == 0 && img) return img;
        }
    }
    if (resId > 0) {
        img = LoadPngRes(resId);
        if (img) return img;
    }
    return NULL;
}
static void BuildBg(int w, int h)
{
    if (g_bgCache && g_bgW == w && g_bgH == h && g_bgTab == g_navTab) return;
    if (g_bgCache) DeleteObject(g_bgCache);
    g_bgTab = g_navTab;
    HDC wdc = GetDC(g_wnd), dc = CreateCompatibleDC(wdc);
    g_bgCache = CreateCompatibleBitmap(wdc, w, h); g_bgW = w; g_bgH = h;
    HGDIOBJ o = SelectObject(dc, g_bgCache);
    HBRUSH b = CreateSolidBrush(C_BG); RECT r = {0, 0, w, h}; FillRect(dc, &r, b); DeleteObject(b);
    GpGraphics *g = NULL; GdipCreateFromHDC(dc, &g);
    if (g) {
        GdipSetInterpolationMode(g, 7); GdipSetSmoothingMode(g, 4);
        GpImage *img = (g_navTab == 0) ? g_imgBgConfig : g_imgBgExtra;
        if (!img) img = g_imgBg;
        if (img) {   /* cover crop */
            UINT iw = 0, ih = 0; GdipGetImageWidth(img, &iw); GdipGetImageHeight(img, &ih);
            int sw = iw, sh = (int)((double)iw * h / w); if (sh > (int)ih) { sh = ih; sw = (int)((double)ih * w / h); }
            GdipDrawImageRectRectI(g, img, 0, 0, w, h, (iw - sw) / 2, (ih - sh) / 2, sw, sh, 2, NULL, NULL, NULL);
        }
        GdipDeleteGraphics(g);
    }
    SelectObject(dc, o); DeleteDC(dc); ReleaseDC(g_wnd, wdc);
}

/* ---------------------------------------------------------------- drawing primitives (GDI+, anti-aliased) */
static GpPath *RoundPath(int x, int y, int w, int h, int r)
{
    GpPath *p = NULL; GdipCreatePath(0, &p); int d = r * 2;
    if (r <= 0) { GdipAddPathLineI(p, x, y, x + w, y); GdipAddPathLineI(p, x + w, y, x + w, y + h); GdipAddPathLineI(p, x + w, y + h, x, y + h); }
    else {
        GdipAddPathArcI(p, x, y, d, d, 180, 90); GdipAddPathArcI(p, x + w - d, y, d, d, 270, 90);
        GdipAddPathArcI(p, x + w - d, y + h - d, d, d, 0, 90); GdipAddPathArcI(p, x, y + h - d, d, d, 90, 90);
    }
    GdipClosePathFigure(p); return p;
}
static void Box(GpGraphics *g, RECT r, int rad, UINT32 fill, UINT32 border, float bw)
{
    GpPath *p = RoundPath(r.left, r.top, r.right - r.left - 1, r.bottom - r.top - 1, rad);
    if (fill) { GpBrush *b = NULL; GdipCreateSolidFill(fill, &b); GdipFillPath(g, b, p); GdipDeleteBrush(b); }
    if (border) { GpPen *pen = NULL; GdipCreatePen1(border, bw, 2, &pen); GdipDrawPath(g, pen, p); GdipDeletePen(pen); }
    GdipDeletePath(p);
}
static void GradBox(GpGraphics *g, RECT r, int rad, UINT32 c1, UINT32 c2, BOOL fill, float bw)
{
    POINT a = {r.left, r.top}, z = {r.right, r.top}; GpBrush *lb = NULL; GdipCreateLineBrushI(&a, &z, c1, c2, 0, &lb);
    GpPath *p = RoundPath(r.left, r.top, r.right - r.left - 1, r.bottom - r.top - 1, rad);
    if (fill) GdipFillPath(g, lb, p); else { GpPen *pen = NULL; GdipCreatePen2(lb, bw, 2, &pen); GdipDrawPath(g, pen, p); GdipDeletePen(pen); }
    GdipDeletePath(p); GdipDeleteBrush(lb);
}

/* Broken geometry & Chamfer paths (matching Play button UFC styling) */
static GpPath *ChamferPath(int x, int y, int w, int h, int cut)
{
    GpPath *p = NULL; GdipCreatePath(0, &p);
    if (cut <= 0) {
        GdipAddPathLineI(p, x, y, x + w, y);
        GdipAddPathLineI(p, x + w, y, x + w, y + h);
        GdipAddPathLineI(p, x + w, y + h, x, y + h);
    } else {
        if (cut * 2 > w) cut = w / 2;
        if (cut * 2 > h) cut = h / 2;
        GdipAddPathLineI(p, x + cut, y, x + w - cut, y);
        GdipAddPathLineI(p, x + w - cut, y, x + w, y + cut);
        GdipAddPathLineI(p, x + w, y + cut, x + w, y + h - cut);
        GdipAddPathLineI(p, x + w, y + h - cut, x + w - cut, y + h);
        GdipAddPathLineI(p, x + w - cut, y + h, x + cut, y + h);
        GdipAddPathLineI(p, x + cut, y + h, x, y + h - cut);
        GdipAddPathLineI(p, x, y + h - cut, x, y + cut);
        GdipAddPathLineI(p, x, y + cut, x + cut, y);
    }
    GdipClosePathFigure(p); return p;
}

static GpPath *Cut2Path(int x, int y, int w, int h, int cut)
{
    GpPath *p = NULL; GdipCreatePath(0, &p);
    if (cut <= 0) {
        GdipAddPathLineI(p, x, y, x + w, y);
        GdipAddPathLineI(p, x + w, y, x + w, y + h);
        GdipAddPathLineI(p, x + w, y + h, x, y + h);
    } else {
        if (cut * 2 > w) cut = w / 2;
        if (cut * 2 > h) cut = h / 2;
        GdipAddPathLineI(p, x + cut, y, x + w, y);
        GdipAddPathLineI(p, x + w, y, x + w, y + h - cut);
        GdipAddPathLineI(p, x + w, y + h - cut, x + w - cut, y + h);
        GdipAddPathLineI(p, x + w - cut, y + h, x, y + h);
        GdipAddPathLineI(p, x, y + h, x, y + cut);
        GdipAddPathLineI(p, x, y + cut, x + cut, y);
    }
    GdipClosePathFigure(p); return p;
}

static void ChamferBox(GpGraphics *g, RECT r, int cut, UINT32 fill, UINT32 border, float bw)
{
    GpPath *p = ChamferPath(r.left, r.top, r.right - r.left - 1, r.bottom - r.top - 1, cut);
    if (fill) { GpBrush *b = NULL; GdipCreateSolidFill(fill, &b); GdipFillPath(g, b, p); GdipDeleteBrush(b); }
    if (border) { GpPen *pen = NULL; GdipCreatePen1(border, bw, 2, &pen); GdipDrawPath(g, pen, p); GdipDeletePen(pen); }
    GdipDeletePath(p);
}

static void Cut2Box(GpGraphics *g, RECT r, int cut, UINT32 fill, UINT32 border, float bw)
{
    GpPath *p = Cut2Path(r.left, r.top, r.right - r.left - 1, r.bottom - r.top - 1, cut);
    if (fill) { GpBrush *b = NULL; GdipCreateSolidFill(fill, &b); GdipFillPath(g, b, p); GdipDeleteBrush(b); }
    if (border) { GpPen *pen = NULL; GdipCreatePen1(border, bw, 2, &pen); GdipDrawPath(g, pen, p); GdipDeletePen(pen); }
    GdipDeletePath(p);
}

static void Text(HDC dc, HFONT f, COLORREF c, int x, int y, const wchar_t *s)
{ SelectObject(dc, f); SetTextColor(dc, c); TextOutW(dc, x, y, s, (int)wcslen(s)); }
static void TextR(HDC dc, HFONT f, COLORREF c, RECT r, const wchar_t *s, UINT fl)
{ SelectObject(dc, f); SetTextColor(dc, c); DrawTextW(dc, s, -1, &r, fl | DT_SINGLELINE | DT_NOPREFIX); }
static int TextW(HDC dc, HFONT f, const wchar_t *s) { SelectObject(dc, f); SIZE z; GetTextExtentPoint32W(dc, s, (int)wcslen(s), &z); return z.cx; }

static void SectionAt(HDC dc, GpGraphics *g, int x, int y, const wchar_t *t)
{
    /* Dynamic athletic paint badge / geometric accent */
    int ax = SC(x), ay = SC(y + 2), aw = SC(5), ah = SC(14), cut = SC(2);
    RECT rAcc = {ax, ay, ax + aw, ay + ah};
    Cut2Box(g, rAcc, cut, A(C_BLUE, 255), A(C_GOLD, 255), 1.0f);

    SetTextCharacterExtra(dc, SC(1));
    int tx = ax + aw + SC(8);
    int w = TextW(dc, g_fSection, t);
    Text(dc, g_fSection, RGB(255, 255, 255), tx, SC(y), t);
    SetTextCharacterExtra(dc, 0);

    /* Fading gradient brush-stroke line */
    POINT pt1 = {tx + w + SC(12), SC(y + 8)};
    POINT pt2 = {SC(X1), SC(y + 8)};
    if (pt2.x > pt1.x) {
        GpBrush *lb = NULL;
        GdipCreateLineBrushI(&pt1, &pt2, A(C_BLUE, 220), A(C_FIELDB, 0), 0, &lb);
        GpPen *pen = NULL;
        GdipCreatePen2(lb, (float)SC(1), 2, &pen);
        GdipDrawLineI(g, pen, pt1.x, pt1.y, pt2.x, pt2.y);
        GdipDeletePen(pen);
        GdipDeleteBrush(lb);
    }
}
static void Section(HDC dc, GpGraphics *g, int y, const wchar_t *t) { SectionAt(dc, g, X0, y, t); }

static void SectionRight(HDC dc, GpGraphics *g, int y, const wchar_t *t)
{
    SetTextCharacterExtra(dc, SC(1));
    int w = TextW(dc, g_fSection, t);
    int tx = SC(X1) - w;
    Text(dc, g_fSection, RGB(255, 255, 255), tx, SC(y), t);
    SetTextCharacterExtra(dc, 0);

    /* Dynamic athletic geometric accent badge on the left side of right-aligned title */
    int ax = tx - SC(13), ay = SC(y + 2), aw = SC(5), ah = SC(14), cut = SC(2);
    RECT rAcc = {ax, ay, ax + aw, ay + ah};
    Cut2Box(g, rAcc, cut, A(C_GOLD, 255), A(C_BLUE, 255), 1.0f);

    /* Fading gradient brush-stroke line coming from X0 towards ax */
    POINT pt1 = {SC(X0), SC(y + 8)};
    POINT pt2 = {ax - SC(10), SC(y + 8)};
    if (pt2.x > pt1.x) {
        GpBrush *lb = NULL;
        GdipCreateLineBrushI(&pt1, &pt2, A(C_FIELDB, 0), A(C_BLUE, 220), 0, &lb);
        GpPen *pen = NULL;
        GdipCreatePen2(lb, (float)SC(1), 2, &pen);
        GdipDrawLineI(g, pen, pt1.x, pt1.y, pt2.x, pt2.y);
        GdipDeletePen(pen);
        GdipDeleteBrush(lb);
    }
}

static void SectionCenter(HDC dc, GpGraphics *g, int y, const wchar_t *t)
{
    SetTextCharacterExtra(dc, SC(1));
    int w = TextW(dc, g_fSection, t);
    int midX = SC((X0 + X1) / 2);
    int tx = midX - w / 2;
    Text(dc, g_fSection, RGB(255, 255, 255), tx, SC(y), t);
    SetTextCharacterExtra(dc, 0);

    /* Left athletic dynamic accent badge */
    int ax1 = tx - SC(13), ay = SC(y + 2), aw = SC(5), ah = SC(14), cut = SC(2);
    RECT rAcc1 = {ax1, ay, ax1 + aw, ay + ah};
    Cut2Box(g, rAcc1, cut, A(C_BLUE, 255), A(C_GOLD, 255), 1.0f);

    /* Left fading gradient line from X0 towards ax1 */
    POINT ptL1 = {SC(X0), SC(y + 8)};
    POINT ptL2 = {ax1 - SC(10), SC(y + 8)};
    if (ptL2.x > ptL1.x) {
        GpBrush *lb = NULL;
        GdipCreateLineBrushI(&ptL1, &ptL2, A(C_FIELDB, 0), A(C_BLUE, 220), 0, &lb);
        GpPen *pen = NULL;
        GdipCreatePen2(lb, (float)SC(1), 2, &pen);
        GdipDrawLineI(g, pen, ptL1.x, ptL1.y, ptL2.x, ptL2.y);
        GdipDeletePen(pen);
        GdipDeleteBrush(lb);
    }

    /* Right athletic dynamic accent badge */
    int ax2 = tx + w + SC(8);
    RECT rAcc2 = {ax2, ay, ax2 + aw, ay + ah};
    Cut2Box(g, rAcc2, cut, A(C_GOLD, 255), A(C_BLUE, 255), 1.0f);

    /* Right fading gradient line from ax2 towards X1 */
    POINT ptR1 = {ax2 + aw + SC(10), SC(y + 8)};
    POINT ptR2 = {SC(X1), SC(y + 8)};
    if (ptR2.x > ptR1.x) {
        GpBrush *lb = NULL;
        GdipCreateLineBrushI(&ptR1, &ptR2, A(C_BLUE, 220), A(C_FIELDB, 0), 0, &lb);
        GpPen *pen = NULL;
        GdipCreatePen2(lb, (float)SC(1), 2, &pen);
        GdipDrawLineI(g, pen, ptR1.x, ptR1.y, ptR2.x, ptR2.y);
        GdipDeletePen(pen);
        GdipDeleteBrush(lb);
    }
}

/* Tab button: chamfered button matching the JOGAR PLAY button, placed in front of each fighter.
 * Hovering over an unselected button expands it to the enlarged size (isBig).
 * Clicking an unselected button selects it, keeping it at the enlarged size.
 * Clicking the other button shrinks this one and expands the other.
 * Hovering over the already selected button does nothing because it is already big.
 */
static void DrawTabButton(HDC dc, GpGraphics *g, int tabId, const wchar_t *title)
{
    BOOL active = (g_navTab == tabId);
    int hitId = (tabId == 0) ? H_TAB_CONFIG : H_TAB_PKG;
    BOOL hot = (g_hot == hitId);
    BOOL isBig = active || hot;

    /* Center coordinates (1280x720 base coordinates):
     * Tab 0 (Configurações): Centered in front of Anderson Silva at X=150
     * Tab 1 (Extração de PKG): Centered in front of Alex Poatan at X=430
     * Y center is at 612
     */
    int cx = (tabId == 0) ? 150 : 430;
    int cy = 612;

    int bw = isBig ? 260 : 220;
    int bh = isBig ? 50 : 42;
    int cut = SC(isBig ? 12 : 9);

    RECT r = {SC(cx - bw / 2), SC(cy - bh / 2), SC(cx + bw / 2), SC(cy + bh / 2)};

    /* Register hit area with the current active rectangle */
    AddHit(r.left, r.top, r.right, r.bottom, hitId);

    /* Chamfer geometry: top-left and bottom-right cut (matching JOGAR PlayButton) */
    GpPath *p = NULL;
    GdipCreatePath(0, &p);
    GdipAddPathLineI(p, r.left + cut, r.top, r.right, r.top);
    GdipAddPathLineI(p, r.right, r.top, r.right, r.bottom - cut);
    GdipAddPathLineI(p, r.right, r.bottom - cut, r.right - cut, r.bottom);
    GdipAddPathLineI(p, r.right - cut, r.bottom, r.left, r.bottom);
    GdipAddPathLineI(p, r.left, r.bottom, r.left, r.top + cut);
    GdipClosePathFigure(p);

    HFONT font = isBig ? g_fTabBig : g_fTabNorm;

    if (active) {
        /* SELECTED: Big Champion Gold gradient (identical to ready JOGAR button) */
        COLORREF c1 = RGB(255, 225, 45);
        COLORREF c2 = RGB(218, 175, 10);
        POINT ptA = {r.left, r.top}, ptB = {r.right, r.bottom};
        GpBrush *lb = NULL;
        GdipCreateLineBrushI(&ptA, &ptB, A(c1, 255), A(c2, 255), 0, &lb);
        GdipFillPath(g, lb, p);
        GdipDeleteBrush(lb);

        GpPen *pen = NULL;
        GdipCreatePen1(A(RGB(255, 248, 140), 220), 1.5f, 2, &pen);
        GdipDrawPath(g, pen, p);
        GdipDeletePen(pen);

        SetTextCharacterExtra(dc, 0);
        TextR(dc, font, RGB(10, 14, 24), r, title, DT_CENTER | DT_VCENTER);
    } else if (hot) {
        /* HOVERED UNSELECTED: Increased to BIG size, lights up to show it is selectable */
        COLORREF c1 = RGB(255, 238, 75);
        COLORREF c2 = RGB(225, 185, 18);
        POINT ptA = {r.left, r.top}, ptB = {r.right, r.bottom};
        GpBrush *lb = NULL;
        GdipCreateLineBrushI(&ptA, &ptB, A(c1, 250), A(c2, 250), 0, &lb);
        GdipFillPath(g, lb, p);
        GdipDeleteBrush(lb);

        GpPen *pen = NULL;
        GdipCreatePen1(A(RGB(255, 255, 180), 255), 2.0f, 2, &pen);
        GdipDrawPath(g, pen, p);
        GdipDeletePen(pen);

        SetTextCharacterExtra(dc, 0);
        TextR(dc, font, RGB(10, 14, 24), r, title, DT_CENTER | DT_VCENTER);
    } else {
        /* UNSELECTED NORMAL: Smaller size, sleek dark athletic glass */
        GpBrush *b = NULL;
        GdipCreateSolidFill(A(RGB(14, 20, 36), 230), &b);
        GdipFillPath(g, b, p);
        GdipDeleteBrush(b);

        /* Athletic streak on beveled corner */
        int streakW = SC(4);
        GpPath *ps = NULL;
        GdipCreatePath(0, &ps);
        GdipAddPathLineI(ps, r.left + cut, r.top, r.left + cut + streakW, r.top);
        GdipAddPathLineI(ps, r.left + streakW, r.bottom, r.left, r.bottom);
        GdipAddPathLineI(ps, r.left, r.top + cut, r.left + cut, r.top);
        GdipClosePathFigure(ps);
        GpBrush *bs = NULL;
        GdipCreateSolidFill(A(C_BLUE, 180), &bs);
        GdipFillPath(g, bs, ps);
        GdipDeleteBrush(bs);
        GdipDeletePath(ps);

        GpPen *pen = NULL;
        GdipCreatePen1(A(C_FIELDB, 200), 1.2f, 2, &pen);
        GdipDrawPath(g, pen, p);
        GdipDeletePen(pen);

        SetTextCharacterExtra(dc, 0);
        TextR(dc, font, RGB(215, 228, 245), r, title, DT_CENTER | DT_VCENTER);
    }

    GdipDeletePath(p);
}

/* ---------------------------------------------------------------- fields */
static const Field *Vid(int g) { (void)g; return VID4P; }
static int NVid(int g) { (void)g; return 9; }
static const Field *Gp(int g) { (void)g; return GP4; }
static const Field *Sw(int g, int *n) { (void)g; *n = N(SW4); return SW4; }
static int OptIndex(const Field *f, int v) { for (int i = 0; i < f->nopts; i++) if (f->opts[i].value == v) return i; return -1; }
static const wchar_t *OptName(const Opt *o) { return g_lang ? o->en : o->pt; }

static void Dropdown(HDC dc, GpGraphics *g, int x, int y, int w, const Field *f, int id)
{
    Text(dc, g_fSmall, C_SUB, SC(x), SC(y), T(f->label));
    RECT r = {SC(x), SC(y + 16), SC(x + w), SC(y + 42)};
    BOOL hot = (g_hot == id);
    int cut = SC(5);

    /* Chamfered dropdown box with dark glass fill */
    UINT32 bg = hot ? A(RGB(14, 20, 36), 245) : A(C_FIELD, 235);
    UINT32 border = hot ? A(C_BLUE, 255) : A(C_FIELDB, 220);
    Cut2Box(g, r, cut, bg, border, hot ? 1.5f : 1.0f);

    /* Left athletic paint accent bar (theme blue / gold) */
    RECT rAcc = {r.left + SC(1), r.top + cut, r.left + SC(4), r.bottom - cut};
    GpBrush *bAcc = NULL; GdipCreateSolidFill(hot ? A(C_GOLD, 255) : A(C_BLUE, 200), &bAcc);
    GdipFillRectangleI(g, bAcc, rAcc.left, rAcc.top, rAcc.right - rAcc.left, rAcc.bottom - rAcc.top);
    GdipDeleteBrush(bAcc);

    int v = GetInt(g_sel, f->key, f->def), i = OptIndex(f, v);
    wchar_t custom[32]; const wchar_t *name = i >= 0 ? OptName(&f->opts[i]) : (_snwprintf(custom, 32, L"%d", v), custom);
    RECT tr = {r.left + SC(12), r.top, r.right - SC(26), r.bottom};
    TextR(dc, g_fText, hot ? RGB(255, 255, 255) : C_TEXT, tr, name, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);

    /* Chevron arrow (gold when hot) */
    GpPen *pen = NULL; GdipCreatePen1(A(hot ? C_GOLD : C_SUB, 255), (float)SC(2), 2, &pen);
    int cx = r.right - SC(14), cy = (r.top + r.bottom) / 2;
    GdipDrawLineI(g, pen, cx - SC(4), cy - SC(2), cx, cy + SC(2));
    GdipDrawLineI(g, pen, cx, cy + SC(2), cx + SC(4), cy - SC(2));
    GdipDeletePen(pen);
    AddHit(r.left, r.top, r.right, r.bottom, id);
}

static void Switch(HDC dc, GpGraphics *g, int x, int y, const Field *f, int id)
{
    BOOL on = GetInt(g_sel, f->key, f->def) != 0;
    BOOL hot = (g_hot == id);
    RECT r = {SC(x), SC(y), SC(x + 40), SC(y + 20)};
    int cut = SC(5);

    if (on) {
        /* Selected / ON: Vibrant theme yellow/gold (from the PKG stroke) */
        COLORREF c1 = hot ? RGB(255, 248, 50) : RGB(244, 225, 20);
        COLORREF c2 = hot ? RGB(240, 210, 15) : RGB(225, 190, 10);
        POINT ptA = {r.left, r.top}, ptB = {r.right, r.bottom};
        GpBrush *lb = NULL; GdipCreateLineBrushI(&ptA, &ptB, A(c1, 255), A(c2, 255), 0, &lb);
        GpPath *p = Cut2Path(r.left, r.top, r.right - r.left - 1, r.bottom - r.top - 1, cut);
        GdipFillPath(g, lb, p);
        GpPen *pen = NULL; GdipCreatePen1(A(hot ? RGB(255, 255, 140) : RGB(255, 240, 60), 255), 1.0f, 2, &pen);
        GdipDrawPath(g, pen, p);
        GdipDeletePen(pen);
        GdipDeletePath(p);
        GdipDeleteBrush(lb);

        /* Thumb: High-contrast athletic dark thumb with electric blue center pin */
        int kx = r.right - SC(16);
        RECT rThumb = {kx, r.top + SC(2), kx + SC(14), r.top + SC(18)};
        Cut2Box(g, rThumb, SC(3), A(RGB(10, 14, 26), 255), A(RGB(30, 42, 68), 255), 1.0f);
        GpBrush *bDot = NULL; GdipCreateSolidFill(A(C_BLUE, 255), &bDot);
        GdipFillRectangleI(g, bDot, rThumb.left + SC(4), rThumb.top + SC(4), SC(6), SC(6));
        GdipDeleteBrush(bDot);
    } else {
        /* OFF: Dark night navy with broken chamfered geometry */
        Cut2Box(g, r, cut, A(RGB(12, 16, 28), 240), A(hot ? RGB(60, 85, 130) : C_FIELDB, 255), 1.0f);
        int kx = r.left + SC(2);
        RECT rThumb = {kx, r.top + SC(2), kx + SC(14), r.top + SC(18)};
        Cut2Box(g, rThumb, SC(3), A(RGB(40, 52, 75), 255), A(RGB(70, 85, 115), 255), 1.0f);
    }

    Text(dc, g_fText, on ? RGB(255, 255, 255) : (hot ? C_TEXT : C_SUB), SC(x + 48), SC(y + 1), T(f->label));
    AddHit(r.left, r.top, SC(x + 48) + TextW(dc, g_fText, T(f->label)), r.bottom, id);
}

static void Checkbox(HDC dc, GpGraphics *g, int x, int y, BOOL checked, const wchar_t *text, int id)
{
    BOOL hot = (g_hot == id);
    RECT rBox = {SC(x), SC(y), SC(x + 18), SC(y + 18)};
    int cut = SC(4);

    if (checked) {
        /* Selected: Electric blue theme color */
        Cut2Box(g, rBox, cut, A(C_BLUE, 255), A(hot ? RGB(80, 160, 255) : C_BLUE, 255), 1.0f);
        GpPen *pCheck = NULL; GdipCreatePen1(0xFFFFFFFF, (float)SC(2), 2, &pCheck);
        int cx = rBox.left, cy = rBox.top;
        GdipDrawLineI(g, pCheck, cx + SC(3), cy + SC(9), cx + SC(7), cy + SC(13));
        GdipDrawLineI(g, pCheck, cx + SC(7), cy + SC(13), cx + SC(14), cy + SC(4));
        GdipDeletePen(pCheck);
    } else {
        Cut2Box(g, rBox, cut, A(C_FIELD, 240), A(hot ? C_BLUE : C_FIELDB, 255), 1.0f);
    }
    Text(dc, g_fText, checked ? RGB(255, 255, 255) : (hot ? C_TEXT : C_SUB), SC(x + 26), SC(y + 1), text);
    int tw = TextW(dc, g_fText, text);
    AddHit(rBox.left, rBox.top, SC(x + 26) + tw, rBox.bottom, id);
}

static void CheckboxRight(HDC dc, GpGraphics *g, int y, BOOL checked, const wchar_t *text, int id)
{
    BOOL hot = (g_hot == id);
    int cut = SC(4);
    int boxSz = SC(18);
    int boxRight = SC(X1);
    int boxLeft = boxRight - boxSz;
    RECT rBox = {boxLeft, SC(y), boxRight, SC(y + 18)};

    if (checked) {
        Cut2Box(g, rBox, cut, A(C_BLUE, 255), A(hot ? RGB(80, 160, 255) : C_BLUE, 255), 1.0f);
        GpPen *pCheck = NULL; GdipCreatePen1(0xFFFFFFFF, (float)SC(2), 2, &pCheck);
        int cx = rBox.left, cy = rBox.top;
        GdipDrawLineI(g, pCheck, cx + SC(3), cy + SC(9), cx + SC(7), cy + SC(13));
        GdipDrawLineI(g, pCheck, cx + SC(7), cy + SC(13), cx + SC(14), cy + SC(4));
        GdipDeletePen(pCheck);
    } else {
        Cut2Box(g, rBox, cut, A(C_FIELD, 240), A(hot ? C_BLUE : C_FIELDB, 255), 1.0f);
    }
    int tw = TextW(dc, g_fText, text);
    int textX = boxLeft - SC(8) - tw;
    Text(dc, g_fText, checked ? RGB(255, 255, 255) : (hot ? C_TEXT : C_SUB), textX, SC(y + 1), text);
    AddHit(textX, rBox.top, boxRight, rBox.bottom, id);
}

static void Button(HDC dc, GpGraphics *g, RECT r, const wchar_t *t, int id)
{
    BOOL hot = (g_hot == id && id != -1);
    int cut = SC(6);

    /* Background and broken geometry */
    UINT32 bg = hot ? A(RGB(18, 26, 48), 245) : A(RGB(10, 14, 26), 235);
    UINT32 border = hot ? A(C_BLUE, 255) : A(C_FIELDB, 230);
    Cut2Box(g, r, cut, bg, border, hot ? 1.5f : 1.0f);

    /* Dynamic athletic paint streak / brush-slash on the beveled corner */
    int streakW = SC(4);
    GpPath *ps = NULL; GdipCreatePath(0, &ps);
    GdipAddPathLineI(ps, r.left + cut, r.top, r.left + cut + streakW, r.top);
    GdipAddPathLineI(ps, r.left + streakW, r.bottom, r.left, r.bottom);
    GdipAddPathLineI(ps, r.left, r.top + cut, r.left + cut, r.top);
    GdipClosePathFigure(ps);
    GpBrush *bs = NULL;
    GdipCreateSolidFill(hot ? A(C_GOLD, 255) : A(C_BLUE, 190), &bs);
    GdipFillPath(g, bs, ps);
    GdipDeleteBrush(bs);
    GdipDeletePath(ps);

    COLORREF tc = (id == -1) ? C_SUB : (hot ? RGB(255, 255, 255) : C_TEXT);
    TextR(dc, g_fBtn, tc, r, t, DT_CENTER | DT_VCENTER);

    if (id != -1) AddHit(r.left, r.top, r.right, r.bottom, id);
}

static void PlayButton(HDC dc, GpGraphics *g, RECT r)
{
    BOOL hot = (g_hot == H_PLAY);
    BOOL can = (Status(g_sel) == S_READY || g_proc);
    int cut = SC(14);
    GpPath *p = NULL; GdipCreatePath(0, &p);
    GdipAddPathLineI(p, r.left + cut, r.top, r.right, r.top);
    GdipAddPathLineI(p, r.right, r.top, r.right, r.bottom - cut);
    GdipAddPathLineI(p, r.right, r.bottom - cut, r.right - cut, r.bottom);
    GdipAddPathLineI(p, r.right - cut, r.bottom, r.left, r.bottom);
    GdipAddPathLineI(p, r.left, r.bottom, r.left, r.top + cut);
    GdipClosePathFigure(p);

    if (g_proc) {
        GpBrush *b = NULL; GdipCreateSolidFill(A(RGB(220, 50, 60), hot ? 255 : 230), &b);
        GdipFillPath(g, b, p); GdipDeleteBrush(b);
        SetTextCharacterExtra(dc, SC(1));
        TextR(dc, g_fPlay, RGB(255, 255, 255), r, T(S_STOP), DT_CENTER | DT_VCENTER);
    } else if (can) {
        COLORREF c1 = hot ? RGB(255, 225, 45) : RGB(242, 205, 25);
        COLORREF c2 = hot ? RGB(235, 195, 20) : RGB(218, 175, 10);
        POINT ptA = {r.left, r.top}, ptB = {r.right, r.bottom};
        GpBrush *lb = NULL;
        GdipCreateLineBrushI(&ptA, &ptB, A(c1, 255), A(c2, 255), 0, &lb);
        GdipFillPath(g, lb, p); GdipDeleteBrush(lb);
        SetTextCharacterExtra(dc, SC(1));
        TextR(dc, g_fPlay, RGB(10, 14, 24), r, T(S_PLAY), DT_CENTER | DT_VCENTER);
    } else {
        GpBrush *b = NULL; GdipCreateSolidFill(A(RGB(22, 28, 44), 200), &b);
        GdipFillPath(g, b, p); GdipDeleteBrush(b);
        SetTextCharacterExtra(dc, SC(1));
        TextR(dc, g_fPlay, RGB(90, 102, 125), r, T(S_PLAY), DT_CENTER | DT_VCENTER);
    }
    SetTextCharacterExtra(dc, 0);
    GdipDeletePath(p);
    AddHit(r.left, r.top, r.right, r.bottom, H_PLAY);
}

static COLORREF StatusColor(int s) {
    if (s == S_READY || s == S_RUNNING) return C_GOOD; /* Green when ready or playing */
    if (s == S_PICK || s == S_NOPORT) return C_BAD;   /* Red when game not selected or not found */
    return C_SUB;                                     /* Normal / subdued */
}

/* Validation functions for UFC 4 PKG files */
static BOOL IsValidUfc4Base(const PkgInfo *info)
{
    if (!info || !info->filePath[0]) return FALSE;
    if (!wcscmp(info->category, L"gp")) return FALSE;
    if (!wcscmp(info->titleId, L"CUSA14209") || !wcscmp(info->titleId, L"CUSA14204")) {
        return TRUE;
    }
    return FALSE;
}

static BOOL IsValidUfc4Patch(const PkgInfo *info)
{
    if (!info || !info->filePath[0]) return FALSE;
    if (!wcscmp(info->category, L"gd")) return FALSE;
    if (wcscmp(info->titleId, L"CUSA14209") && wcscmp(info->titleId, L"CUSA14204")) {
        return FALSE;
    }
    if (wcsstr(info->appVer, L"28.00") != NULL ||
        !wcsncmp(info->appVer, L"28.", 3) ||
        !wcscmp(info->appVer, L"028.00") ||
        wcsstr(info->version, L"28.00") != NULL ||
        !wcsncmp(info->version, L"28.", 3) ||
        !wcscmp(info->version, L"028.00")) {
        return TRUE;
    }
    return FALSE;
}

static void InitSplashParticles(int w, int h)
{
    UINT32 colors[] = {
        0xCCFF00, /* neon lime */
        0x00FFD5, /* cyan */
        0xFFFFFF  /* white */
    };
    for (int i = 0; i < SPLASH_NUM_PARTICLES; i++) {
        g_splashParticles[i].x = (float)(rand() % (w > 0 ? w : 1280));
        g_splashParticles[i].y = (float)(rand() % (h > 0 ? h : 720));
        g_splashParticles[i].size = 1.2f + (float)(rand() % 22) / 10.0f;
        g_splashParticles[i].speedY = 0.5f + (float)(rand() % 12) / 10.0f;
        g_splashParticles[i].speedX = ((float)(rand() % 20) - 10.0f) / 25.0f;
        g_splashParticles[i].alpha = 0.25f + (float)(rand() % 65) / 100.0f;
        g_splashParticles[i].color = colors[rand() % 3];
    }
    g_splashParticlesInit = TRUE;
}

static void PaintSplash(HDC dc, GpGraphics *g, int w, int h, float progress, DWORD elapsed)
{
    if (!g_splashParticlesInit) InitSplashParticles(w, h);
    SetBkMode(dc, TRANSPARENT);

    /* 1. Background layer with breathing animation and mouse parallax */
    float breath = 1.0f + 0.012f * sinf((float)elapsed * 0.003f);
    int bw = (int)((float)w * breath * 1.025f);
    int bh = (int)((float)h * breath * 1.025f);
    int bx = (w - bw) / 2 + (int)(g_splashMouseX * 6.0f);
    int by = (h - bh) / 2 + (int)(g_splashMouseY * 4.0f);

    if (g_imgSplashBg) {
        UINT bgW = 0, bgH = 0;
        GdipGetImageWidth(g_imgSplashBg, &bgW);
        GdipGetImageHeight(g_imgSplashBg, &bgH);
        GdipDrawImageRectRectI(g, g_imgSplashBg, bx, by, bw, bh, 0, 0, (INT)bgW, (INT)bgH, 2, NULL, NULL, NULL);
    } else {
        GpBrush *brFallback = NULL;
        GdipCreateSolidFill(0xFF030611, &brFallback);
        GdipFillRectangleI(g, brFallback, 0, 0, w, h);
        GdipDeleteBrush(brFallback);
    }

    /* 2. Floating neon canvas particles (sparks and mist) */
    for (int i = 0; i < SPLASH_NUM_PARTICLES; i++) {
        g_splashParticles[i].y -= g_splashParticles[i].speedY;
        g_splashParticles[i].x += g_splashParticles[i].speedX;
        if (g_splashParticles[i].y < -10.0f) {
            g_splashParticles[i].y = (float)(h + 10);
            g_splashParticles[i].x = (float)(rand() % (w > 0 ? w : 1280));
        }
        if (g_splashParticles[i].x < 0) g_splashParticles[i].x = (float)w;
        if (g_splashParticles[i].x > (float)w) g_splashParticles[i].x = 0;

        BYTE a = (BYTE)(g_splashParticles[i].alpha * 255.0f);
        UINT32 col = (a << 24) | g_splashParticles[i].color;
        GpBrush *brP = NULL;
        if (GdipCreateSolidFill(col, &brP) == 0 && brP) {
            int sz = (int)g_splashParticles[i].size;
            if (sz < 2) sz = 2;
            GdipFillEllipseI(g, brP, (int)g_splashParticles[i].x, (int)g_splashParticles[i].y, sz, sz);
            GdipDeleteBrush(brP);
        }
    }

    /* 3. Foreground text layer (1:1 aligned to background) with shimmer and chromatic aberration glitch */
    int fgx = (w - bw) / 2 + (int)(g_splashMouseX * 10.0f);
    int fgy = (h - bh) / 2 + (int)(g_splashMouseY * 7.0f);

    BOOL isGlitching = (elapsed >= 1200 && elapsed <= 1420) || (elapsed >= 1950 && elapsed <= 2080);

    if (g_imgSplashFg) {
        UINT fgW = 0, fgH = 0;
        GdipGetImageWidth(g_imgSplashFg, &fgW);
        GdipGetImageHeight(g_imgSplashFg, &fgH);

        if (isGlitching) {
            /* Chromatic aberration: Cyan offset (-3, 1) and Red offset (3, -1) */
            GdipDrawImageRectRectI(g, g_imgSplashFg, fgx - 3, fgy + 1, bw, bh, 0, 0, (INT)fgW, (INT)fgH, 2, NULL, NULL, NULL);
            GdipDrawImageRectRectI(g, g_imgSplashFg, fgx + 3, fgy - 1, bw, bh, 0, 0, (INT)fgW, (INT)fgH, 2, NULL, NULL, NULL);
        }

        /* Base foreground text */
        GdipDrawImageRectRectI(g, g_imgSplashFg, fgx, fgy, bw, bh, 0, 0, (INT)fgW, (INT)fgH, 2, NULL, NULL, NULL);

        /* Metallic shimmer sweep */
        float shimProg = (float)(elapsed % 2500) / 2500.0f;
        if (shimProg < 0.65f) {
            float st = shimProg / 0.65f;
            int sx = (int)(st * (w + SC(400))) - SC(200);
            GpBrush *brShim = NULL;
            if (GdipCreateSolidFill(0x1AFFFFFF, &brShim) == 0 && brShim) {
                GpPath *pShim = NULL;
                GdipCreatePath(0, &pShim);
                int shTopY = (int)(h * 0.28f);
                int shBotY = (int)(h * 0.56f);
                int shW = SC(60);
                GdipAddPathLineI(pShim, sx, shTopY, sx + shW, shTopY);
                GdipAddPathLineI(pShim, sx + shW - SC(70), shBotY, sx - SC(70), shBotY);
                GdipClosePathFigure(pShim);
                GdipFillPath(g, brShim, pShim);
                GdipDeletePath(pShim);
                GdipDeleteBrush(brShim);
            }
        }
    }

    /* 4. Fighter HUD loading bar unit (at 60% of height, centered horizontally) */
    int barW = min(SC(620), (int)(w * 0.52f));
    int barX = (w - barW) / 2;
    int barY = (int)(h * 0.60f);
    int barH = SC(13);
    int skew = (int)(barH * 0.287f); /* -16 degrees angle */

    /* Metadata line (pulsing green dot + status text on left, percentage on right) */
    int metaY = barY - SC(20);

    /* Pulsing status dot */
    float pulse = 0.5f + 0.5f * sinf((float)elapsed * 0.010f);
    BYTE dotA = (BYTE)(pulse * 190.0f + 65.0f);
    GpBrush *brDot = NULL;
    if (GdipCreateSolidFill((dotA << 24) | 0xCCFF00, &brDot) == 0 && brDot) {
        GdipFillEllipseI(g, brDot, barX, metaY + SC(5), SC(6), SC(6));
        GdipDeleteBrush(brDot);
    }

    /* Status label */
    const wchar_t *st = L"INICIALIZANDO PROGRAMA...";
    if (progress > 0.35f && progress < 0.75f) st = L"CARREGANDO RECURSOS...";
    else if (progress >= 0.75f && progress < 1.0f) st = L"FINALIZANDO INICIALIZAÇÃO...";
    else if (progress >= 1.0f) st = L"PRONTO!";

    Text(dc, g_fSplashStatus, RGB(184, 255, 0), barX + SC(14), metaY + SC(2), st);

    /* Percentage label */
    wchar_t pctStr[16];
    _snwprintf(pctStr, 16, L"%d%%", (int)(progress * 100.0f));
    int pctW = TextW(dc, g_fSplashPct, pctStr);
    Text(dc, g_fSplashPct, RGB(0, 255, 213), barX + barW - pctW, metaY - SC(1), pctStr);

    /* Bar track (skewed parallelogram) */
    GpPath *trackPath = NULL;
    GdipCreatePath(0, &trackPath);
    GdipAddPathLineI(trackPath, barX + skew, barY, barX + barW + skew, barY);
    GdipAddPathLineI(trackPath, barX + barW - skew, barY + barH, barX - skew, barY + barH);
    GdipClosePathFigure(trackPath);

    GpBrush *brTrackBg = NULL;
    GdipCreateSolidFill(0xEE03081C, &brTrackBg);
    GdipFillPath(g, brTrackBg, trackPath);
    GdipDeleteBrush(brTrackBg);

    /* Bar fill */
    int fillW = (int)((float)(barW - SC(4)) * progress);
    if (fillW > 0) {
        int fx = barX + SC(2);
        int fy = barY + SC(2);
        int fh = barH - SC(4);
        int fskew = (int)(fh * 0.287f);

        GpPath *fillPath = NULL;
        GdipCreatePath(0, &fillPath);
        GdipAddPathLineI(fillPath, fx + fskew, fy, fx + fillW + fskew, fy);
        GdipAddPathLineI(fillPath, fx + fillW - fskew, fy + fh, fx - fskew, fy + fh);
        GdipClosePathFigure(fillPath);

        /* Gradient: #B8FF00 (lime) to #00FFD5 (cyan) */
        POINT pt1 = {fx, fy};
        POINT pt2 = {barX + barW, fy};
        GpBrush *brGrad = NULL;
        GdipCreateLineBrushI(&pt1, &pt2, 0xFFB8FF00, 0xFF00FFD5, 0, &brGrad);
        GdipFillPath(g, brGrad, fillPath);
        GdipDeleteBrush(brGrad);

        /* Animated diagonal stripes clipped to fill */
        GdipSetClipPath(g, fillPath, 0);
        GpPen *penStripe = NULL;
        GdipCreatePen1(0x40000000, (float)SC(4), 2, &penStripe);
        int stripeScroll = (int)(elapsed / 25) % SC(12);
        for (int sx = fx - fh * 2 - SC(12); sx < fx + fillW + fh * 2; sx += SC(12)) {
            GdipDrawLineI(g, penStripe, sx + stripeScroll, fy - SC(2), sx + stripeScroll + fh, fy + fh + SC(2));
        }
        GdipDeletePen(penStripe);
        GdipResetClip(g);

        /* White & Cyan glowing lead bar at tip of fill */
        if (fillW < barW - SC(6)) {
            int lx = fx + fillW;
            GpPen *penGlow = NULL;
            GdipCreatePen1(0x9900FFD5, (float)SC(4), 2, &penGlow);
            GdipDrawLineI(g, penGlow, lx + fskew, fy - SC(1), lx - fskew, fy + fh + SC(1));
            GdipDeletePen(penGlow);

            GpPen *penWhite = NULL;
            GdipCreatePen1(0xFFFFFFFF, (float)SC(2), 2, &penWhite);
            GdipDrawLineI(g, penWhite, lx + fskew, fy - SC(1), lx - fskew, fy + fh + SC(1));
            GdipDeletePen(penWhite);
        }

        GdipDeletePath(fillPath);
    }

    /* Track border (cyan #00FFD5 outline) */
    GpPen *penTrack = NULL;
    GdipCreatePen1(0x7300FFD5, 1.5f, 2, &penTrack);
    GdipDrawPath(g, penTrack, trackPath);
    GdipDeletePen(penTrack);
    GdipDeletePath(trackPath);

    /* 5. Scanlines texture */
    GpPen *penScan = NULL;
    GdipCreatePen1(0x18000000, 1.0f, 2, &penScan);
    for (int sy = 0; sy < h; sy += 3) {
        GdipDrawLineI(g, penScan, 0, sy, w, sy);
    }
    GdipDeletePen(penScan);

    /* Corner vignette */
    GpPen *penVignette = NULL;
    GdipCreatePen1(0x40000000, (float)SC(10), 2, &penVignette);
    GdipDrawLineI(g, penVignette, 0, 0, w, 0);
    GdipDrawLineI(g, penVignette, 0, h, w, h);
    GdipDrawLineI(g, penVignette, 0, 0, 0, h);
    GdipDrawLineI(g, penVignette, w, 0, w, h);
    GdipDeletePen(penVignette);
}

/* ---------------------------------------------------------------- Desktop Shortcut & Fighter Icon Picker */
typedef struct {
    const wchar_t *fighterName;
    const wchar_t *icon1File;
    const wchar_t *icon2File;
} FighterCard;

static const FighterCard g_fighterCards[10] = {
    { L"Alex Poatan",           L"Alex_Poatan_1",           L"Alex_Poatan_2" },
    { L"Anderson Silva",        L"Anderson_Silva_1",        L"Anderson_Silva_2" },
    { L"Charles do Bronx",      L"Charles_do_Bronx_1",      L"Charles_do_Bronx_2" },
    { L"Conor McGregor",        L"Conor_McGregor_1",        L"Conor_McGregor_2" },
    { L"Georges St-Pierre",     L"Georges_St-Pierre_1",     L"Georges_St-Pierre_2" },
    { L"Islam Makhachev",       L"Islam_Makhachev_1",       L"Islam_Makhachev_2" },
    { L"Jon Jones",             L"Jon_Jones_1",             L"Jon_Jones_2" },
    { L"José Aldo",             L"Jose_Aldo_1",             L"Jose_Aldo_2" },
    { L"Khabib Nurmagomedov",   L"Khabib_Nurmagomedov_1",   L"Khabib_Nurmagomedov_2" },
    { L"Logo Original",         L"Original",                NULL }
};

typedef struct {
    const wchar_t *displayName;
    const wchar_t *fighterName;
    const wchar_t *baseName;
    int cardIdx;
    int variant;
} IconItem;

static const IconItem g_iconItems[19] = {
    { L"Alex Poatan (Estilo 1)",         L"Alex Poatan",         L"Alex_Poatan_1",         0, 1 },
    { L"Alex Poatan (Estilo 2)",         L"Alex Poatan",         L"Alex_Poatan_2",         0, 2 },
    { L"Anderson Silva (Estilo 1)",      L"Anderson Silva",      L"Anderson_Silva_1",      1, 1 },
    { L"Anderson Silva (Estilo 2)",      L"Anderson Silva",      L"Anderson_Silva_2",      1, 2 },
    { L"Charles do Bronx (Estilo 1)",    L"Charles do Bronx",    L"Charles_do_Bronx_1",    2, 1 },
    { L"Charles do Bronx (Estilo 2)",    L"Charles do Bronx",    L"Charles_do_Bronx_2",    2, 2 },
    { L"Conor McGregor (Estilo 1)",      L"Conor McGregor",      L"Conor_McGregor_1",      3, 1 },
    { L"Conor McGregor (Estilo 2)",      L"Conor McGregor",      L"Conor_McGregor_2",      3, 2 },
    { L"Georges St-Pierre (Estilo 1)",   L"Georges St-Pierre",   L"Georges_St-Pierre_1",   4, 1 },
    { L"Georges St-Pierre (Estilo 2)",   L"Georges St-Pierre",   L"Georges_St-Pierre_2",   4, 2 },
    { L"Islam Makhachev (Estilo 1)",     L"Islam Makhachev",     L"Islam_Makhachev_1",     5, 1 },
    { L"Islam Makhachev (Estilo 2)",     L"Islam Makhachev",     L"Islam_Makhachev_2",     5, 2 },
    { L"Jon Jones (Estilo 1)",           L"Jon Jones",           L"Jon_Jones_1",           6, 1 },
    { L"Jon Jones (Estilo 2)",           L"Jon Jones",           L"Jon_Jones_2",           6, 2 },
    { L"José Aldo (Estilo 1)",           L"José Aldo",           L"Jose_Aldo_1",           7, 1 },
    { L"José Aldo (Estilo 2)",           L"José Aldo",           L"Jose_Aldo_2",           7, 2 },
    { L"Khabib Nurmagomedov (Estilo 1)", L"Khabib Nurmagomedov", L"Khabib_Nurmagomedov_1", 8, 1 },
    { L"Khabib Nurmagomedov (Estilo 2)", L"Khabib Nurmagomedov", L"Khabib_Nurmagomedov_2", 8, 2 },
    { L"FightPort 4 (Logo Original)",    L"FightPort 4",         L"Original",              9, 0 }
};

static int g_selectedIconIndex = 18;
static BOOL g_iconPickerOpen = FALSE;
static GpImage *g_iconImages[19] = {0};
static BOOL g_iconsLoaded = FALSE;
static wchar_t g_iconMsg[256] = L"";
static COLORREF g_iconMsgColor = C_GOOD;

static void LoadIconPickerImages(void)
{
    if (g_iconsLoaded) return;
    for (int i = 0; i < 19; i++) {
        if (!g_iconImages[i]) {
            const wchar_t *subDirs[] = { L"src\\icons", L"icons", L"src", NULL };
            const wchar_t *exts[] = { L".ico", L".png", NULL };

            for (int d = 0; subDirs[d] && !g_iconImages[i]; d++) {
                for (int e = 0; exts[e] && !g_iconImages[i]; e++) {
                    wchar_t relPath[MAX_PATH];
                    _snwprintf(relPath, MAX_PATH, L"%s\\%s%s", subDirs[d], g_iconItems[i].baseName, exts[e]);
                    wchar_t fullPath[MAX_PATH];
                    Join(fullPath, g_exeDir, relPath);
                    if (FileExists(fullPath)) {
                        GdipLoadImageFromFile(fullPath, &g_iconImages[i]);
                    } else if (FileExists(relPath)) {
                        GdipLoadImageFromFile(relPath, &g_iconImages[i]);
                    }
                }
            }
            if (!g_iconImages[i] && i == 18 && g_imgLogo) {
                g_iconImages[i] = g_imgLogo;
            }
        }
    }
    g_iconsLoaded = TRUE;
}

static void FreeIconPickerImages(void)
{
    for (int i = 0; i < 19; i++) {
        if (g_iconImages[i]) {
            if (i == 18 && g_iconImages[i] == g_imgLogo) {
                /* retain g_imgLogo */
            } else {
                GdipDisposeImage(g_iconImages[i]);
            }
            g_iconImages[i] = NULL;
        }
    }
    g_iconsLoaded = FALSE;
}

static BOOL ShortcutExistsOnDesktop(void)
{
    wchar_t desktop[MAX_PATH];
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktop))) {
        return FALSE;
    }
    wchar_t lnk[MAX_PATH];
    Join(lnk, desktop, L"UFC 4.lnk");
    if (FileExists(lnk)) return TRUE;
    Join(lnk, desktop, L"FightPort 4.lnk");
    if (FileExists(lnk)) return TRUE;
    Join(lnk, desktop, L"EA SPORTS UFC 4.lnk");
    return FileExists(lnk);
}

static void LoadSelectedIcon(void)
{
    g_selectedIconIndex = GetPrivateProfileIntW(L"FightPort4", L"selected_icon", 18, g_pIni);
    if (g_selectedIconIndex < 0 || g_selectedIconIndex >= 19) {
        g_selectedIconIndex = 18;
    }
}

static void SaveSelectedIcon(void)
{
    wchar_t val[16];
    _snwprintf(val, 16, L"%d", g_selectedIconIndex);
    WritePrivateProfileStringW(L"FightPort4", L"selected_icon", val, g_pIni);
    if (g_selectedIconIndex >= 0 && g_selectedIconIndex < 19) {
        WritePrivateProfileStringW(L"FightPort4", L"selected_icon_name", g_iconItems[g_selectedIconIndex].baseName, g_pIni);
    }
}

static void CreateOrUpdateShortcut(BOOL forceRecreate)
{
    wchar_t desktop[MAX_PATH];
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktop))) {
        Msg(C_BAD, g_lang ? L"Could not locate Desktop folder." : L"Não foi possível localizar a Área de Trabalho.");
        _snwprintf(g_iconMsg, 256, g_lang ? L"Error: Desktop folder not found." : L"Erro: Pasta da Área de Trabalho não encontrada.");
        g_iconMsgColor = C_BAD;
        return;
    }

    wchar_t targetExe[MAX_PATH];
    GetModuleFileNameW(NULL, targetExe, MAX_PATH);

    wchar_t lnkPath[MAX_PATH];
    Join(lnkPath, desktop, L"UFC 4.lnk");

    wchar_t lnkFightPort[MAX_PATH];
    Join(lnkFightPort, desktop, L"FightPort 4.lnk");
    wchar_t lnkAlt[MAX_PATH];
    Join(lnkAlt, desktop, L"EA SPORTS UFC 4.lnk");

    if (!FileExists(lnkPath)) {
        if (FileExists(lnkFightPort)) {
            MoveFileW(lnkFightPort, lnkPath);
        } else if (FileExists(lnkAlt)) {
            MoveFileW(lnkAlt, lnkPath);
        }
    }

    BOOL alreadyExisted = FileExists(lnkPath);

    /* Determine icon path */
    wchar_t fullIcoPath[MAX_PATH] = L"";
    if (g_selectedIconIndex >= 0 && g_selectedIconIndex < 19) {
        wchar_t relIco[MAX_PATH];
        _snwprintf(relIco, MAX_PATH, L"src\\icons\\%s.ico", g_iconItems[g_selectedIconIndex].baseName);
        Join(fullIcoPath, g_exeDir, relIco);
        if (!FileExists(fullIcoPath)) {
            _snwprintf(relIco, MAX_PATH, L"icons\\%s.ico", g_iconItems[g_selectedIconIndex].baseName);
            Join(fullIcoPath, g_exeDir, relIco);
        }
    } else {
        Join(fullIcoPath, g_exeDir, L"src\\icons\\Original.ico");
        if (!FileExists(fullIcoPath)) {
            Join(fullIcoPath, g_exeDir, L"icons\\Original.ico");
        }
    }

    if (!FileExists(fullIcoPath)) {
        wcscpy(fullIcoPath, targetExe);
    }

    IShellLinkW *psl = NULL;
    HRESULT hr = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (void **)&psl);
    if (SUCCEEDED(hr)) {
        IPersistFile *ppf = NULL;
        hr = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void **)&ppf);
        if (SUCCEEDED(hr)) {
            if (!forceRecreate && alreadyExisted) {
                ppf->lpVtbl->Load(ppf, lnkPath, STGM_READWRITE);
            }
            psl->lpVtbl->SetPath(psl, targetExe);
            psl->lpVtbl->SetArguments(psl, L"--direct-play");
            psl->lpVtbl->SetWorkingDirectory(psl, g_exeDir);
            psl->lpVtbl->SetDescription(psl, L"EA SPORTS UFC 4");
            psl->lpVtbl->SetIconLocation(psl, fullIcoPath, 0);

            hr = ppf->lpVtbl->Save(ppf, lnkPath, TRUE);
            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }

    if (SUCCEEDED(hr)) {
        SaveSelectedIcon();
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW, lnkPath, NULL);
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

        const wchar_t *iconName = (g_selectedIconIndex >= 0 && g_selectedIconIndex < 19) 
            ? g_iconItems[g_selectedIconIndex].displayName : L"Original";

        if (alreadyExisted && !forceRecreate) {
            _snwprintf(g_iconMsg, 256, g_lang ? L"✔ Icon changed to: %s!"
                                              : L"✔ Ícone alterado com sucesso para: %s!", iconName);
            Msg(C_GOOD, g_lang ? L"Desktop shortcut icon changed to: %s"
                               : L"Ícone do atalho na Área de Trabalho alterado para: %s", iconName);
        } else {
            _snwprintf(g_iconMsg, 256, g_lang ? L"✔ Shortcut created on Desktop: UFC 4 (%s)!"
                                              : L"✔ Atalho criado na Área de Trabalho: UFC 4 (%s)!", iconName);
            Msg(C_GOOD, g_lang ? L"Shortcut created on Desktop: UFC 4.lnk"
                               : L"Atalho criado na Área de Trabalho: UFC 4.lnk");
        }
        g_iconMsgColor = C_GOOD;
    } else {
        _snwprintf(g_iconMsg, 256, g_lang ? L"Failed to create or update desktop shortcut."
                                          : L"Falha ao criar ou alterar atalho na Área de Trabalho.");
        g_iconMsgColor = C_BAD;
        Msg(C_BAD, g_lang ? L"Failed to create desktop shortcut." : L"Falha ao criar o atalho na Área de Trabalho.");
    }
}

static void CreateDesktopShortcut(void)
{
    CreateOrUpdateShortcut(TRUE);
}

static void ActionBtn(HDC dc, GpGraphics *g, RECT r, const wchar_t *t, int id, BOOL isGold)
{
    BOOL hot = (g_hot == id && id != -1);
    int cut = SC(6);

    UINT32 bg;
    UINT32 border;
    COLORREF tc;

    if (isGold) {
        bg = hot ? A(RGB(50, 40, 14), 250) : A(RGB(30, 22, 8), 240);
        border = hot ? A(RGB(255, 245, 140), 255) : A(C_GOLD, 240);
        tc = hot ? RGB(255, 255, 255) : C_GOLD;
    } else {
        bg = hot ? A(RGB(18, 26, 48), 245) : A(RGB(10, 14, 26), 235);
        border = hot ? A(C_BLUE, 255) : A(C_FIELDB, 230);
        tc = hot ? RGB(255, 255, 255) : C_TEXT;
    }

    Cut2Box(g, r, cut, bg, border, hot ? 1.6f : 1.0f);

    int streakW = SC(4);
    GpPath *ps = NULL; GdipCreatePath(0, &ps);
    GdipAddPathLineI(ps, r.left + cut, r.top, r.left + cut + streakW, r.top);
    GdipAddPathLineI(ps, r.left + streakW, r.bottom, r.left, r.bottom);
    GdipAddPathLineI(ps, r.left, r.top + cut, r.left + cut, r.top);
    GdipClosePathFigure(ps);
    GpBrush *bs = NULL;
    GdipCreateSolidFill(isGold ? A(C_GOLD, 255) : (hot ? A(C_BLUE, 255) : A(C_FIELDB, 200)), &bs);
    GdipFillPath(g, bs, ps);
    GdipDeleteBrush(bs);
    GdipDeletePath(ps);

    TextR(dc, g_fBtn, tc, r, t, DT_CENTER | DT_VCENTER);
    if (id != -1) AddHit(r.left, r.top, r.right, r.bottom, id);
}

static void DrawIconPickerModal(HDC dc, GpGraphics *g, int w, int h)
{
    /* 1. Dark semi-transparent dim overlay over the entire screen */
    GpBrush *dim = NULL;
    GdipCreateSolidFill(0xEA030612, &dim);
    GdipFillRectangleI(g, dim, 0, 0, w, h);
    GdipDeleteBrush(dim);

    /* Modal absorbs background clicks */
    AddHit(0, 0, w, h, H_ICON_MODAL_BG);

    /* 2. Modal Card Frame */
    RECT rModal = { SC(40), SC(33), SC(1240), SC(687) };
    Cut2Box(g, rModal, SC(10), A(RGB(9, 13, 26), 252), A(RGB(45, 65, 110), 255), 1.5f);

    /* Athletic top accent line */
    GpPen *penTop = NULL;
    GdipCreatePen1(A(C_BLUE, 220), (float)SC(2), 2, &penTop);
    GdipDrawLineI(g, penTop, rModal.left + SC(20), rModal.top, rModal.right - SC(20), rModal.top);
    GdipDeletePen(penTop);

    /* 3. Header */
    Text(dc, g_fCard, RGB(255, 255, 255), SC(65), SC(46),
         g_lang ? L"DESKTOP SHORTCUT & EXECUTABLE ICON SELECTOR"
                : L"PERSONALIZAR ÍCONE DO JOGO NA ÁREA DE TRABALHO");

    Text(dc, g_fSmall, C_SUB, SC(67), SC(76),
         g_lang ? L"Choose from 19 icons (9 fighters with 2 photo variations each + original logo). Create shortcut or change icon anytime."
                : L"Escolha entre 19 opções (9 lutadores com 2 variações cada + logo original). Crie o executável/atalho ou altere o ícone.");

    /* Close [X] Button */
    RECT rClose = { SC(1192), SC(44), SC(1226), SC(76) };
    BOOL closeHot = (g_hot == H_ICON_CLOSE);
    Cut2Box(g, rClose, SC(4), closeHot ? A(RGB(180, 40, 40), 220) : A(RGB(20, 26, 44), 220),
            closeHot ? A(RGB(255, 80, 80), 255) : A(C_FIELDB, 200), 1.0f);
    TextR(dc, g_fBtn, closeHot ? RGB(255, 255, 255) : C_SUB, rClose, L"✕", DT_CENTER | DT_VCENTER);
    AddHit(rClose.left, rClose.top, rClose.right, rClose.bottom, H_ICON_CLOSE);

    /* 4. 10 Cards Grid (5 columns x 2 rows) */
    int gridStartX = SC(52);
    int gridStartY = SC(104);
    int cardW = SC(224);
    int cardH = SC(162);
    int gapX = SC(14);
    int gapY = SC(12);

    for (int c = 0; c < 10; c++) {
        int col = c % 5;
        int row = c / 5;
        int cx = gridStartX + col * (cardW + gapX);
        int cy = gridStartY + row * (cardH + gapY);
        RECT rCard = { cx, cy, cx + cardW, cy + cardH };

        BOOL cardActive = (c < 9 && (g_selectedIconIndex == c * 2 || g_selectedIconIndex == c * 2 + 1)) ||
                          (c == 9 && g_selectedIconIndex == 18);

        Cut2Box(g, rCard, SC(6),
                cardActive ? A(RGB(14, 22, 44), 240) : A(RGB(8, 12, 24), 230),
                cardActive ? A(C_BLUE, 210) : A(C_FIELDB, 150),
                cardActive ? 1.5f : 1.0f);

        if (c < 9) {
            /* Fighter Card: 2 icons side-by-side */
            int iw = SC(86), ih = SC(86);
            int iy = cy + SC(12);
            int ix1 = cx + SC(18);
            int ix2 = cx + SC(120);

            int idxs[2] = { c * 2, c * 2 + 1 };
            int ixs[2] = { ix1, ix2 };

            for (int v = 0; v < 2; v++) {
                int idx = idxs[v];
                RECT rIcon = { ixs[v], iy, ixs[v] + iw, iy + ih };
                BOOL isSel = (g_selectedIconIndex == idx);
                BOOL isHot = (g_hot == H_ICON_START + idx);

                Cut2Box(g, rIcon, SC(4), A(RGB(4, 6, 14), 255),
                        isSel ? A(C_GOLD, 255) : (isHot ? A(C_BLUE, 255) : A(C_FIELDB, 180)),
                        isSel ? 2.5f : (isHot ? 1.8f : 1.0f));

                if (g_iconImages[idx]) {
                    UINT wImg = 0, hImg = 0;
                    GdipGetImageWidth(g_iconImages[idx], &wImg);
                    GdipGetImageHeight(g_iconImages[idx], &hImg);
                    GdipDrawImageRectRectI(g, g_iconImages[idx],
                                           rIcon.left + SC(3), rIcon.top + SC(3), iw - SC(6), ih - SC(6),
                                           0, 0, (INT)wImg, (INT)hImg, 2, NULL, NULL, NULL);
                }

                if (isSel) {
                    int bw = SC(18);
                    RECT rBadge = { rIcon.right - bw, rIcon.top, rIcon.right, rIcon.top + bw };
                    Cut2Box(g, rBadge, SC(2), A(C_GOLD, 255), A(RGB(255, 255, 200), 255), 1.0f);
                    TextR(dc, g_fSmall, RGB(0, 0, 0), rBadge, L"✓", DT_CENTER | DT_VCENTER);
                }

                AddHit(rIcon.left, rIcon.top, rIcon.right, rIcon.bottom, H_ICON_START + idx);
            }

            /* Single clean name centered underneath the pair */
            RECT rName = { cx + SC(4), cy + SC(108), cx + cardW - SC(4), cy + cardH - SC(8) };
            COLORREF nameCol = cardActive ? C_GOLD : RGB(235, 240, 255);
            TextR(dc, g_fSection, nameCol, rName, g_fighterCards[c].fighterName, DT_CENTER | DT_VCENTER);

        } else {
            /* Card 9: Original Logo (centered) */
            int iw = SC(86), ih = SC(86);
            int iy = cy + SC(12);
            int ix = cx + (cardW - iw) / 2;
            int idx = 18;

            RECT rIcon = { ix, iy, ix + iw, iy + ih };
            BOOL isSel = (g_selectedIconIndex == idx);
            BOOL isHot = (g_hot == H_ICON_START + idx);

            Cut2Box(g, rIcon, SC(4), A(RGB(4, 6, 14), 255),
                    isSel ? A(C_GOLD, 255) : (isHot ? A(C_BLUE, 255) : A(C_FIELDB, 180)),
                    isSel ? 2.5f : (isHot ? 1.8f : 1.0f));

            if (g_iconImages[idx]) {
                UINT wImg = 0, hImg = 0;
                GdipGetImageWidth(g_iconImages[idx], &wImg);
                GdipGetImageHeight(g_iconImages[idx], &hImg);
                GdipDrawImageRectRectI(g, g_iconImages[idx],
                                       rIcon.left + SC(3), rIcon.top + SC(3), iw - SC(6), ih - SC(6),
                                       0, 0, (INT)wImg, (INT)hImg, 2, NULL, NULL, NULL);
            }

            if (isSel) {
                int bw = SC(18);
                RECT rBadge = { rIcon.right - bw, rIcon.top, rIcon.right, rIcon.top + bw };
                Cut2Box(g, rBadge, SC(2), A(C_GOLD, 255), A(RGB(255, 255, 200), 255), 1.0f);
                TextR(dc, g_fSmall, RGB(0, 0, 0), rBadge, L"✓", DT_CENTER | DT_VCENTER);
            }

            AddHit(rIcon.left, rIcon.top, rIcon.right, rIcon.bottom, H_ICON_START + idx);

            /* Centered name underneath */
            RECT rName = { cx + SC(4), cy + SC(108), cx + cardW - SC(4), cy + cardH - SC(8) };
            COLORREF nameCol = isSel ? C_GOLD : RGB(235, 240, 255);
            TextR(dc, g_fSection, nameCol, rName, g_lang ? L"FightPort 4 (Original)" : L"Logo Original (FightPort 4)", DT_CENTER | DT_VCENTER);
        }
    }

    /* 5. Bottom Control & Info Panel */
    RECT rBottom = { SC(52), SC(454), SC(1228), SC(672) };
    Cut2Box(g, rBottom, SC(6), A(RGB(8, 12, 24), 245), A(C_FIELDB, 200), 1.0f);

    /* Selected Icon High-Res Preview (Grandona, quase a tarja inteira) */
    RECT rPrev = { SC(66), SC(464), SC(66 + 196), SC(464 + 196) };
    Cut2Box(g, rPrev, SC(6), A(RGB(4, 6, 14), 255), A(C_GOLD, 255), 2.5f);
    if (g_iconImages[g_selectedIconIndex]) {
        UINT pw = 0, ph = 0;
        GdipGetImageWidth(g_iconImages[g_selectedIconIndex], &pw);
        GdipGetImageHeight(g_iconImages[g_selectedIconIndex], &ph);
        GdipDrawImageRectRectI(g, g_iconImages[g_selectedIconIndex],
                               rPrev.left + SC(5), rPrev.top + SC(5), SC(186), SC(186),
                               0, 0, (INT)pw, (INT)ph, 2, NULL, NULL, NULL);
    }

    /* Info empurrada para o lado */
    Text(dc, g_fSmall, C_GOLD, SC(280), SC(472),
         g_lang ? L"SELECTED ICON:" : L"ÍCONE SELECIONADO:");

    Text(dc, g_fCard, RGB(255, 255, 255), SC(280), SC(496),
         g_iconItems[g_selectedIconIndex].displayName);

    wchar_t fInfo[128];
    _snwprintf(fInfo, 128, L"src\\icons\\%s.ico", g_iconItems[g_selectedIconIndex].baseName);
    Text(dc, g_fSmall, C_SUB, SC(280), SC(528), fInfo);

    BOOL shortcutExists = ShortcutExistsOnDesktop();
    if (shortcutExists) {
        Text(dc, g_fSmall, C_GOOD, SC(280), SC(554),
             g_lang ? L"● Desktop Shortcut: FOUND (UFC 4.lnk)"
                    : L"● Atalho na Área de Trabalho: ENCONTRADO (UFC 4.lnk)");
    } else {
        Text(dc, g_fSmall, C_WARN, SC(280), SC(554),
             g_lang ? L"○ Desktop Shortcut: NOT CREATED YET"
                    : L"○ Atalho na Área de Trabalho: NÃO CRIADO AINDA");
    }

    /* Action Buttons (Apenas Criar e Alterar, sem botão Fechar redundante) */
    RECT rBtnCreate = { SC(650), SC(480), SC(924), SC(538) };
    ActionBtn(dc, g, rBtnCreate,
              g_lang ? L"CREATE DESKTOP SHORTCUT" : L"CRIAR ATALHO NO DESKTOP",
              H_ICON_CREATE_SHORTCUT, FALSE);

    RECT rBtnChange = { SC(938), SC(480), SC(1214), SC(538) };
    ActionBtn(dc, g, rBtnChange,
              g_lang ? L"CHANGE SHORTCUT ICON" : L"ALTERAR ÍCONE DO ATALHO",
              H_ICON_CHANGE_SHORTCUT, TRUE);

    /* Feedback Banner */
    RECT rMsg = { SC(280), SC(590), SC(1214), SC(648) };
    if (g_iconMsg[0] != 0) {
        TextR(dc, g_fText, g_iconMsgColor, rMsg, g_iconMsg, DT_LEFT | DT_VCENTER);
    } else {
        TextR(dc, g_fSmall, C_SUB, rMsg,
              g_lang ? L"Hint: Double-clicking the shortcut on your desktop starts UFC 4 directly with all saved settings."
                     : L"Dica: O atalho na Área de Trabalho abre o UFC 4 diretamente com todas as opções e configurações salvas.",
              DT_LEFT | DT_VCENTER);
    }
}

static void DrawThemedModal(HDC dc, GpGraphics *g, int w, int h)
{
    /* 1. Fullscreen dark dim overlay */
    GpBrush *dim = NULL;
    GdipCreateSolidFill(0xEA030612, &dim);
    GdipFillRectangleI(g, dim, 0, 0, w, h);
    GdipDeleteBrush(dim);

    /* Absorb background clicks */
    AddHit(0, 0, w, h, H_THEMED_MODAL_BG);

    /* 2. Modal card dimensions (centered, athletic champion card) */
    int mw = SC(680);
    int len = (int)wcslen(g_themedModalText);
    int mh = SC(350);
    if (len > 300) mh = SC(390);
    if (len > 550) mh = SC(440);
    if (len > 900) mh = SC(500);

    int mx0 = (w - mw) / 2;
    int my0 = (h - mh) / 2;
    if (my0 < SC(20)) my0 = SC(20);
    RECT rModal = { mx0, my0, mx0 + mw, my0 + mh };

    /* Border color based on icon severity */
    COLORREF borderCol = (g_themedModalIcon == THEMED_ICON_ERROR) ? RGB(240, 50, 60)
                       : (g_themedModalIcon == THEMED_ICON_WARN)  ? RGB(240, 168, 30)
                       : (g_themedModalIcon == THEMED_ICON_QUESTION) ? C_GOLD
                       : C_BLUE;

    Cut2Box(g, rModal, SC(10), A(RGB(9, 13, 26), 252), A(borderCol, 255), 1.5f);

    /* Champion athletic top accent gradient line */
    POINT pA = { rModal.left + SC(16), rModal.top };
    POINT pB = { rModal.right - SC(16), rModal.top };
    GpBrush *lbTop = NULL;
    GdipCreateLineBrushI(&pA, &pB, A(borderCol, 255), A(C_BLUE, 255), 0, &lbTop);
    GpPen *pTop = NULL;
    GdipCreatePen2(lbTop, (float)SC(2), 2, &pTop);
    GdipDrawLineI(g, pTop, pA.x, pA.y, pB.x, pB.y);
    GdipDeletePen(pTop);
    GdipDeleteBrush(lbTop);

    /* Icon Badge in Header (top-left) */
    RECT rBadge = { rModal.left + SC(22), rModal.top + SC(18), rModal.left + SC(54), rModal.top + SC(50) };
    Cut2Box(g, rBadge, SC(6), A(RGB(14, 20, 38), 255), A(borderCol, 255), 1.2f);
    const wchar_t *iconSym = (g_themedModalIcon == THEMED_ICON_ERROR) ? L"✖"
                           : (g_themedModalIcon == THEMED_ICON_WARN)  ? L"⚠"
                           : (g_themedModalIcon == THEMED_ICON_QUESTION) ? L"?"
                           : L"ℹ";
    TextR(dc, g_fBtn, borderCol, rBadge, iconSym, DT_CENTER | DT_VCENTER);

    /* Header Title */
    RECT rTitle = { rModal.left + SC(62), rModal.top + SC(18), rModal.right - SC(60), rModal.top + SC(50) };
    TextR(dc, g_fCard, RGB(255, 255, 255), rTitle, g_themedModalTitle, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);

    /* Close [X] Button */
    RECT rClose = { rModal.right - SC(46), rModal.top + SC(18), rModal.right - SC(18), rModal.top + SC(46) };
    BOOL closeHot = (g_hot == H_THEMED_MODAL_CLOSE);
    Cut2Box(g, rClose, SC(4), closeHot ? A(RGB(180, 40, 40), 220) : A(RGB(20, 26, 44), 220),
            closeHot ? A(RGB(255, 80, 80), 255) : A(C_FIELDB, 200), 1.0f);
    TextR(dc, g_fBtn, closeHot ? RGB(255, 255, 255) : C_SUB, rClose, L"✕", DT_CENTER | DT_VCENTER);
    AddHit(rClose.left, rClose.top, rClose.right, rClose.bottom, H_THEMED_MODAL_CLOSE);

    /* Separator line under header */
    GpPen *pSep = NULL;
    GdipCreatePen1(A(RGB(30, 42, 70), 200), (float)SC(1), 2, &pSep);
    GdipDrawLineI(g, pSep, rModal.left + SC(18), rModal.top + SC(58), rModal.right - SC(18), rModal.top + SC(58));
    GdipDeletePen(pSep);

    /* Body Message Area */
    RECT rBody = { rModal.left + SC(24), rModal.top + SC(72), rModal.right - SC(24), rModal.bottom - SC(68) };
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(220, 230, 245));
    HFONT bodyFont = (len > 320) ? g_fSmall : g_fText;
    SelectObject(dc, bodyFont);
    DrawTextW(dc, g_themedModalText, -1, &rBody, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

    /* Buttons Area at bottom */
    int btnH = SC(38);
    int btnBottom = rModal.bottom - SC(18);
    int btnTop = btnBottom - btnH;

    if (g_themedModalButtons == THEMED_MODAL_OK) {
        int btnW = SC(160);
        RECT rBtn = { rModal.right - SC(24) - btnW, btnTop, rModal.right - SC(24), btnBottom };
        BOOL hot = (g_hot == H_THEMED_MODAL_BTN2);
        COLORREF c1 = hot ? RGB(255, 225, 45) : RGB(242, 205, 25);
        COLORREF c2 = hot ? RGB(235, 195, 20) : RGB(218, 175, 10);
        POINT ptA = {rBtn.left, rBtn.top}, ptB = {rBtn.right, rBtn.bottom};
        GpBrush *lb = NULL; GdipCreateLineBrushI(&ptA, &ptB, A(c1, 255), A(c2, 255), 0, &lb);
        GpPath *pBtn = Cut2Path(rBtn.left, rBtn.top, rBtn.right - rBtn.left - 1, rBtn.bottom - rBtn.top - 1, SC(6));
        GdipFillPath(g, lb, pBtn); GdipDeleteBrush(lb); GdipDeletePath(pBtn);

        const wchar_t *lbl = g_themedModalBtn2Label ? g_themedModalBtn2Label : (g_lang ? L"OK" : L"ENTENDIDO");
        SetTextCharacterExtra(dc, SC(1));
        TextR(dc, g_fBtn, RGB(10, 14, 24), rBtn, lbl, DT_CENTER | DT_VCENTER);
        SetTextCharacterExtra(dc, 0);
        AddHit(rBtn.left, rBtn.top, rBtn.right, rBtn.bottom, H_THEMED_MODAL_BTN2);
    } else {
        int btnW = SC(175);
        RECT rBtn2 = { rModal.right - SC(24) - btnW, btnTop, rModal.right - SC(24), btnBottom };
        RECT rBtn1 = { rBtn2.left - SC(14) - btnW, btnTop, rBtn2.left - SC(14), btnBottom };

        /* Secondary button (Btn1) */
        const wchar_t *lbl1 = g_themedModalBtn1Label ? g_themedModalBtn1Label
                            : (g_themedModalButtons == THEMED_MODAL_YESNO ? (g_lang ? L"NO" : L"NÃO") : (g_lang ? L"CANCEL" : L"CANCELAR"));
        Button(dc, g, rBtn1, lbl1, H_THEMED_MODAL_BTN1);

        /* Primary button (Btn2) */
        BOOL hot2 = (g_hot == H_THEMED_MODAL_BTN2);
        COLORREF c1 = hot2 ? RGB(255, 225, 45) : RGB(242, 205, 25);
        COLORREF c2 = hot2 ? RGB(235, 195, 20) : RGB(218, 175, 10);
        POINT ptA = {rBtn2.left, rBtn2.top}, ptB = {rBtn2.right, rBtn2.bottom};
        GpBrush *lb = NULL; GdipCreateLineBrushI(&ptA, &ptB, A(c1, 255), A(c2, 255), 0, &lb);
        GpPath *pBtn = Cut2Path(rBtn2.left, rBtn2.top, rBtn2.right - rBtn2.left - 1, rBtn2.bottom - rBtn2.top - 1, SC(6));
        GdipFillPath(g, lb, pBtn); GdipDeleteBrush(lb); GdipDeletePath(pBtn);

        const wchar_t *lbl2 = g_themedModalBtn2Label ? g_themedModalBtn2Label
                            : (g_themedModalButtons == THEMED_MODAL_YESNO ? (g_lang ? L"YES" : L"SIM") : (g_lang ? L"CONTINUE" : L"CONTINUAR"));
        SetTextCharacterExtra(dc, SC(1));
        TextR(dc, g_fBtn, RGB(10, 14, 24), rBtn2, lbl2, DT_CENTER | DT_VCENTER);
        SetTextCharacterExtra(dc, 0);
        AddHit(rBtn2.left, rBtn2.top, rBtn2.right, rBtn2.bottom, H_THEMED_MODAL_BTN2);
    }
}

static void Paint(HDC wdc)
{
    RECT rc; GetClientRect(g_wnd, &rc);
    HDC dc = CreateCompatibleDC(wdc); HBITMAP bmp = CreateCompatibleBitmap(wdc, rc.right, rc.bottom); HGDIOBJ ob = SelectObject(dc, bmp);

    if (g_splashActive) {
        GpGraphics *g = NULL; GdipCreateFromHDC(dc, &g); GdipSetSmoothingMode(g, 4); GdipSetPixelOffsetMode(g, 4);
        ULONGLONG now = GetTickCount64();
        DWORD elapsed = (DWORD)(now >= g_splashStart ? now - g_splashStart : 0);
        float progress = min(1.0f, (float)elapsed / (float)SPLASH_DURATION_MS);
        PaintSplash(dc, g, rc.right, rc.bottom, progress, elapsed);
        GdipDeleteGraphics(g);
        BitBlt(wdc, 0, 0, rc.right, rc.bottom, dc, 0, 0, SRCCOPY);
        SelectObject(dc, ob); DeleteObject(bmp); DeleteDC(dc);
        return;
    }

    BuildBg(rc.right, rc.bottom);
    { HDC m = CreateCompatibleDC(wdc); HGDIOBJ o = SelectObject(m, g_bgCache); BitBlt(dc, 0, 0, rc.right, rc.bottom, m, 0, 0, SRCCOPY); SelectObject(m, o); DeleteDC(m); }
    SetBkMode(dc, TRANSPARENT); g_nhits = 0;
    GpGraphics *g = NULL; GdipCreateFromHDC(dc, &g); GdipSetSmoothingMode(g, 4); GdipSetPixelOffsetMode(g, 4);

    /* --- LEFT SIDE: TWO REAL BUTTONS IN FRONT OF THE FIGHTERS (JOGAR STYLE) --- */
    /* Tab 0 (Left, centered in front of Alex Poatan): Configurações */
    DrawTabButton(dc, g, 0, g_lang ? L"SETTINGS" : L"CONFIGURAÇÕES");

    /* Tab 1 (Right, centered in front of 2nd fighter): Extração de PKG */
    DrawTabButton(dc, g, 1, g_lang ? L"PKG EXTRACTOR" : L"EXTRAÇÃO DE PKG");

    /* Subtle footer info at bottom left */
    Text(dc, g_fFoot, A(C_SUB, 150), SC(36), SC(686), L"FightPort 4 1.3.0  •  UFC 4 PC PORT");

    /* --- TOP LEFT: LANGUAGE BUTTONS (EN / PT-BR) & STATUS PILL (COMPACT) --- */
    int xLeft = SC(26), topY = SC(14), botY = SC(36);
    int ptW = SC(46), enW = SC(32), langGap = SC(4), boxGap = SC(8);

    RECT rEN = {xLeft, topY, xLeft + enW, botY};
    RECT rPT = {rEN.right + langGap, topY, rEN.right + langGap + ptW, botY};

    /* EN button */
    BOOL enSel = (g_lang == 1), enHot = (g_hot == H_EN);
    Cut2Box(g, rEN, SC(3), A(enSel ? RGB(16, 26, 52) : RGB(8, 11, 20), 220),
        A(enSel ? C_BLUE : (enHot ? RGB(80, 110, 160) : C_FIELDB), 255), enSel ? 1.5f : 1.0f);
    TextR(dc, g_fSmall, enSel ? C_BLUE : (enHot ? C_TEXT : C_SUB), rEN, L"EN", DT_CENTER | DT_VCENTER);
    AddHit(rEN.left, rEN.top, rEN.right, rEN.bottom, H_EN);

    /* PT-BR button */
    BOOL ptSel = (g_lang == 0), ptHot = (g_hot == H_PT);
    Cut2Box(g, rPT, SC(3), A(ptSel ? RGB(16, 26, 52) : RGB(8, 11, 20), 220),
        A(ptSel ? C_BLUE : (ptHot ? RGB(80, 110, 160) : C_FIELDB), 255), ptSel ? 1.5f : 1.0f);
    TextR(dc, g_fSmall, ptSel ? C_BLUE : (ptHot ? C_TEXT : C_SUB), rPT, L"PT-BR", DT_CENTER | DT_VCENTER);
    AddHit(rPT.left, rPT.top, rPT.right, rPT.bottom, H_PT);

    /* --- RIGHT SIDE: CLEAN INFORMATION CANVAS (X0..X1) --- */
    int gsel = g_sel, st = Status(gsel);

    if (g_navTab == 0) {
        /* TAB 0: CONFIGURAÇÕES - Clean image canvas without heavy panels */

        /* Title & Subtitle */
        Text(dc, g_fCard, RGB(255, 255, 255), SC(X0), SC(14), G[gsel].name);
        wchar_t sub[128]; _snwprintf(sub, 128, g_lang ? L"Native PC Port • Update 28.00" : L"Port nativo para PC • Atualização 28.00");
        Text(dc, g_fSmall, C_SUB, SC(X0 + 2), SC(42), sub);

        /* Status pill in Top Left ("Pronto para jogar" - Compact & Green) */
        COLORREF c = StatusColor(st);
        int pw = TextW(dc, g_fSmall, T(st)) + SC(26);
        RECT p = {rPT.right + boxGap, topY, rPT.right + boxGap + pw, botY};
        Cut2Box(g, p, SC(3), A(RGB(6, 10, 22), 220), A(c, 255), 1.0f);
        GpBrush *b = NULL; GdipCreateSolidFill(A(c, 255), &b);
        GdipFillRectangleI(g, b, p.left + SC(7), (p.top + p.bottom - SC(6)) / 2, SC(6), SC(6));
        GdipDeleteBrush(b);
        RECT t = {p.left + SC(17), p.top, p.right - SC(6), p.bottom};
        TextR(dc, g_fSmall, c, t, T(st), DT_LEFT | DT_VCENTER);

        /* Section 1: GAME */
        Section(dc, g, 70, T(S_GAME));
        RECT fr = {SC(X0), SC(94), SC(X1 - 125), SC(122)};
        Cut2Box(g, fr, SC(5), A(C_FIELD, 240), A(C_FIELDB, 230), 1.0f);
        MoveWindow(g_editGame, fr.left + SC(10), fr.top + SC(5), fr.right - fr.left - SC(18), SC(20), FALSE);

        RECT br = {SC(X1 - 115), SC(94), SC(X1), SC(122)};
        Button(dc, g, br, T(S_BROWSE), H_BROWSE);

        wchar_t pd[MAX_PATH], line[MAX_PATH + 64]; PortDir(gsel, pd);
        _snwprintf(line, MAX_PATH + 64, L"%s: %s   -   %s", T(S_PORT), pd, T(S_CLICKCHANGE));
        RECT pr = {SC(X0), SC(126), SC(X1), SC(142)};
        TextR(dc, g_fSmall, g_hot == H_PORT ? C_TEXT : C_SUB, pr, line, DT_LEFT | DT_VCENTER | DT_PATH_ELLIPSIS);
        AddHit(pr.left, pr.top, pr.left + TextW(dc, g_fSmall, line), pr.bottom, H_PORT);

        /* Section 2: VIDEO (2 columns) */
        Section(dc, g, 154, T(S_VIDEO));
        const Field *v = Vid(gsel);
        int cw = (X1 - X0 - 16) / 2;
        int col2 = X0 + cw + 16;
        /* Row 0 */
        Dropdown(dc, g, X0, 172, cw, &v[0], H_DROP + 0);
        Dropdown(dc, g, col2, 172, cw, &v[1], H_DROP + 1);
        /* Row 1 */
        Dropdown(dc, g, X0, 216, cw, &v[2], H_DROP + 2);
        Dropdown(dc, g, col2, 216, cw, &v[3], H_DROP + 3);
        /* Row 2 */
        Dropdown(dc, g, X0, 260, cw, &v[4], H_DROP + 4);
        Dropdown(dc, g, col2, 260, cw, &v[5], H_DROP + 5);
        /* Row 3 */
        Dropdown(dc, g, X0, 304, cw, &v[6], H_DROP + 6);
        Dropdown(dc, g, col2, 304, cw, &v[7], H_DROP + 7);
        /* Row 4: Motion Blur */
        Dropdown(dc, g, X0, 348, X1 - X0, &v[8], H_DROP + 8);

        /* Adendo FPS da Luta (harmonized dark card) */
        RECT rAdendo = {SC(X0), SC(394), SC(X1), SC(432)};
        Cut2Box(g, rAdendo, SC(6), A(RGB(8, 12, 24), 220), A(RGB(35, 48, 76), 200), 1.0f);
        RECT rLine1 = {rAdendo.left + SC(10), rAdendo.top + SC(2), rAdendo.right - SC(10), rAdendo.top + SC(19)};
        TextR(dc, g_fSmall, RGB(220, 230, 245), rLine1,
            g_lang ? L"ℹ 30 FPS (original): Works up to the selected FPS Limit above."
                   : L"ℹ 30 FPS (original): Opera até o Limite de FPS escolhido acima.",
            DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
        RECT rLine2 = {rAdendo.left + SC(10), rAdendo.top + SC(19), rAdendo.right - SC(10), rAdendo.bottom - SC(2)};
        TextR(dc, g_fSmall, C_GOLD, rLine2,
            g_lang ? L"⚠ 60+ FPS (patch): Unlocks 60+ FPS; character eyelids/mouths become detached."
                   : L"⚠ 60+ FPS (patch): Destrava a 60+ FPS; pálpebras e bocas ficam descoladas.",
            DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);

        /* Section 3: GAMEPLAY */
        Section(dc, g, 444, T(S_GAMEPLAY));
        const Field *gp = Gp(gsel);
        int gw1 = (cw * 62) / 100;
        int gw2 = cw - gw1 - 10;
        Dropdown(dc, g, X0, 464, gw1, &gp[0], H_DROP + 9);
        Dropdown(dc, g, X0 + gw1 + 10, 464, gw2, &gp[1], H_DROP + 10);
        Text(dc, g_fSmall, C_SUB, SC(col2), SC(464), T(S_PLAYER));
        RECT nr = {SC(col2), SC(480), SC(X1), SC(506)};
        Cut2Box(g, nr, SC(5), A(C_FIELD, 240), A(C_FIELDB, 230), 1.0f);
        MoveWindow(g_editPlayer, nr.left + SC(10), nr.top + SC(4), nr.right - nr.left - SC(18), SC(18), FALSE);

        /* Switches: 3 rows x 2 columns */
        int ns; const Field *sw = Sw(gsel, &ns);
        Switch(dc, g, X0, 516, &sw[0], H_SWITCH + 0);
        Switch(dc, g, col2, 516, &sw[1], H_SWITCH + 1);
        Switch(dc, g, X0, 544, &sw[2], H_SWITCH + 2);
        Switch(dc, g, col2, 544, &sw[3], H_SWITCH + 3);
        Switch(dc, g, X0, 572, &sw[4], H_SWITCH + 4);
        Switch(dc, g, col2, 572, &sw[5], H_SWITCH + 5);

        /* Action Buttons Row */
        RECT ab = {SC(X0), SC(616), SC(X0 + 68), SC(660)};
        Button(dc, g, ab, T(S_ADV), H_ADV);

        RECT lb = {SC(X0 + 76), SC(616), SC(X0 + 140), SC(660)};
        Button(dc, g, lb, T(S_LOGS), H_LOGS);

        RECT sb = {SC(X0 + 148), SC(616), SC(X0 + 278), SC(660)};
        Button(dc, g, sb, g_lang ? L"SAVE CONFIG" : L"SALVAR CONFIG.", H_SAVE);

        RECT scb = {SC(X0 + 286), SC(616), SC(X0 + 446), SC(660)};
        Button(dc, g, scb, g_lang ? L"ICONS & SHORTCUT" : L"ÍCONE & ATALHO", H_SHORTCUT);

        RECT pb = {SC(X0 + 456), SC(614), SC(X1), SC(662)};
        PlayButton(dc, g, pb);
    } else {
        /* TAB 1: EXTRAÇÕES PKG (NATIVE PS4 PKG TOOL) - Clean canvas */

        /* Title & Subtitle */
        Text(dc, g_fCard, RGB(255, 255, 255), SC(X0), SC(14), g_lang ? L"PKG Extractor & Tools" : L"Extrações PKG");
        Text(dc, g_fSmall, C_SUB, SC(X0 + 2), SC(42), g_lang ? L"Native PS4 PKG inspection, extraction and update installer" : L"Instalador nativo: extração conjunta de Jogo Base e Atualização 28.00");

        /* Validation status evaluation */
        BOOL baseValid = (g_haveBase && IsValidUfc4Base(&g_pkgBase));
        BOOL patchValid = (g_havePatch && IsValidUfc4Patch(&g_pkgPatch));
        BOOL idMismatch = (g_haveBase && g_havePatch && wcscmp(g_pkgBase.titleId, g_pkgPatch.titleId) != 0);
        BOOL hasInvalid = (g_haveBase && !baseValid) || (g_havePatch && !patchValid) || idMismatch;
        BOOL bothLoaded = (g_haveBase && g_havePatch);
        BOOL canExtract = (bothLoaded && baseValid && patchValid && !idMismatch);

        const wchar_t *pillText = canExtract ? (g_lang ? L"Base + Update 28.00 Ready" : L"Base + Update 28.00 Prontos")
                                : hasInvalid ? (g_lang ? L"Invalid PKG File(s)" : L"Arquivo PKG Inválido")
                                : bothLoaded ? (g_lang ? L"Check Version Requirements" : L"Verificar Versões")
                                : g_haveBase ? (g_lang ? L"Update 28.00 Missing" : L"Atualização 28.00 Ausente")
                                : g_havePatch ? (g_lang ? L"Base Game Missing" : L"Jogo Base Ausente")
                                : (g_lang ? L"No PKG Loaded" : L"Nenhum PKG Carregado");

        /* Status pill in Top Left corner - Compact */
        COLORREF cp = canExtract ? C_GOOD
                    : hasInvalid ? C_BAD
                    : bothLoaded ? C_WARN
                    : (g_haveBase || g_havePatch) ? C_WARN
                    : C_SUB;
        int pw = TextW(dc, g_fSmall, pillText) + SC(26);
        RECT p = {rPT.right + boxGap, topY, rPT.right + boxGap + pw, botY};
        Cut2Box(g, p, SC(3), A(RGB(6, 10, 22), 220), A(cp, 255), 1.0f);
        GpBrush *bp = NULL; GdipCreateSolidFill(A(cp, 255), &bp);
        GdipFillRectangleI(g, bp, p.left + SC(7), (p.top + p.bottom - SC(6)) / 2, SC(6), SC(6));
        GdipDeleteBrush(bp);
        RECT tp = {p.left + SC(17), p.top, p.right - SC(6), p.bottom};
        TextR(dc, g_fSmall, cp, tp, pillText, DT_LEFT | DT_VCENTER);

        /* Section 1: File Selection (Base Game + Update) - Redução da largura especificamente no lado esquerdo */
        int xPkgLeft = X0 + 40;
        SectionAt(dc, g, xPkgLeft, 70, g_lang ? L"PS4 PACKAGES (BASE GAME & UPDATE 28.00 MANDATORY)" : L"PACOTES PS4 (JOGO BASE E ATUALIZAÇÃO 28.00 OBRIGATÓRIOS)");

        /* Button to Browse All (multi-select) and Clear */
        RECT rBtnBrowseAll = {SC(xPkgLeft), SC(92), SC(xPkgLeft + 270), SC(120)};
        Button(dc, g, rBtnBrowseAll, g_lang ? L"SELECT PKG FILE(S)..." : L"SELECIONAR ARQUIVO(S) PKG...", H_PKG_BROWSE_ALL);

        if (g_haveBase || g_havePatch) {
            RECT rBtnClear = {SC(X1 - 80), SC(92), SC(X1), SC(120)};
            Button(dc, g, rBtnClear, g_lang ? L"CLEAR" : L"LIMPAR", H_PKG_CLEAR);
        }

        /* Slot 1: Jogo Base */
        int slot1Y = 126, slotH = 36;
        RECT rSlot1 = {SC(xPkgLeft), SC(slot1Y), SC(X1), SC(slot1Y + slotH)};
        BOOL slot1Invalid = (g_haveBase && !baseValid);
        COLORREF slot1Border = slot1Invalid ? RGB(240, 50, 60) : (g_haveBase ? C_BLUE : C_FIELDB);
        COLORREF slot1Bg = slot1Invalid ? RGB(65, 12, 18) : RGB(10, 14, 24);
        Cut2Box(g, rSlot1, SC(6), A(slot1Bg, 220), A(slot1Border, 220), slot1Invalid ? 1.5f : 1.0f);

        RECT rTagBase = {rSlot1.left + SC(6), rSlot1.top + SC(5), rSlot1.left + SC(100), rSlot1.bottom - SC(5)};
        COLORREF tagBaseBg = slot1Invalid ? RGB(140, 20, 28) : (g_haveBase ? RGB(16, 45, 30) : RGB(20, 26, 40));
        COLORREF tagBaseBorder = slot1Invalid ? RGB(240, 60, 70) : (g_haveBase ? C_GOOD : C_FIELDB);
        COLORREF tagBaseText = slot1Invalid ? RGB(255, 200, 200) : (g_haveBase ? C_GOOD : C_SUB);
        const wchar_t *tagBaseLabel = slot1Invalid ? (g_lang ? L"INVALID BASE" : L"BASE INVÁLIDA")
                                    : (g_haveBase ? (g_lang ? L"BASE GAME OK" : L"JOGO BASE OK")
                                                  : (g_lang ? L"BASE GAME" : L"JOGO BASE"));
        Cut2Box(g, rTagBase, SC(4), A(tagBaseBg, 255), A(tagBaseBorder, 255), 1.0f);
        TextR(dc, g_fSmall, tagBaseText, rTagBase, tagBaseLabel, DT_CENTER | DT_VCENTER);

        RECT rBaseTxt = {rTagBase.right + SC(10), rSlot1.top, rSlot1.right - SC(95), rSlot1.bottom};
        if (slot1Invalid) {
            wchar_t baseErr[300];
            _snwprintf(baseErr, 300, g_lang ? L"%s — INVALID! (ID: %s, v%s). Required: UFC 4 CUSA14209/CUSA14204"
                                            : L"%s — INVÁLIDO! (ID: %s, v%s). Requer: UFC 4 CUSA14209/CUSA14204",
                       g_pkgBase.fileName, g_pkgBase.titleId[0] ? g_pkgBase.titleId : L"?", g_pkgBase.appVer[0] ? g_pkgBase.appVer : L"?");
            TextR(dc, g_fSmall, RGB(255, 120, 130), rBaseTxt, baseErr, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
        } else if (g_haveBase) {
            wchar_t baseTxt[300];
            const wchar_t *rec = !wcscmp(g_pkgBase.titleId, L"CUSA14209") ? (g_lang ? L" [Rec.]" : L" [Rec.]") : L"";
            _snwprintf(baseTxt, 300, L"%s (%s%s • v%s • %s)", g_pkgBase.fileName, g_pkgBase.titleId, rec, g_pkgBase.appVer, g_pkgBase.sizeStr);
            TextR(dc, g_fText, C_TEXT, rBaseTxt, baseTxt, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
        } else {
            TextR(dc, g_fSmall, C_SUB, rBaseTxt,
                  g_lang ? L"No base package loaded — Required: UFC 4 CUSA14209 (Recommended) or CUSA14204"
                         : L"Nenhum pacote carregado — Necessário: UFC 4 CUSA14209 (Recomendado) ou CUSA14204", DT_LEFT | DT_VCENTER);
        }
        RECT rBtnPickBase = {rSlot1.right - SC(86), rSlot1.top + SC(4), rSlot1.right - SC(6), rSlot1.bottom - SC(4)};
        Button(dc, g, rBtnPickBase, g_lang ? L"BROWSE..." : L"PROCURAR...", H_PKG_BROWSE_BASE);

        /* Slot 2: Atualização (Patch) */
        int slot2Y = 170;
        RECT rSlot2 = {SC(xPkgLeft), SC(slot2Y), SC(X1), SC(slot2Y + slotH)};
        BOOL slot2Invalid = (g_havePatch && !patchValid);
        COLORREF slot2Border = slot2Invalid ? RGB(240, 50, 60) : (g_havePatch ? C_GOLD : C_FIELDB);
        COLORREF slot2Bg = slot2Invalid ? RGB(65, 12, 18) : RGB(10, 14, 24);
        Cut2Box(g, rSlot2, SC(6), A(slot2Bg, 220), A(slot2Border, 220), slot2Invalid ? 1.5f : 1.0f);

        RECT rTagPatch = {rSlot2.left + SC(6), rSlot2.top + SC(5), rSlot2.left + SC(110), rSlot2.bottom - SC(5)};
        COLORREF tagPatchBg = slot2Invalid ? RGB(140, 20, 28) : (g_havePatch ? RGB(45, 36, 12) : RGB(20, 26, 40));
        COLORREF tagPatchBorder = slot2Invalid ? RGB(240, 60, 70) : (g_havePatch ? C_GOLD : C_FIELDB);
        COLORREF tagPatchText = slot2Invalid ? RGB(255, 200, 200) : (g_havePatch ? C_GOLD : C_SUB);
        const wchar_t *tagPatchLabel = slot2Invalid ? (g_lang ? L"INVALID UPDATE" : L"UPDATE INVÁLIDO")
                                     : (g_havePatch ? (g_lang ? L"UPDATE 28.00 OK" : L"UPDATE 28.00 OK")
                                                    : (g_lang ? L"UPDATE 28.00" : L"ATUALIZAÇÃO 28.00"));
        Cut2Box(g, rTagPatch, SC(4), A(tagPatchBg, 255), A(tagPatchBorder, 255), 1.0f);
        TextR(dc, g_fSmall, tagPatchText, rTagPatch, tagPatchLabel, DT_CENTER | DT_VCENTER);

        RECT rPatchTxt = {rTagPatch.right + SC(10), rSlot2.top, rSlot2.right - SC(95), rSlot2.bottom};
        if (slot2Invalid) {
            wchar_t patchErr[300];
            _snwprintf(patchErr, 300, g_lang ? L"%s — INVALID! (v%s detected). Required: Update 28.00"
                                             : L"%s — INVÁLIDO! (v%s detectada). Requer: Update 28.00",
                       g_pkgPatch.fileName, g_pkgPatch.appVer[0] ? g_pkgPatch.appVer : L"?");
            TextR(dc, g_fSmall, RGB(255, 120, 130), rPatchTxt, patchErr, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
        } else if (g_havePatch) {
            wchar_t patchTxt[300];
            _snwprintf(patchTxt, 300, L"%s (%s • v%s • %s)", g_pkgPatch.fileName, g_pkgPatch.titleId, g_pkgPatch.appVer, g_pkgPatch.sizeStr);
            TextR(dc, g_fText, C_TEXT, rPatchTxt, patchTxt, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
        } else {
            TextR(dc, g_fSmall, C_SUB, rPatchTxt,
                  g_lang ? L"No update loaded — Required: Update 28.00 (matching Base Game ID)"
                         : L"Nenhum pacote carregado — Necessário: Atualização Update 28.00", DT_LEFT | DT_VCENTER);
        }
        RECT rBtnPickPatch = {rSlot2.right - SC(86), rSlot2.top + SC(4), rSlot2.right - SC(6), rSlot2.bottom - SC(4)};
        Button(dc, g, rBtnPickPatch, g_lang ? L"BROWSE..." : L"PROCURAR...", H_PKG_BROWSE_PATCH);

        /* Section 2: Details & Compatibility (Centralizado a pedido do usuário) */
        SectionCenter(dc, g, 224, g_lang ? L"GAME DETAILS & REQUIREMENTS VERIFICATION" : L"INFORMAÇÕES DO JOGO E VALIDAÇÃO DE REQUISITOS");

        if (hasInvalid) {
            /* CASE 1: RED ALERT BLOCK */
            RECT rAlert = {SC(X0), SC(246), SC(X1), SC(436)};
            Cut2Box(g, rAlert, SC(7), A(RGB(65, 12, 18), 245), A(RGB(240, 50, 60), 255), 1.5f);

            RECT rAlertBanner = {rAlert.left + SC(10), rAlert.top + SC(6), rAlert.right - SC(10), rAlert.top + SC(30)};
            Cut2Box(g, rAlertBanner, SC(4), A(RGB(160, 25, 35), 255), A(RGB(255, 120, 130), 255), 1.0f);
            TextR(dc, g_fBtn, RGB(255, 255, 255), rAlertBanner,
                  g_lang ? L"✖ INVALID FILE / INCOMPATIBLE PKG DETECTED" : L"✖ ARQUIVO INVÁLIDO / INCOMPATÍVEL DETECTADO!", DT_CENTER | DT_VCENTER);

            int ty = rAlert.top + SC(36);
            if (g_haveBase && !baseValid) {
                wchar_t bErr[300];
                _snwprintf(bErr, 300, g_lang ? L"• Base Game error: %s (detected ID: %s, v%s). Not UFC 4!"
                                             : L"• Erro no Jogo Base: %s (detectado ID: %s, v%s). Não é UFC 4!",
                           g_pkgBase.fileName, g_pkgBase.titleId[0] ? g_pkgBase.titleId : L"?", g_pkgBase.appVer[0] ? g_pkgBase.appVer : L"?");
                Text(dc, g_fSmall, RGB(255, 190, 195), rAlert.left + SC(14), ty, bErr);
                ty += SC(18);
            }
            if (g_havePatch && !patchValid) {
                wchar_t pErr[300];
                _snwprintf(pErr, 300, g_lang ? L"• Update error: %s (detected ID: %s, v%s). Required: Update 28.00!"
                                             : L"• Erro na Atualização: %s (detectado ID: %s, v%s). É obrigatório ter o Update 28.00!",
                           g_pkgPatch.fileName, g_pkgPatch.titleId[0] ? g_pkgPatch.titleId : L"?", g_pkgPatch.appVer[0] ? g_pkgPatch.appVer : L"?");
                Text(dc, g_fSmall, RGB(255, 190, 195), rAlert.left + SC(14), ty, pErr);
                ty += SC(18);
            }
            if (idMismatch) {
                wchar_t mErr[300];
                _snwprintf(mErr, 300, g_lang ? L"• Conflict: Base Game ID (%s) does not match Update ID (%s)!"
                                             : L"• Conflito: O Jogo Base (%s) e a Atualização (%s) possuem IDs diferentes!",
                           g_pkgBase.titleId, g_pkgPatch.titleId);
                Text(dc, g_fSmall, RGB(255, 190, 195), rAlert.left + SC(14), ty, mErr);
                ty += SC(18);
            }

            Text(dc, g_fSmall, RGB(255, 230, 120), rAlert.left + SC(14), ty + SC(4),
                 g_lang ? L"MANDATORY REQUIREMENTS FOR UFC 4 EXTRACTION:" : L"ARQUIVOS CORRETOS QUE VOCÊ PRECISA TER:");
            Text(dc, g_fSmall, RGB(255, 255, 255), rAlert.left + SC(14), ty + SC(22),
                 g_lang ? L"✔ Base Game: UFC 4 — CUSA14209 (Recommended) or CUSA14204 (v1.00)"
                        : L"✔ Jogo Base: UFC 4 — CUSA14209 (Recomendado) ou CUSA14204 (v1.00)");
            Text(dc, g_fSmall, RGB(255, 255, 255), rAlert.left + SC(14), ty + SC(40),
                 g_lang ? L"✔ Update: Specific Update 28.00 (must match base Title ID)"
                        : L"✔ Atualização: Update 28.00 obrigatório (mesmo CUSA do jogo base)");
            Text(dc, g_fSmall, RGB(255, 160, 160), rAlert.left + SC(14), ty + SC(60),
                 g_lang ? L"⚠ Both files are strictly mandatory. Running with only one will crash the port."
                        : L"⚠ É obrigatório possuir os DOIS arquivos juntos. Extrair apenas um causa crash.");

        } else if (!g_haveBase && !g_havePatch) {
            /* CASE 2: NO PKG LOADED YET - REQUIREMENTS GUIDE */
            RECT rGuide = {SC(X0), SC(246), SC(X1), SC(436)};
            Cut2Box(g, rGuide, SC(7), A(RGB(10, 16, 30), 240), A(RGB(35, 60, 110), 255), 1.0f);

            Text(dc, g_fBtn, RGB(255, 255, 255), rGuide.left + SC(16), rGuide.top + SC(10),
                 g_lang ? L"MANDATORY REQUIREMENTS FOR UFC 4 EXTRACTION" : L"ARQUIVOS OBRIGATÓRIOS PARA A EXTRAÇÃO DO UFC 4");

            Text(dc, g_fSmall, C_BLUE, rGuide.left + SC(16), rGuide.top + SC(34),
                 g_lang ? L"• BASE GAME: UFC 4 — CUSA14209 (Recommended) or CUSA14204 (Base v1.00)"
                        : L"• JOGO BASE: UFC 4 — CUSA14209 (Recomendado) ou CUSA14204 (Versão base 1.00)");

            Text(dc, g_fSmall, C_GOLD, rGuide.left + SC(16), rGuide.top + SC(56),
                 g_lang ? L"• UPDATE: Specific Update 28.00 mandatory (matching the Base Game Title ID)"
                        : L"• ATUALIZAÇÃO: Update 28.00 específico e obrigatório (mesmo CUSA do jogo base)");

            Text(dc, g_fSmall, C_WARN, rGuide.left + SC(16), rGuide.top + SC(78),
                 g_lang ? L"• MANDATORY RULE: Both Base Game and Update 28.00 are strictly required together."
                        : L"• REGRA OBRIGATÓRIA: É indispensável possuir os DOIS arquivos juntos para extrair.");
            Text(dc, g_fSmall, C_WARN, rGuide.left + SC(16), rGuide.top + SC(96),
                 g_lang ? L"  Extracting only the base or only the update will crash and close the port."
                        : L"  Rodar apenas a base ou apenas a atualização causa CRASH e fecha o port do jogo.");

            Text(dc, g_fSmall, C_SUB, rGuide.left + SC(16), rGuide.top + SC(118),
                 g_lang ? L"• FILE NAMES: The .pkg file names do NOT matter (you can rename them to anything)."
                        : L"• NOMENCLATURA: O nome dos arquivos .pkg não importa (pode renomear como quiser).");
            Text(dc, g_fSmall, C_SUB, rGuide.left + SC(16), rGuide.top + SC(136),
                 g_lang ? L"  FightPort 4 inspects the internal PARAM.SFO header to automatically detect each package."
                        : L"  O FightPort 4 analisa o cabeçalho e PARAM.SFO interno para identificar cada pacote.");

        } else if (!bothLoaded) {
            /* CASE 3: INCOMPLETE (ONE FILE LOADED, OTHER MISSING) */
            RECT rWarn = {SC(X0), SC(246), SC(X1), SC(436)};
            Cut2Box(g, rWarn, SC(7), A(RGB(38, 24, 10), 245), A(RGB(230, 145, 25), 255), 1.2f);

            RECT rWarnBanner = {rWarn.left + SC(10), rWarn.top + SC(6), rWarn.right - SC(10), rWarn.top + SC(30)};
            Cut2Box(g, rWarnBanner, SC(4), A(RGB(180, 100, 20), 255), A(RGB(255, 200, 80), 255), 1.0f);

            if (g_haveBase) {
                TextR(dc, g_fBtn, RGB(255, 255, 255), rWarnBanner,
                      g_lang ? L"⚠ ATTENTION: UPDATE 28.00 IS MANDATORY TO EXTRACT!"
                             : L"⚠ ATENÇÃO: É OBRIGATÓRIO CARREGAR A ATUALIZAÇÃO 28.00!", DT_CENTER | DT_VCENTER);

                wchar_t bOk[300];
                const wchar_t *rec = !wcscmp(g_pkgBase.titleId, L"CUSA14209") ? (g_lang ? L" (Recommended)" : L" (Recomendado)") : L"";
                _snwprintf(bOk, 300, g_lang ? L"✔ Base Game loaded: %s (%s%s, v%s) — Ready"
                                            : L"✔ Jogo Base carregado: %s (%s%s, v%s) — Pronto",
                           g_pkgBase.fileName, g_pkgBase.titleId, rec, g_pkgBase.appVer);
                Text(dc, g_fSmall, C_GOOD, rWarn.left + SC(14), rWarn.top + SC(40), bOk);

                Text(dc, g_fSmall, RGB(255, 220, 150), rWarn.left + SC(14), rWarn.top + SC(62),
                     g_lang ? L"✖ Missing Update: Please select the matching Update 28.00 PKG file."
                            : L"✖ Atualização ausente: Por favor, selecione o arquivo PKG do Update 28.00 correspondente.");

                Text(dc, g_fSmall, RGB(255, 180, 100), rWarn.left + SC(14), rWarn.top + SC(86),
                     g_lang ? L"⚠ CRITICAL: Running UFC 4 with only the Base Game will crash the PC port!"
                            : L"⚠ REGRA CRÍTICA: Executar o UFC 4 apenas com o Jogo Base CRASHA e fecha o port!");
                Text(dc, g_fSmall, RGB(255, 180, 100), rWarn.left + SC(14), rWarn.top + SC(104),
                     g_lang ? L"  Both files must be extracted together to work properly."
                            : L"  Os dois arquivos precisam ser extraídos juntos para que o port funcione perfeitamente.");
            } else {
                TextR(dc, g_fBtn, RGB(255, 255, 255), rWarnBanner,
                      g_lang ? L"⚠ ATTENTION: BASE GAME IS MANDATORY TO EXTRACT!"
                             : L"⚠ ATENÇÃO: É OBRIGATÓRIO CARREGAR O JOGO BASE!", DT_CENTER | DT_VCENTER);

                wchar_t pOk[300];
                _snwprintf(pOk, 300, g_lang ? L"✔ Update loaded: %s (%s, v%s) — Ready"
                                            : L"✔ Atualização carregada: %s (%s, v%s) — Pronta",
                           g_pkgPatch.fileName, g_pkgPatch.titleId, g_pkgPatch.appVer);
                Text(dc, g_fSmall, C_GOOD, rWarn.left + SC(14), rWarn.top + SC(40), pOk);

                Text(dc, g_fSmall, RGB(255, 220, 150), rWarn.left + SC(14), rWarn.top + SC(62),
                     g_lang ? L"✖ Missing Base Game: Please select UFC 4 CUSA14209 (Recommended) or CUSA14204."
                            : L"✖ Jogo Base ausente: Por favor, selecione o UFC 4 CUSA14209 (Recomendado) ou CUSA14204.");

                Text(dc, g_fSmall, RGB(255, 180, 100), rWarn.left + SC(14), rWarn.top + SC(86),
                     g_lang ? L"⚠ CRITICAL: Running UFC 4 with only the Update will crash the PC port!"
                            : L"⚠ REGRA CRÍTICA: Executar o UFC 4 apenas com a Atualização CRASHA e fecha o port!");
                Text(dc, g_fSmall, RGB(255, 180, 100), rWarn.left + SC(14), rWarn.top + SC(104),
                     g_lang ? L"  Both files must be extracted together to work properly."
                            : L"  Os dois arquivos precisam ser extraídos juntos para que o port funcione perfeitamente.");
            }

        } else {
            /* CASE 4: BOTH LOADED AND VALID - ALL READY! */
            PkgInfo *lead = &g_pkgBase;

            /* Capinha do CD encostada no canto direito */
            int iconSz = 104;
            int iconX = X1 - iconSz - 8;
            int iconY = 248;
            RECT rIcon = {SC(iconX), SC(iconY), SC(iconX + iconSz), SC(iconY + iconSz)};
            Cut2Box(g, rIcon, SC(6), A(C_FIELD, 255), A(C_FIELDB, 255), 1.0f);
            if (g_imgPkgIcon) {
                GdipDrawImageRectI(g, g_imgPkgIcon, rIcon.left + SC(4), rIcon.top + SC(4), SC(iconSz - 8), SC(iconSz - 8));
            } else {
                TextR(dc, g_fSmall, C_SUB, rIcon, L"icon0.png", DT_CENTER | DT_VCENTER);
            }

            /* Informações centralizadas */
            RECT rLine1 = {SC(X0), SC(256), SC(X1), SC(282)};
            TextR(dc, g_fCard, RGB(255, 255, 255), rLine1, lead->title[0] ? lead->title : lead->fileName, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);

            wchar_t l1[256];
            const wchar_t *rec = !wcscmp(lead->titleId, L"CUSA14209") ? (g_lang ? L" [Recommended]" : L" [Recomendado]") : L"";
            _snwprintf(l1, 256, L"Title ID: %s%s   |   Região: %s   |   FW: %s", lead->titleId, rec, lead->region, lead->sysVer[0] ? lead->sysVer : L"7.50");
            RECT rLine2 = {SC(X0), SC(286), SC(X1), SC(308)};
            TextR(dc, g_fSmall, C_BLUE, rLine2, l1, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);

            uint64_t totalBytes = g_pkgBase.fileSize + g_pkgPatch.fileSize;
            wchar_t totalStr[32]; PkgFormatSize(totalBytes, totalStr, 32);
            wchar_t lCombine[256];
            _snwprintf(lCombine, 256, g_lang ? L"Base v%s + Update v%s  (Tamanho: %s)"
                                             : L"Jogo Base v%s + Atualização v%s  (Tamanho: %s)",
                       g_pkgBase.appVer, g_pkgPatch.appVer, totalStr);
            RECT rLine3 = {SC(X0), SC(312), SC(X1), SC(334)};
            TextR(dc, g_fSmall, C_TEXT, rLine3, lCombine, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);

            /* Tarja no canto direito abaixo da imagem da capa do jogo */
            wchar_t bText[256];
            _snwprintf(bText, 256, g_lang ? L"✔ Both PKGs verified & compatible! Ready to extract."
                                          : L"✔ Ambos os arquivos verificados e compatíveis! Pronto para extrair.");
            int bw = TextW(dc, g_fSmall, bText) + SC(22);
            int badgeY = iconY + iconSz + 10;
            RECT rBadge = {SC(X1 - 8) - bw, SC(badgeY), SC(X1 - 8), SC(badgeY + 28)};
            Cut2Box(g, rBadge, SC(5), A(RGB(10, 32, 20), 220), A(C_GOOD, 255), 1.0f);
            TextR(dc, g_fSmall, C_GOOD, rBadge, bText, DT_CENTER | DT_VCENTER);
        }

        /* Section 3: Cleanup & Options (Totalmente alinhado à direita no canto direito) */
        SectionRight(dc, g, 452, g_lang ? L"CLEANUP & STORAGE OPTIONS" : L"OPÇÕES DE LIMPEZA E ARMAZENAMENTO");
        CheckboxRight(dc, g, 472, g_autoDeletePkg,
                 g_lang ? L"Delete original PKG file(s) automatically after extraction"
                        : L"Excluir arquivos PKG originais automaticamente após a extração", H_PKG_CHECK_DELETE);
        RECT rWarn = {SC(X0), SC(494), SC(X1), SC(514)};
        TextR(dc, g_fSmall, C_WARN, rWarn,
             g_lang ? L"Notice: PKG files will be deleted permanently (bypassing Recycle Bin) to free up space."
                    : L"Aviso: Os arquivos PKG serão excluídos definitivamente para liberar espaço em disco.",
             DT_RIGHT | DT_VCENTER);

        /* Section 4: Extraction Actions & Progress (Apenas o título encostado na direita) */
        SectionRight(dc, g, 524, g_lang ? L"EXTRACTION & INSTALLATION (BASE + UPDATE 28.00 MANDATORY)" : L"EXTRAÇÃO E INSTALAÇÃO (BASE + UPDATE 28.00 OBRIGATÓRIOS)");
        if (g_pkgExtracting) {
            RECT rProgBg = {SC(X0), SC(546), SC(X1 - 90), SC(580)};
            Cut2Box(g, rProgBg, SC(5), A(C_FIELD, 255), A(C_FIELDB, 255), 1.0f);

            int fillW = MulDiv(rProgBg.right - rProgBg.left, g_pkgExtractPct, 100);
            if (fillW > 0) {
                RECT rProgFill = {rProgBg.left, rProgBg.top, rProgBg.left + fillW, rProgBg.bottom};
                Cut2Box(g, rProgFill, SC(5), A(C_BLUE, 255), 0, 0);
            }

            wchar_t pctStr[32];
            _snwprintf(pctStr, 32, L"%d%%", g_pkgExtractPct);
            TextR(dc, g_fBtn, RGB(255, 255, 255), rProgBg, pctStr, DT_CENTER | DT_VCENTER);

            RECT rCancel = {SC(X1 - 80), SC(546), SC(X1), SC(580)};
            Button(dc, g, rCancel, g_lang ? L"CANCEL" : L"CANCELAR", H_PKG_CANCEL);

            RECT rStat = {SC(X0), SC(588), SC(X1), SC(612)};
            TextR(dc, g_fText, C_BLUE, rStat, g_pkgExtractStatus, DT_RIGHT | DT_VCENTER | DT_END_ELLIPSIS);
        } else {
            RECT rBtnExt = {SC(X0), SC(544), SC(X1), SC(590)};
            BOOL hotExt = (g_hot == H_PKG_EXTRACT_GAME && canExtract);

            if (canExtract) {
                COLORREF c1 = hotExt ? RGB(255, 225, 45) : RGB(242, 205, 25);
                COLORREF c2 = hotExt ? RGB(235, 195, 20) : RGB(218, 175, 10);
                POINT ptA = {rBtnExt.left, rBtnExt.top}, ptB = {rBtnExt.right, rBtnExt.bottom};
                GpBrush *lb = NULL; GdipCreateLineBrushI(&ptA, &ptB, A(c1, 255), A(c2, 255), 0, &lb);
                GpPath *pathExt = Cut2Path(rBtnExt.left, rBtnExt.top, rBtnExt.right - rBtnExt.left - 1, rBtnExt.bottom - rBtnExt.top - 1, SC(8));
                GdipFillPath(g, lb, pathExt); GdipDeleteBrush(lb); GdipDeletePath(pathExt);
            } else {
                Cut2Box(g, rBtnExt, SC(8), A(RGB(14, 18, 30), 255), A(C_FIELDB, 255), 1.0f);
            }
            SetTextCharacterExtra(dc, SC(1));
            const wchar_t *extLabel = canExtract ? (g_lang ? L"EXTRACT BASE + UPDATE 28.00" : L"EXTRAIR BASE + UPDATE 28.00")
                                    : hasInvalid ? (g_lang ? L"INVALID PKG — BLOCKED" : L"ARQUIVO INVÁLIDO — BLOQUEADO")
                                    : (!g_haveBase && !g_havePatch) ? (g_lang ? L"LOAD BASE + UPDATE 28.00" : L"SELECIONE BASE + UPDATE 28.00")
                                    : (!g_havePatch) ? (g_lang ? L"MANDATORY: LOAD UPDATE 28.00" : L"OBRIGATÓRIO: CARREGAR UPDATE 28.00")
                                    : (g_lang ? L"MANDATORY: LOAD BASE GAME" : L"OBRIGATÓRIO: CARREGAR JOGO BASE");
            TextR(dc, g_fBtn, canExtract ? RGB(10, 14, 24) : (hasInvalid ? RGB(240, 80, 80) : C_SUB), rBtnExt, extLabel, DT_CENTER | DT_VCENTER);
            SetTextCharacterExtra(dc, 0);
            if (canExtract) AddHit(rBtnExt.left, rBtnExt.top, rBtnExt.right, rBtnExt.bottom, H_PKG_EXTRACT_GAME);

            int auxW = (X1 - X0 - 12) / 2;
            RECT rBtnCust = {SC(X0), SC(598), SC(X0 + auxW), SC(634)};
            Button(dc, g, rBtnCust, g_lang ? L"CHOOSE FOLDER & EXTRACT..." : L"ESCOLHER PASTA E EXTRAIR...", canExtract ? H_PKG_EXTRACT_CUSTOM : -1);

            RECT rBtnSfo = {SC(X0 + auxW + 12), SC(598), SC(X1), SC(634)};
            Button(dc, g, rBtnSfo, g_lang ? L"INSPECT SFO" : L"INSPECIONAR SFO", (g_haveBase || g_havePatch) ? H_PKG_VIEW_SFO : -1);
        }
    }

    /* Status Bar at Bottom */
    {
        RECT sb = {SC(X0), SC(674), SC(X1), SC(702)};
        Cut2Box(g, sb, SC(5), A(RGB(6, 10, 22), 200), A(RGB(32, 44, 72), 160), 1.0f);
        COLORREF mc = C_SUB;
        const wchar_t *textToShow = NULL;

        if (g_msg[0]) {
            textToShow = g_msg;
            mc = g_msgColor;
        } else {
            textToShow = T(st);
            mc = StatusColor(st);
        }

        GpBrush *b = NULL; GdipCreateSolidFill(A(mc, 255), &b);
        GdipFillEllipseI(g, b, sb.left + SC(12), (sb.top + sb.bottom) / 2 - SC(4), SC(8), SC(8));
        GdipDeleteBrush(b);
        RECT mr = {sb.left + SC(28), sb.top, sb.right - SC(12), sb.bottom};
        TextR(dc, g_fSmall, mc, mr, textToShow, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
    }

    if (g_iconPickerOpen) {
        g_nhits = 0;
        DrawIconPickerModal(dc, g, rc.right, rc.bottom);
    }
    if (g_themedModalActive) {
        g_nhits = 0;
        DrawThemedModal(dc, g, rc.right, rc.bottom);
    }

    GdipDeleteGraphics(g);
    BitBlt(wdc, 0, 0, rc.right, rc.bottom, dc, 0, 0, SRCCOPY);
    SelectObject(dc, ob); DeleteObject(bmp); DeleteDC(dc);
}

/* ---------------------------------------------------------------- selection and actions */
static void LoadEdits(void)
{
    wchar_t b[MAX_PATH];
    GetStr(g_sel, L"game", b, MAX_PATH, L""); SetWindowTextW(g_editGame, b);
    GetStr(g_sel, L"player", b, MAX_PATH, L"Player 1"); SetWindowTextW(g_editPlayer, b);
}
static void SaveEdits(void)
{
    wchar_t b[MAX_PATH], cur[MAX_PATH];
    GetWindowTextW(g_editGame, b, MAX_PATH); StripSlash(b); GetStr(g_sel, L"game", cur, MAX_PATH, L"");
    if (wcscmp(b, cur)) PutStr(g_sel, L"game", b);
    GetWindowTextW(g_editPlayer, b, 64); GetStr(g_sel, L"player", cur, MAX_PATH, L"");
    if (wcscmp(b, cur)) PutStr(g_sel, L"player", b);
}
static void Select(int g)
{
    if (g < 0 || g >= NGAMES || g == g_sel) return;
    SaveEdits(); g_sel = g; g_kvGame = -1; g_msg[0] = 0;
    WritePrivateProfileStringW(L"FightPort4", L"selected_game", G[g].id, g_pIni);
    LoadEdits();
    InvalidateRect(g_wnd, NULL, FALSE);
}

static BOOL PickFolder(wchar_t *path, const wchar_t *title)
{
    IFileOpenDialog *d = NULL; BOOL ok = FALSE;
    if (FAILED(CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, &IID_IFileOpenDialog, (void **)&d))) return FALSE;
    DWORD o; d->lpVtbl->GetOptions(d, &o); d->lpVtbl->SetOptions(d, o | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM); d->lpVtbl->SetTitle(d, title);
    if (path[0]) { IShellItem *si; if (SUCCEEDED(SHCreateItemFromParsingName(path, NULL, &IID_IShellItem, (void **)&si))) { d->lpVtbl->SetFolder(d, si); si->lpVtbl->Release(si); } }
    if (SUCCEEDED(d->lpVtbl->Show(d, g_wnd))) {
        IShellItem *it; if (SUCCEEDED(d->lpVtbl->GetResult(d, &it))) {
            PWSTR p; if (SUCCEEDED(it->lpVtbl->GetDisplayName(it, SIGDN_FILESYSPATH, &p))) { wcsncpy(path, p, MAX_PATH); CoTaskMemFree(p); ok = TRUE; }
            it->lpVtbl->Release(it);
        }
    }
    d->lpVtbl->Release(d); return ok;
}

static BOOL PickFile(wchar_t *path, const wchar_t *title, const COMDLG_FILTERSPEC *filter, int nFilters)
{
    IFileOpenDialog *d = NULL; BOOL ok = FALSE;
    if (FAILED(CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, &IID_IFileOpenDialog, (void **)&d))) return FALSE;
    DWORD o; d->lpVtbl->GetOptions(d, &o); d->lpVtbl->SetOptions(d, o | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);
    d->lpVtbl->SetTitle(d, title);
    if (filter && nFilters > 0) d->lpVtbl->SetFileTypes(d, nFilters, filter);
    if (path[0]) { IShellItem *si; if (SUCCEEDED(SHCreateItemFromParsingName(path, NULL, &IID_IShellItem, (void **)&si))) { d->lpVtbl->SetFolder(d, si); si->lpVtbl->Release(si); } }
    if (SUCCEEDED(d->lpVtbl->Show(d, g_wnd))) {
        IShellItem *it; if (SUCCEEDED(d->lpVtbl->GetResult(d, &it))) {
            PWSTR p; if (SUCCEEDED(it->lpVtbl->GetDisplayName(it, SIGDN_FILESYSPATH, &p))) { wcsncpy(path, p, MAX_PATH); CoTaskMemFree(p); ok = TRUE; }
            it->lpVtbl->Release(it);
        }
    }
    d->lpVtbl->Release(d); return ok;
}

#define WM_APP_PKG_DONE (WM_APP + 10)

typedef struct {
    wchar_t basePath[MAX_PATH];
    wchar_t patchPath[MAX_PATH];
    wchar_t destDir[MAX_PATH];
    BOOL hasBase;
    BOOL hasPatch;
    BOOL toActiveGame;
    int gameIdx;
} PkgWorkerArgs;

static void LoadPkgPath(const wchar_t *path)
{
    if (!path || !path[0]) return;
    PkgInfo info;
    if (!PkgReadInfo(path, &info)) {
        Msg(C_BAD, g_lang ? L"Failed to read PKG: %s" : L"Falha ao ler o PKG: %s", PathFindFileNameW(path));
        return;
    }

    /* Identify category purely from PKG / PARAM.SFO internal content - file name does NOT matter */
    BOOL isPatch = (!wcscmp(info.category, L"gp"));
    if (!isPatch && wcscmp(info.category, L"gd") != 0) {
        if (wcsstr(info.appVer, L"28.") || _wtof(info.appVer) > 1.05f) {
            isPatch = TRUE;
        }
    }

    if (isPatch) {
        g_pkgPatch = info;
        g_havePatch = TRUE;
        BOOL valid = IsValidUfc4Patch(&info);
        if (valid) {
            Msg(C_GOOD, g_lang ? L"Update loaded: %s (v%s, %s) — OK" : L"Atualização carregada: %s (v%s, %s) — OK",
                info.title[0] ? info.title : info.fileName, info.appVer, info.sizeStr);
        } else {
            Msg(C_BAD, g_lang ? L"INVALID UPDATE PKG: v%s detected. Required: Update 28.00 (CUSA14209 or CUSA14204)!"
                              : L"ATUALIZAÇÃO INVÁLIDA: versão v%s detectada. Necessário: Update 28.00 (CUSA14209 ou CUSA14204)!",
                info.appVer[0] ? info.appVer : L"?");
        }
    } else {
        g_pkgBase = info;
        g_haveBase = TRUE;
        if (g_imgPkgIcon) {
            GdipDisposeImage(g_imgPkgIcon);
            g_imgPkgIcon = NULL;
        }
        void *buf = NULL; size_t sz = 0;
        if (PkgExtractEntryToMemory(path, PKG_ENTRY_ICON0_PNG, &buf, &sz)) {
            IStream *s = SHCreateMemStream((const BYTE *)buf, (DWORD)sz);
            if (s) {
                GdipCreateBitmapFromStream(s, &g_imgPkgIcon);
                s->lpVtbl->Release(s);
            }
            PkgFreeMemory(buf);
        }
        BOOL valid = IsValidUfc4Base(&info);
        if (valid) {
            const wchar_t *rec = !wcscmp(info.titleId, L"CUSA14209") ? (g_lang ? L" [Recommended]" : L" [Recomendado]") : L"";
            Msg(C_GOOD, g_lang ? L"Base Game loaded: %s (%s%s, v%s) — OK" : L"Jogo Base carregado: %s (%s%s, v%s) — OK",
                info.title[0] ? info.title : info.fileName, info.titleId, rec, info.appVer);
        } else {
            Msg(C_BAD, g_lang ? L"INVALID BASE GAME PKG: Title ID %s detected. Required: UFC 4 (CUSA14209 or CUSA14204)!"
                              : L"JOGO BASE INVÁLIDO: Title ID %s detectado. Necessário: UFC 4 (CUSA14209 ou CUSA14204)!",
                info.titleId[0] ? info.titleId : L"?");
        }
    }

    if (!g_imgPkgIcon) {
        void *buf = NULL; size_t sz = 0;
        if (PkgExtractEntryToMemory(path, PKG_ENTRY_ICON0_PNG, &buf, &sz)) {
            IStream *s = SHCreateMemStream((const BYTE *)buf, (DWORD)sz);
            if (s) {
                GdipCreateBitmapFromStream(s, &g_imgPkgIcon);
                s->lpVtbl->Release(s);
            }
            PkgFreeMemory(buf);
        }
    }
}

static void PickFiles(HWND parent, BOOL forceBase, BOOL forcePatch)
{
    IFileOpenDialog *d = NULL;
    if (FAILED(CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, &IID_IFileOpenDialog, (void **)&d))) return;

    DWORD o; d->lpVtbl->GetOptions(d, &o);
    d->lpVtbl->SetOptions(d, o | FOS_ALLOWMULTISELECT | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);

    const wchar_t *title = forceBase ? (g_lang ? L"Select PS4 Base Game PKG" : L"Selecionar PKG do Jogo Base")
                         : forcePatch ? (g_lang ? L"Select PS4 Update PKG" : L"Selecionar PKG da Atualização")
                         : (g_lang ? L"Select PS4 PKG File(s)" : L"Selecionar Arquivo(s) PKG de PS4");
    d->lpVtbl->SetTitle(d, title);

    COMDLG_FILTERSPEC filter[] = { { L"PS4 PKG Files (*.pkg)", L"*.pkg" }, { L"All Files (*.*)", L"*.*" } };
    d->lpVtbl->SetFileTypes(d, 2, filter);

    if (SUCCEEDED(d->lpVtbl->Show(d, parent))) {
        IShellItemArray *items = NULL;
        if (SUCCEEDED(d->lpVtbl->GetResults(d, &items))) {
            DWORD count = 0;
            items->lpVtbl->GetCount(items, &count);
            for (DWORD i = 0; i < count; i++) {
                IShellItem *si = NULL;
                if (SUCCEEDED(items->lpVtbl->GetItemAt(items, i, &si))) {
                    PWSTR p = NULL;
                    if (SUCCEEDED(si->lpVtbl->GetDisplayName(si, SIGDN_FILESYSPATH, &p))) {
                        if (forceBase) {
                            PkgInfo info;
                            if (PkgReadInfo(p, &info)) {
                                if (!wcscmp(info.category, L"gp") && !g_havePatch) {
                                    g_pkgPatch = info;
                                    g_havePatch = TRUE;
                                    Msg(C_ORANGE, g_lang ? L"Update file detected and placed into Update slot. Please also select the Base Game."
                                                         : L"Arquivo de Atualização detectado e alocado no slot de Atualização. Selecione também o Jogo Base.");
                                } else {
                                    g_pkgBase = info;
                                    g_haveBase = TRUE;
                                    if (g_imgPkgIcon) { GdipDisposeImage(g_imgPkgIcon); g_imgPkgIcon = NULL; }
                                    void *buf = NULL; size_t sz = 0;
                                    if (PkgExtractEntryToMemory(p, PKG_ENTRY_ICON0_PNG, &buf, &sz)) {
                                        IStream *s = SHCreateMemStream((const BYTE *)buf, (DWORD)sz);
                                        if (s) { GdipCreateBitmapFromStream(s, &g_imgPkgIcon); s->lpVtbl->Release(s); }
                                        PkgFreeMemory(buf);
                                    }
                                    BOOL valid = IsValidUfc4Base(&info);
                                    if (valid) {
                                        const wchar_t *rec = !wcscmp(info.titleId, L"CUSA14209") ? (g_lang ? L" [Recommended]" : L" [Recomendado]") : L"";
                                        Msg(C_GOOD, g_lang ? L"Base Game loaded: %s (%s%s, v%s) — OK" : L"Jogo Base carregado: %s (%s%s, v%s) — OK",
                                            info.title[0] ? info.title : info.fileName, info.titleId, rec, info.appVer);
                                    } else {
                                        Msg(C_BAD, g_lang ? L"INVALID BASE GAME: Title ID %s detected. Required: UFC 4 CUSA14209 (Recommended) or CUSA14204!"
                                                          : L"JOGO BASE INVÁLIDO: Title ID %s detectado. Necessário: UFC 4 CUSA14209 (Recomendado) ou CUSA14204!",
                                            info.titleId[0] ? info.titleId : L"?");
                                    }
                                }
                            }
                        } else if (forcePatch) {
                            PkgInfo info;
                            if (PkgReadInfo(p, &info)) {
                                if (!wcscmp(info.category, L"gd") && !g_haveBase) {
                                    g_pkgBase = info;
                                    g_haveBase = TRUE;
                                    if (g_imgPkgIcon) { GdipDisposeImage(g_imgPkgIcon); g_imgPkgIcon = NULL; }
                                    void *buf = NULL; size_t sz = 0;
                                    if (PkgExtractEntryToMemory(p, PKG_ENTRY_ICON0_PNG, &buf, &sz)) {
                                        IStream *s = SHCreateMemStream((const BYTE *)buf, (DWORD)sz);
                                        if (s) { GdipCreateBitmapFromStream(s, &g_imgPkgIcon); s->lpVtbl->Release(s); }
                                        PkgFreeMemory(buf);
                                    }
                                    Msg(C_ORANGE, g_lang ? L"Base Game detected and placed into Base slot. Please also select Update 28.00."
                                                         : L"Jogo Base detectado e alocado no slot de Jogo Base. Selecione também a Atualização 28.00.");
                                } else {
                                    g_pkgPatch = info;
                                    g_havePatch = TRUE;
                                    BOOL valid = IsValidUfc4Patch(&info);
                                    if (valid) {
                                        Msg(C_GOOD, g_lang ? L"Update loaded: %s (v%s, %s) — OK" : L"Atualização carregada: %s (v%s, %s) — OK",
                                            info.title[0] ? info.title : info.fileName, info.appVer, info.sizeStr);
                                    } else {
                                        Msg(C_BAD, g_lang ? L"INVALID UPDATE: v%s detected. Required: Update 28.00 (CUSA14209 or CUSA14204)!"
                                                          : L"ATUALIZAÇÃO INVÁLIDA: versão v%s detectada. Necessário: Update 28.00 (CUSA14209 ou CUSA14204)!",
                                            info.appVer[0] ? info.appVer : L"?");
                                    }
                                }
                            }
                        } else {
                            LoadPkgPath(p);
                        }
                        CoTaskMemFree(p);
                    }
                    si->lpVtbl->Release(si);
                }
            }
            items->lpVtbl->Release(items);
        }
    }
    d->lpVtbl->Release(d);
}

static void OnPkgProgress(int pct, uint64_t bytesDone, uint64_t bytesTotal, const wchar_t *statusText, void *userParam)
{
    (void)bytesDone; (void)bytesTotal; (void)userParam;
    g_pkgExtractPct = pct;
    if (statusText) wcsncpy(g_pkgExtractStatus, statusText, 255);
    if (g_wnd) InvalidateRect(g_wnd, NULL, FALSE);
}

static int ShowThemedModal(HWND parent,
                           const wchar_t *title,
                           const wchar_t *text,
                           ThemedModalButtons buttons,
                           ThemedModalIcon icon,
                           const wchar_t *customBtn1,
                           const wchar_t *customBtn2)
{
    g_themedModalActive = TRUE;
    wcsncpy(g_themedModalTitle, title ? title : L"", 127);
    g_themedModalTitle[127] = 0;
    wcsncpy(g_themedModalText, text ? text : L"", 2047);
    g_themedModalText[2047] = 0;
    g_themedModalButtons = buttons;
    g_themedModalIcon = icon;
    g_themedModalBtn1Label = customBtn1;
    g_themedModalBtn2Label = customBtn2;
    g_themedModalResult = (buttons == THEMED_MODAL_YESNO) ? IDNO : IDCANCEL;

    /* Hide edit controls during modal */
    ShowWindow(g_editGame, SW_HIDE);
    ShowWindow(g_editPlayer, SW_HIDE);

    InvalidateRect(parent, NULL, FALSE);
    UpdateWindow(parent);

    MSG msg;
    while (g_themedModalActive && GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_navTab == 0 && !g_iconPickerOpen) {
        ShowWindow(g_editGame, SW_SHOW);
        ShowWindow(g_editPlayer, SW_SHOW);
    }
    InvalidateRect(parent, NULL, FALSE);
    return g_themedModalResult;
}

static BOOL PromptAndPrepareExtractionFolder(HWND parent, wchar_t *outDest, size_t maxLen)
{
    /* 1. Themed modal explaining automatic folder creation with the exact requested name */
    int promptRes = ShowThemedModal(parent,
        g_lang ? L"UFC 4 Installation Location" : L"Local de Instalação do UFC 4",
        g_lang ? L"Select the location (drive or folder) where you want to install UFC 4.\n\n"
                 L"AUTOMATIC FOLDER CREATION:\n"
                 L"A folder named \"EA_Sports_UFC_4\" will be created AUTOMATICALLY inside your chosen location.\n"
                 L"You only need to select the parent folder or drive (e.g. D:\\GAMES\\PlayStation_4 or C:\\Games).\n\n"
                 L"Do NOT manually create another folder with this name to prevent duplicate nested folders.\n\n"
                 L"Click \"CONTINUE\" to choose the destination folder in the file explorer."
               : L"Escolha o local (disco ou pasta) onde deseja instalar o UFC 4.\n\n"
                 L"CRIAÇÃO AUTOMÁTICA DE PASTA:\n"
                 L"A pasta \"EA_Sports_UFC_4\" será criada AUTOMATICAMENTE dentro do local selecionado.\n"
                 L"Você só precisa selecionar a pasta pai ou disco (ex: D:\\GAMES\\PlayStation_4 ou C:\\Jogos).\n\n"
                 L"NÃO crie uma pasta com este nome manualmente para evitar pastas duplicadas.\n\n"
                 L"Clique em \"CONTINUAR\" para selecionar a pasta de destino no explorador de arquivos.",
        THEMED_MODAL_OKCANCEL, THEMED_ICON_INFO,
        g_lang ? L"CANCEL" : L"CANCELAR",
        g_lang ? L"CONTINUE..." : L"CONTINUAR...");

    if (promptRes != IDOK) return FALSE;

    /* 2. Pick folder */
    wchar_t picked[MAX_PATH] = L"";
    if (!PickFolder(picked, g_lang ? L"Select Location for UFC 4" : L"Selecione a Pasta Onde Instalar o UFC 4")) {
        return FALSE;
    }
    StripSlash(picked);
    if (!picked[0]) return FALSE;

    /* 3. Determine final target path with exact name EA SPORTS™ UFC® 4 */
    const wchar_t *tail = PathFindFileNameW(picked);
    wchar_t finalDir[MAX_PATH];
    if (tail && (_wcsicmp(tail, UFC4_FOLDER_NAME) == 0 ||
                 _wcsicmp(tail, L"EA_Sports_UFC_4") == 0 ||
                 _wcsicmp(tail, L"EA SPORTS UFC 4") == 0 ||
                 _wcsicmp(tail, L"UFC_4") == 0 ||
                 _wcsicmp(tail, L"UFC4") == 0)) {
        wcsncpy(finalDir, picked, MAX_PATH - 1);
        finalDir[MAX_PATH - 1] = 0;
    } else {
        Join(finalDir, picked, UFC4_FOLDER_NAME);
    }

    /* 4. Themed confirmation modal with exact path */
    wchar_t confirmMsg[800];
    _snwprintf(confirmMsg, 800,
        g_lang ? L"The game will be extracted and installed into:\n\n"
                 L"   %s\n\n"
                 L"• Both the Base Game and Update 28.00 will be extracted into this directory.\n"
                 L"• After extraction, FightPort 4 will automatically configure this game folder.\n\n"
                 L"Do you want to begin extraction now?"
               : L"O jogo será extraído e instalado em:\n\n"
                 L"   %s\n\n"
                 L"• O Jogo Base e a Atualização 28.00 serão descompactados diretamente nesta pasta.\n"
                 L"• Após a extração, o FightPort 4 atualizará automaticamente o caminho do jogo.\n\n"
                 L"Deseja iniciar a extração agora?",
        finalDir);

    int conf = ShowThemedModal(parent,
        g_lang ? L"Confirm Installation Destination" : L"Confirmar Destino da Instalação",
        confirmMsg,
        THEMED_MODAL_YESNO, THEMED_ICON_QUESTION,
        g_lang ? L"NO, CANCEL" : L"NÃO, CANCELAR",
        g_lang ? L"YES, EXTRACT NOW" : L"SIM, INICIAR EXTRAÇÃO");

    if (conf != IDYES) return FALSE;

    SHCreateDirectoryExW(NULL, finalDir, NULL);
    wcsncpy(outDest, finalDir, maxLen - 1);
    outDest[maxLen - 1] = 0;
    return TRUE;
}

static void OnPkgProgressBase(int pct, uint64_t bytesDone, uint64_t bytesTotal, const wchar_t *statusText, void *userParam)
{
    (void)bytesDone; (void)bytesTotal; (void)userParam;
    g_pkgExtractPct = pct / 2;
    if (statusText) _snwprintf(g_pkgExtractStatus, 255, L"[1/2 Base] %s", statusText);
    if (g_wnd) InvalidateRect(g_wnd, NULL, FALSE);
}

static void OnPkgProgressPatch(int pct, uint64_t bytesDone, uint64_t bytesTotal, const wchar_t *statusText, void *userParam)
{
    (void)bytesDone; (void)bytesTotal; (void)userParam;
    g_pkgExtractPct = 50 + (pct / 2);
    if (statusText) _snwprintf(g_pkgExtractStatus, 255, L"[2/2 Atualização] %s", statusText);
    if (g_wnd) InvalidateRect(g_wnd, NULL, FALSE);
}

static DWORD WINAPI PkgExtractThread(LPVOID p)
{
    PkgWorkerArgs a = *(PkgWorkerArgs *)p;
    free(p);

    g_pkgExtracting = TRUE;
    g_pkgExtractCancel = FALSE;
    g_pkgExtractPct = 0;
    wcsncpy(g_lastExtractedDir, a.destDir, MAX_PATH);
    wcsncpy(g_pkgExtractStatus, g_lang ? L"Preparing extraction..." : L"Iniciando extração...", 255);

    BOOL ok = TRUE;

    /* Enforce that BOTH files are strictly mandatory */
    if (!a.hasBase || !a.hasPatch) {
        wcsncpy(g_pkgExtractStatus, g_lang ? L"Extraction blocked: Both Base Game and Update 28.00 are strictly mandatory!"
                                           : L"Extração bloqueada: É obrigatório possuir o Jogo Base e o Update 28.00!", 255);
        g_pkgExtracting = FALSE;
        PostMessageW(g_wnd, WM_APP_PKG_DONE, (WPARAM)FALSE, (LPARAM)a.toActiveGame);
        return 0;
    }

    wcsncpy(g_pkgExtractStatus, g_lang ? L"[1/2] Extracting Base Game..." : L"[1/2] Extraindo Jogo Base...", 255);
    ok = PkgExtractFull(a.basePath, a.destDir, &g_pkgExtractCancel, OnPkgProgressBase, NULL);

    if (ok && !g_pkgExtractCancel) {
        wcsncpy(g_pkgExtractStatus, g_lang ? L"[2/2] Extracting and applying Update..." : L"[2/2] Extraindo e aplicando Atualização...", 255);
        ok = PkgExtractFull(a.patchPath, a.destDir, &g_pkgExtractCancel, OnPkgProgressPatch, NULL);
    }

    g_pkgExtracting = FALSE;
    if (g_pkgExtractCancel) ok = FALSE;

    PostMessageW(g_wnd, WM_APP_PKG_DONE, (WPARAM)ok, (LPARAM)a.toActiveGame);
    return 0;
}

static void Browse(void)
{
    wchar_t p[MAX_PATH], t[160]; GetWindowTextW(g_editGame, p, MAX_PATH); _snwprintf(t, 160, T(S_PICKGAME), G[g_sel].name);
    if (!PickFolder(p, t)) return;
    StripSlash(p); SetWindowTextW(g_editGame, p); PutStr(g_sel, L"game", p);
    Msg(GameOk(p) ? C_GOOD : C_BAD, L"%s", GameOk(p) ? T(S_READY) : T(S_PICK));
}
static void ChangePort(void)
{
    wchar_t p[MAX_PATH], t[200]; PortDir(g_sel, p); _snwprintf(t, 200, T(S_PICKPORT), G[g_sel].name, G[g_sel].exe);
    if (!PickFolder(p, t)) return;
    wchar_t e[MAX_PATH]; Join(e, p, G[g_sel].exe);
    if (!FileExists(e)) { Msg(C_BAD, T(S_NOTFOUND), G[g_sel].exe); return; }
    wchar_t k[32]; _snwprintf(k, 32, L"port_%s", G[g_sel].id); WritePrivateProfileStringW(L"FightPort4", k, p, g_pIni);
    ReadVersion(g_sel); g_kvGame = -1; LoadEdits(); InvalidateRect(g_wnd, NULL, FALSE);
}

static DWORD WINAPI WaitThread(LPVOID p)
{
    HANDLE h = (HANDLE)p; WaitForSingleObject(h, INFINITE);
    DWORD code = 0; GetExitCodeProcess(h, &code); PostMessageW(g_wnd, WM_APP_EXITED, code, 0); return 0;
}

/* ---------------------------------------------------------------- UFC 1-4: freeze catcher
   The SacredFire engine prints "GPU wait stuck" when its GPU command thread has waited more than 2 s for the graphics
   card. The catcher then saves the call stack of every thread of sacredfire-game.exe beside the log
   (<log>.stacks.txt): which thread waits for which, the information a freeze needs to be fixed. */
typedef struct { uint64_t base, size; wchar_t name[64]; } ModInfo;
typedef HRESULT (WINAPI *GetThreadDescriptionFn)(HANDLE, PWSTR *);
static volatile LONG g_watchStop, g_watchSession;
static wchar_t g_frozenFile[MAX_PATH];

static void Out(FILE *f, const wchar_t *fmt, ...)
{
    wchar_t w[1200]; va_list ap; va_start(ap, fmt); _vsnwprintf(w, 1200, fmt, ap); va_end(ap); w[1199] = 0;
    char u[3600]; int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, u, sizeof(u), NULL, NULL); if (n > 1) fwrite(u, 1, (size_t)n - 1, f);
}
static int ModCmp(const void *a, const void *b) { uint64_t x = ((const ModInfo *)a)->base, y = ((const ModInfo *)b)->base; return x < y ? -1 : x > y; }
static int ModAt(const ModInfo *m, int n, uint64_t a)
{
    int lo = 0, hi = n - 1, found = -1;
    while (lo <= hi) { int mid = (lo + hi) / 2; if (m[mid].base <= a) { found = mid; lo = mid + 1; } else hi = mid - 1; }
    return found >= 0 && a < m[found].base + m[found].size ? found : -1;
}
static void Where(wchar_t *out, int cap, uint64_t a, const ModInfo *m, int n)
{
    int i = ModAt(m, n, a);
    if (i >= 0) _snwprintf(out, cap, L"%s+0x%llx", m[i].name, (unsigned long long)(a - m[i].base));
    else _snwprintf(out, cap, L"0x%llx", (unsigned long long)a);
    out[cap - 1] = 0;
}
/* the 8 bytes before a value found on a stack end with a call instruction: it is a return address */
static BOOL CallBefore(const unsigned char *b)
{
#define BB(k) b[8 - (k)]
    if (BB(5) == 0xE8) return TRUE;                                                   /* call rel32 */
    if (BB(6) == 0xFF && BB(5) == 0x15) return TRUE;                                  /* call [rip+disp32] */
    if (BB(2) == 0xFF && (BB(1) & 0xF8) == 0xD0) return TRUE;                         /* call reg */
    if ((BB(3) & 0xF0) == 0x40 && BB(2) == 0xFF && (BB(1) & 0xF8) == 0xD0) return TRUE; /* call r8-r15 */
    if (BB(2) == 0xFF && (BB(1) & 0xF8) == 0x10 && (BB(1) & 7) != 4 && (BB(1) & 7) != 5) return TRUE;   /* call [reg] */
    if (BB(3) == 0xFF && (BB(2) & 0xF8) == 0x50) return TRUE;                         /* call [reg+disp8] */
    if (BB(4) == 0xFF && BB(3) == 0x54) return TRUE;                                  /* call [reg+index+disp8] */
    if (BB(6) == 0xFF && (BB(5) & 0xF8) == 0x90) return TRUE;                         /* call [reg+disp32] */
    if ((BB(4) & 0xF0) == 0x40 && BB(3) == 0xFF && (BB(2) & 0xF8) == 0x50) return TRUE;
    if ((BB(7) & 0xF0) == 0x40 && BB(6) == 0xFF && (BB(5) & 0xF8) == 0x90) return TRUE;
    if ((BB(3) & 0xF0) == 0x40 && BB(2) == 0xFF && (BB(1) & 0xF8) == 0x10) return TRUE;
    return FALSE;
#undef BB
}
static DWORD FindProcess(const wchar_t *exe)
{
    DWORD pid = 0; HANDLE sn = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); PROCESSENTRY32W pe = {sizeof(pe)};
    if (sn == INVALID_HANDLE_VALUE) return 0;
    if (Process32FirstW(sn, &pe)) do { if (!_wcsicmp(pe.szExeFile, exe)) pid = pe.th32ProcessID; } while (Process32NextW(sn, &pe));
    CloseHandle(sn); return pid;
}
static BOOL SaveStacks(DWORD pid, const wchar_t *path, const char *why)
{
    HANDLE hp = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hp) return FALSE;
    static ModInfo mods[1024]; int nm = 0;
    HANDLE ms = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (ms != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W me = {sizeof(me)};
        if (Module32FirstW(ms, &me)) do {
            if (nm < 1024) { mods[nm].base = (uint64_t)(uintptr_t)me.modBaseAddr; mods[nm].size = me.modBaseSize; wcsncpy(mods[nm].name, me.szModule, 63); mods[nm].name[63] = 0; nm++; }
        } while (Module32NextW(ms, &me));
        CloseHandle(ms);
    }
    qsort(mods, nm, sizeof(ModInfo), ModCmp);
    FILE *f = _wfopen(path, L"wb"); if (!f) { CloseHandle(hp); return FALSE; }
    SYSTEMTIME t; GetLocalTime(&t);
    Out(f, L"FightPort 4 %s freeze catcher, %04d-%02d-%02d %02d:%02d:%02d: the threads of sacredfire-game.exe (pid %lu)\r\n", APP_VER, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, pid);
    { wchar_t w[300]; MultiByteToWideChar(CP_UTF8, 0, why, -1, w, 300); w[299] = 0; Out(f, L"Trigger: %s\r\n\r\nModules:\r\n", w); }
    for (int i = 0; i < nm; i++) Out(f, L"  %016llx %08llx %s\r\n", (unsigned long long)mods[i].base, (unsigned long long)mods[i].size, mods[i].name);
    GetThreadDescriptionFn gtd = (GetThreadDescriptionFn)(void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetThreadDescription");
    static unsigned char stack[128 * 1024];
    HANDLE ts = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); THREADENTRY32 te = {sizeof(te)}; int nt = 0;
    if (ts != INVALID_HANDLE_VALUE && Thread32First(ts, &te)) do {
        if (te.th32OwnerProcessID != pid) continue;
        nt++;
        HANDLE ht = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_LIMITED_INFORMATION, FALSE, te.th32ThreadID);
        if (!ht) { Out(f, L"\r\nthread %lu: cannot open it (error %lu)\r\n", te.th32ThreadID, GetLastError()); continue; }
        wchar_t tname[128] = L"";
        if (gtd) { PWSTR d = NULL; if (SUCCEEDED(gtd(ht, &d)) && d) { wcsncpy(tname, d, 127); tname[127] = 0; LocalFree(d); } }
        CONTEXT ctx; memset(&ctx, 0, sizeof(ctx)); ctx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
        SIZE_T have = 0; BOOL got = FALSE;
        if (SuspendThread(ht) != (DWORD)-1) {
            got = GetThreadContext(ht, &ctx);
            if (got) while (have < sizeof(stack)) {   /* the stack from rsp up, page by page, until an unreadable one */
                uint64_t at = ctx.Rsp + have; SIZE_T want = 4096 - (SIZE_T)(at & 4095), n = 0;
                if (want > sizeof(stack) - have) want = sizeof(stack) - have;
                if (!ReadProcessMemory(hp, (LPCVOID)(uintptr_t)at, stack + have, want, &n) || !n) break;
                have += n;
            }
            ResumeThread(ht);
        }
        wchar_t w1[200];
        Out(f, L"\r\nthread %lu \"%s\" base priority %ld\r\n", te.th32ThreadID, tname, te.tpBasePri);
        if (!got) { Out(f, L"  (no context)\r\n"); CloseHandle(ht); continue; }
        Where(w1, 200, ctx.Rip, mods, nm);
        Out(f, L"  rip %s   rsp %llx rbp %llx rax %llx rcx %llx rdx %llx r8 %llx\r\n", w1, (unsigned long long)ctx.Rsp, (unsigned long long)ctx.Rbp,
            (unsigned long long)ctx.Rax, (unsigned long long)ctx.Rcx, (unsigned long long)ctx.Rdx, (unsigned long long)ctx.R8);
        uint64_t bp = ctx.Rbp;   /* frame pointer chain (the engine keeps frame pointers) */
        for (int d = 0; d < 40 && bp; d++) {
            uint64_t fr[2]; SIZE_T n = 0;
            if (!ReadProcessMemory(hp, (LPCVOID)(uintptr_t)bp, fr, sizeof(fr), &n) || n != sizeof(fr) || fr[0] <= bp) break;
            Where(w1, 200, fr[1], mods, nm); Out(f, L"  fp#%-2d %s\r\n", d, w1); bp = fr[0];
        }
        int shown = 0;           /* return addresses found on the stack */
        for (SIZE_T o = 0; o + 8 <= have && shown < 72; o += 8) {
            uint64_t v; memcpy(&v, stack + o, 8);
            int mi = ModAt(mods, nm, v); if (mi < 0 || v < mods[mi].base + 8) continue;
            unsigned char code[8]; SIZE_T n = 0;
            if (!ReadProcessMemory(hp, (LPCVOID)(uintptr_t)(v - 8), code, 8, &n) || n != 8 || !CallBefore(code)) continue;
            Out(f, L"  [rsp+%05llx] %s+0x%llx\r\n", (unsigned long long)o, mods[mi].name, (unsigned long long)(v - mods[mi].base)); shown++;
        }
        CloseHandle(ht);
    } while (Thread32Next(ts, &te));
    if (ts != INVALID_HANDLE_VALUE) CloseHandle(ts);
    Out(f, L"\r\n%d threads\r\n", nt);
    fclose(f); CloseHandle(hp); return nt > 0;
}
typedef struct { int game; LONG session; FILETIME start; } WatchArgs;
static DWORD WINAPI FreezeWatch(LPVOID p)
{
    WatchArgs a = *(WatchArgs *)p; free(p);
    wchar_t la[MAX_PATH] = L"", dir[MAX_PATH], logs[MAX_PATH], log[MAX_PATH] = L"";
    GetEnvironmentVariableW(L"LOCALAPPDATA", la, MAX_PATH); Join(dir, la, G[a.game].data); Join(logs, dir, L"logs");
    ULARGE_INTEGER s0; s0.LowPart = a.start.dwLowDateTime; s0.HighPart = a.start.dwHighDateTime; s0.QuadPart -= 30ull * 10000000ull;
    LONGLONG offset = 0; DWORD stuckAt = 0; char carry[64] = ""; char why[200] = "";
    static char buf[1 << 20];
    while (!g_watchStop && g_watchSession == a.session) {
        Sleep(1000);
        if (!log[0]) {   /* the log this launch writes: the newest one created after the start */
            wchar_t pat[MAX_PATH]; Join(pat, logs, L"*.log"); WIN32_FIND_DATAW fd; HANDLE h = FindFirstFileW(pat, &fd);
            ULARGE_INTEGER best = {0};
            if (h != INVALID_HANDLE_VALUE) {
                do { ULARGE_INTEGER c; c.LowPart = fd.ftCreationTime.dwLowDateTime; c.HighPart = fd.ftCreationTime.dwHighDateTime;
                     if (c.QuadPart >= s0.QuadPart && c.QuadPart > best.QuadPart) { best = c; Join(log, logs, fd.cFileName); } } while (FindNextFileW(h, &fd));
                FindClose(h);
            }
            if (!log[0]) continue;
        }
        HANDLE f = CreateFileW(log, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
        if (f == INVALID_HANDLE_VALUE) continue;
        for (;;) {
            LARGE_INTEGER at; at.QuadPart = offset; SetFilePointerEx(f, at, NULL, FILE_BEGIN);
            size_t keep = strlen(carry); memcpy(buf, carry, keep);
            DWORD got = 0; if (!ReadFile(f, buf + keep, (DWORD)(sizeof(buf) - keep - 1), &got, NULL) || !got) break;
            offset += got; buf[keep + got] = 0;
            for (size_t i = keep; i < keep + got; i++) if (!buf[i]) buf[i] = ' ';   /* stray NUL bytes would end the search early */
            char *hit = strstr(buf, "GPU wait stuck");
            if (hit && !stuckAt) { stuckAt = GetTickCount(); char *e = strchr(hit, '\n'); size_t n = e ? (size_t)(e - hit) : strlen(hit); if (n > 190) n = 190; memcpy(why, hit, n); why[n] = 0; if (n && why[n - 1] == '\r') why[n - 1] = 0; }
            size_t tot = keep + got, c = tot > 40 ? 40 : tot; memcpy(carry, buf + tot - c, c); carry[c] = 0;
        }
        CloseHandle(f);
        if (stuckAt && GetTickCount() - stuckAt >= 5000) {   /* 5 s later: the engine's own wait report is in the log too */
            DWORD pid = FindProcess(L"sacredfire-game.exe");
            wchar_t out[MAX_PATH]; wcscpy(out, log); wchar_t *dot = wcsrchr(out, L'.'); if (dot) *dot = 0; wcsncat(out, L".stacks.txt", MAX_PATH - wcslen(out) - 1);
            BOOL ok = pid && SaveStacks(pid, out, why);
            if (ok) { wcscpy(g_frozenFile, out); PostMessageW(g_wnd, WM_APP_FROZEN, (WPARAM)a.game, 0); }
            break;
        }
    }
    return 0;
}
/* ---------------------------------------------------------------- this PC (for the automatic settings of UFC 1-4)
   The video card through Vulkan (name, vendor, memory, and whether the CPU can see all of its memory: Resizable BAR,
   which Intel Arc cards need to run well), processor threads, RAM and the monitor. */
typedef struct { wchar_t name[256]; uint32_t vendor, type; uint64_t vram, bar; } PcGpu;
static PcGpu g_gpu; static BOOL g_haveGpu, g_oldCpu; static int g_threads, g_ramGb, g_monW, g_monH, g_monHz;
/* an old processor, the same rule as UFC5.exe: no AVX2 (Intel Core 3rd generation and older, AMD FX/Phenom/older APUs,
   most Pentium/Celeron) or a Ryzen 1000-3000 (Zen, Zen+, Zen 2) */
static BOOL CpuIsOld(void)
{
    unsigned a, b, c, d, r[12]; char s[49];
    if (!__get_cpuid_count(7, 0, &a, &b, &c, &d) || !(b & (1u << 5))) return TRUE;
    for (int i = 0; i < 3; i++) __get_cpuid(0x80000002 + i, &r[i * 4], &r[i * 4 + 1], &r[i * 4 + 2], &r[i * 4 + 3]);
    memcpy(s, r, 48); s[48] = 0;
    const char *p = StrStrIA(s, "Ryzen");
    if (!p || StrStrIA(s, "Threadripper")) return FALSE;
    for (p += 5; *p; ) {   /* the model number: the first word of 4+ digits ("Ryzen 5 3600", "Ryzen 7 PRO 2700U") */
        while (*p == ' ') p++;
        int dig = 0; while (p[dig] >= '0' && p[dig] <= '9') dig++;
        if (dig >= 4) return p[0] >= '1' && p[0] <= '3';
        while (*p && *p != ' ') p++;
    }
    return FALSE;
}
typedef void *(WINAPI *PFN_GIPA_)(void *, const char *);
typedef struct { int sType; const void *pNext; const char *app; uint32_t appVer; const char *eng; uint32_t engVer; uint32_t api; } VkAppInfo__;
typedef struct { int sType; const void *pNext; uint32_t flags; const VkAppInfo__ *app; uint32_t nl; const char *const *l; uint32_t ne; const char *const *e; } VkInstInfo__;
static void DetectPc(void)
{
    SYSTEM_INFO si; GetSystemInfo(&si); g_threads = (int)si.dwNumberOfProcessors; g_oldCpu = CpuIsOld();
    MEMORYSTATUSEX ms = {sizeof(ms)}; GlobalMemoryStatusEx(&ms); g_ramGb = (int)((ms.ullTotalPhys + (512ull << 20)) >> 30);
    DEVMODEW dm = {.dmSize = sizeof(dm)}; EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm);
    g_monW = dm.dmPelsWidth ? (int)dm.dmPelsWidth : 1920; g_monH = dm.dmPelsHeight ? (int)dm.dmPelsHeight : 1080; g_monHz = dm.dmDisplayFrequency > 1 ? (int)dm.dmDisplayFrequency : 60;
    g_haveGpu = FALSE;
    HMODULE vk = LoadLibraryW(L"vulkan-1.dll"); if (!vk) return;
    PFN_GIPA_ gipa = (PFN_GIPA_)(void *)GetProcAddress(vk, "vkGetInstanceProcAddr");
    int (WINAPI *create)(const VkInstInfo__ *, const void *, void **) = gipa ? (int (WINAPI *)(const VkInstInfo__ *, const void *, void **))gipa(NULL, "vkCreateInstance") : NULL;
    VkAppInfo__ ai = {0, NULL, "FightPort4", 1, "none", 1, (1u << 22) | (1u << 12)}; VkInstInfo__ ii = {1, NULL, 0, &ai, 0, NULL, 0, NULL}; void *inst = NULL;
    if (!create || create(&ii, NULL, &inst) != 0 || !inst) { FreeLibrary(vk); return; }
    int (WINAPI *enumDev)(void *, uint32_t *, void **) = (int (WINAPI *)(void *, uint32_t *, void **))gipa(inst, "vkEnumeratePhysicalDevices");
    void (WINAPI *props)(void *, void *) = (void (WINAPI *)(void *, void *))gipa(inst, "vkGetPhysicalDeviceProperties");
    void (WINAPI *mem)(void *, void *) = (void (WINAPI *)(void *, void *))gipa(inst, "vkGetPhysicalDeviceMemoryProperties");
    void (WINAPI *destroy)(void *, const void *) = (void (WINAPI *)(void *, const void *))gipa(inst, "vkDestroyInstance");
    uint32_t n = 8; void *devs[8]; uint64_t bestScore = 0;
    if (enumDev && props && mem && enumDev(inst, &n, devs) >= 0) {
        static unsigned char buf[8192];
        for (uint32_t i = 0; i < n && i < 8; i++) {
            PcGpu c = {0}; memset(buf, 0, sizeof(buf)); props(devs[i], buf);
            c.vendor = *(uint32_t *)(buf + 8); c.type = *(uint32_t *)(buf + 16); MultiByteToWideChar(CP_UTF8, 0, (char *)buf + 20, -1, c.name, 256);
            memset(buf, 0, sizeof(buf)); mem(devs[i], buf);
            uint32_t nt = *(uint32_t *)buf, nh = *(uint32_t *)(buf + 260);
            for (uint32_t h = 0; h < nh && h < 16; h++) { uint64_t size = *(uint64_t *)(buf + 264 + h * 16); uint32_t fl = *(uint32_t *)(buf + 272 + h * 16); if ((fl & 1) && size > c.vram) c.vram = size; }
            for (uint32_t t = 0; t < nt && t < 32; t++) {   /* device-local + host-visible memory: its heap is the BAR window */
                uint32_t fl = *(uint32_t *)(buf + 4 + t * 8), hi = *(uint32_t *)(buf + 8 + t * 8);
                if ((fl & 3) == 3 && hi < 16) { uint64_t hs = *(uint64_t *)(buf + 264 + hi * 16); if (hs > c.bar) c.bar = hs; }
            }
            uint64_t score = c.vram + (c.type == 2 ? (1ull << 40) : 0);
            if (!g_haveGpu || score > bestScore) { g_gpu = c; bestScore = score; g_haveGpu = TRUE; }
        }
    }
    if (destroy) destroy(inst, NULL);
    FreeLibrary(vk);
}
static BOOL GpuIsArc(void) { return g_haveGpu && g_gpu.vendor == 0x8086 && g_gpu.type == 2; }
/* Resizable BAR on: the CPU-visible window of video memory is (nearly) all of it, not the old 256 MB */
static BOOL GpuHasRebar(void) { return g_haveGpu && g_gpu.bar > (1ull << 30); }
static int PcTier(void)   /* 2 strong, 1 weak, 0 very weak: the same rule as UFC5.exe */
{
    int vmb = g_haveGpu ? (int)(g_gpu.vram >> 20) : 0;
    if (g_threads <= 4 || (vmb && vmb < 3800) || g_ramGb < 12) return 0;
    if (g_threads < 12 || (vmb && vmb < 7500) || g_ramGb < 16 || g_oldCpu) return 1;   /* an old core is weak, whatever its thread count */
    return 2;
}
static void PcSig(wchar_t *out, int n)
{
    _snwprintf(out, n, L"%d|%d|%s|%llu|%dx%d@%d", g_threads, g_ramGb, g_haveGpu ? g_gpu.name : L"-", g_haveGpu ? (unsigned long long)(g_gpu.vram >> 20) : 0ull, g_monW, g_monH, g_monHz);
    out[n - 1] = 0;
}
/* Parsec mode (UFC 1-4): the host runs the game and a friend joins through Parsec, whose virtual pad becomes Player 2
   (one player per controller is the engine's default, bbport.ini local_multiplayer). What lowers the delay on the
   host: one frame queued instead of several (low_latency, the launcher's BB_FRAMES_AHEAD=1), a steady 60 FPS for
   Parsec's 60 FPS stream and its encoder, and Mailbox (no tearing in the stream; Intel Arc keeps its VSync). The
   settings it replaces are kept in fightport4.ini and come back when it is turned off. */
static void UltraApply(int g)
{
    PutInt(g, L"resolution", 0);
    PutInt(g, L"neo", 0);
    PutInt(g, L"fps_limit", 30);
    PutInt(g, L"upscaler", 1);
}
static void ParsecApply(int g)
{
    PutInt(g, L"low_latency", 1);
    if (GetInt(g, L"fps_limit", 60) != 60) PutInt(g, L"fps_limit", 60);
    if (!GpuIsArc()) PutInt(g, L"present_mode", 0);
}
static void ParsecSet(int g, BOOL on)
{
    static const wchar_t *const keys[3] = {L"low_latency", L"fps_limit", L"present_mode"};
    static const int defs[3] = {0, 60, 0};
    wchar_t pk[48], b[16];
    for (int i = 0; i < 3; i++) {
        _snwprintf(pk, 48, L"parsec_%s_%s", G[g].id, keys[i]);
        if (on) { _snwprintf(b, 16, L"%d", GetInt(g, keys[i], defs[i])); WritePrivateProfileStringW(L"FightPort4", pk, b, g_pIni); }
        else { int v = GetPrivateProfileIntW(L"FightPort4", pk, -1, g_pIni); PutInt(g, keys[i], v >= 0 ? v : defs[i]); WritePrivateProfileStringW(L"FightPort4", pk, NULL, g_pIni); }
    }
    _snwprintf(pk, 48, L"parsec_%s", G[g].id); WritePrivateProfileStringW(L"FightPort4", pk, on ? L"1" : NULL, g_pIni);
    PutInt(g, L"parsec", on ? 1 : 0);
    if (on) ParsecApply(g);
    else if (GetInt(g, L"ultra", 0)) UltraApply(g);
}
static void UltraSet(int g, BOOL on)
{
    static const wchar_t *const keys[4] = {L"resolution", L"neo", L"fps_limit", L"upscaler"};
    static const int defs[4] = {1, 0, 60, 0};
    wchar_t pk[48], b[16];
    for (int i = 0; i < 4; i++) {
        _snwprintf(pk, 48, L"ultra_%s_%s", G[g].id, keys[i]);
        if (on) { _snwprintf(b, 16, L"%d", GetInt(g, keys[i], defs[i])); WritePrivateProfileStringW(L"FightPort4", pk, b, g_pIni); }
        else { int v = GetPrivateProfileIntW(L"FightPort4", pk, -1, g_pIni); PutInt(g, keys[i], v >= 0 ? v : defs[i]); WritePrivateProfileStringW(L"FightPort4", pk, NULL, g_pIni); }
    }
    _snwprintf(pk, 48, L"ultra_%s", G[g].id); WritePrivateProfileStringW(L"FightPort4", pk, on ? L"1" : NULL, g_pIni);
    PutInt(g, L"ultra", on ? 1 : 0);
    if (on) UltraApply(g);
    else if (GetInt(g, L"parsec", 0)) ParsecApply(g);
}
/* the best settings of UFC 1-4 for this PC (what the ports' notes say holds on these games):
   - FPS: UFC 1, 2 and 4 count game time per frame, so 60 (UFC 1 fights stay at its own 30); UFC 3 runs fights at any
     rate, so a strong PC on a 120/144 Hz monitor gets 120/144;
   - resolution: 1080p; very weak PCs 720p with FSR 1.0 to the monitor and sharpening, "ultra" below 4 threads/3 GB;
   - PS4 Pro mode only on strong NVIDIA/AMD cards (UFC 2-4; UFC 1 has none);
   - Intel Arc: VSync (Fifo), the present mode its driver always has, and never PS4 Pro mode. */
static void AutoConfig4(int g)
{
    int t = PcTier(); BOOL arc = GpuIsArc(), nvamd = g_haveGpu && (g_gpu.vendor == 0x10DE || g_gpu.vendor == 0x1002);
    int vmb = g_haveGpu ? (int)(g_gpu.vram >> 20) : 0;
    int fps = 60;
    if (g == 2 && t == 2 && g_monHz >= 120) fps = g_monHz >= 144 ? 144 : 120;
    PutInt(g, L"fullscreen", 1); PutInt(g, L"skip_movies", 1); PutInt(g, L"low_latency", 0);
    PutInt(g, L"frame_stats", 0);   /* the 5-second statistics line in the log lined up with 80-190 ms hitches (UFC 1 frame logs, 2026-10-20) */
    PutInt(g, L"fps_limit", fps); PutInt(g, L"present_mode", arc ? 1 : 0);
    if (t == 0) {
        PutInt(g, L"resolution", 0); PutInt(g, L"display", g_monH >= 1080 ? 1 : 0); PutInt(g, L"upscaler", 1); PutInt(g, L"sharpness", 2);
        PutInt(g, L"ultra", (g_threads <= 4 || (vmb && vmb < 3000)) ? 1 : 0); PutInt(g, L"neo", 0);
    } else {
        PutInt(g, L"resolution", 1); PutInt(g, L"display", 0); PutInt(g, L"upscaler", 0); PutInt(g, L"sharpness", 0); PutInt(g, L"ultra", 0);
        PutInt(g, L"neo", (g != 0 && t == 2 && nvamd && vmb >= 7500) ? 1 : 0);
    }
    /* weak and very weak PCs: the crowd off (community patch, +5 to 10 FPS in the fight) */
    if (PatchState(g, L"crowd", -1) >= 0) PatchState(g, L"crowd", t < 2 ? 1 : 0);
    if (GetInt(g, L"parsec", 0)) ParsecApply(g);   /* Parsec mode stays on through a new auto setup */
    if (GetInt(g, L"ultra", 0)) UltraApply(g);
}
/* a port set up before on an old processor gets, once, only what eases the CPU in a fight:
   PS4 Pro mode off, at most 60 FPS, and the crowd off */
static void OldCpu4(int g)
{
    PutInt(g, L"neo", 0); PutInt(g, L"low_latency", 0);
    if (GetInt(g, L"fps_limit", 60) > 60) PutInt(g, L"fps_limit", 60);
    if (PatchState(g, L"crowd", -1) >= 0) PatchState(g, L"crowd", 1);
    if (GetInt(g, L"parsec", 0)) ParsecApply(g);
}
/* on the first start, and again when the PC changes (other card, more RAM, other monitor):
   UFC 4 gets the settings above. A port that was already set up by hand keeps its settings. */
static int AutoConfigAll(BOOL force)
{
    wchar_t sig[400]; PcSig(sig, 400); int done = 0;
    for (int g = 0; g < NGAMES; g++) {
        if (!PortOk(g)) continue;
        wchar_t k[32], old[400]; _snwprintf(k, 32, L"auto_%s", G[g].id);
        GetPrivateProfileStringW(L"FightPort4", k, L"", old, 400, g_pIni);
        wchar_t sf[MAX_PATH]; SfPath(g, sf);
        BOOL had = FileExists(sf);
        wchar_t ko[32]; _snwprintf(ko, 32, L"oldcpu_%s", G[g].id);
        BOOL oldDone = GetPrivateProfileIntW(L"FightPort4", ko, 0, g_pIni);
        if (g_oldCpu) WritePrivateProfileStringW(L"FightPort4", ko, L"1", g_pIni);
        if (!force && !wcscmp(old, sig)) {
            if (g_oldCpu && !oldDone && had) { g_kvGame = -1; OldCpu4(g); done++; }
            continue;
        }
        if (force || !had || old[0]) { g_kvGame = -1; AutoConfig4(g); done++; }
        else if (g_oldCpu && !oldDone) { g_kvGame = -1; OldCpu4(g); done++; }
        WritePrivateProfileStringW(L"FightPort4", k, sig, g_pIni);
    }
    g_kvGame = -1;
    return done;
}
static void PcSummary(wchar_t *out, int n)
{
    static const wchar_t *const TN[2][3] = {{L"PC muito fraco", L"PC fraco", L"PC forte"}, {L"very weak PC", L"weak PC", L"strong PC"}};
    _snwprintf(out, n, L"%s: %s (%llu GB), %d threads, %d GB RAM, %dx%d %d Hz%s%s", TN[g_lang][PcTier()], g_haveGpu ? g_gpu.name : L"GPU?",
               g_haveGpu ? (unsigned long long)((g_gpu.vram + (512ull << 20)) >> 30) : 0ull, g_threads, g_ramGb, g_monW, g_monH, g_monHz,
               GpuIsArc() ? (g_lang ? L", Intel Arc mode" : L", modo Intel Arc") : L"",
               g_oldCpu ? (g_lang ? L", old processor (light mode)" : L", processador antigo (modo leve)") : L"");
    out[n - 1] = 0;
}

static int g_autoDone4;

/* the PS4 ports run the game's own x86-64 code, built for the PS4's Jaguar CPU: without these instructions the engine
   stops at once ("this CPU lacks BMI1, MOVBE, LZCNT", photo of a player's log, 2026-10-19). Fills the missing names. */
static BOOL CpuLacksPs4(wchar_t *out, int n)
{
    unsigned a, b, c, d; BOOL avx = FALSE, movbe = FALSE, bmi1 = FALSE, lzcnt = FALSE;
    if (__get_cpuid(1, &a, &b, &c, &d)) { movbe = (c >> 22) & 1; avx = (c >> 28) & 1; }
    if (__get_cpuid_count(7, 0, &a, &b, &c, &d)) bmi1 = (b >> 3) & 1;
    if (__get_cpuid(0x80000001, &a, &b, &c, &d)) lzcnt = (c >> 5) & 1;
    out[0] = 0;
    if (!avx) wcscat(out, L"AVX, ");
    if (!bmi1) wcscat(out, L"BMI1, ");
    if (!movbe) wcscat(out, L"MOVBE, ");
    if (!lzcnt) wcscat(out, L"LZCNT, ");
    size_t l = wcslen(out); if (l >= 2) out[l - 2] = 0;
    (void)n; return out[0] != 0;
}
/* the face fix: the community "Skinning Method 0" patch skins the fighters on the CPU, which ends the mesh
   stretching that pulls the mouth and the face out of shape. The 60+ FPS patch already does the same, so it stays off
   with that one. face_fix=0 in fightport4.ini turns it off. */
static void FaceFix(int g)
{
    if (PatchState(g, L"skin0", -1) < 0) return;
    BOOL on = GetPrivateProfileIntW(L"FightPort4", L"face_fix", 1, g_pIni) && PatchState(g, L"60fps", -1) != 1;
    PatchState(g, L"skin0", on ? 1 : 0);
}

static BOOL StartPort(int g, BOOL play)
{
    wchar_t exe[MAX_PATH], dir[MAX_PATH], cmd[MAX_PATH + 32]; PortExe(g, exe); PortDir(g, dir);
    if (!FileExists(exe)) { Msg(C_BAD, T(S_NOTFOUND), G[g].exe); return FALSE; }
    if (play) {
        wchar_t miss[64];
        if (CpuLacksPs4(miss, 64)) {
            Msg(C_BAD, g_lang ? L"This processor lacks %s, which the PS4 game code uses directly: UFC 4 cannot run on it. It needs an Intel Core 4th generation (Haswell) or newer, or an AMD Ryzen."
                              : L"Este processador não tem %s, que o código do jogo de PS4 usa direto: o UFC 4 não roda nele. Precisa de um Intel Core de 4ª geração (Haswell) ou mais novo, ou de um AMD Ryzen.", miss);
            return FALSE;
        }
        FaceFix(g);
    }
    _snwprintf(cmd, MAX_PATH + 32, play ? L"\"%s\" --play" : L"\"%s\"", exe);
    STARTUPINFOW si = {sizeof(si)}; PROCESS_INFORMATION pi = {0};
    FILETIME started; GetSystemTimeAsFileTime(&started);
    BOOL okp = CreateProcessW(exe, cmd, NULL, NULL, FALSE, 0, NULL, dir, &si, &pi); DWORD perr = GetLastError();
    if (!okp) { Msg(C_BAD, T(S_CANTSTART), G[g].exe, perr); return FALSE; }
    if (play) {
        g_proc = pi.hProcess; g_procGame = g;
        CloseHandle(CreateThread(NULL, 0, WaitThread, g_proc, 0, NULL));
        WatchArgs *wa = (WatchArgs *)malloc(sizeof(WatchArgs));
        if (wa) { wa->game = g; wa->start = started; wa->session = InterlockedIncrement(&g_watchSession); InterlockedExchange(&g_watchStop, 0); g_frozenFile[0] = 0;
                  HANDLE th = CreateThread(NULL, 0, FreezeWatch, wa, 0, NULL); if (th) CloseHandle(th); else free(wa); }
        Msg(C_GOOD, T(S_STARTING), G[g].name);
    } else CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return TRUE;
}

/* CreateDesktopShortcut moved above */

static void Play(void)
{
    if (g_proc) {   /* STOP: close the port and every process it started (the game engine) */
        DWORD root = GetProcessId(g_proc), kids[64]; int nk = 0, done = 0;
        kids[nk++] = root;
        while (done < nk) {
            DWORD parent = kids[done++];
            HANDLE sn = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); PROCESSENTRY32W pe = {sizeof(pe)};
            if (sn != INVALID_HANDLE_VALUE) {
                if (Process32FirstW(sn, &pe)) do { if (pe.th32ParentProcessID == parent && nk < 64) kids[nk++] = pe.th32ProcessID; } while (Process32NextW(sn, &pe));
                CloseHandle(sn);
            }
        }
        for (int i = nk - 1; i >= 0; i--) { HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, kids[i]); if (h) { TerminateProcess(h, 1); CloseHandle(h); } }
        return;
    }
    SaveEdits();
    int st = Status(g_sel);
    if (st == S_NOPORT) { Msg(C_BAD, T(S_NOTFOUND), G[g_sel].exe); return; }
    if (st == S_PICK) { Msg(C_BAD, L"%s", T(S_CHOOSEFIRST)); return; }
    if (GetInt(g_sel, L"ultra", 0)) Msg(C_SUB, L"%s", T(S_ULTRAON));
    if (StartPort(g_sel, TRUE)) ShowWindow(g_wnd, SW_MINIMIZE);
}
static void OnExited(DWORD code)
{
    CloseHandle(g_proc); g_proc = NULL; g_procGame = -1;
    InterlockedExchange(&g_watchStop, 1);
    if (g_job) { CloseHandle(g_job); g_job = NULL; }
    ShowWindow(g_wnd, SW_RESTORE); SetForegroundWindow(g_wnd);
    g_kvGame = -1;
    if (code == 0 || code == 1) Msg(C_SUB, L"%s", T(S_CLOSED));
    else Msg(C_BAD, T(S_CLOSEDERR), code);
    LoadEdits();
}
static void OpenLogs(void)
{
    wchar_t p[MAX_PATH];
    wchar_t la[MAX_PATH] = L"", d[MAX_PATH]; GetEnvironmentVariableW(L"LOCALAPPDATA", la, MAX_PATH); Join(d, la, G[g_sel].data); Join(p, d, L"logs"); if (!DirExists(p)) wcscpy(p, d);
    if (!DirExists(p)) { Msg(C_SUB, L"%s", T(S_NOLOGS)); return; }
    ShellExecuteW(g_wnd, L"open", p, NULL, NULL, SW_SHOWNORMAL);
}
static void About(void)
{
    const wchar_t *msg = g_lang ?
        L"EA SPORTS UFC 4 - PC Port\n\n"
        L"The port is built on open-source projects: SacredFire / bbport (GNSB), shadPS4 "
        L"and the libraries listed in the licenses folder. "
        L"GPL-2.0-or-later, source code in the src folder.\n\n"
        L"Not affiliated with Electronic Arts, UFC or Sony. You need your own copy of the game." :
        L"EA SPORTS UFC 4 - Port para PC\n\n"
        L"O port é feito sobre projetos de código aberto: SacredFire / bbport (GNSB), shadPS4 "
        L"e as bibliotecas listadas na pasta licenses. "
        L"GPL-2.0-or-later, código-fonte na pasta src.\n\n"
        L"Sem afiliação com a Electronic Arts, o UFC ou a Sony. É preciso ter a sua própria cópia do jogo.";
    ShowThemedModal(g_wnd, L"EA SPORTS UFC 4", msg, THEMED_MODAL_OK, THEMED_ICON_INFO, NULL, g_lang ? L"CLOSE" : L"FECHAR");
}

static void DropdownMenu(int idx)
{
    if (idx < 9 && idx >= NVid(g_sel)) return;
    const Field *f = idx < 9 ? &Vid(g_sel)[idx] : &Gp(g_sel)[idx - 9];
    HMENU m = CreatePopupMenu(); int cur = OptIndex(f, GetInt(g_sel, f->key, f->def));
    for (int i = 0; i < f->nopts; i++) AppendMenuW(m, MF_STRING | (i == cur ? MF_CHECKED : 0), 1 + i, OptName(&f->opts[i]));
    RECT r = {0}; for (int i = 0; i < g_nhits; i++) if (g_hits[i].id == H_DROP + idx) r = g_hits[i].r;
    POINT pt = {r.left, r.bottom + SC(2)}; ClientToScreen(g_wnd, &pt);
    int c = TrackPopupMenu(m, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, g_wnd, NULL);
    DestroyMenu(m);
    if (c <= 0) return;
    int val = f->opts[c - 1].value;
    PutInt(g_sel, f->key, val);
    if (!wcsncmp(f->key, L"patch:", 6)) {
        if (PatchState(g_sel, f->key + 6, -1) < 0)
            Msg(C_BAD, g_lang ? L"This game has no such community patch (patches\\%s-community.xml)." : L"Este jogo não tem esse patch da comunidade (patches\\%s-community.xml).", G[g_sel].id);
        else if (!wcscmp(f->key, L"patch:60fps")) {
            if (val == 0) {
                Msg(C_GOOD, g_lang ? L"30 FPS original selected: fights run stable and work up to the chosen FPS Limit."
                                   : L"30 FPS original selecionado: luta estável, trabalha até o Limite de FPS escolhido.");
            } else {
                Msg(C_SUB, g_lang ? L"60+ FPS patch on: fight at 60+ FPS. Notice: character eyelids and mouths become detached (loose / 'broken')."
                                  : L"Patch 60+ FPS ativado: luta a 60+ FPS. Aviso: pálpebras e bocas ficam descoladas do personagem (moles / \"quebrado\").");
            }
        }
        else Msg(C_GOOD, g_lang ? L"Saved: applies the next time the game starts." : L"Salvo: vale na próxima vez que o jogo abrir.");
    }
    InvalidateRect(g_wnd, NULL, FALSE);
}
static void AdvMenu4(void)
{
    HMENU m = CreatePopupMenu();
    AppendMenuW(m, MF_STRING, 1, g_lang ? L"Best settings for this PC (automatic)" : L"Melhores configurações para este PC (automático)");
    AppendMenuW(m, MF_STRING, 2, g_lang ? L"All options (the port's own window)" : L"Todas as opções (janela do port)");
    RECT r = {0}; for (int i = 0; i < g_nhits; i++) if (g_hits[i].id == H_ADV) r = g_hits[i].r;
    POINT pt = {r.left, r.bottom + SC(2)}; ClientToScreen(g_wnd, &pt);
    int c = TrackPopupMenu(m, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, g_wnd, NULL);
    DestroyMenu(m);
    if (c == 1) { DetectPc(); g_kvGame = -1; AutoConfig4(g_sel); g_kvGame = -1; wchar_t sum[400]; PcSummary(sum, 400);
                  Msg(C_GOOD, g_lang ? L"%s set up for this PC (%s)." : L"%s configurado para este PC (%s).", G[g_sel].name, sum); LoadEdits(); }
    else if (c == 2) StartPort(g_sel, FALSE);
    InvalidateRect(g_wnd, NULL, FALSE);
}
static void ToggleSwitch(int idx)
{
    int n; const Field *f = &Sw(g_sel, &n)[idx]; if (idx >= n) return;
    if (!wcscmp(f->key, L"fullscreen")) {
        BOOL on = !GetInt(g_sel, f->key, f->def);
        PutInt(g_sel, f->key, on ? 1 : 0);
        if (on) Msg(C_GOOD, g_lang ? L"Fullscreen ON: the game will run in full screen."
                                   : L"Tela cheia LIGADA: o jogo rodará em tela cheia.");
        else    Msg(C_WARN, g_lang ? L"Fullscreen OFF: the game will run in windowed mode."
                                   : L"Tela cheia DESLIGADA: o jogo rodará em modo janela.");
    }
    else if (!wcscmp(f->key, L"neo")) {
        BOOL on = !GetInt(g_sel, f->key, f->def);
        PutInt(g_sel, f->key, on ? 1 : 0);
        if (on) Msg(C_GOOD, g_lang ? L"PS4 Pro mode ON: enhanced graphics and higher internal resolution."
                                   : L"Modo PS4 Pro LIGADO: gráficos aprimorados e resolução interna maior.");
        else    Msg(C_WARN, g_lang ? L"PS4 Pro mode OFF: standard PS4 base mode active (lower GPU usage)."
                                   : L"Modo PS4 Pro DESLIGADO: modo base do PS4 ativo (menor uso de GPU).");
    }
    else if (!wcscmp(f->key, L"skip_movies")) {
        BOOL on = !GetInt(g_sel, f->key, f->def);
        PutInt(g_sel, f->key, on ? 1 : 0);
        if (on) Msg(C_GOOD, g_lang ? L"Skip intro videos ON: opening logos and movies will be skipped."
                                   : L"Pular vídeos de introdução LIGADO: logos e abertura serão ignorados.");
        else    Msg(C_WARN, g_lang ? L"Skip intro videos OFF: opening movies will play normally."
                                   : L"Pular vídeos de introdução DESLIGADO: vídeos de abertura serão reproduzidos.");
    }
    else if (!wcscmp(f->key, L"low_latency")) {
        BOOL on = !GetInt(g_sel, f->key, f->def);
        PutInt(g_sel, f->key, on ? 1 : 0);
        if (on) Msg(C_GOOD, g_lang ? L"Low latency ON: input delay reduced (1 frame queued ahead)."
                                   : L"Baixa latência LIGADA: atraso de entrada reduzido (1 quadro à frente).");
        else    Msg(C_WARN, g_lang ? L"Low latency OFF: default driver frame queue restored."
                                   : L"Baixa latência DESLIGADA: fila de quadros padrão do driver restaurada.");
    }
    else if (!wcscmp(f->key, L"ultra")) {
        BOOL on = !GetInt(g_sel, L"ultra", 0);
        UltraSet(g_sel, on);
        if (on) Msg(C_GOOD, g_lang ? L"Ultra performance ON: 720p, PS4 base mode, 30 FPS, and FSR 1.0 applied for maximum performance."
                                   : L"Ultra desempenho LIGADO: 720p, modo PS4 base, 30 FPS e FSR 1.0 aplicados para máximo desempenho.");
        else    Msg(C_WARN, g_lang ? L"Ultra performance OFF: resolution, PS4 Pro mode, FPS limit, and upscaler restored to previous settings."
                                   : L"Ultra desempenho DESLIGADO: resolução, modo PS4 Pro, limite de FPS e upscaler restaurados ao padrão anterior.");
    }
    else if (!wcscmp(f->key, L"parsec")) {
        BOOL on = !GetInt(g_sel, L"parsec", 0);
        ParsecSet(g_sel, on);
        if (on) Msg(C_GOOD, g_lang ? L"Parsec mode ON: low latency, 60 FPS, and Mailbox applied (share PC with friend as Player 2)."
                                   : L"Modo Parsec LIGADO: baixa latência, 60 FPS e Mailbox aplicados (amigo joga como Jogador 2 no Parsec).");
        else    Msg(C_WARN, g_lang ? L"Parsec mode OFF: low latency, FPS limit, and present mode restored to previous settings."
                                   : L"Modo Parsec DESLIGADO: baixa latência, limite de FPS e apresentação restaurados ao padrão anterior.");
    }
    InvalidateRect(g_wnd, NULL, FALSE);
}

/* ---------------------------------------------------------------- window */
static void MakeFonts(void)
{
    #define F(h, w, face) CreateFontW(-SC(h), 0, 0, 0, w, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, face)
    g_fTitle = F(32, FW_BOLD, L"Bahnschrift"); g_fCard = F(20, FW_BOLD, L"Bahnschrift");
    g_fText = F(14, FW_NORMAL, L"Segoe UI"); g_fSmall = F(12, FW_NORMAL, L"Segoe UI"); g_fSection = F(13, FW_BOLD, L"Bahnschrift");
    g_fPlay = F(20, FW_BOLD, L"Bahnschrift"); g_fBtn = F(12, FW_BOLD, L"Bahnschrift"); g_fFoot = F(11, FW_NORMAL, L"Segoe UI");
    g_fTabBig = F(15, FW_BOLD, L"Bahnschrift"); g_fTabNorm = F(12, FW_BOLD, L"Bahnschrift");
    if (g_fSplashPct) DeleteObject(g_fSplashPct);
    if (g_fSplashStatus) DeleteObject(g_fSplashStatus);
    g_fSplashPct = CreateFontW(-SC(16), 0, 0, 0, FW_BOLD, TRUE /* italic */, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Consolas");
    g_fSplashStatus = F(11, FW_BOLD, L"Bahnschrift");
}

static LRESULT CALLBACK WndProc(HWND w, UINT m, WPARAM wp, LPARAM lp)
{
    switch (m) {
    case WM_CREATE: g_wnd = w; return 0;
    case WM_ERASEBKGND: return 1;
    case WM_TIMER: {
        if (wp == 999) {
            if (g_splashActive) {
                ULONGLONG now = GetTickCount64();
                DWORD elapsed = (DWORD)(now >= g_splashStart ? now - g_splashStart : 0);
                if (elapsed >= (SPLASH_DURATION_MS + 80)) {
                    KillTimer(w, 999);
                    g_splashActive = FALSE;
                    if (g_navTab == 0) {
                        ShowWindow(g_editGame, SW_SHOW);
                        ShowWindow(g_editPlayer, SW_SHOW);
                    }
                }
                InvalidateRect(w, NULL, FALSE);
            }
            return 0;
        }
        break;
    }
    case WM_KEYDOWN: {
        if (g_splashActive) {
            KillTimer(w, 999);
            g_splashActive = FALSE;
            if (g_navTab == 0) {
                ShowWindow(g_editGame, SW_SHOW);
                ShowWindow(g_editPlayer, SW_SHOW);
            }
            InvalidateRect(w, NULL, FALSE);
            return 0;
        }
        if (g_themedModalActive) {
            if (wp == VK_ESCAPE) {
                g_themedModalResult = (g_themedModalButtons == THEMED_MODAL_YESNO) ? IDNO : IDCANCEL;
                g_themedModalActive = FALSE;
                InvalidateRect(w, NULL, FALSE);
                return 0;
            } else if (wp == VK_RETURN) {
                g_themedModalResult = (g_themedModalButtons == THEMED_MODAL_YESNO) ? IDYES : IDOK;
                g_themedModalActive = FALSE;
                InvalidateRect(w, NULL, FALSE);
                return 0;
            }
            return 0;
        }
        if (g_iconPickerOpen) {
            if (wp == VK_ESCAPE) {
                g_iconPickerOpen = FALSE;
                if (g_navTab == 0) {
                    ShowWindow(g_editGame, SW_SHOW);
                    ShowWindow(g_editPlayer, SW_SHOW);
                }
                InvalidateRect(w, NULL, FALSE);
                return 0;
            } else if (wp == VK_RETURN) {
                CreateOrUpdateShortcut(FALSE);
                InvalidateRect(w, NULL, FALSE);
                return 0;
            }
        }
        break;
    }
    case WM_PAINT: { PAINTSTRUCT ps; HDC dc = BeginPaint(w, &ps); Paint(dc); EndPaint(w, &ps); return 0; }
    case WM_CTLCOLOREDIT: { HDC dc = (HDC)wp; SetBkColor(dc, C_FIELD); SetTextColor(dc, C_TEXT); return (LRESULT)g_brField; }
    case WM_MOUSEMOVE: {
        if (g_splashActive) {
            RECT rc; GetClientRect(w, &rc);
            int mx = GET_X_LPARAM(lp), my = GET_Y_LPARAM(lp);
            if (rc.right > 0 && rc.bottom > 0) {
                g_splashMouseX = ((float)mx / (float)rc.right) - 0.5f;
                g_splashMouseY = ((float)my / (float)rc.bottom) - 0.5f;
            }
            return 0;
        }
        int h = HitAt(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
        if (h != g_hot) { g_hot = h; InvalidateRect(w, NULL, FALSE); TRACKMOUSEEVENT t = {sizeof(t), TME_LEAVE, w, 0}; TrackMouseEvent(&t); }
        return 0;
    }
    case WM_MOUSELEAVE: if (g_hot != -1) { g_hot = -1; InvalidateRect(w, NULL, FALSE); } return 0;
    case WM_SETCURSOR: {
        if (g_splashActive) { SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_ARROW)); return TRUE; }
        if (g_themedModalActive) {
            if (LOWORD(lp) == HTCLIENT && (g_hot == H_THEMED_MODAL_BTN1 || g_hot == H_THEMED_MODAL_BTN2 || g_hot == H_THEMED_MODAL_CLOSE)) {
                SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_HAND));
                return TRUE;
            }
            SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_ARROW));
            return TRUE;
        }
        if (LOWORD(lp) == HTCLIENT && g_hot >= 0 && g_hot != H_ICON_MODAL_BG && g_hot != H_THEMED_MODAL_BG) { SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_HAND)); return TRUE; }
        break;
    }
    case WM_LBUTTONDOWN: {
        if (g_splashActive) {
            KillTimer(w, 999);
            g_splashActive = FALSE;
            if (g_navTab == 0) {
                ShowWindow(g_editGame, SW_SHOW);
                ShowWindow(g_editPlayer, SW_SHOW);
            }
            InvalidateRect(w, NULL, FALSE);
            return 0;
        }
        SetFocus(w);
        if (g_themedModalActive) {
            int mx = GET_X_LPARAM(lp), my = GET_Y_LPARAM(lp);
            int h = HitAt(mx, my);
            if (h == H_THEMED_MODAL_BTN2) {
                g_themedModalResult = (g_themedModalButtons == THEMED_MODAL_YESNO) ? IDYES : IDOK;
                g_themedModalActive = FALSE;
                InvalidateRect(w, NULL, FALSE);
            } else if (h == H_THEMED_MODAL_BTN1 || h == H_THEMED_MODAL_CLOSE || h == H_THEMED_MODAL_BG) {
                g_themedModalResult = (g_themedModalButtons == THEMED_MODAL_YESNO) ? IDNO : IDCANCEL;
                g_themedModalActive = FALSE;
                InvalidateRect(w, NULL, FALSE);
            }
            return 0;
        }
        if (g_iconPickerOpen) {
            int mx = GET_X_LPARAM(lp), my = GET_Y_LPARAM(lp);
            int h = HitAt(mx, my);
            if (h >= H_ICON_START && h < H_ICON_START + 19) {
                g_selectedIconIndex = h - H_ICON_START;
                SaveSelectedIcon();
                g_iconMsg[0] = 0;
                InvalidateRect(w, NULL, FALSE);
            } else if (h == H_ICON_CREATE_SHORTCUT) {
                CreateOrUpdateShortcut(TRUE);
                InvalidateRect(w, NULL, FALSE);
            } else if (h == H_ICON_CHANGE_SHORTCUT) {
                CreateOrUpdateShortcut(FALSE);
                InvalidateRect(w, NULL, FALSE);
            } else if (h == H_ICON_CLOSE) {
                g_iconPickerOpen = FALSE;
                if (g_navTab == 0) {
                    ShowWindow(g_editGame, SW_SHOW);
                    ShowWindow(g_editPlayer, SW_SHOW);
                }
                InvalidateRect(w, NULL, FALSE);
            } else {
                RECT rModal = { SC(40), SC(33), SC(1240), SC(687) };
                POINT pt = { mx, my };
                if (!PtInRect(&rModal, pt)) {
                    g_iconPickerOpen = FALSE;
                    if (g_navTab == 0) {
                        ShowWindow(g_editGame, SW_SHOW);
                        ShowWindow(g_editPlayer, SW_SHOW);
                    }
                    InvalidateRect(w, NULL, FALSE);
                }
            }
            return 0;
        }
        int h = HitAt(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
        if (h >= H_CARD && h < H_CARD + NGAMES) Select(h - H_CARD);
        else if (h >= H_DROP && h < H_DROP + 11) DropdownMenu(h - H_DROP);
        else if (h >= H_SWITCH && h < H_SWITCH + 6) ToggleSwitch(h - H_SWITCH);
        else switch (h) {
            case H_TAB_CONFIG:
                if (g_navTab != 0) {
                    g_navTab = 0;
                    ShowWindow(g_editGame, SW_SHOW);
                    ShowWindow(g_editPlayer, SW_SHOW);
                    InvalidateRect(w, NULL, FALSE);
                }
                break;
            case H_TAB_PKG:
                if (g_navTab != 1) {
                    SaveEdits();
                    g_navTab = 1;
                    ShowWindow(g_editGame, SW_HIDE);
                    ShowWindow(g_editPlayer, SW_HIDE);
                    InvalidateRect(w, NULL, FALSE);
                }
                break;
            case H_PKG_CHECK_DELETE:
                g_autoDeletePkg = !g_autoDeletePkg;
                InvalidateRect(w, NULL, FALSE);
                break;
            case H_PKG_CLEAR:
                g_haveBase = FALSE;
                g_havePatch = FALSE;
                memset(&g_pkgBase, 0, sizeof(g_pkgBase));
                memset(&g_pkgPatch, 0, sizeof(g_pkgPatch));
                if (g_imgPkgIcon) {
                    GdipDisposeImage(g_imgPkgIcon);
                    g_imgPkgIcon = NULL;
                }
                Msg(C_SUB, g_lang ? L"Packages cleared." : L"Pacotes PKG limpos.");
                InvalidateRect(w, NULL, FALSE);
                break;
            case H_PKG_BROWSE_ALL:
                PickFiles(w, FALSE, FALSE);
                InvalidateRect(w, NULL, FALSE);
                break;
            case H_PKG_BROWSE_BASE:
                PickFiles(w, TRUE, FALSE);
                InvalidateRect(w, NULL, FALSE);
                break;
            case H_PKG_BROWSE_PATCH:
                PickFiles(w, FALSE, TRUE);
                InvalidateRect(w, NULL, FALSE);
                break;
            case H_PKG_EXTRACT_GAME:
            case H_PKG_EXTRACT_CUSTOM: {
                BOOL bValid = (g_haveBase && IsValidUfc4Base(&g_pkgBase));
                BOOL pValid = (g_havePatch && IsValidUfc4Patch(&g_pkgPatch));
                BOOL mismatch = (g_haveBase && g_havePatch && wcscmp(g_pkgBase.titleId, g_pkgPatch.titleId) != 0);
                if (!g_haveBase || !g_havePatch || !bValid || !pValid || mismatch) {
                    ShowThemedModal(w,
                        g_lang ? L"Extraction Blocked" : L"Extração Bloqueada",
                        g_lang ? L"Extraction blocked!\n\nBoth the Base Game (CUSA14209 or CUSA14204) and Update 28.00 are strictly mandatory.\nRunning with only one file or wrong version will crash the port."
                               : L"Extração bloqueada!\n\nÉ estritamente obrigatório possuir tanto o Jogo Base (CUSA14209 ou CUSA14204) quanto a Atualização Update 28.00 válidos.\nExecutar o port com apenas um dos arquivos ou versão incorreta causa crash e fecha o jogo.",
                        THEMED_MODAL_OK, THEMED_ICON_ERROR, NULL, g_lang ? L"UNDERSTOOD" : L"ENTENDIDO");
                    break;
                }
                if (!g_pkgExtracting) {
                    wchar_t dest[MAX_PATH] = L"";
                    if (!PromptAndPrepareExtractionFolder(w, dest, MAX_PATH)) {
                        break;
                    }

                    PkgWorkerArgs *args = (PkgWorkerArgs *)malloc(sizeof(PkgWorkerArgs));
                    if (args) {
                        args->hasBase = g_haveBase;
                        args->hasPatch = g_havePatch;
                        wcsncpy(args->basePath, g_pkgBase.filePath, MAX_PATH);
                        wcsncpy(args->patchPath, g_pkgPatch.filePath, MAX_PATH);
                        wcsncpy(args->destDir, dest, MAX_PATH);
                        args->toActiveGame = TRUE;
                        int mIdx = g_pkgBase.matchedGameIndex >= 0 ? g_pkgBase.matchedGameIndex : g_pkgPatch.matchedGameIndex;
                        args->gameIdx = mIdx >= 0 ? mIdx : g_sel;
                        HANDLE th = CreateThread(NULL, 0, PkgExtractThread, args, 0, NULL);
                        if (th) CloseHandle(th);
                        else free(args);
                    }
                    InvalidateRect(w, NULL, FALSE);
                }
                break;
            }
            case H_PKG_VIEW_SFO: {
                if (g_haveBase || g_havePatch) {
                    wchar_t sfoMsg[2048] = L"";
                    if (g_haveBase) {
                        _snwprintf(sfoMsg + wcslen(sfoMsg), 2048 - wcslen(sfoMsg),
                            L"[JOGO BASE]\nTitle: %s\nTitle ID: %s\nApp Version: %s\nCategory: %s (%s)\nSystem Version: %s\nContent ID: %s\nFile Size: %llu bytes (%s)\nFile: %s\n\n",
                            g_pkgBase.title, g_pkgBase.titleId, g_pkgBase.appVer,
                            g_pkgBase.category, g_pkgBase.categoryName,
                            g_pkgBase.sysVer, g_pkgBase.contentId,
                            (unsigned long long)g_pkgBase.fileSize, g_pkgBase.sizeStr, g_pkgBase.fileName);
                    }
                    if (g_havePatch) {
                        _snwprintf(sfoMsg + wcslen(sfoMsg), 2048 - wcslen(sfoMsg),
                            L"[ATUALIZAÇÃO / PATCH]\nTitle: %s\nTitle ID: %s\nApp Version: %s\nCategory: %s (%s)\nSystem Version: %s\nContent ID: %s\nFile Size: %llu bytes (%s)\nFile: %s",
                            g_pkgPatch.title, g_pkgPatch.titleId, g_pkgPatch.appVer,
                            g_pkgPatch.category, g_pkgPatch.categoryName,
                            g_pkgPatch.sysVer, g_pkgPatch.contentId,
                            (unsigned long long)g_pkgPatch.fileSize, g_pkgPatch.sizeStr, g_pkgPatch.fileName);
                    }
                    ShowThemedModal(w, L"PARAM.SFO & Metadados PKG", sfoMsg, THEMED_MODAL_OK, THEMED_ICON_INFO, NULL, g_lang ? L"CLOSE" : L"FECHAR");
                }
                break;
            }
            case H_PKG_CANCEL: {
                if (g_pkgExtracting) {
                    g_pkgExtractCancel = TRUE;
                    wcsncpy(g_pkgExtractStatus, g_lang ? L"Cancelling extraction..." : L"Cancelando extração...", 255);
                    InvalidateRect(w, NULL, FALSE);
                }
                break;
            }
            case H_BROWSE: Browse(); break;
            case H_PORT: ChangePort(); break;
            case H_ADV: SaveEdits();
                if (g_proc && g_procGame == g_sel) Msg(C_SUB, g_lang ? L"The game is running: close it first." : L"O jogo está aberto: feche o jogo primeiro.");
                else AdvMenu4();
                InvalidateRect(w, NULL, FALSE); break;
            case H_LOGS: OpenLogs(); break;
            case H_SAVE:
                SaveEdits();
                SfSave();
                Msg(C_GOOD, g_lang ? L"Settings saved! Direct desktop shortcut will use these settings."
                                   : L"Configurações salvas com sucesso! O atalho da área de trabalho usará estas configurações.");
                InvalidateRect(w, NULL, FALSE);
                break;
            case H_SHORTCUT:
                SaveEdits();
                SfSave();
                g_iconPickerOpen = TRUE;
                ShowWindow(g_editGame, SW_HIDE);
                ShowWindow(g_editPlayer, SW_HIDE);
                LoadIconPickerImages();
                g_iconMsg[0] = 0;
                InvalidateRect(w, NULL, FALSE);
                break;
            case H_PLAY: Play(); break;
            case H_ABOUT: About(); break;
            case H_EN: case H_PT: g_lang = h == H_EN; WritePrivateProfileStringW(L"FightPort4", L"language", g_lang ? L"0" : L"1", g_pIni); g_msg[0] = 0; InvalidateRect(w, NULL, FALSE); break;
        }
        return 0;
    }
    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wp;
        int count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
        int loaded = 0;
        for (int i = 0; i < count; i++) {
            wchar_t filePath[MAX_PATH] = L"";
            if (DragQueryFileW(hDrop, i, filePath, MAX_PATH)) {
                wchar_t *ext = PathFindExtensionW(filePath);
                if (ext && _wcsicmp(ext, L".pkg") == 0) {
                    LoadPkgPath(filePath);
                    loaded++;
                }
            }
        }
        if (loaded > 0) {
            g_navTab = 1;
            ShowWindow(g_editGame, SW_HIDE);
            ShowWindow(g_editPlayer, SW_HIDE);
            InvalidateRect(w, NULL, FALSE);
        } else {
            Msg(C_ORANGE, g_lang ? L"Dropped file(s) are not .pkg files." : L"Os arquivos arrastados não são arquivos .pkg.");
            InvalidateRect(w, NULL, FALSE);
        }
        DragFinish(hDrop);
        return 0;
    }
    case WM_APP_PKG_DONE: {
        BOOL ok = (BOOL)wp;
        BOOL toActive = (BOOL)lp;
        if (ok) {
            int gIdx = g_pkgBase.matchedGameIndex >= 0 ? g_pkgBase.matchedGameIndex : (g_pkgPatch.matchedGameIndex >= 0 ? g_pkgPatch.matchedGameIndex : g_sel);
            if (toActive && gIdx >= 0) {
                Select(gIdx);
                const wchar_t *finalDest = g_lastExtractedDir[0] ? g_lastExtractedDir : (g_pkgBase.recommendedDir[0] ? g_pkgBase.recommendedDir : UFC4_FOLDER_NAME);
                PutStr(gIdx, L"game", finalDest);
                SetWindowTextW(g_editGame, finalDest);
                SaveEdits();
                SfSave();
                Msg(C_GOOD, g_lang ? L"Extraction finished successfully! %s is updated and ready to play."
                                   : L"Extração e atualização concluídas com sucesso! %s configurado e pronto para jogar.", G[gIdx].name);
            } else {
                Msg(C_GOOD, g_lang ? L"PKG extraction finished successfully!" : L"Extração dos pacotes PKG concluída com sucesso!");
            }

            /* Permanent PKG Deletion logic */
            BOOL shouldDelete = FALSE;
            if (g_autoDeletePkg) {
                shouldDelete = TRUE;
            } else {
                int ans = ShowThemedModal(w,
                    g_lang ? L"Extraction Completed - Delete PKGs?" : L"Extração Concluída - Excluir Arquivos PKG?",
                    g_lang ? L"Extraction completed successfully!\n\nDo you want to permanently delete the original PKG file(s) to free up disk space?\n\n(Warning: files will be permanently deleted without being sent to the Recycle Bin)."
                           : L"A extração e a atualização foram concluídas com sucesso!\n\nDeseja apagar os arquivos PKG originais para liberar espaço em disco?\n\n(Aviso: Os arquivos serão excluídos de forma definitiva, sem enviar para a Lixeira).",
                    THEMED_MODAL_YESNO, THEMED_ICON_WARN,
                    g_lang ? L"KEEP FILES" : L"MANTER ARQUIVOS",
                    g_lang ? L"DELETE PERMANENTLY" : L"EXCLUIR DEFINITIVAMENTE");
                if (ans == IDYES) {
                    shouldDelete = TRUE;
                }
            }

            if (shouldDelete) {
                int deletedCount = 0;
                if (g_haveBase && FileExists(g_pkgBase.filePath)) {
                    if (DeleteFileW(g_pkgBase.filePath)) deletedCount++;
                }
                if (g_havePatch && FileExists(g_pkgPatch.filePath)) {
                    if (DeleteFileW(g_pkgPatch.filePath)) deletedCount++;
                }
                g_haveBase = FALSE;
                g_havePatch = FALSE;
                memset(&g_pkgBase, 0, sizeof(g_pkgBase));
                memset(&g_pkgPatch, 0, sizeof(g_pkgPatch));
                if (g_imgPkgIcon) {
                    GdipDisposeImage(g_imgPkgIcon);
                    g_imgPkgIcon = NULL;
                }
                Msg(C_GOOD, g_lang ? L"Extraction completed! %d PKG file(s) permanently deleted."
                                   : L"Extração concluída! %d arquivo(s) PKG excluído(s) definitivamente.", deletedCount);
            }
        } else {
            if (g_pkgExtractCancel) {
                Msg(C_ORANGE, g_lang ? L"Extraction cancelled by user." : L"Extração cancelada pelo usuário.");
            } else {
                Msg(C_BAD, g_lang ? L"Extraction failed or encountered errors." : L"Falha durante a extração do PKG.");
            }
        }
        InvalidateRect(w, NULL, FALSE);
        return 0;
    }
    case WM_COMMAND: if (HIWORD(wp) == EN_KILLFOCUS) { SaveEdits(); InvalidateRect(w, NULL, FALSE); } return 0;
    case WM_APP_EXITED: OnExited((DWORD)wp); return 0;
    case WM_APP_AUTOCFG:
        if (g_autoDone4) {
            wchar_t sum[400]; PcSummary(sum, 400);
            Msg(C_GOOD, g_lang ? L"Best settings picked for this PC: %s" : L"Melhores configurações escolhidas para este PC: %s", sum);
            g_autoDone4 = 0; g_kvGame = -1; InvalidateRect(w, NULL, FALSE);
        }
        if (GpuIsArc() && !GpuHasRebar())   /* the one thing an Arc card cannot do without */
            Msg(C_ORANGE, g_lang ? L"Intel Arc without Resizable BAR: turn on \"Re-Size BAR\" and \"Above 4G Decoding\" in the BIOS, or the card loses a lot of FPS."
                                 : L"Intel Arc sem Resizable BAR: ligue \"Re-Size BAR\" e \"Above 4G Decoding\" na BIOS, senão a placa perde muito FPS.");
        return 0;
    case WM_APP_FROZEN:   /* the engine stopped getting GPU work done: say so and where the diagnosis went */
        FlashWindow(w, TRUE);
        Msg(C_BAD, g_lang ? L"%s froze (the GPU stopped answering). The diagnosis was saved to the logs folder (%s). Click STOP to close it." :
                            L"O %s travou (a GPU parou de responder). O diagnóstico foi salvo na pasta de logs (%s). Clique em PARAR para fechar.",
            G[wp < NGAMES ? wp : 0].name, PathFindFileNameW(g_frozenFile));
        InvalidateRect(w, NULL, FALSE); return 0;
    case WM_ACTIVATE: if (LOWORD(wp) != WA_INACTIVE) { g_kvGame = -1; for (int i = 0; i < NGAMES; i++) ReadVersion(i); InvalidateRect(w, NULL, FALSE); } break;
    case WM_DPICHANGED: {
        g_dpi = HIWORD(wp); MakeFonts();
        SendMessageW(g_editGame, WM_SETFONT, (WPARAM)g_fText, TRUE); SendMessageW(g_editPlayer, WM_SETFONT, (WPARAM)g_fText, TRUE);
        RECT *r = (RECT *)lp; SetWindowPos(w, NULL, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOACTIVATE); return 0;
    }
    case WM_CLOSE: SaveEdits(); DestroyWindow(w); return 0;
    case WM_DESTROY:
        if (g_fSplashPct) { DeleteObject(g_fSplashPct); g_fSplashPct = NULL; }
        if (g_fSplashStatus) { DeleteObject(g_fSplashStatus); g_fSplashStatus = NULL; }
        if (g_imgSplashBg) { GdipDisposeImage(g_imgSplashBg); g_imgSplashBg = NULL; }
        if (g_imgSplashFg) { GdipDisposeImage(g_imgSplashFg); g_imgSplashFg = NULL; }
        FreeIconPickerImages(); PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(w, m, wp, lp);
}

/* dark context menus (uxtheme ordinals; ignored where unavailable) */
static void DarkMenus(void)
{
    HMODULE ux = LoadLibraryW(L"uxtheme.dll"); if (!ux) return;
    typedef int (WINAPI *SetMode)(int); typedef void (WINAPI *Flush)(void);
    SetMode sm = (SetMode)(void *)GetProcAddress(ux, MAKEINTRESOURCEA(135)); Flush fl = (Flush)(void *)GetProcAddress(ux, MAKEINTRESOURCEA(136));
    if (sm) sm(2); if (fl) fl();
}



int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, PWSTR cmdline, int show)
{
    (void)prev; (void)cmdline;
    g_inst = inst;
    HANDLE single = CreateMutexW(NULL, TRUE, L"FightPort4_Launcher_Single");
    if (GetLastError() == ERROR_ALREADY_EXISTS) { HWND o = FindWindowW(L"FightPort4Launcher", NULL); if (o) { ShowWindow(o, SW_RESTORE); SetForegroundWindow(o); } return 0; }
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    INITCOMMONCONTROLSEX ic = {sizeof(ic), ICC_STANDARD_CLASSES}; InitCommonControlsEx(&ic);
    GdiplusStartupInput gi = {1, NULL, FALSE, FALSE}; GdiplusStartup(&g_gdip, &gi, NULL);
    DarkMenus();

    GetModuleFileNameW(NULL, g_exeDir, MAX_PATH); wchar_t *s = wcsrchr(g_exeDir, L'\\'); if (s) *s = 0;
    {
        wchar_t la[MAX_PATH] = L"", d[MAX_PATH];
        GetEnvironmentVariableW(L"LOCALAPPDATA", la, MAX_PATH);
        Join(d, la, L"FightPort4");
        CreateDirectoryW(d, NULL);
        Join(g_pIni, d, L"fightport4.ini");
    }
    g_lang = GetPrivateProfileIntW(L"FightPort4", L"language", 1, g_pIni) ? 0 : 1;
    g_sel = 0;
    ReadVersion(0);
    SfLoad(g_sel);
    LoadSelectedIcon();

    /* Direct Play argument from Desktop Shortcut */
    if (cmdline && (wcsstr(cmdline, L"--direct-play") || wcsstr(cmdline, L"--play"))) {
        if (Status(g_sel) == S_READY) {
            DetectPc();
            if (StartPort(g_sel, TRUE)) {
                if (g_proc) {
                    WaitForSingleObject(g_proc, INFINITE);
                    CloseHandle(g_proc);
                }
                CloseHandle(single);
                return 0;
            }
        }
    }

    g_imgBgConfig = LoadImageFileOrRes(L"Background_menu_configurações.png", 102);
    g_imgBgExtra = LoadImageFileOrRes(L"Background_menu_extrações.png", 103);
    g_imgBg = g_imgBgConfig;
    g_imgLogo = NULL;
    g_imgSplashBg = LoadImageFileOrRes(L"splash_bg.png", 104);
    g_imgSplashFg = LoadImageFileOrRes(L"splash_fg.png", 105);
    g_brField = CreateSolidBrush(C_FIELD);

    /* Register explicit AppUserModelID so Windows taskbar does not link
       the running application to custom desktop shortcut icons */
    typedef HRESULT (WINAPI *SetAppIdFn)(PCWSTR);
    HMODULE hShell = LoadLibraryW(L"shell32.dll");
    if (hShell) {
        SetAppIdFn pSetAppId = (SetAppIdFn)(void *)GetProcAddress(hShell, "SetCurrentProcessExplicitAppUserModelID");
        if (pSetAppId) pSetAppId(L"FightPort4.UFC.Launcher");
        FreeLibrary(hShell);
    }

    HICON hAppIcon = LoadIconW(inst, MAKEINTRESOURCEW(1));
    HICON hAppIconSm = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(1), IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    if (!hAppIconSm) hAppIconSm = hAppIcon;

    WNDCLASSEXW wc = {sizeof(wc)}; wc.lpfnWndProc = WndProc; wc.hInstance = inst; wc.lpszClassName = L"FightPort4Launcher";
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.hIcon = hAppIcon;
    wc.hIconSm = hAppIconSm;
    RegisterClassExW(&wc);
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    POINT cur; GetCursorPos(&cur); HMONITOR mon = MonitorFromPoint(cur, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi = {sizeof(mi)}; GetMonitorInfoW(mon, &mi);
    UINT dx = 96, dy = 96; HMODULE sh = LoadLibraryW(L"shcore.dll");
    if (sh) { typedef HRESULT (WINAPI *GDFM)(HMONITOR, int, UINT *, UINT *); GDFM f = (GDFM)(void *)GetProcAddress(sh, "GetDpiForMonitor"); if (f) f(mon, 0, &dx, &dy); }
    g_dpi = (int)dx;
    /* fit small screens: never taller than the work area */
    int wah = mi.rcWork.bottom - mi.rcWork.top;
    while (g_dpi > 96 && SC(BASE_H) + SC(60) > wah) g_dpi -= 8;
    MakeFonts();
    RECT r = {0, 0, SC(BASE_W), SC(BASE_H)}; AdjustWindowRectExForDpi(&r, style, FALSE, 0, g_dpi);
    int ww = r.right - r.left, wh = r.bottom - r.top;
    HWND w = CreateWindowExW(0, wc.lpszClassName, L"FightPort 4 - EA SPORTS UFC 4", style,
                             mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) - ww) / 2, mi.rcWork.top + max(0, (wah - wh) / 2), ww, wh, NULL, NULL, inst, NULL);
    SendMessageW(w, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
    SendMessageW(w, WM_SETICON, ICON_SMALL, (LPARAM)hAppIconSm);
    DragAcceptFiles(w, TRUE);
    BOOL dark = TRUE; DwmSetWindowAttribute(w, 20, &dark, sizeof(dark));
    COLORREF cap = RGB(4, 7, 18); DwmSetWindowAttribute(w, 35 /* DWMWA_CAPTION_COLOR */, &cap, sizeof(cap));
    g_editGame = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 10, 10, w, (HMENU)300, inst, NULL);
    g_editPlayer = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 10, 10, w, (HMENU)301, inst, NULL);
    SendMessageW(g_editGame, WM_SETFONT, (WPARAM)g_fText, TRUE); SendMessageW(g_editPlayer, WM_SETFONT, (WPARAM)g_fText, TRUE);
    SendMessageW(g_editPlayer, EM_LIMITTEXT, 16, 0);
    LoadEdits();

    g_splashActive = TRUE;
    g_splashStart = GetTickCount64();
    InitSplashParticles(SC(BASE_W), SC(BASE_H));
    SetTimer(w, 999, 16, NULL);

    /* Pre-load detected PKGs if available */
    {
        const wchar_t *knownBase = L"D:\\GAMES\\PlayStation_4\\UFC_4\\EA.SPORTS.UFC.4_CUSA14209_v1.00_[7.50]_OPOISSO893.pkg";
        const wchar_t *knownPatch = L"D:\\GAMES\\PlayStation_4\\UFC_4\\EA.SPORTS.UFC.4_CUSA14209_v28.00_[7.50]-PS4.pkg";
        if (FileExists(knownBase)) LoadPkgPath(knownBase);
        if (FileExists(knownPatch)) LoadPkgPath(knownPatch);

        const wchar_t *knownBase2 = L"D:\\GAMES\\PlayStation_4\\EA SPORTS\u2122 UFC\u00AE 4\\EA.SPORTS.UFC.4_CUSA14209_v1.00_[7.50]_OPOISSO893.pkg";
        const wchar_t *knownPatch2 = L"D:\\GAMES\\PlayStation_4\\EA SPORTS\u2122 UFC\u00AE 4\\EA.SPORTS.UFC.4_CUSA14209_v28.00_[7.50]-PS4.pkg";
        if (FileExists(knownBase2)) LoadPkgPath(knownBase2);
        if (FileExists(knownPatch2)) LoadPkgPath(knownPatch2);

        const wchar_t *knownBase3 = L"D:\\GAMES\\PlayStation_4\\EA_Sports_UFC_4\\EA.SPORTS.UFC.4_CUSA14209_v1.00_[7.50]_OPOISSO893.pkg";
        const wchar_t *knownPatch3 = L"D:\\GAMES\\PlayStation_4\\EA_Sports_UFC_4\\EA.SPORTS.UFC.4_CUSA14209_v28.00_[7.50]-PS4.pkg";
        if (FileExists(knownBase3)) LoadPkgPath(knownBase3);
        if (FileExists(knownPatch3)) LoadPkgPath(knownPatch3);

        const wchar_t *knownDlBase = L"C:\\Users\\hindl\\Downloads\\EA Sports UFC 4\\EA.SPORTS.UFC.4_CUSA14209_v1.00_[7.50]_OPOISSO893.pkg";
        if (FileExists(knownDlBase) && !g_haveBase) LoadPkgPath(knownDlBase);

        wchar_t pat[MAX_PATH]; Join(pat, g_exeDir, L"*.pkg");
        WIN32_FIND_DATAW pfd; HANDLE hFind = FindFirstFileW(pat, &pfd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(pfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    wchar_t full[MAX_PATH]; Join(full, g_exeDir, pfd.cFileName);
                    LoadPkgPath(full);
                }
            } while (FindNextFileW(hFind, &pfd));
            FindClose(hFind);
        }
    }
    ShowWindow(w, show); UpdateWindow(w);
    DetectPc();
    if (AutoConfigAll(FALSE)) {
        wchar_t sum[400]; PcSummary(sum, 400);
        Msg(C_GOOD, g_lang ? L"Best settings picked for this PC: %s" : L"Melhores configurações escolhidas para este PC: %s", sum);
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    CloseHandle(single);
    return 0;
}
