#include "stream.h"

/**
 * read lines from input file and write each line to pipes
 * each line will contain words as in Project 1
 */
void emit(char * inputFileName) {
     //getting a pointer to the start of the file
    FILE *fp = getFilePointer(inputFileName);
    //initializing variables
    char buffer[chunkSize];
    int length = 0;
    if(fp == NULL){
        printf("unable to access file\n");
    }
    //looping through each line in the file
    while ((length = getLineFromFile(fp, buffer, sizeof(buffer))) != -1) {
        printf("%s\n", buffer);
    }
}
/***
 *
 * Stream process will read from the files created by Master
 */
int main(int argc, char *argv[]) {

    //accessing parameters
    mapperID = strtol(argv[1], NULL, 10);
    int nMappers = strtol(argv[2], NULL, 10);

    char ipFdr[2000] = "";

    //creating path for file pointer
    sprintf(ipFdr, "MapperInput/Mapper%d.txt", mapperID);

    // getting a pointer to the start of the file
    FILE *fp = getFilePointer(ipFdr);

    //initializing variables
    char filePath[chunkSize];
    size_t len = chunkSize;
    int length = 0;

    //looping through each line in the file
    while ((length = getLineFromFile(fp, filePath, len)) != -1) {
        filePath[strlen(filePath) - 1] = '\0';
        emit(filePath);
    }
    return EXIT_SUCCESS;
}
