#include<stdio.h>//extra information : atoi is impossible and in command ',' needs to fix
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/stat.h>
#define MAX_CMD 256
#define ARG_COUNT_CHECK(num1, num2) do{\
    if((num1) != (num2))\
    return 0;\
}while(0)
typedef struct _node{
    char *cmd;
    struct _node *next;
}Hi_Node;
typedef struct  _Hnode{
    Hi_Node *front;
    Hi_Node *back;
}Hi_Head ;
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

Hi_Head *hi_head = NULL;
Hash_bucket *hash_head = NULL;

unsigned char* mem_head = NULL;

void change_target_to_eos(char *,char );
int parse_arg(char *,int , char **);
void history_free();
void history_push();
int help(int, char**);
int dir(int, char**);
int quit(int, char**);
int history(int, char**);
int dump(int, char**);
int edit(int, char**);
int fill(int, char**);
int reset(int, char**);
void hash_init();
void push_hash_node(int, char [], int);
int hash_func(char*);
void opcode_input();
void hash_free();
int opcode(int, char**);
int opcodelist(int, char**);

int main(){;
    char *command = NULL;
    char **argv = NULL;
    int argc = 0,i;
    hash_init();
    opcode_input();
    command = (char*)calloc(MAX_CMD+1,sizeof(char));
    mem_head = (unsigned char*)malloc(sizeof(unsigned char) * (65536*16));
    argv=(char**)malloc(sizeof(char*)*3);
    for( i = 0 ; i < 3 ; i++ )
      argv[i] = (char*)malloc(sizeof(char)*(MAX_CMD + 1));
    hi_head = (Hi_Head*)malloc(sizeof(Hi_Head));
    hi_head -> front = NULL;
    hi_head -> back = NULL; 
    while(1){
        argc = 0;
        for(i=0 ; i < MAX_CMD ; i++)
          command[i] = '\0';

        printf("sicsim> ");
        fgets(command,MAX_CMD,stdin);


        command[strlen(command)-1] = '\0';
        history_push(command);

        change_target_to_eos(command,' ');
        change_target_to_eos(command,'\t');
        change_target_to_eos(command,',');
        argc = parse_arg(command,MAX_CMD, argv);
        if(argc == -1){
            printf("error!\n");
            continue;
        }
        if(strcmp(command,"h") == 0 || 
           strcmp(command,"help") == 0)
          help(argc, argv);

        else if(strcmp(command,"d") == 0 || 
                strcmp(command,"dir") == 0)
          dir(argc, argv);

        else if(strcmp(command,"q") == 0 || 
                strcmp(command,"quit") == 0){
            if(quit(argc, argv))
              break;
        }
        else if(strcmp(command,"hi") == 0 || 
                strcmp(command,"history") == 0)
          history(argc, argv);

        else if(strcmp(command,"du") == 0 || 
                strcmp(command,"dump") == 0)
          dump(argc, argv);

        else if(strcmp(command,"e") == 0 || 
                strcmp(command,"edit") == 0)
          edit(argc, argv);

        else if(strcmp(command,"f") == 0 || 
                strcmp(command,"fill") == 0)
          fill(argc, argv);

        else if(strcmp(command,"reset") == 0)
          reset(argc, argv);

        else if(strcmp(command,"opcode") == 0){
            printf("%s is argv\n",argv[0]);
            opcode(argc, argv);
        }
        else if(strcmp(command,"opcodelist") == 0)
          opcodelist(argc, argv);


        else{
            printf("error!\n");
        }


    }
    history_free();
    for( i = 0 ; i < 3 ; i++ )
      free(argv[i]);
    free(argv);
    free(mem_head);
    free(command);
    hash_free();
    return 0;
}

void change_target_to_eos(char *command,char del){
    int i;
    for(i = 0; i < MAX_CMD + 1; i++){
        if(command[i] == del){
          command[i] = '\0';
        }
    }
}

int parse_arg(char *command,int max, char **argv){
    char *temp = NULL, *end = NULL;
    int count = 0;
    int i=0;
    i = strlen(command)+1;
    temp = command + strlen(command) + 1;
    end = &command[MAX_CMD-1];
    while(end  > temp){
        if(count > 3){
            count= -1;
            goto endl;
        }
        while(*temp == '\0') {
            temp++;
            if(end-1 <= temp)
              goto endl;
            
        }
        strcpy(argv[count], temp);
        i += strlen(temp)+1;
        temp += strlen(temp) + 1;
        count++;
    }

endl:
    return count;
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
        end = (start/16+10)*16-1;
    }
    else{
        start = strtol(argv[0],&ptr1,16);
        if(argc == 1)
          end =( start/16+10)*16-1;
        else 
          end = strtol(argv[1],&ptr2,16);
    }

    if(ptr1!=NULL && *ptr1 != 0) return 0;
    if(ptr2!=NULL && *ptr2 != 0) return 0;

    prev_end = end + 1;
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
    for( i = 1 ; i < index ; i++)
      temp = temp -> next;
    if(temp -> back == NULL)
      temp -> front = temp -> back =new;
    else{
        temp -> back -> next = new ;
        temp -> back = new;
    }
}
int hash_func(char *id){
    int sum = 1 , i;
    for(i = 0 ; i < 10; i++){
        sum += id[i]%21;
        if(!id[i])
          break;

    }
    return sum%21;
}
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

int opcode(int argc, char** argv){

    ARG_COUNT_CHECK(argc, 1);
    char *ptr = &argv[0][0];
    Hash_bucket *btemp = hash_head;
    Hash_Node *ntemp = NULL;
    int index ,i;

    index = hash_func(ptr);
    printf("%d is indes\n",index);
    for( i = 1 ; i < index ; i++)
      btemp = btemp -> next;
    ntemp = btemp -> front;
    while(ntemp != NULL && strcmp(ptr, ntemp -> name) != 0  )
      ntemp = ntemp -> next;
    if(ntemp == NULL)
      return 0;

    printf("opcode is %02x\n",ntemp->opcode);

    return 1;
}
int opcodelist(int argc, char** argv){
    ARG_COUNT_CHECK(argc, 0);
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
    return 1;
}
