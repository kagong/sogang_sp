#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/stat.h>
#include"hash.h"
#include"functions.h"
#include"20151575.h"
#define MEM_MAX 65536*16
#define ARG_COUNT_CHECK(num1, num2) do{\
    if((num1) != (num2))\
    return 0;\
}while(0)

#define MEM_IDX_CHECK(start, end) do{\
    if((end) < (start))\
    return 0;\
    if((start) < 0 || (start)>= 65536 * 16)\
    return 0;\
    if((end) < 0 || (end)>= 65536 * 16)\
    return 0;\
}while(0)

typedef struct _node{
    char *cmd;
    struct _node *next;
}Hi_Node;

typedef struct  _Hnode{
    Hi_Node *front;
    Hi_Node *back;
}Hi_Head;

Hi_Head *hi_head = NULL;

unsigned char* mem_head = NULL;

void functions_init(){
    hash_init();
    opcode_input();

    mem_head = (unsigned char*)malloc(sizeof(unsigned char) * (MEM_MAX));

    hi_head = (Hi_Head*)malloc(sizeof(Hi_Head));
    hi_head -> front = hi_head -> back = NULL; 
    
}

void functions_free(){
    history_free();
    hash_free();
    free(mem_head);
}

void history_push(char* command){	

    Hi_Node *new = NULL;
    new = (Hi_Node*)malloc(sizeof(Hi_Node));
    new -> cmd = (char*)malloc(sizeof(char)*MAX_CMD);
    new -> next = NULL;
    strncpy(new -> cmd, command, MAX_CMD);

    if(hi_head -> front == NULL){

        hi_head -> front = new;	
        hi_head -> back = new;	
    }
    else{
        hi_head -> back -> next = new;
        hi_head -> back = new;
    }

}
void history_pop(){
    Hi_Node *temp = NULL;
    temp = hi_head -> front;

    if(temp == NULL){ 
      printf("history error!\n");
      return ;
    }

    if(hi_head -> front == hi_head -> back){// only one node
        free(hi_head -> back -> cmd);
        free(hi_head ->back);
        hi_head -> front = hi_head -> back = NULL;
    }
    else{
        while(temp -> next != hi_head -> back)
          temp = temp -> next;

        free(temp -> next -> cmd);
        free(temp -> next);
        hi_head -> back = temp;
    }
}
void history_free(){
    Hi_Node *temp = NULL, *temp2 = NULL;
    temp = hi_head -> front;
    while(temp != NULL){
        temp2 = temp -> next;
        free(temp -> cmd);
        free(temp);
        temp = temp2;
    }

    free(hi_head);
}

int help(int argc, char** argv){
    ARG_COUNT_CHECK(argc,0);

    printf("h[elp]\n");	
    printf("d[ir]\n");	
    printf("q[uit]\n");	
    printf("hi[story]\n");	
    printf("du[mp] [start, end]\n");	
    printf("e[dit] address, value\n");	
    printf("f[ill] start, end, value\n");	
    printf("reset\n");	
    printf("opcode mnemonic\n");	
    printf("opcodelist\n");	
    return 1;
}

int dir(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
    struct dirent *entry;
    struct stat info;
    DIR *dp;
    dp = opendir(".");
    if(dp == NULL) return 0;
    while((entry = readdir(dp))){
        if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
          continue;
        printf("%s",entry->d_name);
        stat(entry->d_name,&info);
        switch(info.st_mode & S_IFMT){
          case S_IFDIR: printf("/\t");break;

          case S_IFREG:
                        if((info.st_mode & S_IRWXU & S_IXUSR) != 0)
                          printf("*\t");
                        else
                          printf("\t");

                        break;
          default: printf("\t"); break;

        }
    }
    printf("\n");
    closedir(dp);


    return 1;

}

int quit(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);

    return 1;
}
int history(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
    int i = 0;
    Hi_Node * temp = NULL;
    temp = hi_head -> front;
    if(hi_head -> front == hi_head -> back)
      return 1;
    while(temp != NULL){

        printf("%d\t%s\n",i,temp -> cmd);

        temp = temp -> next;
        ++i;
    }
    return 1;
}
int dump(int argc, char** argv){
    static int prev_end=0;
    char* ptr1 = NULL, *ptr2 = NULL;
    int start=0, end=0;
    int i,j;
    if(argc == 0){
        start = prev_end;
        end = start + 160-1;
    }
    else{
        start = strtol(argv[0],&ptr1,16);
        MEM_IDX_CHECK(0, start);
        if(argc == 1)
          end =start + 160-1;
        else {
          end = strtol(argv[1],&ptr2,16);
          MEM_IDX_CHECK(0, end);
        }
    }
    if(ptr1!=NULL && *ptr1 != 0) return 0;
    if(ptr2!=NULL && *ptr2 != 0) return 0;

    if(end >= MEM_MAX)
      end = MEM_MAX - 1;

    prev_end = end + 1;
    if(prev_end >= MEM_MAX)
      prev_end = 0;

    for(i = start / 16 ; i <= end / 16 ; i++){
        printf("%06x ",i*16);
        for( j = 0 ; j <16 ; j++){
            if( start <= i*16 + j && i*16 + j <= end)
              printf("%02x ",mem_head[i*16 + j]);
            else
              printf("   ");
        }
        printf("; ");
        for(j = 0 ; j <16 ; j++){
            if( (i*16 + j > end && start > i*16 + j) ||  (mem_head[i*16 + j] == 0))
              printf(".");
            else
              printf("%c",mem_head[i*16 + j]);

        }
        printf("\n");
    }
    return 1;
}
int edit(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 2);
    char* ptr1 = NULL, *ptr2 = NULL;
    int index = strtol(argv[0],&ptr1,16);
    int hex = strtol(argv[1],&ptr2,16);

    if(ptr1!=NULL && *ptr1 != 0) return 0;
    if(ptr2!=NULL && *ptr2 != 0) return 0;
    MEM_IDX_CHECK(0, index);
    if(hex < 0) return 0;

    mem_head[index] = (unsigned char)hex;

    return 1;
}
int fill(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 3);

    char* ptr1 = NULL, *ptr2 = NULL, *ptr3 = NULL;
    int start = strtol(argv[0],&ptr1,16);
    int end = strtol(argv[1],&ptr2,16);
    int hex = strtol(argv[2],&ptr3,16);
    int i;

    if(ptr1!=NULL && *ptr1 != 0) return 0;
    if(ptr2!=NULL && *ptr2 != 0) return 0;
    if(ptr3!=NULL && *ptr3 != 0) return 0;
    MEM_IDX_CHECK(start, end);
    if(hex < 0) return 0;

    for( i = start ; i <= end ; i++)
      mem_head[i] = (unsigned char)hex;
    return 1;
}
int reset(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
    int i;
    for( i = 0 ; i <= 16 * 65536 ; i++)
      mem_head[i] = 0;

    return 1;
}
void opcode_input(){
    FILE* fp = NULL;
    char name[10], line[MAX_CMD]; 
    int opcode, format;

    fp = fopen("opcode.txt","r");
    while(fscanf(fp,"%s",line) != -1){

        opcode = strtol(line,NULL,16);

        fscanf(fp,"%s",line);
        strcpy(name, line);

        fscanf(fp,"%s",line);
        format = line[0] -'0';
        push_hash_node(opcode, name, format);
    }

    fclose(fp);
}
int opcode(int argc, char** argv){

    ARG_COUNT_CHECK(argc, 1);
    char *ptr = &argv[0][0];
    int opcode;
    opcode = hash_search(ptr);

    if(opcode == -1)
      printf("opcode doesn't exist\n");
    else  
      printf("opcode is %02x\n",opcode);
    return 1;
}
int opcodelist(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
    hash_traversal();
    return 1;
}
