#pragma once

#include "CPU.h"
#include "Memoria.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <ranges>
#include <algorithm>


class MontadorSemVibecode {
public:
    MontadorSemVibecode();
    ~MontadorSemVibecode() = default;

    bool montar(const std::string& caminhoArquivoFonte, const std::string& caminhoArquivoDestino);

private:
  std::unordered_set<std::string> directives = {
    "START",
    "END",
    "BYTE",
    "WORD",
    "RESB",
    "RESW"
  };
  std::unordered_map<std::string, std::uint32_t> instruçoesFormato2 = {
      {"ADDR", 0x90},
      {"CLEAR", 0x4},
      {"COMPR", 0xA0},
      {"DIVR", 0x9C},
      {"MULR", 0x98},
      {"RMO", 0xAC},
      {"SHIFTL", 0xA4},
      {"SHIFTR", 0xA8},
      {"SUBR", 0x94},
      {"TIXR", 0xB8}
  };
  std::unordered_map<std::string, std::uint32_t> instruçoesFormato34 = {
      {"ADD", 0x18},
      {"AND", 0x40},
      {"COMP", 0x28},
      {"DIV", 0x24},
      {"J", 0x3C},
      {"JEQ", 0x30},
      {"JGT", 0x34},
      {"JLT", 0x38},
      {"JSUB", 0x48},
      {"LDA", 0x0},
      {"LDB", 0x68},
      {"LDCH",0x50},
      {"LDL", 0x8},
      {"LDS", 0x6C},
      {"LDT", 0x74},
      {"LDX", 0x4},
      {"MUL", 0x20},
      {"OR", 0x44},
      {"RSUB", 0x4C},
      {"STA", 0x0C},
      {"STB", 0x78},
      {"STCH",0x54},
      {"STL", 0x14},
      {"STS", 0x7C},
      {"STT", 0x84},
      {"STX", 0x10},
      {"SUB", 0x1C},
      {"TIX", 0x2C},
  };

  int parseHexOpcode(const std::string& opcode) {
    try {
      return std::stoi(opcode);
    }
    catch (const std::invalid_argument&) {
      throw std::runtime_error("Opcode invalido (nao eh hexadecimal): " + opcode);
    }
    catch (const std::out_of_range&) {
      throw std::runtime_error("Opcode hexadecimal fora do intervalo de int: " + opcode);
    }
  }
};