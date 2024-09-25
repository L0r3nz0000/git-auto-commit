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

typedef struct {
  Header *headers;
  int size;
} Headers;

typedef Header _Data;
typedef struct {
  _Data *data;
  int size;
} Data;

void error(const char *msg) { perror(msg); exit(0); }

void add_header(Headers headers, Header header) {
    // Alloca la memoria per il nuovo header
    if (headers.size == 0) {
        headers.headers = malloc(sizeof(Headers));
    } else {
        headers.headers = realloc(headers.headers, sizeof(Header) * (headers.size+1));
    }

    headers.headers[headers.size] = header;
    headers.size ++;
}

char *post(char* host, char* endpoint, Headers headers) {
    return _request(host, endpoint, headers, "POST");
}

char *get(char* host, char* endpoint, Headers headers) {
    return _request(host, endpoint, headers, "GET");

}

// TODO: implementare funzione per l asequnza di fibonacci

char *_request(char* host, char* endpoint, Headers headers, char* method) {
    /* first what are we going to send and where are we going to send it? */
    int portno =        80;
    char *message_fmt = "%s %s HTTP/1.0\r\n\r\n";

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char message[1024];

    /* Initial size of response buffer */
    int response_size = 1024;
    char *response = (char *)malloc(response_size);
    if (response == NULL) {
        error("ERROR allocating memory for response");
    }

    /* fill in the parameters */
    sprintf(message, message_fmt, method, endpoint);
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
            response = (char *)realloc(response, response_size);
            if (response == NULL) {
                error("ERROR reallocating memory for response");
            }
        }

        bytes = read(sockfd, response + received, 512);
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received += bytes;
    } while (1);

    /* Null-terminate the response */
    response[received] = '\0';

    /* close the socket */
    close(sockfd);

    /* process response */
    printf("Response:\n%s\n", response);

    /*
     * if the number of received bytes is the total size of the
     * array then we have run out of space to store the response
     * and it hasn't all arrived yet - so that's a bad thing
     */
    if (received == total)
        error("ERROR storing complete response from socket");
    
    return response;
}
