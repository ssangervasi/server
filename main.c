#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "network.h"
#include "linkedlist.h"

//NICKIE AND SEBASTIAN DID THIS.
//THERE ARE PROBLEMS WITH THE PYTHON WHE WE SEND A 404 BACK TO IT. OTHERWISE IT WORKS GREAT.
//I BLAME THE PYTHON PROGRAM FOR ANY PROBLEMS.


// global variable; can't be avoided because
// of asynchronous signal interaction
int still_running = TRUE;
void signal_handler(int sig) {
    still_running = FALSE;
}

FILE* logfile = NULL;
pthread_mutex_t* loglock= NULL;

void usage(const char *progname) {
    fprintf(stderr, "usage: %s [-p port] [-t numthreads]\n", progname);
    fprintf(stderr, "\tport number defaults to 3000 if not specified.\n");
    fprintf(stderr, "\tnumber of threads is 1 by default.\n");
    exit(0);
}

void* sock_consume(void *arg){ // NOTE: arg will be a pointer to the linked list of available sockets.
	int newsock = 0;
	linkedlist* socks = (linkedlist*)(arg);
	while(still_running){
		pthread_mutex_lock(socks->listlock);
		while((socks->tail)->socket == -1 && still_running == TRUE){
			pthread_cond_wait(socks->listempty, socks->listlock);
		}
		if(still_running ==FALSE){
			break;
		}
		char* ip  = strdup((((socks->head)->next)->ip));
		int port = (((socks->head)->next)->port);
		newsock = list_remove(&(socks->head), &(socks->tail));
		pthread_mutex_unlock(socks->listlock);

		char* buffer = malloc(sizeof(char)*1024);
		if(getrequest(newsock,buffer,1024)==0){
			char* filename = malloc(sizeof(char)*strlen(buffer));
			if(buffer[0] == '/') {
				strcpy(filename, (const char*)(buffer + 1));
				//printf("_%s_\n_%s_\n",buffer, filename);		
			} else{
				strcpy(filename, (const char*)(buffer));
			}			
			int endsize = 0;
			int status = 200;
			struct stat *finfo = malloc(sizeof(struct stat));
			if(0 == stat((const char*)filename, finfo)){
				int size = (int)(finfo->st_size);
				
				char* header = malloc(sizeof(int)*(strlen(HTTP_200)));
				sprintf(header, HTTP_200, size);
				senddata(newsock, (const char*) header, strlen(header));
				
				int fdesc = open((const char*) filename, O_SYNC); 
				char* buf = malloc(sizeof(char)*1024);
				buf[1023] = '\0';
				int moredata = read(fdesc, (void*)buf, 1024);
				for(; moredata>0; moredata = read(fdesc, (void*)buf, 1023)){
					senddata(newsock, (const char*)buf, moredata);
				}
				free(buf);
				close(fdesc);
				endsize = sizeof(char)*strlen(header) + size;
				free(header);
				
			}else{
				char* tosend = strdup(HTTP_404);
				senddata(newsock, (const char*) tosend, sizeof(char)*strlen(tosend));
				free(tosend);
				endsize = sizeof(char)*strlen(HTTP_404);
				status = 404;
			}
			//Do the logging of stuff to the file.	
			
			time_t start;
			struct tm actual;
			char hold[80];
			time(&start);
			actual = *localtime(&start);
			strftime(hold, sizeof(hold), "%a %Y-%m-%d %H:%M:%S %Z", &actual);
			char* logstr = malloc(sizeof(char)*100);
			sprintf(logstr, LOG_STR, ip, port, hold, filename, status, endsize);
			printf("__%s__\n", logstr);

			pthread_mutex_lock(loglock);
			//write(logfile, (const void*) logstr, sizeof(char)*strlen(logstr));
			fprintf(logfile, (const char*) logstr);			
			pthread_mutex_unlock(loglock);
			free(logstr);
			free(ip);
			//
			free(finfo);
			free(filename);
		}
		free(buffer);	
	}
	return NULL;
}

void runserver(int numthreads, unsigned short serverport) {
    //////////////////////////////////////////////////

	linkedlist* socks = malloc(sizeof(linkedlist));
	socks-> head = malloc(sizeof(struct node));
	(socks->head)->socket = -1;
	socks->tail = socks->head;
	socks->listlock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(socks->listlock, NULL);
	socks->listempty = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(socks->listempty,NULL);

	//Getting the log file ready
	//logfile = malloc(sizeof(FILE));
	logfile = fopen((const char*) LOG,"a");
	loglock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(loglock, NULL);	
    //
    // create your pool of threads here
	if(numthreads<1){
		numthreads = 1;
	}
	pthread_t** threads = malloc(sizeof(pthread_t*)*numthreads);
	int i = 0;
	for(; i < numthreads; i++){
		threads[i] = malloc(sizeof(pthread_t));
		pthread_create(threads[i], NULL, sock_consume, (void*)socks);
	}

    //////////////////////////////////////////////////
	
    int main_socket = prepare_server_socket(serverport);
    if (main_socket < 0) {
        exit(-1);
    }
    signal(SIGINT, signal_handler);

    struct sockaddr_in client_address;
    socklen_t addr_len;

    fprintf(stderr, "Server listening on port %d.  Going into request loop.\n", serverport);
    while (still_running) {
        struct pollfd pfd = {main_socket, POLLIN};
        int prv = poll(&pfd, 1, 10000); //should be 10000

        if (prv == 0) {				
            still_running = FALSE;
			pthread_cond_broadcast(socks->listempty);
            break;
        } else if (prv < 0) {
            PRINT_ERROR("poll");
            still_running = FALSE;
			pthread_cond_broadcast(socks->listempty);
            break;
        }
        addr_len = sizeof(client_address);
        memset(&client_address, 0, addr_len);

        int new_sock = accept(main_socket, (struct sockaddr *)&client_address, &addr_len);
        if (new_sock > 0) {
            
            fprintf(stderr, "Got connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

           ////////////////////////////////////////////////////////
           /* You got a new connection.  Hand the connection off
            * to one of the threads in the pool to process the
            * request.
            *
            * Don't forget to close the socket (in the worker thread)
            * when you're done.
            */
           ////////////////////////////////////////////////////////

			pthread_mutex_lock(socks-> listlock);
			list_insert(new_sock, &socks->tail);
			(socks->tail)-> ip = strdup(inet_ntoa(client_address.sin_addr)); //malloc! The free is taken care of in list_remove.
			(socks->tail)-> port = ntohs(client_address.sin_port);
			pthread_cond_broadcast(socks->listempty);
			pthread_mutex_unlock(socks->listlock);

        }
    }
	//printf("smee_\n");
	
	i = 0;
	for(; i < numthreads; i++){
		pthread_join(*(threads[i]), NULL);
		free(threads[i]);	
	}
	//printf("smoo\n");
	free(threads);

	free(socks->listlock);
	free(socks->listempty);
	free(socks->head);
	free(socks);
	
    fprintf(stderr, "Server shutting down.\n");
	pthread_mutex_lock(loglock);
	fclose(logfile);
	//free(logfile);
	pthread_mutex_unlock(loglock);
	free(loglock);
    close(main_socket);
}


int main(int argc, char **argv) {
    unsigned short port = 3000;
    int num_threads = 1;

    int c;
    while (-1 != (c = getopt(argc, argv, "hp:"))) {
        switch(c) {
            case 'p':
                port = atoi(optarg);
                if (port < 1024) {
                    usage(argv[0]);
                }
                break;

            case 't':
                num_threads = atoi(optarg);
                if (num_threads < 1) {
                    usage(argv[0]);
                }
                break;
            case 'h':
            default:
                usage(argv[0]);
                break;
        }
    }

    runserver(num_threads, port);
    
    fprintf(stderr, "Server done.\n");
    exit(0);
}
