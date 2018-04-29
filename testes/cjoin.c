/*#############################################
# Programa de teste: sincronizacao de termino:
# - main() cria 2 threads e faz join na ID1;
# - ID1 executa e faz join na ID2;
# - ID2 termina e desbloqueia ID1;
# - ID1 termina e desbloqueia main();
# - retorna para main() e termina execucao
##############################################*/

#include "../include/cthread.h"
#include <stdio.h>

void func1(int *tids) {
	printf("Eu sou a thread ID %d fazendo join na thread ID %d\n", tids[0], tids[1]);
	printf("Eu sou a thread ID %d: retorno cjoin(%d): %d\n",tids[0],tids[1],cjoin(tids[1]));
	printf("Eu sou a thread ID %d terminando\n", tids[0]);
}

void func2(int *tids) {
	printf("Eu sou a thread ID %d comecando\n", tids[1]);
	printf("Eu sou a thread ID %d terminando\n", tids[1]);
}

int main(){
	int tids[2];	

	tids[0] = ccreate((void *(*)(void*))func1, (void*)&tids, 0);
	printf("main(): id1: %d\n",tids[0]);

	tids[1] = ccreate((void *(*)(void*))func2, (void*)&tids, 0);
	printf("main(): id2: %d\n",tids[1]);

	printf("main(): cjoin(%d)\n",tids[0]);
	printf("main(): retorno cjoin(): %d\n",cjoin(tids[0]));	

    return 0;
}
