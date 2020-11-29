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
    int NUMBER_OF_PROCESS_CREATED;
    long int Avg_response_time;
    long int Avg_turnAround_time;
    long int response[MAX_NUMBER_OF_JOBS];
    long int turnAround[MAX_NUMBER_OF_JOBS];
};

struct element * LinkedListSet[MAX_PRIORITY];

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
        if(LinkedListSet[(otemp->iPriority)-1] == NULL){
            // No process in the priority buffer
            struct element * pHead;
            struct element * pTail;
            // Initialize the first process
            addLast(otemp, &LinkedListSet[(otemp->iPriority)-1], &pTail);
            // Link pHead of the priority buffer to the right position of LinkedListSet
        }else{
            struct element * subPtr = LinkedListSet[(otemp->iPriority)-1];
            // Find the pTail
            while(subPtr -> pNext != NULL){
                subPtr = subPtr -> pNext; // Move to pNext
            }
            struct element * pTail = subPtr;
            addLast(otemp, &LinkedListSet[(otemp->iPriority)-1], &pTail);
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
    pthread_exit(0); // if NUMBER_OF_PROCESS_CREATED reached MAX_NUMBER_OF_JOBS, exit the thread
}

void *consumer_func(void* arg){
    int index = *(int* )arg;
    while(1){

        sem_wait(&sem.full);  // full-- (if full < 0, then go to sleep)
        sem_wait(&sem.mutex);  // mutex-- (if mutex < 0, then go to sleep)

        // ---------- Enter Critical Section ----------

        int i,flag = 0;
        struct element * pHead;
        for(i=0;i<MAX_PRIORITY;i++){
            if(LinkedListSet[i] != NULL){
                //Indicate that there has at least one process is waiting for consume
                flag = 1;
                pHead = LinkedListSet[i];
                break;
            }
        }//else, flag = 0

        if(sem.NUMBER_OF_PROCESS_CREATED == MAX_NUMBER_OF_JOBS && flag == 0){
            // In this case, producer should all exited, and all elements are consumed, but cosumers are not exited yet
            sem_post(&sem.mutex);
            sem_post(&sem.full); // Avoid sleep forever
            break;
        }

        if(flag == 1){
            struct element * subPtr = pHead;
            while(subPtr -> pNext != NULL){
                subPtr = subPtr -> pNext; // Move to pNext
            }
            struct element * pTail = subPtr;
            // Get the process from the head
            struct process * otemp = (struct process *)(pHead -> pData);
            // Run Process!
            struct timeval oStartTime;
            struct timeval oEndTime;
            // struct timeval currentTime;
            // gettimeofday(&currentTime, NULL);
            // PQ -> PreemptiveJob
            while(1){
                if(otemp->iInitialBurstTime == otemp->iRemainingBurstTime){
                    //First Running
                    runPreemptiveJob(otemp, &oStartTime, &oEndTime);
                    sem.response[otemp -> iProcessId] = getDifferenceInMilliSeconds(otemp -> oTimeCreated, oStartTime);
                    printf("Response Time = %d, ", sem.response[otemp -> iProcessId]);
                }else{
                    //Process has been interrupted by Time Slice before, now it's not first running
                    runPreemptiveJob(otemp, &oStartTime, &oEndTime);
                }
                printf("Consumer = %d, ", index);
                printf("Process Id = %d, ", otemp -> iProcessId);
                printf("Previous Burst Time = %d, ",otemp -> iPreviousBurstTime);
                printf("New Burst Time = %d\n",otemp -> iRemainingBurstTime);
                //Check Remaining Burst Time
                if(otemp->iRemainingBurstTime == 0){
                    removeFirst(&LinkedListSet[(otemp->iPriority)-1], &pTail); // Process Finished, Remove pHead
                    sem.turnAround[otemp -> iProcessId] = getDifferenceInMilliSeconds(otemp -> oTimeCreated, oEndTime);
                    // Print Response / TurnAround Time for each process
                    printf("TurnAround Time = %d\n", sem.turnAround[otemp -> iProcessId]);
                    sem.Avg_response_time += sem.response[otemp -> iProcessId];
                    sem.Avg_turnAround_time += sem.turnAround[otemp -> iProcessId];
                    break; // Once a process has been consumed, the consumer quit critical section
                }else{
                    // Time Slice Interrupted -> Process still not Finished
                    // Add to the Sub Linked List Again, Remove from the Head
                    addLast(otemp, &LinkedListSet[(otemp->iPriority)-1], &pTail);
                    removeFirst(&LinkedListSet[(otemp->iPriority)-1], &pTail);
                    // printf("Remain Time: %d\n\n", otemp->iRemainingBurstTime);
                }
            }
        }
        
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
    printf("Average Response Time: %d \n", sem.Avg_response_time);
    printf("Average TuranAround Time: %d \n", sem.Avg_turnAround_time);
    sem_destroy(&sem.empty);
    sem_destroy(&sem.full);
    sem_destroy(&sem.mutex);
    return 0;
}
