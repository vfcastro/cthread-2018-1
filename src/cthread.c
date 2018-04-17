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

/*##########################
# FUNCOES AUXILIARES
##########################*/
int init(){
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

	getcontext(&(tcb->context));
	return SUCCESS;
}

/*##########################
# FUNCOES DA API
##########################*/
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
