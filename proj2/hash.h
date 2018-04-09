#ifndef HASH_H
#define HASH_H

void hash_init();
void push_hash_node(int, char [], int);
int hash_func(char*);
void hash_free();
int hash_search(char *,const char *);
void hash_traversal();

#endif
