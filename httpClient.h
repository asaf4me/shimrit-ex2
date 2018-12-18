#if !defined(HTTPCLIENT_H)
#define HTTPCLIENT_H

/*DEFINES*/
typedef enum {false,true} bool;
#define ERROR -1;

/*Void function to handle diffrent type of errors, the motivation for this function is to give more visuality way
to interect with the user from the command promt*/
void error_case_handler(char *color, char *msg);

/*Url parser, in case of right input to come, the first opertion we do is to parse the command to the http requast.
In case of failure the function will return NULL, otherwise the char***/
char **parsingUrl(const int argc, const char **argv);

/*Free the memory allocation the we have allocated during the program run.
If the function identify that the parsing is NULL it will return nothing, else, it will free all the memory allocatios*/
void freeHttp(char **httpReq,int argc);

/*Boolean function to identify whatever the user input if fit to the program or not, a true will return in case of right input, otherwise false*/
bool validation(char **httpReq);

#endif