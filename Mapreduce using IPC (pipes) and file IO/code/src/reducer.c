#include "reducer.h"

/**
 * Write final word count to files.
 * The file should be in a corresponding folder in output/FinalData/
 */
void writeFinalDSToFiles(void) {
    for (int i = 0; i < MaxWordLength ; i++)
    {
        char pathName[maxFileNameLength] = "";
        sprintf(pathName,"%s/%d.txt",finalDir, (i+1)); // build up path name output/FinalData/i+1.txt
        char line[20000] = "";
        if(finalDS[i] != 0){
          sprintf(line, "%d %d", (i+1), finalDS[i]); // build up string with word length and sum of words of that length
          writeLineToFile(pathName, line); // write each line to the correct file
        }
    }
}

/**
 * Read lines from files, and calculate a total count for a specific word length
 */
void reduce(char * intermediateFileName) {
  FILE *fp = getFilePointer(intermediateFileName); // get a file pointer to current input file
  //initializing variables
  char buffer[chunkSize];
  size_t len = MaxWordLength; // size of the line in bytes
  int length = 0; // number of characters read plus newline
  int wordLength; // the length of the current
  char space[] = " ";
  int num = 0; // incrementer variable to get sum of words that had a certain length
  char* numOccurences; // the number of times a word of a given length appeared in a file

  if((length = getLineFromFile(fp, buffer, len)) != -1) { //read each line until end of input reached
    char folder[MaxWordLength]; //initialize array for folder
    strcpy(folder, buffer); // copy line content into folder array
    strtok(folder, space); //just get what is before space (word length)
    wordLength = atoi(folder); // convert it to an int
    numOccurences = strchr(buffer, ' '); // get number of times a word of a given length appeared in a file
    num = atoi(numOccurences); // convert input to an integer
  }
  wordLength--;
  finalDS[wordLength] += num; // sum up count of words of a given length
}

int main(int argc, char *argv[]) {
    if(argc != 4){ // error handling to check if the incorrect number of command lines arguements was entered
		printf("Incorrect number of args: given %d, expected 2.\n", argc - 1);
        return -1;
	}
	// initialize variables
	reducerID = strtol(argv[1], NULL, 10);
	int nReducers = strtol(argv[2], NULL, 10);
  int nMappers = strtol(argv[3], NULL, 10);
    if(nReducers > 20){ // error handling to check if the correct number of mappers was entered
    printf("Incorrect number of reducers: given %d, expected 20 or less.\n", nReducers);
      return -1;
    }
    char *myTasks[MaxNumIntermediateFiles] = {NULL};
    char *interFilesArr[200] = {NULL};
    count = 0;
    int numFiles[nReducers];

    // getting all the filepaths in an array
    storeInterFilePaths(&interFilesArr[0], intermediateDir);

    // storing the total number of files to be distributed
    storesCount(numFiles, nMappers, nReducers);

    // getReducerTasks function returns a list of intermediate file names that this reducer should process
    int nTasks = getReducerTasks(nReducers, reducerID, numFiles, &myTasks[0], &interFilesArr[0]);
    int tIdx;
    for (tIdx = 0; tIdx < nTasks; tIdx++) { // call reduce on each intermediate file
      reduce(myTasks[tIdx]);
      free(myTasks[tIdx]);
    }

    writeFinalDSToFiles(); // write finalDS to final folder


	return EXIT_SUCCESS;
}
