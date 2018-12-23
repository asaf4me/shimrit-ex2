
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

typedef enum
{
    false,
    true
} bool;

#define ERROR -1
#define BUFFER 50
#define BUFFER_SIZE 2048

typedef struct request
{
    char *url;
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
    request->length = BUFFER;
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
    if (request->arguments == NULL)
    {
        printf("Memory allocation error, return ERROR[-1]");
        return ERROR;
    }
    for (int i = 0; i < numOfArguments; i++)
    {
        ++args;
        if (args != NULL)
        {
            if (validation(*args) == true)
            {
                request->arguments[i] = *args;
                counter++;
                request->length += strlen(*args) + 1;
            }
        }
        else
        {
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
        request->length += strlen(request->port) + 1;
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
        request->length += strlen(request->path) + 1;
    }
    return !ERROR;
}

char *http(Request *request)
{
    if (request->url == NULL)
    {
        return NULL;
    }
    char *posix = (char *)malloc(request->length * sizeof(char));
    memset(posix, 0, request->length);
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
            if (request->arguments[i] != NULL)
            {
                strcat(posix, request->arguments[i]);
                if (i != request->argumentNum - 1)
                    strcat(posix, "&");
            }
        }
    }
    strcat(posix, " HTTP/1.0\r\nHost: ");
    strcat(posix, request->hostName);
    if (request->port != NULL)
    {
        strcat(posix, ":");
        strcat(posix, request->port);
    }
    if (request->contentLength > 0)
    {
        strcat(posix, "\r\nContent-length:");
        char buffer[BUFFER];
        sprintf(buffer, "%d", request->contentLength);
        strcat(posix, buffer);
    }
    strcat(posix, "\r\n\r\n");
    if (request->body != NULL)
        strcat(posix, request->body);
    return posix;
}

int make_socket(Request *request)
{
    struct hostent *hp;
    struct sockaddr_in addr;
    int live = 1, sock;
    if ((hp = gethostbyname(request->hostName)) == NULL)
    {
        perror("gethostbyname");
        return ERROR;
    }
    memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
    if (request->port != NULL)
        addr.sin_port = htons(atoi(request->port));
    else
        addr.sin_port = htons(80);
    addr.sin_family = AF_INET;
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&live, sizeof(int));
    if (sock == -1)
    {
        perror("setsockopt");
        return ERROR;
    }
    if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("connect");
        return ERROR;
    }
    return sock;
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
    if (request == NULL)
        return EXIT_FAILURE;
    for (int i = 1; i < argc; i++)
    {
        if (strstr(argv[i], "http://") != NULL)
        {
            if (parseUrl(argv[i], request) == ERROR)
            {
                message("red", "url parse failed\n");
                freeRequest(request);
                return EXIT_FAILURE;
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
                return EXIT_FAILURE;
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
                freeRequest(request);
                return EXIT_FAILURE;
            }
        }
    }
    char *posix = http(request);
    if (posix != NULL)
    {
        int fd;
        char buffer[BUFFER_SIZE];
        message("green", "Parse success, message about to send:\n");
        printf("%s\n\n", posix);
        if ((fd = make_socket(request)) == ERROR)
        {
            free(posix);
            freeRequest(request);
            return EXIT_FAILURE;
        }
        write(fd, posix, strlen(posix));
        bzero(buffer, BUFFER_SIZE);
        while (read(fd, buffer, BUFFER_SIZE - 1) != 0)
        {
            fprintf(stderr, "%s", buffer);
            bzero(buffer, BUFFER_SIZE);
        }
        shutdown(fd, SHUT_RDWR);
        close(fd);
        free(posix);
    }
    freeRequest(request);
    return EXIT_SUCCESS;
}