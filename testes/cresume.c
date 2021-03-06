/*#############################################
# Programa de teste: suspensao e retomada:
# - main() cria 2 threads e faz join na ID1;
# - ID1 suspende ID2 e termina desbloqueando main();
# - main() retorna e faz join na ID2;
# - ID2 executa e desbloqueia main();
# - retorna para main() e termina execucao
##############################################*/

#include "../include/cthread.h"
#include <stdio.h>

void func1(int *tid) {
	printf("Eu sou a thread ID %d suspendendo a thread ID %d\n", tid[0], tid[1]);
	csuspend(tid[1]);
	printf("Eu sou a thread ID %d terminando\n", tid[0]);
}

void func2(int *tid) {
	printf("Eu sou a thread ID %d terminando\n", tid[1]);
}

int main(){
	int tid[2];	

	tid[0] = ccreate((void *(*)(void*))func1, (void*)&tid, 0);
	printf("main(): id1: %d\n",tid[0]);

	tid[1] = ccreate((void *(*)(void*))func2, (void*)&tid, 0);
	printf("main(): id2: %d\n",tid[1]);

	printf("main(): cjoin(%d)\n",tid[0]);
	printf("main(): retorno cjoin(%d): %d\n",tid[0],cjoin(tid[0]));
	

	printf("main(): cresume(%d)\n",tid[1]);
	printf("main(): retorno cresume(%d): %d\n",tid[1],cresume(tid[1]));
	
	printf("main(): cjoin(%d)\n",tid[1]);
	printf("main(): retorno cjoin(%d): %d\n",tid[1],cjoin(tid[1]));
	
	
 	printf("main(): fim\n");	

    return 0;
}
