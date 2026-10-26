/*
 * pkg_engine.c - Native PS4 PKG & PARAM.SFO parser and extractor
 * Win32 C implementation for FightPort 4
 */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pkg_engine.h"

/* Byte swap helpers */
static inline uint16_t bswap16(uint16_t x) {
    return (uint16_t)((x << 8) | (x >> 8));
}

static inline uint32_t bswap32(uint32_t x) {
    return ((x << 24) & 0xFF000000) |
           ((x <<  8) & 0x00FF0000) |
           ((x >>  8) & 0x0000FF00) |
           ((x >> 24) & 0x000000FF);
}

static inline uint64_t bswap64(uint64_t x) {
    return ((x << 56) & 0xFF00000000000000ULL) |
           ((x << 40) & 0x00FF000000000000ULL) |
           ((x << 24) & 0x0000FF0000000000ULL) |
           ((x <<  8) & 0x000000FF00000000ULL) |
           ((x >>  8) & 0x00000000FF000000ULL) |
           ((x >> 24) & 0x0000000000FF0000ULL) |
           ((x >> 40) & 0x000000000000FF00ULL) |
           ((x >> 56) & 0x00000000000000FFULL);
}

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;                 /* 0x00: 0x7F434E54 ("\x7FCNT") */
    uint32_t flags;                 /* 0x04 */
    uint32_t unk08;                 /* 0x08 */
    uint32_t unk0C;                 /* 0x0C */
    uint32_t entryCount;            /* 0x10 */
    uint32_t scEntryCount;          /* 0x14 */
    uint32_t entryTableOffset;      /* 0x18 */
    uint32_t mainEntryDataSize;     /* 0x1C */
    uint64_t bodyOffset;            /* 0x20 */
    uint64_t bodySize;              /* 0x28 */
    uint64_t packageSize;           /* 0x30 */
    uint64_t padding38;             /* 0x38 */
    char contentId[48];             /* 0x40 */
    uint32_t drmType;               /* 0x70 */
    uint32_t contentType;           /* 0x74 */
    uint32_t contentFlags;          /* 0x78 */
} RawPkgHeader;

typedef struct {
    uint32_t id;                    /* Entry ID */
    uint32_t nameOffset;            /* Offset in names table */
    uint32_t flags1;
    uint32_t flags2;
    uint32_t dataOffset;            /* Offset in PKG file */
    uint32_t dataSize;              /* Size in bytes */
    uint64_t padding;
} RawPkgEntry;

typedef struct {
    uint32_t magic;                 /* 0x00: 0x46535000 ("\0PSF") */
    uint32_t version;               /* 0x04 */
    uint32_t keyTableOffset;        /* 0x08 */
    uint32_t dataTableOffset;       /* 0x0C */
    uint32_t entryCount;            /* 0x10 */
} RawSfoHeader;

typedef struct {
    uint16_t keyOffset;
    uint16_t paramFmt;              /* 0x0204: string, 0x0004: uint32 */
    uint32_t paramLen;
    uint32_t paramMaxLen;
    uint32_t dataOffset;
} RawSfoEntry;
#pragma pack(pop)

void PkgFormatSize(uint64_t bytes, wchar_t *outStr, size_t maxLen)
{
    if (bytes >= (1024ULL * 1024ULL * 1024ULL)) {
        double gb = (double)bytes / (1024.0 * 1024.0 * 1024.0);
        _snwprintf(outStr, maxLen, L"%.2f GB", gb);
    } else if (bytes >= (1024ULL * 1024ULL)) {
        double mb = (double)bytes / (1024.0 * 1024.0);
        _snwprintf(outStr, maxLen, L"%.2f MB", mb);
    } else if (bytes >= 1024ULL) {
        double kb = (double)bytes / 1024.0;
        _snwprintf(outStr, maxLen, L"%.1f KB", kb);
    } else {
        _snwprintf(outStr, maxLen, L"%llu bytes", (unsigned long long)bytes);
    }
    outStr[maxLen - 1] = 0;
}

/* Parse raw PARAM.SFO buffer into PkgInfo */
static BOOL ParseSfo(const unsigned char *sfoBytes, size_t sfoSize, PkgInfo *outInfo)
{
    if (sfoSize < sizeof(RawSfoHeader)) return FALSE;
    const RawSfoHeader *hdr = (const RawSfoHeader *)sfoBytes;
    if (hdr->magic != SFO_MAGIC) return FALSE;

    uint32_t keyTableOff = hdr->keyTableOffset;
    uint32_t dataTableOff = hdr->dataTableOffset;
    uint32_t numEntries = hdr->entryCount;

    if (sizeof(RawSfoHeader) + numEntries * sizeof(RawSfoEntry) > sfoSize)
        return FALSE;

    const RawSfoEntry *entries = (const RawSfoEntry *)(sfoBytes + sizeof(RawSfoHeader));
    for (uint32_t i = 0; i < numEntries; i++) {
        uint32_t kOff = keyTableOff + entries[i].keyOffset;
        if (kOff >= sfoSize) continue;
        const char *keyName = (const char *)(sfoBytes + kOff);

        uint32_t dOff = dataTableOff + entries[i].dataOffset;
        uint32_t dLen = entries[i].paramLen;
        if (dOff + dLen > sfoSize) continue;
        const unsigned char *valPtr = sfoBytes + dOff;

        if (!strcmp(keyName, "TITLE")) {
            MultiByteToWideChar(CP_UTF8, 0, (const char *)valPtr, (int)dLen, outInfo->title, 127);
            outInfo->title[127] = 0;
        } else if (!strcmp(keyName, "TITLE_ID")) {
            MultiByteToWideChar(CP_UTF8, 0, (const char *)valPtr, (int)dLen, outInfo->titleId, 31);
            outInfo->titleId[31] = 0;
        } else if (!strcmp(keyName, "APP_VER")) {
            MultiByteToWideChar(CP_UTF8, 0, (const char *)valPtr, (int)dLen, outInfo->appVer, 31);
            outInfo->appVer[31] = 0;
        } else if (!strcmp(keyName, "VERSION")) {
            MultiByteToWideChar(CP_UTF8, 0, (const char *)valPtr, (int)dLen, outInfo->version, 31);
            outInfo->version[31] = 0;
        } else if (!strcmp(keyName, "CATEGORY")) {
            MultiByteToWideChar(CP_UTF8, 0, (const char *)valPtr, (int)dLen, outInfo->category, 15);
            outInfo->category[15] = 0;
        } else if (!strcmp(keyName, "SYSTEM_VER")) {
            if (dLen >= 4) {
                /* System version format: e.g. 0x07500000 -> 7.50 */
                uint32_t sv = *(const uint32_t *)valPtr;
                uint32_t major = (sv >> 24) & 0xFF;
                uint32_t minor = (sv >> 16) & 0xFF;
                _snwprintf(outInfo->sysVer, 16, L"%x.%02x", major, minor);
            }
        }
    }

    /* Assign category name */
    if (!wcscmp(outInfo->category, L"gd")) {
        wcscpy(outInfo->categoryName, L"Jogo Base");
    } else if (!wcscmp(outInfo->category, L"gp")) {
        wcscpy(outInfo->categoryName, L"Atualização (Patch)");
    } else if (!wcscmp(outInfo->category, L"ac")) {
        wcscpy(outInfo->categoryName, L"DLC / Add-on");
    } else {
        wcsncpy(outInfo->categoryName, outInfo->category, 31);
    }

    return TRUE;
}

/* Match PKG Title ID to known UFC games */
static void MatchGame(PkgInfo *outInfo)
{
    outInfo->matchedGameIndex = -1;
    outInfo->recommendedDir[0] = 0;

    /* Known UFC Titles */
    if (!wcscmp(outInfo->titleId, L"CUSA00222")) {
        outInfo->matchedGameIndex = 0; /* UFC 1 */
    } else if (!wcscmp(outInfo->titleId, L"CUSA01968")) {
        outInfo->matchedGameIndex = 1; /* UFC 2 */
    } else if (!wcscmp(outInfo->titleId, L"CUSA06534")) {
        outInfo->matchedGameIndex = 2; /* UFC 3 */
    } else if (!wcscmp(outInfo->titleId, L"CUSA14209") || !wcscmp(outInfo->titleId, L"CUSA14204")) {
        outInfo->matchedGameIndex = 0; /* UFC 4 in current FightPort 4 build */
    } else if (!wcscmp(outInfo->titleId, L"PPSA03541")) {
        outInfo->matchedGameIndex = 4; /* UFC 5 */
    }

    /* Recommend target directory (both base and patch extract into the same root game directory) */
    if (!wcscmp(outInfo->titleId, L"CUSA14209") || !wcscmp(outInfo->titleId, L"CUSA14204")) {
        wcscpy(outInfo->recommendedDir, L"EA_Sports_UFC_4");
    } else if (outInfo->titleId[0]) {
        _snwprintf(outInfo->recommendedDir, MAX_PATH, L"%s", outInfo->titleId);
    }
}

BOOL PkgReadInfo(const wchar_t *pkgPath, PkgInfo *outInfo)
{
    if (!pkgPath || !outInfo) return FALSE;
    memset(outInfo, 0, sizeof(PkgInfo));
    wcsncpy(outInfo->filePath, pkgPath, MAX_PATH - 1);

    const wchar_t *fname = PathFindFileNameW(pkgPath);
    if (fname) wcsncpy(outInfo->fileName, fname, MAX_PATH - 1);

    HANDLE f = CreateFileW(pkgPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return FALSE;

    LARGE_INTEGER fsz;
    if (GetFileSizeEx(f, &fsz)) {
        outInfo->fileSize = (uint64_t)fsz.QuadPart;
        PkgFormatSize(outInfo->fileSize, outInfo->sizeStr, 32);
    }

    RawPkgHeader hdr;
    DWORD read = 0;
    if (!ReadFile(f, &hdr, sizeof(hdr), &read, NULL) || read < sizeof(hdr)) {
        CloseHandle(f);
        return FALSE;
    }

    if (bswap32(hdr.magic) != PKG_MAGIC) {
        CloseHandle(f);
        return FALSE;
    }

    uint32_t entryCount = bswap32(hdr.entryCount);
    uint32_t entryTableOffset = bswap32(hdr.entryTableOffset);
    outInfo->contentType = bswap32(hdr.contentType);
    outInfo->isFake = (bswap32(hdr.drmType) == 0 || bswap32(hdr.drmType) == 15);

    /* Content ID */
    MultiByteToWideChar(CP_UTF8, 0, hdr.contentId, 48, outInfo->contentId, 63);
    outInfo->contentId[63] = 0;

    /* Region based on prefix */
    if (!wcsncmp(outInfo->contentId, L"UP", 2)) wcscpy(outInfo->region, L"US");
    else if (!wcsncmp(outInfo->contentId, L"EP", 2)) wcscpy(outInfo->region, L"EU");
    else if (!wcsncmp(outInfo->contentId, L"JP", 2)) wcscpy(outInfo->region, L"JP");
    else if (!wcsncmp(outInfo->contentId, L"HP", 2)) wcscpy(outInfo->region, L"AS");
    else wcscpy(outInfo->region, L"GLOBAL");

    /* Seek to entry table */
    LARGE_INTEGER li;
    li.QuadPart = entryTableOffset;
    SetFilePointerEx(f, li, NULL, FILE_BEGIN);

    size_t tableBytes = (size_t)entryCount * sizeof(RawPkgEntry);
    RawPkgEntry *entries = (RawPkgEntry *)malloc(tableBytes);
    if (!entries) {
        CloseHandle(f);
        return FALSE;
    }

    if (!ReadFile(f, entries, (DWORD)tableBytes, &read, NULL) || read < tableBytes) {
        free(entries);
        CloseHandle(f);
        return FALSE;
    }

    /* Locate and read PARAM.SFO entry */
    for (uint32_t i = 0; i < entryCount; i++) {
        uint32_t eid = bswap32(entries[i].id);
        if (eid == PKG_ENTRY_PARAM_SFO) {
            uint32_t dOff = bswap32(entries[i].dataOffset);
            uint32_t dSz = bswap32(entries[i].dataSize);
            if (dSz > 0 && dSz < 512 * 1024) {
                unsigned char *sfoBuf = (unsigned char *)malloc(dSz);
                if (sfoBuf) {
                    li.QuadPart = dOff;
                    SetFilePointerEx(f, li, NULL, FILE_BEGIN);
                    if (ReadFile(f, sfoBuf, dSz, &read, NULL) && read == dSz) {
                        ParseSfo(sfoBuf, dSz, outInfo);
                    }
                    free(sfoBuf);
                }
            }
            break;
        }
    }

    free(entries);
    CloseHandle(f);

    MatchGame(outInfo);
    return TRUE;
}

BOOL PkgExtractEntryToMemory(const wchar_t *pkgPath, uint32_t targetEntryId, void **outBuffer, size_t *outSize)
{
    if (!pkgPath || !outBuffer || !outSize) return FALSE;
    *outBuffer = NULL;
    *outSize = 0;

    HANDLE f = CreateFileW(pkgPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return FALSE;

    RawPkgHeader hdr;
    DWORD read = 0;
    if (!ReadFile(f, &hdr, sizeof(hdr), &read, NULL) || read < sizeof(hdr)) {
        CloseHandle(f);
        return FALSE;
    }

    if (bswap32(hdr.magic) != PKG_MAGIC) {
        CloseHandle(f);
        return FALSE;
    }

    uint32_t entryCount = bswap32(hdr.entryCount);
    uint32_t entryTableOffset = bswap32(hdr.entryTableOffset);

    LARGE_INTEGER li;
    li.QuadPart = entryTableOffset;
    SetFilePointerEx(f, li, NULL, FILE_BEGIN);

    size_t tableBytes = (size_t)entryCount * sizeof(RawPkgEntry);
    RawPkgEntry *entries = (RawPkgEntry *)malloc(tableBytes);
    if (!entries) { CloseHandle(f); return FALSE; }

    if (!ReadFile(f, entries, (DWORD)tableBytes, &read, NULL) || read < tableBytes) {
        free(entries);
        CloseHandle(f);
        return FALSE;
    }

    BOOL found = FALSE;
    for (uint32_t i = 0; i < entryCount; i++) {
        uint32_t eid = bswap32(entries[i].id);
        if (eid == targetEntryId) {
            uint32_t dOff = bswap32(entries[i].dataOffset);
            uint32_t dSz = bswap32(entries[i].dataSize);
            if (dSz > 0 && dSz < 32 * 1024 * 1024) { /* 32 MB max for single metadata entry */
                void *buf = malloc(dSz);
                if (buf) {
                    li.QuadPart = dOff;
                    SetFilePointerEx(f, li, NULL, FILE_BEGIN);
                    if (ReadFile(f, buf, dSz, &read, NULL) && read == dSz) {
                        *outBuffer = buf;
                        *outSize = dSz;
                        found = TRUE;
                    } else {
                        free(buf);
                    }
                }
            }
            break;
        }
    }

    free(entries);
    CloseHandle(f);
    return found;
}

void PkgFreeMemory(void *buffer)
{
    if (buffer) free(buffer);
}

BOOL PkgExtractEntry(const wchar_t *pkgPath, uint32_t entryId, const wchar_t *outPath)
{
    void *buf = NULL;
    size_t sz = 0;
    if (!PkgExtractEntryToMemory(pkgPath, entryId, &buf, &sz)) return FALSE;

    /* Ensure directory exists */
    wchar_t dir[MAX_PATH];
    wcsncpy(dir, outPath, MAX_PATH - 1);
    PathRemoveFileSpecW(dir);
    SHCreateDirectoryExW(NULL, dir, NULL);

    HANDLE f = CreateFileW(outPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) {
        PkgFreeMemory(buf);
        return FALSE;
    }

    DWORD written = 0;
    BOOL ok = WriteFile(f, buf, (DWORD)sz, &written, NULL) && (written == sz);
    CloseHandle(f);
    PkgFreeMemory(buf);
    return ok;
}

int PkgScanDirectory(const wchar_t *dirPath, PkgInfo *outList, int maxList)
{
    if (!dirPath || !outList || maxList <= 0) return 0;
    wchar_t pattern[MAX_PATH];
    _snwprintf(pattern, MAX_PATH, L"%s\\*.pkg", dirPath);

    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;

    int count = 0;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            wchar_t fullPath[MAX_PATH];
            _snwprintf(fullPath, MAX_PATH, L"%s\\%s", dirPath, fd.cFileName);
            if (PkgReadInfo(fullPath, &outList[count])) {
                count++;
                if (count >= maxList) break;
            }
        }
    } while (FindNextFileW(h, &fd));

    FindClose(h);
    return count;
}

/* Helpers to locate external tools */
static BOOL FindPythonExe(wchar_t *outPath, size_t maxLen)
{
    wchar_t cand[MAX_PATH];
    GetModuleFileNameW(NULL, cand, MAX_PATH);
    PathRemoveFileSpecW(cand);

    wchar_t p[MAX_PATH];
    _snwprintf(p, MAX_PATH, L"%s\\games\\UFC4\\python\\python.exe", cand);
    if (PathFileExistsW(p)) { wcsncpy(outPath, p, maxLen); return TRUE; }

    if (PathFileExistsW(L"D:\\GAMES\\PlayStation_4\\meu\\meuport\\games\\UFC4\\python\\python.exe")) {
        wcsncpy(outPath, L"D:\\GAMES\\PlayStation_4\\meu\\meuport\\games\\UFC4\\python\\python.exe", maxLen);
        return TRUE;
    }

    _snwprintf(p, MAX_PATH, L"%s\\python\\python.exe", cand);
    if (PathFileExistsW(p)) { wcsncpy(outPath, p, maxLen); return TRUE; }

    wchar_t parent[MAX_PATH];
    wcsncpy(parent, cand, MAX_PATH);
    PathRemoveFileSpecW(parent);
    _snwprintf(p, MAX_PATH, L"%s\\games\\UFC4\\python\\python.exe", parent);
    if (PathFileExistsW(p)) { wcsncpy(outPath, p, maxLen); return TRUE; }

    if (SearchPathW(NULL, L"python.exe", NULL, (DWORD)maxLen, outPath, NULL)) return TRUE;

    return FALSE;
}

static BOOL FindRunnerScript(wchar_t *outPath, size_t maxLen)
{
    wchar_t cand[MAX_PATH];
    GetModuleFileNameW(NULL, cand, MAX_PATH);
    PathRemoveFileSpecW(cand);

    wchar_t p[MAX_PATH];
    _snwprintf(p, MAX_PATH, L"%s\\tools\\pkg_tool\\extract_runner.py", cand);
    if (PathFileExistsW(p)) { wcsncpy(outPath, p, maxLen); return TRUE; }

    if (PathFileExistsW(L"D:\\GAMES\\PlayStation_4\\meu\\meuport\\tools\\pkg_tool\\extract_runner.py")) {
        wcsncpy(outPath, L"D:\\GAMES\\PlayStation_4\\meu\\meuport\\tools\\pkg_tool\\extract_runner.py", maxLen);
        return TRUE;
    }

    _snwprintf(p, MAX_PATH, L"%s\\extract_runner.py", cand);
    if (PathFileExistsW(p)) { wcsncpy(outPath, p, maxLen); return TRUE; }

    return FALSE;
}

/* Full PKG Extraction routine */
BOOL PkgExtractFull(const wchar_t *pkgPath, const wchar_t *destDir, volatile BOOL *cancelFlag, PkgProgressCallback cb, void *userParam)
{
    if (!pkgPath || !destDir) return FALSE;

    SHCreateDirectoryExW(NULL, destDir, NULL);

    /* 1. Extract metadata items first (SFO and icons) */
    if (cb) cb(5, 0, 100, L"Extraindo metadados (PARAM.SFO)...", userParam);
    wchar_t sfoDir[MAX_PATH], sfoFile[MAX_PATH];
    _snwprintf(sfoDir, MAX_PATH, L"%s\\sce_sys", destDir);
    SHCreateDirectoryExW(NULL, sfoDir, NULL);
    _snwprintf(sfoFile, MAX_PATH, L"%s\\sce_sys\\param.sfo", destDir);
    PkgExtractEntry(pkgPath, PKG_ENTRY_PARAM_SFO, sfoFile);

    if (cancelFlag && *cancelFlag) return FALSE;

    if (cb) cb(8, 0, 100, L"Extraindo ícone (icon0.png)...", userParam);
    wchar_t iconFile[MAX_PATH];
    _snwprintf(iconFile, MAX_PATH, L"%s\\sce_sys\\icon0.png", destDir);
    PkgExtractEntry(pkgPath, PKG_ENTRY_ICON0_PNG, iconFile);

    if (cancelFlag && *cancelFlag) return FALSE;

    /* 2. Run extraction through extract_runner.py (using embedded python or system python) */
    wchar_t pyExe[MAX_PATH] = L"";
    wchar_t scriptPath[MAX_PATH] = L"";
    BOOL havePy = FindPythonExe(pyExe, MAX_PATH);
    BOOL haveScript = FindRunnerScript(scriptPath, MAX_PATH);

    if (havePy && haveScript) {
        if (cb) cb(10, 0, 100, L"Iniciando motor de descompactação PFS/PKG...", userParam);

        wchar_t cmd[MAX_PATH * 4];
        _snwprintf(cmd, sizeof(cmd)/sizeof(wchar_t), L"\"%s\" \"%s\" \"%s\" \"%s\"",
                   pyExe, scriptPath, pkgPath, destDir);

        SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE};
        HANDLE hRead = NULL, hWrite = NULL;
        if (CreatePipe(&hRead, &hWrite, &sa, 0)) {
            SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

            STARTUPINFOW si = {sizeof(si)};
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
            si.hStdOutput = hWrite;
            si.hStdError = hWrite;
            PROCESS_INFORMATION pi = {0};

            if (CreateProcessW(pyExe, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, destDir, &si, &pi)) {
                CloseHandle(hWrite);

                char pipeBuf[1024];
                DWORD readBytes = 0;
                char lineAccum[2048] = "";
                size_t lineLen = 0;
                int currentPct = 10;

                while (ReadFile(hRead, pipeBuf, sizeof(pipeBuf) - 1, &readBytes, NULL) && readBytes > 0) {
                    if (cancelFlag && *cancelFlag) {
                        TerminateProcess(pi.hProcess, 1);
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                        CloseHandle(hRead);
                        return FALSE;
                    }

                    pipeBuf[readBytes] = 0;
                    for (DWORD i = 0; i < readBytes; i++) {
                        char ch = pipeBuf[i];
                        if (ch == '\r' || ch == '\n') {
                            if (lineLen > 0) {
                                lineAccum[lineLen] = 0;
                                if (strncmp(lineAccum, "PROGRESS:", 9) == 0) {
                                    int p = 0;
                                    char *dash = strchr(lineAccum, '-');
                                    if (sscanf(lineAccum + 9, "%d%%", &p) == 1) {
                                        currentPct = p;
                                    }
                                    const char *statusPart = dash ? (dash + 2) : (lineAccum + 9);
                                    wchar_t wStatus[256];
                                    MultiByteToWideChar(CP_UTF8, 0, statusPart, -1, wStatus, 255);
                                    wStatus[255] = 0;
                                    if (cb) cb(currentPct, 0, 100, wStatus, userParam);
                                }
                                lineLen = 0;
                            }
                        } else if (lineLen < sizeof(lineAccum) - 1) {
                            lineAccum[lineLen++] = ch;
                        }
                    }
                }

                WaitForSingleObject(pi.hProcess, 5000);
                DWORD exitCode = 1;
                GetExitCodeProcess(pi.hProcess, &exitCode);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                CloseHandle(hRead);

                if (exitCode != 0) {
                    if (cb) cb(currentPct, 0, 100, L"Falha na extração dos arquivos do jogo.", userParam);
                    return FALSE;
                }
            } else {
                CloseHandle(hWrite);
                CloseHandle(hRead);
                return FALSE;
            }
        }
    } else {
        if (cb) cb(50, 0, 100, L"Gravando arquivos do pacote no destino...", userParam);
    }

    /* Verification: ensure eboot.bin exists in destDir or subdirectories */
    wchar_t checkEboot[MAX_PATH];
    _snwprintf(checkEboot, MAX_PATH, L"%s\\eboot.bin", destDir);
    if (!PathFileExistsW(checkEboot)) {
        /* Check if inside a subdirectory and flatten up */
        WIN32_FIND_DATAW fd;
        wchar_t searchPat[MAX_PATH];
        _snwprintf(searchPat, MAX_PATH, L"%s\\*", destDir);
        HANDLE hFind = FindFirstFileW(searchPat, &fd);
        BOOL foundSub = FALSE;
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && fd.cFileName[0] != L'.') {
                    wchar_t subEboot[MAX_PATH];
                    _snwprintf(subEboot, MAX_PATH, L"%s\\%s\\eboot.bin", destDir, fd.cFileName);
                    if (PathFileExistsW(subEboot)) {
                        foundSub = TRUE;
                        break;
                    }
                }
            } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
        }
        if (!foundSub) {
            if (cb) cb(100, 0, 100, L"Erro: eboot.bin não foi encontrado no destino.", userParam);
            return FALSE;
        }
    }

    if (cb) cb(100, 100, 100, L"Extração concluída com sucesso!", userParam);
    return TRUE;
}
