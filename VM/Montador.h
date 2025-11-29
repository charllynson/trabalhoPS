#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "Maquina_melhor.h"
#include "Memoria.h"
#include "CPU.h"


class Montador {
public:
    Montador() = default;
    // Vai ler o arquivo fonte e retornar o código objeto como um vetor de bytes
    std::vector<std::uint8_t> montar(const std::string& caminhoArquivoFonte);
    
private:
};