#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */

typedef struct {
  char *name;
  char *value;
} Header;

// Struct per gli headers
typedef struct {
  Header *headers;
  int size;
} Headers;

// Struct per il body della richiesta
typedef Header _Data;
typedef struct {
  _Data *data;
  int size;
} Data;

// Struct per i cookies
typedef Header _Cookie;
typedef struct {
  _Cookie *data;
  int size;
} Cookies;

// Struct per tornare la rispoosta http
typedef struct {
  char *code;
  char *text;
  Headers headers;
  Cookies cookies;
} Response;

Response *post(char* host, char* endpoint, Headers headers, int port);
Response *get(char* host, char* endpoint, Headers headers, int port);
Response *_request(char* host, char* endpoint, Headers headers, char* method, int port);
void add_header(Headers *headers, Header header);
void error(const char *msg);
char *headers_string(Headers headers);

void error(const char *msg) { perror(msg); exit(0); }

void add_header(Headers *headers, Header header) {
    // Alloca la memoria per il nuovo header
    if (headers->size == 0) {
        headers->headers = malloc(sizeof(Headers));
    } else {
        headers->headers = realloc(headers->headers, sizeof(Header) * (headers->size+1));
    }

    headers->headers[headers->size] = header;
    headers->size ++;
}

// Ritorna la stringa a partire dalla lista degli headers
char *headers_string(Headers headers) {
    char *headers_str = malloc(sizeof(char));
    headers_str[0] = '\0';

    for (int i = 0; i < headers.size; i++) {
        if (headers.headers[i].name == NULL || headers.headers[i].value == NULL) {
            error("Error: header name or value is NULL");
        }

        headers_str = strcat(headers_str, headers.headers[i].name);
        headers_str = strcat(headers_str, ": ");
        headers_str = strcat(headers_str, headers.headers[i].value);
        headers_str = strcat(headers_str, "\n");
    }
    return headers_str;
}

Response *post(char* host, char* endpoint, Headers headers, int port) {
    return _request(host, endpoint, headers, "POST", port);
}

Response *get(char* host, char* endpoint, Headers headers, int port) {
    return _request(host, endpoint, headers, "GET", port);
}

Response *_request(char* host, char* endpoint, Headers headers, char* method, int port) {
    /* first what are we going to send and where are we going to send it? */
    int portno = port;
    char *message_fmt = "%s %s HTTP/1.0\n%s\r\n\r\n"; // %s = method, %s = endpoint, %s = headers

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char message[1024];

    /* Initial size of response buffer */
    int response_size = 1024;
    Response *response = (Response *)malloc(sizeof(Response));
    if (response == NULL) {
        error("ERROR allocating memory for response");
    }
    char *response_string = (char *)malloc(response_size);
    if (response_string == NULL) {
        error("ERROR allocating memory for response");
    }

    char *headers_str = headers_string(headers);

    /* fill in the parameters */
    sprintf(message, message_fmt, method, endpoint, headers_str);
    printf("Request:\n%s\n",message);

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) error("ERROR no such host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd,message+sent,total-sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    /* receive the response */
    received = 0;
    do {
        /* Expand the buffer if necessary */
        if (received + 512 >= response_size) {
            response_size *= 2;
            response_string = (char *)realloc(response_string, response_size);
            if (response_string == NULL) {
                error("ERROR reallocating memory for response");
            }
        }

        bytes = read(sockfd, response_string + received, 512);
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received += bytes;
    } while (1);

    /* Null-terminate the response */
    response_string[received] = '\0';

    /* close the socket */
    close(sockfd);

    response->text = response_string;

    /*
     * if the number of received bytes is the total size of the
     * array then we have run out of space to store the response
     * and it hasn't all arrived yet - so that's a bad thing
     */
    if (received == total)
        error("ERROR storing complete response from socket");
    
    return response;
}
