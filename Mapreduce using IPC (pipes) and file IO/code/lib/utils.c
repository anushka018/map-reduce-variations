#include "utils.h"

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

FILE * getFilePointer(char *inputFileName) {
    return fopen(inputFileName, "r");
}

ssize_t getLineFromFile(FILE *fp, char *line, size_t len) {
    memset(line, '\0', len);
    return getline(&line, &len, fp);
}

void storeInterFilePaths(char **interFilesArr, char *intermediateDir){
  char *fileBuf;
	struct stat* buf = (struct stat*)malloc(sizeof(struct stat));

	DIR* dir = opendir(intermediateDir);
	struct dirent* entry;
  // traversing directory to access filepaths
	while ((entry = readdir(dir)) != NULL) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

    // checking if it is a directory
		if (entry->d_type == DT_DIR) {

      //creating filepath
			char next[strlen(intermediateDir) + strlen(entry->d_name) + 2];
			next[0] = '\0';
			strcat(next, intermediateDir);
			strcat(next, "/");
			strcat(next, entry->d_name);

      // recursive call
			storeInterFilePaths(interFilesArr, next);

		} else {
            // creating filepath
            fileBuf = (char *) malloc(chunkSize*(sizeof(char)));
            sprintf(fileBuf, "%s/%s", intermediateDir, entry->d_name);
            interFilesArr[interCount] = fileBuf;
            interCount++;
		}
    }
}

void storesCount(int numFiles[], int nMappers, int nReducers){
    totalFiles = 20 * nMappers;
    int numPaths = totalFiles / nReducers;
    int remainder = totalFiles % nReducers;
    for(int i = 0; i < nReducers; i++){
       numFiles[i] = numPaths;
       if(i == nReducers - 1){
           numFiles[i] += remainder;
       }
    }
}


int getReducerTasks(int nReducers, int reducerID, int numFiles[], char **myTasks, char **interFilesArr){
    int count = numFiles[reducerID-1];
    if(reducerID == 1){
        //assigning tasks to reducer
        for(int i = 0; i < 100; i++){
            myTasks[i] = interFilesArr[i];
        }
    }
    else{
        int sumFiles = 0;
        for(int i = 0; i < reducerID; i++){
            sumFiles += numFiles[i];
        }
        //assigning tasks to reducer
        for(int i = sumFiles - numFiles[reducerID-1]; i < sumFiles; i++){
            myTasks[i] = interFilesArr[i];
        }
    }
    return totalFiles;
}

void createMapperInputDir(int mappers){
    char folderName[chunkSize] = "MapperInput";
    // creating the MapperInput directory
    mkdir(folderName, ACCESSPERMS);

    // creating the files within MapperInput
    for(int i = 0; i < mappers; i++){
        char pathName[30000] = "";
        // creating the path for each .txt file in the mapper input folder
        sprintf(pathName, "%s/Mapper%d.txt",folderName, i);
        char line[1] = "";
        // writing each line to the designated .txt file in the intermediate folder
        writeLineToFile(pathName, line);
    }
    fileCount = 0;
}

void traverseInputFileDirectory(char *path, int mappers){
	struct stat* buf = (struct stat*)malloc(sizeof(struct stat));
  // opening directory
	DIR* dir = opendir(path);
	struct dirent* entry;

  // traversing the directory
	while ((entry = readdir(dir)) != NULL) {
        lstat(path, buf);
        if(S_ISLNK(buf->st_mode) == 0){
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

        // checking if it is a directory
		    if (entry->d_type == DT_DIR) {

          // creating filepath
		    	char next[strlen(path) + strlen(entry->d_name) + 2];
		    	next[0] = '\0';
		    	strcat(next, path);
		    	strcat(next, "/");
		    	strcat(next, entry->d_name);
		    	traverseInputFileDirectory(next, mappers);
		    } else {
                // creating filepath
                char fileBuf[chunkSize] = "";
                char pathName[chunkSize] = "";
                sprintf(fileBuf, "%s/%s\n", path, entry->d_name);
                sprintf(pathName, "MapperInput/Mapper%d.txt", fileCount);
                fileCount++;

                // opening the file
                int fd = open(pathName, O_CREAT | O_WRONLY | O_APPEND, 0777);
                if (fd < 0){
                    printf("ERROR: Cannot open the file %s\n", pathName);
                    fflush(stdout);
                    exit(EXIT_FAILURE);
                }
                // writing to the file
                int ret = write(fd, fileBuf, strlen(fileBuf));
                if(ret < 0){
                    printf("ERROR: Cannot write to file %s\n", pathName);
                    fflush(stdout);
                    exit(EXIT_FAILURE);
                }

                // ensures that filepaths are being evenly distributed
                if(fileCount == mappers){
                    fileCount = 0;
                }
		    }
        }
	}

	free(buf);
}

int isValidDir(char *folder) {
    struct stat sb;
    return (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode));
}

void _removeOutputDir(){
    pid_t pid = fork();
    if(pid == 0){
        char *argv[] = {"rm", "-rf", "output", NULL};
        if (execvp(*argv, argv) < 0) {
            printf("ERROR: exec failed\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else{
        wait(NULL);
    }
}

void _createOutputDir(){
    mkdir("output", ACCESSPERMS);
    mkdir("output/IntermediateData", ACCESSPERMS);
    mkdir("output/FinalData", ACCESSPERMS);
}

void _createInterFolders(){
    int wordLen;
    for(wordLen = 1; wordLen <= MaxWordLength; wordLen++) {
        char dirName[50];
        memset(dirName, '\0', 50);
        sprintf(dirName, "output/IntermediateData/%d", wordLen);
        mkdir(dirName, ACCESSPERMS);
    }
}

void bookeepingCode(){
    _removeOutputDir();
    sleep(1);
    _createOutputDir();
    _createInterFolders();
}
