#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <sys/time.h>

struct timeval start;
struct timeval end;

typedef struct lambda_params{
  int start;
  int end;
  std::function<void(int)> lambda;
}lambda_params;




typedef struct lambda_params2{
  int start1;
  int end1;
  int start2;
  int end2;
  std::function<void(int, int)>  lambda;

}lambda_params2;


int user_main(int argc, char **argv);
/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */


void demonstration(std::function<void()> && lambda) {
  lambda();
}

void* fnc_thread(void* arg){

  struct lambda_params * p = (struct lambda_params *)arg;
  for(int i=p->start;i<=p->end;i++){
    p->lambda(i);
  }
  return NULL;
}




void* fnc_thread2(void* arg){

  struct lambda_params2 * p = (struct lambda_params2 *)arg;

  for(int i=p->start2;i<p->end2;i++){
    for(int j=p->start1;j<=p->end1;j++){
      p->lambda(i,j);
    }
  }

  return NULL;
}



pthread_t* threads;
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads)
{
  gettimeofday(&start,NULL);
  int chunk_size=(high-low+1)/numThreads;
  threads = new pthread_t[numThreads];
  lambda_params params[numThreads];

  for(int i=0;i<numThreads;i++){
    int start= low+i*chunk_size;
    int end = (i == numThreads - 1) ? high : start + chunk_size - 1;
    params[i].start=start;
    params[i].end=end;
    params[i].lambda=lambda;
    
    if (pthread_create(&threads[i],NULL,fnc_thread,&params[i]) != 0){
      printf("Thread Creation failed!\n");
    }
  }
  for(int i =0;i<numThreads;i++){
    pthread_join(threads[i],NULL);
  }
  gettimeofday(&end,NULL);
  double duration = (end.tv_sec - start.tv_sec) +
                             (end.tv_usec - start.tv_usec) / 1000000.0;

  delete[] threads;

  printf("---------------------------------\n");
  printf("Duration for execution %.10f ms\n" , duration);
  printf("No of threads %d \n",numThreads);
  printf("---------------------------------\n");

}


void parallel_for(int low1, int high1,  int low2, int high2, std::function<void(int, int)>  &&lambda, int numThreads)
{
  gettimeofday(&start,NULL);
  int chunk_size=(high1-low1+1)/numThreads;
  threads = new pthread_t[numThreads];
  lambda_params2 params[numThreads];
  for(int i=0;i<numThreads;i++){

    int start= low1+i*chunk_size;
    int end = (i == numThreads - 1) ? high1 : start + chunk_size - 1;

    params[i].start1=start;
    params[i].start2=low2;
    params[i].end1=end;
    params[i].end2=high2;

    params[i].lambda = lambda;
    if (pthread_create(&threads[i],NULL,fnc_thread2,&params[i]) != 0){
        printf("Pthread creation failed!\n");
    }
  }


  for(int i =0;i<numThreads;i++){
    pthread_join(threads[i],NULL);
  }

  gettimeofday(&end,NULL);
  delete[] threads;
  double duration = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
  printf("---------------------------------\n");
  printf("Duration for execution %.10f ms\n" , duration);
  printf("No of threads %d \n",numThreads);
  printf("---------------------------------\n");
}

int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);
 
  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main

