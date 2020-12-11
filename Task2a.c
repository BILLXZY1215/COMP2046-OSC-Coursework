#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include "coursework.h"
#include "linkedlist.h"


struct data {
    sem_t empty;
    sem_t full;
    sem_t mutex;
    struct element * pHead;
    struct element * pTail;
    int NUMBER_OF_PROCESS_CREATED;
    double Avg_response_time;
    double Avg_turnAround_time;
    int response[MAX_NUMBER_OF_JOBS];
    int turnAround[MAX_NUMBER_OF_JOBS];
};

struct data sem;

void *producer_func(void* arg){
    int index = *(int* )arg;
    while(1){

        sem_wait(&sem.empty);  // empty-- (if empty < 0, then go to sleep)

        sem_wait(&sem.mutex);  // mutex-- (if mutex < 0, then go to sleep)

        // ---------- Enter Critical Section ----------

        if(sem.NUMBER_OF_PROCESS_CREATED == MAX_NUMBER_OF_JOBS){
            // All processes produced successfully
            sem_post(&sem.full);
            sem_post(&sem.mutex);
            break;
        }


        // Create One Process if not exceed MAX_NUMBER_OF_JOBS
        struct process * otemp = generateProcess();
        
        // Add to the buffer (SJF)
        struct element * str = sem.pHead;
        if(str == NULL){ // First Input
            addLast(otemp, &sem.pHead, &sem.pTail);
        }else if(str->pNext == NULL){ // Second Input
            struct process * process = (struct process *)(str->pData);
            if(otemp->iInitialBurstTime > process->iInitialBurstTime){
                addLast(otemp, &sem.pHead, &sem.pTail);
            }else{
                addFirst(otemp, &sem.pHead, &sem.pTail);
            }
        }else{ // Third+ Input
            int flag = 0;
            while( str -> pNext != NULL){
                struct element * behindStr = sem.pHead;
                struct process * process1 = (struct process *)(str -> pData);
                struct process * process2 = (struct process *)((str -> pNext) -> pData);
                if(otemp->iInitialBurstTime <= process1->iInitialBurstTime){
                    if(flag){
                        struct element * pTemp = (struct element *) malloc (sizeof(struct element));
                        // pTemp: element to be inserted
                        pTemp -> pData = otemp;
                        pTemp -> pNext = str;
                        // Repointed to the inserted element
                        // Originally, behindStr -> pNext = str;
                        behindStr -> pNext = pTemp;
                        break;
                    }else{
                        // flag = 0 -> at pHead
                        addFirst(otemp, &sem.pHead, &sem.pTail);
                        break;
                    }
                }else if(otemp->iInitialBurstTime > process2->iInitialBurstTime){
                    if(flag){
                        behindStr = behindStr->pNext;
                    }else{
                        flag = 1;
                    }
                    str=str->pNext;
                    if(str->pNext == NULL){ // Insert behind Tail
                        addLast(otemp, &sem.pHead, &sem.pTail);
                        break;
                    }
                }else{  // process1->iInitialBurstTime < otemp->iInitialBurstTime <= process2->iInitialBurstTime
                    flag = 1;
                    struct element * pNextNext = str -> pNext;
                    struct element * pTemp = (struct element *) malloc (sizeof(struct element));
                    // pTemp: element to be inserted
                    pTemp -> pData = otemp;
                    pTemp -> pNext = pNextNext;
                    // Repointed to the inserted element
                    str -> pNext = pTemp;
                    break;
                }
            }

        }

        sem.NUMBER_OF_PROCESS_CREATED ++;
        printf("Producer = %d, ", index);
        printf("Item Produced = %d, ", sem.NUMBER_OF_PROCESS_CREATED);
        printf("New Process Id = %d, ", otemp->iProcessId);
        printf("Burst Time = %d\n", otemp->iInitialBurstTime);

        // ---------- Exit Critical Section ----------

        sem_post(&sem.mutex); // mutex++
        sem_post(&sem.full);  // full++
    }
    pthread_exit(NULL); // if NUMBER_OF_PROCESS_CREATED reached MAX_NUMBER_OF_JOBS, exit the thread
}

void *consumer_func(void* arg){
    int index = *(int* )arg;
    while(1){

        sem_wait(&sem.full);  // full-- (if full < 0, then go to sleep)

        sem_wait(&sem.mutex);  // mutex-- (if mutex < 0, then go to sleep)

        // ---------- Enter Critical Section ----------

        if(sem.NUMBER_OF_PROCESS_CREATED == MAX_NUMBER_OF_JOBS){
            // In this case, producer should all exited
            sem_post(&sem.full); // Avoid sleep forever
            sem_post(&sem.mutex);
            if(sem.pTail == NULL){
                // All elements are consumed, but cosumers are not exited yet
                break;
            }
        }
        // Get the process (waiting for remove) from the head
        struct process * otemp = (struct process *)removeFirst(&sem.pHead, &sem.pTail);

        // ---------- Exit Critical Section ----------

        sem_post(&sem.mutex); // mutex++

        // Run Process!
        struct timeval oStartTime;
        struct timeval oEndTime;
        // struct timeval currentTime;
        // gettimeofday(&currentTime, NULL);
        // SJF -> NonPreemptiveJob
        runNonPreemptiveJob(otemp, &oStartTime, &oEndTime);
        sem.response[otemp -> iProcessId] = getDifferenceInMilliSeconds(otemp -> oTimeCreated, oStartTime);
        sem.turnAround[otemp -> iProcessId] = getDifferenceInMilliSeconds(otemp -> oTimeCreated, oEndTime);
        sem.Avg_response_time += sem.response[otemp -> iProcessId];
        sem.Avg_turnAround_time += sem.turnAround[otemp -> iProcessId];
        // Print Response / TurnAround Time for each process
        printf("Consumer = %d, ", index);
        printf("Process Id = %d, ", otemp -> iProcessId);
        printf("Previous Burst Time = %d, ",otemp -> iPreviousBurstTime);
        printf("New Burst Time = %d, ",otemp -> iRemainingBurstTime);
        printf("Response Time = %d, ", sem.response[otemp -> iProcessId]);
        printf("TurnAround Time: %d\n", sem.turnAround[otemp -> iProcessId]);
        free(otemp); // avoid memeory leak

        sem_post(&sem.empty);  // empty++
    }
    pthread_exit(NULL); // if NUMBER_OF_PROCESS_CREATED reached MAX_NUMBER_OF_JOBS, exit the thread
}

int main(){
    sem_init(&sem.empty, 0, MAX_BUFFER_SIZE); // Initialize the empty to MAX_BUFFER_SIZE
    sem_init(&sem.full, 0, 0); // Initialize the full to 0
    //empty and full is from 0 to MAX_BUFFER_SIZE
    sem_init(&sem.mutex, 0, 1); // Initialize the mutex to 1 => Only one thread can enter the critical section
    // Initialize data struct
    sem.NUMBER_OF_PROCESS_CREATED = 0;
    sem.pHead = NULL;
    sem.pTail = NULL;
    sem.Avg_response_time = 0;
    sem.Avg_turnAround_time = 0;

    pthread_t producerArray[NUMBER_OF_PRODUCERS];
    pthread_t consumerArray[NUMBER_OF_CONSUMERS];
    int i = 0;
    int index_producer[NUMBER_OF_PRODUCERS];
    int index_consumer[NUMBER_OF_PRODUCERS];
    for(i = 0; i < NUMBER_OF_PRODUCERS; i++){
        pthread_t producer;
        index_producer[i] = i;
        pthread_create(&producer, NULL, producer_func, &index_producer[i]);
        producerArray[i] = producer;
    }
    for(i = 0; i < NUMBER_OF_CONSUMERS; i++){
        pthread_t consumer;
        index_consumer[i] = i;
        pthread_create(&consumer, NULL, consumer_func, &index_consumer[i]);
        consumerArray[i] = consumer;
    }

    // Wait for all subthread to exit
    for(i = 0; i < NUMBER_OF_PRODUCERS; i++){
        pthread_join(producerArray[i], NULL);
    }
    for(i = 0; i < NUMBER_OF_CONSUMERS; i++){
        pthread_join(consumerArray[i], NULL);
    }
    int n = MAX_NUMBER_OF_JOBS;
    sem.Avg_response_time /= n;
    sem.Avg_turnAround_time /= n;
    printf("----------\n");
    printf("Average Response Time: %lf \n", sem.Avg_response_time);
    printf("Average TuranAround Time: %lf \n", sem.Avg_turnAround_time);
    sem_destroy(&sem.empty);
    sem_destroy(&sem.full);
    sem_destroy(&sem.mutex);
    return 0;
}
