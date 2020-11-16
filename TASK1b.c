
#include <stdio.h>
#include <sys/time.h>
#include "coursework.h"
#include "linkedlist.h"

void generatePQ(struct process * otemps[]);
void implementPQlinkedList(struct process * otemps[], struct element ** pHead, struct element ** pTail);
void printLinkedList(struct element * pHead);

int main(){
    struct element * pHead = NULL;
    struct element * pTail = NULL;
    struct process * otemps[NUMBER_OF_PROCESSES];
    float Avg_response_time = 0;
    float Avg_turnAround_time = 0;
    long int response[NUMBER_OF_PROCESSES];
    long int turnAround[NUMBER_OF_PROCESSES];
    // Generate NUMBER_OF_PROCESSES and store in an array and sort by iPriority -> Implement PQ Algorithm
    generatePQ(otemps);
    // Add to Linked List
    implementPQlinkedList(otemps, &pHead, &pTail);

    //Print Linked List as Format:
    //-----sub-----
    //Process: 8
    //Priority: 2
    //BurstTime: 91
    //-----sub end------
    //-------------------------------------------------------------------------------
    // printLinkedList(pHead);
    //-------------------------------------------------------------------------------

    // Ready! Get the current Time first
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    //Run Processes!
    while(pHead != NULL){
        struct element * pSubHead = (struct element *)(pHead -> pData);
        if(pSubHead -> pNext == NULL){
            //Only one process in the Sub Linked List: Non-preemptive Implementation
            struct timeval oStartTime;
            struct timeval oEndTime;
            struct process * otemp = (struct process *)(pSubHead -> pData);
            runNonPreemptiveJob(otemp, &oStartTime, &oEndTime);
            // 0 <= otemp->iProcessId <= NUMBER_OF_PROCESSES, so we can see it as an index
            response[otemp->iProcessId] = getDifferenceInMilliSeconds(currentTime, oStartTime);
            turnAround[otemp->iProcessId] = getDifferenceInMilliSeconds(currentTime, oEndTime);
            Avg_response_time += response[otemp->iProcessId];
            Avg_turnAround_time += turnAround[otemp->iProcessId];
            // Print ResponseTime / TurnAroundTime For Each Process
            printf("Process %d: \n", otemp -> iProcessId);
            printf("Response Time: %d \n", response[otemp->iProcessId]);
            printf("TurnAround Time: %d \n", turnAround[otemp->iProcessId]);
        }else{
            //Multiple processes in the Sub Linked List: Preemptive Implementation
            struct element * subPtr = pSubHead;
            // Find the pSubTail
            while(subPtr -> pNext != NULL){
                subPtr = subPtr -> pNext; // Move to pNext
            }
            struct element * pSubTail = subPtr;
            while(pSubTail != NULL){
                struct timeval oStartTime;
                struct timeval oEndTime;
                struct process * otemp = (struct process *)(pSubHead -> pData);
                if(otemp->iInitialBurstTime == otemp->iRemainingBurstTime){
                    //First Running
                    runPreemptiveJob(otemp, &oStartTime, &oEndTime);
                    response[otemp->iProcessId] = getDifferenceInMilliSeconds(currentTime, oStartTime);
                }else{
                    //Process has been interrupted by Time Slice before, now it's not first running
                    runPreemptiveJob(otemp, &oStartTime, &oEndTime);
                }
                //Check Remaining Burst Time
                if(otemp->iRemainingBurstTime == 0){
                    // Process Finished
                    turnAround[otemp->iProcessId] = getDifferenceInMilliSeconds(currentTime, oEndTime);
                    removeFirst(&pSubHead, &pSubTail); //Process Finished, delete from the head
                    // Print ResponseTime / TurnAroundTime For Each Process
                    printf("Process %d: \n", otemp -> iProcessId);
                    printf("Response Time: %d \n", response[otemp->iProcessId]);
                    printf("TurnAround Time: %d \n", turnAround[otemp->iProcessId]);
                    Avg_response_time += response[otemp->iProcessId];
                    Avg_turnAround_time += turnAround[otemp->iProcessId];
                }else{
                    // Time Slice Interrupted -> Process still not Finished
                    // Add to the Sub Linked List Again, Remove from the Head
                    addLast(otemp, &pSubHead, &pSubTail);
                    removeFirst(&pSubHead, &pSubTail);
                }
            }

        }
        removeFirst(&pHead, &pTail); //Process Finished, delete from the head
    }
    //Print Average ResponseTime / TurnAroundTime
    Avg_response_time = Avg_response_time / NUMBER_OF_PROCESSES;
    Avg_turnAround_time = Avg_turnAround_time / NUMBER_OF_PROCESSES;
    printf("----------\n");
    printf("Average Response Time: %.2f\n", Avg_response_time);
    printf("Average turnAround Time: %.2f\n", Avg_turnAround_time);
    return 0;
}

void generatePQ(struct process * otemps[]){
    int count = 0;
    for(count = 0; count < NUMBER_OF_PROCESSES; count++){
        otemps[count] = generateProcess();
    }
    int i,j,k = 0;
    struct process * temp1 = NULL;
    struct process * temp2 = NULL;
    // Bubble Sort (from small to big)
    for(i = 0; i < NUMBER_OF_PROCESSES-1; i++){
        for(j = 0; j < NUMBER_OF_PROCESSES-1; j++){
            temp1 = otemps[j];
            temp2 = otemps[j+1];
            int priority1 = temp1->iPriority;
            int priority2 = temp2->iPriority;
            if(priority1 > priority2){
                // Swap two processes
                otemps[j] = temp2;
                otemps[j+1] = temp1;
            }
        }
    }
    // Priority: 2 6 6 11 12 13 19 24 27 31

    //Bubble Sort: Same Priority -> Apply SJF for each process
    // for(k = 0; k < NUMBER_OF_PROCESSES-1; k++){
    //     temp1 = otemps[k];
    //     temp2 = otemps[k+1];
    //     int priority1 = temp1->iPriority;
    //     int priority2 = temp2->iPriority;
    //     int Time1 = temp1->iInitialBurstTime;
    //     int Time2 = temp2->iInitialBurstTime;
    //     if(priority1 == priority2){
    //         if(Time1>Time2){
    //             // Swap two processes
    //             otemps[k] = temp2;
    //             otemps[k+1] = temp1;
    //         }
    //     }
    // }
}

void implementPQlinkedList(struct process * otemps[], struct element ** pHead, struct element ** pTail){
    int count = 0;
    struct element * pSubHead = NULL;
    struct element * pSubTail = NULL;
    //Create the first Sub Linked List
    addLast(otemps[count], &pSubHead, &pSubTail);
    for(count = 1; count < NUMBER_OF_PROCESSES; count++){
        if(otemps[count] -> iPriority == otemps[count-1] -> iPriority){
            // Create Multiple(Sub) Linked List <-> Same Priority
            addLast(otemps[count], &pSubHead, &pSubTail);
        }else{
            //Add pSubHead(the pHead of the sub Linked List) as the pData of the main Linked List
            addLast(pSubHead, pHead, pTail);
            // Clear Pointer
            pSubHead = NULL;
            pSubTail = NULL;
            // Create A New Sub Linked List!
            addLast(otemps[count], &pSubHead, &pSubTail);
        }
    }
    //Add the last SubList Header as the pData of the main Linked List
    addLast(pSubHead, pHead, pTail);
}

void printLinkedList(struct element * pHead){
    struct element * current = pHead;
    //Ergodic Main LinkedList
    while(current != NULL){
        struct element * pSubHead = (struct element *)(current -> pData);
        //Ergodic Each Sub LinkedList
        printf("------sub-----\n");
        while(pSubHead != NULL){
            struct process * otemp = (struct process *)(pSubHead -> pData);
            printf("Process: %d\n", otemp->iProcessId);
            printf("Priority: %d\n", otemp->iPriority);
            printf("BurstTime: %d\n\n", otemp->iInitialBurstTime);
            pSubHead = pSubHead->pNext;
        }
        printf("------sub end-----\n");
        current = current->pNext;
    }
    printf("--------------------\n");
}
