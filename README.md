# SIC/XE — Montador, Ligador e Máquina Virtual

Implementação em C++20 de um **montador de dois passos**, **ligador-relocador**, **carregador absoluto** e **máquina virtual** para a arquitetura SIC/XE, com interface gráfica em Qt6.

## Dependências

| Dependência | Versão mínima | Instalação (Ubuntu/Debian) |
|---|---|---|
| g++ (com suporte a C++20) | 10+ | `sudo apt install g++` |
| CMake | 3.16+ | `sudo apt install cmake` |
| Ninja (opcional, recomendado) | — | `sudo apt install ninja-build` |
| Qt6 Widgets | 6.x | `sudo apt install qt6-base-dev` |

## Compilação

```bash
mkdir -p build && cd build
cmake .. -G Ninja
ninja
```

Isso gera dois executáveis:

| Executável | Descrição |
|---|---|
| `VM_SIC` | Versão terminal (CLI) — executa o teste hardcoded em `main.cpp` |
| `VM_SIC_GUI` | Interface gráfica Qt6 |

Para compilar apenas um deles:

```bash
ninja VM_SIC_GUI   # só a GUI
ninja VM_SIC       # só o CLI
```

## Execução

### GUI

```bash
./build/VM_SIC_GUI
```

### CLI

```bash
cd build && ./VM_SIC
```

O CLI monta os arquivos definidos em `VM/main.cpp`, linka e executa automaticamente. Os caminhos dos fontes são relativos ao diretório `build/`, por isso é necessário executar de dentro dele.

## Usando a Interface Gráfica

O fluxo na GUI segue a ordem do pipeline:

### 1. Adicionar módulos fonte

- Clique em **Abrir** para selecionar um arquivo `.txt` e visualizar seu conteúdo.
- Use **+ Módulo** para adicionar mais arquivos à lista de módulos (para linkagem multi-módulo).
- Use **− Módulo** para remover um módulo selecionado da lista.

### 2. Montar

- Clique em **1. Montar**.
- O montador gera um arquivo `.obj` para cada módulo da lista.
- O conteúdo dos arquivos objeto (registros H, D, R, T, M, E) aparece no painel **Arquivo Objeto**.

### 3. Linkar e Carregar

- Defina o **PROGADDR** (endereço de carga em hexadecimal) no campo ao lado. O padrão é `0000`.
- Clique em **2. Linkar + Carregar**.
- O ligador-relocador constrói a ESTAB, aplica os M records e gera a imagem binária.
- O carregador copia a imagem para a memória da máquina virtual no endereço especificado.

### 4. Executar

- **▶ Executar Tudo** — roda o programa até halt (self-loop) ou erro.
- **⏭ Passo** — executa uma única instrução por vez (step-by-step), permitindo acompanhar a evolução dos registradores e da memória a cada passo.

### 5. Painéis de inspeção

- **Registradores** — mostra A, X, L, B, S, T, PC e SW em hexadecimal, atualizados a cada passo.
- **Memória** — grid de 16 bytes por linha. O byte apontado pelo PC é destacado em amarelo. Use o campo **Endereço (hex)** + **Ir** para navegar.
- **Log de Execução** — mostra cada instrução executada e seu efeito.

### 6. Reset

- **↻ Reset** limpa a máquina, os registradores, a memória e o log, permitindo recomeçar.

## Estrutura do Projeto

```
├── CMakeLists.txt
├── data/                        # Programas fonte SIC/XE de teste
│   ├── programa4.txt            # Exemplo do livro (Beck)
│   ├── teste_modA.txt           # Módulo A (programa principal)
│   └── teste_modB.txt           # Módulo B (sub-rotina)
├── VM/
│   ├── MontadorSemVibecode.h/cpp  # Montador de dois passos
│   ├── Ligador.h/cpp              # Ligador-Relocador + Carregador Absoluto
│   ├── Maquina_melhor.h/cpp       # Máquina virtual SIC/XE
│   ├── CPU.h/cpp                  # Registradores da CPU
│   ├── Memoria.h/cpp              # Memória byte-endereçável
│   ├── MainWindow.h/cpp           # Interface gráfica (Qt6)
│   ├── main_gui.cpp               # Entry point da GUI
│   ├── main.cpp                   # Entry point do CLI
│   └── ProcessadorDeMacros.cpp    # Processador de macros (independente)
└── build/                         # Diretório de compilação
```

## Formato dos Arquivos Fonte

Os arquivos de entrada seguem o formato assembly SIC/XE com colunas separadas por espaço/tab:

```
COPY    START   0
        EXTDEF  BUFFER,BUFEND,LENGTH
        EXTREF  RDREC,WRREC
FIRST   STL     RETADR
        ...
        END     FIRST
```

- **Rótulo** na coluna 1 (sem espaço antes), ou espaço/tab se não houver rótulo.
- Diretivas suportadas: `START`, `END`, `BYTE`, `WORD`, `RESB`, `RESW`, `BASE`, `EQU`, `EXTDEF`, `EXTREF`.
- Prefixo `+` para formato 4, `#` para imediato, `@` para indireto.
