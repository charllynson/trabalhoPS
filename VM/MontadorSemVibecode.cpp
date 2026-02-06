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

    std::cout << "Endereço atual do " << opcode << " é " << std::hex << std::uppercase << address_count << "\n";

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
      Se o tamanho atual do buffer estiver no limite de 69 caracteres hex de registro T, encerrar buffer e escrever linha no txt
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
  std::uint32_t baseAddress;
  std::string r1,r2;

  std::vector<std::string> modifiers;

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

  address_count = programStartAddress;

  // Parser do registro T
  // Para cada registro novo, colocar T + address_count + row size + obj codes stackados
  buffer += "T";
  buffer += parseToHexWithPad(address_count, 6);

  while (std::getline(file, line)) {
    std::istringstream ss(line);

    // Ignorar rótulo SEMPRE
    if (line[0] != ' ' && line[0] != '\t') {
      ss >> rotulo >> instr;
    } else {
      ss >> instr;
    }

    int estimatedNextSize = 0;
    if (instruçoesFormato2.count(instr)) estimatedNextSize = 4;
    else if (instruçoesFormato34.count(instr)) estimatedNextSize = 6;

    if (buffer.length() + estimatedNextSize > 69) {
      outfile << "T"
              << buffer.substr(1, 6) 
              << parseToHexWithPad(buffer.substr(7).size() / 2, 2) 
              << buffer.substr(7)
              << "\n";
      buffer.clear();
      buffer += "T";
      buffer += parseToHexWithPad(address_count, 6);
    }

    // Parser para instruções de formato 2 (opcode 8bits | reg1 4bits | reg2 4bits)
    if (instruçoesFormato2.count(instr)) {
      if (instr == "SHIFTR") {
        buffer += parseToHexWithPad(instruçoesFormato2[instr], 2);
        buffer += parseToHexWithPad(0x0, 1);
        buffer += parseToHexWithPad(0x0, 1);
        address_count += 2;
        continue;
      }

      ss >> r1 >> r2;

      // Posso usar packing de bits em vez disso
      auto instrOpcode = parseToHexWithPad(instruçoesFormato2[instr], 2);
      auto reg1 = parseToHexWithPad(registers[r1], 1);
      auto reg2 = parseToHexWithPad(registers[r2], 1);

      buffer += instrOpcode;
      buffer += reg1;
      buffer += reg2;

      address_count += 2;
    } 

    // Parser para instruções de formato 3 ou 4
    else if (instruçoesFormato34.count(instr)) {
      bool n,i,x,b,p,e;
      e = false;

      // tratar de instruções especiais desse formato sem operandos
      if (instr == "RSUB") {
        n=1; i=1; x=0; b=0; p=0; b=0;
        auto object = pack_fmt3(instruçoesFormato34[instr] >> 2, n,i,x,b,p,e, 0);       
        buffer += parseToHexWithPad(object, 6);
        address_count += 3;
        continue;
      }

      address_count += 3;

      std::string label;
      ss >> label;
      std::int16_t resultingDispl;

      if (label[0] == '@') {
        n = 1;
        i = 0;
        label = label.substr(1);
      }
      else if (label[0] == '#') {
        n = 0;
        i = 1;
        label = label.substr(1);
      }  else {
        n = 1;
        i = 1;
      }

      size_t indexing = label.find_first_of(',');
      if (indexing != std::string::npos) {
        label = label.substr(0, indexing);
        x = 1;
      } else {
        x = 0;
      }

      if (symbol_table.count(label)) {
        // PRECISO MESMO USAR CASTING DOS OPERANDOS AQUI?
        std::int32_t displ = static_cast<int32_t>(symbol_table[label]) - static_cast<int32_t>(address_count);

        if (displ >= -2048 && displ <= 2047) {
          // 12 bits inferiores
          resultingDispl = displ & 0xFFF;
          p = true;
          b = false;
        } else {
          std::int32_t dispBase = static_cast<int32_t>(symbol_table[label]) - static_cast<int32_t>(baseAddress);
          if (dispBase >= 0 && dispBase <= 4095) {
            b = true;
            p = false;
            resultingDispl = dispBase & 0xFFF;
          } else {
            std::cerr << "Endereço fora dos limites.\n";
            return 1;
          }
        }
      } else if (std::isdigit(static_cast<unsigned char>(label[0]))){
        resultingDispl = parseHexOpcode(label);
        b = false;
        p = false;
      } else {
        std::cerr << "Operando não numérico desconhecido. Abortando o programa...\n";
        return false;
      }
      
      std::uint8_t opcode = instruçoesFormato34[instr];

      auto object = pack_fmt3(opcode >> 2, n,i,x,b,p,e, resultingDispl);
      buffer += parseToHexWithPad(object, 6);
    } 
    else if (instruçoesFormato34.count(instr.substr(1))) {
      address_count += 4;
      bool n,i,x,b,p,e;
      e = true;
      b = false;
      p = false;

      std::string label;
      ss >> label;
      std::uint32_t resultingDispl;

      if (label[0] == '@') {
        n = true;
        i = false;
        label = label.substr(1);

        resultingDispl = symbol_table[label];
      } else if (label[0] == '#') {
        n = false;
        i = true;
        label = label.substr(1);

        resultingDispl = parseHexOpcode(label);
      } else {
        n = true;
        i = true;
        resultingDispl = symbol_table[label];
      }

      size_t indexing = label.find_first_of(',');
      if (indexing != std::string::npos) {
        label = label.substr(0, indexing+1);
        x = 1;
      } else {
        x = 0;
      }

      std::uint8_t opcode = instruçoesFormato34[instr.substr(1)];
      auto object = pack_fmt4(opcode >> 2, n,i,x,b,p,e, resultingDispl);
      buffer += parseToHexWithPad(object, 8);

      // Gerar registros M
      if (symbol_table.count(label)) {
        std::string currentModifier;
        currentModifier += "M";
        std::ostringstream oss;
        // o endereço a ser modificado pelo carregador está no segundo byte da instrução,
        // por isso o -4 + 1
        oss << std::uppercase << std::hex 
          << std::right << std::setw(6) << std::setfill('0')
          << (address_count - 4) + 1
          << std::right << std::setw(2) << 0x05;
        modifiers.push_back(currentModifier += oss.str());
      }
    } 
    // Parser para diretivas
    else if (directives.count(instr)) {
      std::string operand;
      std::uint32_t hexa_operand;
      if (instr == "END") {
        // Agregando os registros M que indiquem as posições LOC + quantos nibbles modificar
        if (buffer.length() > 7) {
          outfile << "T"
                << buffer.substr(1, 6) 
                << parseToHexWithPad(buffer.substr(7).size() / 2, 2) 
                << buffer.substr(7)
                << "\n";
        }
        buffer.clear();
        for (const auto& modifier : modifiers) {
          outfile << modifier << "\n";
        }
        ss >> operand; 

        std::uint32_t startExec = operand.empty() ? programStartAddress : symbol_table[operand];
        outfile << "E" << parseToHexWithPad(startExec, 6);
        return true;
      } else if (instr == "WORD") {
        address_count += 3;
        ss >> operand;

        if (std::isdigit(static_cast<unsigned int>(operand[0]))) {
          buffer += parseToSignedHexWithPad(parseHexOpcode(operand), 6);
        } else if (symbol_table.count(operand)) {
          buffer += parseToHexWithPad(symbol_table[operand], 6);
        }
      } else if (instr == "RESW") {
        if (buffer.length() > 7) {
          outfile << "T"
                  << buffer.substr(1, 6)
                  << parseToHexWithPad(buffer.substr(7).size() / 2, 2) 
                  << buffer.substr(7) 
                  << "\n";
        }
        buffer.clear();
        ss >> operand;
        hexa_operand = parseHexOpcode(operand);
        address_count += 3 * hexa_operand;

        buffer += "T";
        buffer += parseToHexWithPad(address_count, 6);
      } else if (instr == "RESB") {
        if (buffer.length() > 7) {
          outfile << "T"
                  << buffer.substr(1, 6)
                  << parseToHexWithPad(buffer.substr(7).size() / 2, 2) 
                  << buffer.substr(7) 
                  << "\n";
        }
        buffer.clear();
        ss >> operand;
        hexa_operand = parseHexOpcode(operand);
        address_count += hexa_operand;

        buffer += "T";
        buffer += parseToHexWithPad(address_count, 6);
      } else if (instr == "BYTE") {
        // USAR parseHexOpcode COM STOI(OPCODE, 16)!!!!!!!!!
        int count = 0;
        ss >> operand;
        size_t first = operand.find('\'');
        size_t last = operand.find_last_of('\'');

        if (operand[0] == 'C') {
          if (first != std::string::npos && last != std::string::npos && last > first) {
            count = last - first - 1;

            for (auto it = operand.begin()+first+1; it != operand.begin()+last; ++it) {
              buffer += parseToHexWithPad(static_cast<int>(*it), 2);
            }
          }
        } else if (operand[0] == 'X') {
          if (first != std::string::npos && last != std::string::npos && last > first) {
            count = (last - first - 1)/2;
          }

          if ((last - first + 1) % 2) {
            buffer += '0'; 
          }
          for (auto it = operand.begin()+first+1; it != operand.begin()+last; ++it) {
            buffer += *it;
          }
        }
        address_count += count;
      } else if (instr == "BASE") {
        ss >> operand;
        if (symbol_table.count(operand)) {
          baseAddress = symbol_table[operand];
        } else {
          baseAddress = parseHexOpcode(operand);
        }
      }
    }
  }


  return true;
}