#ifndef VM_SIC_MAQUINA_H
#define VM_SIC_MAQUINA_H

#include "CPU.h"
#include "Memoria.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

class Maquina{
    public:
    CPU cpu;
    Memoria memoria;
 
    Maquina(std::size_t tamanho_memoria);
    void carregarPrograma(const std::string& caminhoArquivo);
    void executar();
    void passo();

    private:
    std::int32_t& getRegistradorPorNumero(std::uint8_t num);
};




#endif