
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
    request->arguments = NULL;
    request->method = NULL;
    request->body = NULL;
    request->hostName = NULL;
    request->path = NULL;
    request->port = NULL;
    request->contentLength = 0;
    return request;
}

void freeRequest(Request *request)
{
    if (request->arguments != NULL)
        free(request->arguments);
    if (request->method != NULL)
        free(request->method);
    if (request->body != NULL)
        free(request->body);
    if (request->hostName != NULL)
        free(request->hostName);
    if (request->port != NULL)
        free(request->port);
    if (request->path != NULL)
        free(request->path);
    free(request);
}

void message(char *color, char *msg)
{
    if (strcmp(color, "red") == 0)
    {
        printf("\033[0;31m");
        printf("Usage: %s", msg);
        printf("\033[0m");
    }
    else if (strcmp(color, "green") == 0)
    {
        printf("\033[0;32m");
        printf("%s", msg);
        printf("\033[0m");
    }
    else if (strcmp(color, "blue") == 0)
    {
        printf("\033[0;34m");
        printf("%s", msg);
        printf("\033[0m");
    }
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
    request->body = (char *)malloc(strlen(body) * sizeof(char) + 1);
    if (request == NULL)
    {
        printf("Memory allocation error, return null");
        return ERROR;
    }
    strcpy(request->body, body);
    request->body[strlen(body)] = '\0';
    request->contentLength = strlen(body);
    return !ERROR;
}

int parseUrl(char *url, Request *request)
{
    char *begin = strchr(url, '/'), *end = strchr(begin, ':');
    request->hostName = (char *)malloc(strlen(begin) * sizeof(char));
    if (end != NULL)
    {
        strncpy(request->hostName, begin + 2, end - begin - 2);
        printf("host name is: %s\n", request->hostName);
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
            request->method = (char *)malloc(strlen("GET") * sizeof(char) + 1);
            strcpy(request->method, "GET");
            request->method[strlen("GET")] = '\0';
        }
    }
    if (request->method == NULL)
    {
        request->method = (char *)malloc(strlen("POST") * sizeof(char) + 1);
        strcpy(request->method, "POST");
        request->method[strlen("POST")] = '\0';
    }

    freeRequest(request);
    return EXIT_SUCCESS;
}