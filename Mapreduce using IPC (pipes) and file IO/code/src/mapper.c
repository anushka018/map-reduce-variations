#include "mapper.h"

/**
 * Write word count that is stored in an intermediate data structure to files.
 * The file should be in a corresponding folder in output/IntermediateData/ 
 */
void writeInterDSToFiles(void) {
    //loops through the intermeidate array
    for (int i = 0; i < MaxWordLength ; i++)
    {
        char pathName[maxFileNameLength] = "";
        //creating the path for each .txt file in the intermeidate folder
        sprintf(pathName,"%s/%d/m_%d.txt",intermediateDir,(i+1), mapperID);
        char line[20000] = "";
        //creating the line to be written to the file
        sprintf(line, "%d %d", i+1, interDS[i]);
        //writing each line to the designated .txt file in the intermediate folder
        writeLineToFile(pathName, line);
    }
}

/**
 * parse lines from pipes, and count words by word length
 */
void parse(char * line) {
    //initializing variables
    int wordLen = 0;
    //looping through each character in the line
    for(int i=0; i< strlen(line); i++){
      //checking to see if the character is a space or a newline
	    if(line[i] == ' ' || line[i] == '\n'){
            //setting the values of interDS if they are NOT 0
            if(wordLen != 0){
                interDS[wordLen-1]++;
            }
            //resetting the value of the word length count
            wordLen = 0;
	    }
        //incrementing the count of characters in a word
        else{
            wordLen++;
        }
    }
}

int main(int argc, char *argv[]) {

    mapperID = strtol(argv[1], NULL, 10);
    char buffer[chunkSize];
    int err;
    while (err = read(STDIN_FILENO, buffer, chunkSize) > 0) {
        parse(buffer);
    }

    writeInterDSToFiles();
    return EXIT_SUCCESS;
}