#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

typedef enum
{
    false,
    true
} bool;

void message(char *color, char *msg)
{
    if (strcmp(color, "red") == 0)
    {
        printf("\033[0;31m");
        printf("%s", msg);
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

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

void debugging(char *url, char *port, char *path, char *body)
{
    message("blue", "Host Name is\n");
    printf("%s\n", url);
    message("blue", "Port is\n");
    printf("%s\n", port);
    message("blue", "Path is\n");
    printf("%s\n", path);
    if (body != NULL)
    {
        message("green", "Body is\n");
        printf("%s\n", body);
    }
}

char *getHostName(char *url)
{
    int length = 0;
    for (int i = 0; i < strlen(url); i++)
    {
        if (url[i] == ':' || url[i] == '/')
            break;
        length++;
    }
    char *hostName = (char *)malloc(length * sizeof(char) + 1);
    if (hostName == NULL)
    {
        message("red", "Memory error: memmory allocation at get host name failed, return NULL\n");
        return NULL;
    }
    strncpy(hostName, url, length);
    hostName[length] = '\0';
    return hostName;
}

char *getPort(char *port)
{
    int length = 0;
    char *res = NULL;
    if (port == NULL)
    {
        res = (char *)malloc(strlen("80") * sizeof(char) + 1);
        if (res == NULL)
        {
            message("red", "Memory error: memmory allocation at get host port failed, return NULL\n");
            return NULL;
        }
        strcpy(res, "80");
        res[strlen("80")] = '\0';
        return res;
    }

    for (int i = 1; i < strlen(port); i++)
    {
        if (port[i] > '9' || port[i] < '0')
            break;
        length++;
    }
    res = (char *)malloc(length * sizeof(char) + 1);
    if (res == NULL)
    {
        message("red", "Memory error: memmory allocation at get host port failed, return NULL\n");
        return NULL;
    }
    int j = 0;
    for (int i = 1; i < strlen(port); i++)
    {
        if (port[i] < '9' && port[i] > '0')
        {
            res[j] = port[i];
            j++;
        }
    }
    res[length] = '\0';
    return res;
}

char *getPath(char *url)
{
    char *res = NULL;
    if (url == NULL)
    {
        res = (char *)malloc(strlen("") * sizeof(char) + 1);
        if (res == NULL)
        {
            message("red", "Memory error: memmory allocation at get path name failed, return NULL\n");
            return NULL;
        }
        res[0] = '\0';
        return res;
    }
    res = (char *)malloc(strlen(url) * sizeof(char) + 1);
    if (res == NULL)
    {
        message("red", "Memory error: memmory allocation at get path name failed, return NULL\n");
        return NULL;
    }
    strcpy(res, url);
    res[strlen(url)] = '\0';
    return res;
}

char *getBody(int argc, char *argv[], bool *post)
{
    char *body = NULL;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0)
        {
            *post = true;
            if (i < argc - 1)
            {
                body = (char *)malloc(strlen(argv[i + 1]) * sizeof(char) + 1);
                if (body == NULL)
                {
                    message("red", "Memory error: memmory allocation at http parse failed: tried to locate memory to body, return NULL\n");
                    return NULL;
                }
                strcpy(body, argv[i + 1]);
                body[strlen(argv[i + 1])] = '\0';
            }
            else
            {
                body = (char *)malloc(strlen("") * sizeof(char) + 1);
                if (body == NULL)
                {
                    message("red", "Memory error: memmory allocation at http parse failed: tried to locate memory to body, return NULL\n");
                    return NULL;
                }
                body[0] = '\0';
            }
        }
    }
    return body;
}


char *http(int argc, char *argv[])
{
    bool post = false;
    char *url = NULL, *port = NULL, *path = NULL, *hostName = NULL, *body = NULL;
    for (int i = 1; i < argc; i++)
    {
        if (startsWith("http://www.", argv[i]) == true)
            url = argv[i];
    }
    if (url == NULL)
    {
        message("red", "Input case error: Url must start with http://www., example:\n");
        printf("http://www.google.com\n");
        return NULL;
    }
    url = strchr(url, 'w');
    hostName = getHostName(url);
    if (strchr(url, ':'))
    {
        url = strchr(url, ':');
        port = getPort(url);
    }
    else
    {
        port = getPort(NULL);
    }
    url = strchr(url, '/');
    path = getPath(url);
    body = getBody(argc, argv, &post);
    debugging(hostName, port, path, body); //need to be removed before handle
    free(hostName);
    free(port);
    free(path);
    if (body != NULL)
        free(body);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        message("red", "Input case error: not enogh argument, example:\n");
        printf("client -r <NUM_OF_ARS> <arg1=?...> -p 'body' http://www.yoyo.com/pub/files/foo.html\n");
        return EXIT_FAILURE;
    }
    http(argc, argv);

    return EXIT_SUCCESS;
}