#include "config.h"

char devicePath[2024] = "/dev/ttyUSB0";
int analogueFuzz = 2;

int parseConfig(char *filePath)
{
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
                    strcpy(devicePath, token);
                }

                /* Grab the Fuzz Settings */
                if (strcmp(token, "ANALOGUE_FUZZ") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token[strlen(token) - 1] == '\n')
                        token[strlen(token) - 1] = '\0';
                    analogueFuzz = atoi(token);
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
