#include <stdio.h>
#include <sys/time.h>
#include "coursework.h"
#include "linkedlist.h"


void generateSJF(struct process * otemps[]);
void implementSJFlinkedList(struct process * otemps[], struct element ** pHead, struct element ** pTail);

int main(){
    int count = 0;
    struct element * pHead = NULL;
    struct element * pTail = NULL;
    struct process * otemps[NUMBER_OF_PROCESSES];

    // Generate NUMBER_OF_PRODUCERS_PROCESSES and store in an array and sort by iInitialBurstTime -> Implement SJF Algorithm
    generateSJF(otemps);
    // Add to Linked List
    implementSJFlinkedList(otemps, &pHead, &pTail);
    // Ready! Get the current Time first
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    // Run Processes!
    for(count = 0; count< NUMBER_OF_PROCESSES; count++){
        struct timeval oStartTime;
        struct timeval oEndTime;
        //From LinkedList: Get the struct of process
        struct process * otemp = (struct process *)(pHead -> pData);
        // SJF is based on non-preemptive job!
        runNonPreemptiveJob(otemp, &oStartTime, &oEndTime);
        printf("Process %d: \n", otemp -> iProcessId);
        printf("Response Time: %d \n", getDifferenceInMilliSeconds(currentTime, oStartTime));
        printf("TurnAround Time: %d \n", getDifferenceInMilliSeconds(currentTime, oEndTime));
        // Process Finished, Remove it from the Head!
        removeFirst(&pHead, &pTail);
    }

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




