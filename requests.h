#include "requests.c"

char *post(char* host, char* endpoint, Headers headers);
char *get(char* host, char* endpoint, Headers headers);
char *_request(char* host, char* endpoint, Headers headers, char* method);
void add_header(Headers h, Header);
void error(const char *msg);