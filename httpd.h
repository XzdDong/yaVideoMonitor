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
#define PORT 8080
#define BACKLOG 3
#define FRAME_NUM 1
#define ERROR_PATH "./web/error"
int server_init( struct sockaddr_in *server_addr,struct sockaddr_in *client_addr);

int accept_loop(int server_fd,struct sockaddr_in *client_addr,int *client_fd,pthread_t pthread_id);
int process_server(void *client_fd);

int http_req_parse(int client);
int get_line(int sock, char *buf, int size);
int my_send(int fd,char *sendbuf);
int my_recv(int fd,char *recvmsg);
int send_error(int client,int error);
int send_file(int client,char *path);




#endif

