#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void functions_init();
void functions_free();
void history_pop();
void history_free();
void history_push();
int help(int, char**);
int dir(int, char**);
int quit(int, char**);
int history(int, char**);
int dump(int, char**);
int edit(int, char**);
int fill(int, char**);
int mem_store(int address,int size);
void mem_load(int address,int value,int size);
int reset(int, char**);
int Is_opcode(char *opcode);
int opcode_format(char *);
int opcode_code(char *);
void opcode_input();
int opcode(int, char**);
int opcodelist(int, char**);
int assemble(int argc, char** argv);
int type(int argc, char** argv);
int symbol(int argc, char** argv);
int call_progaddr(int argc, char** argv);
int call_loader(int argc, char** argv);
int call_run(int argc, char** argv);
int call_bp(int argc, char** argv);

#endif
