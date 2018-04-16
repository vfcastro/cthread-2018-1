LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/

all: cthread.o libcthread.a

cthread.o: #dependências para a regra1
	gcc -c -o $(BIN_DIR)cthread.o $(SRC_DIR)cthread.c -Wall
libcthread.a:
	ar crs $(LIB_DIR)libcthread.a $(BIN_DIR)cthread.o $(BIN_DIR)support.o
clean:
	rm -rf $(LIB_DIR)libcthread.a $(BIN_DIR)cthread.o


