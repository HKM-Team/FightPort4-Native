# UFC 4 native port 0.2.0

## 0.2.6 (FightPort 4)

- **Safer texture copies:** every image copy, upload and tiling pass is checked against its
  textures and buffers before it reaches the graphics card (the fix for UFC 2's Create
  Fighter crash, code 23).
- **Community patches** (by Lepexa and Antigravity), all off by default: 60+ FPS, resolutions,
  crowd off, shadows off, motion blur off, skinning method and more. See README-Windows.md
  ("Patches da comunidade") to turn one on.
- Better crash reports: a GPU loss now names the texture involved.

## 0.2.5 (FightPort 4)

- **Auto settings:** on the first start FightPort 4 reads your PC (graphics card and its memory,
  processor threads, RAM, monitor) and picks this game's settings. The new **Auto settings**
  button does it again any time. FSR stays off (native port: not needed); PS4 Pro mode only on
  strong NVIDIA / AMD cards.
- **UFC 4 first-start defaults:** 60 FPS (higher rates can break UFC 4), Mailbox, no sharpening.

## 0.2.4 (FightPort 4)

- **Louder, clearer sound.** The game mixes 7.1; on headphones and stereo speakers the port now
  folds it to 2 channels itself, without the attenuation that made voices and commentary quiet.
  New **Volume** setting (50% to 300%) in the launcher and FightPort 4, with a limiter against
  clipping.
- **Steam Deck (Linux):** a Deck profile, on by default there: 1280x720 with FSR 1.0 to the
  800p screen, 2 frames queued (fewer stutters with the CPU and GPU at full clocks), 60 FPS cap.
  `STEAM_DECK_PROFILE=0` in the .conf turns it off.
- **ReShade** works (Vulkan): see README-Windows.md.
- **Better crash reports:** a crash inside the graphics driver (the "code 3221225477" error) now
  writes where it happened to the log, and slow scenes log which part of the frame costs the time.
- Since the last version: the Career FPS collapse (UFC 3: crash code 3 after loading your
  fighter) is fixed in all three games.

[Português](#português)

The update for what players reported after 0.1.0. Your saves, settings and shader cache are
kept: extract the new version over the old one, or into a new folder.

## What's new

- **Two or more controllers, two or more players.** Each gamepad is now its own PS4 user, so a
  second controller can join as Player 2 in fighter select. It works with two identical pads
  (two Xbox controllers) and with Parsec's virtual controllers. It is on by default; the in-game
  menu (**Insert** or **L3+R3**) has the switch "One player per controller" and shows which pad
  is which player.
- **CUSA14204** (the UFC 4 edition sold in the Americas) is accepted, with the 28.00 update, as
  well as CUSA14209.
- **Game folder detection.** The error `prepare failed: [Errno 2] No such file or directory:
  ...\sce_module\libc.prx` is gone:
  - an update dumped without `sce_module` takes the base game's, as the console does;
  - the base game and its update are paired even when their folder names say nothing (by
    their `param.sfo`), also as `CUSA14209-app` + `CUSA14209-patch`;
  - choosing the folder that contains the game (an extractor's output folder) works;
  - when something is still missing, the launcher's message says what and how to fix it.
- **Fewer frame drops.** When the game copied memory next to data the GPU had written (fight
  intros, menus), the port waited for the GPU each time: up to 1,400 times per second in the
  logs. These copies are now tracked byte by byte and no longer wait. This waiting also grew
  during long career sessions, which made the frame rate fall over time.
- **Career saves.** A save is written to a temporary file and then put in place of the old
  one. On Windows that last step failed when another program (often an antivirus) had the save
  open, which could leave the two files of a save from different moments: the "damaged save"
  message. The replacement now retries and uses a method that works with open files. If it
  still cannot happen, the previous save is kept intact and the log says so.
- **Intel processors.** UFC 4 uses three AMD-only instructions (SSE4a). On Intel CPUs they are
  now replaced by equivalent code when the game loads; before, the game closed when it reached
  them.
- **Intel Arc graphics cards.** The Intel fixes from mancelli's shadPS4 fork for UFC 4 (tested
  there on an Arc B580) are included: black squares on fighters, floor flicker and shadow gaps.
  They are on only with Intel's drivers (`BB_INTEL_FIXES=0` turns them off for a test).
- **Linux.** A Linux release is available (`UFC4Native-linux-x86_64.tar.gz`), with a launcher
  window (GTK 4) and the command line `./ufc4.sh --play /path/to/game`. Requires glibc 2.38+
  (Ubuntu 24.04, Debian 13, Fedora 39, Arch, SteamOS 3.6 or newer).
- **Career opening restored.** "Skip intro videos" skipped UFC 4's Career opening scenes too
  (they live with the first-run videos); now only the startup logos are skipped.
- **Smoother fights.** The game's threads now run with the PS4's floating-point mode
  (flush-to-zero), as on the console. Math on very small numbers (physics, animation, audio)
  no longer runs on the CPU's slow path. `BB_GUEST_MXCSR=0` restores the old behaviour for a test.
- The launcher's error messages now say why a step failed instead of only "see the log".

## Upgrading

- Windows: extract `UFC4Native-windows-x86_64.zip` and run `UFC4.exe`. Data, saves and logs stay
  in `%LOCALAPPDATA%\UFC4Native`.
- Linux: extract `UFC4Native-linux-x86_64.tar.gz` and run `./UFC4.sh` (or `./ufc4.sh --play`).
  Data, saves and logs: `~/.local/share/UFC4Native`.
- On Intel graphics cards the shaders are compiled again once, the first time each scene loads.

## Known issues

- Career mode: the image can look too contrasted, and blood on faces and on the mat can be
  missing (Quick Fight is fine). Under investigation; the package has a diagnostics mode for it
  (`BB_AUTO_DIAG=20` saves frame captures and thumbnails in the logs folder).

## If something is wrong

Send the newest `.log` from the launcher's **Open logs folder** (for frame rate problems, enable
**Frame statistics** first). No game files are included or needed in a report.

---

## Português

A atualização com o que os jogadores relataram depois da 0.1.0. Seus saves, configurações e o
cache de shaders são mantidos: extraia a versão nova por cima da antiga ou numa pasta nova.

## Novidades

- **Dois ou mais controles, dois ou mais jogadores.** Cada controle agora é um usuário do PS4,
  então o segundo controle entra como Jogador 2 na seleção de lutador. Funciona com dois
  controles iguais (dois de Xbox) e com os controles virtuais do Parsec. Vem ligado; o menu do
  jogo (**Insert** ou **L3+R3**) tem a opção "One player per controller" e mostra qual controle
  é qual jogador.
- **CUSA14204** (a edição do UFC 4 vendida nas Américas) é aceita com o update 28.00, além da
  CUSA14209.
- **Detecção da pasta do jogo.** O erro `prepare failed: [Errno 2] No such file or directory:
  ...\sce_module\libc.prx` acabou:
  - um update extraído sem a pasta `sce_module` usa a do jogo base, como o console faz;
  - o jogo base e o update são juntados mesmo quando o nome das pastas não diz nada (pelo
    `param.sfo`), e também como `CUSA14209-app` + `CUSA14209-patch`;
  - escolher a pasta que contém o jogo (a pasta de saída do extrator) funciona;
  - se ainda faltar algo, a mensagem do launcher diz o que falta e como resolver.
- **Menos quedas de FPS.** Quando o jogo copiava memória ao lado de dados que a placa de vídeo
  tinha escrito (introduções das lutas, menus), o port esperava a GPU a cada vez: até 1.400 vezes
  por segundo nos logs. Essas cópias agora são acompanhadas byte a byte e não esperam mais. Essa
  espera também crescia em sessões longas de carreira, o que fazia o FPS cair com o tempo.
- **Saves da carreira.** O save é gravado num arquivo temporário que depois substitui o antigo.
  No Windows esse último passo falhava quando outro programa (muitas vezes o antivírus) estava com
  o save aberto, e as duas partes do save podiam ficar de momentos diferentes: a mensagem de save
  danificado. Agora a troca tenta de novo e usa um método que funciona com arquivos abertos. Se
  mesmo assim não der, o save anterior fica intacto e o log avisa.
- **Processadores Intel.** O UFC 4 usa três instruções que só existem em processadores AMD
  (SSE4a). Em processadores Intel elas agora são trocadas por código equivalente quando o jogo
  carrega; antes, o jogo fechava ao chegar nelas.
- **Placas Intel Arc.** As correções para Intel do fork do shadPS4 do mancelli para o UFC 4
  (testadas lá numa Arc B580) foram incluídas: quadrados pretos nos lutadores, piso piscando e
  falhas nas sombras. Só ligam com os drivers da Intel (`BB_INTEL_FIXES=0` desliga para teste).
- **Linux.** Há uma versão para Linux (`UFC4Native-linux-x86_64.tar.gz`), com janela de launcher
  (GTK 4) e linha de comando `./ufc4.sh --play /caminho/do/jogo`. Precisa de glibc 2.38+
  (Ubuntu 24.04, Debian 13, Fedora 39, Arch, SteamOS 3.6 ou mais novos).
- **Abertura da Carreira de volta.** "Pular vídeos de introdução" também pulava as cenas de
  abertura da Carreira do UFC 4 (elas ficam junto dos vídeos do primeiro acesso); agora só os
  logos do início são pulados.
- **Lutas mais fluidas.** As threads do jogo agora rodam no modo de ponto flutuante do PS4
  (flush-to-zero), como no console. Contas com números muito pequenos (física, animação, áudio)
  não caem mais no caminho lento do processador. `BB_GUEST_MXCSR=0` volta ao comportamento antigo
  para teste.
- As mensagens de erro do launcher agora dizem por que uma etapa falhou, em vez de só "veja o log".

## Como atualizar

- Windows: extraia o `UFC4Native-windows-x86_64.zip` e abra o `UFC4.exe`. Dados, saves e logs
  continuam em `%LOCALAPPDATA%\UFC4Native`.
- Linux: extraia o `UFC4Native-linux-x86_64.tar.gz` e rode `./UFC4.sh` (ou `./ufc4.sh --play`).
  Dados, saves e logs: `~/.local/share/UFC4Native`.
- Em placas Intel os shaders são compilados de novo uma vez, na primeira carga de cada cena.

## Problemas conhecidos

- Modo Carreira: a imagem pode ficar contrastada demais e pode faltar sangue no rosto e no
  chão (na Luta Rápida está normal). Em investigação; o pacote tem um modo de diagnóstico para
  isso (`BB_AUTO_DIAG=20` salva capturas de quadros e miniaturas na pasta de logs).

## Se algo der errado

Mande o `.log` mais recente do botão **Open logs folder** do launcher (para quedas de FPS, ligue
**Frame statistics** antes). Nenhum arquivo do jogo vai no relatório.
