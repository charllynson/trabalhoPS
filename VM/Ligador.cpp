#include "Ligador.h"


// ============================================================
//  HELPERS: readField e writeField
// ============================================================

// Lê um campo de N nibbles da imagem de memória a partir de 'addr'.
// Se N é ímpar (ex: 5 nibbles de formato 4), o nibble alto do primeiro
// byte não pertence ao campo — só o nibble baixo.
// Retorna o valor com extensão de sinal.
int32_t Ligador::readField(std::uint32_t addr, int nibbles) {
  int numBytes = (nibbles + 1) / 2;  // ex: 5 nibbles → 3 bytes, 6 nibbles → 3 bytes
  bool odd = (nibbles % 2 != 0);

  int32_t value = 0;

  for (int i = 0; i < numBytes; ++i) {
    value = (value << 8) | memoria[addr + i];
  }

  // Se ímpar, mascarar fora o nibble alto do primeiro byte
  // Ex: para 5 nibbles, mascarar com 0x0FFFFF (20 bits)
  if (odd) {
    int32_t mask = (1 << (nibbles * 4)) - 1;
    value &= mask;
  }

  // Extensão de sinal: se o bit mais significativo do campo estiver ligado
  int signBit = nibbles * 4 - 1;
  if (value & (1 << signBit)) {
    // Preencher os bits acima com 1s (extensão de sinal)
    value |= (~0) << (nibbles * 4);
  }

  return value;
}

// Escreve um valor de volta na memória, preservando bits que não pertencem ao campo.
void Ligador::writeField(std::uint32_t addr, int nibbles, int32_t value) {
  int numBytes = (nibbles + 1) / 2;
  bool odd = (nibbles % 2 != 0);

  // Mascarar o valor para caber no campo
  int32_t mask = (nibbles >= 8) ? ~0 : ((1 << (nibbles * 4)) - 1);
  uint32_t masked = static_cast<uint32_t>(value) & mask;

  if (odd) {
    // Preservar o nibble alto do primeiro byte (contém flags nixbpe etc.)
    uint8_t preserved = memoria[addr] & 0xF0;
    // Escrever o nibble baixo do primeiro byte
    memoria[addr] = preserved | ((masked >> ((numBytes - 1) * 8)) & 0x0F);
    // Escrever os bytes restantes
    for (int i = 1; i < numBytes; ++i) {
      memoria[addr + i] = (masked >> ((numBytes - 1 - i) * 8)) & 0xFF;
    }
  } else {
    // Par: escrever todos os bytes normalmente
    for (int i = 0; i < numBytes; ++i) {
      memoria[addr + i] = (masked >> ((numBytes - 1 - i) * 8)) & 0xFF;
    }
  }
}


// ============================================================
//  LIGADOR-RELOCADOR (duas passagens)
// ============================================================
//
// Entrada: vetor de caminhos para arquivos .obj gerados pelo montador
//          + endereço de carga (PROGADDR)
//
// Fluxo geral:
//   PASSAGEM 1: ler H e D de cada módulo para construir a ESTAB
//               (tabela de símbolos externos com endereços absolutos)
//
//   PASSAGEM 2: ler T (copiar object code na memória)
//               e M (aplicar modificações/relocações usando ESTAB)
//
bool Ligador::LigadorRelocador(std::vector<std::string> modulos, std::uint32_t progaddr) {

  ESTAB.clear();
  progAddr = progaddr;  // salvar para o CarregadorAbsoluto

  // ==========================================================
  //  PASSAGEM 1: Construir a ESTAB
  // ==========================================================
  //
  // Para cada módulo:
  //   - Ler o registro H para obter nome e tamanho da seção
  //   - Calcular CSADDR (endereço de carga desta seção)
  //   - Ler registros D e inserir cada símbolo exportado na ESTAB
  //     com endereço absoluto = CSADDR + endereço_relativo
  //   - Avançar CSADDR += tamanho da seção para o próximo módulo
  //

  std::uint32_t CSADDR = progaddr;

  // Guardar o CSADDR de cada módulo para reusar na passagem 2
  std::vector<std::uint32_t> csaddrs;

  std::cout << "=== PASSAGEM 1: Construindo ESTAB ===" << "\n";

  for (const auto& caminho : modulos) {
    std::ifstream file(caminho);
    if (!file.is_open()) {
      std::cerr << "Erro ao abrir módulo: " << caminho << "\n";
      return false;
    }

    csaddrs.push_back(CSADDR);

    std::string line;
    std::string CSNAME;
    std::uint32_t CSLENGTH = 0;

    while (std::getline(file, line)) {
      if (line.empty()) continue;

      char tipo = line[0];

      // ---- Registro H (Header) ----
      // Formato: H | nome(6) | endereço_início(6) | tamanho(6)
      // Ex:      HCOPY  000000001027
      if (tipo == 'H') {
        CSNAME = line.substr(1, 6);
        // Remover espaços de padding do nome
        size_t end = CSNAME.find_last_not_of(' ');
        if (end != std::string::npos) CSNAME = CSNAME.substr(0, end + 1);

        CSLENGTH = hexToUint(line.substr(13, 6));

        // Inserir o próprio nome do programa na ESTAB
        if (ESTAB.count(CSNAME)) {
          std::cerr << "Erro: símbolo duplicado na ESTAB: " << CSNAME << "\n";
          return false;
        }
        ESTAB[CSNAME] = CSADDR;

        std::cout << "  Módulo: " << CSNAME
                  << " | CSADDR: " << toHex(CSADDR, 6)
                  << " | Tamanho: " << toHex(CSLENGTH, 6) << "\n";
      }

      // ---- Registro D (Definições externas) ----
      // Formato: D | (nome(6) + endereço(6))* repetido
      // Ex:      DBUFFER00003DBUFEND001033LENGTH000030
      else if (tipo == 'D') {
        std::string defs = line.substr(1); // tudo depois do 'D'

        // Cada definição ocupa 12 caracteres: 6 de nome + 6 de endereço
        for (size_t pos = 0; pos + 12 <= defs.size(); pos += 12) {
          std::string symName = defs.substr(pos, 6);
          // Remover padding de espaços
          size_t endPos = symName.find_last_not_of(' ');
          if (endPos != std::string::npos) symName = symName.substr(0, endPos + 1);

          std::uint32_t symAddr = hexToUint(defs.substr(pos + 6, 6));
          std::uint32_t absoluteAddr = CSADDR + symAddr;

          if (ESTAB.count(symName)) {
            std::cerr << "Erro: símbolo duplicado na ESTAB: " << symName << "\n";
            return false;
          }
          ESTAB[symName] = absoluteAddr;

          std::cout << "    DEF: " << symName << " → " << toHex(absoluteAddr, 6) << "\n";
        }
      }

      // Registros R, T, M, E: ignorados na passagem 1
      else if (tipo == 'E') {
        // Se for o primeiro módulo, capturar o endereço de execução
        if (&caminho == &modulos[0]) {
          if (line.size() > 1) {
            execAddr = CSADDR + hexToUint(line.substr(1, 6));
          } else {
            execAddr = progaddr;
          }
        }
        break;
      }
    }

    // Avançar o endereço de carga para o próximo módulo
    CSADDR += CSLENGTH;
    file.close();
  }

  // Tamanho total do programa ligado
  totalLength = CSADDR - progaddr;
  std::cout << "  Tamanho total do programa ligado: " << toHex(totalLength, 6) << "\n";

  // Alocar imagem de memória
  memoria.assign(totalLength, 0x00);

  std::cout << "\nESTAB completa:\n";
  for (const auto& [nome, addr] : ESTAB) {
    std::cout << "  " << nome << " → " << toHex(addr, 6) << "\n";
  }


  // ==========================================================
  //  PASSAGEM 2: Carregar código e aplicar modificações
  // ==========================================================
  //
  // Para cada módulo:
  //   - Registros T: copiar os bytes do object code para memoria[]
  //     no endereço CSADDR + endereço_relativo_do_T
  //
  //   - Registros M: aplicar modificações de relocação/ligação
  //     Ler o campo da memória, somar/subtrair o valor do símbolo
  //     da ESTAB, e escrever de volta
  //

  std::cout << "\n=== PASSAGEM 2: Carregando código e aplicando M records ===" << "\n";

  for (size_t modIdx = 0; modIdx < modulos.size(); ++modIdx) {
    std::ifstream file(modulos[modIdx]);
    if (!file.is_open()) {
      std::cerr << "Erro ao abrir módulo na passagem 2: " << modulos[modIdx] << "\n";
      return false;
    }

    CSADDR = csaddrs[modIdx];
    std::string line;

    while (std::getline(file, line)) {
      if (line.empty()) continue;

      char tipo = line[0];

      // ---- Registro T (Text) ----
      // Formato: T | endereço_início(6) | tamanho_em_bytes(2) | object_code(até 60 hex chars)
      // Ex:      T0000001E17201B69201B4B10000002201729000033200890000FA00F3F2FEB4F0000
      if (tipo == 'T') {
        std::uint32_t startAddr = hexToUint(line.substr(1, 6));
        std::uint32_t byteCount = hexToUint(line.substr(7, 2));
        std::string objectCode = line.substr(9);

        // Endereço absoluto na imagem = CSADDR + startAddr relativo
        // Porém, como a imagem começa em progaddr, o offset é:
        std::uint32_t memOffset = (CSADDR + startAddr) - progaddr;

        std::cout << "  T: copiando " << byteCount << " bytes em "
                  << toHex(CSADDR + startAddr, 6) << "\n";

        // Cada par de caracteres hex = 1 byte
        for (std::uint32_t i = 0; i < byteCount && (i * 2 + 1) < objectCode.size(); ++i) {
          std::string byteHex = objectCode.substr(i * 2, 2);
          std::uint8_t byte = static_cast<uint8_t>(hexToUint(byteHex));
          memoria[memOffset + i] = byte;
        }
      }

      // ---- Registro M (Modification) ----
      // Formato: M | endereço(6) | nibbles(2) | +/-SÍMBOLO
      // Ex:      M00000705+COPY
      //          M00000405+RDREC
      //
      // Significado:
      //   No endereço (CSADDR + endereço), modificar um campo de N nibbles,
      //   somando ou subtraindo o valor do símbolo encontrado na ESTAB.
      //
      else if (tipo == 'M') {
        std::uint32_t modAddr = hexToUint(line.substr(1, 6));
        int nibbles = static_cast<int>(hexToUint(line.substr(7, 2)));

        // O sinal e o símbolo começam na posição 9
        char sinal = '+';
        std::string simbolo;

        if (line.size() > 9) {
          sinal = line[9];
          simbolo = line.substr(10);
          // Remover espaços de trailing
          size_t endPos = simbolo.find_last_not_of(" \t\r\n");
          if (endPos != std::string::npos) simbolo = simbolo.substr(0, endPos + 1);
        } else {
          // M record sem símbolo explícito: assume o nome da seção atual
          // (caso do montador que gera M00000705 sem +NOME)
          sinal = '+';
          // Buscar o nome da seção deste módulo na ESTAB via csaddrs
          for (const auto& [nome, addr] : ESTAB) {
            if (addr == csaddrs[modIdx]) {
              simbolo = nome;
              break;
            }
          }
        }

        // Buscar o símbolo na ESTAB
        if (!ESTAB.count(simbolo)) {
          std::cerr << "Erro: símbolo não encontrado na ESTAB: " << simbolo << "\n";
          return false;
        }

        std::uint32_t symValue = ESTAB[simbolo];

        // Endereço absoluto na imagem de memória
        std::uint32_t absoluteAddr = CSADDR + modAddr;
        std::uint32_t memOffset = absoluteAddr - progaddr;

        // Ler o valor atual do campo na memória
        int32_t currentValue = readField(memOffset, nibbles);

        // Aplicar a modificação
        if (sinal == '+') {
          currentValue += static_cast<int32_t>(symValue);
        } else {
          currentValue -= static_cast<int32_t>(symValue);
        }

        // Escrever o valor modificado de volta
        writeField(memOffset, nibbles, currentValue);

        std::cout << "  M: " << sinal << simbolo
                  << " em " << toHex(absoluteAddr, 6)
                  << " (" << nibbles << " nibbles)"
                  << " valor ESTAB=" << toHex(symValue, 6) << "\n";
      }

      else if (tipo == 'E') {
        break;
      }
    }

    file.close();
  }


  // ==========================================================
  //  RESULTADO: Imprimir a imagem de memória
  // ==========================================================

  std::cout << "\n=== Imagem de memória final ===" << "\n";
  std::cout << "Endereço de carga: " << toHex(progaddr, 6) << "\n";
  std::cout << "Endereço de execução: " << toHex(execAddr, 6) << "\n";
  std::cout << "Tamanho total: " << toHex(totalLength, 6) << " bytes\n\n";

  // Imprimir em linhas de 16 bytes
  for (std::uint32_t i = 0; i < totalLength; i += 16) {
    std::cout << toHex(progaddr + i, 6) << ": ";
    for (std::uint32_t j = 0; j < 16 && (i + j) < totalLength; ++j) {
      std::cout << toHex(memoria[i + j], 2) << " ";
    }
    std::cout << "\n";
  }

  std::cout << "\n=== Ligação e relocação concluídas com sucesso ===" << "\n";
  return true;
}

void Ligador::CarregadorAbsoluto(Maquina& maquina) {
  maquina.memoria.limpar();

  for (auto i = 0; i < totalLength; ++i) {
    maquina.memoria.write(progAddr + i, memoria[i]);
  }

  maquina.cpu.r.PC= execAddr;

  std::cout << "Programa carregado na máquina no endereço "
            << toHex(progAddr, 6)
            << ", PC inicial: " << toHex(execAddr, 6) << "\n";
}