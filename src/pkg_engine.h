/*
 * pkg_engine.h - Native PS4 PKG & PARAM.SFO parser and extractor
 * Win32 C implementation for FightPort 4
 */
#ifndef PKG_ENGINE_H
#define PKG_ENGINE_H

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PKG_MAGIC 0x7F434E54   /* "\x7FCNT" in big-endian */
#define SFO_MAGIC 0x46535000   /* "\0PSF" in little-endian */

/* Standard PKG Entry IDs */
#define PKG_ENTRY_DIGESTS        0x0001
#define PKG_ENTRY_ENTRY_NAMES    0x0010
#define PKG_ENTRY_LICENSE_DAT    0x0020
#define PKG_ENTRY_PFS_IMAGE      0x0200
#define PKG_ENTRY_PARAM_SFO      0x1000
#define PKG_ENTRY_CHANGEINFO_XML 0x1008
#define PKG_ENTRY_ICON0_PNG      0x1200
#define PKG_ENTRY_PIC0_PNG       0x1220
#define PKG_ENTRY_PIC1_PNG       0x1240
#define PKG_ENTRY_SND0_AT9       0x1260
#define PKG_ENTRY_TROPHY00_TRP   0x1400

typedef struct {
    wchar_t filePath[MAX_PATH];
    wchar_t fileName[MAX_PATH];
    wchar_t title[128];
    wchar_t titleId[32];       /* e.g. "CUSA14209" */
    wchar_t contentId[64];     /* e.g. "EP0006-CUSA14209_00-EASPORTSUFC4GAME" */
    wchar_t appVer[32];        /* e.g. "01.00" or "28.00" */
    wchar_t version[32];       /* SFO VERSION */
    wchar_t category[16];      /* "gd" (game), "gp" (patch), "ac" (addon) */
    wchar_t categoryName[32];  /* "Jogo Base", "Atualização", "DLC / Addon" */
    wchar_t region[16];        /* "US", "EU", "JP", "AS" */
    wchar_t sysVer[16];        /* e.g. "7.50" */
    uint64_t fileSize;         /* total bytes */
    wchar_t sizeStr[32];       /* formatted "24.58 GB" */
    uint32_t contentType;      /* 0x1a, etc. */
    BOOL isFake;               /* True for FPKG */
    int matchedGameIndex;      /* Index in FightPort 4 games array, or -1 if not matched */
    wchar_t recommendedDir[MAX_PATH]; /* e.g. "games\\UFC4" or "games\\UFC4\\CUSA14209-patch" */
} PkgInfo;

/* Callback for extraction progress: pct (0..100), current filename, user param */
typedef void (*PkgProgressCallback)(int pct, uint64_t bytesDone, uint64_t bytesTotal, const wchar_t *statusText, void *userParam);

/* Functions */
BOOL PkgReadInfo(const wchar_t *pkgPath, PkgInfo *outInfo);
BOOL PkgExtractEntry(const wchar_t *pkgPath, uint32_t entryId, const wchar_t *outPath);
BOOL PkgExtractEntryToMemory(const wchar_t *pkgPath, uint32_t entryId, void **outBuffer, size_t *outSize);
void PkgFreeMemory(void *buffer);

/* Scan directory for .pkg files */
int PkgScanDirectory(const wchar_t *dirPath, PkgInfo *outList, int maxList);

/* Formats byte size to human readable (e.g. "24.58 GB") */
void PkgFormatSize(uint64_t bytes, wchar_t *outStr, size_t maxLen);

/* Extract full PKG or install to target directory */
BOOL PkgExtractFull(const wchar_t *pkgPath, const wchar_t *destDir, volatile BOOL *cancelFlag, PkgProgressCallback cb, void *userParam);

#ifdef __cplusplus
}
#endif

#endif /* PKG_ENGINE_H */
