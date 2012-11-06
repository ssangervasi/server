#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

struct node {
	int socket;
    struct node *next;
};
 

typedef struct {
	struct node head;

} linkedlist;
  
void list_insert(int socket, struct node **head);

void list_remove(int socket, struct node **head);
 
void list_clear(struct node *list);

