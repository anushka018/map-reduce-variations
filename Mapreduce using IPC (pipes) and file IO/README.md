# Information
Test Machine: CSEL-LIND40-16
Date: 3/19/2021
Names:
Kinza Ahmed - ahme0170
Daniela Moreno Ahumada - moren225
Anushka Angamuthu - angam003

# Project 2 : MapReduce - Word counts of different lengths
The purpose of this project is to use the MapReduce framework to count the number of times a certain word length appears in text files. MapReduce is a great framework that allows us to divide our work into subproblems, a lot similar to the Divide and Conquer algorithm. This is a bit different than project 1 since we implement utility functions to perform File IO and perform inter process communiation (IPC) using pipes to send the data to mappers.

## Master function (mapreduce.c)

The master function is the main process that drives this whole project. First it creates the directories that are needed for the stream and mapper process. Then, spawns the stream processes which executes stream and waits for all child processes to finish executing. Then, spawns the mapper processes which executes mapper and waits for all child processes to finish executing. Then spawns the reducer proceses which exectures reducer and wait for al the child processes to finish executing. 

## Stream function (stream.c)

The stream part of our project is the divide phase in the project, where we write to our pipes. TThe stream phase essentially reads the input text files from the file directories that are given, and writes them to corresponding pipes. 

## Map function (mapper.c)

The map part of our project is the divide phase in the project, where we divide our work into subprocesses. The map mapped a certain number of files to each process, the number of files mapped is the same for each mapper process. The map would go through each files per mapper and read the file, and count the number of times a certain word length occurs. Those results were stored in the interDS datastructure. 

After that was populated, we wrote the values of interDS to the output/Intermediate folder for the reducer to use. 

## Reduce function (reducer.c)

While the reduce part of our project does divide itself into subproblems(processes), it also combines the work that we did in mapper. The reduce class takes the files that we created in the InterMediate folder and then combines the results of each process based off of the word lengths. Once the results are stored in a FinalDS data structure, we create files into the output/Final folder creating 20 files which hold the number of times a certain word length occured. 

## Compile
	> make clean
	> make

## Execution
	// always call make clean and make before you start execution
	// ./mapreduce nMappers nReducer inputFileDir
	> ./mapreduce 5 3 test/T0

## Result
Check output folder for all the results
	
## Contributions
All members contributed equally to the project as we held paired programming sessions to develop the code. Code commenting was taken on by each one of us and we designated certain tasks to evenly divide the work. 





