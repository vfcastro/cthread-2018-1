/*#############################################
# Programa de teste: semaforos:
# - main() cria 2 threads e inicializa semaforo binario sem;
# - main() faz cwait(sem) e apos libera a CPU (cyield);
# - ID1 executa e faz cwait(sem): eh bloqueada no semaforo;
# - ID2 executa, suspende ID1 (ID1 vai para BLOQ_SUS) e termina;
# - main() retorna, faz csignal(sem) (libera ID1 da fila do semaforo)
#   resume ID1 e libera CPU;
# - ID1 retorna, faz csignal(sem) e termina;
# - main() retorna e termina
##############################################*/

#include "../include/cthread.h"
#include <stdio.h>
csem_t sem;

void func1(int *tid) {
	printf("Eu sou a thread ID %d fazendo cwait(&sem)\n", tid[0]);
	printf("Eu sou a thread ID %d: retorno cwait(&sem): %d\n",tid[0],cwait(&sem));	
	printf("Eu sou a thread ID %d: csignal(&sem): %d\n",tid[0],csignal(&sem));
	printf("Eu sou a thread ID %d terminando\n", tid[0]);
}

void func2(int *tid) {
	printf("Eu sou a thread ID %d comecando\n", tid[1]);
	printf("Eu sou a thread ID %d fazendo csuspend(%d): %d\n",tid[1],tid[0],csuspend(tid[0]));
	printf("Eu sou a thread ID %d terminando\n", tid[1]);
}

int main(){
	if(csem_init(&sem,1) < 0){
		printf("main(): csem_init(1) failed!\n");
		return -1;
	}

	int tid[2];	

	tid[0] = ccreate((void *(*)(void*))func1, (void*)&tid, 0);
	printf("main(): id1: %d\n",tid[0]);

	tid[1] = ccreate((void *(*)(void*))func2, (void*)&tid, 0);
	printf("main(): id2: %d\n",tid[1]);

	printf("main(): cwait(&sem): %d\n",cwait(&sem));
	printf("main(): fazendo cyield()\n");
	printf("main(): retorno cyield(): %d\n",cyield());
	printf("main(): csignal(&sem): %d\n",csignal(&sem));
	printf("main(): cresume(%d): %d\n",tid[0],cresume(tid[0]));
	printf("main(): fazendo cyield()\n");
	printf("main(): retorno cyield(): %d\n",cyield());

	
	printf("main(): terminando\n");

    return 0;
}
