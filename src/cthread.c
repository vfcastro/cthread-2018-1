#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/support.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//##########################
// VARIAVEIS GLOBAIS CTHREAD
//##########################


#define SUCCESS 0
#define ERROR -1
#define SEARCH_BLOQ_JOIN_TRUE 0
#define SEARCH_BLOQ_JOIN_FALSE -1
#define SEARCH_BLOQ_SEM_TRUE 0
#define SEARCH_BLOQ_SEM_FALSE -1

// Filas de estado
FILA2	APTO;
FILA2	EXEC;
FILA2	BLOQ;
FILA2	APTO_SUS;
FILA2	BLOQ_SUS;
// Filas auxiliares
FILA2	BLOQ_JOIN;

int	CTHREAD_INIT = 0;
int TID = 1;
ucontext_t sched_ctx;
ucontext_t finish_ctx;




//##########################
// FUNCOES AUXILIARES:
//##########################

//========================
// Move o TCB identificado por tid da fila origem para o fim da fila destino
int move_thread_bytid(int tid, PFILA2 origem, PFILA2 destino, int novoEstado){
	// Percorre fila ORIGEM pelo tid 
	PNODE2 nodeFila;
	TCB_t *tcb;
	if(FirstFila2(origem) != SUCCESS){
    	printf("move_thread_bytid(): FirstFila2(origem) failed!\n");
		return ERROR;
	}

	do{
		nodeFila = GetAtIteratorFila2(origem);
		if(nodeFila == NULL){
			printf("move_thread_bytid(): GetAtIteratorFila2(origem) failed!");
			return ERROR;
		}
		tcb = nodeFila->node;
		// Encontrou o tid
		if(tcb->tid == tid){	
			// Remove TCB da fila ORIGEM
			if(DeleteAtIteratorFila2(origem) != SUCCESS){
				printf("move_thread_bytid(): DeleteAtIteratorFila2(origem) failed!\n");
				return ERROR;
			}

			// Insere TCB da thread em DESTINO
			PNODE2 nodeFilaNovo = (PNODE2)malloc(sizeof(NODE2));
			if(nodeFilaNovo == NULL){
				printf("move_thread_bytid(): malloc(sizeof(NODE2)) failed!\n");
				return ERROR;
			}    
			nodeFilaNovo->node = tcb;
			if(AppendFila2(destino,nodeFilaNovo) != SUCCESS){
				printf("move_thread_bytid(): AppendFila2(destino,nodeFilaNovo) failed!\n");
		  		return ERROR;
			}
			// Atualiza estado da thread no TCB
			tcb->state = novoEstado;
		}
	}	
	while(tcb->tid != tid && NextFila2(origem) == SUCCESS);

	return SUCCESS;
}


//========================
// Checa se um tid existe na fila origem
// SEARCH_BLOQ_JOIN_FALSE busca em fila de estados, retorna SUCCESS ou ERROR
// SEARCH_BLOQ_JOIN_TRUE busca em fila do tipo BLOQ_JOIN (reuso de codigo) e retorna o blocked_tid ou ERROR
// SEARCH_BLOQ_SEM_FALSE busca em fila de estados, retorna SUCCESS ou ERROR
// SEARCH_BLOQ_SEM_TRUE busca em fila de semaforos (reuso de codigo) e retorna o tid ou ERROR
int check_tid(int tid, PFILA2 origem, int SEARCH_BLOQ_JOIN, int SEARCH_BLOQ_SEM){
	// Seta o iterador no inicio da fila ORIGEM
	if(FirstFila2(origem) != SUCCESS){
    	//printf("check_tid(): FirstFila2(origem) failed!\n");
		return ERROR;
	}
	
	PNODE2 nodeFila;
	TCB_t *tcb;
	JOIN_t *join;
	// Percorre a lista em busca pelo tid 
	do{
		nodeFila = GetAtIteratorFila2(origem);
		if(nodeFila == NULL){
			printf("check_tid(): GetAtIteratorFila2(origem) failed!");
			return ERROR;
		}

		// A busca se da em fila de TCB
		if(SEARCH_BLOQ_JOIN == SEARCH_BLOQ_JOIN_FALSE && 
			SEARCH_BLOQ_SEM == SEARCH_BLOQ_SEM_FALSE){
   			tcb = nodeFila->node;
			// Encontrou o tid, retorna sucesso
			if(tcb->tid == tid)
				return SUCCESS;
		}

		// A busca se da em fila de BLOQ_JOIN
		if(SEARCH_BLOQ_JOIN == SEARCH_BLOQ_JOIN_TRUE){
   			join = nodeFila->node;
			// Encontrou o tid, retorna tid da thread bloqueada
			if(join->joined_tid == tid)
				return join->blocked_tid;
		}

	}	
	while(NextFila2(origem) == SUCCESS);

	// Nao encontrou, retorna erro
	return ERROR;

}

//========================
// Retorna ponteiro do TCB do inicio da fila origem
TCB_t* get_tcb(PFILA2 origem){
	// Recupera o TCB do inicio da fila ORIGEM
	if(FirstFila2(origem) != SUCCESS){
    	printf("get_tcb(): FirstFila2(origem) failed!\n");
		return NULL;
	}	
	
	PNODE2 nodeFila;
	TCB_t *tcb;
	nodeFila = GetAtIteratorFila2(origem);
	if(nodeFila == NULL){
		printf("get_tcb(): GetAtIteratorFila2(origem) failed!");
		return NULL;
	}
   	tcb = nodeFila->node;
	
	return tcb;
}


//========================
// Chamada ao termino de uma thread. Libera os recursos necessarios
void finish(){
	// Recupera o TCB da fila EXEC
	TCB_t *tcb = get_tcb(&EXEC);
	if(tcb == NULL){
		printf("finish(): get_tcb(&EXEC) failed!");
		exit(ERROR);
	}

	
	// Checa se ha alguma thread bloqueada em join aguardando o termino de EXEC
	int blocked_tid = check_tid(tcb->tid,&BLOQ_JOIN,SEARCH_BLOQ_JOIN_TRUE,SEARCH_BLOQ_SEM_FALSE);
	if( blocked_tid != ERROR){
		// Thread blocked_tid esta bloqueada pela thread em EXEC. Deve-se:
		// Remover entrada da fila BLOQ_JOIN
		// Mover a thread blocked_tid de BLOQ -> APTO ou BLOQ_SUS -> APTO_SUS

		// Remove entrada da fila BLOQ_JOIN. Iterador ja esta no node blocked_tid
		PNODE2 nodeFila = GetAtIteratorFila2(&BLOQ_JOIN);
		if(nodeFila == NULL){
			printf("finish(): GetAtIteratorFila2(&BLOQ_JOIN) failed!\n");
			exit(ERROR);
		}
		JOIN_t *join_node = nodeFila->node;
		if(DeleteAtIteratorFila2(&BLOQ_JOIN) != SUCCESS){
			printf("finish(): DeleteAtIteratorFila2(&BLOQ_JOIN) failed!\n");
			exit(ERROR);
		}		
		// Libera memoria node fila BLOQ_JOIN
		free(join_node);

		// Caso em BLOQ, move de BLOQ -> APTO
		if(check_tid(blocked_tid,&BLOQ,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == SUCCESS){
			if(move_thread_bytid(blocked_tid,&BLOQ,&APTO,PROCST_APTO) != SUCCESS){
				printf("finish(): move_thread_bytid(blocked_tid,&BLOQ,&APTO,PROCST_APTO) failed!\n");
				exit(ERROR);
			}
		}
		// Caso em BLOQ_SUS, move de BLOQ_SUS -> APTO_SUS
		else{
			if(check_tid(blocked_tid,&BLOQ_SUS,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == SUCCESS)
				if(move_thread_bytid(blocked_tid,&BLOQ_SUS,&APTO_SUS,PROCST_APTO_SUS) != SUCCESS){
					printf("finish(): move_thread_bytid(blocked_tid,&BLOQ_SUS,&APTO_SUS,PROCST_APTO_SUS) failed!\n");
					exit(ERROR);
				}
		}	
	}


	// Remove TCB da fila EXEC
	if(FirstFila2(&EXEC) != SUCCESS){
    	printf("finish(): FirstFila2(&EXEC) failed!\n");
		exit(ERROR);
	}	
	if(DeleteAtIteratorFila2(&EXEC) != SUCCESS){
		printf("finish(): DeleteAtIteratorFila2(&EXEC) failed!");
		exit(ERROR);
	}
	
	// Libera a memoria alocada
	free(tcb->context.uc_stack.ss_sp);
	free(tcb);

	// Chama o escalonador
	setcontext(&sched_ctx);
}


//========================
// Move o primeiro da fila origem para o fim da fila destino
int move_thread(PFILA2 origem, PFILA2 destino, int novoEstado){
	// Recupera o TCB da fila ORIGEM
	TCB_t *tcb = get_tcb(origem);
	if(tcb == NULL){
		printf("move_thread(): get_tcb(origem) failed!\n");
		return ERROR;
	}

	// Remove TCB da fila ORIGEM
	if(FirstFila2(origem) != SUCCESS){
    	printf("move_thread(): FirstFila2(origem) failed!\n");
		return ERROR;
	}	
	if(DeleteAtIteratorFila2(origem) != SUCCESS){
		printf("move_thread(): DeleteAtIteratorFila2(origem) failed!\n");
		return ERROR;
	}	

	// Insere TCB da thread em DESTINO
    PNODE2 nodeFilaNovo = (PNODE2)malloc(sizeof(NODE2));
    if(nodeFilaNovo == NULL){
		printf("move_thread(): malloc(sizeof(NODE2)) failed!\n");
    	return ERROR;
	}    
	nodeFilaNovo->node = tcb;
	if(AppendFila2(destino,nodeFilaNovo) != SUCCESS){
		printf("move_thread(): AppendFila2(destino,nodeFilaNovo) failed!\n");
  		return ERROR;
	}
	
	// Atualiza estado da thread no TCB
	tcb->state = novoEstado;

	return SUCCESS;
}

//========================
// Imprime dados do TCB da fila
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
// Scheduler e dispatcher
void sched(){
	// Move primeira thread de APTO para EXEC
    if(move_thread(&APTO,&EXEC,PROCST_EXEC) != SUCCESS){
		printf("sched(): move_thread(&APTO,&EXEC,PROCST_EXEC) failed!\n");
		exit(ERROR);
	}

	// Muda para o contexto de EXEC
	TCB_t *tcb = get_tcb(&EXEC);
	if(tcb == NULL){
		printf("sched(): get_tcb(&EXEC) failed!\n");
		exit(ERROR);
	}
	setcontext(&(tcb->context));

}

//========================
// Inicializa estruturas de dados da cthread
int init(){
	// Cria contexto para o sched()
	char *stack = (char*)malloc(SIGSTKSZ);
    if(stack == NULL){
		printf("init(): malloc(sizeof(SIGSTKSZ)) failed!\n");
    	return ERROR;
	}

	if(getcontext(&sched_ctx) == ERROR){
		printf("init(): getcontext(&sched_ctx) failed!\n");
		return ERROR;
	}	

    sched_ctx.uc_stack.ss_sp = stack;
    sched_ctx.uc_stack.ss_size = SIGSTKSZ;
    sched_ctx.uc_link = NULL;
    makecontext(&sched_ctx, sched, 0);	

	// Cria contexto para o finish()
	char *stack_finish = (char*)malloc(SIGSTKSZ);
    if(stack_finish == NULL){
		printf("init(): malloc(sizeof(SIGSTKSZ)) failed!");
    	return ERROR;
	}

	if(getcontext(&finish_ctx) == ERROR){
		printf("init(): getcontext(&sched_ctx) failed!");
		return ERROR;
	}	

    finish_ctx.uc_stack.ss_sp = stack_finish;
    finish_ctx.uc_stack.ss_size = SIGSTKSZ;
    finish_ctx.uc_link = NULL;
    makecontext(&finish_ctx, finish, 0);	
	

	// Inicializa filas
	if(CreateFila2(&APTO) != SUCCESS || CreateFila2(&EXEC) != SUCCESS ||
	   	CreateFila2(&BLOQ) || CreateFila2(&APTO_SUS) || CreateFila2(&BLOQ_SUS) ||
          CreateFila2(&BLOQ_JOIN)){
    	printf("init(): CreateFila2() failed!");
		return ERROR;
	}

	// Cria TCB da main
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

	nodeFila->node = tcb;
	if(AppendFila2(&EXEC,nodeFila) != SUCCESS){
		printf("init(): AppendFila2(&EXEC,nodeFila) failed!");
  		return ERROR;
	}

    tcb->tid = 0;
    tcb->state = PROCST_EXEC;

	CTHREAD_INIT = 1;

	return SUCCESS;
}

//========================
// Retorna TID sequencial
int get_tid(){
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
	// Inicializa cthread caso nao esteja
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
    tcb->context.uc_link = &finish_ctx;
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

	// Retorna o TID
	return tcb->tid;
}
//========================

int cyield(void){
	// Inicializa cthread caso nao esteja
	if(!CTHREAD_INIT){
		if(init() == ERROR){
        	printf("cyield(): init() failed!\n");
	    	return ERROR;		
		}
	}		

	// Recupera TCB da thread em EXEC
	TCB_t *tcb = get_tcb(&EXEC);
	if(tcb == NULL){
		printf("cyield(): get_tcb(&EXEC); failed!\n");
		return ERROR;
	}

	// Move thread de EXEC para APTO
	if(move_thread(&EXEC,&APTO,PROCST_APTO) != SUCCESS){
		printf("cyield(): move_thread(&EXEC,&APTO,PROCST_APTO) failed!");
		return ERROR;
	}

	// Salva contexto da thread no TCB
	if(getcontext(&(tcb->context)) == ERROR){
		printf("cyield(): getcontext(&(tcb->context)) failed!");
		return ERROR;
	}

	// Fila EXEC liberada, chama o escalonador
	if(FirstFila2(&EXEC) != SUCCESS)
		setcontext(&sched_ctx);

	// Thread retoma execucao
	return SUCCESS;
}
//========================

int cjoin(int tid){
	// Inicializa cthread caso nao esteja
	if(!CTHREAD_INIT){
		if(init() == ERROR){
        	printf("cjoin(): init() failed!\n");
	    	return ERROR;		
		}
	}

	// Checa se o tid existe nas filas de estados
	if(check_tid(tid,&APTO,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == ERROR &&
		check_tid(tid,&BLOQ,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == ERROR &&
		check_tid(tid,&APTO_SUS,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == ERROR &&
		check_tid(tid,&BLOQ_SUS,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == ERROR)
		// Nao encontrou o tid em nenhuma fila
		return ERROR;

	// Checa se ha outra thread bloqueada pelo mesmo tid
	if(check_tid(tid,&BLOQ_JOIN,SEARCH_BLOQ_JOIN_TRUE,SEARCH_BLOQ_SEM_FALSE) != ERROR)
		return ERROR;

	// Recupera TCB da thread em EXEC
	TCB_t *tcb = get_tcb(&EXEC);
	if(tcb == NULL){
		printf("cjoin(): get_tcb(&EXEC); failed!\n");
		return ERROR;
	}

	// Inclui tid na fila de bloqueados por join (BLOQ_JOIN):
	// Aloca e inicializa struct de bloqueados por join (JOIN_t)
	JOIN_t *join = (JOIN_t*)malloc(sizeof(JOIN_t));
    if(join == NULL){
		printf("cjoin(): malloc(sizeof(JOIN_t)) failed!");
    	return ERROR;
	}
    join->blocked_tid = tcb->tid;
    join->joined_tid = tid;

	// Aloca elemento na fila de bloqueados por join (BLOQ_JOIN) e adiciona JOIN_t:
    PNODE2 nodeFila = (PNODE2)malloc(sizeof(NODE2));
    if(nodeFila == NULL){
		printf("cjoin(): malloc(sizeof(NODE2)) failed!");
    	return ERROR;
	}    
	nodeFila->node = join;
	if(AppendFila2(&BLOQ_JOIN,nodeFila) != SUCCESS){
		printf("cjoin(): AppendFila2(&BLOQ_JOIN,nodeFila) failed!");
  		return ERROR;
	}

	// Move thread de EXEC para BLOQ
	if(move_thread(&EXEC,&BLOQ,PROCST_BLOQ) != SUCCESS){
		printf("cjoin(): move_thread(&EXEC,&BLOQ,PROCST_BLOQ) failed!");
		return ERROR;
	}

	// Salva contexto da thread atual no TCB
	if(getcontext(&(tcb->context)) == ERROR){
		printf("cjoin(): getcontext(&(tcb->context)) failed!");
		return ERROR;
	}

	// Fila EXEC liberada, chama o escalonador
	if(FirstFila2(&EXEC) != SUCCESS)
		setcontext(&sched_ctx);
	

	return SUCCESS;	
}
//========================

int csuspend(int tid){
	// Inicializa cthread caso nao esteja
	if(!CTHREAD_INIT){
		if(init() == ERROR){
        	printf("csuspend(): init() failed!\n");
	    	return ERROR;		
		}
	}

	// Checa se o tid existe na fila APTO
	if(check_tid(tid,&APTO,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == SUCCESS){
		// Encontrou o tid na fila APTO. Move para fila APTO_SUS
		if(move_thread_bytid(tid,&APTO,&APTO_SUS,PROCST_APTO_SUS) == ERROR){
			printf("csuspend(): move_thread_bytid(tid,&APTO,&APTO_SUS,PROCST_APTO_SUS) failed!\n");
			return ERROR;
		}
		else return SUCCESS;
	}

	// Checa se o tid existe na fila BLOQ
	if(check_tid(tid,&BLOQ,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == SUCCESS){
		// Encontrou o tid na fila BLOQ. Move para fila BLOQ_SUS
		if(move_thread_bytid(tid,&BLOQ,&BLOQ_SUS,PROCST_BLOQ_SUS) == ERROR){
			printf("csuspend(): move_thread_bytid(tid,&BLOQ,&BLOQ_SUS,PROCST_BLOQ_SUS) failed!\n");
			return ERROR;
		}
		else return SUCCESS;
	}

	// Nao encontrou o tid em APTO ou BLOQ para suspender. Retorna ERRO
	return ERROR;

}
//========================

int cresume(int tid){
	// Inicializa cthread caso nao esteja
	if(!CTHREAD_INIT){
		if(init() == ERROR){
        	printf("cresume(): init() failed!\n");
	    	return ERROR;		
		}
	}

	// Checa se o tid existe na fila APTO_SUS
	if(check_tid(tid,&APTO_SUS,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == SUCCESS){
		// Encontrou o tid na fila APTO_SUS. Move para fila APTO
		if(move_thread_bytid(tid,&APTO_SUS,&APTO,PROCST_APTO) == ERROR){
			printf("cresume(): move_thread_bytid(tid,&APTO_SUS,&APTO,PROCST_APTO) failed!\n");
			return ERROR;
		}
		else return SUCCESS;
	}

	// Checa se o tid existe na fila BLOQ_SUS
	if(check_tid(tid,&BLOQ_SUS,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == SUCCESS){
		// Encontrou o tid na fila BLOQ_SUS. Move para fila BLOQ
		if(move_thread_bytid(tid,&BLOQ_SUS,&BLOQ,PROCST_BLOQ) == ERROR){
			printf("cresume(): move_thread_bytid(tid,&BLOQ_SUS,&BLOQ,PROCST_BLOQ) failed!\n");
			return ERROR;
		}
		else return SUCCESS;
	}

	// Nao encontrou o tid em APTO_SUS ou BLOQ_SUS para resumir. Retorna ERRO
	return ERROR;

}
//========================


int csem_init(csem_t *sem, int count){
	// Inicializa cthread caso nao esteja
	if(!CTHREAD_INIT)
		if(init() == ERROR){
        	printf("csem_init(): init() failed!\n");
	    	return ERROR;		
		}	

	// Aloca memoria para a fila do semaforo
	PFILA2 fila = (PFILA2)malloc(sizeof(FILA2));
	if(fila == NULL){
		printf("csem_init(): (PFILA)malloc(sizeof(FILA2) failed!\n");
		return ERROR;
	}
	sem->fila = fila;

	// Inicializa fila do semaforo
	if(CreateFila2(sem->fila) != SUCCESS){
    	printf("csem_init(): CreateFila2(sem->fila) failed!\n");
		return ERROR;
	}
	// Inicializa contador
	sem->count = count;

	return SUCCESS;
}
//========================

int cwait(csem_t *sem){
	// Inicializa cthread caso nao esteja
	if(!CTHREAD_INIT)
		if(init() == ERROR){
        	printf("cwait(): init() failed!\n");
	    	return ERROR;		
		}

	
	// Se o semaforo esta livre, decrementa e segue execucao
	if(sem->count >0){
		sem->count = sem->count - 1;
		return SUCCESS;
	}
	else{
		// Semaforo ocupado:
		// - Decrementa semaforo
		sem->count = sem->count - 1;

		// - Move thread de EXEC para BLOQ:
		// Recupera TCB da thread em EXEC
		TCB_t *tcb = get_tcb(&EXEC);
		if(tcb == NULL){
			printf("cwait(): get_tcb(&EXEC); failed!\n");
			return ERROR;
		}

		// Move thread de EXEC para BLOQ
		if(move_thread(&EXEC,&BLOQ,PROCST_BLOQ) != SUCCESS){
			printf("cwait(): move_thread(&EXEC,&BLOQ,PROCST_BLOQ) failed!\n");
			return ERROR;
		}

		// - Adiciona tid da thread bloqueada na fila do semaforo:
		// Aloca memoria pra o tid
		int *tid = (int*)malloc(sizeof(int));
		if(tid == NULL){
			printf("cwait(): (int*)malloc(sizeof(int)) failed!\n");
			return ERROR;
		}
		*tid = tcb->tid;

		// Aloca memoria para o node da fila do semaforo
		PNODE2 nodeFila = (PNODE2)malloc(sizeof(NODE2));
		if(nodeFila == NULL){
			printf("cwait(): malloc(sizeof(NODE2)) failed!\n");
			return ERROR;
		}    
		nodeFila->node = tid;

		// Adiciona o node na fila
		if(AppendFila2(sem->fila,nodeFila) != SUCCESS){
			printf("cwait(): AppendFila2(&(sem->fila),nodeFila) failed!\n");
	  		return ERROR;
		}

		// Salva contexto da thread
		if(getcontext(&(tcb->context)) == ERROR){
			printf("cwait(): getcontext(&(tcb->context)) failed!\n");
			return ERROR;
		}

		// Fila EXEC liberada, chama o escalonador
		if(FirstFila2(&EXEC) != SUCCESS)
			setcontext(&sched_ctx);

	}

	// Thread retoma execucao
	return SUCCESS;

}
//========================


int csignal(csem_t *sem){
	// Inicializa cthread caso nao esteja
	if(!CTHREAD_INIT)
		if(init() == ERROR){
        	printf("csignal(): init() failed!\n");
	    	return ERROR;		
		}

	// Incrementa semaforo indicando liberacao de recurso
	sem->count = sem->count + 1;


	// Recupera o node do inicio da fila do semaforo.
	// Se fila vazia, retorna (nao ha thread para desbloquear)
	if(FirstFila2(sem->fila) != SUCCESS){
    	//printf("csignal(): FirstFila2(sem->fila) failed!\n");
		return SUCCESS;
	}	
	
	// Ha thread bloqueada. Recupera o primeiro tid da fila
	PNODE2 nodeFila;
	int *tid;
	nodeFila = GetAtIteratorFila2(sem->fila);
	if(nodeFila == NULL){
		printf("csignal(): GetAtIteratorFila2(sem->fila) failed!\n");
		return ERROR;
	}
   	tid = nodeFila->node;

	// Caso tid esteja em BLOQ, move para APTO
	if(check_tid(*tid,&BLOQ,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == SUCCESS){
		if(move_thread_bytid(*tid,&BLOQ,&APTO,PROCST_APTO) != SUCCESS){
			printf("csignal(): move_thread_bytid(*tid,&BLOQ,&APTO,PROCST_APTO) failed!\n");
			return ERROR;
		}
	}
	// Caso tid esteja em BLOQ_SUS, move para APTO_SUS
	else{
		if(check_tid(*tid,&BLOQ_SUS,SEARCH_BLOQ_JOIN_FALSE,SEARCH_BLOQ_SEM_FALSE) == SUCCESS){
			if(move_thread_bytid(*tid,&BLOQ_SUS,&APTO_SUS,PROCST_APTO_SUS) != SUCCESS){
				printf("csignal(): move_thread_bytid(*tid,&BLOQ_SUS,&APTO_SUS,PROCST_APTO_SUS) failed!\n");
				return ERROR;
			}
		}
		// Nao econtrou a thread bloqueada nas filas: estados inconsistentes!
		else return ERROR;		
	}

	// Libera node da fila do semaforo
	if(DeleteAtIteratorFila2(sem->fila) != SUCCESS){
		printf("csignal(): DeleteAtIteratorFila2(sem->fila) failed!\n");
		return ERROR;
	}
	
	return SUCCESS;
}
//========================		
