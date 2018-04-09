#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"20151575.h"
#include"assemble.h" // command 마지막에 주석 오는건?
#include"functions.h"
#define LOCATE_TEMP_NAME "temp.txt"

#define MODE_SIMPLE 	0X00030000
#define MODE_INDIRECT 	0X00020000
#define MODE_IMMEDIATE 	0X00010000
#define MODE_INDEX 		0X00008000
#define MODE_BASE	 	0X00004000
#define MODE_PC 		0X00002000
#define MODE_EXTEND 	0X00001000

#define REG_A 0x00
#define REG_X 0x01
#define REG_L 0x02
#define REG_B 0x03
#define REG_S 0x04
#define REG_T 0x05
#define REG_F 0x06

#define HEX_CONVERT_CHECK(ptr,flag) do{\
    if((ptr)!=NULL && *(ptr) != 0){\
        (flag) = 1;}\
}while(0)

typedef struct _symbol{
    char label[11];
    int loc;
}symbol_node;

symbol_node **symbol_table = NULL;
int symbol_max ,symbol_len;

int start,end,m_size=0;
char name[100];
int m_record[100];

int assembler(char *file_name){
    if(strcmp(&file_name[strlen(file_name)-4], ".asm") != 0 ){
        printf("Is is not .asm\n");
        return 0;
    }

    symbol_clear();
    if(assemble_pass_1(file_name))
      assemble_pass_2(file_name);

    return 1;
}
int assemble_pass_1(char* file_name){
    char line[111],*result = NULL,*ptr = NULL;
    char label[8],*opcode=NULL,*arg1=NULL,*arg2=NULL;
    FILE *ifp = NULL, *ofp = NULL;
    int i, LOCATER = 0,prev=0,argc,flag = 0,num=0,error = 0;
    ifp = fopen(file_name,"r");
    ofp = fopen(LOCATE_TEMP_NAME,"w");

    if(ifp == NULL){
        printf("Don't exist .asm file\n");
        return 0;
    }
    if(ofp == NULL){
        printf("file open error!\n");
        return 0;
    }
    for(i=0;i<111;i++)
      line[i] = 0;


    while((result = fgets(line,111,ifp)) != NULL ) {
        num++;
        flag = 0;
        prev = LOCATER;
        // line[strlen(line)-1] = ' ';
        line[strlen(line)-1] = '\0';
        opcode = arg1 = arg2 = NULL;
        argc = parsing_line(line,label,&opcode,&arg1,&arg2);
        /*
           printf("************\nlabel - %s\n opcode - %s\n arg1 - %s\n arg2 - %s\n**************\n\n",
           label
           ,opcode!= NULL ? opcode:"none"  
           ,arg1!= NULL ? arg1:"none"
           ,arg2!= NULL ? arg2:"none");
           */
        if(argc == 0 && opcode == NULL){
            fprintf(ofp,"%d\n",-1);
            continue;
        }
        if(strcmp(opcode,"END") == 0){
            fprintf(ofp,"%d\n",-1);
            fprintf(ofp,"%d\n",-1);
            end = LOCATER;
            break;
        }

        else if(strcmp(opcode,"START") == 0){
            if(arg1 == NULL){
                printf("Line%d : ",num);
                printf("pass1 opcode error! \n");
                error = 1;
                break;
            }
            strcpy(name,label);
            start = strtol(arg1,&ptr,16); //error want
            HEX_CONVERT_CHECK(ptr,flag);
            if(flag){
                printf("Line%d : ",num);
                printf("It Isn't Hex\n");
                error = 1;
                break;
            }

            LOCATER = start;
            fprintf(ofp,"%05X\n",LOCATER);
        }
        else if(strcmp(opcode,"BASE") == 0||label[0] == '.'){
            fprintf(ofp,"%d\n",-1);
        }
        else{
            if(strlen(label) != 0){
                if(!Is_symbol(label))//no double label 좀더 고치자
                  Insert_symbol(label,LOCATER);
                else if(label[0] == '.') //all arguments need to nomalizate
                  continue;
                else{
                    printf("Line%d : ",num);
                    printf("pass1 label error!\n");
                    error = 1;
                    break;
                }
            }

            if(opcode != NULL){
                i = (opcode[0] == '+') ? 1:0;
                if(Is_opcode(&opcode[i])){
                    LOCATER += opcode_format(&opcode[i]);
                    if(i == 1)
                      LOCATER+=1;
                }
                else {
                    if(arg1 == NULL){
                        printf("Line%d : ",num);
                        printf("pass1 No argument!\n");
                        error = 1;
                        break;
                    }
                    if(strcmp(opcode,"WORD") == 0){
                        LOCATER += 3;
                        HEX_CONVERT_CHECK(ptr,flag);
                        if(flag){
                            printf("Line%d : ",num);
                            printf("It Isn't Hex\n");
                            error = 1;
                            break;
                        }

                    }

                    else if(strcmp(opcode,"RESW") == 0){
                        LOCATER += strtol(arg1,&ptr,10)*3; // 좀더 해야함
                        HEX_CONVERT_CHECK(ptr,flag);
                        if(flag) {
                            printf("Line%d : ",num);
                            printf("It Isn't Hex\n");
                            error = 1;
                            break;
                        }
                    }

                    else if(strcmp(opcode,"RESB") == 0){
                        LOCATER += strtol(arg1,&ptr,10);
                        HEX_CONVERT_CHECK(ptr,flag);
                        if(flag){
                            printf("Line%d : ",num);
                            printf("It Isn't Hex\n");
                            error = 1;
                            break;
                        }
                    }

                    else if(strcmp(opcode,"BYTE") == 0){
                        if(arg1[0] == 'C' || arg1[0] == 'X'){
                            int cnt;
                            cnt = find_charater(&arg1[1]);

                            if(cnt == -1){
                                printf("Line%d : ",num);
                                printf("pass1 BYTE count error!\n");
                                error = 1;
                                break;
                            }

                            else if(arg1[0] != 'X')
                              LOCATER += cnt;

                            else
                              LOCATER += cnt/2;

                        }
                        else{
                            printf("Line%d : ",num);
                            printf("pass1 BYTE error\n"); // 여기는 아직 모르겠다.
                            error = 1;
                            break;
                        }

                    }
                    else {
                        printf("Line%d : ",num);
                        printf("pass1 unknown opcode!\n");
                        error = 1;
                        break;
                    }
                }
                fprintf(ofp,"%05X\n",LOCATER);

            }

        }
    }

    if(fclose(ifp) || fclose(ofp)){
        printf("Error on file close\n");
        return 0;
    }
    else if(error)
      return 0;
    return 1;
}
int assemble_pass_2(char* file_name){
    char copy[111];
    char line[111], label[8] ,*opcode ,*arg1 ,*arg2, *ptr=NULL;
    FILE *ifp = NULL,*locate_fp=NULL,*obj_fp = NULL, *lst_fp = NULL;
    int i,prev, LOCATER = 0,format,hex,opcode_start = 0,object_code=0,arg_start=0,target_addr=0,flag = 0;
    int B=0, argc, error = 0;

    ifp = fopen(file_name,"r");
    locate_fp = fopen(LOCATE_TEMP_NAME,"r");
    strcpy(&file_name[strlen(file_name)-3], "lst");
    lst_fp = fopen (file_name,"w");
    strcpy(&file_name[strlen(file_name)-3], "obj");
    obj_fp = fopen (file_name,"w");

    if(ifp == NULL){
        printf("Don't exist .asm file\n");
        return 0;
    }
    if(locate_fp == NULL){
        printf("locate temp file open error!\n");
        return 0;
    }
    if(lst_fp == NULL){
        printf("locate temp file open error!\n");
        return 0;
    }
    for(i=0;i<111;i++)
      line[i] = 0;
    i=0;

    while(fgets(line,111,ifp) != NULL ) {
        opcode = arg1 = arg2 = NULL;
        //    line[strlen(line)-1] = ' ';  //Change Command's '\n' to EOS
        line[strlen(line)-1] = '\0';  //Change Command's '\n' to EOS
        strcpy(copy,line);
        object_code = 0;
        flag = 0;
        ++i;
        fscanf(locate_fp,"%X",&LOCATER);//다음거를 긁어오게 만들어야함
        argc = parsing_line(line,label,&opcode,&arg1,&arg2);

        if(argc == -1) {
            printf("Line%d : ",i);
            printf("pass2 line parsing error!\n");
            error = 1;
            break;
        } 

        if(LOCATER == -1){
            fprintf(lst_fp,"%3d %4s %s\n",i*5,"    ",copy);

            if(opcode == NULL)
              continue;
            else if(strcmp(opcode,"END") == 0){
                T_push_to_obj(obj_fp,-1,LOCATER);
                M_print_to_obj(obj_fp);
                E_print_to_obj(obj_fp);
                break;
            }
            else if(strcmp(opcode,"BASE") == 0){
                if(arg1 != NULL && Is_symbol(arg1))
                  B = symbol_addr(arg1);
            }

            else if(strcmp(opcode,"NOBASE") == 0){
                if(arg1 != NULL){
                    printf("Line%d : ",i);
                    printf("pass2 Nobase must have no arguments! error!\n");
                    error = 1;
                    break;
                }
                B = 0;
            }
            continue;
        }


        if(strcmp(opcode,"START") == 0){
            H_print_to_obj(obj_fp);
            print_to_lst(lst_fp,i,LOCATER,copy,-1);// 길이마다 따로 출력해야할듯.
            prev = LOCATER;
            continue;
        }

        if(opcode[0] == '+')
          opcode_start = 1;
        else
          opcode_start = 0;
        if(Is_opcode(&opcode[opcode_start])){

            format = opcode_format(&opcode[opcode_start]);
            hex = opcode_code(&opcode[opcode_start]);

            if(format == 1){
                if(arg1 != NULL || arg2 != NULL){
                    printf("Line%d : ",i);
                    printf("pass2 extra arguments error!\n");
                    error = 1;
                    break;
                }

                object_code = hex;
            }
            else if(format == 2){
                object_code = hex <<8;

                if(Is_Reg(arg1))
                  object_code |= To_Reg(arg1) << 4;

                else if(Is_const(arg1))
                  object_code |= To_const(arg1) << 4;

                if(Is_Reg(arg2))
                  object_code |= To_Reg(arg2);
                else if(Is_const(arg2))
                  object_code |= To_const(arg2);

            }
            else if(format == 3){
                if(opcode_start == 1){
                    object_code = hex << 24;
                    object_code |=  MODE_EXTEND  << 8;
                    if(arg1 != NULL && arg2 == NULL ){

                        if(Is_symbol(arg1)){
                            M_push_to_obj(obj_fp,prev);
                            object_code |= symbol_addr(arg1);
                            object_code |=  MODE_SIMPLE  << 8;
                        }
                        else if(Is_const(arg1)){
                            object_code |= To_const(arg1);
                            object_code |=  MODE_IMMEDIATE  << 8;
                        }
                        else if(Is_addr(arg1)){
                            object_code |= symbol_addr(&arg1[1]);
                            object_code |=  MODE_INDIRECT  << 8;
                        }
                        else{
                            printf("Line%d : ",i);
                            printf("pass2 type 4 arguments error!\n");
                            error = 1;
                            break;
                        }
                    }
                    else{
                        printf("Line%d : ",i);
                        printf("pass2 unknown for type 4\n");
                        error = 1;
                        break;
                    }
                }
                else{
                    object_code = hex << 16;
                    if(argc == 2){                
                        if(strcmp(&arg2[0],"X") != 0){
                            printf("Line%d : ",i);
                            printf("pass2 Index Register error!\n");
                            error = 1;
                            break;
                        }
                        object_code |= (MODE_INDEX | MODE_SIMPLE);
                    }
                    else if(argc != 1){
                        if(strcmp(opcode,"RSUB") != 0){
                            printf("Line%d : ",i);
                            printf("pass2 Didn't exist argument error!\n");
                            error = 1;
                            break;
                        }
                        object_code |= MODE_SIMPLE ;
                        T_push_to_obj(obj_fp,object_code,LOCATER);
                        print_to_lst(lst_fp,i,prev,copy,object_code);// 길이마다 따로 출력해야할듯.
                        prev = LOCATER;
                        continue;
                    }
                    else if(Is_const(arg1)){
                        arg_start = 1;
                        object_code |= MODE_IMMEDIATE;
                    }
                    else if(Is_addr(arg1)){
                        arg_start = 1;
                        object_code |= MODE_INDIRECT;
                    }
                    else{
                        arg_start = 0;
                        object_code |= MODE_SIMPLE;
                    }

                    if(Is_symbol(&arg1[arg_start])){
                        target_addr = symbol_addr(&arg1[arg_start]);

                        if(Is_Raddr(LOCATER,target_addr))
                          object_code |= (MODE_PC | (0xfff &To_Raddr(LOCATER,target_addr)));

                        else if(Is_Baddr(B,target_addr))
                          object_code |= (MODE_BASE | To_Baddr(B,target_addr));

                        else{
                            printf("Line%d : ",i);
                            printf("pass2 Address range error!\n");
                            error = 1;
                            break;
                        }
                    }
                    else if(Is_const(arg1)){
                        object_code |= To_const(arg1);//추가 처리; 오류랑 범위

                    }
                    else{
                        printf("Line%d : ",i);
                        printf("pass2 Unknown type for type 3 error!\n");
                        error = 1;
                        break;
                    }
                }
            }
        }
        else if(strcmp(opcode,"WORD") == 0){
            //addtional error handling for strtol
            hex = strtol(arg1,&ptr,10);
            HEX_CONVERT_CHECK(ptr,flag);
            if(flag){  
                printf("Line%d : ",i);
                printf("It Isn't Hex\n");
                error = 1;
                break;
            }

            object_code = hex;
        }
        else if(strcmp(opcode,"BYTE") == 0){
            if(arg1[0] == 'C' || arg1[0] == 'X'){
                int cnt;
                cnt = find_charater(&arg1[1]);

                if(cnt == -1){
                    printf("Line%d : ",i);
                    printf("pass2 BYTE count error!\n");
                    error = 1;
                    break;
                }

                else if(arg1[0] != 'X'){
                    for(int iter = 0 ; iter < cnt ; iter++)
                      object_code|= arg1[1+iter]<<(cnt-iter-1)*8;
                }
                else{
                    hex = strtol(&arg1[1],&ptr,16);
                    HEX_CONVERT_CHECK(ptr,flag);
                    if(flag)  {
                        printf("Line%d : ",i);
                        printf("It Isn't Hex\n");
                        error = 1;
                        break;
                    }
                    object_code = hex;
                }
            }
            else{
                printf("Line%d : ",i);
                printf("pass2 BYTE error!\n"); // 여기는 아직 모르겠다.
                error = 1;
                break;
            }
        }
        else if(strcmp(opcode,"RESW") == 0 || strcmp(opcode,"RESB") == 0){
            object_code = -1;
        }
        else{
            printf("Line%d : ",i);
            printf("PASS2 Unknown opcode error!\n");
            error = 1;
            break;
        }

        T_push_to_obj(obj_fp,object_code,LOCATER);
        print_to_lst(lst_fp,i,prev,copy,object_code);// 길이마다 따로 출력해야할듯.
        prev = LOCATER;
    }
    if(fclose(ifp) || fclose(locate_fp) || fclose(lst_fp)){
        printf("Error on file close\n");
        return 0;
    }
    remove(LOCATE_TEMP_NAME);
    if(error)
      return 0;
    file_name[strlen(file_name)-4] = '\0';
    printf("output file: [%s.lst] [%s.obj]\n",file_name, file_name);
    return 1;
}

// if , line then label = '.' , line = ~string

// more and more 앞 스페이스도 제거
//20151575 껄 쓰되 . 앞에 \0 넣고  배열이 아니라 포인터로 나눔 그리고 line의 arg1
//arg2 가 있을 수도 있고 없을 수도 있음
int parsing_line(char line[111], char label[8],char** opcode,char **arg1 , char **arg2){//추가적인작업필요
    int i=0;
    char temp[8];
    char *ptr = NULL;
    char *argv[3];
    int argc;

    ptr = strchr(line,'.');
    if(ptr != NULL){
        *ptr = '\0';
    }

    for(i=0;i<7;i++)
      label[i] = line[i];
    label[7] = '\0';
    ptr = label;
    trim(ptr);
    strcpy(temp,ptr);
    strcpy(label,temp);

    if(strlen(line) < 8)
      return 0;

    strcpy(line,&line[7]);
    command_nomalization(line);
    argc = parse_arg(line, argv);

    *opcode = line;
    *arg1 = argc > 0 ? argv[0] : NULL;
    *arg2 = argc > 1 ? argv[1] : NULL;
    if(argc == 1 && strlen(*arg1) == 0){
        arg1 = NULL;
        --argc;
    }
    else if(argc == 2 && strlen(*arg2) == 0){
        arg2 = NULL;
        --argc;
    }
    return argc;
}
void symbol_init(){
    symbol_len = 0;
    symbol_max = 100;
    symbol_table = (symbol_node**)malloc(symbol_max * sizeof(symbol_node*));
}
void symbol_clear(){
    int i;
    for(i=0;i<symbol_len;i++){
        free(symbol_table[i]);
        symbol_table[i] = NULL;
    }
    symbol_len = 0;
}
void symbol_free(){
    int i;
    for(i=0;i<symbol_len;i++)
      free(symbol_table[i]);

    free(symbol_table);
}
int Is_symbol(char* label){
    int i;
    for(i = 0 ; i < symbol_len ; i ++){
        if(strcmp(symbol_table[i]->label,label) == 0)
          return 1;
    }
    return 0;
}
int symbol_addr(char* symbol){
    int i;
    for(i = 0 ; i < symbol_len ; i ++){
        if(strcmp(symbol_table[i]->label,symbol) == 0)
          return symbol_table[i] -> loc;
    }
    return -1;
}
void Insert_symbol(char *symbol, int loc_now){ 
    int i, index;
    symbol_node* new = NULL;
    new = (symbol_node*)malloc(sizeof(symbol_node));
    new -> loc = loc_now;
    strcpy(new -> label,symbol);
    if(symbol_len + 1 > symbol_max){
        symbol_max *= 2;
        symbol_table =(symbol_node**)realloc(symbol_table,symbol_max*(sizeof(symbol_node*)));
    }
    for(i=0;i<symbol_len;i++){
        if(strcmp(symbol_table[i]->label,symbol) > 0)
          break;
    }
    index = i;
    for(i=symbol_len-1 ; i >= index ;i--){
        symbol_table[i+1] = symbol_table[i];
    }
    ++symbol_len;
    symbol_table[index] = new;
}
int Is_Reg(char *argument){
    if(argument == NULL)
      return 0;
    if(strlen(argument) != 1)
      return 0;
    switch(argument[0]){
      case 'A':
      case 'X':
      case 'L':
      case 'B':
      case 'S':
      case 'T':
      case 'F':
        return 1;
      default:
        return 0;
    }
}
int To_Reg(char *argument){
    switch(argument[0]){
      case 'A':
        return REG_A;
      case 'X':
        return REG_X;
      case 'L':
        return REG_L;
      case 'B':
        return REG_B;
      case 'S':
        return REG_S;
      case 'T':
        return REG_T;
      case 'F':
        return REG_F;
    }
}
int Is_const(char *argument){//symbol 까지확인
    char *ptr = NULL;
    int n,flag=0;
    if(argument == NULL)
      return 0;
    if(argument[0] != '#')
      return 0;

    if(Is_symbol(&argument[1]))
      return 1;

    n = strtol(&argument[1],&ptr,10);
    HEX_CONVERT_CHECK(ptr,flag);
    if(flag)  return 0;

    return 1;
}
int To_const(char *argument){//symbol 까지변환
    int n;
    if(Is_symbol(&argument[1]))
      return symbol_addr(&argument[1]);

    return strtol(&argument[1],NULL,10);
}
int Is_addr(char *argument){

    if(argument[0] != '@')
      return 0;

    else if(Is_symbol(&argument[1]))
      return 1;

    return 0;
}
int Is_Raddr(int loc,int addr){
    if(-2048 <=addr - loc && addr - loc <= 2047){
        return 1;
    }
    else
      return 0;
}
int To_Raddr(int loc,int addr){
    int n = addr - loc;
    return n;

}
int Is_Baddr(int B,int addr){
    if(0 <= addr - B && addr - B <= 4095){
        return 1;
    }
    else
      return 0;
}
int To_Baddr(int B,int addr){
    return addr - B;
}
void H_print_to_obj(FILE* obj_fp){

    fprintf(obj_fp,"H%-6s%06X%06X\n",name,start,end-start);
}
void T_push_to_obj(FILE* obj_fp,int object_code, int loc){
    static int buffer[30];
    static int buffer_size =0;
    static int flag = 0;
    static int field=0 ,heap=0; 

    if(field < start)
      field = start;
    if(object_code != -1){
        buffer[buffer_size++] = object_code;
        flag = 0;
        heap = loc;
    }
    else if(flag)
      field = loc;

    else
      flag = 1;

    if(buffer_size != 0) {
        if(flag ||  heap-field >=0x1D){   //flush

            fprintf(obj_fp,"T%06X",field);
            fprintf(obj_fp,"%02X",heap-field);
            for(int i =0 ; i < buffer_size ; i++){
                if(buffer[i] > 0xffff)
                  fprintf(obj_fp,"%06X",buffer[i]);
                else if(buffer[i] > 0xff)
                  fprintf(obj_fp,"%04X",buffer[i]);
                else
                  fprintf(obj_fp,"%02X",buffer[i]);
            }

            fprintf(obj_fp,"\n");
            buffer_size = 0;
            field = heap;
        }
    }


}
void M_push_to_obj(FILE* obj_fp,int loc){
    m_record[m_size++] = loc+1 - start;
}
void M_print_to_obj(FILE* obj_fp){
    int i;
    for( i = 0 ; i < m_size ; i++ )
      fprintf(obj_fp , "M%06X05\n",m_record[i]);
}
void E_print_to_obj(FILE* obj_fp){
    fprintf(obj_fp , "E%06X",start);
}


void print_to_lst(FILE* fp,int i,int loc,char *line,int code){
    fprintf(fp,"%3d %04X %-23s ",i*5,loc,line);
    if(code != -1){
        if(code > 0xffff)
          fprintf(fp," %06X",code);
        else if(code > 0xff)
          fprintf(fp," %04X",code);
        else
          fprintf(fp," %02X",code);
    }
    fprintf(fp,"\n");
}
int find_charater(char *line){
    char *ptr = NULL;
    char temp[24];
    int n,i;
    if(line[0] != '\'')
      return -1;
    ptr = strchr(line+1,'\'');
    if (ptr != NULL) 
      n = (int)(ptr - line - 1);
    else
      return -1;
    *ptr = '\0';
    strcpy(temp , &line[1]);
    strcpy(line , temp);

    return n;
}
void print_symbol_table(){
    int i;
    for(i=0; i<symbol_len; i++)
      printf("\t%s\t%X\n",symbol_table[i]->label,symbol_table[i]->loc);
}
