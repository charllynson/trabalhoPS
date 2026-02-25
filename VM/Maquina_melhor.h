#ifndef VM_SIC_MAQUINA_H
#define VM_SIC_MAQUINA_H

#include "CPU.h"
#include "Memoria.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <functional>

// Status retornado por passo()
enum class StepResult { OK, HALT, ERROR };

class Maquina{
    public:
    CPU cpu;
    Memoria memoria;

    // Callback opcional de log — a GUI registra o dela, o CLI usa cout
    std::function<void(const std::string&)> logCallback;

    Maquina(std::size_t tamanho_memoria);
    void carregarPrograma(const std::string& caminhoArquivo);
    void executar();
    StepResult passo();

    // Helper: formata msg e envia pro callback (ou cout se não houver)
    void log(const std::string& msg);

    private:
    std::int32_t& getRegistradorPorNumero(std::uint8_t num);
};




#endif