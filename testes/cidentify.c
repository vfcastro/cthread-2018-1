#include "../include/cthread.h"
#include <stdio.h>

int main(){
  char msg[40];
  if(cidentify(msg,sizeof(msg)) == 0)
    printf("%s\n",msg);


  return 0;
}
