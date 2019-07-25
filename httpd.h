#ifndef HTTPD_H
#define HTTPD_H

#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>
#include <string.h>
#include "ctype.h"
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/time.h>
#define PORT 8080
#define BACKLOG 3

#define WEB_PATH "./web"
#define ERROR_PATH "./web/error"
#define BOUNDARY "boundarydonotcross"
#define STD_HEADER  "Connection: keep-alive\r\n" \
"Server: yaVideomonitor\r\n"\
    //"Keep-Alive:timeout "

   // "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
   //"Pragma: no-cache\r\n" 
static const struct {
    const char *dot_extension;
    const char *mimetype;
} mimetypes[] = {
    { ".html", "text/html" },
    { ".htm",  "text/html" },
    { ".css",  "text/css" },
    { ".js",   "text/javascript" },
    { ".txt",  "text/plain" },
    { ".jpg",  "image/jpeg" },
    { ".jpeg", "image/jpeg" },
    { ".png",  "image/png"},
    { ".gif",  "image/gif" },
    { ".ico",  "image/x-icon" },
    { ".swf",  "application/x-shockwave-flash" },
    { ".cab",  "application/x-shockwave-flash" },
    { ".jar",  "application/java-archive" },
    { ".json", "application/json" }
};

int server_init( struct sockaddr_in *server_addr,struct sockaddr_in *client_addr);

int accept_loop(int server_fd,struct sockaddr_in *client_addr,int *client_fd,pthread_t pthread_id);
int process_server(void *client_fd);

int http_req_parse(int client);
int get_line(int sock, char *buf, int size);
int my_send(int fd,char *sendbuf);
int my_recv(int fd,char *recvmsg);
int send_error(int client,int error);
int send_file(int client,char *path);
int send_stream(int client);

int send_jpeg_frame(int client,char *jpegbuf,int size);




#endif

