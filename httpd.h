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

#define ERROR_PATH "./web/error"
#define BOUNDARY "boundarydonotcross"
#define STD_HEADER "Connection:keep-alive\r\n" \
    "Server:yaVideomonitor\r\n" 
   // "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
   //"Pragma: no-cache\r\n" 


int server_init( struct sockaddr_in *server_addr,struct sockaddr_in *client_addr);

int accept_loop(int server_fd,struct sockaddr_in *client_addr,int *client_fd,pthread_t pthread_id);
int process_server(void *client_fd);

int http_req_parse(int client);
int get_line(int sock, char *buf, int size);
int my_send(int fd,char *sendbuf);
int my_recv(int fd,char *recvmsg);
int send_error(int client,int error);
int send_file(int client,char *path);
int send_jpeg_frame(int client,uint8_t *jpegbuf,int size);




#endif

