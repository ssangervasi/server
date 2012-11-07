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
	struct node tail;
	pthread_mutex_t listlock;
	pthread_cond_t listempty;
} linkedlist;
  
void list_insert(int socket, struct node **tail);

int list_remove(struct node **head);
 
void list_clear(struct node *list);

