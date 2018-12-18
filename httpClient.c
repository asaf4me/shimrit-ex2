#include "httpClient.h"
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void error_case_handler(char *color, char *msg)
{
    if (strcmp(color, "red") == 0)
    {
        printf("\033[0;31m");
        printf("%s", msg);
        printf("\033[0m");
    }
}

char **parsingUrl(const int argc, const char **argv)
{
    char **parser = (char **)malloc(argc * sizeof(char *));
    if (parser != NULL)
    {
        char *element;
        int arvIndex, index;
        for (arvIndex = 1, index = 0; arvIndex < argc && index < argc; arvIndex++, index++)
        {
            element = strtok((char *)argv[arvIndex], " ");
            parser[index] = (char *)malloc(strlen(element) + 1);
            strcpy(parser[index], element);
            parser[index][strlen(element)] = '\0';
        }
        if(validation(parser,argc) == true)
            return parser;
        else
            freeHttp(parser,argc);
    }
    return NULL;
}

void freeHttp(char **httpReq, int argc)
{
    if (httpReq == NULL)
        return;
    for (int i = 0; i < argc - 1; i++)
    {
        if (httpReq[i])
            free(httpReq[i]);
    }
    free(httpReq);
}

bool validation(char **httpReq ,int argc)
{
    for(int i = 0; i < argc ; i ++)
    {

    }

    return false;
}