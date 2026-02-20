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

struct ExprResult {
  int32_t value = 0;
  // 0 = absoluto, 1 = relocável simples
  int relocCount = 0;
  // referencias externas pendentes {simbolo, sinal (+1 ou -1)}
  std::vector<std::pair<std::string, int>> externalRefs;
  bool isAbsolute() const { return relocCount == 0 && externalRefs.empty(); }
  bool isRelocatable() const { return relocCount == 1 && externalRefs.empty(); }
  bool isValid() const { return relocCount >= 0 && relocCount <= 1; }
};


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
    "BASE",
    "EQU",
    "EXTDEF",
    "EXTREF",
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

  struct ExprTerm {
    std::string token;
    int sign;
  };

  std::vector<ExprTerm> tokenizeExpression(const std::string& expr) {
    std::vector<ExprTerm> terms;
    int currentSign = +1;
    std::string currentToken;

    for (size_t i = 0; i <= expr.size(); ++i) {
      char c = (i < expr.size()) ? expr[i] : '\0';

      if (c == '+' || c == '-' || c == '\0') {
        // finalizar token atual

        std::string trimmed;
        for (char ch : currentToken) {
          if (ch != ' ' && ch != '\t') {
            trimmed += ch;
          }
        }

        if (!trimmed.empty()) {
          terms.push_back({trimmed, currentSign});
        }
        currentToken.clear();
        if (c == '+') {
          currentSign = +1;
        } else if (c == '-') {
          currentSign = -1;
        }
      } else {
        currentToken += c;
      }
    }
    return terms;
  }

  // Avalia um único termo e retorna seu valor + atributo de relocação
  ExprResult evaluateTerm(
      const std::string& token,
      int sign,
      const std::unordered_map<std::string, std::uint32_t>& symbol_table,
      const std::unordered_set<std::string>& extrefs,
      std::uint32_t currentAddress)
  {
    ExprResult result;

    if (token == "*") {
      // Endereço atual do Location Counter
      result.value = sign * static_cast<int32_t>(currentAddress);
      result.relocCount = sign; // +1 se positivo, -1 se negativo
    }
    else if (!token.empty() && (std::isdigit(static_cast<unsigned char>(token[0])) || 
             (token[0] == '-' && token.size() > 1))) {
      // Constante numérica — absoluta
      int32_t val = std::stoi(token);
      result.value = sign * val;
      result.relocCount = 0;
    }
    else if (extrefs.count(token)) {
      // Símbolo externo — valor 0 no momento, gera M record
      result.value = 0;
      result.relocCount = 0;
      result.externalRefs.push_back({token, sign});
    }
    else if (symbol_table.count(token)) {
      // Símbolo local — relocável
      result.value = sign * static_cast<int32_t>(symbol_table.at(token));
      result.relocCount = sign; // +1 ou -1
    }
    else {
      throw std::runtime_error("Simbolo indefinido na expressao: " + token);
    }

    return result;
  }

  // Avalia uma expressão completa e retorna o resultado agregado
  ExprResult evaluateExpression(
      const std::string& expr,
      const std::unordered_map<std::string, std::uint32_t>& symbol_table,
      const std::unordered_set<std::string>& extrefs,
      std::uint32_t currentAddress)
  {
    auto terms = tokenizeExpression(expr);

    ExprResult total;
    total.value = 0;
    total.relocCount = 0;

    for (const auto& term : terms) {
      ExprResult termResult = evaluateTerm(
          term.token, term.sign, symbol_table, extrefs, currentAddress);

      total.value += termResult.value;
      total.relocCount += termResult.relocCount;

      // Acumular referências externas
      for (const auto& ref : termResult.externalRefs) {
        total.externalRefs.push_back(ref);
      }
    }

    // Validar: relocCount deve ser 0 (absoluto) ou 1 (relocável simples)
    if (total.relocCount < 0 || total.relocCount > 1) {
      throw std::runtime_error(
          "Expressao com relocacao invalida (reloc count = " 
          + std::to_string(total.relocCount) + "): " + expr);
    }

    return total;
  }

  // Gera M records a partir de um ExprResult para um endereço específico
  // address: endereço do campo a ser modificado
  // nibbles: quantos nibbles (half-bytes) o campo tem (5 para fmt4, 6 para WORD)
  // programName: nome do programa para relocação de símbolos locais


  /*
    É muito importante salientar que toda expressão com saldo 1 é relativa ao início
    da control section.
  */
  std::vector<std::string> generateMRecords(
      const ExprResult& expr,
      std::uint32_t address,
      int nibbles,
      const std::string& programName)
  {
    std::vector<std::string> records;

    // Se relocável (relocCount == 1), gerar M record para o programa
    if (expr.relocCount == 1) {
      std::ostringstream oss;
      oss << "M"
          << std::uppercase << std::hex
          << std::right << std::setw(6) << std::setfill('0') << address
          << std::right << std::setw(2) << std::setfill('0') << nibbles
          << "+" << programName;
      records.push_back(oss.str());
    }

    // Para cada referência externa, gerar um M record
    for (const auto& [symbol, sign] : expr.externalRefs) {
      std::ostringstream oss;
      oss << "M"
          << std::uppercase << std::hex
          << std::right << std::setw(6) << std::setfill('0') << address
          << std::right << std::setw(2) << std::setfill('0') << nibbles
          << (sign > 0 ? "+" : "-") << symbol;
      records.push_back(oss.str());
    }

    return records;
  }
};