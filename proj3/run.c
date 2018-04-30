#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"functions.h"

#define BYTE 1
#define BYTE_BIT 8

#define LT 1
#define EQ 2
#define GT 3

int A; int X; int L;
int B; int S; int T;
int F; int SW; int PC;

unsigned int bp_table[3000];
int bp_max = 0;

void format_2_functions(unsigned int instruction);
void ADDR(int* reg1,int *reg2);
void CLEAR(int* reg1,int *reg2);
void COMPR(int* reg1,int *reg2);
void DIVR(int* reg1,int *reg2);
void MULR(int* reg1,int *reg2);
void RMO(int* reg1,int *reg2);
void SHIFTL(int* reg1,int *reg2);
void SUBR(int* reg1,int *reg2);
void TIXR(int* reg1,int *reg2);
void format_3_4_functions(unsigned int instruction,int format);
void ADD(int address,int ni_flag);
void COMP(int address,int ni_flag);
void DIV(int address,int ni_flag);
void J(int address,int ni_flag);
void JEQ(int address,int ni_flag);
void JGT(int address,int ni_flag);
void JLT(int address,int ni_flag);
void JSUB(int address,int ni_flag);
void LDA(int address,int ni_flag);
void LDB(int address,int ni_flag);
void LDCH(int address,int ni_flag);
void LDL(int address,int ni_flag);
void LDS(int address,int ni_flag);
void LDT(int address,int ni_flag);
void LDX(int address,int ni_flag);
void MUL(int address,int ni_flag);
void RSUB(int address,int ni_flag);
void STA(int address,int ni_flag);
void STB(int address,int ni_flag);
void STCH(int address,int ni_flag);
void STL(int address,int ni_flag);
void STS(int address,int ni_flag);
void STSW(int address,int ni_flag);
void STT(int address,int ni_flag);
void STX(int address,int ni_flag);
void SUB(int address,int ni_flag);
void TD(int address,int ni_flag);
void TIX(int address,int ni_flag);
void register_dump(int flag,int addr);
void register_init(){
    A = X = L = B = S = T = F = SW = 0;
}
int run(int addr){
    int i,format,error_flag = 0,prev;
    unsigned int target = 0;
    PC = addr;
    //fetch
    //rsub 가 0으로 jump하면 -1로 세팅하자ㅇ좀더 ㅇ작업 해주세
    while(PC != -1){
        prev = PC;
        //fetch
        target = (unsigned int)mem_store(PC,BYTE);
        //decode & operand fetch
        switch(target & 0xF0){
            case 0XC0:      //format 1
            case 0XF0:
                PC+=1;
                format = 1;
                break;
            case 0X90:      //format 2
            case 0XA0:
            case 0XB0:
                PC+=2;
                format = 2;
                target <<= BYTE_BIT;
                target |= (unsigned int)mem_store(PC+BYTE ,BYTE);
                break;
            default:        //format 3 or 4
                target = (target << 2*BYTE_BIT ) | (unsigned int)mem_store(PC+BYTE ,2*BYTE);
                if(target & 0x001000 == 0){
                    format = 3;
                    PC+=3;
                }
                else{
                    format = 4;
                    PC += 4;
                    target <<= BYTE_BIT;
                    target |= (unsigned int)mem_store(PC+3*BYTE ,BYTE);
                }
                break;
        }

        //excute
        switch(format){
            case 1:
                break;
            case 2:
                format_2_functions(target);
                break;
            case 3:
            case 4:
                format_3_4_functions(target,format);
                break;
            default:
                error_flag = 1;
                goto done;
        }
        for(i = 0 ; i < bp_max ; i++){
            if(prev <= bp_table[i] && bp_table[i] < PC){
                register_dump(0,bp_table[i]);
                return PC;
            }
        }

    }
done:
    if(error_flag){
        printf("Error! In run time\n");
        return -1;
    }
    register_dump(1,0);
    return 0;

}
void format_2_functions(unsigned int instruction){
    int *reg1,*reg2;
    switch((instruction & 0x00F0) >> 4){
        case 0: reg1 = &A; break;
        case 1: reg1 = &X; break;
        case 2: reg1 = &L; break;
        case 3: reg1 = &B; break;
        case 4: reg1 = &S; break;
        case 5: reg1 = &T; break;
        case 6: reg1 = &F; break;
        case 7: reg1 = &SW; break;
        case 8: reg1 = &PC;break;
        default: 
                printf("error! unknown register\n");
                return ;
    }
    switch(instruction & 0x000F){
        case 0: reg2 = &A; break;
        case 1: reg2 = &X; break;
        case 2: reg2 = &L; break;
        case 3: reg2 = &B; break;
        case 4: reg2 = &S; break;
        case 5: reg2 = &T; break;
        case 6: reg2 = &F; break;
        case 7: reg2 = &SW; break;
        case 8: reg2 = &PC; break;
        default: 
                printf("error! unknown register\n");
                return ;
    }
    switch((instruction & 0xFF00) >> 8){
        case 0x90: ADDR(reg1,reg2); break;
        case 0xB4: CLEAR(reg1,reg2); break;
        case 0xA0: COMPR(reg1,reg2); break;
        case 0x9C: DIVR(reg1,reg2); break;
        case 0x98: MULR(reg1,reg2); break;
        case 0xAC: RMO(reg1,reg2); break;
        case 0xA4: SHIFTL(reg1,reg2); break;
        case 0x94: SUBR(reg1,reg2); break;
        case 0xB8: TIXR(reg1,reg2); break;
        default: 
                printf("error! unknown function\n");
                return ;
    }
}
void ADDR(int* reg1,int *reg2){
    *reg2 += *reg1;
}
void CLEAR(int* reg1,int *reg2){
    *reg1 = 0;
}
void COMPR(int* reg1,int *reg2){
    if(*reg1 < *reg2)
        SW = LT;
    else if(*reg1 == *reg2)
        SW = EQ;
    else
        SW = GT;
}
void DIVR(int* reg1,int *reg2){
    *reg2 /= *reg1 != 0 ? *reg1 : 1;
}
void MULR(int* reg1,int *reg2){
    *reg2 *= *reg1;
}
void RMO(int* reg1,int *reg2){
    *reg2 = *reg1;
}
void SHIFTL(int* reg1,int *reg2){
    int n = *reg2+1,i;
    unsigned int temp = (unsigned int)*reg1, msb;
    for(i = 0 ; i < n ; i++){
        msb = (temp & 0x80000000) >> 31;
        temp = (temp << 1) | (msb & 0x1);
    }
}
void SUBR(int* reg1,int *reg2){
    *reg2 -= *reg1;
}
void TIXR(int* reg1,int *reg2){
    ++X;
    if(X < *reg1)
        SW = LT;
    else if( X == *reg1)
        SW = EQ;
    else 
        SW = GT;

}

void format_3_4_functions(unsigned int instruction,int format){
    int opcode,ni_flag, x_flag, bp_flag, address;
    if(format == 3){
        opcode = (instruction & 0x00FC0000) >> 16;
        ni_flag = (instruction & 0x00030000) >> 16;
        x_flag = (instruction & 0x00008000) >> 15;
        bp_flag = (instruction & 0x00006000) >> 13;
        address = instruction & 0x00000FFF;
    }
    else{
        opcode = (instruction &  0xFC000000) >> 24;
        ni_flag = (instruction & 0x03000000) >> 24;
        x_flag = (instruction &  0x00800000) >> 23;
        bp_flag = (instruction &  0x00600000) >> 21;
        address = instruction &  0x000FFFFF;
    }
    if(bp_flag == 0x01)//pc relative addressing
        address += PC;
    else if(bp_flag == 0x02) //base addressing
        address += B;

    if(x_flag)
        address += X;

    if(format == 3)
        address &= 0x00000FFF;
    else
        address &= 0x000FFFFF;
    switch(opcode){
        case 0X18: ADD(address,ni_flag); break;
        case 0X28: COMP(address,ni_flag); break;
        case 0X24: DIV(address,ni_flag); break;
        case 0X3C: J(address,ni_flag); break;
        case 0X30: JEQ(address,ni_flag); break;
        case 0X34: JGT(address,ni_flag); break;
        case 0X38: JLT(address,ni_flag); break;
        case 0X48: JSUB(address,ni_flag); break;
        case 0X00: LDA(address,ni_flag); break;
        case 0X68: LDB(address,ni_flag); break;
        case 0X50: LDCH(address,ni_flag); break;
        case 0X08: LDL(address,ni_flag); break;
        case 0X6C: LDS(address,ni_flag); break;
        case 0X74: LDT(address,ni_flag); break;
        case 0X04: LDX(address,ni_flag); break;
        case 0X20: MUL(address,ni_flag); break;
        case 0X4C: RSUB(address,ni_flag); break;
        case 0X0C: STA(address,ni_flag); break;
        case 0X78: STB(address,ni_flag); break;
        case 0X54: STCH(address,ni_flag); break;
        case 0X14: STL(address,ni_flag); break;
        case 0X7C: STS(address,ni_flag); break;
        case 0XE8: STSW(address,ni_flag); break;
        case 0X84: STT(address,ni_flag); break;
        case 0X10: STX(address,ni_flag); break;
        case 0X1C: SUB(address,ni_flag); break;
        case 0XE0: TD(address,ni_flag); break;
        case 0X2C: TIX(address,ni_flag); break;
    }
}
void ADD(int address,int ni_flag){

    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        A += mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        A += address;
    }
    else{//indirect
        A += mem_store(mem_store(address,3),3);
    }
}
void COMP(int address,int ni_flag){
    int target;
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        target = mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        target = address;
    }
    else{//indirect
        target = mem_store(mem_store(address,3),3);
    }
    if(A < target)
        SW = LT;
    else if(A == target)
        SW = EQ;
    else 
        SW = GT;
}
void DIV(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        A /= mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        A /= address;
    }
    else{//indirect
        A /= mem_store(mem_store(address,3),3);
    }
}
void J(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        PC = address;
    }
    else{//indirect
        PC = mem_store(address,3);
    }
}
void JEQ(int address,int ni_flag){
    if(SW != EQ)
        return ;
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        PC = address;
    }
    else{//indirect
        PC = mem_store(address,3);
    }
}
void JGT(int address,int ni_flag){
    if(SW != GT)
        return ;
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        PC = address;
    }
    else{//indirect
        PC = mem_store(address,3);
    }
}
void JLT(int address,int ni_flag){
    if(SW != LT)
        return ;
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        PC = address;
    }
    else{//indirect
        PC = mem_store(address,3);
    }
}
void JSUB(int address,int ni_flag){
    L = PC;
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        PC = address;
    }
    else{//indirect
        PC = mem_store(address,3);
    }
}
void LDA(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        A += mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        A += address;
    }
    else{//indirect
        A += mem_store(mem_store(address,3),3);
    }
}
void LDB(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        B += mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        B += address;
    }
    else{//indirect
        B += mem_store(mem_store(address,3),3);
    }
}
void LDCH(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        A += mem_store(address,1);
    }
    else if(ni_flag == 0x1){//immediate
        A += address;
    }
    else{//indirect
        A += mem_store(mem_store(address,3),1);
    }
}
void LDL(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        L += mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        L += address;
    }
    else{//indirect
        L += mem_store(mem_store(address,3),3);
    }
}
void LDS(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        S += mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        S += address;
    }
    else{//indirect
        S += mem_store(mem_store(address,3),3);
    }
}
void LDT(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        T += mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        T += address;
    }
    else{//indirect
        T += mem_store(mem_store(address,3),3);
    }
}
void LDX(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        X += mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        X += address;
    }
    else{//indirect
        X += mem_store(mem_store(address,3),3);
    }
}
void MUL(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        A *= mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        A *= address;
    }
    else{//indirect
        A *= mem_store(mem_store(address,3),3);
    }
}
void RSUB(int address,int ni_flag){
    PC = L == 0 ? -1 : L;
}
void STA(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        mem_load(address,A,3);
    }
    else{//indirect
        mem_load(mem_store(address,3),A,3);
    }
}
void STB(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        mem_load(address,B,3);
    }
    else{//indirect
        mem_load(mem_store(address,3),B,3);
    }
}
void STCH(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        mem_load(address,A,1);
    }
    else{//indirect
        mem_load(mem_store(address,3),A,1);
    }
}
void STL(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        mem_load(address,L,3);
    }
    else{//indirect
        mem_load(mem_store(address,3),L,3);
    }
}
void STS(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        mem_load(address,S,3);
    }
    else{//indirect
        mem_load(mem_store(address,3),S,3);
    }
}
void STSW(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        mem_load(address,SW,3);
    }
    else{//indirect
        mem_load(mem_store(address,3),SW,3);
    }
}
void STT(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        mem_load(address,T,3);
    }
    else{//indirect
        mem_load(mem_store(address,3),T,3);
    }
}
void STX(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3 || ni_flag == 0x1){//simple immediate
        mem_load(address,X,3);
    }
    else{//indirect
        mem_load(mem_store(address,3),X,3);
    }
}
void SUB(int address,int ni_flag){
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        A -= mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        A -= address;
    }
    else{//indirect
        A -= mem_store(mem_store(address,3),3);
    }
}
void TD(int address,int ni_flag){
    SW = LT;
}
void TIX(int address,int ni_flag){
    X++;
    int target;
    if(ni_flag == 0x00 || ni_flag == 0x3){//simple
        target = mem_store(address,3);
    }
    else if(ni_flag == 0x1){//immediate
        target = address;
    }
    else{//indirect
        target = mem_store(mem_store(address,3),3);
    }
    if(X < target)
        SW = LT;
    else if( X == target)
        SW = EQ;
    else
        SW = GT;
}
int bp(char *argument){
    int i;
    int hex;
    char *ptr;
    if(argument == NULL){
        printf("\tbreakpoint\n");
        printf("\t----------\n");
        for(i = 0 ; i < bp_max ; i++)
            printf("\t%04X\n",bp_table[i]);
    }
    else if(strcmp(argument,"clear") == 0){
        bp_max = 0;
        printf("\t[ok] clear all breakpoints\n");
    }
    else{
        hex = strtol(argument,&ptr,16);
        bp_table[bp_max++] = hex;
    }
}
void register_dump(int flag,int addr){
    printf("\t\t A : %-6X X : %-6X\n",A&0X00FFFFFF,X&0X00FFFFFF);
    printf("\t\t L : %-6X PC: %-6X\n",L&0X00FFFFFF,PC&0X00FFFFFF);
    printf("\t\t B : %-6X S : %-6X\n",B&0X00FFFFFF,S&0X00FFFFFF);
    printf("\t\t T : %-6X\n",T&0X00FFFFFF);
    if(flag)
        printf("End Program\n");
    else
        printf("Stop at checkpoint[%X]\n",addr);
}

