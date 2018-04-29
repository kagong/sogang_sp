#ifndef ASSEMBLE_H
#define ASSEMBLE_H


int assembler(char *file_name);
int assemble_pass_1(char* file_name);
int assemble_pass_2(char* file_name);
int parsing_line(char line[111], char label[8],char** opcode,char **arg1 , char **arg2);
void symbol_init();
void symbol_clear();
void symbol_free();
int Is_symbol(char* label);
int symbol_addr(char* );
void Insert_symbol(char *symbol, int loc_now);
int Is_Reg(char *);
int To_Reg(char *);
int Is_const(char *);
int To_const(char *);
int Is_addr(char *);
int Is_Raddr(int ,int);
int To_Raddr(int ,int);
int Is_Baddr(int ,int);
int To_Baddr(int ,int);
void H_print_to_obj(FILE* obj_fp);
void T_push_to_obj(FILE* obj_fp,int object_code,int);
void M_push_to_obj(FILE* obj_fp,int loc);
void M_print_to_obj(FILE* obj_fp);
void E_print_to_obj(FILE* obj_fp);
void print_to_lst(FILE* ,int ,int ,char* ,int );
int find_charater(char *line);
void print_symbol_table();

#endif
