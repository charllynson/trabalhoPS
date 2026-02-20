#include "Maquina_melhor.h"

Maquina::Maquina(std::size_t tamanho_memoria) : memoria(tamanho_memoria){}

/*
=========================================================================================
Carregar o programa na memória a partir de um arquivo binário.
Cada byte do arquivo é lido e armazenado sequencialmente na memória da máquina.
As instruções seguem o padrão de formato 1, 2, 3/4 conforme SIC/XE.
Conseguimos distinguir of formatos 3/4 pela flag 'e' no quarto bit do terceiro nibble.

Exemplo: LDA #0xA1C (formato 3, imediato)
0000 0011 0000 1010 0001 1100 
opcode ni|xbpe |disp 12bits |
=========================================================================================
*/
// void Maquina::carregarPrograma(const std::string& caminhoArquivo) {
//     std::ifstream arquivo(caminhoArquivo, std::ios::binary);
//     if (!arquivo) {
//         std::cerr << "Erro ao abrir o arquivo: " << caminhoArquivo << std::endl;
//         return;
//     }
// 
//     std::size_t endereco = 0;
//     int byte;
//     while((byte = arquivo.get()) != EOF) {
//         memoria.setByte(endereco++, static_cast<std::uint8_t>(byte));   
//     }
// 
//     // início do programa
//     cpu.r.PC = 0;
// }

/*
=========================================================================================
Começar o loop de execução da máquina assim que o programa é carregado.
=========================================================================================
*/
void Maquina::executar() {
    bool rodando = true;
    while(rodando){
        auto pc_antes = cpu.r.PC;
        passo();

        // Detectar self-loop (ex: J HALT onde HALT aponta para si mesmo)
        if (cpu.r.PC == pc_antes) {
            std::cout << "Halt detectado (loop no endereço 0x"
                      << std::hex << cpu.r.PC << std::dec << ")" << std::endl;
            rodando = false;
        }
        // Se PC ultrapassar o tamanho da memória, parar a execução
        if(cpu.r.PC >= memoria.getTamanhoBytes()) {
            std::cout << "Fim da execução" << std::endl;
            rodando = false; 
        }
    }
}

/*
=========================================================================================
Retornar uma referência direta para um dos registradores do CPU com base no
número (0-9) fornecido, que pode ser usado para modificar o valor do registrador.
=========================================================================================
*/
std::int32_t& Maquina::getRegistradorPorNumero(std::uint8_t num) {
    switch (num) {
        case RegID::A: return cpu.r.A;
        case RegID::X: return cpu.r.X;
        case RegID::L: return cpu.r.L;
        case RegID::B: return cpu.r.B;
        case RegID::S: return cpu.r.S;
        case RegID::T: return cpu.r.T;
        default:
            std::cerr << "Registrador inválido: " << (int)num << std::endl;
            static std::int32_t dummy = 0;
            return dummy;
    }
}

/*
=========================================================================================
Iniciar o passo da execução da instrução atual apontada pelo PC.
Isso implica em ler o opcode e inferir o formato da instrução (1, 2, 3/4).
=========================================================================================
*/
void Maquina::passo() {
    /*
      A MEMÓRIA NÃO É LIDA POR PALAVRAS!
    */
    // Fornece um endereço do byte na memória e retorna o byte correspondente dentro da palavra
    // Ler um byte da memória
    auto lerByte = [this](std::size_t endereco_byte) -> std::uint8_t {
        return static_cast<std::uint8_t>(memoria.read(endereco_byte));
    };

    // Ler uma palavra (3 bytes) da memória, com extensão de sinal de 24→32 bits
    auto lerPalavra = [this](std::size_t endereco_byte) -> std::int32_t {
        std::uint32_t b1 = memoria.read(endereco_byte);
        std::uint32_t b2 = memoria.read(endereco_byte + 1);
        std::uint32_t b3 = memoria.read(endereco_byte + 2);
        std::uint32_t word = (b1 << 16) | (b2 << 8) | b3;
        // Extensão de sinal: 24 bits → 32 bits
        if (word & 0x800000) word |= 0xFF000000;
        return static_cast<std::int32_t>(word);
    };

    // Escrever uma palavra (3 bytes) na memória
    auto escreverPalavra = [this](std::size_t endereco_byte, std::int32_t valor) {
        memoria.setByte(endereco_byte,     (valor >> 16) & 0xFF);
        memoria.setByte(endereco_byte + 1, (valor >> 8)  & 0xFF);
        memoria.setByte(endereco_byte + 2,  valor        & 0xFF);
    };

    auto pc_inicial = cpu.r.PC;
    std::uint8_t byte1 = memoria.read(pc_inicial);

    // Resgatar os 6 bits mais significativos
    // 0xFC = 1111 1100
    std::uint8_t opcode = byte1 & 0xFC;

    /*
      ESSA É UMA INSTRUÇÃO DE FORMATO 3
    */
    // Formato do RSUB byte
    //if (opcode == 0x4C) { // RSUB (Formato 1)
    //    cpu.r.PC = cpu.r.L;
    //    std::cout << "[EXEC] RSUB - PC = " << cpu.r.PC << "\n";
    //    return; // Finaliza a execução do passo e começar novamente no PC atualizado
    //}
    
    // Formato 2 bytes
    if (opcode == 0x90 || opcode == 0x04 || opcode == 0x98 || opcode == 0xAC ||
        opcode == 0xA0 || opcode == 0x9C || opcode == 0xA4 || opcode == 0xA8 ||
        opcode == 0x94 || opcode == 0xB8) {
        cpu.r.PC += 2; // Instruções de 2 bytes
        std::uint8_t regs = memoria.read(pc_inicial + 1);
        std::uint8_t num_r1 = (regs >> 4) & 0x0F;
        std::uint8_t num_r2 = regs & 0x0F;
        
        std::int32_t& r1 = getRegistradorPorNumero(num_r1);

        switch(opcode) {
            case 0x04: { // CLEAR r1
                r1 = 0;
                std::cout << "[EXEC] CLEAR - R1: " << (int)num_r1 << " = 0\n";
                break;
            }
            case 0x90: { // ADDR r1, r2
                auto& r2 = getRegistradorPorNumero(num_r2);
                r2 += r1;
                std::cout << "[EXEC] ADDR - R2" << (int)num_r2 << " += R1" << (int)num_r1 << "\n";
                break;
            }
            case 0x98: { // MULR r1, r2
                auto& r2 = getRegistradorPorNumero(num_r2);
                r2 *= r1;
                std::cout << "[EXEC] MULR - R" << (int)num_r2 << " *= R" << (int)num_r1 << "\n";
                break;
            }
            case 0xAC: { // RMO r1, r2
                auto& r2 = getRegistradorPorNumero(num_r2);
                r2 = r1;
                std::cout << "[EXEC] RMO - R" << (int)num_r2 << " = R" << (int)num_r1 << "\n";
                break;
            }
            case 0xA0: { // COMPR r1, r2
                auto& r2 = getRegistradorPorNumero(num_r2);
                
                // Registrador especial de STATUS
                if (r1 < r2) {
                    cpu.r.SW = SMALLER;
                } else if (r1 == r2) {
                    cpu.r.SW = EQUAL;
                } else {
                    cpu.r.SW = BIGGER;
                }
                std::cout << "[EXEC] COMPR - R" << (int)num_r1 << " : R" << (int)num_r2 << "\n";
                break;
            }
            case 0x9C: { // DIVR r1, r2
                auto& r2 = getRegistradorPorNumero(num_r2);
                r2 /= r1;
                std::cout << "[EXEC] DIVR - R" << (int)num_r2 << " /= R" << (int)num_r1 << "\n";
                break;    
            }
            case 0xA4: { // SHIFTL r1, n
                auto& n = num_r2;
                // +1 para permitir deslocamentos entre 1-16
                int shift_amount = n + 1;
                r1 <<= shift_amount;
                std::cout << "[EXEC] SHIFTL - R" << (int)num_r1 << " <<= N" << (int)shift_amount << "\n";
                break;
            }
            case 0xA8: { // SHIFTR r1, n
                auto& n = num_r2;
                int shift_amount = n + 1;
                r1 >>= shift_amount;
                std::cout << "[EXEC] SHIFTR - R" << (int)num_r1 << " >>= N" << (int)shift_amount << "\n";
                break;
            }
            case 0x94: { // SUBR r1, r2
                auto& r2 = getRegistradorPorNumero(num_r2);
                r2 -= r1;
                std::cout << "[EXEC] SUBR - R" << (int)num_r2 << " -= R" << (int)num_r1 << "\n";
                break;
            }
            case 0xB8: { // TIXR r1
                cpu.r.X++;

                if (cpu.r.X < r1) {
                    cpu.r.SW = SMALLER;
                } else if (cpu.r.X == r1) {
                    cpu.r.SW = EQUAL;
                } else {
                    cpu.r.SW = BIGGER;
                }
                std::cout << "[EXEC] TIXR - X incrementado para " << cpu.r.X 
                          << ". Comparando com R" << (int)num_r1 
                          << " -> SW = " << cpu.r.SW << "\n";
                break;
            }
        }
        return;
    }

    // Formato 3/4

    // Extrair ni do primeiro Byte
    bool n = (byte1 >> 1) & 1;
    bool i = byte1 & 1;

    // Extrair xbpe do segundo Byte
    std::uint8_t byte2 = memoria.read(pc_inicial + 1);
    bool x = (byte2 >> 7) & 1;
    bool b = (byte2 >> 6) & 1;
    bool p = (byte2 >> 5) & 1;
    bool e = (byte2 >> 4) & 1;

    // disp será o último nibble do segundo Byte junto com os dois nibbles do terceiro Byte
    // formando um inteiro de 12 bits (se for instrução de 3 Bytes)
    std::int32_t disp;
    std::uint32_t target_address = 0;

    if (e) { // Formato 4
        cpu.r.PC += 4;
        std::uint8_t byte3 = memoria.read(pc_inicial + 2);
        std::uint8_t byte4 = memoria.read(pc_inicial + 3);
        disp = ((byte2 & 0x0F) << 16) | (byte3 << 8) | byte4;
        target_address = disp;
    } else { // Formato 3
        cpu.r.PC += 3;
        std::uint8_t byte3 = memoria.read(pc_inicial + 2);
        disp = ((byte2 & 0x0F) << 8) | byte3;
        
        // Eu preciso adicionar essa lógica para extensão de sinal, se o bit 11 for 1
        // então eu preciso colocar todos os bits remanescentes entre os 32 bits como 1.
        // Eu não preciso fazer isso com o base, porque ele já é unsigned.
        if (p) { // PC-relative (disp é signed 12-bit: -2048 a +2047)
            if (disp & 0x800) disp |= 0xFFFFF000; // extensão de sinal
            target_address = cpu.r.PC + disp;
        } else if (b) { // Base-relative (disp é unsigned 12-bit: 0 a 4095)
            target_address = cpu.r.B + disp;
        } else { // Direto
            target_address = disp;
        }
    }

    // Endereçamento indireto (n=1, i=0):
    // target_address aponta para um endereço na memória que contém o EA real
    if (n && !i) {
        target_address = lerPalavra(target_address) & 0xFFFFFF;
    }

    // Endereçamento indexado (x=1): aplicável com simples (n=1,i=1) ou SIC (n=0,i=0)
    if (x) {
        target_address += cpu.r.X;
    }

    // Obtenção do operando
    std::int32_t operando;

    if (!n && i) {
        // Imediato (n=0, i=1): target_address É o valor do operando
        operando = static_cast<std::int32_t>(target_address);
    } else {
        // Simples (n=1,i=1), Indireto (já resolvido), ou SIC (n=0,i=0):
        // ler palavra (3 bytes) da memória no endereço efetivo
        operando = lerPalavra(target_address);
    }

    // Execução da instrução
    switch(opcode) {
        case 0x00: { // LDA m
            cpu.r.A = operando;
            std::cout << "[EXEC] LDA - A = " << cpu.r.A << "\n";
            break;
        }
        case 0x0C: { // STA m
            escreverPalavra(target_address, cpu.r.A);
            std::cout << "[EXEC] STA - mem[" << target_address << "] = " << cpu.r.A << "\n";
            break;
        }
        case 0x18: { // ADD m
            cpu.r.A += operando;
            std::cout << "[EXEC] ADD - A = " << cpu.r.A << "\n";
            break;
        }
        case 0x3C: { // J m
            cpu.r.PC = target_address;
            std::cout << "[EXEC] J - PC = " << cpu.r.PC << "\n";
            break;
        }
        case 0x40: { // AND m
            cpu.r.A &= operando;
            std::cout << "[EXEC] AND - A = " << cpu.r.A << " : m " << (int)operando << "\n";
            break;
        }
        case 0x28: { // COMP m
            if (cpu.r.A < operando) {
                cpu.r.SW = SMALLER;
            } else if (cpu.r.A == operando) {
                cpu.r.SW = EQUAL;
            } else {
                cpu.r.SW = BIGGER;
            }

            std::cout << "[EXEC] COMPR - A" << (int)cpu.r.A << " : m " << (int)operando << "\n";
            break;
        }
        case 0x24: { // DIV m
            if (operando == 0) {
                std::cerr << "[ERRO] Tentativa de divisão por zero, abortando processo";
                return;
            }
            cpu.r.A /= operando;
            std::cout << "[EXEC] DIV - A = " << cpu.r.A << "\n";
            break;
        }
        case 0x30: { // JEQ m
            if (cpu.r.SW == EQUAL) {
                cpu.r.PC = target_address;
            }
            std::cout << "[EXEC] JEQ";
            break;
        }
        case 0x34: { // JGT m
            if (cpu.r.SW == BIGGER) {
                cpu.r.PC = target_address;
            }
            std::cout << "[EXEC] JGT";
            break;
        }
        case 0x38: { // JLT m
            if (cpu.r.SW == SMALLER) {
                cpu.r.PC = target_address;
            }
            std::cout << "[EXEC] JLT";
            break;
        }
        case 0x48: { // JSUB m
            cpu.r.L = cpu.r.PC;
            cpu.r.PC = target_address;
            std::cout << "[EXEC] JSUB - L = " << cpu.r.L << ", PC = " << cpu.r.PC << "\n";
            break;
        }
        case 0x68: { // LDB m
            cpu.r.B = operando;
            std::cout << "[EXEC] LDB - B = " << cpu.r.B << "\n";
            break;
        }
        case 0x50: { // LDCH m
            auto byte_carregado = lerByte(target_address);
            auto a_preservado = cpu.r.A & 0xFFFF00;
            cpu.r.A = a_preservado | byte_carregado;
            std::cout << "[EXEC] LDCH - A = " << cpu.r.A << "\n";
            break;
        }
        case 0x08: { // LDL m
            cpu.r.L = operando;
            std::cout << "[EXEC] LDL - L = " << cpu.r.L << "\n";
            break;
        }
        case 0x6C: { // LDS m
            cpu.r.S = operando;
            std::cout << "[EXEC] LDS - S = " << cpu.r.S << "\n";
            break;
        }
        case 0x74: { // LDT m
            cpu.r.T = operando;
            std::cout << "[EXEC] LDT - T = " << cpu.r.T << "\n";
            break;
        }
        case 0x04: { // LDX m
            cpu.r.X = operando;
            std::cout << "[EXEC] LDX - X = " << cpu.r.X << "\n";
            break;
        }
        case 0x20: { // MUL m
            cpu.r.A *= operando;
            std::cout << "[EXEC] MUL - A = " << cpu.r.A << "\n";
            break;
        }
        case 0x44: { // OR m
            cpu.r.A |= operando;
            std::cout << "[EXEC] OR - A = " << cpu.r.A << "\n";
            break;
        }
        case 0x4C: { // RSUB m
            cpu.r.PC = cpu.r.L;
            std::cout << "[EXEC] RSUB - PC = " << cpu.r.PC << "\n";
            break;
        }
        case 0x78: { // STB m 
            escreverPalavra(target_address, cpu.r.B);
            std::cout << "[EXEC] STB - mem[" << target_address << "] = " << cpu.r.B << "\n";
            break;
        }
        case 0x54: { // STCH m
            std::uint8_t byte_para_armazenar = cpu.r.A & 0xFF;
            memoria.setByte(target_address, byte_para_armazenar);
            std::cout << "[EXEC] STCH - mem[" << target_address << "] = " << (int)byte_para_armazenar << "\n";
            break;
        }
        case 0x14: { // STL m
            escreverPalavra(target_address, cpu.r.L);
            std::cout << "[EXEC] STL - mem[" << target_address << "] = " << cpu.r.L << "\n";
            break;
        }
        case 0x7C: { // STS m
            escreverPalavra(target_address, cpu.r.S);
            std::cout << "[EXEC] STS - mem[" << target_address << "] = " << cpu.r.S << "\n";
            break;
        }
        case 0x84: { // STT m
            escreverPalavra(target_address, cpu.r.T);
            std::cout << "[EXEC] STT - mem[" << target_address << "] = " << cpu.r.T << "\n";
            break;  
        }
        case 0x10: { // STX m
            escreverPalavra(target_address, cpu.r.X);
            std::cout << "[EXEC] STX - mem[" << target_address << "] = " << cpu.r.X << "\n";
            break;
        }
        case 0x1C: { // SUB m
            cpu.r.A -= operando;
            std::cout << "[EXEC] SUB - A = " << cpu.r.A << "\n";
            break;
        }
        case 0x2C: { // TIX m
            cpu.r.X++;
            if (cpu.r.X < operando) {
                cpu.r.SW = SMALLER;
            } else if (cpu.r.X == operando) {
                cpu.r.SW = EQUAL;
            } else {
                cpu.r.SW = BIGGER;
            }
            std::cout << "[EXEC] TIX - X incrementado para " << cpu.r.X 
                      << ". Comparando com m " << (int)operando
                      << " -> SW = " << cpu.r.SW << "\n";
            break;
        }
        default:
             std::cerr << "[ERRO] Opcode F3/F4 não implementado: 0x" << std::hex << (int)opcode << std::dec << std::endl;
    }
}