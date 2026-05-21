# ============================================================
#  Makefile — Bin Packing (Busca Local)
# ============================================================

# Compilador
CXX = g++

# Flags de compilação
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra

# Nome do executável
TARGET = binpacking_or_instancy

# Arquivos fonte
SRC = binpacking_or_instancy.cpp

# ============================================================

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) binpack1.txt 100

clean:
	rm -f $(TARGET)

debug:
	$(CXX) -g -std=c++17 -Wall -Wextra $(SRC) -o $(TARGET)

# ============================================================