#include "linkedlist.h"
    
    
void list_insert(int socket, struct node **head) {
     struct node *newnode = malloc(sizeof(struct node));
     strncpy(newnode->socket, socket, 127);
     newnode->next = *head;
     *head = newnode;
	 (*head)->next = NULL;
}
 
void list_clear(struct node *list) {
    while (list != NULL) {	
        struct node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

void list_remove(int socket, struct node **head) {
    struct node *drop = *head;
	struct node *prev = *head;
	if(strcmp(drop->socket, socket) == 0){
		*head = (*head)->next;
		free(drop);
		return;
	}
	while((drop)!= NULL && strcmp(drop->socket, socket) != 0){
		prev = drop;		
		drop = drop->next;

	}
    if(drop!=NULL){
		prev->next = drop->next;
		free(drop);
	}
	return;
}

