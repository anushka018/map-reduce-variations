#ifndef MAPREDUCE_H
#define MAPREDUCE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "utils.h"

char *inputFileDir;
void spawnMapper(int nMappers);
void waitForAll(int processes);
void spawnReducers(int nReducers, int nMappers);
void openPipes(int nMappers);
void spawnStream(int nMappers);


#endif