#include "Maquina.h"

Maquina::Maquina(std::size_t tamanho_memoria) : memoria(tamanho_memoria){}

/*
======================================
    Função defasada, usar Loader.cpp
======================================
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
//     cpu.r.PC = 0; //inicio do programa
// 
// }

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

void Maquina:: passo(){
    //le opcode
    auto lerByte = [this](std::size_t endereco_byte) -> std::uint8_t {
        //cada palavra tem 3 bytes
        std::size_t palavra = endereco_byte / 3;    
        std::size_t deslocamento = endereco_byte % 3;
        std::size_t valor = memoria.read(palavra);
        return (valor >> (16 - 8 * deslocamento)) & 0xFF;
    };

    std::uint8_t opcode = lerByte(cpu.r.PC);    
    cpu.r.PC += 1;  

    switch(opcode){
        
        
        //LDA - Load Accumulator
        case 0x00: {
            std::uint32_t endereco_operando = memoria.read(cpu.r.PC / 3);
            cpu.r.PC += 3; //avança 3 bytes (1 palavra)

            cpu.r.A = memoria.read(endereco_operando);
              std::cout << "[EXEC] LDA - A = " << cpu.r.A
              << " (mem[" << endereco_operando << "])\n";
            break;
        }
        //STA - Store Accumulator
        case 0x0C: {
            std::uint32_t endereco_operando = memoria.read(cpu.r.PC / 3);
            cpu.r.PC += 3;

            memoria.write(endereco_operando, cpu.r.A);
              std::cout << "[EXEC] STA - mem[" << endereco_operando << "] = " << cpu.r.A << "\n";
            break;
        }
        //RSUB - Return from Subroutine
        case 0x4C: {
            cpu.r.PC = cpu.r.L;
              std::cout << "[EXEC] RSUB - PC = " << cpu.r.PC << "\n";
              break;
        }

        case 0x18: { //ADD - Soma
            std::uint32_t endereco_operando = memoria.read(cpu.r.PC / 3);
            cpu.r.PC += 3;  

            cpu.r.A += memoria.read(endereco_operando);
              std::cout << "[EXEC] ADD - A = " << cpu.r.A
              << " (mem[" << endereco_operando << "])\n";
            break;
        }
        case 0x3C: { //JMP - Jump
            std::uint32_t endereco_destino = memoria.read(cpu.r.PC / 3);
            cpu.r.PC = endereco_destino;  

              std::cout << "[EXEC] JMP - PC = " << cpu.r.PC << "\n";
            break;
        }
        
        default:
            std::cerr << "[ERRO] Opcode não implementado: 0x"
            << std::hex << std::setw(2) << std::setfill('0')
            << (int)opcode << std::dec << std::endl;
            cpu.r.PC += 1; // evita travar
            break;
    }


}