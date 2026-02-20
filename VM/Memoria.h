#ifndef VM_SIC_MEMORY_H
#define VM_SIC_MEMORY_H


#include <cstdint>
#include <vector>
#include <iostream>

constexpr std::size_t MEMORIA_TAMANHO = 16384; // 16 Kb

class Memoria {
public:
    std::vector<std::uint8_t> m_bytes;

    Memoria(std::size_t tamanhoEmBytes = MEMORIA_TAMANHO) {
        m_bytes.resize(tamanhoEmBytes, 0);
    };
    std::uint32_t read(std::size_t endereço_palavra) const;
    void write(std::size_t endereço_byte, std::uint8_t valor_byte);

    void setByte(std::size_t endereco_byte, std::uint8_t valor) {
      if (endereco_byte < m_bytes.size()) {
        m_bytes[endereco_byte] = valor;
      } 
    } 

    void limpar() {
      std::fill(m_bytes.begin(), m_bytes.end(), 0);
    }

    std:: size_t getTamanhoBytes() const{
        return m_bytes.size();
    }
};


#endif