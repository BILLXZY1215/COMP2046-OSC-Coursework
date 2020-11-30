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
        // Create One Process if not exceed MAX_NUMBER_OF_JOBS
        struct process * otemp = generateProcess();
        sem_wait(&sem.mutex);  // mutex-- (if mutex < 0, then go to sleep)

        // ---------- Enter Critical Section ----------

        if(sem.NUMBER_OF_PROCESS_CREATED == MAX_NUMBER_OF_JOBS){
            sem_post(&sem.mutex);
            sem_post(&sem.full);
            break;
        }

        // Add process to LinkedList （sort by PQ）
        struct element * ele = (struct element *) malloc (sizeof(struct element));
        ele->pData = otemp;

        if(sem.pHead == NULL){ // First Implement
            addLast(ele, &sem.pHead, &sem.pTail);
        }else if(sem.pHead -> pNext == NULL){ //Second Implement
            struct element * pSubHead = (struct element *)(sem.pHead->pData);
            struct process * process = (struct process *)(pSubHead->pData);
            if(process->iPriority > otemp->iPriority){
                //otemp has higher priority
                addFirst(ele, &sem.pHead, &sem.pTail);
            }else if(process->iPriority == otemp->iPriority){
                //otemp has the same priority
                addFirst(otemp, &pSubHead, &pSubHead);
            }else{
                //otemp has lower priority
                addLast(ele, &sem.pHead, &sem.pTail);
            }
        }else{
            int flag = 0;
            // Third+ Implement

            struct element * behindStr = sem.pHead;
            struct element * str = sem.pHead;
            while(str -> pNext != NULL){
                struct element * pSubHead1 = str->pData;
                struct element * pSubHead2 = str->pNext->pData;
                struct process * process1 = pSubHead1->pData;
                struct process * process2 = pSubHead2->pData;
                if(otemp->iPriority < process1->iPriority){
                    if(flag){
                        struct element * pTemp = (struct element *) malloc (sizeof(struct element));
                        // pTemp: element to be inserted
                        pTemp -> pData = ele;
                        pTemp -> pNext = str;
                        // Repointed to the inserted element
                        // Originally, behindStr -> pNext = str;
                        behindStr -> pNext = pTemp;
                        break;
                    }else{
                        // flag = 0 -> at pHead
                        addFirst(ele, &sem.pHead, &sem.pTail);
                        break;
                    }

                }else if(otemp->iPriority == process1->iPriority){
                    struct element * subPtr = pSubHead1;
                    // Find the pSubTail
                    while(subPtr -> pNext != NULL){
                        subPtr = subPtr -> pNext; // Move to pNext
                    }
                    struct element * pSubTail1 = subPtr;
                    addFirst(otemp, &pSubHead1, &pSubTail1);
                    break;

                }else if(otemp->iPriority < process2->iPriority){
                    // process1->iPriority < otemp->iPriority < process2->iPriority
                    flag = 1;
                    struct element * pNextNext = str -> pNext;
                    struct element * pTemp = (struct element *) malloc (sizeof(struct element));
                    pTemp->pData = ele;
                    pTemp->pNext = pNextNext;
                    str->pNext = pTemp;
                    break;

                }else if(otemp->iPriority == process2->iPriority){
                    struct element * subPtr = pSubHead1;
                    // Find the pSubTail
                    while(subPtr -> pNext != NULL){
                        subPtr = subPtr -> pNext; // Move to pNext
                    }
                    struct element * pSubTail2 = subPtr;
                    addFirst(otemp, &pSubHead2, &pSubTail2);
                    break;

                }else{
                    // process2->iPriority < otemp->iPriority
                    if(flag){
                        behindStr = behindStr->pNext;
                    }else{
                        flag = 1;
                    }
                    str=str->pNext;
                    if(str->pNext == NULL){ // Insert behind Tail
                        addLast(ele, &sem.pHead, &sem.pTail);
                    }
                }
            }
        }

        sem.NUMBER_OF_PROCESS_CREATED ++;
        printf("Producer = %d, ", index);
        printf("Item Produced = %d, ", sem.NUMBER_OF_PROCESS_CREATED);
        printf("New Process Id = %d, ", otemp->iProcessId);
        printf("Burst Time = %d, ", otemp->iInitialBurstTime);
        printf("Priority = %d\n", otemp->iPriority);
        
        // ---------- Exit Critical Section ----------

        sem_post(&sem.mutex); // mutex++
        
        int time = otemp->iInitialBurstTime;
        while(time>0){
            sem_post(&sem.full); // post enough semaphore for consumer
            time = time - TIME_SLICE;
        }

    }
    pthread_exit(0); // if NUMBER_OF_PROCESS_CREATED reached MAX_NUMBER_OF_JOBS, exit the thread
}

void *consumer_func(void* arg){
    int index = *(int* )arg;

    while(1){
        sem_wait(&sem.full);  // full-- (if full < 0, then go to sleep)

        // ---------- Enter Critical Section ----------

        sem_wait(&sem.mutex);
        if(sem.NUMBER_OF_PROCESS_CREATED == MAX_NUMBER_OF_JOBS && sem.pHead == NULL){
            sem_post(&sem.full);
            sem_post(&sem.mutex);
            break;
        }
        struct element * pSubHead;
        struct process * otemp;
        if(sem.pHead==NULL){
            sem_post(&sem.full);
            sem_post(&sem.mutex);
            continue;
        }
        // The head of sub linked list
        pSubHead = (struct element *)removeFirst(&sem.pHead, &sem.pTail);
        struct element * subPtr = pSubHead;
        // Find the pSubTail
        while(subPtr -> pNext != NULL){
            subPtr = subPtr -> pNext; // Move to pNext
        }
        struct element * pSubTail = subPtr;
        if(pSubHead==NULL){
            sem_post(&sem.full);
            sem_post(&sem.mutex);
            continue;
        }
        otemp = (struct process *)removeFirst(&pSubHead, &pSubTail);
        if(otemp->iRemainingBurstTime > TIME_SLICE){
            // process hasn't finished, cannot be removed directly, needs to addLast
            addLast(otemp, &pSubHead, &pSubTail);
        }else{
            // Process finished running, activate producer
            sem_post(&sem.empty);
        }
        sem_post(&sem.mutex);

        // ---------- Exit Critical Section ----------

        struct timeval oStartTime;
        struct timeval oEndTime;

        int first = 0;
        if(otemp->iInitialBurstTime == otemp->iRemainingBurstTime){
            first = 1;
            //First Running
            runPreemptiveJob(otemp, &oStartTime, &oEndTime);
            sem.response[otemp->iProcessId] = getDifferenceInMilliSeconds(otemp->oTimeCreated, oStartTime);
        }else{
            //Process has been interrupted by Time Slice before, now it's not first running
            runPreemptiveJob(otemp, &oStartTime, &oEndTime);
        }

        //Check Remaining Burst Time
        if(otemp->iRemainingBurstTime == 0){
            first = 2;
            // Process Finished
            sem.turnAround[otemp->iProcessId] = getDifferenceInMilliSeconds(otemp->oTimeCreated, oEndTime);
            // Print ResponseTime / TurnAroundTime For Each Process
            sem.Avg_response_time += sem.response[otemp->iProcessId];
            sem.Avg_turnAround_time += sem.turnAround[otemp->iProcessId];
        }

        sem_wait(&sem.mutex);

        // ---------- Enter Critical Section ----------
        printf("Consumer = %d, ", index);
        if(first==1){
            printf("Response Time = %d, ", sem.response[otemp->iProcessId]);
        }
        printf("Process Id = %d, ", otemp -> iProcessId);
        printf("Priority = %d, ", otemp -> iPriority);
        printf("Previous Burst Time = %d, ", otemp -> iPreviousBurstTime);
        if(first==2){
            printf("Remaining Burst Time = %d, ", otemp -> iRemainingBurstTime);
            printf("TurnAround Time: %d \n", sem.turnAround[otemp->iProcessId]);
        }else{
            printf("Remaining Burst Time = %d \n", otemp -> iRemainingBurstTime);
        }

        if(pSubHead!=NULL){
            // Element removed back
            struct element * ele = pSubHead;
            // The whole priority queue has not been totally consumed, write back to the main queue
            struct process * process = (struct process *)ele->pData;
            int priority = process->iPriority;
            if(sem.pHead == NULL){
                addLast(ele, &sem.pHead, &sem.pTail);
            }else if(sem.pHead -> pNext == NULL){
                struct element * pSub = (struct element *)(sem.pHead->pData);
                struct process * pro = (struct process *)(pSub->pData);
                if(pro->iPriority > priority){
                    addFirst(ele, &sem.pHead, &sem.pTail);
                }else{
                    addLast(ele, &sem.pHead, &sem.pTail);
                }
            }else{
                int flag = 0;
                struct element * str = sem.pHead;
                struct element * behindStr = sem.pHead;
                while(str -> pNext != NULL){
                    struct element * pSubHead1 = str->pData;
                    struct element * pSubHead2 = str->pNext->pData;
                    struct process * process1 = pSubHead1->pData;
                    struct process * process2 = pSubHead2->pData;
                    if(priority < process1->iPriority){
                        if(flag){
                            struct element * pTemp = (struct element *) malloc (sizeof(struct element));
                            // pTemp: element to be inserted
                            pTemp -> pData = ele;
                            pTemp -> pNext = str;
                            // Repointed to the inserted element
                            // Originally, behindStr -> pNext = str;
                            behindStr -> pNext = pTemp;
                            break;
                        }else{
                            // flag = 0 -> at pHead
                            addFirst(ele, &sem.pHead, &sem.pTail);
                            break;
                        }
                    }else if(priority > process2->iPriority){
                        if(flag){
                            behindStr = behindStr->pNext;
                        }else{
                            flag = 1;
                        }
                        str=str->pNext;
                        if(str->pNext == NULL){ // Insert behind Tail
                            addLast(ele, &sem.pHead, &sem.pTail);
                        }

                    }else{
                        //process1->iPriority < priority < process2->iPriority
                        flag = 1;
                        struct element * pNextNext = str -> pNext;
                        struct element * pTemp = (struct element *) malloc (sizeof(struct element));
                        pTemp->pData = ele;
                        pTemp->pNext = pNextNext;
                        str->pNext = pTemp;
                        break;
                    }
                }
            }
        }

        // ---------- Exit Critical Section ----------

        sem_post(&sem.mutex);


    }
    pthread_exit(0); // if NUMBER_OF_PROCESS_CREATED reached MAX_NUMBER_OF_JOBS, exit the thread
}

int main(){
    int i = 0;
    sem_init(&sem.empty, 0, MAX_BUFFER_SIZE); // Initialize the empty to MAX_BUFFER_SIZE
    sem_init(&sem.full, 0, 0); // Initialize the full to 0
    //empty and full is from 0 to MAX_BUFFER_SIZE
    sem_init(&sem.mutex, 0, 1); // Initialize the mutex to 1 => Only one thread can enter the critical section

    // Initialize data struct
    sem.NUMBER_OF_PROCESS_CREATED = 0;
    sem.Avg_response_time = 0;
    sem.Avg_turnAround_time = 0;

    sem.pHead = NULL;
    sem.pTail = NULL;

    pthread_t producerArray[NUMBER_OF_PRODUCERS];
    pthread_t consumerArray[NUMBER_OF_CONSUMERS];

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
    sem_destroy(&sem.mutex);
    sem_destroy(&sem.full);
    sem_destroy(&sem.empty);
    return 0;
}
