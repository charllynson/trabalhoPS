#include "MontadorSemVibecode.h"


MontadorSemVibecode::MontadorSemVibecode() {}


bool MontadorSemVibecode::montar(const std::string& caminhoArquivoFonte, const std::string& caminhoArquivoDestino) {
  /*
  |PRIMEIRO PASSO: Resolver símbolos com seus endereços
    Inicializa contador de endereço
    Carregar linha
      Se o primeiro caractere for espaço ou \t, é rótulo, se for rótulo:
        guarda no hashmap(tabela de simbolo) o rótulo com o endereço
      Senão:
        partir para a próxima linha, calculando a dimensionalidade da linha atual (formato)
        entre instruções e diretivas
  */

  std::unordered_map<std::string, std::uint32_t> symbol_table;
  std::string line;
  std::ifstream file(caminhoArquivoFonte);
  std::uint32_t address_count = 0;


  if (!file.is_open()) {
    std::cerr << "Erro ao abrir o arquivo fonte\n";
    return false;
  }

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string label;
    std::string opcode;
    std::uint32_t hexa_opcode;
    std::string displacement;
    std::string operand;
    std::uint32_t hexa_operand;

    // é rótulo
    if (line[0] != ' ' && line[0] != '\t') {
      iss >> label;
      symbol_table[label] = address_count;
    }

    iss >> opcode;

    std::cout << "Endereço atual do " << opcode << " é " << address_count << "\n";

    // std::cout << "iss >> opcode : " << opcode << "\n";

    // try {
    //   hexa_opcode = parseHexOpcode(opcode);
    // } catch (const std::exception& e) {
    //   std::cerr << "Erro ao processar opcode: " << e.what() << "\n";
    // }

    if (directives.count(opcode)) {
      if (opcode == "START") {
        iss >> operand;
        try {
          hexa_operand = parseHexOpcode(operand);
        } catch (const std::exception& e) {
          std::cerr << "Erro ao processar operando: " << e.what() << "\n";
        }
        address_count = hexa_operand;
      } else if (opcode == "END") {
        break;
      } else if (opcode == "WORD") {
        address_count += 3;
      } else if (opcode == "RESW") {
        iss >> operand;
        try {
          hexa_operand = parseHexOpcode(operand);
        } catch (const std::exception& e) {
          std::cerr << "Erro ao processar operando: " << e.what() << "\n";
        }
        address_count += 3 * hexa_operand;
      } else if (opcode == "RESB") {
        iss >> operand;
        try {
          hexa_operand = parseHexOpcode(operand);
        } catch (const std::exception& e) {
          std::cerr << "Erro ao processar operando: " << e.what() << "\n";
        }
        address_count += hexa_operand;
      } else if (opcode == "BYTE") {
        int count = 0;
        iss >> operand;
        size_t first = operand.find('\'');
        size_t last = operand.find_last_of('\'');

        if (operand[0] == 'C') {
          if (first != std::string::npos && last != std::string::npos && last > first) {
            count = last - first - 1;
          }
        } else if (operand[0] == 'X') {
          if (first != std::string::npos && last != std::string::npos && last > first) {
            count = (last - first - 1)/2;
          }
        }

        address_count += count;
      }
    } else if (instruçoesFormato2.count(opcode)) {
      address_count += 2;
    } else if (instruçoesFormato34.count(opcode)) {
      address_count += 3;
    } else if (instruçoesFormato34.count(opcode.substr(1))) {
      address_count += 4;
    }
  }

  std::cout << "Tabela de símbolos:\n";
  for (const auto& [key, value] : symbol_table) {
    std::cout << key << " -> " << value << "\n";
  }
  /*
  |SEGUNDO PASSO

  */

  return false;
}