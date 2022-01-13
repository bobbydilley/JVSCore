/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * 2022 Contributor and DE10-Nano tester: Javier Rodas (@JaviRodasG) <javier.rodas@gmail.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

#include "config.h"

#define DEFAULT_DEVICE_PATH "/dev/ttyUSB0"
#define DEFAULT_ANALOGUE_FUZZ 2
#define DEFAULT_ENABLE_ANALOGUE 1

int parseConfig(char *filePath, JVSConfig *jvsConfig)
{
    // Setup default values
    jvsConfig->analogueFuzz = DEFAULT_ANALOGUE_FUZZ;
    jvsConfig->enableAnalogue = DEFAULT_ENABLE_ANALOGUE;
    strcpy(jvsConfig->devicePath, DEFAULT_DEVICE_PATH);

    FILE *fp;
    char buffer[1024];
    if ((fp = fopen(filePath, "r")) != NULL)
    {
        fgets(buffer, 1024, fp);
        while (!feof(fp))
        {
            if (buffer[0] != '#' && buffer[0] != 0 && strcmp(buffer, "") != 0)
            {
                char *token = strtok(buffer, " ");

                /* Grab the Device Path */
                if (strcmp(token, "DEVICE_PATH") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token[strlen(token) - 1] == '\n')
                        token[strlen(token) - 1] = '\0';
                    strcpy(jvsConfig->devicePath, token);
                }

                /* Grab the Fuzz Settings */
                if (strcmp(token, "ANALOGUE_FUZZ") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token[strlen(token) - 1] == '\n')
                        token[strlen(token) - 1] = '\0';
                    jvsConfig->analogueFuzz = atoi(token);
                }

                /* Grab the Enable Analogue Settings */
                if (strcmp(token, "ENABLE_ANALOGUE") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token[strlen(token) - 1] == '\n')
                        token[strlen(token) - 1] = '\0';
                    jvsConfig->enableAnalogue = atoi(token);
                }
            }
            fgets(buffer, 1024, fp);
        }
    }
    else
    {
        return 0;
    }
    fclose(fp);
    return 1;
}
