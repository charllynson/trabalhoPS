#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include "Maquina_melhor.h"

class Ligador {
public:
  Ligador() {}

  // Ligador-Relocador: recebe lista de caminhos para arquivos .obj
  // e o endereço de carga (PROGADDR). Gera imagem binária absoluta.
  bool LigadorRelocador(std::vector<std::string> modulos, std::uint32_t progaddr);
  void CarregadorAbsoluto(Maquina& maquina);

  // Imagem de memória do programa ligado (público p/ inspeção)
  std::vector<std::uint8_t> memoria;

private:
  // ESTAB: tabela de símbolos externos (nome → endereço absoluto)
  std::unordered_map<std::string, std::uint32_t> ESTAB;

  // Endereço de execução (definido pelo primeiro módulo com registro E)
  std::uint32_t execAddr = 0;

  // Tamanho total do programa ligado
  std::uint32_t totalLength = 0;

  // Endereço de carga
  std::uint32_t progAddr = 0;

  // ---- helpers de parsing ----

  // Converte uma string hexadecimal para uint32_t
  std::uint32_t hexToUint(const std::string& hex) {
    return static_cast<std::uint32_t>(std::stoul(hex, nullptr, 16));
  }

  // Formata um inteiro em hexadecimal com padding de zeros à esquerda
  std::string toHex(std::uint32_t value, int width) {
    std::ostringstream oss;
    oss << std::uppercase << std::hex
        << std::right << std::setw(width) << std::setfill('0')
        << value;
    return oss.str();
  }

  // Lê N nibbles (half-bytes) da memória a partir de um endereço de byte.
  // Se N é ímpar, o nibble mais significativo é o nibble baixo do primeiro byte.
  // Retorna o valor com extensão de sinal para 32 bits.
  int32_t readField(std::uint32_t addr, int nibbles);

  // Escreve um valor de volta na memória, preservando nibbles que não fazem parte do campo.
  void writeField(std::uint32_t addr, int nibbles, int32_t value);
};