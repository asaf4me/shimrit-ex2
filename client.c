
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
#define BUFFER_SIZE 4096

/*Struct to hold all the command line arguments together*/
typedef struct request
{
    char *url;         //make copy of the url
    char *hostName;    //parse url to host name
    char *port;        //hold port
    char *path;        //hold path
    char *body;        //hold body if -p flag is up
    char **arguments;  //catch all the desired arguments
    int contentLength; //the body length for the header
    int argumentNum;   //hold the number of the arguments
    int length;        //total length for the header
} Request;

Request *create_request() //header constructor
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

void free_request(Request *request) //free heep memory allocation
{
    if (request->url != NULL)
        free(request->url);
    if (request->arguments != NULL)
        free(request->arguments);
    free(request);
}

void message(char *color, char *msg) //making evrything more beautiful :-)
{
    if (strcmp(color, "red") == 0)
        printf("\033[0;31mUsage: %s\033[0m", msg);
    else if (strcmp(color, "green") == 0)
        printf("\033[0;32m%s\033[0m", msg);
    else if (strcmp(color, "blue") == 0)
        printf("\033[0;34m%s\033[0m", msg);
}

bool validation(char *ptr) //validation function for the parsing
{
    if (strstr(ptr, "http:") != NULL || strcmp(ptr, "-p") == 0 || strchr(ptr, '=') == NULL)
        return false;
    return true;
}

int parse_arguments(char **args, Request *request, int argc, int index) //parse arguments, if it failed it return ERROR[-1]
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

int parse_body(char *body, Request *request) //parse body, if it failed it return ERROR[-1]
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

int parse_url(char *url, Request *request) //parse url, if it failed it return ERROR[-1]
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

char *create_http(Request *request) //connecting the http header together, if it failed it return NULL
{
    if (request->url == NULL)
    {
        return NULL;
    }
    char *posix = (char *)malloc(request->length * sizeof(char));
    memset(posix, 0, request->length);
    if (posix == NULL)
    {
        printf("Memory allocation error, return null");
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

int make_socket(Request *request) //putting the socket up
{
    struct hostent *hp;
    struct sockaddr_in addr;
    int live = 1, sock;
    if ((hp = gethostbyname(request->hostName)) == NULL) //converting host name to ip
    {
        herror("gethostbyname");
        return ERROR;
    }
    memcpy(&addr.sin_addr, hp->h_addr, hp->h_length); //coping the host to the struct
    if (request->port != NULL)                        //if there is diffrent port then 80
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
    message("green", "connecting...\n");
    if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
    {
        message("red", "connect failed\n");
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
    Request *request = create_request();
    if (request == NULL)
        return EXIT_FAILURE;
    for (int i = 1; i < argc; i++)
    {
        if (strstr(argv[i], "http://") != NULL) //identify the url, send it to a parse function
        {
            if (parse_url(argv[i], request) == ERROR)
            {
                message("red", "url parse failed\n");
                free_request(request);
                return EXIT_FAILURE;
            }
        }
        if (strcmp(argv[i], "-p") == 0) //identify the body, send it to a parse function
        {
            if (i == argc - 1) //check what ever if the flag is at the end of the command
            {
                message("red", "invalid body argument\n");
                printf("Example for correct input:\n./client -r <num> x=1 x=2 -p hello http://www.google.com\n");
                free_request(request);
                return EXIT_FAILURE;
            }
            if (parse_body(argv[i + 1], request) == ERROR)
            {
                message("red", "body parse failed\n");
                free_request(request);
                return EXIT_FAILURE;
            }
        }
        if (strcmp(argv[i], "-r") == 0) //identify the arguments, send it to a parse function
        {
            if (i == argc - 1) //check what ever if the flag is at the end of the command
            {
                message("red", "invalid body argument\n");
                printf("Example for correct input:\n./client -r <num> x=1 x=2 -p hello http://www.google.com\n");
                free_request(request);
                return EXIT_FAILURE;
            }
            if (parse_arguments(&argv[i + 1], request, argc, i) == ERROR)
            {
                message("red", "arguments parse failed\n");
                free_request(request);
                return EXIT_FAILURE;
            }
        }
    }

    char *posix = create_http(request);
    if (posix == NULL)
    {
        free_request(request);
        return EXIT_FAILURE;
    }
    int fd;
    char buffer[BUFFER_SIZE];
    message("green", "Parse success, message about to send:\n");
    printf("%s\n\n", posix);
    if ((fd = make_socket(request)) == ERROR)
    {
        free(posix);
        free_request(request);
        return EXIT_FAILURE;
    }
    printf("connection established! server response: \n\n");
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
    free_request(request);
    return EXIT_SUCCESS;
}