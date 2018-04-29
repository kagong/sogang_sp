#ifndef LOADER_H
#define LOADER_H

void set_progaddr(int addr);
void link_load_free();
int loader(FILE *first,FILE *second,FILE *third,int num);
int run();
int bp(char *);
#endif
