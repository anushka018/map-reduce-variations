#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <zconf.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include "../include/protocol.h"

// Initializing variables
FILE *logfp;
int fileCount;

// getting the file pointer to read from the file
FILE * getFilePointer(char *inputFileName) {
    return fopen(inputFileName, "r");
}

// returning lines from the given file
ssize_t getLineFromFile(FILE *fp, char *line, size_t len) {
    memset(line, '\0', len);
    return getline(&line, &len, fp);
}

// removing the output directory
void _removeOutputDir(){
    pid_t pid = fork();
    if(pid == 0){
        char *argv[] = {"rm", "-rf", "ClientInput", NULL};
        if (execvp(*argv, argv) < 0) {
            printf("ERROR: exec failed\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else{
        wait(NULL);
    }
}

// creates the log file
void createLogFile(void) {
    pid_t p = fork();
    if (p == 0)
        execl("/bin/rm", "rm", "-rf", "log", NULL);

    wait(NULL);
    mkdir("log", ACCESSPERMS);
    logfp = fopen("log/log_client.txt", "w");
}

// checks to see if the given folder is a valid directory
int isValidDir(char *folder) {
    struct stat sb;
    return (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode));
}

// function used to write lines to a given file
void writeLineToFile(char *filepath, char *line) {
    int fd = open(filepath, O_CREAT | O_WRONLY, 0777);
    if (fd < 0){
        printf("ERROR: Cannot open the file %s\n", filepath);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    int ret = write(fd, line, strlen(line));
    if(ret < 0){
        printf("ERROR: Cannot write to file %s\n", filepath);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
}

// creates each clients input directory
void createClientsInputDir(int clients){
    // removing the output directory
    _removeOutputDir();
    char folderName[1024] = "ClientInput";
    mkdir(folderName, ACCESSPERMS);
    for(int i = 1; i <= clients; i++){
        char pathName[30000] = "";
        // creating the path for each .txt file in the Clients input folder
        sprintf(pathName, "%s/Client%d.txt",folderName, i);
        char line[1] = "";
        // writing each line to the designated .txt file in the intermediate folder
        writeLineToFile(pathName, line);
    }
    fileCount = 1;
}

// traverses inputfile directory and create ClientsInput directory
void traverseInputFileDirectory(char *path, int clients){

	struct stat* buf = (struct stat*)malloc(sizeof(struct stat));

	DIR* dir = opendir(path);
	struct dirent* entry;
  // loops until the entry in the directory is NULL
	while ((entry = readdir(dir)) != NULL) {
        lstat(path, buf);
        if(S_ISLNK(buf->st_mode) == 0){
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

        //checking if the entry is a directory
		    if (entry->d_type == DT_DIR) {

		    	char next[strlen(path) + strlen(entry->d_name) + 2];
		    	next[0] = '\0';
          // creating the file path
		    	strcat(next, path);
		    	strcat(next, "/");
		    	strcat(next, entry->d_name);
          //recursive call
		    	traverseInputFileDirectory(next, clients);
		    } else {
                char fileBuf[1024] = "";
                char pathName[1024] = "";
                //creating the filepath
                sprintf(fileBuf, "%s/%s\n", path, entry->d_name);
                sprintf(pathName, "ClientInput/Client%d.txt", fileCount);
                fileCount++;

                //opening the file from a given filepath name
                int fd = open(pathName, O_CREAT | O_WRONLY | O_APPEND, 0777);
                if (fd < 0){
                    printf("ERROR: Cannot open the file %s\n", pathName);
                    fflush(stdout);
                    exit(EXIT_FAILURE);
                }
                //writing the filepath to the file
                int ret = write(fd, fileBuf, strlen(fileBuf));
                if(ret < 0){
                    printf("ERROR: Cannot write to file %s\n", pathName);
                    fflush(stdout);
                    exit(EXIT_FAILURE);
                }
                if(fileCount == clients+1){
                    fileCount = 1;
                }
		    }
        }
	}

	free(buf);
}

/**
 * parse lines from pipes, and count words by word length
 */
void parse(char * line, int *interDS) {
    // initializing variables
    int wordLen = 0;
    // looping through each character in the line
    for(int i=0; i< strlen(line); i++){
      // checking to see if the character is a space or a newline
	    if(line[i] == ' ' || line[i] == '\n'){
            // setting the values of interDS if they are NOT 0
            if(wordLen != 0){
                interDS[wordLen-1]++;
            }
            // resetting the value of the word length count
            wordLen = 0;
	    }
        // incrementing the count of characters in a word
        else{
            wordLen++;
        }
    }
}

// updating PER-FILE word length statistics
void updateWSTAT(char *inputFileName, int *interDS){
    FILE *fp = getFilePointer(inputFileName);
    //initializing variables
    char buffer[1024];
    int length = 0;
    if(fp == NULL){
        printf("unable to access file\n");
    }
    //looping through each line in the file
    while ((length = getLineFromFile(fp, buffer, sizeof(buffer))) != -1) {
        parse(buffer, interDS);
    }
}

void spawnClients(){
  //looping through the number of clients
	for (int i = 1; i <= nClients; i++)
	{
    // forks nclients child processes
		pid_t pid = fork();
    // error handling to check if fork was called successfully
		if (pid < 0) {
        	fprintf(stderr, "ERROR: Failed to fork\n");
		}

    // in child process
		if (pid == 0){
			// Create a TCP socket.
            int sockfd = socket(AF_INET , SOCK_STREAM , 0);

            // Specify an address to connect to (we use the local host or 'loop-back' address).
            struct sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_port = htons(serverPort);
            address.sin_addr.s_addr = inet_addr(serverIP);
            int msgBuf[23];
            int getMyResp[3];
	        int updateResponse[3];
	        int getAllUpdatesResp[3];
	        int getWSTATResp[22];
            // Connecting the client to the server
            if (connect(sockfd, (struct sockaddr *) &address, sizeof(address)) == 0) {
                // logging updates
                char logMsg[1000] = "";
                sprintf(logMsg,"[%d] open connection\n",1);
                fputs(logMsg, logfp);
                // loops 4 times for the 4 request types
                for(int j = 1; j <= 4; j++){
                    // request for UPDATE_WSTAT
                    if(j == UPDATE_WSTAT){
                        char ipFdr[2000] = "";
                        sprintf(ipFdr, "ClientInput/Client%d.txt", i);
                        FILE *fp = getFilePointer(ipFdr);
                        // initializing variables
                        char filePath[1024];
                        size_t len = 1024;
                        int length = 0;
                        int count = 0;
                        // looping through each line in the file
                        while ((length = getLineFromFile(fp, filePath, len)) != -1) {
                            filePath[strlen(filePath) - 1] = '\0';
                            int interDS[20];
                            for(int i = 0; i < 20; i++){
                                interDS[i] = 0;
                            }
                            // updating PER-FILE word length statistics
                            updateWSTAT(filePath, interDS);
                            // setting the message fields
                            msgBuf[0] = j;
                            msgBuf[1] = i;
                            msgBuf[22] = 1;
                            count++;
                            // updating the interDS data structure
                            for(int k = 0; k < 20; k++){
                                msgBuf[k+2] = interDS[k];
                            }
                            // sending data to the server
                            send(sockfd, (void *) &msgBuf, 23*sizeof(int), 0);
                            // checking if the server did not recieve the message
                            if(read(sockfd, updateResponse, sizeof(updateResponse)) < 0){
		                        printf("recv failed\n");
                                return;
	                        }
                        }
                        // updating the log
                        logMsg[1000] = "";
                        sprintf(logMsg,"[%d] UPDATE_WSTAT: %d\n",msgBuf[1], count);
                        fputs(logMsg, logfp);
                    }
                    // request for GET_MY_UPDATES
                    else if(j == GET_MY_UPDATES){
                        // setting the message fields
                        msgBuf[0] = j;
                        msgBuf[1] = i;
                        msgBuf[22] = 1;
                        for(int k = 0; k < 20; k++){
                            msgBuf[k+2] = 0;
                        }
                        // sending data to the server
                        send(sockfd, (void *)&msgBuf, 23*sizeof(int), 0);
                        // checking if the server did not recieve the message
                        if(read(sockfd, getMyResp, 3*sizeof(int)) < 0){
		                    printf("recv failed\n");
		                    return;
	                    }
                        // updating the log
                        logMsg[1000] = "";
                        sprintf(logMsg,"[%d] GET_MY_UPDATES: %d %d\n",msgBuf[1], getMyResp[1], getMyResp[2]);
                        fputs(logMsg, logfp);
                    }
                    // request for GET_MY_UPDATES
                    else if(j == GET_ALL_UPDATES){
                        // setting the message fields
                        msgBuf[0] = j;
                        msgBuf[1] = i;
                        msgBuf[22] = 1;
                        for(int k = 0; k < 20; k++){
                            msgBuf[k+2] = 0;
                        }
                        // sending data to the server
                        send(sockfd, (void *)&msgBuf, 23*sizeof(int), 0);
                        // checking if the server did not recieve the message
                        if(read(sockfd, getAllUpdatesResp, 3*sizeof(int)) < 0){
		                    printf("recv failed\n");
		                    return;
	                    }
                        // updating the log
                        logMsg[1000] = "";
                        sprintf(logMsg,"[%d] GET_ALL_UPDATES: %d %d\n",msgBuf[1], getAllUpdatesResp[1], getAllUpdatesResp[2]);
                        fputs(logMsg, logfp);
                    }
                    // request for GET_WSTAT
                    else if(j == GET_WSTAT){
                        // setting the message fields
                        msgBuf[0] = j;
                        msgBuf[1] = i;
                        msgBuf[22] = 0;
                        for(int k = 0; k < 20; k++){
                            msgBuf[k+2] = 0;
                        }
                        // sending data to the server
                        send(sockfd, (void *)&msgBuf, 23*sizeof(int), 0);
                        // checking if the server did not recieve the message
                        if(read(sockfd, getWSTATResp, sizeof(getWSTATResp))  < 0){
		                    printf("recv failed\n");
		                    return;
	                      }
                        // updating the log
                        logMsg[1000] = "";
                        sprintf(logMsg,"[%d] GET_WSTAT: %d",msgBuf[1], getWSTATResp[1]);
                        fputs(logMsg, logfp);
                        for(int k = 2; k < 22; k++){
                            sprintf(logMsg," %d",getWSTATResp[k]);
                            fputs(logMsg, logfp);
                        }
                        sprintf(logMsg,"\n");
                        fputs(logMsg, logfp);
                        // checking if the connection was successful
                        if(getAllUpdatesResp[1] == RSP_NOK){
                            sprintf(logMsg,"[%d] close connection (failed execution)\n",i);
                        }
                        else{
                            sprintf(logMsg,"[%d] close connection (successful execution)\n",i);
                        }
                        fputs(logMsg, logfp);
                    }
                }
                //close connection
                close(sockfd);
            }
            else {
                perror("Connection failed!");
            }
	        exit(0);
	     }
	}
}

// wait for all child processes to finish terminating
void waitForAll(int processes) {
	for (int i = 0; i < processes; i++)
	{
		wait(NULL);
	}
}

int main(int argc, char *argv[]) {

  // process input arguments
	if (argc < 5) {

		printf("Wrong number of args, expected %d, given %d\n", 4, argc-1);
		exit(1);
	}

    // create log file
    createLogFile();

    inputFileDir = argv[1];
    // check if the input file directory is a valid directory
    if(!isValidDir(inputFileDir))
        exit(EXIT_FAILURE);
    nClients = strtol(argv[2], NULL, 10);
    // creating each clients input directory
    createClientsInputDir(nClients);
    // traverses inputfile directory and create ClientsInput directory
    traverseInputFileDirectory(inputFileDir,nClients);

    serverIP = argv[3];
    serverPort = strtol(argv[4], NULL, 10);
    // spawn client processes
	spawnClients();

    // wait for all client processes to terminate
    waitForAll(nClients);

    // close log file
    fclose(logfp);

    return EXIT_SUCCESS;

}
