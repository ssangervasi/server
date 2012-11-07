#include "linkedlist.h"
    
    
void list_insert(int socket, struct node **tail) {
     struct node *newnode = malloc(sizeof(struct node));
     newnode->next = *tail;
     newnode->socket=socket;
     *tail = newnode;
}
 
void list_clear(struct node *list) {
    while (list != NULL) {	
        struct node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

int list_remove( struct node **head) {
    int sockval = (**head)->socket;
    *head = NUll;
	return sockval;
}

