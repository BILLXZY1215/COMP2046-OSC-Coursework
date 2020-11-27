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
    long int Avg_response_time = 0;
    long int Avg_turnAround_time = 0;
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

    // //Bubble Sort: Same Priority -> Apply SJF for each process
    // struct element * str = pHead;
    // while(str != NULL){
    //     struct element * substr = str->pData;
    //     while(substr->pNext!=NULL){
    //         struct process * temp1 = (struct process *)(substr->pData);
    //         struct process * temp2 = (struct process *)(substr->pNext->pData);
    //         int priority1 = temp1->iPriority;
    //         int priority2 = temp2->iPriority;
    //         int Time1 = temp1->iInitialBurstTime;
    //         int Time2 = temp2->iInitialBurstTime;
    //         if(priority1 == priority2){
    //             if(Time1>Time2){
    //                 // Swap two processes
    //                 substr->pData = temp2;
    //                 substr->pNext->pData = temp1;
    //             }
    //         }
    //         substr = substr->pNext;
    //     }
    //     str = str->pNext;
    // }

    //-----sub end------
    //-------------------------------------------------------------------------------
    printLinkedList(pHead);
    //-------------------------------------------------------------------------------

    //Run Processes!
    while(pHead != NULL){
        struct element * pSubHead = (struct element *)(pHead -> pData);
        // Preemptive Implementation
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
            printf("Process Id = %d, ", otemp -> iProcessId);
            printf("Priority = %d, ", otemp -> iPriority);
            printf("Previous Burst Time = %d, ", otemp -> iPreviousBurstTime);
            printf("Remaining Burst Time = %d\n", otemp -> iRemainingBurstTime);
            if(otemp->iInitialBurstTime == otemp->iRemainingBurstTime){
                //First Running
                runPreemptiveJob(otemp, &oStartTime, &oEndTime);
                response[otemp->iProcessId] = getDifferenceInMilliSeconds(otemp->oTimeCreated, oStartTime);
                printf("Response Time: %d \n", response[otemp->iProcessId]);
            }else{
                //Process has been interrupted by Time Slice before, now it's not first running
                runPreemptiveJob(otemp, &oStartTime, &oEndTime);
            }
            //Check Remaining Burst Time
            if(otemp->iRemainingBurstTime == 0){
                // Process Finished
                turnAround[otemp->iProcessId] = getDifferenceInMilliSeconds(otemp->oTimeCreated, oEndTime);
                removeFirst(&pSubHead, &pSubTail); //Process Finished, delete from the head
                // Print ResponseTime / TurnAroundTime For Each Process
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
        removeFirst(&pHead, &pTail); //Process Finished, delete from the head
    }
    //Print Average ResponseTime / TurnAroundTime
    Avg_response_time = Avg_response_time / NUMBER_OF_PROCESSES;
    Avg_turnAround_time = Avg_turnAround_time / NUMBER_OF_PROCESSES;
    printf("Average Response Time: %d\n", Avg_response_time);
    printf("Average turnAround Time: %d\n", Avg_turnAround_time);
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
        struct process * init = (struct process *)(pSubHead -> pData);
        printf("Priority = %d\n\t", init->iPriority);
        while(pSubHead != NULL){
            struct process * otemp = (struct process *)(pSubHead -> pData);
            printf("Process Id: %d, ", otemp->iProcessId);
            printf("Priority = %d, ", otemp->iPriority);
            printf("Initial Burst Time = %d, ", otemp->iInitialBurstTime);
            if(pSubHead->pNext == NULL){
                printf("Remaining BurstTime: %d\n", otemp->iRemainingBurstTime);
            }else{
                printf("Remaining BurstTime: %d\n\t", otemp->iRemainingBurstTime);
            }
            pSubHead = pSubHead->pNext;
        }
        current = current->pNext;
    }
}
