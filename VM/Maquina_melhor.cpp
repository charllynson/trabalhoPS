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
}

/*
=========================================================================================
Começar o loop de execução da máquina assim que o programa é carregado.
=========================================================================================
*/
void Maquina::executar() {
    bool rodando = true;
    while(rodando){
        passo();
        //se PC ultrapassar o tamanho da memoria, para a execução
        if(cpu.r.PC >= memoria.getTamanhoBytes()) {
            std::cout <<"Fim da execução " << std::endl;
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
        case 0: return cpu.r.A;
        case 1: return cpu.r.X;
        case 2: return cpu.r.L;
        case 3: return cpu.r.B;
        case 4: return cpu.r.S;
        case 5: return cpu.r.T;
        case 8: return cpu.r.PC; // contador de programa em Byte
        case 9: return cpu.r.SW; // código condicional CC
        default:
            std::cerr << "Registrador inválido: " << (int)num << std::endl;
    }
}

/*
=========================================================================================
Iniciar o passo da execução da instrução atual apontada pelo PC.
Isso implica em ler o opcode e inferir o formato da instrução (1, 2, 3/4).
=========================================================================================
*/
void Maquina::passo() {
    // Fornece um endereço do byte na memória e retorna o byte correspondente dentro da palavra
    auto lerByte = [this](std::size_t endereco_byte) -> std::uint8_t {
        std::size_t palavra_idx = endereco_byte / 3;
        std::size_t deslocamento = endereco_byte % 3;
        std::uint32_t valor = memoria.read(palavra_idx);
        return (valor >> (16 - 8 * deslocamento)) & 0xFF;
    };

    // Ler uma palavra (3 bytes) da memória a partir de um endereço de byte
    auto lerPalavra = [this, &lerByte](std::size_t endereco_byte) -> std::uint32_t {
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
    std::uint8_t byte1 = lerByte(pc_inicial);

    // Resgatar os 6 bits mais significativos
    std::uint8_t opcode = byte1 & 0xFC;

    // Formato 1 byte
    if (opcode == 0x4C) { // RSUB (Formato 1)
        cpu.r.PC = cpu.r.L;
        std::cout << "[EXEC] RSUB - PC = " << cpu.r.PC << "\n";
        return; // Finaliza a execução do passo
    }
    
    // Formato 2 bytes
    if (opcode == 0x90 || opcode == 0x04 || opcode == 0x98 || opcode == 0xAC) {
        cpu.r.PC += 2; // Instruções de 2 bytes
        std::uint8_t regs = lerByte(pc_inicial + 1);
        std::uint8_t num_r1 = (regs >> 4) & 0x0F;
        std::uint8_t num_r2 = regs & 0x0F; // r2 é o nibble da direita para ADDR, etc.
        
        std::int32_t& r1 = getRegistradorPorNumero(num_r1);

        switch(opcode) {
            case 0x04: // CLEAR r1
                r1 = 0;
                std::cout << "[EXEC] CLEAR - R" << (int)num_r1 << " = 0\n";
                break;
            case 0x90: { // ADDR r2, r1
                std::int32_t& r2_ref = getRegistradorPorNumero(num_r2);
                r1 += r2_ref; // A convenção é r2 = r2 + r1, mas o seu código soma em r1. Ajuste se necessário.
                std::cout << "[EXEC] ADDR - R" << (int)num_r1 << " += R" << (int)num_r2 << "\n";
                break;
            }
        }
        return;
    }

    // Formato 3/4
    bool n = (byte1 >> 1) & 1;
    bool i = byte1 & 1;

    std::uint8_t byte2 = lerByte(pc_inicial + 1);
    
    bool x = (byte2 >> 7) & 1;
    bool b = (byte2 >> 6) & 1;
    bool p = (byte2 >> 5) & 1;
    bool e = (byte2 >> 4) & 1;

    std::int32_t disp;
    std::uint32_t target_address = 0;

    if (e) { // Formato 4
        cpu.r.PC += 4;
        std::uint8_t byte3 = lerByte(pc_inicial + 2);
        std::uint8_t byte4 = lerByte(pc_inicial + 3);
        disp = ((byte2 & 0x0F) << 16) | (byte3 << 8) | byte4;
        target_address = disp;
    } else { // Formato 3
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

    // Imediato
    if (i) {
        operando = target_address; // O "endereço" é o próprio valor
    } else {
        // Se for indireto, busca o endereço final na memória
        std::uint32_t endereco_efetivo = target_address;
        if (n) { 
            endereco_efetivo = lerPalavra(target_address);
        }
        operando = lerPalavra(endereco_efetivo);
    }

    // Execução da instrução
    switch(opcode) {
        case 0x00: // LDA
            cpu.r.A = operando;
            std::cout << "[EXEC] LDA - A = " << cpu.r.A << "\n";
            break;
        case 0x0C: // STA
            memoria.write(target_address, cpu.r.A);
            std::cout << "[EXEC] STA - mem[" << target_address << "] = " << cpu.r.A << "\n";
            break;
        case 0x18: // ADD
            cpu.r.A += operando;
            std::cout << "[EXEC] ADD - A = " << cpu.r.A << "\n";
            break;
        case 0x3C: // JMP
            cpu.r.PC = target_address;
            std::cout << "[EXEC] JMP - PC = " << cpu.r.PC << "\n";
            break;
        default:
             std::cerr << "[ERRO] Opcode F3/F4 não implementado: 0x" << std::hex << (int)opcode << std::dec << std::endl;
    }
}