#include "linkedlist.h"
    
    
void list_insert(int socket, struct node **tail) {
     struct node *newnode = malloc(sizeof(struct node));
     (*tail)->next = newnode;
     newnode->socket=socket;
	 newnode-> ip = NULL;
	 newnode-> port = -1;
     *tail = newnode;
	 (*tail)->next =NULL;
}
 
void list_clear(struct node *list) {
    while (list != NULL) {	
        struct node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

int list_remove( struct node **head, struct node **tail) {
    int sockval = ((*head)->next)->socket;
	struct node* holder = (*head)->next;
	(*head)->next = ((*head)->next)->next;
	if((*head)->next == NULL){
		*tail = *head;
	}
	free(holder->ip);
	free(holder);
	return sockval;
}

