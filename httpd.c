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
void client_handle_conn(int s, int c){
    return ;
}
int main(int argc, char *argv[]) {
  int s, c;
  char *port;

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
