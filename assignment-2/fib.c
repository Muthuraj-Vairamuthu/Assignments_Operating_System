#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fib(int n) {
  if(n<2) return n;
  else return fib(n-1)+fib(n-2);
}

int main(int argc, char* argv[]){
  if (argc < 2 || argc > 2){
    printf("Invalid arguments!");
  }else{
    int n = atoi(argv[1]);
    printf("%d",fib(n));
  }

  return 0;
}
