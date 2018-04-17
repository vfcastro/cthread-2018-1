#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/support.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 0
#define ERROR -1

FILA2	APTO;
FILA2	EXEC;
int	CTHREAD_INIT = 0;
int TID = 1;
static ucontext_t sched_ctx;

//##########################
// FUNCOES AUXILIARES
//##########################

//========================

void sched(){

}
//========================

int init(){
	// Cria contexto do dispatcher
	char *stack = (char*)malloc(sizeof(SIGSTKSZ));
    if(stack == NULL){
		printf("init(): malloc(sizeof(SIGSTKSZ)) failed!");
    	return ERROR;
	}

	if(getcontext(&sched_ctx) == ERROR){
		printf("init(): getcontext(&sched_ctx) failed!");
		return ERROR;
	}	

    sched_ctx.uc_stack.ss_sp = stack;
    sched_ctx.uc_stack.ss_size = sizeof(stack);
    sched_ctx.uc_link = NULL;
    makecontext(&sched_ctx, sched, 0);	

	// Cria contexto da main
	if(CreateFila2(&APTO) != SUCCESS || CreateFila2(&EXEC) != SUCCESS){
    	printf("init(): CreateFila2() failed!");
		return ERROR;
	}

    PNODE2 nodeFila = (PNODE2)malloc(sizeof(NODE2));
    if(nodeFila == NULL){
		printf("init(): malloc(sizeof(NODE2)) failed!");
    	return ERROR;
	}

	TCB_t *tcb = (TCB_t*)malloc(sizeof(TCB_t));
    if(tcb == NULL){
		printf("init(): malloc(sizeof(TCB_t)) failed!");
    	return ERROR;
	}
    tcb->tid = 0;
    tcb->state = PROCST_EXEC;

	nodeFila->node = tcb;
	if(AppendFila2(&EXEC,nodeFila) != SUCCESS){
		printf("init(): AppendFila2(&EXEC,nodeFila) failed!");
  		return ERROR;
	}	

	CTHREAD_INIT = 1;

	if(getcontext(&(tcb->context)) == ERROR){
		printf("init(): getcontext(&(tcb->context)) failed!");
		return ERROR;
	}
    	
	return SUCCESS;
}
//========================

int getTid(){
	int t = TID;
	TID = TID + 1;
	return t;
}
//========================



//##########################
//# FUNCOES DA API
//##########################

//========================

int cidentify (char *name, int size){
	if(!CTHREAD_INIT)
		if(init() == ERROR){
        	printf("cidentify(): init() failed!\n");
	    	return ERROR;		
		}
	
/*	if(FirstFila2(&EXEC) != SUCCESS)
    	printf("FirstFila2(&EXEC) failed!");
	PNODE2 nodeFila = GetAtIteratorFila2(&EXEC);
    TCB_t *tcb = nodeFila->node;
	printf("tid:%d",tcb->state);
*/

	char id[] = "Vinicius Fraga de Castro 193026";
	if(strncpy(name,id,size) != NULL)
		return SUCCESS;
	else
		return ERROR; 
}
//========================

int ccreate (void *(*start)(void *), void *arg, int prio){
	if(!CTHREAD_INIT)
		if(init() == ERROR){
        	printf("ccreate(): init() failed!\n");
	    	return ERROR;		
		}	

	char *stack = (char*)malloc(sizeof(SIGSTKSZ));
    if(stack == NULL){
		printf("ccreate(): malloc(sizeof(SIGSTKSZ)) failed!");
    	return ERROR;
	}

	TCB_t *tcb = (TCB_t*)malloc(sizeof(TCB_t));
    if(tcb == NULL){
		printf("ccreate(): malloc(sizeof(TCB_t)) failed!");
    	return ERROR;
	}
    tcb->tid = getTid();
    tcb->state = PROCST_APTO;

	if(getcontext(&(tcb->context)) == ERROR){
		printf("ccreate(): getcontext(&(tcb->context)) failed!");
		return ERROR;
	}
    tcb->context.uc_stack.ss_sp = stack;
    tcb->context.uc_stack.ss_size = sizeof(stack);
    tcb->context.uc_link = &sched_ctx;
    makecontext(&(tcb->context), (void (*)(void))start, 1, (void *)arg);



	return 0;
}
//========================
