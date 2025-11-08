//
// Created by jsfonseca on 21/10/2025.
//

#ifndef VM_SIC_CPU_H
#define VM_SIC_CPU_H

#include <cstdint>
enum RegID {A = 0, X = 1, L = 2, B = 3, S = 4, T = 5, F = 6, PC = 8, SW = 9};
enum status {BIGGER = 1, EQUAL = 0, SMALLER = -1};

struct Registradores {;
    std::int32_t A = 0;  // Acumulador
    std::int32_t PC = 0; // Contador de programa
    status SW = EQUAL; // Palavra de status

    std::int32_t X = 0;  // Registrador de índice
    std::int32_t L = 0;  // Registrador de ligação
    std::int32_t B = 0;  // Base

    std::int32_t S = 0;  // Registrador geral S
    std::int32_t T = 0;  // Registrador geral T
};

class CPU {
public:
    Registradores r;
};


#endif //VM_SIC_CPU_H