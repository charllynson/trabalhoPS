//
// Created by jsfonseca on 21/10/2025.
//

#ifndef VM_SIC_MEMORY_H
#define VM_SIC_MEMORY_H


#include <cstdint>
#include <vector>

constexpr std::size_t MEMORIA_TAMANHO = 131072; // 32KB

class Memoria {
private:
    std::vector<std::uint8_t> m_bytes;

public:
    Memoria(std::size_t tamanho_em_palavras = MEMORIA_TAMANHO) {
        m_bytes.resize(tamanho_em_palavras * 3, 0);
    };
    std::uint32_t read(std::size_t endereço_palavra) const;
    void write(std::size_t endereço_palavra, std::int32_t valor);
};


#endif //VM_SIC_MEMORY_H