#ifndef VM_SIC_MAQUINA_H
#define VM_SIC_MAQUINA_H

#include "CPU.h"
#include "Memoria.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <stdexcept>

class Maquina{
    private: 
    CPU cpu;
    Memoria memoria;
    bool m_running = false; // NOVO: Flag para controlar o ciclo de execução

    public: 
    explicit Maquina(std::size_t tamanho_memoria = 1024);
    void carregarPrograma(const std::string& caminhoArquivo);
    void executar();
    void passo();
    std::int32_t& getRegistradorPorNumero(std::uint8_t num);

    // ACCESSORS PARA A GUI
    CPU& getCPU() { return cpu; }
    const CPU& getCPU() const { return cpu; }
    Memoria& getMemoria() { return memoria; }
    const Memoria& getMemoria() const { return memoria; }
    
    // NOVO: Verifica se a VM está rodando
    bool is_running() const { return m_running; }
};

#endif