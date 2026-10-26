#!/bin/sh
# Cross-compile with llvm-mingw (https://github.com/mstorsjo/llvm-mingw); on Windows run the same commands.
x86_64-w64-mingw32-windres app.rc -O coff -o app.res
x86_64-w64-mingw32-clang -O2 -municode -mwindows -Wall -o FightPort4.exe fightport4.c pkg_engine.c app.res \
  -lcomctl32 -lshell32 -lshlwapi -luxtheme -lgdiplus -lole32 -ldwmapi -lgdi32 -luser32 -ladvapi32 -lmsimg32 -luuid -s
