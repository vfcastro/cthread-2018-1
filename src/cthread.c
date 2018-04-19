#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/support.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 0
#define ERROR -1
#define UPDATE_CTX_TRUE 1
#define UPDATE_CTX_FALSE 0


FILA2	APTO;
FILA2	EXEC;
int	CTHREAD_INIT = 0;
int TID = 1;
ucontext_t sched_ctx;

//##########################
// FUNCOES AUXILIARES
//##########################

//========================

int move_thread(PFILA2 origem, PFILA2 destino, int novoEstado, int atualizaCtx){
	printf("move_thread()\n");
	// Recupera o TCB da fila ORIGEM
	if(FirstFila2(origem) != SUCCESS){
    	printf("move_thread(): FirstFila2(origem) failed!\n");
		return ERROR;
	}	
	
	PNODE2 nodeFilaAtual;
	TCB_t *tcb;
	nodeFilaAtual = GetAtIteratorFila2(origem);
	if(nodeFilaAtual == NULL){
		printf("move_thread(): GetAtIteratorFila2(origem) failed!");
		return ERROR;
	}
   	tcb = nodeFilaAtual->node;

	// Remove TCB da fila ORIGEM
	if(DeleteAtIteratorFila2(origem) != SUCCESS){
		printf("move_thread(): DeleteAtIteratorFila2(origem) failed!");
		return ERROR;
	}	

	// Insere TCB da thread em DESTINO
    PNODE2 nodeFilaNovo = (PNODE2)malloc(sizeof(NODE2));
    if(nodeFilaNovo == NULL){
		printf("move_thread(): malloc(sizeof(NODE2)) failed!");
    	return ERROR;
	}
    
	nodeFilaNovo->node = tcb;
	if(AppendFila2(destino,nodeFilaNovo) != SUCCESS){
		printf("move_thread(): AppendFila2(destino,nodeFilaNovo) failed!");
  		return ERROR;
	}
	
	// Atualiza estado e contexto da thread no TCB
	tcb->state = novoEstado;
	if(UPDATE_CTX_TRUE){
		if(getcontext(&(tcb->context)) == ERROR){
			printf("move_thread(): getcontext(&(tcb->context)) failed!");
			return ERROR;
		}		
	}

	return SUCCESS;
}
//========================

void print_queue(PFILA2 fila, char* name){
	//imprime APTOS
	PNODE2 nodeFila;
	TCB_t *tcb;
	if(FirstFila2(fila) != SUCCESS)
    	printf("print_queue(&%s): FirstFila2(fila) failed!\n",name);
	else {
		nodeFila = GetAtIteratorFila2(fila);
    	tcb = nodeFila->node;
		printf("print_queue(&%s): tid:%d\n",name,tcb->tid);
		while(NextFila2(fila) == SUCCESS){
			nodeFila = GetAtIteratorFila2(fila);	
			tcb = nodeFila->node;
			printf("print_queue(&%s): tid:%d\n",name,tcb->tid);
		}
	}
}
//========================

void sched(){
	printf("sched()\n");
	PNODE2 nodeFila;	
	TCB_t *tcb;

	print_queue(&APTO,"APTO");
	print_queue(&EXEC,"EXEC");

	// Move primeira thread de APTO para EXEC
    if(move_thread(&APTO,&EXEC,PROCST_EXEC,UPDATE_CTX_FALSE) != SUCCESS)
		printf("sched(): move_thread(&APTO,&EXEC,PROCST_EXEC,UPDATE_CTX_FALSE) failed!");

	print_queue(&APTO,"APTO");
	print_queue(&EXEC,"EXEC");

	// Muda para o contexto de EXEC
	if(FirstFila2(&EXEC) != SUCCESS)
    	printf("sched(): FirstFila2(&EXEC) failed!\n");
	else{	
		nodeFila = GetAtIteratorFila2(&EXEC);
		if(nodeFila == NULL)
			printf("sched(): GetAtIteratorFila2(&EXEC) failed!");
		else{
   			tcb = nodeFila->node;
			printf("sched(): tcb->context.uc_stack.ss_size: %d\n",tcb->context.uc_stack.ss_size);
			//setcontext(&(tcb->context));
		}
	}

}
//========================

int init(){
	printf("init()\n");
	// Cria contexto do dispatcher
	char *stack = (char*)malloc(SIGSTKSZ);
    if(stack == NULL){
		printf("init(): malloc(sizeof(SIGSTKSZ)) failed!");
    	return ERROR;
	}

	if(getcontext(&sched_ctx) == ERROR){
		printf("init(): getcontext(&sched_ctx) failed!");
		return ERROR;
	}	

    sched_ctx.uc_stack.ss_sp = stack;
    sched_ctx.uc_stack.ss_size = SIGSTKSZ;
    sched_ctx.uc_link = NULL;
    makecontext(&sched_ctx, sched, 0);	
	

	// Inicializa filas
	if(CreateFila2(&APTO) != SUCCESS || CreateFila2(&EXEC) != SUCCESS){
    	printf("init(): CreateFila2() failed!");
		return ERROR;
	}

	// Cria contexto da main
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

int get_tid(){
	printf("get_tid()\n");
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
	printf("cidentify()\n");
	if(!CTHREAD_INIT)
		if(init() == ERROR){
        	printf("cidentify(): init() failed!\n");
	    	return ERROR;		
		}

	char id[] = "Vinicius Fraga de Castro 193026";
	if(strncpy(name,id,size) != NULL)
		return SUCCESS;
	else
		return ERROR; 
}
//========================

int ccreate (void *(*start)(void *), void *arg, int prio){
	printf("ccreate()\n");
	// Inicializa cthread caso nao esteja
	if(!CTHREAD_INIT)
		if(init() == ERROR){
        	printf("ccreate(): init() failed!\n");
	    	return ERROR;		
		}	

	// Aloca pilha para a thread
	char *stack = (char*)malloc(SIGSTKSZ);
    if(stack == NULL){
		printf("ccreate(): malloc(sizeof(SIGSTKSZ)) failed!");
    	return ERROR;
	}

	// Aloca e inicializa TCB da thread
	TCB_t *tcb = (TCB_t*)malloc(sizeof(TCB_t));
    if(tcb == NULL){
		printf("ccreate(): malloc(sizeof(TCB_t)) failed!");
    	return ERROR;
	}
    tcb->tid = get_tid();
    tcb->state = PROCST_APTO;

	// Inicializa contexto da thread
	if(getcontext(&(tcb->context)) == ERROR){
		printf("ccreate(): getcontext(&(tcb->context)) failed!");
		return ERROR;
	}
    tcb->context.uc_stack.ss_sp = stack;
    tcb->context.uc_stack.ss_size = SIGSTKSZ;
    tcb->context.uc_link = &sched_ctx;
    makecontext(&(tcb->context), (void (*)(void))start, 1, (void *)arg);

	// Aloca elemento na fila APTO com o TCB associado
    PNODE2 nodeFila = (PNODE2)malloc(sizeof(NODE2));
    if(nodeFila == NULL){
		printf("ccreate(): malloc(sizeof(NODE2)) failed!");
    	return ERROR;
	}
    
	nodeFila->node = tcb;
	if(AppendFila2(&APTO,nodeFila) != SUCCESS){
		printf("ccreate(): AppendFila2(&APTO,nodeFila) failed!");
  		return ERROR;
	}

	return tcb->tid;
}
//========================

int cyield(void){
	printf("cyield()\n");
	// Move thread de EXEC para APTO
	if(move_thread(&EXEC,&APTO,PROCST_APTO,UPDATE_CTX_FALSE) != SUCCESS){
		printf("cyield(): move_thread(&EXEC,&APTO,PROCST_APTO) failed!");
  		return ERROR;
	}

	// Se a fila EXEC esta vazia, chama o escalonador
	if(FirstFila2(&EXEC) != SUCCESS)
		setcontext(&sched_ctx);


	return SUCCESS;
}
//========================













