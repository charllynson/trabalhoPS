//
// Created by jsfonseca on 21/10/2025.
//

#include "Memoria.h"

// lê 3 bytes da memória e combina eles em um inteiro de 32 bits
std::uint32_t Memoria::read(std::size_t endereço_palavra) const {
    std::size_t endereço_byte = endereço_palavra * 3;

    // pega os 3 bytes individuais
    std::uint8_t byte1 = m_bytes[endereço_byte]; // byte mais significativo
    std::uint8_t byte2 = m_bytes[endereço_byte + 1];
    std::uint8_t byte3 = m_bytes[endereço_byte + 2]; // byte menos significativo

    // combina os 3 bytes em um único inteiro usando SHIFT e OR
    std::int32_t valor_palavra = (byte1 << 16) | (byte2 << 8) | byte3;

    return valor_palavra;
}

void Memoria::write(std::size_t endereço_palavra, std::int32_t valor) {
    size_t endereço_byte = endereço_palavra * 3;

    // cada linha desloca os 8 bits corretos, e faz uma AND com 0xFF para apagar o resto
    m_bytes[endereço_byte]     = (valor >> 16) & 0xFF;
    m_bytes[endereço_byte + 1] = (valor >> 8)  & 0xFF;
    m_bytes[endereço_byte + 2] = valor         & 0xFF;
}