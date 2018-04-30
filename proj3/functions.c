#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/stat.h>
#include"hash.h"
#include"functions.h"
#include"assemble.h"
#include"loader.h"
#include"run.h"

#define MEM_MAX 65536*16

//Error Checking Mecro
#define ARG_COUNT_CHECK(num1, num2) do{\
    if((num1) != (num2)){\
        printf("Arguments Error!\n");\
    return 0;}\
}while(0)

#define MEM_IDX_CHECK(start, end) do{\
    if(((end) < (start)) \
    || ((start) < 0 || (start)>= 65536 * 16)\
      ||((end) < 0 || (end)>= 65536 * 16)){\
        printf("Index Error!\n");\
    return 0;}\
}while(0)

#define HEX_CONVERT_CHECK(ptr) do{\
    if((ptr)!=NULL && *(ptr) != 0){\
        printf("It Isn't Hex\n");\
        return 0;}\
}while(0)

/*
   DataStructure For History
   - Implemented as a linked list
   */
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

/*
   Function for
   init meme and hash for using opcodetable 
   */
void functions_init(){
    hash_init();
    opcode_input();
    symbol_init();

    mem_head = (unsigned char*)malloc(sizeof(unsigned char) * (MEM_MAX));

    hi_head = (Hi_Head*)malloc(sizeof(Hi_Head));
    hi_head -> front = hi_head -> back = NULL; 
    
}

//Two list Deallocation
void functions_free(){
    history_free();
    hash_free();
    symbol_free();
    free(mem_head);
    link_load_free();
}

//Push new node to history list
void history_push(char* command){	

    int len;
    Hi_Node *new = NULL;
    
    len = strlen(command) + 1;

    new = (Hi_Node*)malloc(sizeof(Hi_Node));
    new -> cmd = (char*)malloc(sizeof(char)*len);
    new -> next = NULL;
    strcpy(new -> cmd, command);

    if(hi_head -> front == NULL){ // First node
        hi_head -> front = new;	
        hi_head -> back = new;	
    }
    else{
        hi_head -> back -> next = new;
        hi_head -> back = new;
    }

}

// Except the history node when invaild command is entered
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
        while(temp -> next != hi_head -> back) //find in front of the last node
          temp = temp -> next;

        free(temp -> next -> cmd);
        free(temp -> next);
        hi_head -> back = temp;
    }
}

//Deallocation Resource
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


//Print Shell
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
    printf("assemble filename\n");	
    printf("type filename\n");	
    printf("symbol\n");	
    printf("progaddr\n");
    printf("loader\n");
    printf("run\n");
    printf("bp\n");
    return 1;
}

//Function for dir list
int dir(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
    struct dirent *entry;
    struct stat info;
    DIR *dp;
    dp = opendir(".");
    if(dp == NULL) return 0;
    while((entry = readdir(dp))){
        if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0) //Except now and parent dir
          continue;
        printf("%s",entry->d_name);
        stat(entry->d_name,&info);
        switch(info.st_mode & S_IFMT){      //Masking 
          case S_IFDIR: printf("/\t");break;    //dir bit

          case S_IFREG:                         
                        if((info.st_mode & S_IRWXU & S_IXUSR) != 0)   //Excute bit
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

/* Function for 
   Until now, save the vaild commands
   */
int history(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
    int i = 0;
    Hi_Node * temp = NULL;

    temp = hi_head -> front;
    if(hi_head -> front == hi_head -> back) //when First command is entered as "history"
      return 1;

    while(temp != NULL){

        printf("%d\t%s\n",i,temp -> cmd);

        temp = temp -> next;
        ++i;
    }
    return 1;
}

/*Function for
  printing Memory value
   */
int dump(int argc, char** argv){
    static int prev_end=0;
    char* ptr1 = NULL, *ptr2 = NULL;
    int start=0, end=0;
    int i,j;

    //Indexing by number of Arguments
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
          MEM_IDX_CHECK(start, end);
        }
    }

    HEX_CONVERT_CHECK(ptr1);
    HEX_CONVERT_CHECK(ptr2);

    if(end >= MEM_MAX)
      end = MEM_MAX - 1;

    prev_end = end + 1;

    if(prev_end >= MEM_MAX)
      prev_end = 0;

    for(i = start / 16 ; i <= end / 16 ; i++){      // line iter
        printf("%06X ",i*16);
        for( j = 0 ; j <16 ; j++){                  // mem value iter
            if( start <= i*16 + j && i*16 + j <= end)
             printf("%02X ",mem_head[i*16 + j]);
            else
              printf("   ");
        }
        printf("; ");
        for(j = 0 ; j <16 ; j++){                   //mem ascil iter
            if( (i*16 + j > end && start > i*16 + j) ||  (mem_head[i*16 + j] == 0))
              printf(".");
            else if( 0x20 <= mem_head[i*16 + j] && mem_head[i*16 + j] <=0x7E)
              printf("%c",mem_head[i*16 + j]);
            else
              printf(".");

        }
        printf("\n");
    }
    return 1;
}

/*
   Function for
   Assign a value to a specific memory 
   */
int edit(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 2);

    char* ptr1 = NULL, *ptr2 = NULL;
    int index = strtol(argv[0],&ptr1,16);
    int hex = strtol(argv[1],&ptr2,16);

    HEX_CONVERT_CHECK(ptr1);
    HEX_CONVERT_CHECK(ptr2);
    MEM_IDX_CHECK(0, index);

    if( 255 < hex || hex < 0){
        printf("Hex Value Error\n");
        return 0;
    }

    mem_head[index] = (unsigned char)hex;

    return 1;
}

/*
   Function for
   Assign a value to specific range of memory 
   */
int fill(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 3);

    char* ptr1 = NULL, *ptr2 = NULL, *ptr3 = NULL;
    int start = strtol(argv[0],&ptr1,16);
    int end = strtol(argv[1],&ptr2,16);
    int hex = strtol(argv[2],&ptr3,16);
    int i;

    HEX_CONVERT_CHECK(ptr1);
    HEX_CONVERT_CHECK(ptr2);
    HEX_CONVERT_CHECK(ptr3);
    MEM_IDX_CHECK(start, end);

    if(255 < hex || hex < 0) {
        printf("Hex Value Error\n");
        return 0;
    }

    for( i = start ; i <= end ; i++)
      mem_head[i] = (unsigned char)hex;

    return 1;
}
int mem_store(int address,int size){
    unsigned int i = 0,retval = 0;
    for( i = 0 ; i < size ; i++){
        retval <<= 2*4;
        retval += (unsigned int)mem_head[address + i];
    }
    return retval;
}
void mem_load(int address,int value,int size){
    int i = 0;
    for( i = 0 ; i < size ; i++){
        mem_head[address + i] = (unsigned char)(0x000000FF & (value >> 2*4*(size - i - 1)));
    }
}
/*
   Function for
   Assign a zero to all memory
   */
int reset(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
    int i;

    for( i = 0 ; i <= 16 * 65536 ; i++)
      mem_head[i] = 0;

    return 1;
}

int Is_opcode(char *opcode_name){
    int opcode;
    opcode = hash_search(opcode_name,"opcode");
    return opcode != -1 ? 1 : 0;
}
int opcode_format(char *opcode_name){
    int format;
    format = hash_search(opcode_name,"format");
    if(format == -1){
        printf("Opcode Doesn't Exist\n");
        return -1;
    }
    else
      return format;
}
int opcode_code(char *opcode_name){
    int code;
    code = hash_search(opcode_name,"opcode");
    if(code == -1){
        printf("Opcode Doesn't Exist\n");
        return -1;
    }
    else
      return code;
}

/*
   Function for
   Read "opcode.txt" and push it to hash_list
   */
void opcode_input(){
    FILE* fp = NULL;
    char name[10], line[10]; 
    int opcode, format;

    fp = fopen("opcode.txt","r");
    while(fscanf(fp,"%X %s %s",&opcode,name,line) != EOF){
        format = line[0] -'0';
        push_hash_node(opcode, name, format);
    }

    fclose(fp);
}

/*
   Function for
   Search opcode and Print it
   */
int opcode(int argc, char** argv){

    ARG_COUNT_CHECK(argc, 1);
    char *ptr = &argv[0][0];
    int opcode;
    opcode = opcode_format(ptr);

    if(opcode == -1)
      printf("Opcode Doesn't Exist\n");
    else  
      printf("opcode is %02x\n",opcode);
    return 1;
}

/*
   Function for
   print all opcode
   */
int opcodelist(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
    hash_traversal();
    return 1;
}
int assemble(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 1);
    assembler(argv[0]);
    return 1;
}
int type(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 1);
    char line[256],result;
    FILE *fp = NULL;
    fp = fopen(argv[0],"r");
    if(fp == NULL){
        printf("Don't exist file\n");
        return 0;
    }
    while( fread(&result, sizeof(char), 1, fp)){
		printf("%c", result);
    }
    if(fclose(fp)){
        printf("Error on file close\n");
        return 0;
    }
    return 1;
}
int symbol(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
    print_symbol_table();
    return 1;
}
int call_progaddr(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 1);
    char *ptr = NULL;
    int addr = strtol(argv[0],&ptr,16);
    HEX_CONVERT_CHECK(ptr);
    set_progaddr(addr);

    return 1;
}
int call_loader(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 1);
    FILE *first = NULL, *second = NULL, *third = NULL;
    int num = 0,retval=0;
    char *saveptr = NULL,*token = NULL;
    
    token = strtok_r(argv[0]," ",&saveptr);
    if(token == NULL)
        return 0;
    else{
        first = fopen(token,"r");
        if(first == NULL){
            printf("Don't exist %s file\n",token);
            return 0;
        }
        ++num;
    }

    token = strtok_r(NULL," ",&saveptr);
    if(token != NULL){
        second = fopen(token,"r");
        if(second == NULL){
            printf("Don't exist %s file\n",token);
            return 0;
        }
        ++num;
        token = strtok_r(NULL," ",&saveptr);
        if(token != NULL){
            third = fopen(token,"r");
             if(third == NULL){
                 printf("Don't exist %s file\n",token);
                return 0;
            }
             ++num;
        }
    }

    retval = loader(first,second,third,num);

    if(fclose(first) || fclose(second) || fclose(third)){
        printf("file close error!\n");
        return 0;
    }
    return retval;
}
int call_run(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
    int temp;
    temp = run(get_progaddr());
    if(temp == -1)
        return 0;
    set_progaddr(temp);
    return 1;
}
int call_bp(int argc, char** argv){
    if(argc > 1){
        printf("Break Point Function Error! :Too many Arguments!!\n");
        return 0;
    }
    else if(argc == 0)
        return bp(NULL);
    else
        return bp(argv[0]);
}
    
