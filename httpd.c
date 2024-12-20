#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// global and macros
#define LISTEN_ADDR "127.0.0.1"
char *error;
// structs
struct HttpRequest {
  char method[8];
  char url[128];
};

typedef struct HttpRequest http_req;
// returns 0 if the socket gets error else returns *fd type of result
int srv_init(int port) {
  int s;
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    error = "socket() error";
    printf("ERROR while forming socket in the socket function");
    return 0;
  }
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(LISTEN_ADDR);
  server.sin_port = htons(port);

  if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
    error = "bind() error";
    close(s);
    printf("Error while binding the socket ");
  }
  if (listen(s, 5) < 0) {
    error = "listen() error";
    close(s);
    printf("Error while listening");
  }
  return s;
}

// returns 0 on error, or returns the new client's socket id
int client_accept(int s) {
  int c;
  socklen_t address_length = 0;
  struct sockaddr_in client;
  memset(&client, 0, sizeof(client));
  c = accept(s, (struct sockaddr *)&client, &address_length);
  if (c < 0) {
    error = "accept() error on client side";
    return 0;
  }
  return c;
}

// returns 0 on error, or return
void client_handle_conn(int s, int c) { return; }

// returns 0 on error or returns of type http_req struct
http_req *parse_http(char *str) {
  http_req *req;
  char *method_end, *url_start, *url_end;

  req = malloc(sizeof(http_req));
  if (!req) {
    error = "parse_http() memory allocation error";
    return NULL;
  }

  method_end = strchr(str, ' ');
  if (!method_end) {
    error = "parse_http() NO SPACE FOUND after method";
    free(req);
    return NULL;
  }

  // Copy the method into req->method
  size_t method_length = method_end - str;
  if (method_length >= sizeof(req->method)) {
    error = "parse_http() method too long";
    free(req);
    return NULL;
  }
  strncpy(req->method, str, method_length);
  req->method[method_length] = '\0'; // Null-terminate the method string

  // Find the start of the URL (skip the space after the method)
  url_start = method_end + 1;

  // Find the end of the URL (space after URL)
  url_end = strchr(url_start, ' ');
  if (!url_end) {
    error = "parse_http() NO SPACE FOUND after URL";
    free(req);
    return NULL;
  }

  // Copy the URL into req->url
  size_t url_length = url_end - url_start;
  if (url_length >= sizeof(req->url)) {
    error = "parse_http() URL too long";
    free(req);
    return NULL;
  }
  strncpy(req->url, url_start, url_length);
  req->url[url_length] = '\0'; // Null-terminate the URL string

  return req;
}

int main(int argc, char *argv[]) {
  int s, c;
  char *port;
  http_req *req;

  char *template;
  template = "GET /something HTTP/1.1";

  req = parse_http(template);

  printf("Method : %s \nURL : %s\n", req->method, req->url);
  free(req);

  if (argc < 2) {
    fprintf(stderr, "Usage is %s <listening_port> ", argv[0]);
    return -1;
  } else {
    port = argv[1];
  }

  s = srv_init(atoi(port));
  if (!s) {
    fprintf(stderr, "%s\n", error);
    return -1;
  }
  printf("listening on %s:%s\n", LISTEN_ADDR, port);

  while (1) {
    c = client_accept(s);
    if (!c) {
      fprintf(stderr, "%s\n", error);
      continue;
    }
    printf("Incoming connection \n");

    if (!fork()) {
      client_handle_conn(s, c);
    }
    // for the main process return the nwe process Id
    // for the new process returns 0
  }

  return -1;
}
