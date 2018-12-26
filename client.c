#include <assert.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum
{
    false,
    true
} bool;

#define ERROR -1
#define BUFFER 50
#define BUFFER_SIZE 1024

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

Request *create_request() // header constructor
{
    Request *request = (Request *)malloc(sizeof(Request));
    assert(request != NULL);
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

void free_request(Request *request) // free heep memory allocation
{
    if (request->url != NULL)
        free(request->url);
    if (request->arguments != NULL)
        free(request->arguments);
    if (request->body != NULL)
        free(request->body);
    free(request);
}

void message(char *color, char *msg) // colored printf
{
    if (strcmp(color, "red") == 0)
        printf("\033[0;31m%s\033[0m", msg);
    else if (strcmp(color, "green") == 0)
        printf("\033[0;32m%s\033[0m", msg);
    else if (strcmp(color, "blue") == 0)
        printf("\033[0;34m%s\033[0m", msg);
}

bool validation(char *ptr) //validation function for the parsing
{
    if (strchr(ptr, '=') == NULL || ptr[0] == '=' || ptr[strlen(ptr) - 1] == '=')
        return false;
    return true;
}

void debug(Request *request)
{
    if (request->url != NULL)
        printf("Url is: %s\n", request->url);
    if (request->body != NULL)
        printf("Body is: %s\n", request->body);
    if (request->hostName != NULL)
        printf("Host name is: %s\n", request->hostName);
    if (request->path != NULL)
        printf("Path is: %s\n", request->path);
    if (request->port != NULL)
        printf("Port is: %s\n", request->port);
    if (request->arguments != NULL)
    {
        printf("Arguments are: ");
        for (int i = 0; i < request->argumentNum; i++)
        {
            if (request->arguments[i] != NULL)
                printf("%s ,", request->arguments[i]);
        }
        printf("\n");
    }
}

int parse_arguments(int argc, char **argv, Request *request) //parse arguments, if it failed it return ERROR[-1]
{
    if (request->arguments != NULL)
    {
        message("red", "Usage: arguments already declered\n");
        return ERROR;
    }
    int index = 0, argumentsIndex = 0, counter = 0;
    for (int i = 0; i < argc; i++)
    {
        if (argv[i] != NULL)
        {
            if (strcmp(argv[i], "-r") == 0)
            {
                index = i;
                break;
            }
        }
    }
    if (index == 0)
        return !ERROR;
    if (index == argc - 1 && strcmp(argv[index], "-r") == 0)
    {
        message("red", "Usage: aruments cant be empty\n");
        return ERROR;
    }
    index++;
    int numOfArguments = atoi(argv[index]);
    if (numOfArguments == 0)
    {
        message("red", "Usage: after argument flag a number of arguments must be declered\n");
        return ERROR;
    }
    request->argumentNum = numOfArguments;
    request->arguments = (char **)malloc(numOfArguments * sizeof(char *));
    assert(request->arguments != NULL);
    for (int i = index; i <= index + numOfArguments; i++)
    {
        if (argv[i] != NULL)
        {
            if (validation(argv[i]) == true)
            {
                request->arguments[argumentsIndex] = argv[i];
                argumentsIndex++;
                counter++;
                request->length += strlen(argv[i]) + 1;
            }
        }
    }
    index = index + numOfArguments + 1;
    for (int i = index; i < argc; i++)
    {
        if (argv[i] != NULL && index < argc - 1)
        {
            if (strcmp(argv[i], "-p") != 0 && strstr(argv[i], "http://") == NULL)
                return ERROR;
        }
        else if (validation(argv[i]) == true)
            counter++;
    }
    if (counter != numOfArguments)
    {
        if (counter > numOfArguments)
        {
            message("red", "Usage: too many arguments\n");
            return ERROR;
        }
        message("red", "Usage: not enough arguments\n");
        return ERROR;
    }

    return !ERROR;
}

int parse_body(int argc, char **argv, Request *request) //parse body, if it failed it return ERROR[-1]
{
    if (request->body != NULL)
    {
        message("red", "Usage: body already declered\n");
        return ERROR;
    }
    int index = 0;
    for (int i = 0; i < argc; i++)
    {
        if (argv[i] != NULL)
        {
            if (strcmp(argv[i], "-p") == 0)
            {
                index = i;
                break;
            }
        }
    }
    if (index == 0)
        return !ERROR;
    if (index == argc - 1 && strcmp(argv[index], "-p") == 0)
    {
        message("red", "Usage: body cant be empty\n");
        return ERROR;
    }
    request->body = (char *)malloc(strlen(argv[index + 1]) * sizeof(char) + 1);
    assert(request->body != NULL);
    strcpy(request->body, argv[index + 1]);
    request->body[strlen(argv[index + 1])] = '\0';
    request->contentLength = strlen(argv[index + 1]);
    request->length += request->contentLength;
    argv[index + 1] = NULL;
    return !ERROR;
}

int parse_url(int argc, char **argv, Request *request) //parse url, if it failed it return ERROR[-1]
{
    if (request->url != NULL)
    {
        message("red", "Usage: url already declered\n");
        return ERROR;
    }
    char *url = NULL;
    for (int i = 0; i < argc; i++)
    {
        if (argv[i] != NULL)
        {
            if (strstr(argv[i], "http://") != NULL)
            {
                url = argv[i];
                break;
            }
        }
    }
    if (url == NULL)
    {
        message("red", "Usage: url not declered\n");
        return ERROR;
    }
    request->url = (char *)malloc(strlen(url) * sizeof(char) + 1);
    strcpy(request->url, url);
    request->url[strlen(url)] = '\0';
    request->length += strlen(request->url);
    char *ptr = strchr(request->url, '/') + 2;
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
    char *posix = (char *)malloc(request->length * sizeof(char));
    assert(posix != NULL);
    memset(posix, 0, request->length);
    if (posix == NULL)
    {
        perror("malloc");
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

int make_socket(Request *request, char *posix) //putting the socket up
{
    struct hostent *hp;
    struct sockaddr_in addr;
    int live = 1, sock;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
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
    message("green", "connection established!\n");
    int sum = 0, nbytes = 0;
    while (true)
    {
        int nbytes = write(sock, posix, strlen(posix));
        sum += nbytes;
        if (sum == strlen(posix))
            break;
        if (nbytes < 0)
        {
            perror("write");
            exit(1);
        }
    }
    sum = 0;
    message("blue", "message sent, server response:\n");
    while ((nbytes = read(sock, buffer, BUFFER_SIZE - 1)) != 0)
    {
        sum += nbytes;
        fprintf(stderr, "%s", buffer);
        bzero(buffer, BUFFER_SIZE);
    }
    printf("\nTotal content read: %d\n", sum);
    shutdown(sock, SHUT_RDWR);
    close(sock);
    return !ERROR;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        message("red", "Usage: invalid command line arguments\n");
        printf("Example for correct input:\n./client -r <num> x=1 x=2 -p hello http://www.google.com\n");
        return EXIT_FAILURE;
    }
    Request *request = create_request();
    int iterate = parse_body(argc, argv, request);
    if (iterate == ERROR)
    {
        message("red", "body parse failed\n");
        free_request(request);
        return EXIT_FAILURE;
    }
    iterate = parse_arguments(argc, argv, request);
    if (iterate == ERROR)
    {
        message("red", "arguments parse failed\n");
        free_request(request);
        return EXIT_FAILURE;
    }
    iterate = parse_url(argc, argv, request);
    if (iterate == ERROR)
    {
        message("red", "url parse failed\n");
        free_request(request);
        return EXIT_FAILURE;
    }
    char *posix = create_http(request);
    if (posix == NULL)
    {
        free_request(request);
        return EXIT_FAILURE;
    }
    message("green", "Parse success, message about to send:\n");
    printf("%s\n", posix);
    iterate = make_socket(request, posix);
    if (iterate == ERROR)
    {
        perror("socket");
        free(posix);
        free_request(request);
        return EXIT_FAILURE;
    }
    free(posix);
    free_request(request);
    return EXIT_SUCCESS;
}