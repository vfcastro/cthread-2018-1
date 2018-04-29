/*#############################################
# Programa de teste: liberacao voluntaria(cyield):
# - main() faz cyield e retorna execucao para si
##############################################*/

#include "../include/cthread.h"
#include <stdio.h>

int main(){
	
  printf("main(): cyield()\n");
  printf("main(): retorno cyield(): %d\n",cyield());	
  


  return 0;
}
