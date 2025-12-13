#ifndef MACRO_PROCESSOR_H
#define MACRO_PROCESSOR_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>

using namespace std;

// Estrutura para representar uma macro
struct MacroDefinition {
    string name;
    vector<string> params;
    vector<string> body;
    
    MacroDefinition() {}
    
    MacroDefinition(const string& n, const vector<string>& p, const vector<string>& b)
        : name(n), params(p), body(b) {}
};

class Processador {
private:
    map<string, MacroDefinition> macroTable;
    vector<string> inputLines;
    vector<string> outputLines;
    int lineIndex;
    bool verbose;
    int errorCount;
    
    // Funções auxiliares
    string toUpper(const string& str);
    string trim(const string& str);
    string removeInlineComment(const string& line);
    vector<string> split(const string& str, char delimiter);
    bool isMacroDefinition(const string& line);
    bool isMacroCall(const string& line);
    
    // Processamento
    void processMacroDefinition();
    vector<string> expandMacro(const string& callLine, int lineNum);
    
    // Log e erro
    void log(const string& message);
    void error(const string& message, int lineNum = -1);
    
public:
    Processador(bool v = false);
    bool processFile(const string& inputFile, const string& outputFile = "MASMAPRG.ASM");
    void printMacroTable();
};

// =================================================================

Processador::Processador(bool v) : lineIndex(0), verbose(v), errorCount(0) {}

string Processador::toUpper(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

string Processador::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

string Processador::removeInlineComment(const string& line) {
    size_t pos = line.find(';');
    if (pos != string::npos) {
        return trim(line.substr(0, pos));
    }
    return line;
}

vector<string> Processador::split(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    
    while (getline(ss, token, delimiter)) {
        token = trim(token);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

void Processador::log(const string& message) {
    if (verbose) {
        cout << "[DEBUG] " << message << endl;
    }
}

void Processador::error(const string& message, int lineNum) {
    errorCount++;
    if (lineNum >= 0) {
        cerr << "ERRO (linha " << lineNum << "): " << message << endl;
    } else {
        cerr << "ERRO: " << message << endl;
    }
}

bool Processador::isMacroDefinition(const string& line) {
    vector<string> tokens = split(line, ' ');
    if (tokens.size() < 2) return false;
    return toUpper(tokens[1]) == "MACRO";
}

bool Processador::isMacroCall(const string& line) {
    string cleanLine = line;
    
    // Remove rótulo se houver
    size_t colonPos = line.find(':');
    if (colonPos != string::npos) {
        cleanLine = trim(line.substr(colonPos + 1));
    }
    
    vector<string> tokens = split(cleanLine, ' ');
    if (tokens.empty()) return false;
    
    string macroName = toUpper(tokens[0]);
    return macroTable.find(macroName) != macroTable.end();
}

void Processador::processMacroDefinition() {
    int startLine = lineIndex;
    string definitionLine = trim(inputLines[lineIndex]);
    definitionLine = removeInlineComment(definitionLine);
    
    vector<string> tokens = split(definitionLine, ' ');
    string macroName = toUpper(tokens[0]);
    
    // Extrai parâmetros
    vector<string> params;
    if (tokens.size() > 2) {
        // Junta todos os tokens após MACRO e separa por vírgula
        string paramStr;
        for (size_t i = 2; i < tokens.size(); i++) {
            paramStr += tokens[i] + " ";
        }
        
        vector<string> paramList = split(paramStr, ',');
        for (const string& param : paramList) {
            string p = trim(param);
            if (p[0] == '&') {
                p = p.substr(1);
            }
            params.push_back(toUpper(p));
        }
    }
    
    log("Processando macro '" + macroName + "' com " + 
        to_string(params.size()) + " parametros");
    
    // Coleta o corpo da macro
    lineIndex++;
    vector<string> macroBody;
    int nestingLevel = 1;
    
    while (lineIndex < inputLines.size() && nestingLevel > 0) {
        string line = inputLines[lineIndex];
        string stripped = trim(line);
        string cleanLine = removeInlineComment(stripped);
        
        if (cleanLine.empty()) {
            macroBody.push_back(line);
            lineIndex++;
            continue;
        }
        
        // Verifica macro aninhada
        if (isMacroDefinition(cleanLine)) {
            nestingLevel++;
            log("Macro aninhada detectada (nivel " + to_string(nestingLevel) + ")");
            macroBody.push_back(line);
        }
        // Verifica MEND
        else if (toUpper(cleanLine) == "MEND") {
            nestingLevel--;
            log("MEND encontrado (nivel " + to_string(nestingLevel) + ")");
            if (nestingLevel > 0) {
                macroBody.push_back(line);
            }
        }
        else {
            macroBody.push_back(line);
        }
        
        lineIndex++;
    }
    
    if (nestingLevel > 0) {
        error("Macro '" + macroName + "' nao foi fechada (falta MEND)", startLine + 1);
        return;
    }
    
    // Armazena a macro
    macroTable[macroName] = MacroDefinition(macroName, params, macroBody);
    
    // Adiciona comentário na saída
    outputLines.push_back("; MACRO " + macroName + " definida (" + 
                         to_string(params.size()) + " parametros)");
    
    log("Macro '" + macroName + "' armazenada com " + 
        to_string(macroBody.size()) + " linhas");
}

vector<string> Processador::expandMacro(const string& callLine, int lineNum) {
    string cleanCall = callLine;
    string label = "";
    
    // Remove rótulo se houver
    size_t colonPos = callLine.find(':');
    if (colonPos != string::npos) {
        label = trim(callLine.substr(0, colonPos));
        cleanCall = trim(callLine.substr(colonPos + 1));
    }
    
    vector<string> tokens = split(cleanCall, ' ');
    string macroName = toUpper(tokens[0]);
    
    // Obtém argumentos
    vector<string> args;
    if (tokens.size() > 1) {
        string argStr;
        for (size_t i = 1; i < tokens.size(); i++) {
            argStr += tokens[i] + " ";
        }
        args = split(argStr, ',');
    }
    
    // Recupera a macro
    if (macroTable.find(macroName) == macroTable.end()) {
        error("Macro '" + macroName + "' nao definida", lineNum);
        return {callLine};
    }
    
    MacroDefinition& macroDef = macroTable[macroName];
    
    log("Expandindo macro '" + macroName + "' com " + 
        to_string(args.size()) + " argumentos");
    
    // Verifica número de argumentos
    if (args.size() < macroDef.params.size()) {
        error("Macro '" + macroName + "' espera " + 
              to_string(macroDef.params.size()) + " argumentos, mas recebeu " + 
              to_string(args.size()), lineNum);
    }
    
    // Cria mapa de substituições
    map<string, string> substitutions;
    for (size_t i = 0; i < macroDef.params.size(); i++) {
        if (i < args.size()) {
            substitutions[macroDef.params[i]] = args[i];
        } else {
            substitutions[macroDef.params[i]] = "";
        }
    }
    
    // Expande o corpo
    vector<string> expanded;
    bool labelAdded = label.empty();
    
    for (const string& line : macroDef.body) {
        string expandedLine = line;
        
        // Substitui parâmetros
        for (const auto& sub : substitutions) {
            string param = "&" + sub.first;
            size_t pos = 0;
            
            // Substitui todas as ocorrências (case-insensitive)
            while ((pos = toUpper(expandedLine).find(toUpper(param), pos)) != string::npos) {
                // Verifica se é uma palavra completa
                bool isWordBoundary = true;
                if (pos + param.length() < expandedLine.length()) {
                    char nextChar = expandedLine[pos + param.length()];
                    if (isalnum(nextChar) || nextChar == '_') {
                        isWordBoundary = false;
                    }
                }
                
                if (isWordBoundary) {
                    expandedLine.replace(pos, param.length(), sub.second);
                    pos += sub.second.length();
                } else {
                    pos += param.length();
                }
            }
        }
        
        // Adiciona rótulo na primeira linha não-vazia
        string stripped = trim(expandedLine);
        if (!labelAdded && !stripped.empty() && stripped[0] != ';') {
            size_t indent = expandedLine.find_first_not_of(" \t");
            if (indent == string::npos) indent = 0;
            
            expandedLine = string(indent, ' ') + label + ": " + trim(expandedLine);
            labelAdded = true;
        }
        
        // Verifica chamada aninhada
        string cleanExpanded = removeInlineComment(stripped);
        if (!cleanExpanded.empty()) {
            size_t colonPos2 = cleanExpanded.find(':');
            if (colonPos2 != string::npos) {
                cleanExpanded = trim(cleanExpanded.substr(colonPos2 + 1));
            }
            
            vector<string> callTokens = split(cleanExpanded, ' ');
            if (!callTokens.empty() && 
                macroTable.find(toUpper(callTokens[0])) != macroTable.end()) {
                // Chamada aninhada - expande recursivamente
                log("Chamada aninhada detectada: " + cleanExpanded);
                vector<string> nestedExpansion = expandMacro(cleanExpanded, lineNum);
                expanded.insert(expanded.end(), nestedExpansion.begin(), nestedExpansion.end());
            } else {
                expanded.push_back(expandedLine);
            }
        } else {
            expanded.push_back(expandedLine);
        }
    }
    
    return expanded;
}

bool Processador::processFile(const string& inputFile, const string& outputFile) {
    // Lê arquivo de entrada
    ifstream inFile(inputFile);
    if (!inFile.is_open()) {
        error("Arquivo '" + inputFile + "' nao encontrado");
        return false;
    }
    
    string line;
    while (getline(inFile, line)) {
        inputLines.push_back(line);
    }
    inFile.close();
    
    log("Arquivo '" + inputFile + "' lido com " + 
        to_string(inputLines.size()) + " linhas");
    
    // Processa todas as linhas
    lineIndex = 0;
    outputLines.clear();
    errorCount = 0;
    
    while (lineIndex < inputLines.size()) {
        string currentLine = inputLines[lineIndex];
        string stripped = trim(currentLine);
        
        // Ignora linhas vazias
        if (stripped.empty()) {
            outputLines.push_back(currentLine);
            lineIndex++;
            continue;
        }
        
        // Ignora comentários
        if (stripped[0] == ';') {
            outputLines.push_back(currentLine);
            lineIndex++;
            continue;
        }
        
        string cleanLine = removeInlineComment(stripped);
        
        // Verifica definição de macro
        if (isMacroDefinition(cleanLine)) {
            log("Definicao de macro encontrada na linha " + to_string(lineIndex + 1));
            processMacroDefinition();
        }
        // Verifica chamada de macro
        else if (isMacroCall(cleanLine)) {
            log("Chamada de macro encontrada na linha " + to_string(lineIndex + 1) + 
                ": " + cleanLine);
            vector<string> expanded = expandMacro(cleanLine, lineIndex + 1);
            outputLines.insert(outputLines.end(), expanded.begin(), expanded.end());
            lineIndex++;
        }
        else {
            // Linha normal
            outputLines.push_back(currentLine);
            lineIndex++;
        }
    }
    
    if (errorCount > 0) {
        cout << "\nProcessamento concluido com " << errorCount << " erro(s)" << endl;
        return false;
    }
    
    // Escreve arquivo de saída
    ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        error("Erro ao escrever arquivo de saida");
        return false;
    }
    
    for (const string& outputLine : outputLines) {
        outFile << outputLine << endl;
    }
    outFile.close();
    
    cout << "\n✓ Arquivo '" << outputFile << "' gerado com sucesso!" << endl;
    cout << "  - " << macroTable.size() << " macro(s) definida(s)" << endl;
    cout << "  - " << outputLines.size() << " linha(s) no arquivo de saida" << endl;
    
    return true;
}

void Processador::printMacroTable() {
    cout << "\n" << string(60, '=') << endl;
    cout << "TABELA DE MACROS" << endl;
    cout << string(60, '=') << endl;
    
    if (macroTable.empty()) {
        cout << "(nenhuma macro definida)" << endl;
        return;
    }
    
    for (const auto& entry : macroTable) {
        const MacroDefinition& macroDef = entry.second;
        cout << "\nMacro: " << macroDef.name << endl;
        cout << "  Parametros (" << macroDef.params.size() << "): ";
        
        if (macroDef.params.empty()) {
            cout << "(nenhum)";
        } else {
            for (size_t i = 0; i < macroDef.params.size(); i++) {
                cout << macroDef.params[i];
                if (i < macroDef.params.size() - 1) cout << ", ";
            }
        }
        
        cout << endl;
        cout << "  Corpo: " << macroDef.body.size() << " linha(s)" << endl;
    }
}

#endif // MACRO_PROCESSOR_H