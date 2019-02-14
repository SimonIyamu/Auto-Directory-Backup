#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listTypes.h"

ListNode *head = NULL;

/* Pushes a file path to the wd map */
void list_push(int key, const char *data){
    ListNode *newNode = (ListNode *)malloc(sizeof(ListNode));
    newNode->key=key;
    newNode->data= (char *) malloc(sizeof(char)*(strlen(data)+1));
    strcpy(newNode->data,data);
    newNode->next = NULL;

    if(head == NULL)
        head = newNode;
    else{
        ListNode *temp = head;
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next = newNode;
    }
}

/* Removes the entry of given key */
void list_remove(int key){
    ListNode *current = head;
    ListNode *previous= NULL;

    if(head==NULL)
        return;

    while(current->key != key){
        if(current->next==NULL)
            return;
        else{
            previous = current;
            current = current->next;
        }
    }

    if(current == head)
        head = head->next;
    else
        previous->next = current->next;
    free(current->data);
    free(current);
}

/* Return the path that is mapped to the given wd */
const char *list_find(int key){
    if(head==NULL)
        return NULL;    

    ListNode *temp = head;
    while(temp!=NULL){
        if(temp->key==key)
            return temp->data;
        temp=temp->next;
    }
    /* Not found*/
    return NULL;
}

/* Print map */
void list_print(){
    ListNode *temp = head;
    while(temp!=NULL){
        printf("Wd:%d File:%s\n",temp->key,temp->data);
        temp=temp->next;
    }
}
