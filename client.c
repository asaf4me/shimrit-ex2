
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

typedef struct req
{
    char *method;
    char *hostName;
    char *port;
    char *path;
    char *body;
    char *arguments;
} Req;

Req *createReq()
{
    Req *req = (Req *)malloc(sizeof(Req));
    if (req == NULL)
    {
        printf("Memory allocation error, return null");
        return NULL;
    }
    req->arguments = NULL;
    req->method = NULL;
    req->body = NULL;
    req->hostName = NULL;
    req->path = NULL;
    req->port = NULL;
    return req;
}

void freeReq(Req *req)
{
    if (req->arguments != NULL)
        free(req->arguments);
    if (req->method != NULL)
        free(req->method);
    if (req->body != NULL)
        free(req->body);
    if (req->hostName != NULL)
        free(req->hostName);
    if (req->port != NULL)
        free(req->port);
    if (req->path != NULL)
        free(req->path);
    free(req);
}

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
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

int arguments(char *argv, Req *req)
{
    return ERROR;
}

int parseBody(char *body, Req *req)
{
    if (strcmp(body, "-r") == 0)
    {
        message("red", "invalid body argument\n");
        printf("Example for correct input:\n./client -r <num> x=1 x=2 -p hello http://www.google.com\n");
        return ERROR;
    }
    return ERROR;
}

int parseUrl(char *url, Req *req)
{

    return ERROR;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        message("red", "invalid command line arguments\n");
        printf("Example for correct input:\n./client -r <num> x=1 x=2 -p hello http://www.google.com\n");
        return EXIT_FAILURE;
    }
    Req *req = createReq();
    for (int i = 1; i < argc; i++)
    {
        if (startsWith(argv[i], "http://"))
        {
            if (parseUrl(argv[i], req) == ERROR)
            {
                message("red", "url parse failed\n");
                freeReq(req);
            }
        }
        if (strcmp(argv[i], "-p") == 0)
        {
            if (i == argc - 1)
            {
                message("red", "invalid body argument\n");
                printf("Example for correct input:\n./client -r <num> x=1 x=2 -p hello http://www.google.com\n");
                freeReq(req);
                return EXIT_FAILURE;
            }
            if (parseBody(argv[i + 1], req) == ERROR)
            {
                message("red", "body parse failed\n");
                freeReq(req);
            }
            req->method = (char *)malloc(strlen("GET") * sizeof(char) + 1);
            strcpy(req->method, "GET");
            req->method[strlen("GET")] = '\0';
        }
    }
    if (req->method == NULL)
    {
        req->method = (char *)malloc(strlen("POST") * sizeof(char) + 1);
        strcpy(req->method, "POST");
        req->method[strlen("POST")] = '\0';
    }

    freeReq(req);
    return EXIT_SUCCESS;
}