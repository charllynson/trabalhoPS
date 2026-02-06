cmake_minimum_required(VERSION 3.1)
project(VM_SIC)

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)


# find_package(Qt5 COMPONENTS
#         Core
#         Gui
#         Widgets
#         REQUIRED)

add_executable(VM_SIC 
        ./VM/main.cpp
        ./VM/Memoria.cpp
        ./VM/Memoria.h
        ./VM/CPU.cpp
        ./VM/CPU.h
        ./VM/MontadorSemVibecode.cpp
        ./VM/MontadorSemVibecode.h)

# target_link_libraries(VM_SIC
#         Qt5::Core
#         Qt5::Gui
#         Qt5::Widgets
# )

target_link_libraries(VM_SIC)

