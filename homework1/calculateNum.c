#include "calculateNum.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void generateData(void **array, unsigned long n, int m, int flag) {
  /* generate random data
 the array size is n
 the number range size is M
 flag is the data type 1 is integers 0 strings
 */
  srand((unsigned)time(NULL));
  if (flag) {
    *array = malloc(sizeof(int) * n);
    int *head = (int *)*array;
    for (int i = 0; i < n; i++) {
      int num = rand() % m + 1;
      *head = num;
      head++;
    }
  } else {
    *array = malloc(sizeof(char) * n);
    char *head = (char *)*array;
    for (int i = 0; i < n; i++) {
      int num = rand() % 26;
      *head = (char)'A' + num;
      head++;
    }
  }
}

void calculate(int element, int *target, int length) {
  /*
  Single process calculation of execution time
  */
  printf("single process\n");
  TIC int count = 0;
  for (int i = 0; i < length; i++) {
    if (target[i] == element) {
      count += 1;
    }
  }
  TOC printf("the element number is: %d\n", count);
}

void multiProcessCalculate(int element, int target[], int length, int mutinum) {
  /*
  Multi-process computing
  length is the array size
  mutinum is the number of child process, restricted to multiples of 2
  */
  int count = 0; // parent process count
  int subCount;  // child process count
  int interval = length / mutinum;
  printf("Multi-process number: %d\n", mutinum);
  TIC for (int i = 0; i < mutinum; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("fock failed\n");
      exit(-1);
    }
    if (pid == 0) {
      // child process
      int subCount = 0;
      for (int j = interval * i; j < interval * (i + 1); j++) {
        if (target[j] == element) {
          subCount++;
        }
      }
      // printf("No. of subprocess: %d pid is: %d  sub count: %d\n",i,getpid(),
      // subCount);
      exit(subCount);
    }
  }
  // parent process
  for (int i = 0; i < mutinum; i++) {
    int endPid = wait(&subCount);
    // printf("end child pid: %d  recoive number: %d\n", endPid, subCount/256);
    count += subCount / 256;
  }
  TOC printf("the element number is: %d\n", count);
}

void multiThreadCalculateLock(int element, int target[], int length,
                              int mutinum) {
  /*
  Multi-Tread computing using global val
  length is the array size
  mutinum is the number of child thread,restricted to multiples of 2
  */
  int count = 0;
  int interval = length / mutinum;
  pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  pthread_t Thread[mutinum];
  printf("Multi-Tread (global) number: %d\n", mutinum);
  void *calculateLock(int step) {
    // printf("child thread: %d\n", (int)pthread_self());
    for (int i = interval * step; i < interval * (step + 1); i++) {
      if (element == target[i]) {
        pthread_mutex_lock(&lock);
        count++;
        pthread_mutex_unlock(&lock);
      }
    }
    return NULL;
  }

  TIC for (int i = 0; i < mutinum; i++) {
    pthread_t thread;
    int err = pthread_create(&thread, NULL, calculateLock, i);
    if (err != 0) {
      printf("can't create thread");
      exit(-1);
    }
    Thread[i] = thread;
  }
  for (int i = 0; i < mutinum; i++) {
    pthread_join(Thread[i], NULL);
  }
  TOC printf("the element number is: %d\n", count);
}

void multiThreadCalculateExit(int element, int target[], int length,
                              int mutinum) {
  /*
  Multi-Tread computing using pthread_exit
  length is the array size
  mutinum is the number of child thread,restricted to multiples of 2
  */
  int interval = length / mutinum;
  int count = 0;
  pthread_t Thread[mutinum];
  printf("Multi-Tread (exit) number: %d\n", mutinum);
  void *calculateExit(int step) {
    // printf("child thread: %d\n", (int)pthread_self());
    int subCount = 0;
    for (int i = interval * step; i < interval * (step + 1); i++) {
      if (element == target[i]) {
        subCount++;
      }
    }
    pthread_exit(subCount);
  }

  TIC for (int i = 0; i < mutinum; i++) {
    pthread_t thread;
    int err = pthread_create(&thread, NULL, calculateExit, i);
    if (err != 0) {
      printf("can't create thread");
      exit(-1);
    }
    Thread[i] = thread;
  }
  for (int i = 0; i < mutinum; i++) {
    int subCount;
    pthread_join(Thread[i], &subCount);
    count += subCount;
  }
  TOC printf("the element number is: %d\n", count);
}

int main() {
  int m = 200;                                         // the range of number
  unsigned long N[] = {256, 262144, 2097152, 4194304}; // the array number

  int M[] = {2, 4, 8};
  for (int i = 0; i < 4; i++) {
    void *num;
    generateData(&num, N[i], m, 1);
    int *number = (int *)num;
    // Limit the number of elements to be queried
    srand((unsigned)time(NULL));
    int randNum = rand() % 240 + 1;
    for (int k = 20; k <= randNum; k++) {
      number[k] = 300;
    }
    printf("--------------array size: %ld----------\n", N[i]);
    printf("One process result\n");
    printf("------------------------\n");
    calculate(300, number, N[i]);
    printf("------------------------\n");
    printf("multi-process and multi thread result\n");
    for (int j = 0; j < 3; j++) {
      printf("------------------------\n");
      multiProcessCalculate(300, number, N[i], M[j]);
      printf("-------------------------\n");
      multiThreadCalculateLock(300, number, N[i], M[j]);
      printf("-------------------------\n");
      multiThreadCalculateExit(300, number, N[i], M[j]);
    }
    free(num);
  }
  return 0;
}
