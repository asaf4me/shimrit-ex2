#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "httpClient.h"

/*Private function's*/
void printParser(char **httpReq, int argc)
{
    for (int i = 0; i < argc - 1; i++)
    {
        printf("%s ", httpReq[i]);
    }

    printf("\n");
}

int main(int argc, char const *argv[])
{

    if (argc == 1)
    {
        error_case_handler("red", "Input case error: not enogh argument, example:\n");
        printf("client -r <NUM_OF_ARS> <arg1=?...> -p 'body' http://www.yoyo.com/pub/files/foo.html\n");
        return ERROR;
    }
    char **httpReq = parsingUrl(argc, argv);
    if (httpReq == NULL)
    {
        error_case_handler("red", "Memmory allocation error\n");
        return ERROR;
    }
    printParser(httpReq, argc);
    freeHttp(httpReq,argc);

    return EXIT_SUCCESS;
}