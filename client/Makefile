# Anderson Cardoso Gonçalves - github.com/ansoncg 

# Makefile para ficar fora da pasta de estruturas

# Target: Pre-requitos
# <TAB> <como fazer>
# $@ -> Nome do target
# $< -> Primeiro pre-requisito
# $^ -> Todos os pre-requisitos

FLAGS_FINAL = #-O3 -march=native -w #-funroll-loops
FLAGS_AVISO = -Wall -Wextra -Wconversion -pedantic
FLAGS_DEBUG = -ggdb3 -O0
FLAGS_VALGRIND = --leak-check=full --show-leak-kinds=all --track-origins=yes
FLAGS_SAN = -fsanitize=thread #-fsanitize=address -fsanitize=undefined
FLAGS_LIGA = -pthread -lSDL2#-lm

CC = gcc

EXE = programa # Otimizado
EXE_V = programaV # Valgrind
EXE_S = programaS # Sanitize
EXE_D = programaD # Debugger

FONTE_ESTR = $(wildcard Estruturas/src/*.c) # Caminho das estruturas
FONTE_PROG = $(wildcard *.c) # Caminho da main
OBJ = $(FONTE_PROG:.c=.o) $(FONTE_ESTR:.c=.o) 

#--------------

# Final
all: CFLAGS = $(FLAGS_FINAL)
all: $(EXE) 

$(EXE): $(OBJ) 
	$(CC) $^ -o $@ $(FLAGS_LIGA) 

run: all
	./$(EXE)

#--------------

# Sanitize
s: CFLAGS = $(FLAGS_AVISO) $(FLAGS_DEBUG) -fsanitize=thread #-fsanitize=undefined
s: $(EXE_S)
	./$(EXE_S) 

$(EXE_S): $(OBJ)
	$(CC) $(FLAGS_SAN) $^ -o $@ $(FLAGS_LIGA) 

#--------------

# Valgrind
v: CFLAGS = $(FLAGS_AVISO) $(FLAGS_DEBUG) 
v: $(EXE_V)
	valgrind $(FLAGS_VALGRIND) ./$(EXE_V)

$(EXE_V): $(OBJ)
	$(CC) $^ -o $@ $(FLAGS_LIGA)

#--------------

#GDB
d: CFLAGS = $(FLAGS_AVISO) $(FLAGS_DEBUG) 
d: $(EXE_D)
	gdb -tui ./$(EXE_D)

$(EXE_D): $(OBJ)
	$(CC) $^ -o $@ $(FLAGS_LIGA)

clean:
	rm -f $(OBJ) 
	rm programa*

.PHONY: all clean
