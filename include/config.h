#ifndef CONFIG_H_
#define CONFIG_H_

#define MAX_STRING_LENGTH 1024

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct JVSConfig
{
    char devicePath[MAX_STRING_LENGTH];
    int analogueFuzz;
} JVSConfig;

extern char devicePath[];
extern int analogueFuzz;

int parseConfig(char *filePath, JVSConfig *jvsConfig);

#endif // CONFIG_H_
