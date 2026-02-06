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
#include <memory>
#include <iomanip>
#include <cstdint>


class MontadorSemVibecode {
public:
    MontadorSemVibecode();
    ~MontadorSemVibecode() = default;

    bool montar(const std::string& caminhoArquivoFonte, const std::string& caminhoArquivoDestino);

private:
  std::unordered_map<std::string, std::uint32_t> registers = {
    {"A", 0x0},
    {"X", 0x1},
    {"L", 0x2},
    {"B", 0x3},
    {"S", 0x4},
    {"T", 0x5},
    {"F", 0x6},
    {"PC", 8},
    {"SW", 9}
  };

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

  std::string parseToHexWithPad(const std::uint64_t value, const int width) {
    std::ostringstream oss;
    oss << std::uppercase << std::hex 
        << std::right << std::setw(width) << std::setfill('0')
        << value;

    return oss.str();
  }

  int parseHexOpcodeWithBase16(const std::string& opcode) {
    try {
      return std::stoi(opcode, nullptr, 16);
    }
    catch (const std::invalid_argument&) {
      throw std::runtime_error("Opcode invalido (nao eh hexadecimal): " + opcode);
    }
    catch (const std::out_of_range&) {
      throw std::runtime_error("Opcode hexadecimal fora do intervalo de int: " + opcode);
    }
  }

  std::string parseToSignedHexWithPad(const std::int64_t value, const int width) {
    std::ostringstream oss;
    oss << std::uppercase << std::hex 
        << std::right << std::setw(width) << std::setfill('0')
        << value;

    return oss.str();
  }

  uint32_t pack_fmt3(uint8_t opcode,
                   bool n, bool i, bool x, bool b, bool p, bool e,
                   int16_t disp)
  {
    uint32_t op = (opcode & 0x3F);

    uint32_t flags =
        (n << 5) |
        (i << 4) |
        (x << 3) |
        (b << 2) |
        (p << 1) |
        (e << 0);

    uint32_t disp12 = disp & 0xFFF;

    return (op << 18) | (flags << 12) | disp12;
  }

  uint32_t pack_fmt4(uint8_t opcode,
                   bool n, bool i, bool x, bool b, bool p, bool e,
                   uint32_t addr)
  {
    uint32_t op = (opcode & 0x3F);

    e = 1;
    b = 0;
    p = 0;

    uint32_t flags =
        (n << 5) |
        (i << 4) |
        (x << 3) |
        (b << 2) |
        (p << 1) |
        (e << 0);

    uint32_t addr20 = addr & 0xFFFFF;

    return (op << 26) | (flags << 20) | addr20;
  }
};