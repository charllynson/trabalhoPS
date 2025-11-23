cmake_minimum_required(VERSION 4.0)
project(VM_SIC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

# Lista de todos os arquivos fonte e de cabeçalho
set(SOURCE_FILES
    main.cpp
    Memoria.cpp
    Memoria.h
    CPU.cpp
    CPU.h
    Maquina_melhor.cpp
    Maquina_melhor.h
    InterfaceGrafica.cpp
    InterfaceGrafica.h
)

add_executable(VM_SIC ${SOURCE_FILES})

target_link_libraries(VM_SIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
)