#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#define  THREAD_NUM 50
#define NITER 1

int cnt = 5;
sem_t mutex, mutex2;
pthread_t pthreads[THREAD_NUM];

void * Count(void * a)
{
    printf("Hello from thread %u - I was created in iteration %d !\n", (int)pthread_self(), (int)a);
    pthread_exit(NULL);
   /* int i, tmp;
    for(i = 0; i < NITER; i++)
    {
       /* sem_wait(&mutex);
        printf("Thread %d, Semáforo tomado con éxito!\n", pid);
        cnt--;
        sem_post(&mutex);*/
        /*sem_wait(&mutex2);
        printf("Thread %d, Semáforo tomado con éxito 2!\n", pid);
        cnt++;
        sem_post(&mutex2);

    }*/
}

void threadCreation(){
    int rc;
    for (int i = 0; i < THREAD_NUM; ++i) {
        rc = pthread_create(&pthreads[i], NULL, Count, (void*)i);
        if(rc)
        {
            printf("\n ERROR creating thread 1");
            exit(1);
        }
        printf("\n I am thread %u. Created new thread (%u) in iteration %d ...\n", (int)pthread_self(), (int)pthreads[i], i);
        if (i % 58 == 0) sleep(1);
    }
    pthread_exit(NULL);
}

int main() {
    //sem_init(&mutex, 0, 1);
    //sem_init(&mutex2, 0, 1);
    printf("Hello, World!\n");
    threadCreation();
    printf("Count: %d\n", cnt);
    return 0;
}