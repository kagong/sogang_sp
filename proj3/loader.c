#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "loader.h"
#include "functions.h"
#include "run.h"

int prog_addr;

struct symbol_node{
    char name[7];
    int addr;
    struct symbol_node *next;
};

struct section_node{
    char name[7];
    int addr;
    int length;
    struct symbol_node *next;
}extern_table[3];

struct reference_node{
    char name[7];
    int Is_vaild;
}reference_table[3000];

int ref_max = 0,extern_max=0;


int link_pass_1(FILE *first,FILE *second,FILE *third,int num);
int link_pass_2(FILE *first,FILE *second,FILE *third,int num);
void insert_link_tables(FILE *fp);
void insert_extern_table(int max,char name[7],int value);
void insert_reference_table(char name[7]);
int check_tables();
void load_obj(FILE* fp);
int search_extern_table(char name[7]);
void print_load_map();
//이름 정규화
void name_nomal(char name[7]){
    int i;
    for(i = 0 ; i < 7 ; i++){
        if('a' <= name[i] && name[i] <= 'z')
            continue;
        else if('A' <= name[i] && name[i] <= 'Z')
            continue;
        else{
            name[i] = '\0';
            return ;
        }
    }
}

void set_progaddr(int addr){
    prog_addr = addr;
}
int get_progaddr(){
    return prog_addr;
}
//자원 해제
void link_load_free(){
    int i;
    struct symbol_node* temp = NULL;
    ref_max = 0;
    if(extern_max == 0)
        return ;
    else{
        for(i = 0 ; i < extern_max ; i++){
            while(extern_table[i].next != NULL){
                temp = extern_table[i].next->next;
                free(extern_table[i].next);
                extern_table[i].next = temp;
            }
        }
        extern_max = 0;
    }
}
//pass 1 pass2 call
int loader(FILE *first,FILE *second,FILE *third,int num){
    int retval;
    link_load_free();//끝날때 한번더 free
    register_init();
    retval = link_pass_1(first,second,third,num);

    if(retval){
        retval = link_pass_2(first,second,third,num);

        if(retval){
            print_load_map();
            return 1;
        }
    }
    return 0;
}
int link_pass_1(FILE *first,FILE *second,FILE *third,int num){
    int error_flag=0;
    switch(num){
        case 3:
            insert_link_tables(first);//참조 테이블과 symbol table 생성
            insert_link_tables(second);
            insert_link_tables(third);
            break;
        case 2:
            insert_link_tables(first);
            insert_link_tables(second);
            break;
        case 1:
            insert_link_tables(first);
            break;
        default :
            printf("link_loader pass 1 argument count error!\n");
            error_flag = 1;
            goto done;
            break;
    }
    error_flag = !check_tables();        //reference table에 유효하지 않는 symbol 확인
done:
    if(error_flag)
        return 0;
    return 1;
}

int link_pass_2(FILE *first,FILE *second,FILE *third,int num){
    switch(num){
        case 3:
            load_obj(first);        //로딩 functions
            load_obj(second);
            load_obj(third);
            break;
        case 2:
            load_obj(first);
            load_obj(second);
            break;
        case 1:
            load_obj(first);
            break;
        default :
            printf("link_loader pass 2 argument count error!\n");
            return 0;
    }
    return 1;
}
void insert_link_tables(FILE *fp){
    int start_addr = extern_max == 0 ? prog_addr : extern_table[extern_max-1].addr + extern_table[extern_max-1].length;
    char line[256];
    char name_temp[7],hex_temp[7],*ptr=NULL;
    int i,max,hex;
    rewind(fp);
    name_temp[6] = '\0';
    hex_temp[6] = '\0';
    while(fgets(line,256,fp) != NULL){
        switch(line[0]){
            case '.':
                break;
            case 'H':
                strncpy(extern_table[extern_max].name,&line[1],6);
                name_nomal(extern_table[extern_max].name);
                extern_table[extern_max].length = strtol(&line[13],&ptr,16);
                extern_table[extern_max].addr = start_addr;
                start_addr += extern_table[extern_max].length;
                break;
            case 'D':
                max = strlen(line);
                for( i = 1 ; i + 11 < max ; i += 12 ){
                    strncpy(name_temp,&line[i],6);
                    name_nomal(name_temp);
                    strncpy(hex_temp,&line[i+6],6);
                    hex = strtol(hex_temp,&ptr,16);
                    insert_extern_table(extern_max,name_temp,hex);
                }
                break;
            case 'R':
                max = strlen(line);
                for( i = 1 ; i + 7 < max; i += 8){
                    strncpy(name_temp,&line[i+2],6);
                    name_nomal(name_temp);
                    insert_reference_table(name_temp);
                }
                break;
            case 'T':
            case 'M':
            case 'E':
                break;
            default:
                printf("unknown record! %s\n",line);
        }
    }
    extern_max++;
}
void insert_extern_table(int max,char name[7],int value){
    int addr = extern_table[max].addr + value;
    struct symbol_node *temp = extern_table[max].next,*new = NULL;
    new = (struct symbol_node *)malloc(sizeof(struct symbol_node));
    strncpy(new->name,name,7);
    new -> addr = addr;
    new -> next = NULL;

    if(temp == NULL)
        extern_table[max].next = new;
    else{
        while(temp -> next != NULL)
            temp = temp -> next;

        temp -> next = new;
    }


}
void insert_reference_table(char name[7]){
    int i;
    for(i = 0 ; i < ref_max; i++){
        if(strncmp(name,reference_table[i].name,7) == 0)
            return ;
    }
    strncpy(reference_table[ref_max].name,name,7);
    reference_table[ref_max++].Is_vaild = 0;

}
int check_tables(){
    int i,j,error_flag = 0;
    char *name = NULL;
    struct symbol_node* temp = NULL;
    for(i = 0 ; i < extern_max ; i++){
        name = extern_table[i].name;
        temp = extern_table[i].next;
        for(j = 0 ; j < ref_max; j++){
            if(strncmp(name,reference_table[j].name,7) == 0)
                reference_table[j].Is_vaild = 1;
        }
        while(temp != NULL){
            name = temp->name;
            for(j = 0 ; j < ref_max; j++){
                if(strncmp(name,reference_table[j].name,7) == 0)
                    reference_table[j].Is_vaild = 1;
            }
            temp = temp->next;
        }

    }
    for(i = 0 ; i < ref_max ; i++){
        if(!reference_table[i].Is_vaild){
            error_flag = 1;
            break;
        }
    }
    if(error_flag){
        printf("Unknown Reference Record Extern Symbol : %s\n",reference_table[i].name);
        return 0;
    }
    return 1;

}
void load_obj(FILE* fp){
    int start_addr;
    char line[256];
    int reference_addr[100];
    char name_temp[7],hex_temp[7],*ptr=NULL;
    int i,max,hex,size,target, value,upper;
    int add_flag = 0;
    rewind(fp);
    name_temp[6] = '\0';
    hex_temp[6] = '\0';
    while(fgets(line,256,fp) != NULL){
        add_flag = 0;
        ptr = strchr(line,'\n');
        if(ptr != NULL)
            *ptr = '\0';
        ptr = strchr(line,'\r');
        if(ptr != NULL)
            *ptr = '\0';
        switch(line[0]){
            case '.':
                break;
            case 'H':
                strncpy(name_temp,&line[1],6);
                name_nomal(name_temp);
                reference_addr[0] = search_extern_table(name_temp);
                break;
            case 'R':
                max = strlen(line);
                for( i = 1 ; i < max; i += 8){

                    strncpy(hex_temp,&line[i],2);
                    hex_temp[2]='\0';
                    hex = strtol(hex_temp,&ptr,16);

                    strncpy(name_temp,&line[i+2],6);
                    name_nomal(name_temp);
                    reference_addr[hex-1] = search_extern_table(name_temp);
                }
                break;
            case 'T':
                strncpy(hex_temp,&line[1],6);
                start_addr = strtol(hex_temp,&ptr,16) + reference_addr[0];

                strncpy(hex_temp,&line[7],2);
                hex_temp[2] = '\0';
                hex = strtol(hex_temp,&ptr,16);
                for(i = 0 ; i < hex*2 ; i +=6 ){

                    if(i + 6 > hex*2){
                        size =(hex*2 - i)/2; 
                        strncpy(hex_temp,&line[9+i],size*2);
                        hex_temp[size*2]='\0';
                    }
                    else{
                        size = 3;
                        strncpy(hex_temp,&line[9+i],6);
                    }
                    value = strtol(hex_temp,&ptr,16);
                    mem_load(start_addr+i/2,value,size);
                }

                break;
            case 'M':

                strncpy(hex_temp,&line[1],6);
                start_addr = strtol(hex_temp,&ptr,16) + reference_addr[0];

                strncpy(hex_temp,&line[7],2);
                hex_temp[2] = '\0';
                hex = strtol(hex_temp,&ptr,16);

                if(line[9] == '+')
                    add_flag = 1;

                strncpy(hex_temp,&line[10],2);
                hex_temp[2] = '\0';
                target = strtol(hex_temp,&ptr,16);

                value = mem_store(start_addr,(hex+1)/2);
                upper = (value >> hex*4) << hex*4;
                value -= upper;
                if(add_flag)
                    value += reference_addr[target-1];
                else
                    value -= reference_addr[target-1];
                value = (value <<(32 - hex*4)) >>(32 - hex*4);
                value += upper;
                mem_load(start_addr,value,(hex+1)/2);
                break;
            case 'D':
            case 'E':
                break;
            default:
                printf("unknown record! %s\n",line);
        }
    }
}
//symbol table 을 탐색 해주는 함수로써 symbol의 addr을 반환
int search_extern_table(char name[7]){
    int i;
    struct symbol_node *temp = NULL;
    for(i = 0 ;i < extern_max;i++){
        if(strcmp(name,extern_table[i].name) == 0)
            return extern_table[i].addr;
        temp = extern_table[i].next;
        while(temp != NULL){
            if(strncmp(temp->name,name,7) == 0)
                return temp -> addr;
            temp = temp -> next;
        }
    }
    printf("error! would not find extern address....\n");
    return -1;
}
void print_load_map(){
    int i,total=0;
    struct symbol_node* temp = NULL;
    printf("%-6s\t%-6s\t%-8s\t%-8s\n","control","symbol","address","length");
    printf("%-6s\t%-6s\n","section","name");
    printf("----------------------------------------\n");
    for(i = 0 ; i < extern_max;i++){
        total += extern_table[i].length;
        printf("%-6s\t%-6s\t%-8X\t%04X\n",extern_table[i].name,"",extern_table[i].addr,extern_table[i].length);
        temp = extern_table[i].next;
        while(temp != NULL){
            printf("%-6s\t%-6s\t%-8X\n","",temp->name,temp->addr);
            temp = temp -> next;
        }
    }
    printf("----------------------------------------\n");
    printf("%6s\t%6s\t%-8s\t%04X\n","","","total length",total);
}
