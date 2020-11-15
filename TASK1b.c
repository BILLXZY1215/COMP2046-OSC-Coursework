#include <stdio.h>
#include <sys/time.h>
#include "coursework.h"
#include "linkedlist.h"

void generatePQ(struct process * otemps[]);
void implementPQlinkedList(struct process * otemps[], struct element ** pHead, struct element ** pTail);

int main(){
    int count = 0;
    struct element * pHead = NULL;
    struct element * pTail = NULL;
    struct process * otemps[NUMBER_OF_PROCESSES];
    // Generate NUMBER_OF_PRODUCERS_PROCESSES and store in an array and sort by iPriority -> Implement PQ Algorithm
    generatePQ(otemps);
    // Add to Linked List 
    implementPQlinkedList(otemps, &pHead, &pTail);

    //Print Linked List 

    struct element * current = pHead;
    while(1){
        struct element * pSubHead = (struct element *)(current -> pData);
        while(1){
            struct process * otemp = (struct process *)(pSubHead -> pData);
            printf("Process: %d\n", otemp->iProcessId);
            printf("Priority: %d\n\n", otemp->iPriority);
            printf("BurstTime: %d\n\n", otemp->iInitialBurstTime);
            pSubHead = pSubHead->pNext;
            if(pSubHead == NULL){
                break;
            }
        }
        current = current->pNext;
        if(current == NULL){
            break;
        }
    }



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
    for(k = 0; k < NUMBER_OF_PROCESSES-1; k++){
        temp1 = otemps[k];
        temp2 = otemps[k+1];
        int priority1 = temp1->iPriority;
        int priority2 = temp2->iPriority;
        int Time1 = temp1->iInitialBurstTime;
        int Time2 = temp2->iInitialBurstTime;
        if(priority1 == priority2){
            if(Time1>Time2){
                // Swap two processes
                otemps[k] = temp2;
                otemps[k+1] = temp1;
            }
        }
    }
}

void implementPQlinkedList(struct process * otemps[], struct element ** pHead, struct element ** pTail){
    int count = 0;
    struct element * pSubHead = NULL;
    struct element * pSubTail = NULL;
    //Create the first Sub Linked List
    addLast(otemps[count], &pSubHead, &pSubTail);
    for(count = 1; count < NUMBER_OF_PROCESSES; count++){
        if(otemps[count] -> iPriority == otemps[count+1] -> iPriority){
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