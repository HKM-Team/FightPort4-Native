#!/usr/bin/env bash
# FightPort 4 for Linux and Steam Deck.
# Runs the Windows launcher (FightPort4.exe) and the ports it starts through Proton (umu-launcher) or Wine.
#
#   bash fightport4.sh             open FightPort 4
#   bash fightport4.sh --install   add FightPort 4 to the applications menu
#   bash fightport4.sh --winecfg   open the Wine settings of FightPort 4's prefix
#
# Game folders on Linux appear in FightPort 4 as drive Z: (for example Z:\home\you\Games\UFC 4).
# Environment: WINEPREFIX (default ~/.local/share/FightPort4/prefix), PROTONPATH (default GE-Proton, for umu-run),
# FIGHTPORT4_WINE=1 to use Wine even when umu-run is installed.
set -u

HERE="$(cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")" && pwd)"
DATA="${XDG_DATA_HOME:-$HOME/.local/share}/FightPort4"
export WINEPREFIX="${WINEPREFIX:-$DATA/prefix}"
export WINEDEBUG="${WINEDEBUG:--all}"
export WINEDLLOVERRIDES="${WINEDLLOVERRIDES:-winemenubuilder.exe=d}"
mkdir -p "$DATA"

say() {
    if [ -n "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ] && command -v zenity >/dev/null 2>&1; then
        zenity --info --title="FightPort 4" --width=420 --text="$1" 2>/dev/null || echo "$1"
    else
        echo "$1"
    fi
}

if [ ! -f "$HERE/FightPort4.exe" ]; then
    say "FightPort4.exe não está ao lado deste script ($HERE). Extraia o pacote inteiro e rode o fightport4.sh de dentro dele."
    exit 1
fi

case "${1:-}" in
--install)
    APPS="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
    mkdir -p "$APPS"
    cat > "$APPS/fightport4.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=FightPort 4
Comment=Port para PC do EA SPORTS UFC 4
Exec=bash "$HERE/fightport4.sh"
Path=$HERE
Icon=applications-games
Terminal=false
Categories=Game;
EOF
    chmod +x "$HERE/fightport4.sh" 2>/dev/null
    say "FightPort 4 foi adicionado ao menu de aplicativos."
    exit 0
    ;;
--help | -h)
    sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'
    exit 0
    ;;
esac

cd "$HERE" || exit 1

if [ -z "${FIGHTPORT4_WINE:-}" ] && command -v umu-run >/dev/null 2>&1; then
    # Proton through umu-launcher: the same runtime Steam uses, with its Vulkan and controller fixes
    export GAMEID="${GAMEID:-umu-default}" STORE="${STORE:-none}" PROTONPATH="${PROTONPATH:-GE-Proton}"
    [ "${1:-}" = "--winecfg" ] && exec umu-run winecfg
    exec umu-run "$HERE/FightPort4.exe" "$@"
fi

if command -v wine >/dev/null 2>&1; then
    major="$(wine --version 2>/dev/null | sed -E 's/^wine-([0-9]+).*/\1/')"
    if [ -n "$major" ] && [ "$major" -lt 9 ] 2>/dev/null; then
        say "Seu Wine é a versão $major. O FightPort 4 precisa do Wine 9 ou mais novo (ou do umu-launcher, que usa o Proton)."
    fi
    if [ ! -d "$WINEPREFIX/drive_c" ]; then
        echo "FightPort 4: criando o prefixo do Wine em $WINEPREFIX (só na primeira vez)..."
        WINEARCH=win64 wineboot -u >/dev/null 2>&1
    fi
    [ "${1:-}" = "--winecfg" ] && exec winecfg
    exec wine "$HERE/FightPort4.exe" "$@"
fi

say "Instale o Wine 9 ou mais novo, ou o umu-launcher (Proton):
  Ubuntu/Debian: sudo apt install wine64
  Fedora: sudo dnf install wine
  Arch: sudo pacman -S wine   (ou: umu-launcher)
No Steam Deck, adicione o FightPort4.exe como jogo de fora da Steam e escolha o Proton (GE-Proton)."
exit 1
