#include "Maquina_melhor.h"
#include <stdexcept> 

Maquina::Maquina(std::size_t tamanho_memoria) : memoria(tamanho_memoria){}

/*
=========================================================================================
Carregar o programa na memória a partir de um arquivo binário.
=========================================================================================
*/
void Maquina::carregarPrograma(const std::string& caminhoArquivo) {
    std::ifstream arquivo(caminhoArquivo, std::ios::binary);
    if (!arquivo) {
        std::cerr << "Erro ao abrir o arquivo: " << caminhoArquivo << std::endl;
        return;
    }

    std::size_t endereco = 0;
    int byte;
    while((byte = arquivo.get()) != EOF) {
        memoria.setByte(endereco++, static_cast<std::uint8_t>(byte));   
    }

    // início do programa
    cpu.r.PC = 0;
    m_running = false; // Garante que não esteja rodando após carregar
}

/*
=========================================================================================
Começar o loop de execução da máquina. Agora controlado pelo flag m_running.
=========================================================================================
*/
void Maquina::executar() {
    // Inicializa o flag de execução
    m_running = true; 
    
    while(m_running){ // Loop controlado pelo flag
        try {
            passo();
            // A condição de parada (PC fora dos limites) é verificada dentro de passo()
        } catch (const std::exception& e) {
            std::cerr << "Erro fatal durante a execucao: " << e.what() << std::endl;
            m_running = false;
        }
    }
}

/*
=========================================================================================
Retornar uma referência direta para um dos registradores.
CORREÇÃO: Lança uma exceção no default para evitar o warning e garantir retorno.
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
            // Lança uma exceção (runtime_error exige #include <stdexcept>)
            throw std::runtime_error("Tentativa de acessar registrador invalido.");
    }
}

/*
=========================================================================================
Iniciar o passo da execução da instrução atual.
=========================================================================================
*/
void Maquina::passo() {
    // Fornece um endereço do byte na memória
    auto lerByte = [this](std::size_t endereco_byte) -> std::uint8_t {
        const std::vector<std::uint8_t>& m_bytes = memoria.getMBytes();
        if (endereco_byte < m_bytes.size()) {
            return m_bytes[endereco_byte];
        }
        return 0; // Retorna 0 se fora dos limites
    };

    // Ler uma palavra (3 bytes) da memória a partir de um endereço de byte
    auto lerPalavra = [this, &lerByte](std::size_t endereco_byte) -> std::uint32_t {
        if (endereco_byte + 2 >= memoria.getTamanhoBytes()) {
            std::cerr << "ERRO: Tentativa de ler palavra fora dos limites da memória em 0x" << std::hex << endereco_byte << std::dec << std::endl;
            return 0;
        }

        std::uint8_t b1 = lerByte(endereco_byte);
        std::uint8_t b2 = lerByte(endereco_byte + 1);
        std::uint8_t b3 = lerByte(endereco_byte + 2);
        return (b1 << 16) | (b2 << 8) | b3;
    };

    // Escrever uma palavra (3 bytes) na memória a partir de um endereço de byte
    auto escreverPalavra = [this](std::size_t endereco_byte, std::uint32_t valor) {
        memoria.setByte(endereco_byte, (valor >> 16) & 0xFF);
        memoria.setByte(endereco_byte + 1, (valor >> 8) & 0xFF);
        memoria.setByte(endereco_byte + 2, valor & 0xFF);
    };

    std::size_t pc_inicial = cpu.r.PC;
    
    // VERIFICAÇÃO DE LIMITE CRÍTICO
    if (pc_inicial >= memoria.getTamanhoBytes()) {
        std::cerr << "[FIM] PC fora dos limites da memória (PC = 0x" << std::hex << pc_inicial << std::dec << ")\n";
        m_running = false; // Desliga o flag se PC for inválido
        return;
    }

    std::uint8_t byte1 = lerByte(pc_inicial);

    std::uint8_t opcode = byte1 & 0xFC;

    // Formato 1 byte
    if (opcode == 0x4C) { // RSUB (Formato 1)
        cpu.r.PC = cpu.r.L;
        std::cout << "[EXEC] RSUB - PC = " << cpu.r.PC << "\n";
        m_running = false; // **CONDIÇÃO DE PARADA**
        return; 
    }
    
    // Formato 2 bytes
    if (opcode == 0x90 || opcode == 0x04 || opcode == 0x98 || opcode == 0xAC ||
        opcode == 0xA0 || opcode == 0x9C || opcode == 0xA4 || opcode == 0xA8 ||
        opcode == 0x94 || opcode == 0xB8) {
        cpu.r.PC += 2; 
        std::uint8_t regs = lerByte(pc_inicial + 1);
        std::uint8_t num_r1 = (regs >> 4) & 0x0F;
        std::uint8_t num_r2 = regs & 0x0F;
        
        try {
            std::int32_t& r1 = getRegistradorPorNumero(num_r1);
            
            switch(opcode) {
                case 0x04: { // CLEAR r1
                    r1 = 0;
                    std::cout << "[EXEC] CLEAR - R" << (int)num_r1 << " = 0\n";
                    break;
                }
                case 0x90: { // ADDR r1, r2
                    std::int32_t& r2 = getRegistradorPorNumero(num_r2);
                    r2 += r1;
                    std::cout << "[EXEC] ADDR - R" << (int)num_r2 << " += R" << (int)num_r1 << "\n";
                    break;
                }
                case 0x98: { // MULR r1, r2
                    std::int32_t& r2 = getRegistradorPorNumero(num_r2);
                    r2 *= r1;
                    std::cout << "[EXEC] MULR - R" << (int)num_r2 << " *= R" << (int)num_r1 << "\n";
                    break;
                }
                case 0xAC: { // RMO r1, r2
                    std::int32_t& r2 = getRegistradorPorNumero(num_r2);
                    r2 = r1;
                    std::cout << "[EXEC] RMO - R" << (int)num_r2 << " = R" << (int)num_r1 << "\n";
                    break;
                }
                case 0xA0: { // COMPR r1, r2
                    std::int32_t& r2 = getRegistradorPorNumero(num_r2);
                    
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
                    std::int32_t& r2 = getRegistradorPorNumero(num_r2);
                    if (r1 == 0) throw std::runtime_error("Divisao por zero em DIVR.");
                    r2 /= r1;
                    std::cout << "[EXEC] DIVR - R" << (int)num_r2 << " /= R" << (int)num_r1 << "\n";
                    break;    
                }
                case 0xA4: { // SHIFTL r1, n
                    auto& n = num_r2;
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
                    std::int32_t& r2 = getRegistradorPorNumero(num_r2);
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
        } catch (const std::exception& e) {
            std::cerr << "ERRO de registrador em Formato 2: " << e.what() << std::endl;
        }

        return;
    }

    // Formato 3/4

    // Extrair ni do primeiro Byte
    bool n = (byte1 >> 1) & 1;
    bool i = byte1 & 1;

    // Extrair xbpe do segundo Byte
    std::uint8_t byte2 = lerByte(pc_inicial + 1);
    bool x = (byte2 >> 7) & 1;
    bool b = (byte2 >> 6) & 1;
    bool p = (byte2 >> 5) & 1;
    bool e = (byte2 >> 4) & 1;

    std::int32_t disp;
    std::uint32_t target_address = 0;

    if (e) { // Formato 4
        if (pc_inicial + 3 >= memoria.getTamanhoBytes()) {
            std::cerr << "ERRO: Leitura do Formato 4 fora dos limites.\n";
            return;
        }
        cpu.r.PC += 4;
        std::uint8_t byte3 = lerByte(pc_inicial + 2);
        std::uint8_t byte4 = lerByte(pc_inicial + 3);
        disp = ((byte2 & 0x0F) << 16) | (byte3 << 8) | byte4;
        target_address = disp;
    } else { // Formato 3
        if (pc_inicial + 2 >= memoria.getTamanhoBytes()) {
            std::cerr << "ERRO: Leitura do Formato 3 fora dos limites.\n";
            return;
        }
        cpu.r.PC += 3;
        std::uint8_t byte3 = lerByte(pc_inicial + 2);
        disp = ((byte2 & 0x0F) << 8) | byte3;
        // Extensão de sinal para deslocamento de 12 bits (para PC e Base relative)
        if (disp & 0x800) {
            disp |= 0xFFFFF000;
        }

        if (p) { // PC-relative
            target_address = cpu.r.PC + disp;
        } else if (b) { // Base-relative
            target_address = cpu.r.B + disp;
        } else { // Direto
            target_address = disp;
        }
    }

    // Endereçamento indexado
    if (x) {
        target_address += cpu.r.X;
    }

    // Obtenção do operando
    std::uint32_t operando;

    // i==1 então Imediato
    if (i) {
        operando = target_address; 
    } else {
        std::uint32_t endereco_efetivo = target_address;
        
        // endereço de um ponteiro
        if (n) { 
            endereco_efetivo = lerPalavra(target_address);
        }
        // endereço do operando
        operando = lerPalavra(endereco_efetivo);
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
            if (target_address >= memoria.getTamanhoBytes()) {
                std::cerr << "ERRO: LDCH fora dos limites.\n";
                return;
            }
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
            m_running = false; // **CONDIÇÃO DE PARADA**
            break;
        }
        case 0x78: { // STB m 
            escreverPalavra(target_address, cpu.r.B);
            std::cout << "[EXEC] STB - mem[" << target_address << "] = " << cpu.r.B << "\n";
            break;
        }
        case 0x54: { // STCH m
            if (target_address >= memoria.getTamanhoBytes()) {
                std::cerr << "ERRO: STCH fora dos limites.\n";
                return;
            }
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
             // Lança exceção para tratamento de erro não implementado
             throw std::runtime_error("Opcode F3/F4 nao implementado ou invalido.");
    }
}