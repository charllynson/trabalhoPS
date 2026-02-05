#include "MontadorSemVibecode.h"

#define PROGRAMSTARTADDRESS 0x000000

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

  std::cout << "Contador de endereço (LOC) final: " << address_count << "\n";
  /*
  |SEGUNDO PASSO: montar código objeto com os opcodes, nixbpe, desl. ou regs. de acordo com o formato
    Definir header no .txt output com LOC final - LOC inicial e nome do programa
    Definir Buffer de registros T, colocando o LOC da próxima instrução juntamente ao tamanho do Buffer

    Para cada T: Iterar sobre cada linha do source

      Inicializar Buffer uint32_t que não ultrapasse 30 bytes de código objeto
      Verificar formato de instrução
      Setar a variável binária objeto com o respectivo tamanho
      Gerar os bits do opcode e colocar na variável
      Setar flags n e i de acordo com as dicas @ ou #
      Setar a flag x de acordo com o sufixo do operando (REVISAR COMO FICA O FORMATO PORQUE OPERANDO X É REAL)
      Setar flag e se tiver um + na instrução
      
      Tentar calcular Destino (symbtable) - PC+3, se couber entre -2048 e 2047 usa p=1 e b=0
      Senão, tente Destino (symbtable) - Base, usa b=1 e p=0
      Se nada der certo, gerar um erro fatal


      *****
      Se o tamanho atual do buffer estiver no limite de 69 bytes de código objeto, encerrar buffer e escrever linha no txt
      Se encontrarmos uma diretiva do tipo RESB e RESW, encerrar o T atual e começar outro no endereço logo após a lacuna 
      *****

  */

  file.clear();
  file.seekg(0, std::ios::beg);
  std::ofstream outfile(caminhoArquivoDestino);

  // H: 1 bit para o registro H, 6 bits para o começo, 6 bits para o tamanho (address_count - começo)
  // T: 1 bit para o registro T, 6 bits para o LOC, 2 bits para tamanho e 30 bits para object code
  std::string buffer;
  std::string rotulo;
  std::string instr;
  std::uint64_t desl;
  std::uint32_t r1,r2;

  std::uint64_t currentObjectCode = 0b000000000000;

  // Parser do registro Header
  std::getline(file, line);
  std::istringstream ss(line);
  std::string programName;
  std::string directive;
  std::uint64_t programStartAddress;
  ss >> programName >> directive >> programStartAddress;
  std::uint64_t programLength = address_count - programStartAddress;

  outfile << "H"
          << std::left << std::setw(6) << std::setfill(' ') << programName
          << std::uppercase << std::hex
          << std::right << std::setw(6) << std::setfill('0') << programStartAddress
          << std::right << std::setw(6) << std::setfill('0') << programLength
          << '\n';


  // Parser do registro T
  // while (std::getline(file, line)) {
  //   std::istringstream ss(line);

  //   if (line[0] != ' ' && line[0] != '\t') {
  //     ss >> rotulo >> instr;
  //   } else {
  //     ss >> instr;
  //   }

  //   // Parser para instruções de formato 2 (opcode 8bits | reg1 4bits | reg2 4bits)
  //   if (instruçoesFormato2.count(instr)) {
  //     ss >> r1 >> r2;


  //   }



    

  // }


  return false;
}