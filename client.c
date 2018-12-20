
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
    char *arguments;
    int contentLength;
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
    request->method = "GET";
    request->body = NULL;
    request->hostName = NULL;
    request->path = NULL;
    request->port = NULL;
    request->contentLength = 0;
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

int parseArguments(char *argv, Request *request)
{
    return ERROR;
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
    char *ptr = strchr(url, '/') + 2;
    request->hostName = ptr;
    ptr = strchr(ptr, ':');
    if (ptr != NULL)
    {
        *ptr = '\0';
        request->port = ++ptr;
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
    }
    return !ERROR;
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
            {
                message("red", "url parse failed\n");
                freeRequest(request);
            }
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
                freeRequest(request);
            }
            request->method = "POST";
        }
    }
    freeRequest(request);
    return EXIT_SUCCESS;
}