#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"20151575.h"
#include"functions.h"

int main(){
    char command[MAX_CMD+1];
    char *argv[3];
    int argc = 0,i,flag = 0;
    functions_init();
    while(1){
        flag = 1;
        argc = 0;
        for(i=0 ; i< 3 ; i++)
          argv[i] = NULL;
        for(i=0 ; i <= MAX_CMD ; i++)
          command[i] = '\0';

        printf("sicsim> ");
        fgets(command,MAX_CMD,stdin);

        command[strlen(command)-1] = '\0';
        history_push(command); //push to history list(linked list)

        command_nomalization(command);
        argc = parse_arg(command, argv);

        if(argc == -1){
            printf("error!\n");
            continue;
        }

        if(strcmp(command,"h") == 0 || 
           strcmp(command,"help") == 0)
          flag = help(argc, argv);

        else if(strcmp(command,"d") == 0 || 
                strcmp(command,"dir") == 0)
          flag = dir(argc, argv);

        else if(strcmp(command,"q") == 0 || 
                strcmp(command,"quit") == 0){
            if(quit(argc, argv))
              break;
            else
              flag = 0;
        }
        else if(strcmp(command,"hi") == 0 || 
                strcmp(command,"history") == 0)
          flag = history(argc, argv);

        else if(strcmp(command,"du") == 0 || 
                strcmp(command,"dump") == 0)
          flag = dump(argc, argv);

        else if(strcmp(command,"e") == 0 || 
                strcmp(command,"edit") == 0)
          flag = edit(argc, argv);

        else if(strcmp(command,"f") == 0 || 
                strcmp(command,"fill") == 0)
          flag = fill(argc, argv);

        else if(strcmp(command,"reset") == 0)
          flag = reset(argc, argv);

        else if(strcmp(command,"opcode") == 0)
            flag = opcode(argc, argv);
        else if(strcmp(command,"opcodelist") == 0)
          flag = opcodelist(argc, argv);

        else{
            printf("error!\n");
            history_pop();
        }

        if(!flag){
          printf("error!!\n");
          history_pop();
        }



    }
    functions_free();
    return 0;
}
/*
   Command Nomaliztion 
      case 1: string space eos 
      case 2: string space string space eos
      case 3: string space string , string space eos
      case 4: string space string , string , string space eos
   */
void command_nomalization(char* command){
    int max = strlen(command);
    command[max] = ' ';
    command[max + 1] = '\0';
    change_tab_to_space(command);      
    remove_duplication_space(command);
    if(command[0] == ' ')
        strcpy(command , command + 1);
}
void change_tab_to_space(char *command){
    int i,max = strlen(command);
      for(i = 0; i < max; i++){
          if(command[i] == '\t'){
              command[i] = ' '; 
          }
      }
}
void remove_duplication_space(char* command){
    int i,max = strlen(command);
    char temp[MAX_CMD],*start = NULL;
    for(i = 0; i <= max; i++){ // include EOS 
        if(start == NULL && command[i] == ' ') {
            start =command + i;
        }
        else if(start != NULL && command[i] != ' '){
            if(start == command + i -1){
                start = NULL;
                continue;
            }
            else{
                strcpy(temp ,command + i);
                strcpy(start+1 ,temp);
                i = start - command+ 1;
                start = NULL;
            }

        }
    }

}
/*
   Function
   Parsing base on EOS
   */
void trim(int argc, char **argv){
    int i,len;
    for(i=0 ; i < argc ; i++){
        if(argv[i][0] == ' ')
            ++(argv[i]);

        len = strlen(argv[i]);

        if(argv[i][len-1] == ' ')
           argv[i][len-1] = '\0';
    }
}
int parse_arg(char *command,char **argv){
    int i;
    int count = 0;
    char *ptr = NULL, *start = NULL;


    ptr = strchr(command,' ');

    if(ptr == NULL){
        printf("invaild parsing case!!\n");
        return -1;
    }
    *ptr = '\0';
    if(*(ptr+1) != '\0'){
        start = ++ptr;
        for(ptr = strchr(ptr,','); ptr != NULL ; ptr = strchr(ptr,',')){
            if( *(ptr + 1)== ','){ 
                count = -1;
                break;
            }
            *ptr = '\0';
            ++ptr;
            argv[count++] = start;
            start = ptr;
        }
        if(ptr == NULL)
            argv[count++] = start;
        trim(count, argv);
        for(i=0;i<count;i++){
            if(argv[i][0] == ' ' || argv[i][0] == '\0') 
              count = -1;
        }
    }

    return count;
}

