#ifndef MAPPER_H
#define MAPPER_H

#include "utils.h"

int mapperID;
char *inputFileDir;
char *intermediateDir = "output/IntermediateData";
int interDS[MaxWordLength];

void parse(char * line);
void writeInterDSToFiles(void);

#endif