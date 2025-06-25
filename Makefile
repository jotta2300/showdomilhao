BINNAME = show_milhao

# ==============================================================================
# Variáveis de Configuração
# ==============================================================================

# Diretório do Raylib (AJUSTE ESTE CAMINHO PARA ONDE O RAYLIB ESTÁ INSTALADO NO SEU PC)
RAYLIB_PATH = C:\\raylib\\w64devkit

# Arquivo(s) fonte do seu projeto
SRC = show_milhao.c # Se tiver mais arquivos, separe por espaço (e.g., hello.c game.c)

# Nome do executável de saída
BIN = $(BINNAME).exe # Definido aqui para ser consistente

# ==============================================================================
# Flags do Compilador GCC
# ==============================================================================

# Flags gerais de compilação
CFLAGS = -g -Wall -Wextra -std=c99 -DPLATFORM_DESKTOP -D_DEFAULT_SOURCE -Wno-missing-braces

# Flags para o Raylib (incluindo dependências do Windows)
# As flags "-I" e "-L" são cruciais aqui
RAYLIB_LINK_FLAGS = -I $(RAYLIB_PATH)\\include -L $(RAYLIB_PATH)\\lib -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows

# ==============================================================================
# Targets
# ==============================================================================

all: $(BIN)

$(BIN): $(SRC)
	@echo "Compilando $(BIN)..."
	gcc $(SRC) $(CFLAGS) $(RAYLIB_LINK_FLAGS) -o $(BIN)
	@echo "Compilação concluída."

run: $(BIN)
	@echo "Executando $(BIN)..."
	./$(BIN)

debug: $(BIN)
	gdb $(BIN)

clean:
	@echo "Limpando arquivos..."
	del $(BIN) # Para Windows (use rm -f $(BIN) para Linux/macOS)
	@echo "Limpeza concluída."

.PHONY: all run debug clean

help:
	@echo "Makefile para projetos Raylib no Windows (MinGW-w64)"
	@echo ""
	@echo "Comandos disponíveis:"
	@echo "  make all    - Compila o projeto (cria $(BIN))"
	@echo "  make run    - Compila e executa o projeto"
	@echo "  make debug  - Compila e inicia o depurador GDB"
	@echo "  make clean  - Remove o executável e outros arquivos gerados"
	@echo ""