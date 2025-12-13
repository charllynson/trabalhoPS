#include "Processador.h"
#include <iostream>
#include <string>

using namespace std;

void printUsage() {
    cout << string(60, '=') << endl;
    cout << "PROCESSADOR DE MACROS - Uma Passagem" << endl;
    cout << string(60, '=') << endl;
    cout << "\nUso: ./processador <arquivo_entrada.asm> [opcoes]" << endl;
    cout << "\nOpcoes:" << endl;
    cout << "  -v, --verbose    Modo verbose (debug)" << endl;
    cout << "  -o <arquivo>     Nome do arquivo de saida (padrao: MASMAPRG.ASM)" << endl;   
    cout << "  -t, --table      Imprime tabela de macros ao final" << endl;
    cout << "\nExemplos:" << endl;
    cout << "  ./processador programa.asm" << endl;
    cout << "  ./processador programa.asm -v" << endl;
    cout << "  ./processador programa.asm -o saida.asm -t" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    // Parse dos argumentos
    string inputFile = argv[1];
    string outputFile = "MASMAPRG.ASM";
    bool verbose = false;
    bool showTable = false;
    
    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        }
        else if (arg == "-t" || arg == "--table") {
            showTable = true;
        }
        else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        }
    }
    
    // Imprime header
    cout << string(60, '=') << endl;
    cout << "PROCESSADOR DE MACROS - Uma Passagem" << endl;
    cout << string(60, '=') << endl;
    cout << "\nArquivo de entrada: " << inputFile << endl;
    cout << "Arquivo de saida:   " << outputFile << endl;
    if (verbose) {
        cout << "Modo: VERBOSE" << endl;
    }
    cout << endl;
    
    // Processa o arquivo
    Processador processor(verbose);
    bool success = processor.processFile(inputFile, outputFile);
    
    // Imprime tabela de macros se solicitado
    if (success && showTable) {
        processor.printMacroTable();
    }
    
    return success ? 0 : 1;
}