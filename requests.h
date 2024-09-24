#include "requests.c"

void post(char* host, char* endpoint, char** headers, int headers_len, char** data, int data_len, char response[4096]);
void add_header(Headers h, Header);
void error(const char *msg);

typedef struct {
  char *name;
  char *value;
} Header;

typedef struct {
  Header *headers;
  int size = 0;
} Headers;