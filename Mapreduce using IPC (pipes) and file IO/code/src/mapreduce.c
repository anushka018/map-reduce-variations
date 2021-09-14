#include "mapreduce.h"

int main(int argc, char *argv[]) {
	if(argc != 4){ // error handling to check if the incorrect number of command lines arguements was entered
		printf("Incorrect number of args: given %d, expected 3.\n", argc - 1);
        return -1;
	}
	interCount = 0;
	int nMappers = strtol(argv[1], NULL, 10); // get number of mappers
	int nReducers 	= strtol(argv[2], NULL, 10); // get number of reducers
    inputFileDir = argv[3];
    if(!isValidDir(inputFileDir)) // check if the input file directory is a valid directory
        exit(EXIT_FAILURE);

	bookeepingCode(); // call code for creation and removal of directories

	// creating the MapperInput Directory and the all of the .txt files
	createMapperInputDir(nMappers);

	// getting all the filepaths and storing them in the MapperInput directory files
	traverseInputFileDirectory(inputFileDir,nMappers);

	// opening the pipes for inter process communication
	openPipes(nMappers);

	// spawn stream
	spawnStream(nMappers);

	// spawn mappers
	spawnMapper(nMappers);

	// closing pipes
	for(int i = 0; i < nMappers*2; i++){
		close(fd[i]);
	}
	// wait for all children to complete execution
	waitForAll(nMappers*2);
	// spawn reducers
	spawnReducers(nReducers, nMappers);
	// wait for all children to complete execution
	waitForAll(nReducers);

	return EXIT_SUCCESS;
}

void openPipes(int nMappers){
	// opening the pipes for inter process communication
	for(int i = 0; i < nMappers; i++){
		pipe(fd + (i*2));
	}
}

void spawnMapper(int nMappers){
	pid_t pid; 	//Initialize variable for pid
	for (int i = 0; i < nMappers; i++)
	{
		pid_t pid = fork(); // forks nMappers child processes
		if (pid < 0) { // error handling to check if fork was called successfully
        	fprintf(stderr, "ERROR: Failed to fork\n");
		}

		if (pid == 0){ // in child process
			// intialize char arrays because exec arguements must be converted to strings
			char str_i[10];
			char str_nMappers[10];

			sprintf(str_i, "%d", i); // build string for mapperID arguement
			sprintf(str_nMappers, "%d", nMappers); // build string for nMappers arguement

			// creating parameter array for exec call
			char *args[] = {"mapper", str_i, str_nMappers, inputFileDir, NULL};

			// closing all other read ends
			for(int j = 0; j < nMappers*2; j++){
				if(j != i*2){
					close(fd[j]);
				}
			}
			// STDIN redirection
			dup2(fd[(i*2)], STDIN_FILENO);
			close(fd[(i*2)]); // closing the write end of the pipes
			execv("mapper", args); // call exec with args array
			printf("execv failed in spawnMapper\n"); // error handling for exec
			exit(0);
		}
	}
}

void spawnStream(int nMappers){
	pid_t pid; 	//Initialize variable for pid
	for (int i = 0; i < nMappers; i++)
	{
		pid_t pid = fork(); // forks nMappers child processes
		if (pid < 0) { // error handling to check if fork was called successfully
        	fprintf(stderr, "ERROR: Failed to fork\n");
		}

		if (pid == 0){ // in child process
			// intialize char arrays because exec arguements must be converted to strings
			char str_i[10];
			char str_nMappers[10];

			sprintf(str_i, "%d", i); // build string for mapperID arguement
			sprintf(str_nMappers, "%d", nMappers); // build string for nMappers arguement

			// creating parameter array for exec call
			char *args[] = {"stream", str_i, str_nMappers, NULL};

			// closing all other write ends
			for(int j = 0; j < nMappers*2; j++){
				if(j != (i*2)+1){
					close(fd[j]);
				}
			}
			dup2(fd[(i*2)+1], STDOUT_FILENO); // STDOUT redirection

			close(fd[(i*2)]+1); // closing the read end of the pipes
			execv("stream", args); // call exec with args array
			printf("execv failed in spawnStream\n"); // error handling for exec
			exit(0);
		}
	}
}

void waitForAll(int processes) { // wait for all child processes to finish terminating
	pid_t terminated_pid;
	for (int i = 0; i < processes; i++)
	{
		terminated_pid = wait(NULL);
	}
}

void spawnReducers(int nReducers, int nMappers){
	pid_t pid; //Initialize variable for pid
	for (int i = 0; i < nReducers; i++) // forks nReducers child processes
	{
		pid_t pid = fork();
		if (pid < 0) {  // error handling to check if fork was called successfully
        	fprintf(stderr, "ERROR: Failed to fork\n");
		}
		if (pid == 0){ // in child process
			// intialize char arrays because exec arguements must be converted to strings
			char str_i[10];
			char str_nReducers[10];
			char str_nMappers[10];

			sprintf(str_i, "%d", i+1); // build string for mapperID arguement
			sprintf(str_nReducers, "%d", nReducers); // build string for nMappers arguement
			sprintf(str_nMappers, "%d", nMappers); // build string for nMappers arguement
			char *args[] = {"reducer", str_i, str_nReducers, str_nMappers, NULL};
			execv("reducer", args); // call exec with args array
			printf("execv failed in spawnReducers\n"); // error handling for exec
			exit(0);
		}
	}
}
