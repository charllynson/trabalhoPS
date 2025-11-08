#include "Maquina.h"

Maquina::Maquina(std::size_t tamanho_memoria) : memoria(tamanho_memoria){}

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

    cpu.r.PC = 0; //inicio do programa

}

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

std::int32_t& Maquina::getRegistradorPorNumero(std::uint8_t num) {
    switch (num) {
        case 0: return cpu.r.A;
        case 1: return cpu.r.X;
        case 2: return cpu.r.L;
        case 3: return cpu.r.B;
        case 4: return cpu.r.S;
        case 5: return cpu.r.T;
        default:
            // Lançar uma exceção ou tratar o erro é uma boa prática
            std::cerr << "Registrador inválido: " << (int)num << std::endl;
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


        case 0x90: { //ADDR - Add to Register
            // 1011 1001
            // fazer shift 0000 1011
            // fazer and para separar 1011
            std::uint8_t registradores = lerByte(cpu.r.PC);
            cpu.r.PC += 1;

            std::uint8_t num_reg1 = (registradores >> 4) & 0x0F;
            std::uint8_t num_reg2 = registradores & 0x0F;

            std::int32_t& r1 = getRegistradorPorNumero(num_reg1);
            std::int32_t& r2 = getRegistradorPorNumero(num_reg2);

            r2 += r1;

            std::cout << "[EXEC] ADDR - R" << (int)num_reg2 << " = " << num_reg2
            << " + (R" << (int)num_reg1 << ")\n"; //TODO: arrumar cout
        }

        case 0x04: { //CLEAR Set register to 0
            std::uint8_t registradores = lerByte(cpu.r.PC);
            cpu.r.PC += 1;

            std::uint8_t num_reg1 = (registradores >> 4) & 0x0F;
            
            std::int32_t& r1 = getRegistradorPorNumero(num_reg1);

            r1 = 0;

            std::cout << "[EXEC] CLEAR - R" << (int)num_reg1 << " = " << "0\n"; //TODO: arrumar cout
        }

        case 0x98: { //MULR Multiply to Register
            std::uint8_t registradores = lerByte(cpu.r.PC);
            cpu.r.PC += 1;

            std::uint8_t num_reg1 = (registradores >> 4) & 0x0F;
            std::uint8_t num_reg2 = registradores & 0x0F; // mulr registrador registrador = nnnn(num_reg2)+nnnn(num_reg1) 

            std::int32_t& r1 = getRegistradorPorNumero(num_reg2);
            std::int32_t& r2 = getRegistradorPorNumero(num_reg1);

            r2 *= r1;

            std::cout << "[EXEC] MULR - R" << (int)num_reg2 << " = " << r2
            << " (R" << (int)num_reg1 << ")\n"; //TODO: arrumar cout
        }

        case 0xAC: { //RMO Register Move
            std::uint8_t registradores = lerByte(cpu.r.PC);
            cpu.r.PC += 1;

            std::uint8_t num_reg1 = (registradores >> 4) & 0x0F;
            std::uint8_t num_reg2 = registradores & 0x0F;

            std::int32_t& r1 = getRegistradorPorNumero(num_reg2);
            std::int32_t& r2 = getRegistradorPorNumero(num_reg1);

            r2 = r1;

            std::cout << "[EXEC] DIVR - R" << (int)num_reg2 << " = " << (int)num_reg1 << "\n"; //TODO: arrumar cout
        }

        case 0x9C: { //DIVR Divison to Register
            std::uint8_t registradores = lerByte(cpu.r.PC);
            cpu.r.PC += 1;

            std::uint8_t num_reg1 = (registradores >> 4) & 0x0F;
            std::uint8_t num_reg2 = registradores & 0x0F;

            std::int32_t& r1 = getRegistradorPorNumero(num_reg2);
            std::int32_t& r2 = getRegistradorPorNumero(num_reg1);

            r2 /= r1;

            std::cout << "[EXEC] DIVR - R" << (int)num_reg2 << " /= " << (int)num_reg1 << "\n"; //TODO: arrumar cout
        }
        
        case 0xA0: { //COMPR Compare registers
            std::uint8_t registradores = lerByte(cpu.r.PC);
            cpu.r.PC += 1;

            std::uint8_t num_reg1 = (registradores >> 4) & 0x0F;
            std::uint8_t num_reg2 = registradores & 0x0F;

            std::int32_t& r1 = getRegistradorPorNumero(num_reg2);
            std::int32_t& r2 = getRegistradorPorNumero(num_reg1);

            auto resultado = r1 - r2;

            if (resultado == 0) {
                cpu.r.SW = 0;
            } else if (resultado < 0) {
                cpu.r.SW = -1;
            } else {
                cpu.r.SW = 1;
            }

            std::cout << "[EXEC] COMPR - R" << (int)num_reg1 << " : " << (int)num_reg2 << "\n"; //TODO: arrumar cout
        }
        
        default:
            std::cerr << "[ERRO] Opcode não implementado: 0x"
            << std::hex << std::setw(2) << std::setfill('0')
            << (int)opcode << std::dec << std::endl;
            cpu.r.PC += 1; // evita travar
            break;
    }


}