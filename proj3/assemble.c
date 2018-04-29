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

//pass1 과 pass2를 나눠 호출합니다.
int assembler(char *file_name){
    if(strcmp(&file_name[strlen(file_name)-4], ".asm") != 0 ){
        printf("Is is not .asm\n");
        return 0;
    }

    symbol_clear(); //symbol table 을 비움
    if(assemble_pass_1(file_name))
      if(assemble_pass_2(file_name))
          return 1;
    
    symbol_clear();     //정상 종료가 아닐경우
    return 0;
}

/*sic 머신의 pass_1 을 기반으로 예외처리를 추가하여 구현하였습니다.
  symbol table에 push 뿐만 아니라 각 명령어에 대한 loc 도 계산하여 중간 파일에 삽입 합니다.
  예외 처리를 통해 오류가 발견되면 0 반환 합니다.
  */
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
    if(ofp == NULL){        //intermediate file
        printf("file open error!\n");
        return 0;
    }
    for(i=0;i<111;i++) //inital
      line[i] = 0;


    while((result = fgets(line,111,ifp)) != NULL ) {
        //init
        num++;        //next line
        prev = LOCATER; //save locater
        opcode = arg1 = arg2 = NULL; 
        line[strlen(line)-1] = '\0';
        argc = parsing_line(line,label,&opcode,&arg1,&arg2);//line parsing 

        if(argc == 0 && opcode == NULL){ //space line
            fprintf(ofp,"%d\n",-1);     //not loc line -> -1
            continue;
        }
        if(strcmp(opcode,"END") == 0){  
            fprintf(ofp,"%d\n",-1);
            fprintf(ofp,"%d\n",-1);
            end = LOCATER;            
            break;
        }

        else if(strcmp(opcode,"START") == 0){
            if(arg1 == NULL){                 //시작 주소가 없음
                printf("Line%d : ",num);
                printf("pass1 opcode error! \n");
                error = 1;
                break;
            }
            strcpy(name,label);               //프로그램 네임
            start = strtol(arg1,&ptr,16);     //16 진수 변환
            HEX_CONVERT_CHECK(ptr,flag);
            if(flag){                         //변환 실패
                printf("Line%d : ",num);
                printf("It Isn't Hex\n");
                error = 1;
                break;
            }

            LOCATER = start;                  //시작주소 설정
            fprintf(ofp,"%05X\n",LOCATER);
        }
        else if(strcmp(opcode,"BASE") == 0||label[0] == '.'){   //나머지 지시자 처리
            fprintf(ofp,"%d\n",-1);
        }
        else{
            if(strlen(label) != 0){       
                if(!Is_symbol(label))         //symbol table 에 처음 등록 된다면?
                  Insert_symbol(label,LOCATER);
                else if(label[0] == '.') 
                  continue;
                else{                         //label 이 유효 하지 않음
                    printf("Line%d : ",num);
                    printf("pass1 label error!\n");
                    error = 1;
                    break;
                }
            }

            if(opcode != NULL){               
                i = (opcode[0] == '+') ? 1:0; //4형식 일 경우 opcode의 시작주소를 정함
                if(Is_opcode(&opcode[i])){
                    LOCATER += opcode_format(&opcode[i]);
                    if(i == 1)                // 4 형식은 locater가 1이 증가
                      LOCATER+=1;
                }
                else {                  
                    if(arg1 == NULL){         //변수의 초기값이 업는 경우 
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
                        LOCATER += strtol(arg1,&ptr,10)*3;
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

                    else if(strcmp(opcode,"BYTE") == 0){//BYTE의 경우 초기화 값에 따라 탄력적으로 길이를 증가
                        if(arg1[0] == 'C' || arg1[0] == 'X'){
                            int cnt;
                            cnt = find_charater(&arg1[1]);  //초기값의 갯수를 샘

                            if(cnt == -1){
                                printf("Line%d : ",num);
                                printf("pass1 BYTE count error!\n");
                                error = 1;
                                break;
                            }

                            else if(arg1[0] != 'X')     //16 진수 형식이 아니라면
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
        remove(LOCATE_TEMP_NAME);
        return 0;
    }
    else if(error){//에러일 경우 임시 파일 삭제

      remove(LOCATE_TEMP_NAME);
      return 0;
    }
    return 1;
}
int assemble_pass_2(char* file_name){
    char copy[111];
    char line[111], label[8] ,*opcode ,*arg1 ,*arg2, *ptr=NULL;
    FILE *ifp = NULL,*locate_fp=NULL,*obj_fp = NULL, *lst_fp = NULL;
    int i,prev, LOCATER = 0,format,hex,opcode_start = 0,object_code=0,arg_start=0,target_addr=0,flag = 0;
    int B=0, argc, error = 0;
    //prev - 직전 명령어의 주소를 저장
    ifp = fopen(file_name,"r");                   //ASM 파일
    locate_fp = fopen(LOCATE_TEMP_NAME,"r");      //임시파일
    strcpy(&file_name[strlen(file_name)-3], "lst");
    lst_fp = fopen (file_name,"w");               //LST 파일
    strcpy(&file_name[strlen(file_name)-3], "obj");
    obj_fp = fopen (file_name,"w");               //OBJ 파일

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
        opcode = arg1 = arg2 = NULL;            //초기화
        line[strlen(line)-1] = '\0';  
        strcpy(copy,line);                      //line은 파싱을 하므로 이를 임시로 다른곳에 저장
        object_code = 0;
        flag = 0;
        ++i;
        fscanf(locate_fp,"%X",&LOCATER);        //구조적으로 다음 line의 locate를 불러옴
        argc = parsing_line(line,label,&opcode,&arg1,&arg2);

        if(argc == -1) {                        //파싱 실패
            printf("Line%d : ",i);
            printf("pass2 line parsing error!\n");
            error = 1;
            break;
        } 

        if(LOCATER == -1){                      //현재가 빈공간 이나 지시자 주석인 경우
            fprintf(lst_fp,"%3d %4s %s\n",i*5,"    ",copy);

            if(opcode == NULL)                  //빈공간
              continue;
            else if(strcmp(opcode,"END") == 0){
                T_push_to_obj(obj_fp,-1,LOCATER);   //object 파일에 출력을 마무리함
                M_print_to_obj(obj_fp);
                E_print_to_obj(obj_fp);
                break;
            }
            else if(strcmp(opcode,"BASE") == 0){    //base R 주어진 값으로 바꿈
                if(arg1 != NULL && Is_symbol(arg1))
                  B = symbol_addr(arg1);
            }

            else if(strcmp(opcode,"NOBASE") == 0){  //base R 을 초기화함
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


        if(strcmp(opcode,"START") == 0){      //start 일 경우 obj 에 출력 초기화, 현재 locater를 prev 에 저장함
            H_print_to_obj(obj_fp);
            print_to_lst(lst_fp,i,LOCATER,copy,-1);
            prev = LOCATER;
            continue;
        }

        if(opcode[0] == '+')      //4형식일 경우 opcode 의 시작주소를 조정
          opcode_start = 1;
        else
          opcode_start = 0;
        if(Is_opcode(&opcode[opcode_start])){ 

            format = opcode_format(&opcode[opcode_start]);
            hex = opcode_code(&opcode[opcode_start]);

            if(format == 1){      //1형식
                if(arg1 != NULL || arg2 != NULL){
                    printf("Line%d : ",i);
                    printf("pass2 extra arguments error!\n");
                    error = 1;
                    break;
                }

                object_code = hex;
            }
            else if(format == 2){   //2형식
                object_code = hex <<8;

                if(Is_Reg(arg1))        //레지스터인지 확인
                  object_code |= To_Reg(arg1) << 4;

                else if(Is_const(arg1)) //상수인지 확인
                  object_code |= To_const(arg1) << 4;

                if(Is_Reg(arg2))
                  object_code |= To_Reg(arg2);
                else if(Is_const(arg2))
                  object_code |= To_const(arg2);

            }
            else if(format == 3){   //3형식
                if(opcode_start == 1){  //4형식
                    object_code = hex << 24;
                    object_code |=  MODE_EXTEND  << 8;
                    if(arg1 != NULL && arg2 == NULL ){

                        if(Is_symbol(arg1)){            //default setting
                            M_push_to_obj(obj_fp,prev);
                            object_code |= symbol_addr(arg1);
                            object_code |=  MODE_SIMPLE  << 8;
                        }
                        else if(Is_const(arg1)){        //상수형 세팅
                            object_code |= To_const(arg1);
                            object_code |=  MODE_IMMEDIATE  << 8;
                        }
                        else if(Is_addr(arg1)){         //indirect addressing setting
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
                else{ //3형식
                    object_code = hex << 16;
                    if(argc == 2){  //인덱스 모드              
                        if(strcmp(&arg2[0],"X") != 0){
                            printf("Line%d : ",i);
                            printf("pass2 Index Register error!\n");
                            error = 1;
                            break;
                        }
                        object_code |= (MODE_INDEX | MODE_SIMPLE);
                    }
                    else if(argc != 1){   //3형식에 인자가 없다면? -> RSUB를 제외하고 예외처리
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

                        if(Is_Raddr(LOCATER,target_addr))                                     // PC addressing
                          object_code |= (MODE_PC | (0xfff &To_Raddr(LOCATER,target_addr))); // 음수인 경우를 대비하여 마스크를 씨움

                        else if(Is_Baddr(B,target_addr))                                    //BASE addressing
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
                      object_code|= arg1[1+iter]<<(cnt-iter-1)*8;             //OBJECT CODE 계산
                }
                else{                                                       //character 인경우
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
            else{         //BYTE 뒤의 초기화값이 16진수도 문자도 아닌경우
                printf("Line%d : ",i);
                printf("pass2 BYTE error!\n"); 
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
        print_to_lst(lst_fp,i,prev,copy,object_code);
        prev = LOCATER;
    }

    if(fclose(ifp) || fclose(locate_fp) || fclose(lst_fp) || fclose(obj_fp)){
        printf("Error on file close\n");
        return 0;
    }

    remove(LOCATE_TEMP_NAME);//임시 파일 삭제
    
    if(error)
      return 0;
    file_name[strlen(file_name)-4] = '\0';
    printf("output file: [%s.lst] [%s.obj]\n",file_name, file_name);
    return 1;
}

int parsing_line(char line[111], char label[8],char** opcode,char **arg1 , char **arg2){
    int i=0;
    char temp[8];
    char *ptr = NULL;
    char *argv[3];
    int argc;

    ptr = strchr(line,'.');                         //주석일 경우
    if(ptr != NULL){
        *ptr = '\0';
    }

    for(i=0;i<7;i++)                                
      label[i] = line[i];
    label[7] = '\0';
    ptr = label;
    trim(ptr);                                      //"    string    " 을 "string" 으로 바꿈
    strcpy(temp,ptr);
    strcpy(label,temp);

    if(strlen(line) < 8)
      return 0;

    strcpy(line,&line[7]);
    command_nomalization(line);                     //parse_arg 를 위해 일반화 진행
    argc = parse_arg(line, argv);                   //line을 parsing 함

    *opcode = line;                                 //line엔 opcode만 남음
    *arg1 = argc > 0 ? argv[0] : NULL;              // arg1이 생성되면 대입
    *arg2 = argc > 1 ? argv[1] : NULL;              // arg2가 생성되면 대입
    if(argc == 1 && strlen(*arg1) == 0){            // arg1가 공백일시 제외
        arg1 = NULL;
        --argc;
    }
    else if(argc == 2 && strlen(*arg2) == 0){       //arg2가 공백일시 제외
        arg2 = NULL;
        --argc;
    }
    return argc;
}
void symbol_init(){                               //symbol table을 초기화
    symbol_len = 0;
    symbol_max = 100;
    symbol_table = (symbol_node**)malloc(symbol_max * sizeof(symbol_node*));
}
void symbol_clear(){                              //symbol을 다음 assemble 를 위해 초기화 함
    int i;
    for(i=0;i<symbol_len;i++){
        free(symbol_table[i]);
        symbol_table[i] = NULL;
    }
    symbol_len = 0;
}
void symbol_free(){                               //프로그램이 끝나 할당 해제함
    int i;
    for(i=0;i<symbol_len;i++)
      free(symbol_table[i]);

    free(symbol_table);
}
int Is_symbol(char* label){                       //해당 라벨이symbol table에 존재하는지 확인
    int i;
    for(i = 0 ; i < symbol_len ; i ++){
        if(strcmp(symbol_table[i]->label,label) == 0)
          return 1;
    }
    return 0;
}
int symbol_addr(char* symbol){                    //해당 라벨의 address를 반환함
    int i;
    for(i = 0 ; i < symbol_len ; i ++){
        if(strcmp(symbol_table[i]->label,symbol) == 0)
          return symbol_table[i] -> loc;
    }
    return -1;
}
void Insert_symbol(char *symbol, int loc_now){    //symbol table에 새로운 symbol을 대입
    int i, index;
    symbol_node* new = NULL;
    new = (symbol_node*)malloc(sizeof(symbol_node));
    new -> loc = loc_now;
    strcpy(new -> label,symbol);
    if(symbol_len + 1 > symbol_max){              //symbol table이 작다면
        symbol_max *= 2;
        symbol_table =(symbol_node**)realloc(symbol_table,symbol_max*(sizeof(symbol_node*)));
    }
    for(i=0;i<symbol_len;i++){
        if(strcmp(symbol_table[i]->label,symbol) > 0)
          break;
    }

    index = i;                                    //symbol을 알파벳 순으로 정렬
    for(i=symbol_len-1 ; i >= index ;i--){
        symbol_table[i+1] = symbol_table[i];
    }
    ++symbol_len;
    symbol_table[index] = new;
}
int Is_Reg(char *argument){                       //주어진 argument가 register인지 확인
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

//주어진 argument에 해당하는 register의 번호를 반환
//Is_Reg와 같이 써서 확정적으로 argument 는 register
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

//주어진 argument가 #로 시작하는 상수인지 확인하고 #다음이 상수인지 symbol 인지 확인하고 맞다면 1을 반환 아니라면 0을 반환함
int Is_const(char *argument){    
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

// 주어진 argument에 해당하는 상수를 반환함
// Is_const와 함께 쓰이므로 argument는 무조건 상수임
int To_const(char *argument){
    int n;
    if(Is_symbol(&argument[1]))
      return symbol_addr(&argument[1]);

    return strtol(&argument[1],NULL,10);
}

//argument가 @ + symbol의 형태인지 확인하고 맞다면 1을 반환함
int Is_addr(char *argument){

    if(argument[0] != '@')
      return 0;

    else if(Is_symbol(&argument[1]))
      return 1;

    return 0;
}

//해당 주소의 표현 방법이 pc relative addressing 인지 확인
int Is_Raddr(int loc,int addr){
    if(-2048 <=addr - loc && addr - loc <= 2047){
        return 1;
    }
    else
      return 0;
} 

//해당 주소의 표현 방법이 pc relative addressing 에 해당하는 주소로 반환
int To_Raddr(int loc,int addr){
    int n = addr - loc;
    return n;

}

//해당 주소의 표현 방법이 base addressing 인지 확인
int Is_Baddr(int B,int addr){
    if(0 <= addr - B && addr - B <= 4095){
        return 1;
    }
    else
      return 0;
}

//해당 주소의 표현 방법이 base addressing 에 해당하는 주소를 반환
int To_Baddr(int B,int addr){
    return addr - B;
}

//오브젝트 파일의 헤더를 출력
void H_print_to_obj(FILE* obj_fp){

    fprintf(obj_fp,"H%-6s%06X%06X\n",name,start,end-start);
}

/*
   오브젝트의 text 부분을 출력하는 함수
   내부의 버퍼에 출력값들을 저장해 놓고 적당한 시점에 flush를 진행
   */
void T_push_to_obj(FILE* obj_fp,int object_code, int loc){
    static int buffer[30];
    static int buffer_size =0;
    static int flag = 0;              //flush flag
    static int field=0 ,heap=0; 

    if(field < start)
      field = start;
    if(object_code != -1){            //object code가 정상적인 상태일때
        buffer[buffer_size++] = object_code;
        flag = 0;
        heap = loc;
    }
    else if(flag)
      field = loc;

    else
      flag = 1;

    if(buffer_size != 0) {
        if(flag ||  heap-field >=0x1d){               //flush

            fprintf(obj_fp,"T%06X",field);
            fprintf(obj_fp,"%02X",heap-field);
            for(int i =0 ; i < buffer_size ; i++){
                if(buffer[i] > 0xffff)                //출력양식 조정
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

//modified record 에 출력값을 push함
void M_push_to_obj(FILE* obj_fp,int loc){
    m_record[m_size++] = loc+1 - start;
}

//record에 있는 값들을 출력함
void M_print_to_obj(FILE* obj_fp){
    int i;
    for( i = 0 ; i < m_size ; i++ )
      fprintf(obj_fp , "M%06X05\n",m_record[i]);
}

//end code를 출력
void E_print_to_obj(FILE* obj_fp){
    fprintf(obj_fp , "E%06X\n",start);
}

//lst 파일에 출력을 진행
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
/*
   line하나를 입력받아 그곳에 '' 안에 존재하는 값들이 몇개 인지 확인하고
   이를 반환.
   */
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

//symbol table이 비어있다면 비어있다를 출력.
// 비어있지 않다면 않의 값을 출력
void print_symbol_table(){
    int i;
    for(i=0; i<symbol_len; i++)
      printf("\t%s\t%X\n",symbol_table[i]->label,symbol_table[i]->loc);
    if(symbol_len == 0)
      printf("symbol table is empty!!\n");
}
