cmake_minimum_required(VERSION 3.16)
project(VM_SIC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ── Arquivos comuns (backend) ──────────────────────────────
set(BACKEND_SOURCES
    ./VM/Memoria.cpp
    ./VM/CPU.cpp
    ./VM/MontadorSemVibecode.cpp
    ./VM/Ligador.cpp
    ./VM/Maquina_melhor.cpp
)

set(BACKEND_HEADERS
    ./VM/Memoria.h
    ./VM/CPU.h
    ./VM/MontadorSemVibecode.h
    ./VM/Ligador.h
    ./VM/Maquina_melhor.h
)

# ── Target CLI (terminal) ─────────────────────────────────
add_executable(VM_SIC
    ./VM/main.cpp
    ${BACKEND_SOURCES}
    ${BACKEND_HEADERS}
)

# ── Target GUI (Qt6) ──────────────────────────────────────
add_executable(VM_SIC_GUI
    ./VM/main_gui.cpp
    ./VM/MainWindow.cpp
    ./VM/MainWindow.h
    ${BACKEND_SOURCES}
    ${BACKEND_HEADERS}
)

target_link_libraries(VM_SIC_GUI Qt6::Widgets)

