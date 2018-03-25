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
void opcode_input();
int opcode(int, char**);
int opcodelist(int, char**);

#endif
