#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include <stdio.h>
#include <string.h>

int cidentify (char *name, int size){
  char id[] = "Vinicius Fraga de Castro 193026";
  if(strncpy(name,id,size) != NULL)
    return 0;
  else
    return -1; 
}
