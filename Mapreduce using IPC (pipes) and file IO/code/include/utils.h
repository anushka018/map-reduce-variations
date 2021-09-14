#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>

#define chunkSize 1024
#define MaxNumProcesses 20
#define MaxWordLength 20
#define maxFileNameLength 200
#define MaxNumInputFiles 100
#define MaxNumIntermediateFiles 400

int fileCount;
int fd[100];
int count;
int reducerFiles;
int totalFiles;
int interCount;

/**
 * stores the total number of filepaths and how many need to be distributed to each
      reducer process
 * @param  arr[]     where the filepath info is stored
 * @param  nMappers  number of mappers
 * @param  nReducers    number of reducers
 */
void storesCount(int arr[], int nMappers, int nReducers);

/**
 * Stores all the directory filepaths into an array
 * @param  **arr    the array to store the filepaths
 * @param  *intermediateDir  the directory that contains the files
 */
void storeInterFilePaths(char **arr, char *intermediateDir);

/**
 * Creates the MapperInput directory and the mapper number of .txt files
 * @param  mappers    the number of mappers
 */
void createMapperInputDir(int mappers);

/**
 * Finds all the filepaths from a directory and distributes them among the MapperInput directory files
 * @param  *path   the directory path
 * @param  mappers  the number of mappers
 */
void traverseInputFileDirectory(char *path, int mappers);


int getMapperTasks(int nMappers, int mapperID, char *inputFileDir, char **myTasks);

 /**
  * gets the reducer tasks that correspond to each reducer process
  * @param  nReducers    the number of reducers
  * @param  reducerID  the reducer id
  * @param  numFiles[]    array that stores how many files should be in each process
  * @param  **myTask    where the list of intermediate file names are stored
  * @param  **arr    the intermediate array with all filepaths
  * @return The number of intermediate files the reducer should process
  */
 int getReducerTasks(int nReducers, int reducerID, int numFiles[], char **myTasks, char **arr);

// file I/O
/**
 * Get a pointer to a opened file based on the file name
 * @param *inputFileName  the file path
 * @return a file pointer pointing to the file
 */
FILE * getFilePointer(char *inputFileName);

/**
 * Read an entire line from a file
 * @param  *fp    the file to be read
 * @param  *line  contain the line content
 * @param  len    the size of the line
 * @return the number of character reads (including the newline \n, but not including terminator)
           -1 when reaching the end of file or error occurs
 */
ssize_t getLineFromFile(FILE *fp, char *line, size_t len);

/**
 * Open a file, and write a line to the file
 * @param *filepath  the file path
 * @param *line      the line content
 */
void writeLineToFile(char *filepath, char *line);

// directory
void bookeepingCode();
int isValidDir(char *folder);

#endif
