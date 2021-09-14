#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <zconf.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "../include/protocol.h"

//variable setup for mutexes, socket address, and thread argument struct
pthread_mutex_t mutexRH;
pthread_mutex_t mutexCS;
struct sockaddr_in clientAddress;
struct threadArg {
    int clientFd;
    char *clientIP;
    int clientPort;
};

void updateWSTAT(int request[]) {
	if(request == NULL){ //error checking if the request is null
		printf("Unsuccessful request\n");
	}
	//update result histogram
	for(int i = 2; i < 22; i++){
		if(&request[i] != NULL){
            //lock to ensure synchronization in the critical section
			pthread_mutex_lock(&mutexRH);
			resultHistogram[i-2] += request[i]; //update global histogram
			pthread_mutex_unlock(&mutexRH);
		}
	}
	//update client status depending on the client id
	pthread_mutex_lock(&mutexCS);
	clientStatus[request[1]]++;
	pthread_mutex_unlock(&mutexCS);
}

int getAllUpdates() {
	int sum = 0;
	for(int i = 0; i < 20; i++){
		pthread_mutex_lock(&mutexCS);
        //lock to ensure synchronization in the critical section
		if(&clientStatus[i] != NULL || &clientStatus[i] != 0){ // error checking 
			sum += clientStatus[i]; // sum up current number of updates for each client
		}
		pthread_mutex_unlock(&mutexCS);
	}
	return sum; 
}

// partse the request from the server and call the appropriate method depsning on the request code
void *parseRequest(void *args){
	int request[23];
	int getMyResp[3];
	int updateResponse[3];
	int getAllUpdatesResp[3];
	int getWSTATResp[22];
	struct threadArg *arg = args;
	while(1){
		if(read(arg->clientFd, request, 23*sizeof(int)) <= 0){ // get the request information from the client
			printf("recv failed\n"); //error checking 
			for(int i = 0; i < 23; i++){ // print out the request array
				printf("request[%d]: %d\n", i, request[i]);
			}
			return NULL;
		}
        // switch on the request code
		switch(request[0]){
			case UPDATE_WSTAT: //code = 1
				updateWSTAT(request);
                // set flag for if this was a valid or not
				updateResponse[1] = RSP_OK;
				if(request[1] <= 0){
					updateResponse[1] = RSP_NOK;
				}
                // assign information to updateResponse array
				updateResponse[0] = request[0];
				updateResponse[2] = request[1];
                // send the information to the client
				send(arg->clientFd, (void *)&updateResponse, 3*sizeof(int), 0);
                // output message
				printf("[%d] UPDATE_WSTAT\n", request[1]);
				break;
			case GET_MY_UPDATES: //code = 2
                // set flag for if this was a valid or not
				getMyResp[1] = RSP_OK;
				if(request[1] <= 0){
					getMyResp[1] = RSP_NOK;
				}
                // assign information to updateResponse array and get total updates for current client
				getMyResp[0] = GET_MY_UPDATES;
				getMyResp[2] = clientStatus[request[1]];
                // send the information to the client
				send(arg->clientFd, (void *)&getMyResp, 3*sizeof(int), 0);
                // output message
				printf("[%d] GET_MY_UPDATES\n", request[1]);
				break;
			case GET_ALL_UPDATES: //code = 2
			    // set flag for if this was a valid or not	
                getAllUpdatesResp[1] = RSP_OK;
				if(request[1] <= 0){
					getAllUpdatesResp[1] = RSP_NOK;
				}
                // assign information to updateResponse array and call getAllUpdates() to get toal number of updates for every client
				getAllUpdatesResp[0] = GET_ALL_UPDATES;
				getAllUpdatesResp[2] = getAllUpdates();
                // send the information to the client
				send(arg->clientFd, (void *)&getAllUpdatesResp, 3*sizeof(int), 0);
                // output message
				printf("[%d] GET_ALL_UPDATES\n", request[1]);
				break;
			case GET_WSTAT: //code = 4
                // set flag for if this was a valid or not
				getWSTATResp[1] = RSP_OK;
				if(request[1] <= 0){
					getWSTATResp[1] = RSP_NOK;
				}
				getWSTATResp[0] = GET_WSTAT; // set response code 
				for(int i = 0; i < 20; i++){ // get the values from the results histogram by iterating through it
					getWSTATResp[i+2] = resultHistogram[i];
				}
                // send the information to the client
				send(arg->clientFd, (void *)&getWSTATResp, 22*sizeof(int), 0);
                // output message
				printf("[%d] GET_WSTAT\n", request[1]);
				break;
			default: // error checking 
				printf("Incorrect request code\n");
				break;
		}
		if(request[22] == 0){ //check if we should close the connection
			printf("close connection from %s:%d\n",inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);
            // close and cleanup
			close(arg->clientFd);
    		free(arg);
			return NULL;
		}
	}
}


int main(int argc, char *argv[]) {
    //variable setup
	struct sockaddr_in servAddress;
	pthread_t threads[MAX_CONCURRENT_CLIENTS];
	int count = 0;
    // process input arguments
    if (argc > 2) { // error checking
		printf("Wrong number of args, expected %d, given %d\n", 1, argc - 1);
		exit(1);
	}
    // initialize mutexes
	if(pthread_mutex_init(&mutexRH, NULL) < 0){
		printf("initializing of the mutex failed\n"); // error checking
		return 0;
	}

	if(pthread_mutex_init(&mutexCS, NULL) < 0){
		printf("initializing of the mutex failed\n"); // error checking
		return 0;
	}
    // TCP socket creation and verification 
	int sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1) {
        printf("socket creation failed\n"); // error checking
        exit(EXIT_FAILURE);
    }
	memset(&servAddress, 0, sizeof(servAddress));

	// Bind socket to a local address.
	servAddress.sin_family = AF_INET;
	servAddress.sin_port = htons(strtol(argv[1], NULL, 10));
	servAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	int enable = 1;
	if(setsockopt(sock,SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))<0){
		printf("setsockopt(SO_REUSEADDR) failed\n"); // error checking
		exit(EXIT_FAILURE);
	}
    if (bind(sock, (struct sockaddr *) &servAddress, sizeof(servAddress))) {
        printf("socket bind failed\n");  // error checking
        exit(EXIT_FAILURE);
    }

	// We must now listen on this port 
    if (listen(sock, MAX_CONCURRENT_CLIENTS)) {
        printf("Listen failed\n");  // error checking
        exit(EXIT_FAILURE);
    } else {
        printf("server is listening\n");
    }

    while (1) {
        // Now accept the incoming connection by calling accept 
		socklen_t size = sizeof(struct sockaddr_in);
		int clientfd = accept(sock, (struct sockaddr*) &clientAddress, &size);
		if (clientfd < 0) {
            printf("server accept failed\n");  // error checking
            exit(EXIT_FAILURE);
        }
		else{ //succesfully connected 
			printf("open connection from %s:%d\n", inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);
		}
        // setup arguements for the parsing of the request
		struct threadArg *arg = (struct threadArg *) malloc(sizeof(struct threadArg));
        arg->clientFd = clientfd;
        arg->clientIP = inet_ntoa(clientAddress.sin_addr);
        arg->clientPort = clientAddress.sin_port;
        // call function to parse the client request
		parseRequest((void *)arg);
		//Create detachable threads
        int s;
        s = pthread_create(&threads[count], NULL, parseRequest, (void *)arg);
        if (s != 0) {
            fprintf(stderr, "pthread_create failed.\n"); // error checking
            exit(EXIT_FAILURE);
        }
        // detach threads
		s = pthread_detach(threads[count]);
        if (s != 0) {
            fprintf(stderr, "pthread_detach failed.\n"); // error checking
            exit(EXIT_FAILURE);
        }
        // increment count and reset to zero if more than the max number of concurrent clients
		count++;
		if (count == MAX_CONCURRENT_CLIENTS) count = 0;

    }
    // mutex deletion cleanup
    pthread_mutex_destroy(&mutexRH);
	pthread_mutex_destroy(&mutexCS);
	// Close the socket.
	close(sock);
    exit(EXIT_SUCCESS);
} 