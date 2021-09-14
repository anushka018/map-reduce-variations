#ifndef STREAM_H
#define STREAM_H

#include "utils.h"


int mapperID;
char *inputFileDir;
char *intermediateDir = "output/IntermediateData";
int interDS[MaxWordLength];

void emit(char *line);
void map(char * inputFileName);

#endif