LIB_DIR=../lib/

all: cyield ccreate cjoin

cjoin:
	gcc -o cjoin cjoin.c -L$(LIB_DIR) -lcthread -Wall
cyield:
	gcc -o cyield cyield.c -L$(LIB_DIR) -lcthread -Wall
ccreate:
	gcc -o ccreate ccreate.c -L$(LIB_DIR) -lcthread -Wall
clean:
	rm -rf cyield ccreate cjoin
