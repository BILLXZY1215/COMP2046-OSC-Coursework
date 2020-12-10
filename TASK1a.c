#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "coursework.h"
#include "linkedlist.h"


void generateSJF(struct process * otemps[]);
void implementSJFlinkedList(struct process * otemps[], struct element ** pHead, struct element ** pTail);
void printLinkedList(struct element * pHead);

int main(){
    struct element * pHead = NULL;
    struct element * pTail = NULL;
    struct process * otemps[NUMBER_OF_PROCESSES];
    long int Avg_response_time = 0;
    long int Avg_turnAround_time = 0;
    long int response[NUMBER_OF_PROCESSES];
    long int turnAround[NUMBER_OF_PROCESSES];

    // Generate NUMBER_OF_PROCESSES and store in an array and sort by iInitialBurstTime -> Implement SJF Algorithm
    generateSJF(otemps);
    // Add to Linked List
    implementSJFlinkedList(otemps, &pHead, &pTail);

    //Print Linked List as Format:
    // Process 9:
    // Burst Time: 23
    //-----------------------------------------------------
    // printLinkedList(pHead);
    //-----------------------------------------------------
    // // Ready! Get the current Time first
    // struct timeval currentTime;
    // gettimeofday(&currentTime, NULL);
    // Run Processes!
    while(pHead != NULL){
        struct timeval oStartTime;
        struct timeval oEndTime;
        //From LinkedList: Get the struct of process (at pHead!)
        struct process * otemp = (struct process *)(pHead -> pData);
        // SJF is based on non-preemptive job!
        runNonPreemptiveJob(otemp, &oStartTime, &oEndTime);
        // (0 <= otemp->iProcessId <= NUMBER_OF_PROCESSES), so we can see it as an index
        response[otemp->iProcessId] = getDifferenceInMilliSeconds(otemp->oTimeCreated, oStartTime);
        turnAround[otemp->iProcessId] = getDifferenceInMilliSeconds(otemp->oTimeCreated, oEndTime);
        Avg_response_time += response[otemp->iProcessId];
        Avg_turnAround_time += turnAround[otemp->iProcessId];
        //Print ResponseTime / TurnAroundTime For Each Process
        printf("Process Id = %d, ", otemp -> iProcessId);
        printf("Priority = %d, ", otemp -> iPriority);
        printf("Priority = %d, ", otemp -> iPriority);
        printf("Previous Burst Time = %d, ", otemp -> iPreviousBurstTime);
        printf("Remaining Burst Time = %d, ", otemp -> iRemainingBurstTime);
        printf("Response Time: %d, ", response[otemp->iProcessId]);
        printf("TurnAround Time: %d\n", turnAround[otemp->iProcessId]);
        // Process Finished, Remove it from the Head!
        free(removeFirst(&pHead, &pTail));
    }
    //Print Average ResponseTime / TurnAroundTime
    Avg_response_time = Avg_response_time / NUMBER_OF_PROCESSES;
    Avg_turnAround_time = Avg_turnAround_time / NUMBER_OF_PROCESSES;
    printf("Average Response Time: %d\n", Avg_response_time);
    printf("Average turnAround Time: %d\n", Avg_turnAround_time);
    return 0;
}

void generateSJF(struct process * otemps[]){
    int count = 0;
    for(count = 0; count < NUMBER_OF_PROCESSES; count++){
        otemps[count] = generateProcess();
    }
    int i,j = 0;
    struct process * temp1 = NULL;
    struct process * temp2 = NULL;
    // Bubble Sort (from small to big)
    for(i = 0; i < NUMBER_OF_PROCESSES-1; i++){
        for(j = 0; j < NUMBER_OF_PROCESSES-1; j++){
            temp1 = otemps[j];
            temp2 = otemps[j+1];
            int time1 = temp1->iInitialBurstTime;
            int time2 = temp2->iInitialBurstTime;
            if(time1 > time2){
                // Swap two processes
                otemps[j] = temp2;
                otemps[j+1] = temp1;
            }
        }
    }
    //Burst Time: 23 28 63 91 100 114 134 137 141 144
}


void implementSJFlinkedList(struct process * otemps[], struct element ** pHead, struct element ** pTail){
    int count = 0;
    for(count = 0; count < NUMBER_OF_PROCESSES; count++){
        addLast(otemps[count], pHead, pTail);
    }
}

void printLinkedList(struct element * pHead){
    while(pHead != NULL){
        struct process * otemp = (struct process *)(pHead -> pData);
        printf("Process %d: \n", otemp->iProcessId);
        printf("Burst Time: %d\n", otemp->iInitialBurstTime);
        pHead = pHead -> pNext;
    }
    printf("----------\n");
}
