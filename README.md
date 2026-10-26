# FightPort 4 Native (1.3.0) — EA SPORTS UFC 4 Ultra-Lightweight PC Port

[![Version](https://img.shields.io/badge/Edition-Native%20(1.3.0)-blue.svg)](#)
[![Team](https://img.shields.io/badge/Maintained%20by-HKM%20Team-blue.svg)](https://github.com/HKM-Team)
[![License](https://img.shields.io/badge/License-GPL--2.0--or--later-green.svg)](licenses/LICENSE-GPL-2.0.txt)
[![Footprint](https://img.shields.io/badge/Total%20Size-~90%20MB-brightgreen.svg)](#)

Dedicated ultra-lightweight launcher and single-click PKG extraction system for the native PC port of **EA SPORTS UFC 4** (PlayStation 4, `CUSA14209` & `CUSA14204`, update **28.00**).

---

## 🥊 English (EN)

### Overview
**FightPort 4 Native** is the ultra-lightweight edition of FightPort, engineered specifically for maximum performance and low resource consumption on modest and entry-level PCs.
Operating 100% natively over pure **Vulkan** translation without the overhead of heavy upscaling modules, frame generation, or neural renderers, this version weighs only ~90 MB total while delivering fluid, responsive combat.

### Key Highlights
- **Ultra-Lightweight Footprint:** Entire port directory is under 95 MB (compared to 1.2 GB of standard upscaler editions).
- **100% Pure Native Vulkan:** No wrapper injection latency, no external upscaler conflicts, perfect for low-end graphics cards.
- **Visual Athlete Icon Selector:** 19 exclusive high-resolution desktop icons featuring 9 UFC legends (Poatan, Charles, Jon Jones, Khabib, Aldo, and more).
- **Automated PKG Extractor:** One-click extraction and update merger from the `EXTRAÇÃO DE PKG` tab.

### How to Install & Run
You can choose between two installation formats:

#### Option A: Portable Archive (.zip)
1. Download `FightPort4_Native_1.3.0.zip` from [Releases](https://github.com/HKM-Team).
2. Extract the archive into your preferred directory (e.g., `D:\Games\FightPort4-Native`).
3. Launch `FightPort4.exe`.

#### Option B: Executable Installer (.exe)
1. Download `FightPort4_Native_1.3.0_Setup.exe` from [Releases](https://github.com/HKM-Team).
2. Run the setup wizard, choose your installation path, and let it create the Desktop and Start Menu shortcuts automatically.
3. Launch FightPort 4 Native directly from your desktop!

#### Extracting Game Files (First Run)
1. In the `EXTRAÇÃO DE PKG` tab, click **`PROCURAR...`**.
2. Select your Base Game PKG (`v1.00`, `CUSA14209` or `CUSA14204`) and the official Update PKG (`v28.00`) together.
3. Click **`Extrair para o Jogo`**. FightPort 4 unrolls and merges the files directly into `games\UFC4`.
4. Click **`JOGAR`** and enjoy pure native gameplay!

---

## 🥊 Português (PT-BR)

### Visão Geral
O **FightPort 4 Native (1.3.0)** é a edição **ultra-leve** do FightPort, desenvolvida sob medida para entregar máxima fluidez e baixo consumo de memória em PCs fracos ou configurações de entrada.
Executando de forma 100% nativa sob a camada de tradução em **Vulkan puro**, sem o peso de motores de upscaling ou geração de quadros artificiais, esta versão ocupa apenas ~90 MB no disco e garante combate rápido e estável.

### Principais Inovações
- **Consumo Mínimo de Disco e Memória:** O projeto completo tem menos de 95 MB no total.
- **Desempenho Puro e Sem Travamentos:** Sem injeções de DLLs pesadas de upscalers ou redes neurais; ideal para placas de vídeo integradas ou mais modestas compatíveis com Vulkan.
- **Seletor Visual com 19 Ícones:** Escolha a foto do seu atleta favorito e aplique no atalho do seu Desktop com atualização instantânea no Windows Explorer.
- **Extrator Nativo de PKG:** Descompacte o jogo base (v1.00) e a atualização oficial (v28.00) com apenas um clique.

### Como Instalar e Jogar
Você pode escolher entre duas formas de distribuição:

#### Opção A: Pacote Portátil (.zip)
1. Baixe o pacote `FightPort4_Native_1.3.0.zip` nas [Releases oficiais da HKM Team](https://github.com/HKM-Team).
2. Descompacte o arquivo `.zip` na pasta de sua preferência.
3. Execute o inicializador **`FightPort4.exe`**.

#### Opção B: Instalador Executável (.exe)
1. Baixe o assistente `FightPort4_Native_1.3.0_Setup.exe` nas [Releases oficiais da HKM Team](https://github.com/HKM-Team).
2. Siga os passos na tela escolhendo o local de instalação. Ele criará os atalhos na Área de Trabalho e no Menu Iniciar automaticamente.
3. Abra o jogo direto pelo atalho criado no Desktop!

---

## 🛠️ Como Compilar o FightPort 4 Native
O FightPort 4 é escrito em C puro com as APIs nativas do Win32 e GDI+:
1. Abra o prompt do Visual Studio Developer (`vcvars64.bat`).
2. Navegue até a pasta `src\`.
3. Execute `build.bat`:
   ```cmd
   cd src
   build.bat
   ```
4. O executável `FightPort4.exe` será gerado na raiz do projeto.

---

## 📄 Licenças e Avisos Legais
- **FightPort 4 UI & Core:** Licenciado sob a Tri-Licença HKM (MIT, Apache 2.0 e GPLv3).
- **SacredFire / shadPS4:** Licenciados sob a GPL v2 ou posterior.
- **Aviso:** Este projeto é um inicializador de código aberto e **não contém arquivos protegidos por direitos autorais do jogo**. O usuário deve fornecer seus próprios arquivos legítimos do EA SPORTS UFC 4. Sem qualquer vínculo ou endosso de Electronic Arts, Sony Interactive Entertainment ou UFC.
