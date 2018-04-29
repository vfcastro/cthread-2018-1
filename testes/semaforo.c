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
	printf("main(): fazendo cyield()\n");
	printf("main(): retorno cyield(): %d\n",cyield());

	
	printf("main(): terminando\n");

    return 0;
}
