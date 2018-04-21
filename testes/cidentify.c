#include "../include/cthread.h"
#include <stdio.h>

void func0(int *arg) {
	printf("Eu sou a thread ID %d\n", *((int *)arg));
}

void func1(int *arg) {
	printf("Eu sou a thread ID %d\n", *((int *)arg));
}

int main(){
  char msg[40];
  int id0, id1;

//  id0 = ccreate((void *(*)(void*))func0, (void*)&id0, 0);
//  printf("main(): id0: %d\n",id0);
//  id1 = ccreate((void *(*)(void*))func1, (void*)&id1, 0);
//  printf("main(): id1: %d\n",id1);

//  if(cidentify(msg,sizeof(msg)) == 0)
//    printf("%s\n",msg);
	
  printf("main(): cyield()\n");
  cyield();
  printf("main(): retorno cyield()\n");	
  


  return 0;
}
