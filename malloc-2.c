#include <stdlib.h>
#include <stdio.h>
int main(int argc, char *argv[]){
  int *ip = (int *)malloc(sizeof(int));
  *ip = 98765;
  printf("%d\n", *ip);
  exit(0);
}
