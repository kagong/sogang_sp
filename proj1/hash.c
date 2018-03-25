#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "hash.h"


/*
   DataStructure - Hash
   -Implemented as a linked list
   */
typedef struct _hash_node{
    int opcode;
    char name[10];
    int format;
    struct _hash_node *next;
}Hash_Node;

typedef struct _bucket{
    int index;
    Hash_Node *front;
    Hash_Node *back;
    struct  _bucket *next;
}Hash_bucket;

Hash_bucket *hash_head = NULL;

/*
   Function for
   init hash list
   */
void hash_init(){
    Hash_bucket* temp = NULL;
    int i;

    hash_head = (Hash_bucket*)malloc(sizeof(Hash_bucket));
    hash_head -> index = 1;
    hash_head -> front = NULL;
    hash_head -> back = NULL;
    hash_head -> next = NULL;

    temp = hash_head;

    for(i = 2; i<= 20 ; i++){
        temp -> next = (Hash_bucket*)malloc(sizeof(Hash_bucket));
        temp -> next ->index = i;
        temp -> next -> front = temp -> next -> back = NULL;
        temp -> next -> next = NULL;
        temp = temp -> next;
    }

}

/*
   Function for
   push new node to hash list
   */
void push_hash_node(int opcode, char name[10], int format){
    Hash_Node* new = NULL;
    Hash_bucket* temp = NULL;
    int index,i;

    new = (Hash_Node*)malloc(sizeof(Hash_Node));

    new -> opcode = opcode;
    strcpy(new -> name, name);
    new -> format = format;
    new ->next = NULL;

    index = hash_func(&name[0]);
    temp = hash_head;
    for( i = 1 ; i < index ; i++)             //find the index's bucket
      temp = temp -> next;

    if(temp -> back == NULL)                  //first node
      temp -> front = temp -> back =new;
    else{
        temp -> back -> next = new ;
        temp -> back = new;
    }
}

/*
   Function for
   calculate hash value
   */
int hash_func(char *id){
    int sum = 1 , i;
    for(i = 0 ; i < 10; i++){
        sum += id[i]%21;
        if(!id[i])
          break;

    }
    return sum%21;
}

/*
   Function for
   Deallocation resource for hash
   */
void hash_free(){
    Hash_bucket *iter = NULL, *prev = NULL;
    Hash_Node *temp = NULL;
    for(iter = hash_head; iter != NULL; iter = iter ->next){
        if(prev) free(prev);

        if(iter ->front == NULL){
            prev = iter;
            continue;
        }

        while(iter -> front -> next != NULL){
            temp = iter -> front -> next;
            iter -> front -> next = iter -> front -> next -> next;
            free(temp);
        }
        free(iter -> front);

        prev = iter;
    }
    if(prev) free(prev);

}
/*
   Function for
   name-based hash search
   */
int hash_search(char *name){
    Hash_bucket *btemp = hash_head;
    Hash_Node *ntemp = NULL;
    int index,i;

    index = hash_func(name);

    for( i = 1 ; i < index ; i++)
      btemp = btemp -> next;

    ntemp = btemp -> front;

    while(ntemp != NULL && strcmp(name, ntemp -> name) != 0  )
      ntemp = ntemp -> next;

    if(ntemp == NULL)
      return -1;

    return ntemp->opcode;
    
}
/*
   Function for
   hash ordering travel and print it
   */
void hash_traversal(){

    Hash_bucket *btemp = hash_head;
    Hash_Node *ntemp = NULL;
    int i;

    for(i = 0; i < 20; i++){

        printf("%d : ",i);
        ntemp = btemp -> front;

        while(ntemp != NULL){
            printf("[%s,%02x]",ntemp->name,ntemp->opcode);

            if(ntemp -> next != NULL){
                ntemp = ntemp -> next;
                printf(" -> ");
            }
            else
              break;
        }

        printf("\n");
        btemp = btemp -> next;
    }
    
}
