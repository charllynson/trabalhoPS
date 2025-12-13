
# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Nome do executável
TARGET = Processador

# Arquivos fonte
SOURCES = Processador.cpp
HEADERS = Processador.h

# Arquivo objeto
OBJECTS = $(SOURCES:.cpp=.o)

# Regra principal
all: $(TARGET)

# Compila o executável
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)
	@echo ""
	@echo "✓ Compilação concluída com sucesso!"
	@echo "  Execute: ./$(TARGET) <arquivo.asm>"

# Compila arquivos .cpp em .o
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpa arquivos compilados
clean:
	rm -f $(OBJECTS) $(TARGET) MASMAPRG.ASM
	@echo "✓ Arquivos limpos"

# Testa com arquivo de exemplo
test: $(TARGET)
	@echo "Testando processador de macros..."
	./$(TARGET) exemplo.asm -v -t

# Ajuda
help:
	@echo "Makefile para Processador de Macros"
	@echo ""
	@echo "Uso:"
	@echo "  make          - Compila o programa"
	@echo "  make clean    - Remove arquivos compilados"
	@echo "  make test     - Compila e testa com exemplo.asm"
	@echo "  make help     - Mostra esta mensagem"

.PHONY: all clean test hel