#ifndef LOADER_H
#define LOADER_H

void set_progaddr(int addr);
int get_progaddr();
void link_load_free();
int loader(FILE *first,FILE *second,FILE *third,int num);
#endif
