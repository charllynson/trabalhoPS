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

    void setByte(std::size_t endereco_byte, std::uint8_t valor) {
       // Correção do erro lógico: só escreve se o endereço estiver dentro dos limites.
       if(endereco_byte < m_bytes.size()) { 
        m_bytes[endereco_byte] = valor;
    } } 

    std:: size_t getTamanhoBytes() const{
        return m_bytes.size();
    }
    
    // NOVO ACCESSOR PARA A GUI
    const std::vector<std::uint8_t>& getMBytes() const {
        return m_bytes;
    }
};


#endif //VM_SIC_MEMORY_H a