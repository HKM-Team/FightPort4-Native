# FightPort 4 — Versão 1.3.0 Release Notes

**Data de Lançamento:** 21 de Outubro de 2026  
**Status:** Oficial / Estável  

---

## 🥊 Principais Destaques da Versão 1.3.0

A versão **1.3.0** traz uma grande consolidação de estabilidade, nova organização interna de arquivos e pastas, extração aprimorada de PKG com fluxo seguro e o seletor visual de ícones de lutadores na Área de Trabalho.

### 1. Seletor Visual de Ícones & Gestão de Atalhos na Área de Trabalho
- **19 Ícones Exclusivos em Alta Resolução:**
  - 9 grandes lutadores do UFC com 2 variações fotográficas cada (Alex Poatan, Anderson Silva, Charles do Bronx, Conor McGregor, Georges St-Pierre, Islam Makhachev, Jon Jones, José Aldo, Khabib Nurmagomedov).
  - Ícone clássico oficial FightPort 4 (Logo Original).
- **Criação e Alteração em Tempo Real:** Criação do atalho na Área de Trabalho (`UFC 4.lnk`) e atualização imediata via `SHChangeNotify` sem reiniciar o Windows.
- **Proteção da Barra de Tarefas e Título:** O atalho do Desktop recebe o lutador escolhido, enquanto a janela e a barra de tarefas do Windows preservam a logo oficial do programa via `AppUserModelID` dedicado.

### 2. Extração de PKG Confiável e Proteção de Pastas
- **Extração com Progresso Preciso:** Monitoramento em tempo real de 0% a 100% cobrindo o processamento completo de arquivos.
- **Isolamento de Diretórios:** Previne extrações acidentais dentro do diretório do port (`games\UFC4`), avisando ao usuário e criando a subpasta dedicada do jogo (`UFC_4`) no diretório externo escolhido, evitando pastas duplicadas.
- **Detecção Automática Pós-Extração:** O inicializador identifica automaticamente o caminho extraído e atualiza o campo de jogo e o `sacredfire.ini`.
- **Cores Dinâmicas na Barra de Status:** Indicador visual claro por cores (Verde quando pronto para jogar, Vermelho quando o jogo não for encontrado e cor padrão quando o jogo é encerrado).

### 3. Organização Limpa do Repositório
- Todas as imagens PNG reunidas na pasta dedicada `src\png\`.
- Ícones centralizados na pasta `src\icons\`.
- Licenças agrupadas em `licenses\`.
- Telas de carregamento e protótipos organizados em `src\`.

---

## 🛠️ Detalhes Técnicos e Compilação
- Código nativo em C / Win32 / GDI+ sem dependências externas adicionais.
- Build compilado e verificado com MSVC 64-bit e recursos de versão Win32 atualizados para `1.3.0.0`.
