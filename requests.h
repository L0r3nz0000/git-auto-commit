#include "requests.c"

Response *post(char* host, char* endpoint, Headers headers, int port);
Response *get(char* host, char* endpoint, Headers headers, int port);
Response *_request(char* host, char* endpoint, Headers headers, char* method, int port);
void add_header(Headers *headers, Header header);
void error(const char *msg);