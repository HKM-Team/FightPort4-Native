# UFC 4 native port — Windows (experimental)

Port nativo do **EA SPORTS UFC 4** (PlayStation 4, `CUSA14209` ou `CUSA14204`, update **28.00**),
feito a partir do port nativo do UFC 3 (https://github.com/GNSB/UFC-3-PS4-PORT-LINUX-WINDOWS,
GPL-2.0-or-later). **Não contém arquivos do jogo**: você precisa do seu próprio dump.
O que mudou nesta versão: `RELEASE-NOTES.md`.

## Requisitos

- Windows 10 (1903+) ou 11, 64 bits; processador com AVX (Intel ou AMD); placa de vídeo com
  Vulkan 1.3 (NVIDIA, AMD ou Intel Arc).
- A pasta do jogo **extraída** com o `eboot.bin` descriptografado e o update 28.00
  (`eboot.bin`, `sce_sys`, `sce_module`, `databigs`…). Também serve:
  - o jogo base com a pasta do update ao lado (`CUSA14209` + `CUSA14209-patch`, ou dois nomes
    quaisquer: o launcher junta os dois pelo `param.sfo`);
  - a pasta que contém a pasta do jogo (a pasta de saída do extrator).

## Como jogar

1. Extraia esta pasta em qualquer lugar (fora da pasta do jogo e fora do OneDrive).
2. Abra **`UFC4.exe`**, clique em **Browse...**, escolha a pasta do UFC 4 e aperte **Play**.
3. Dados, saves e logs ficam em `%LOCALAPPDATA%\UFC4Native`.
4. Dois jogadores: conecte dois controles (também funciona com dois controles iguais e pelo
   Parsec); o segundo entra como Jogador 2 na seleção de lutador. O menu do jogo (**Insert** ou
   **L3+R3**) mostra qual controle é qual jogador e tem a opção "One player per controller".
5. Se algo der errado: **Open logs folder** e mande o `.log` mais novo. Para quedas de FPS,
   marque **Frame statistics** antes.

## Desempenho

- **FPS limit → 30 FPS (PS4)** é o mais estável (o próprio UFC 4 configura o vídeo do PS4 para
  30 FPS); 60 FPS funciona na maioria dos PCs.
- Shaders são compilados na primeira vez que aparecem e guardados em cache
  (`%LOCALAPPDATA%\UFC4Native\user\cache`): a primeira luta numa arena nova pode ter uma tela
  preta curta; as seguintes ficam mais lisas. Não marque "Clear the shader cache" sem motivo.

## Volume

O som baixo foi corrigido: o jogo mistura em 7.1 e, em fone ou caixa estéreo, o port agora faz a
mistura para 2 canais sem abaixar vozes e narração. Se ainda quiser mais alto (ou mais baixo), use
**Volume** no launcher (50% a 300%; um limitador evita estourar o som).

## ReShade

O port usa Vulkan e funciona com o ReShade (https://reshade.me):

1. Rode o instalador do ReShade e escolha o executável **`bin\sacredfire-game.exe`** dentro da
   pasta do port (`UFC4.exe` é só o launcher; no FightPort 4: `games\UFC4\bin\sacredfire-game.exe`).
2. Em "rendering API" escolha **Vulkan**, e depois os efeitos que quiser.
3. Abra o jogo pelo launcher como sempre; **Home** abre o menu do ReShade.

Dicas: use efeitos de cor e nitidez (LumaSharpen, Vibrance, LUT…). Efeitos que dependem de
profundidade (MXAO, DOF) podem não achar o buffer certo. Se o jogo não abrir com o ReShade,
desinstale-o pelo mesmo instalador.

## Patches da comunidade (opcionais)

`patches\UFC4-community.xml` traz patches da comunidade (Lepexa, Antigravity): 60+ FPS, correção
do chão do octógono, resoluções, sem torcida (+5-10 FPS), sem sombras, sem motion blur, método de
skinning. **Todos vêm desligados.** Para ligar um: abra o arquivo no Bloco de Notas, ache o patch
pelo `Name` com a versão do seu jogo (`AppVer`) e troque `isEnabled="false"` por `isEnabled="true"`.
Na próxima vez que o jogo abrir, o log mostra `Patches: external ...`. Alguns têm efeitos
colaterais (veja o `Note`, por exemplo lutadores caindo pelo octógono): desligue se der problema.

## O que não funciona

- **Online**: o servidor incluído (`tools\ufc3_server.py`) é o do UFC 3; não use Join/Host.
- Os patches de resolução 2K/4K em `patches\experiments` são do UFC 3 e não se aplicam ao UFC 4.

Este projeto não é afiliado nem endossado pela Electronic Arts, Sony Interactive Entertainment
ou UFC. GPL-2.0-or-later; licenças de terceiros em `licenses\`.
