#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

using namespace std;

struct MacroDefinition {
    vector<string> body; // Linhas de código da macro
    vector<string> params; // Parâmetros formais (ex: &A, &B)
};

map<string, MacroDefinition> NAMTAB;
bool expanding = false;

// Função auxiliar para dividir string por espaços
vector<string> split(const string &s) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (tokenStream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Função para substituir argumentos no corpo da macro
string substituteArgs(string line, const vector<string>& formalParams, const vector<string>& actualArgs) {
    string expandedLine = line;
    for (size_t i = 0; i < formalParams.size(); i++) {
        size_t pos = expandedLine.find(formalParams[i]);
        while (pos != string::npos) {
            if (i < actualArgs.size()) {
                expandedLine.replace(pos, formalParams[i].length(), actualArgs[i]);
                pos = expandedLine.find(formalParams[i], pos + actualArgs[i].length());
            } else break; 
            /* Se chegar aqui significa que não temos mais argumentos reais para substituir, então paramos de procurar e substituímos.
            Isso evita loops infinitos caso haja mais ocorrências do parâmetro formal do que argumentos reais.*/
        }
    }
    return expandedLine;
}

void processFile(string inputPath, string outputPath) {
    ifstream inputFile(inputPath);
    ofstream outputFile(outputPath);
    
    if (!inputFile.is_open()) {
        cerr << "Erro ao abrir arquivo de entrada." << endl;
        return;
    }

    string line;
    bool definingMacro = false;
    string currentMacroName;

    while (getline(inputFile, line)) {

        if (!line.empty() && line[0] == '.') {
            outputFile << line << endl;
            continue; 
        }
        
        vector<string> tokens = split(line);
        if (tokens.empty()) continue;

        // Lógica de Identificação de Colunas (Rótulo, Opcode, Operando)
        
        string label, opcode, operand;
        if (line[0] != ' ' && line[0] != '\t') {
            label = tokens[0];
            if (tokens.size() > 1) opcode = tokens[1];
            if (tokens.size() > 2) operand = tokens[2];
        } else {
            label = "";
            if (tokens.size() > 0) opcode = tokens[0];
            if (tokens.size() > 1) operand = tokens[1];
        }

        if (opcode == "MACRO") {
            definingMacro = true;
            currentMacroName = label;
            
            // Salva parâmetros formais
            MacroDefinition newMacro;
            string currentArg;
            stringstream ss(operand);
            while (getline(ss, currentArg, ',')) {
                newMacro.params.push_back(currentArg);
            }
            NAMTAB[currentMacroName] = newMacro;
            continue; // Não escreve a linha MACRO
        }

        if (definingMacro) {
            if (opcode == "MEND") {
                definingMacro = false;
            } else {
                // adiciona linha ao corpo da macro na DEFTAB
                NAMTAB[currentMacroName].body.push_back(line);
            }
            continue; // não escreve o corpo da macro durante a def
        }

        if (NAMTAB.find(opcode) != NAMTAB.end()) {
            MacroDefinition m = NAMTAB[opcode];
            
            // Parsear argumentos reais
            vector<string> actualArgs;
            string currentArg;
            stringstream ss(operand);
            while (getline(ss, currentArg, ',')) {
                actualArgs.push_back(currentArg);
            }

            outputFile << "." << line << endl;

            for (size_t i = 0; i < m.body.size(); i++) {
                string processedLine = substituteArgs(m.body[i], m.params, actualArgs);

                if (i == 0 && !label.empty()) {
                    size_t startPos = processedLine.find_first_not_of(" \t");
                    if (startPos == string::npos) startPos = 0;
                    outputFile << label << "\t" << processedLine.substr(startPos) << endl;
                } else {
                    // linhas sapós apenas são impressas
                    outputFile << processedLine << endl;
                }
            }
        }
        // Linha normal
        else {
            outputFile << line << endl;
        }
    }

    inputFile.close();
    outputFile.close();
    cout << "Processamento concluído. Verifique " << outputPath << endl;
}

int main() {
    processFile("source.asm", "MASMAPRG.asm");
    return 0;
}