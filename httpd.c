#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
struct sFile {
  char filename[64];
  char *file_content;
  int size;
};

typedef struct HttpRequest http_req;
typedef struct sFile File;
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
  req->url[url_length] = '\0';

  return req;
}

// return 0 on error or return the client data;
//
char *read_from_client(int c) {
  static char buf[512];
  memset(buf, 0, sizeof(buf));
  if (read(c, buf, 511) < 0) {
    error = "read() error";
    return 0;
  } else {
    return buf;
  }
}

void http_headers(int c, int code) {
  char buf[512];
  const char *status_text = (code == 200) ? "OK" : "Not Found";

  memset(buf, 0, 512);
  snprintf(buf, 511,
           "HTTP/1.0 %d %s\r\n"
           "Server: httpd.c\r\n"
           "Cache-Control: no-store, no-cache, max-age=0, private\r\n"
           "Content-Language: en\r\n"
           "Expires: -1\r\n"
           "X-Frame-Options: SAMEORIGIN\r\n",
           code, status_text);

  write(c, buf, strlen(buf));
}

void http_response(int c, char *contenttype, char *data) {
  char buf[512];
  int content_length = strlen(data);

  // Send content type and length headers
  memset(buf, 0, 512);
  snprintf(buf, 511,
           "Content-Type: %s\r\n"
           "Content-Length: %d\r\n"
           "\r\n",
           contenttype, content_length);

  write(c, buf, strlen(buf));

  write(c, data, content_length);
}

File *read_file(char *filename) {
  struct stat st;
  char *buffer;
  int fd, bytes_read, total_bytes = 0;
  File *f;

  // Get file stats to know the size
  if (stat(filename, &st) != 0) {
    return NULL;
  }

  fd = open(filename, O_RDONLY);
  if (fd < 0) {
    return NULL;
  }

  f = malloc(sizeof(struct sFile));
  if (!f) {
    close(fd);
    return NULL;
  }

  // Allocate based on actual file size
  f->file_content = malloc(st.st_size);
  if (!f->file_content) {
    close(fd);
    free(f);
    return NULL;
  }

  strncpy(f->filename, filename, 63);
  f->filename[63] = '\0';
  f->size = st.st_size;

  // Read the entire file
  while (total_bytes < st.st_size) {
    bytes_read =
        read(fd, f->file_content + total_bytes, st.st_size - total_bytes);
    if (bytes_read <= 0) {
      close(fd);
      free(f->file_content);
      free(f);
      return NULL;
    }
    total_bytes += bytes_read;
  }

  close(fd);
  return f;
}

// returns 1 if all good else return 0;
int sendfile(int c, char *contenttype, File *file) {
  char buf[512];
  char *p;
  int n, x;

  if (!file)
    return 0;

  // Send headers first
  memset(buf, 0, 512);
  snprintf(buf, 511,
           "Content-Type: %s\r\n"
           "Content-Length: %d\r\n"
           "\r\n",
           contenttype, file->size);

  write(c, buf, strlen(buf));

  // Send file content
  n = file->size;
  p = file->file_content;
  while (n > 0) {
    x = write(c, p, (n < 512) ? n : 512);
    if (x < 1) {
      return 0;
    }
    n -= x;
    p += x;
  }

  return 1;
}

// returns 0 on error, or return
void client_handle_conn(int s, int c) {
  http_req *req;
  char *p;
  char *res;
  char filepath[256]; // Increased buffer size for safety
  File *f;

  p = read_from_client(c);
  if (!p) {
    fprintf(stderr, "error while reading client %s\n", error);
    close(c);
    return;
  }

  req = parse_http(p);
  if (!req) {
    fprintf(stderr, "error while parsing http %s\n", error);
    close(c);
    return;
  }

  printf("method : %s and route %s\n ", req->method, req->url);

  // Handle image requests
  if (!strcmp(req->method, "GET") && !strncmp(req->url, "/img/", 5)) {
    // Construct the file path - assuming images are in a subdirectory called
    // "img"
    memset(filepath, 0, sizeof(filepath));
    snprintf(filepath, sizeof(filepath) - 1, ".%s", req->url);

    f = read_file(filepath);
    if (!f) {
      res = "File not found";
      http_headers(c, 404);
      http_response(c, "text/plain", res);
    } else {
      http_headers(c, 200);
      // Determine content type based on file extension
      char *ext = strrchr(filepath, '.');
      char *content_type = "application/octet-stream"; // default
      if (ext) {
        if (!strcmp(ext, ".png"))
          content_type = "image/png";
        else if (!strcmp(ext, ".jpg") || !strcmp(ext, ".jpeg"))
          content_type = "image/jpeg";
        else if (!strcmp(ext, ".gif"))
          content_type = "image/gif";
      }

      if (!sendfile(c, content_type, f)) {
        res = "HTTP Server Error";
        http_response(c, "text/plain", res);
      }
      free(f->file_content);
      free(f);
    }
  }
  // Handle web page request
  else if (!strcmp(req->method, "GET") && !strcmp(req->url, "/app/web")) {
    res = "<html><img src='/img/cat.png' alt='super cat' /></html>";
    http_headers(c, 200);
    http_response(c, "text/html", res);
  }
  // Handle all other requests
  else {
    res = "File not found";
    http_headers(c, 404);
    http_response(c, "text/plain", res);
  }

  free(req);
  close(c);
}

int main(int argc, char *argv[]) {
  int s, c;
  char *port;
  http_req *req;
  //   char buff [512];

  //   char *template;
  //   template = "GET /something HTTP/1.1";
  // memset(buff, 0, 512);
  // strcpy(buff, template);
  //   req = parse_http(buff);

  //   printf("Method : %s \nURL : %s\n", req->method, req->url);
  //   free(req);

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
