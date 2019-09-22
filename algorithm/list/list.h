#ifndef LIST_H
#define LIST_H

struct List{
	int data;
	struct List *next;
};
typedef struct List *jlink;

void list_insert(jlink *list, int idx, int data);

void list_insert_head(jlink *list, int data);

void list_insert_tail(jlink *list, int data);
#endif
