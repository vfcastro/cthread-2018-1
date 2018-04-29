LIB_DIR=../lib/

all: cyield ccreate cjoin csuspend cresume semaforo

semaforo:
	gcc -o semaforo semaforo.c -L$(LIB_DIR) -lcthread -Wall	
cresume:
	gcc -o cresume cresume.c -L$(LIB_DIR) -lcthread -Wall
csuspend:
	gcc -o csuspend csuspend.c -L$(LIB_DIR) -lcthread -Wall
cjoin:
	gcc -o cjoin cjoin.c -L$(LIB_DIR) -lcthread -Wall
cyield:
	gcc -o cyield cyield.c -L$(LIB_DIR) -lcthread -Wall
ccreate:
	gcc -o ccreate ccreate.c -L$(LIB_DIR) -lcthread -Wall
clean:
	rm -rf cyield ccreate cjoin csuspend cresume semaforo
