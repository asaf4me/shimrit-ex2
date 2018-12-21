
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum
{
    false,
    true
} bool;

#define ERROR -1

typedef struct request
{
    char *url;
    char *method;
    char *hostName;
    char *port;
    char *path;
    char *body;
    char **arguments;
    int contentLength;
    int argumentNum;
    int length;
} Request;

Request *createRequest()
{
    Request *request = (Request *)malloc(sizeof(Request));
    if (request == NULL)
    {
        printf("Memory allocation error, return null");
        return NULL;
    }
    request->url = NULL;
    request->arguments = NULL;
    request->body = NULL;
    request->hostName = NULL;
    request->path = NULL;
    request->port = NULL;
    request->contentLength = 0;
    request->argumentNum = 0;
    request->length = 50;
    return request;
}

void freeRequest(Request *request)
{
    if (request->url != NULL)
        free(request->url);
    if (request->arguments != NULL)
        free(request->arguments);
    free(request);
}

void message(char *color, char *msg)
{
    if (strcmp(color, "red") == 0)
        printf("\033[0;31mUsage: %s\033[0m", msg);
    else if (strcmp(color, "green") == 0)
        printf("\033[0;32m%s\033[0m", msg);
    else if (strcmp(color, "blue") == 0)
        printf("\033[0;34m%s\033[0m", msg);
}

bool validation(char *ptr)
{
    if (strstr(ptr, "http:") != NULL || strcmp(ptr, "-p") == 0 || strchr(ptr, '=') == NULL)
        return false;
    return true;
}

int parseArguments(char **args, Request *request, int argc, int index)
{
    int numOfArguments = atoi(*args);
    if (numOfArguments == 0 || strstr(*args, "http:") != NULL || strcmp(*args, "-p") == 0 || (numOfArguments + index) >= argc - 1)
    {
        printf("Not enough arguments... return error\n");
        return ERROR;
    }
    request->argumentNum = numOfArguments;
    int counter = 0;
    request->arguments = (char **)malloc(numOfArguments * sizeof(char *));
    for (int i = 0; i < numOfArguments; i++)
    {
        ++args;
        if (args != NULL)
        {
            if (validation(*args) == true)
            {
                request->arguments[i] = *args;
                counter++;
                request->length += strlen(*args);
            }
            else
                request->arguments[i] = NULL;
        }
    }
    if (counter != numOfArguments)
    {
        free(request->arguments);
        return ERROR;
    }
    return !ERROR;
}

int parseBody(char *body, Request *request)
{

    if (strcmp(body, "-r") == 0 || strstr(body, "http:") != NULL)
    {
        message("red", "invalid body argument\n");
        printf("Example for correct input:\n./client -r <num> x=1 x=2 -p hello http://www.google.com\n");
        return ERROR;
    }
    request->body = body;
    request->contentLength = strlen(body);
    request->length += request->contentLength;
    return !ERROR;
}

int parseUrl(char *url, Request *request)
{
    request->url = (char *)malloc(strlen(url) * sizeof(char) + 1);
    if (request->url == NULL)
    {
        printf("Memory allocation error, return ERROR[-1]");
        return ERROR;
    }
    strcpy(request->url, url);
    request->url[strlen(url)] = '\0';
    request->length += strlen(request->url);
    char *ptr = strchr(url, '/') + 2;
    request->hostName = ptr;
    ptr = strchr(ptr, ':');
    if (ptr != NULL)
    {
        *ptr = '\0';
        request->port = ++ptr;
        request->length += strlen(request->port);
    }
    else
    {
        ptr = request->hostName;
    }
    ptr = strchr(ptr, '/');
    if (ptr != NULL)
    {
        *ptr = '\0';
        request->path = ++ptr;
        request->length += strlen(request->path);
    }
    return !ERROR;
}

char *http(Request *request)
{
    if (request->url == NULL)
    {
        message("red", "Invalid url\n");
        printf("Example for correct input:\n./client http://www.google.com\n");
        return NULL;
    }
    char *posix = (char *)malloc(request->length * sizeof(char));
    if (posix == NULL)
    {
        printf("Memory allocation error, return ERROR[-1]");
        return NULL;
    }
    if (request->body != NULL)
        strcat(posix, "POST /");
    else
        strcat(posix, "GET /");
    if (request->path != NULL)
        strcat(posix, request->path);
    if (request->arguments != NULL)
    {
        strcat(posix, "?");
        for (int i = 0; i < request->argumentNum; i++)
        {
            strcat(posix, request->arguments[i]);
            if (i != request->argumentNum - 1)
                strcat(posix, "&");
        }
    }
    strcat(posix, " HTTP/1.0\r\nHost: ");
    strcat(posix, request->hostName);
    if (request->contentLength > 0)
    {
        strcat(posix, "\r\nContent-length:");
    }
    else
    {
        strcat(posix, "\r\nContent-length:");
    }

    return posix;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        message("red", "invalid command line arguments\n");
        printf("Example for correct input:\n./client -r <num> x=1 x=2 -p hello http://www.google.com\n");
        return EXIT_FAILURE;
    }
    Request *request = createRequest();
    for (int i = 1; i < argc; i++)
    {
        if (strstr(argv[i], "http://") != NULL)
        {
            if (parseUrl(argv[i], request) == ERROR)
                message("red", "url parse failed\n");
        }
        if (strcmp(argv[i], "-p") == 0)
        {
            if (i == argc - 1)
            {
                message("red", "invalid body argument\n");
                printf("Example for correct input:\n./client -r <num> x=1 x=2 -p hello http://www.google.com\n");
                freeRequest(request);
                return EXIT_FAILURE;
            }
            if (parseBody(argv[i + 1], request) == ERROR)
            {
                message("red", "body parse failed\n");
            }
        }
        if (strcmp(argv[i], "-r") == 0)
        {
            if (i == argc - 1)
            {
                message("red", "invalid body argument\n");
                printf("Example for correct input:\n./client -r <num> x=1 x=2 -p hello http://www.google.com\n");
                freeRequest(request);
                return EXIT_FAILURE;
            }
            if (parseArguments(&argv[i + 1], request, argc, i) == ERROR)
            {
                message("red", "arguments parse failed\n");
            }
        }
    }
    char *posix = http(request);
    if (posix != NULL)
    {
        printf("%s\n", posix);
    }
    free(posix);
    freeRequest(request);
    return EXIT_SUCCESS;
}