#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
extract_runner.py - PS4 PKG Extraction and Restructuring Engine for FightPort 4
Integrates OrbisPkgTool with automated bundle extraction, stream monitoring,
and automatic restructuring into the SacredFire / UFC4 game folder layout.
"""

import os
import sys
import shutil
import struct
import subprocess
import time
from pathlib import Path

def log(msg):
    sys.stdout.write(msg + "\n")
    sys.stdout.flush()

def find_dotnet():
    # 1. Check PATH
    which = shutil.which("dotnet")
    if which and os.path.isfile(which):
        return which

    # 2. Check standard Windows installations
    candidates = [
        r"C:\Program Files\dotnet\dotnet.exe",
        r"C:\Program Files (x86)\dotnet\dotnet.exe",
        os.path.join(os.environ.get("LOCALAPPDATA", ""), r"Microsoft\dotnet\dotnet.exe"),
    ]
    for c in candidates:
        if c and os.path.isfile(c):
            return c

    # 3. Check registry if on Windows
    try:
        import winreg
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\dotnet\Setup\InstalledVersions\x64\sharedHost")
        val, _ = winreg.QueryValueEx(key, "Path")
        winreg.CloseKey(key)
        if val and os.path.isfile(val):
            return val
    except Exception:
        pass

    return None

def extract_bundle_files_if_needed(tool_dir, pkg_tool_exe):
    orbis_dll = os.path.join(tool_dir, "OrbisPkgTool.dll")
    if os.path.isfile(orbis_dll):
        return True

    if not pkg_tool_exe or not os.path.isfile(pkg_tool_exe):
        return False

    log("Extraindo ferramentas internas do PS4 PKG Tool...")
    try:
        with open(pkg_tool_exe, "rb") as f:
            data = f.read()

        # Find .NET 6 SingleFile bundle manifest
        bid_pos = data.rfind(b"xvI9QfIBQ1Vg")
        if bid_pos == -1:
            return False

        pos = bid_pos + 12
        deps_offset, deps_size = struct.unpack("<qq", data[pos:pos+16])
        pos += 16
        cfg_offset, cfg_size = struct.unpack("<qq", data[pos:pos+16])
        pos += 16
        flags = struct.unpack("<Q", data[pos:pos+8])[0]
        pos += 8

        # 285 entries
        count = 285
        os.makedirs(tool_dir, exist_ok=True)
        for _ in range(count):
            offset, size, comp = struct.unpack("<qqq", data[pos:pos+24])
            pos += 24
            type_b = data[pos]
            pos += 1
            name_len = 0
            shift = 0
            while True:
                b = data[pos]
                pos += 1
                name_len |= (b & 0x7f) << shift
                if (b & 0x80) == 0:
                    break
                shift += 7
            name = data[pos:pos+name_len].decode("utf-8")
            pos += name_len

            # Extract OrbisPkgTool and dependencies
            if any(name.startswith(p) for p in [
                "OrbisPkgTool", "ICSharpCode.SharpZipLib", "DotNetZip",
                "K4os.Compression.LZ4", "Newtonsoft.Json", "PS4PKGTool.Core", "PS4PKGTool.Assets"
            ]):
                out_path = os.path.join(tool_dir, name)
                os.makedirs(os.path.dirname(out_path), exist_ok=True)
                with open(out_path, "wb") as out_f:
                    out_f.write(data[offset:offset+size])

        # Write runtimeconfig
        runtime_cfg = {
            "runtimeOptions": {
                "tfm": "net10.0",
                "frameworks": [
                    {"name": "Microsoft.NETCore.App", "version": "10.0.10"},
                    {"name": "Microsoft.WindowsDesktop.App", "version": "10.0.10"}
                ],
                "rollForward": "LatestMajor"
            }
        }
        import json
        with open(os.path.join(tool_dir, "OrbisPkgTool.runtimeconfig.json"), "w") as cfg_f:
            json.dump(runtime_cfg, cfg_f, indent=2)

        return os.path.isfile(orbis_dll)
    except Exception as e:
        log(f"Erro ao descompactar bundle: {e}")
        return False

def merge_tree(src, dst):
    """Recursively moves or copies all items from src to dst, replacing existing files."""
    os.makedirs(dst, exist_ok=True)
    for root, dirs, files in os.walk(src):
        rel = os.path.relpath(root, src)
        target_dir = dst if rel == "." else os.path.join(dst, rel)
        os.makedirs(target_dir, exist_ok=True)
        for f in files:
            s_file = os.path.join(root, f)
            d_file = os.path.join(target_dir, f)
            try:
                if os.path.exists(d_file):
                    os.remove(d_file)
                shutil.move(s_file, d_file)
            except Exception:
                try:
                    shutil.copy2(s_file, d_file)
                    os.remove(s_file)
                except Exception:
                    pass

def main():
    if len(sys.argv) < 3:
        log("Uso: extract_runner.py <caminho_pkg> <pasta_destino>")
        sys.exit(1)

    pkg_path = os.path.abspath(sys.argv[1])
    dest_dir = os.path.abspath(sys.argv[2])

    if not os.path.isfile(pkg_path):
        log(f"ERROR: Arquivo PKG não encontrado: {pkg_path}")
        sys.exit(1)

    os.makedirs(dest_dir, exist_ok=True)

    # Base script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    tool_dir = script_dir

    # Search for PS4 PKG Tool.exe in common locations
    pkg_tool_exe = None
    search_dirs = [
        os.getcwd(),
        os.path.dirname(os.getcwd()),
        r"D:\GAMES\PlayStation_4\meu\meuport",
        r"D:\GAMES\PlayStation_4",
        r"D:\GAMES\PlayStation_4\UFC_4",
    ]
    for d in search_dirs:
        cand = os.path.join(d, "PS4 PKG Tool.exe")
        if os.path.isfile(cand):
            pkg_tool_exe = cand
            break

    # Ensure OrbisPkgTool is available
    if not extract_bundle_files_if_needed(tool_dir, pkg_tool_exe):
        orbis_dll = os.path.join(tool_dir, "OrbisPkgTool.dll")
        if not os.path.isfile(orbis_dll):
            log("ERROR: Não foi possível localizar ou extrair o OrbisPkgTool.dll.")
            sys.exit(1)

    orbis_dll = os.path.join(tool_dir, "OrbisPkgTool.dll")

    # Locate dotnet.exe
    dotnet_exe = find_dotnet()
    if not dotnet_exe:
        log("ERROR: O .NET Runtime (dotnet.exe) não foi encontrado no sistema.")
        sys.exit(1)

    log(f"PROGRESS: 5% - Iniciando extração com OrbisPkgTool...")

    # Staging directory on the same volume for instant atomic moves
    stage_dir = os.path.join(dest_dir, "_stage_pkg_extract")
    if os.path.exists(stage_dir):
        try:
            shutil.rmtree(stage_dir)
        except Exception:
            pass
    os.makedirs(stage_dir, exist_ok=True)

    cmd = [
        dotnet_exe,
        orbis_dll,
        "extract",
        "--verbose",
        pkg_path,
        stage_dir
    ]

    log(f"PROGRESS: 10% - Descompactando arquivos do PKG (aguarde)...")

    # Launch process and stream stdout
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        bufsize=1
    )

    pct = 10
    last_update = time.time()
    file_count = 0

    while True:
        line = proc.stdout.readline()
        if not line and proc.poll() is not None:
            break

        line_clean = line.strip()
        if line_clean:
            file_count += 1
            now = time.time()
            if now - last_update >= 0.3:
                # Progressively advance up to 90% during extraction
                if pct < 90:
                    pct += 1
                disp_line = line_clean
                if len(disp_line) > 60:
                    disp_line = "..." + disp_line[-57:]
                log(f"PROGRESS: {pct}% - {disp_line}")
                last_update = now

    ret_code = proc.wait()
    if ret_code != 0:
        log(f"ERROR: Falha na extração do pacote (código de erro: {ret_code})")
        try:
            shutil.rmtree(stage_dir)
        except Exception:
            pass
        sys.exit(ret_code)

    log("PROGRESS: 92% - Reestruturando diretórios e mesclando arquivos...")

    # Restructure from stage_dir to dest_dir:
    # 1. Image0 contains eboot.bin, game files and assets
    image0_dir = os.path.join(stage_dir, "Image0")
    if os.path.isdir(image0_dir):
        merge_tree(image0_dir, dest_dir)

    # 2. Sc0 contains param.sfo, icon0.png, etc. -> into sce_sys
    sc0_dir = os.path.join(stage_dir, "Sc0")
    if os.path.isdir(sc0_dir):
        sce_sys_target = os.path.join(dest_dir, "sce_sys")
        merge_tree(sc0_dir, sce_sys_target)

    # 3. Any other remaining root files in stage_dir
    for item in os.listdir(stage_dir):
        if item in ("Image0", "Sc0"):
            continue
        item_path = os.path.join(stage_dir, item)
        target_path = os.path.join(dest_dir, item)
        if os.path.isdir(item_path):
            merge_tree(item_path, target_path)
        else:
            try:
                if os.path.exists(target_path):
                    os.remove(target_path)
                shutil.move(item_path, target_path)
            except Exception:
                pass

    # Cleanup stage_dir
    try:
        shutil.rmtree(stage_dir)
    except Exception:
        pass

    log("PROGRESS: 96% - Validando layout do jogo...")

    # 4. Check eboot.bin in dest_dir
    eboot_path = os.path.join(dest_dir, "eboot.bin")
    if not os.path.isfile(eboot_path):
        # Look for eboot.bin inside subdirectories up to 3 levels deep
        for root, dirs, files in os.walk(dest_dir):
            if "eboot.bin" in [f.lower() for f in files]:
                actual_eboot = os.path.join(root, "eboot.bin")
                if os.path.isfile(actual_eboot) and root != dest_dir:
                    log(f"Reorganizando pasta interna encontrada: {root} -> {dest_dir}")
                    merge_tree(root, dest_dir)
                    break

    # Final verification
    if not os.path.isfile(os.path.join(dest_dir, "eboot.bin")):
        log("ERROR: eboot.bin não encontrado na pasta de destino após a extração!")
        sys.exit(2)

    log("PROGRESS: 100% - Extração concluída com sucesso!")
    log("DONE: SUCCESS")
    sys.exit(0)

if __name__ == "__main__":
    main()
