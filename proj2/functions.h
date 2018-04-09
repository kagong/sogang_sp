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

#endif
