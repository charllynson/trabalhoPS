#include "Memoria.h"

std::uint32_t Memoria::read(std::size_t endereço_byte) const {
  if (endereço_byte < this->getTamanhoBytes()) return m_bytes[endereço_byte];
  std::cerr << "Erro ao ler a memória no endereço " << endereço_byte << " sendo que o tamanho é " << getTamanhoBytes() << "\n";
  return -1;
}

void Memoria::write(std::size_t endereço_byte, std::uint8_t valor_byte) {
  if (endereço_byte < this->getTamanhoBytes()) m_bytes[endereço_byte] = valor_byte;
}