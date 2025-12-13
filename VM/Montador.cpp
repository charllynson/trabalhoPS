#include "Montador.h"
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <cctype>

std::vector<uint8_t> Montador::montar(const std::string& caminhoArquivoFonte) {
    std::ifstream arquivoFonte(caminhoArquivoFonte);
    std::vector<uint8_t> codigoObjeto;
    std::unordered_set<std::string> instruçoesFormato2 = {
        "ADDR", "CLEAR", "COMPR", "DIVR", "MULR", "RMO",
        "SHIFTL", "SHIFTR", "SUBR", "TIXR"
    };
    std::unordered_set<std::string> instruçoesFormato34 = {
        "LDA", "STA", "ADD", "SUB", "MUL", "DIV",
        "COMP", "J", "JEQ", "JGT", "JLT", "AND",
        "OR", "JSUB", "RSUB", "LDX", "STX", "TIX"
    };

    int contadorEndereco = 0;

    if (!arquivoFonte.is_open()) {
        std::cerr << "Erro ao abrir o arquivo fonte: " << caminhoArquivoFonte << std::endl;
        return {};
    }

    std::string linha;
    std::unordered_map<std::string, uint32_t> tabelaSimbolos;

    /*
    ==========================================================================================
        Primeira passada: construir a tabela de símbolos e calcular endereços
    ==========================================================================================
    */
    
    while (std::getline(arquivoFonte, linha)) {
        if (linha.empty() || linha[0] == '.') {
            continue;
        }

        std::stringstream ss(linha);
        std::string etiqueta, instrucao, operando;

        // se a linha começa com espaço não tem etiqueta
        if (linha[0] != ' ' && linha[0] != '\t') {
            ss >> etiqueta;
            if (!etiqueta.empty()) {
                tabelaSimbolos[etiqueta] = contadorEndereco;
            }
        }

        // próxima parte é a instrução
        ss >> instrucao;

        // o resto da linha é o operando, e pegamos o getline pra pegar tudo
        // incluindo operandos com virgulas como "A,X"
        std::getline(ss, operando);

        // remover espaços em branco no inicio do operando
        operando.erase(0, operando.find_first_not_of(" \t"));
        // remover espaços em brancos no final do operando
        operando.erase(operando.find_last_not_of(" \t") + 1);

        // Lógica para incrementar o contador de endereço (LC)
        if (instruçoesFormato2.count(instrucao)) {
            contadorEndereco += 2;
        } else if (instruçoesFormato34.count(instrucao)) {
            // Verifica se é formato 4 (com '+')
            if (!instrucao.empty() && instrucao[0] == '+') {
                contadorEndereco += 4;
            } else {
                contadorEndereco += 3;
            }
        } else if (instrucao == "WORD") {
            contadorEndereco += 3;
        } else if (instrucao == "RESW") {
            // stoi converte string para int
            if (!operando.empty()) {
                contadorEndereco += 3 * std::stoi(operando);
            }
        } else if (instrucao == "RESB") {
            if (!operando.empty()) {
                contadorEndereco += std::stoi(operando);
            }
        } else if (instrucao == "BYTE") {
            // C'EOF' -> 3 bytes. X'F1' -> 1 byte.
            if (!operando.empty() && operando[0] == 'C') {
                auto p1 = operando.find('\'');
                auto p2 = operando.find_last_of('\'');
                if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
                    std::string cont = operando.substr(p1+1, p2-p1-1);
                    contadorEndereco += cont.size();
                }
            } else if (!operando.empty() && operando[0] == 'X') {
                auto p1 = operando.find('\'');
                auto p2 = operando.find_last_of('\'');
                if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
                    std::string hexs = operando.substr(p1+1, p2-p1-1);
                    contadorEndereco += (hexs.size() + 1) / 2;
                }
            }
        }
    }

    // reabrir o arquivo para a segunda passada
    arquivoFonte.clear();
    arquivoFonte.seekg(0);

    std::unordered_map<std::string, uint8_t> opcode = {
          // Formato 3/4
    {"ADD", 0x18}, {"AND", 0x40}, {"COMP", 0x28}, {"DIV", 0x24},
    {"J", 0x3C}, {"JEQ", 0x30}, {"JGT", 0x34}, {"JLT", 0x38},
    {"JSUB", 0x48}, {"LDA", 0x00}, {"LDB", 0x68}, {"LDCH", 0x50},
    {"LDL", 0x08}, {"LDS", 0x6C}, {"LDT", 0x74}, {"LDX", 0x04},
    {"MUL", 0x20}, {"OR", 0x44}, {"RSUB", 0x4C},
    {"STA", 0x0C}, {"STB", 0x78}, {"STCH", 0x54}, {"STL", 0x14},
    {"STS", 0x7C}, {"STT", 0x84}, {"STX", 0x10}, {"SUB", 0x1C},
    {"TIX", 0x2C},

    // Formato 2
    {"ADDR", 0x90}, {"CLEAR", 0xB4}, {"COMPR", 0xA0}, {"DIVR", 0x9C},
    {"MULR", 0x98}, {"RMO", 0xAC}, {"SHIFTL", 0xA4}, {"SHIFTR", 0xA8},
    {"SUBR", 0x94}, {"TIXR", 0xB8}
    };

    // utilitários de trim
    auto ltrim = [](std::string &s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
    };
    auto rtrim = [](std::string &s) {
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
    };
    auto trimAll = [&](std::string s){
        ltrim(s); rtrim(s); return s;
    };

    // parse number decimal / hex 0x... / hex literal como "4096" ou "FF"
    auto parseNumber = [&](const std::string &t){
        std::string s = trimAll(t);
        if(s.empty()) return 0;

        //detecta hex com prefixo 0x
        if(s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')){
            return std::stoi(s.substr(2), nullptr, 16);
        }

        //detecta negativo
        bool negative = (s[0] == '-');
        std::string body = negative ? s.substr(1) : s;

        //se for apenas digitos: decimal
        bool allDec = !body.empty() && std::all_of(body.begin(), body.end(), [](char c){ return std::isdigit(c); });
        if(allDec){
            return negative ? -std::stoi(body) : std::stoi(body);
        }
        
        // Tenta como hexadecimal sem prefixo
        try {
            return std::stoi(body, nullptr, 16);
        } catch (...) {
            return 0;
        }
    };
    
    auto regToNum = [](const std::string &r)->uint8_t {
        std::string s = r;
        //trim simples
        size_t a = s.find_first_not_of(" \t");
        size_t b = s.find_last_not_of(" \t");
        if (a==std::string::npos) return 0;
        s = s.substr(a, b-a+1);
        if (s.empty()) return 0;
        if (std::isdigit(s[0])) return static_cast<uint8_t>(std::stoi(s));
        if (s == "A") return 0;
        if (s == "X") return 1;
        if (s == "L") return 2;
        if (s == "B") return 3;
        if (s == "S") return 4;
        if (s == "T") return 5;
        if (s == "F") return 6;
        return 0;
    };

    int locctr = 0;
    int startAddress = 0;
    int baseAddress = -1;
    int programLength = contadorEndereco;
    std::size_t lineNo = 0;

    // Segunda passada 
    while(std::getline(arquivoFonte, linha)){
        lineNo++;
        if(linha.empty() || linha[0] == '.') continue;

        std::stringstream ss(linha);
        std::string etiqueta, instrucao, operando;

        // label
        if(linha[0] != ' ' && linha[0] != '\t' ){
            ss >> etiqueta;
        }

        // instrução
        ss >> instrucao;
        // resto da linha => operando (pode conter vírgulas)
        std::getline(ss, operando);
        operando = trimAll(operando);

        // handle START
        if (instrucao == "START") {
            if (!operando.empty()) {
                startAddress = parseNumber(operando);
                locctr = startAddress;
            } else {
                locctr = 0;
            }
            continue;
        }

        // BASE / NOBASE
        if (instrucao == "BASE") {
            if (!operando.empty()) {
                // pode ser label ou número
                if (tabelaSimbolos.count(operando)) baseAddress = tabelaSimbolos[operando];
                else baseAddress = parseNumber(operando);
            }
            continue;
        }
        if (instrucao == "NOBASE") {
            baseAddress = -1;
            continue;
        }

        // END -> para a montagem
        if (instrucao == "END") break;

        // Diretivas de dados
        if (instrucao == "BYTE") {
            if (!operando.empty() && operando[0] == 'C') {
                auto p1 = operando.find('\'');
                auto p2 = operando.find_last_of('\'');
                if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
                    std::string cont = operando.substr(p1+1, p2-p1-1);
                    for (char c: cont) codigoObjeto.push_back(static_cast<uint8_t>(c));
                    locctr += (int)cont.size();
                }
            } else if (!operando.empty() && operando[0] == 'X') {
                auto p1 = operando.find('\'');
                auto p2 = operando.find_last_of('\'');
                if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
                    std::string hexs = operando.substr(p1+1, p2-p1-1);
                    // cada 2 hex -> 1 byte
                    for (size_t i = 0; i + 1 < hexs.size(); i += 2) {
                        std::string bstr = hexs.substr(i, 2);
                        uint8_t val = static_cast<uint8_t>(std::stoi(bstr, nullptr, 16));
                        codigoObjeto.push_back(val);
                    }
                    locctr += (int)((hexs.size()+1)/2);
                }
            } else {
                // caso inesperado, ignorar
            }
            continue;
        }

        if (instrucao == "WORD") {
            int val = 0;
            if (!operando.empty()) {
                if (tabelaSimbolos.count(operando)) val = (int)tabelaSimbolos[operando];
                else val = parseNumber(operando);
            }
            codigoObjeto.push_back((val >> 16) & 0xFF);
            codigoObjeto.push_back((val >> 8) & 0xFF);
            codigoObjeto.push_back(val & 0xFF);
            locctr += 3;
            continue;
        }

        if (instrucao == "RESW") {
            int n = !operando.empty() ? parseNumber(operando) : 0;
            locctr += 3 * n;
            continue;
        }
        if (instrucao == "RESB") {
            int n = !operando.empty() ? parseNumber(operando) : 0;
            locctr += n;
            continue;
        }

        // Instruções formato 2
        if (instruçoesFormato2.count(instrucao)) {
            uint8_t opcode_val = opcode[instrucao];
            codigoObjeto.push_back(opcode_val);
            
            // Formato: INSTR R1,R2
            std::string r1, r2;
            size_t commaPos = operando.find(',');
            if (commaPos != std::string::npos) {
                r1 = operando.substr(0, commaPos);
                r2 = operando.substr(commaPos + 1);
            } else {
                r1 = operando;
                r2 = "";
            }
            
            uint8_t r1_num = regToNum(r1);
            uint8_t r2_num = regToNum(r2);
            codigoObjeto.push_back((r1_num << 4) | r2_num);
            locctr += 2;
            continue;
        }

        // Instruções formato 3/4
        if (instruçoesFormato34.count(instrucao)) {
            bool isFormat4 = false;
            std::string instr = instrucao;
            
            // Detectar formato 4 (começa com +)
            if (!instr.empty() && instr[0] == '+') {
                isFormat4 = true;
                instr = instr.substr(1); // Remove o +
            }
            
            if (opcode.find(instr) == opcode.end()) {
                std::cerr << "Erro linha " << lineNo << ": instrução '" << instr << "' desconhecida\n";
                continue;
            }
            
            uint8_t opcode_val = opcode[instr];
            int targetAddr = 0;
            
            // Resolver endereço do operando
            if (!operando.empty()) {
                if (tabelaSimbolos.count(operando)) {
                    targetAddr = tabelaSimbolos[operando];
                } else {
                    targetAddr = parseNumber(operando);
                }
            }
            
            if (isFormat4) {
                // Formato 4: 4 bytes (e=1, x=0, b=0, p=0)
                codigoObjeto.push_back(opcode_val);
                codigoObjeto.push_back((targetAddr >> 16) & 0xFF);
                codigoObjeto.push_back((targetAddr >> 8) & 0xFF);
                codigoObjeto.push_back(targetAddr & 0xFF);
                locctr += 4;
            } else {
                // Formato 3: 3 bytes, usar endereçamento PC-relativo ou base
                int displacement = targetAddr - (locctr + 3);
                bool usePCRel = (displacement >= -2048 && displacement <= 2047);
                
                uint16_t addr = 0;
                if (usePCRel) {
                    addr = (1 << 13) | (displacement & 0x1FFF); // p=1
                } else if (baseAddress >= 0) {
                    displacement = targetAddr - baseAddress;
                    if (displacement >= 0 && displacement <= 4095) {
                        addr = (1 << 12) | (displacement & 0xFFF); // b=1
                    }
                }
                
                codigoObjeto.push_back(opcode_val);
                codigoObjeto.push_back((addr >> 8) & 0xFF);
                codigoObjeto.push_back(addr & 0xFF);
                locctr += 3;
            }
            continue;
        }
    }

    return codigoObjeto;
}