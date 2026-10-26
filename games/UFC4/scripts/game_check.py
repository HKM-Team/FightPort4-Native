#!/usr/bin/env python3
"""The game files this port runs, keyed on the executable's loaded image.

Profile (ufc4 fork): EA SPORTS UFC 4 CUSA14209 with the 28.00 update merged in. The check
compares the loaded executable image, whose hash is the same whatever tool dumped it (the SELF
headers around it differ). BB_SKIP_GAME_CHECK=1 skips it. bbport's Bloodborne profile was removed
on 2026-10-09.

Two builds of the 1.21 update exist: the one rebuilt for firmware 5.05 (sdk_ver=05050000 in
param.sfo; the image in SUPPORTED) and the original one for firmware 6.72+ (KNOWN_672 holds its
image hash). Same game, different executable. A 1.21 executable that is not a known old one is
accepted as `variant='6.72+'`: patches.py finds its patch sites by signature instead of address,
and the address-keyed diagnostics (which know the 5.05 build's addresses) stay off.
"""
import hashlib
import os
import sys
from pathlib import Path

# image sha256 -> profile. `name` is what the runtime and the GPU menu read (BB_GAME_PROFILE);
# `modules` are the sce_module PRXs link_modules.py links natively; `patches` says whether
# patches.py has a patch file for it.
# ufc4: EA SPORTS UFC 4 CUSA14209 with update 28.00 merged in (the dump this port was brought up
# on). Other UFC 4 executables of the same title are accepted as variant 'other' (see profile()).
SUPPORTED = {
    '73862ae3556543cd3fdc2c95ed46a608ce26f9a75b44f15f7f93809a68372602': dict(
        name='ufc4', title='CUSA14209', version='28.00', game='EA SPORTS UFC 4', update='CUSA14209-patch',
        modules=('libc.prx', 'libSceFios2.prx'), patches=False),
    # ufc1: EA SPORTS UFC (2014) CUSA00222 with the 1.08 update merged in: a fixed-address
    # executable (ET_SCE_EXEC at 0x400000; prepare.py and the loader place it there).
    'e4fe67c5cc644390f61d9eb2095b4bb650c4f97ae3f74208ccce6cd5e46093e2': dict(
        name='ufc1', title='CUSA00222', version='01.08', game='EA SPORTS UFC', update='CUSA00222-patch',
        modules=('libc.prx', 'libSceFios2.prx'), patches=False),
}
BY_TITLE = {p['title']: p for p in SUPPORTED.values()}
# ufc4: the other UFC 4 title, CUSA14204 (America, also sold in Japan): the same game in a
# different executable. Accepted with any image (variant '6.72+': nothing keyed on the reference
# image's addresses runs; patches.py finds the patch site by signature).
for _title in ('CUSA14204',):
    BY_TITLE[_title] = dict(BY_TITLE['CUSA14209'], title=_title, update=f'{_title}-patch')
OLD_IMAGES = {}
KNOWN_672 = set()
SUPPORTED_TITLE = 'CUSA14209'
SUPPORTED_VERSION = '28.00'
SUPPORTED_IMAGE = '73862ae3556543cd3fdc2c95ed46a608ce26f9a75b44f15f7f93809a68372602'


def image_sha256(game):
    """The SHA-256 of eboot.bin's loaded image (its loadable segments at their addresses)."""
    from prepare import parse_self, span  # same directory
    elf, header, ph, _segments, _missing = parse_self((Path(game) / 'eboot.bin').read_bytes())
    loads = [p for p in ph if p['type'] in (1, 0x61000010)]
    image = bytearray(max(p['vaddr'] + p['memsz'] for p in loads))
    for p in loads:
        image[p['vaddr']:p['vaddr'] + p['filesz']] = span(elf, p['offset'], p['filesz'])
    return hashlib.sha256(image).hexdigest()


def profile(game, image_hash=None):
    """The profile dict for a supported game folder, else None (BB_SKIP_GAME_CHECK=1: a profile
    guessed from param.sfo's TITLE_ID when one exists, so run.sh can still branch)."""
    try:
        image_hash = image_hash or image_sha256(game)
    except (OSError, ValueError, IndexError, StopIteration, KeyError):
        image_hash = None
    if image_hash in SUPPORTED:
        return dict(SUPPORTED[image_hash], variant='5.05')
    if image_hash and problem(game, image_hash) is None and os.environ.get('BB_SKIP_GAME_CHECK') != '1':
        info = sfo_info(game)
        return dict(BY_TITLE[info['TITLE_ID']], variant='6.72+', known=image_hash in KNOWN_672)
    if os.environ.get('BB_SKIP_GAME_CHECK') != '1':
        return None
    info = sfo_info(game)
    found = BY_TITLE.get(info.get('TITLE_ID'))
    return dict(found, variant='?') if found else None


def _sce_module(folder):
    """folder's sce_module when it holds libc.prx (names in any letter case), else None."""
    try:
        for entry in Path(folder).iterdir():
            if entry.name.lower() == 'sce_module' and entry.is_dir():
                if any(f.name.lower() == 'libc.prx' and f.is_file() for f in entry.iterdir()):
                    return entry
    except OSError:
        pass
    return None


def module_dir(game):
    """The sce_module folder whose PRXs (libc.prx, libSceFios2.prx) this game links: the game
    folder's own, else, for an update dumped without them (the console takes the base game's
    then), the one of the base game beside it: a folder of the same TITLE_ID next to the game
    folder. None when there is none."""
    game = Path(game)
    own = _sce_module(game)
    if own:
        return own
    title = sfo_info(game).get('TITLE_ID')
    if not title:
        return None
    try:
        siblings = sorted(p for p in game.resolve().parent.iterdir() if p.is_dir() and p.resolve() != game.resolve())
    except OSError:
        return None
    for sibling in siblings:
        if sfo_info(sibling).get('TITLE_ID') == title and (found := _sce_module(sibling)):
            return found
    return None


def module_file(game, name):
    """sce_module/<name> of module_dir(game) (the name in any letter case)."""
    folder = module_dir(game)
    if folder is None:
        raise FileNotFoundError(f"{Path(game) / 'sce_module' / name}: {explain('no_modules', *_title_version(game))}")
    for entry in folder.iterdir():
        if entry.name.lower() == name.lower():
            return entry
    return folder / name


def _title_version(game):
    info = sfo_info(game)
    return info.get('TITLE_ID', '?'), info.get('APP_VER', '?')


def sfo_info(game):
    from prepare import sfo
    try:
        return sfo((Path(game) / 'sce_sys/param.sfo').read_bytes())
    except (OSError, ValueError, IndexError):
        return {}


def problem(game, image_hash=None):
    """None for a supported game, else (kind, title, version): kind is 'missing_update' (base
    game or an older update), 'wrong_eboot' (param.sfo names the supported update, eboot.bin is
    another version), 'other_title' (another edition, region or game), 'unreadable' or
    'no_modules' (no sce_module/libc.prx in the folder or in its base game's beside it)."""
    if os.environ.get('BB_SKIP_GAME_CHECK') == '1':
        return None
    from prepare import sfo
    try:
        info = sfo((Path(game) / 'sce_sys/param.sfo').read_bytes())
    except (OSError, ValueError, IndexError):
        info = {}
    title, version = info.get('TITLE_ID', '?'), info.get('APP_VER', '?')
    try:
        image_hash = image_hash or image_sha256(game)
    except (OSError, ValueError, IndexError, StopIteration, KeyError):
        return 'unreadable', title, version
    if image_hash not in SUPPORTED:
        if title not in BY_TITLE:
            return 'other_title', title, version
        if version in ('01.00', '1.00') and BY_TITLE[title]['version'] != version:
            return 'missing_update', title, version
        if image_hash in OLD_IMAGES:
            return 'wrong_eboot', title, version
        # else another build of a supported title (6.72+): accepted, see the module docstring
    if module_dir(game) is None:
        return 'no_modules', title, version
    return None


def explain(kind, title, version):
    """What is wrong and what to do, for the log (English)."""
    found = f'Found {title} version {version} (sce_sys/param.sfo).'
    p = BY_TITLE.get(title, BY_TITLE[SUPPORTED_TITLE])
    supported = ', '.join(f"{q['game']} {q['title']} {q['version']}" for q in SUPPORTED.values())
    return {
        'missing_update': f"{found} The port needs the {p['version']} update merged into the game folder: copy "
                          f"everything from the dumped {p['version']} update (e.g. {p['update']}) into the "
                          'game folder, replacing files (eboot.bin and sce_sys too).',
        'wrong_eboot': f"{found} param.sfo is from {p['version']} but eboot.bin is not: copy eboot.bin from "
                       f"the dumped {p['version']} update into the game folder, replacing the old one.",
        'other_title': f'{found} Supported: {supported}, and EA SPORTS UFC 4 CUSA14204 28.00; this folder is another game.',
        'unreadable': f'{found} eboot.bin could not be read as a decrypted PS4 executable: dump '
                      'the game and its update again.',
        'no_modules': f"{found} The game folder has eboot.bin but no sce_module folder with libc.prx (the "
                      "game's own system modules), so the dump is incomplete. Put the base game folder and "
                      f"its {p['version']} update folder side by side and choose either one (they are merged "
                      "automatically), or copy the sce_module folder from the base game into this folder.",
    }[kind]


if __name__ == '__main__':
    # game_check.py GAME_DIR: prints the profile name (run.sh branches on it), or fails with the
    # reason. With --variant: the update build instead ("5.05", "6.72+", or "?" when the check
    # is skipped), for BB_GAME_VARIANT.
    args = [a for a in sys.argv[1:] if a != '--variant']
    game = Path(args[0]) if args else Path('.')
    found = problem(game)
    if found:
        sys.exit('Unsupported game files: ' + explain(*found))
    p = profile(game)
    if '--variant' in sys.argv:
        print(p.get('variant', '?') if p else '?')
    else:
        print(p['name'] if p else 'unknown')
