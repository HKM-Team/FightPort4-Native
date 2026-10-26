#!/usr/bin/env python3
"""Merge a PS4 update folder over its base game folder, as the console does.

PKG extractors usually leave the base game and its update side by side:
    <dir>/CUSA06534          (base 1.00)
    <dir>/CUSA06534-patch    (update 1.14; also -PATCH, -update, -UPDATE)
This makes <dir>/CUSA06534-merged next to them: every base file hard-linked (no extra disk
space; the base folder is never written), then every update file linked over it. On a volume
without hard links the files are copied. A marker records the source folders and sizes, so later
launches reuse the merged folder. Prints the folder to play: the merged one, or the given
folder unchanged when it has no update beside it (already merged).

ufc4: the folder given may also be one that holds the dump (an extractor's output folder,
"Image0", a folder of games): the game folder inside it (3 levels at most) is played, a base
game before an update. Base and update are paired by name (CUSA14209 or CUSA14209-app with
CUSA14209-patch, ...) or, when the names say nothing ("UFC 4" and "Nouveau dossier"), by
param.sfo: the same TITLE_ID, CATEGORY gd (a game) and gp (its update).
"""
from __future__ import annotations

import json
import os
import shutil
import struct
import sys
from pathlib import Path

SUFFIXES = ('-patch', '-update', '_patch', '_update', ' patch', ' update')
BASE_SUFFIXES = ('-app', '_app', ' app', '-base', '_base', ' base')
MARKER = '.merged_update.json'
# Game content folders: never searched for a game folder inside them.
CONTENT = {'sce_module', 'sce_sys', 'dvdroot_ps4', 'databigs', '$recycle.bin', 'system volume information'}


def is_game(folder: Path) -> bool:
    return (folder / 'eboot.bin').is_file()


def is_merged(folder: Path) -> bool:
    return folder.name.lower().endswith('-merged') or (folder / MARKER).is_file()


def sfo(folder: Path) -> dict:
    """param.sfo's TITLE_ID, CATEGORY and APP_VER ({} when unreadable)."""
    try:
        data = (folder / 'sce_sys' / 'param.sfo').read_bytes()
        magic, _version, keys, values, count = struct.unpack_from('<4sIIII', data, 0)
        if magic != b'\0PSF':
            return {}
        result = {}
        for i in range(count):
            key, fmt, size, _capacity, off = struct.unpack_from('<HHIII', data, 20 + i * 16)
            name = data[keys + key:data.index(0, keys + key)].decode('ascii', 'replace')
            if name in ('TITLE_ID', 'CATEGORY', 'APP_VER') and fmt != 0x404:
                result[name] = data[values + off:values + off + size].rstrip(b'\0').decode('utf-8', 'replace')
        return result
    except (OSError, ValueError, struct.error):
        return {}


def strip(name: str, suffixes) -> str | None:
    low = name.lower()
    for suffix in suffixes:
        if low.endswith(suffix):
            return name[:-len(suffix)]
    return None


def is_update(folder: Path) -> bool:
    return strip(folder.name, SUFFIXES) is not None or sfo(folder).get('CATEGORY') == 'gp'


def neighbours(folder: Path):
    """The other game folders beside this one (merged folders made here are not)."""
    try:
        entries = sorted(folder.parent.iterdir()) if folder.parent.is_dir() else []
    except OSError:
        return []
    return [e for e in entries if e.is_dir() and e != folder and is_game(e) and not is_merged(e)]


def split(folder: Path):
    """(base, update) for a base or an update folder given by the user; update may be None."""
    if is_merged(folder):
        return folder, None
    others = neighbours(folder)
    by_name = {e.name.lower(): e for e in others}
    stem = strip(folder.name, SUFFIXES)
    if stem is not None:  # an update folder: its base is beside it (CUSA14209, CUSA14209-app, ...)
        for suffix in ('',) + BASE_SUFFIXES:
            if (stem + suffix).lower() in by_name:
                return by_name[(stem + suffix).lower()], folder
    else:
        stem = strip(folder.name, BASE_SUFFIXES) or folder.name
        for suffix in SUFFIXES:
            if (stem + suffix).lower() in by_name:
                return folder, by_name[(stem + suffix).lower()]
    # Names that say nothing: param.sfo pairs a game (CATEGORY gd) with its update (gp).
    own = sfo(folder)
    title = own.get('TITLE_ID')
    if title:
        same = [(e, info) for e in others if (info := sfo(e)).get('TITLE_ID') == title]
        if own.get('CATEGORY') == 'gp':
            bases = [e for e, info in same if info.get('CATEGORY') == 'gd']
            if len(bases) == 1:
                return bases[0], folder
        elif own.get('CATEGORY') == 'gd':
            updates = sorted(((info.get('APP_VER', ''), e) for e, info in same if info.get('CATEGORY') == 'gp'),
                             key=lambda u: u[0], reverse=True)
            if updates:
                return folder, updates[0][1]
    return folder, None


def resolve(folder: Path) -> Path | None:
    """The game folder to play for the folder the user chose: itself, or the game folder inside
    it (3 levels at most; a base game before an update, the shallowest first). None: none."""
    if is_game(folder):
        return folder
    found = []
    for dirpath, dirnames, filenames in os.walk(folder):
        here = Path(dirpath)
        if any(n.lower() == 'eboot.bin' for n in filenames) and here != folder:
            found.append(here)
            dirnames[:] = []
            continue
        depth = len(here.relative_to(folder).parts)
        dirnames[:] = [] if depth >= 3 else sorted(d for d in dirnames if d.lower() not in CONTENT)
    found = [p for p in found if is_game(p)]
    if not found:
        return None
    return min(found, key=lambda p: (is_merged(p), is_update(p), len(p.parts), str(p).lower()))


def files(root: Path):
    for dirpath, _, names in os.walk(root):
        for n in names:
            p = Path(dirpath) / n
            yield p.relative_to(root), p


def signature(base: Path, update: Path):
    total = lambda root: sum(p.stat().st_size for _, p in files(root))
    return {'base': str(base.resolve()), 'update': str(update.resolve()),
            'base_bytes': total(base), 'update_bytes': total(update)}


def place(source: Path, target: Path, stats):
    """target becomes a link to (or a copy of) source; an existing target is replaced, never
    written through (it may be a link to a base file)."""
    target.parent.mkdir(parents=True, exist_ok=True)
    temp = target.with_name(target.name + '.merge-tmp')
    if temp.exists():
        temp.unlink()
    try:
        os.link(source, temp)
        stats['linked'] += 1
    except OSError:
        shutil.copy2(source, temp)
        stats['copied'] += 1
    os.replace(temp, target)


def merge(base: Path, update: Path) -> Path:
    merged = base.with_name(base.name + '-merged')
    sig = signature(base, update)
    marker = merged / MARKER
    try:
        if json.loads(marker.read_text(encoding='utf-8')) == sig:
            return merged
    except (OSError, ValueError):
        pass
    if merged.exists():
        shutil.rmtree(merged)
    stats = {'linked': 0, 'copied': 0}
    for rel, path in files(base):
        place(path, merged / rel, stats)
    for rel, path in files(update):
        place(path, merged / rel, stats)
    marker.write_text(json.dumps(sig, indent=1), encoding='utf-8')
    print(f'merge_update: {update.name} over {base.name} -> {merged} '
          f'({stats["linked"]} hard links, {stats["copied"]} copies)', file=sys.stderr)
    if stats['copied'] and not stats['linked']:
        print('merge_update: this drive has no hard links, so the game was copied '
              '(it uses as much space again)', file=sys.stderr)
    return merged


def main():
    if len(sys.argv) != 2:
        raise SystemExit('usage: merge_update.py <game folder>')
    chosen = Path(sys.argv[1])
    folder = resolve(chosen)
    if folder is None:
        raise SystemExit(f'No eboot.bin in {chosen} or in the folders inside it: choose the folder of the '
                         'game dump (the one with eboot.bin, sce_sys and sce_module).')
    if folder != chosen:
        print(f'merge_update: the game folder inside {chosen} is {folder}', file=sys.stderr)
    base, update = split(folder)
    print(merge(base, update) if update else folder)


if __name__ == '__main__':
    main()
