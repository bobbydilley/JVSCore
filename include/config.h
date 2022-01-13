/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

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
    int enableAnalogue;
} JVSConfig;

int parseConfig(char *filePath, JVSConfig *jvsConfig);

#endif // CONFIG_H_
