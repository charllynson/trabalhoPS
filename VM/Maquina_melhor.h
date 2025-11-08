#ifndef VM_SIC_MAQUINA_H
#define VM_SIC_MAQUINA_H

#include "CPU.h"
#include "Memoria.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

class Maquina{
    private: CPU cpu;
    Memoria memoria;

    public: 
    explicit Maquina(std::size_t tamanho_memoria = 1024);
    void carregarPrograma(const std::string& caminhoArquivo);
    void executar();
    void passo();
    std::int32_t& getRegistradorPorNumero(std::uint8_t num);

};




#endif