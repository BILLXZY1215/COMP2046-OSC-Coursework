
#include <stdio.h>
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
    long int Avg_response_time;
    long int Avg_turnAround_time;
    long int response[MAX_NUMBER_OF_JOBS];
    long int turnAround[MAX_NUMBER_OF_JOBS];
};

struct data sem;

void *producer_func(void* arg){
    int index = *(int* )arg;
    while(1){

        sem_wait(&sem.empty);  // empty-- (if empty < 0, then go to sleep)
        sem_wait(&sem.mutex);  // mutex-- (if mutex < 0, then go to sleep)

        // ---------- Enter Critical Section ----------

        if(sem.NUMBER_OF_PROCESS_CREATED == MAX_NUMBER_OF_JOBS){
            sem_post(&sem.mutex);
            sem_post(&sem.full);
            break;
        }
        // Create One Process if not exceed MAX_NUMBER_OF_JOBS
        struct process * otemp = generateProcess();
        // Add to the buffer
        addLast(otemp, &sem.pHead, &sem.pTail);
        sem.NUMBER_OF_PROCESS_CREATED ++;
        printf("Producer = %d, ", index);
        printf("Item Produced = %d, ", sem.NUMBER_OF_PROCESS_CREATED);
        printf("New Process Id = %d, ", otemp->iProcessId);
        printf("Burst Time = %d\n", otemp->iInitialBurstTime);


        // ---------- Exit Critical Section ----------

        sem_post(&sem.mutex); // mutex++
        sem_post(&sem.full);  // full++
    }
    pthread_exit(0); // if NUMBER_OF_PROCESS_CREATED reached MAX_NUMBER_OF_JOBS, exit the thread
}

void *consumer_func(void* arg){
    int index = *(int* )arg;
    while(1){

        sem_wait(&sem.full);  // full-- (if full < 0, then go to sleep)
        sem_wait(&sem.mutex);  // mutex-- (if mutex < 0, then go to sleep)

        if(sem.NUMBER_OF_PROCESS_CREATED == MAX_NUMBER_OF_JOBS && sem.pTail == NULL){
            // In this case, producer should all exited, and all elements are consumed, but cosumers are not exited yet
            sem_post(&sem.mutex);
            sem_post(&sem.full); // Avoid sleep forever
            break;
        }

        // ---------- Enter Critical Section ----------

        // Get the process from the head
        struct process * otemp = (struct process *)(sem.pHead -> pData);
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
        // Remove From the buffer
        removeFirst(&sem.pHead, &sem.pTail);

        // ---------- Exit Critical Section ----------

        sem_post(&sem.mutex); // mutex++
        sem_post(&sem.empty);  // empty++
    }
    pthread_exit(0); // if NUMBER_OF_PROCESS_CREATED reached MAX_NUMBER_OF_JOBS, exit the thread
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
    for(i = 0; i < NUMBER_OF_PRODUCERS; i++){
        pthread_t producer;
        int index = i;
        pthread_create(&producer, NULL, producer_func, &index);
        producerArray[i] = producer;
    }
    for(i = 0; i < NUMBER_OF_CONSUMERS; i++){
        pthread_t consumer;
        int index = i;
        pthread_create(&consumer, NULL, consumer_func, &index);
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
    printf("Average Response Time: %d \n", sem.Avg_response_time);
    printf("Average TuranAround Time: %d \n", sem.Avg_turnAround_time);
    sem_destroy(&sem.empty);
    sem_destroy(&sem.full);
    sem_destroy(&sem.mutex);
    return 0;
}
