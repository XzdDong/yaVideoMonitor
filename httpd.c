/*****************************************************************************
File name: httpd.c
Description: 解析http
Author: xzdong
*****************************************************************************/


#include "httpd.h"

#include "camera.h"

/*************************************************
Function: server_init()
Description: 初始化tcp连接,并开始监听
Input:
Output: 
Return: 
Others:
*************************************************/

int server_init( struct sockaddr_in *server_addr,struct sockaddr_in *client_addr){
    int server_fd;
    printf("Server initialization starts\n");
 
    server_fd = socket(AF_INET,SOCK_STREAM,0);
    
    if(server_fd<0){
        perror("server:create");
        exit(-1); 
    }
    printf("sever socket fd:%d\n",server_fd);
    bzero(server_addr,sizeof(struct sockaddr_in)); 
    server_addr->sin_family = AF_INET;
    (server_addr->sin_addr).s_addr = htonl(INADDR_ANY);
    server_addr->sin_port = htons(PORT); 
 
    if(bind(server_fd,(struct sockaddr*)server_addr,sizeof(*server_addr))<0){

        perror("server:socket  bind");
        exit(-1);

    }
    if(listen(server_fd,BACKLOG)<0){
        perror("server:listen");
        exit(-1);

    }
    printf("Server initialization completed\n");
    
    return server_fd; 
}
/*************************************************
Function: accept_loop
Description: 循环创建子线程接收socket连接请求
Input:
Output: 
Return: 
Others:
*************************************************/

int accept_loop(int server_fd,struct sockaddr_in *client_addr,int *client_fd,pthread_t pthread_id){

     void* threadval;

    static socklen_t peer_addr_size; 


    peer_addr_size=sizeof(struct sockaddr);

    *client_fd=accept(server_fd,(struct sockaddr*)client_addr,&peer_addr_size);
    printf("client fd:%d\n",*client_fd);
    if(client_fd<0){
        perror("accept");
        exit(-1);
    }
    if(pthread_create(&pthread_id,NULL,(void*)&process_server,(void*)client_fd)!=0){
        perror("thread create"); 
        exit(-1);
    }
    return 1;
      // pthread_join(pthread_id,threadval);    


}
/*************************************************
Function: process_server
Description: server执行函数
Input:
Output: 
Return: 
Others:
*************************************************/

int process_server(void *client_fd){
    printf("process server\n");
    int client_sock=*(int*)client_fd;
    /*解析http请求*/      
    http_req_parse(client_sock);

           //sleep(1);
           //printf("close: %d\n",client_sock);
           //close(client_sock);
    return 0;
       
}
/*************************************************
Function: http_req_parse
Description: 解析HTTP请求
Input:
Output: 
Return: 
Others:
*************************************************/
int http_req_parse(int client){

    char url[100];
    char method[20];
    char buf[255];
    char path[255];
    int charnum,i = 0,j = 0;
    char *query_thing=NULL;
    char *p_info=NULL;
    int cgi = 0;
    /*接收数据*/
    char sendbuf[65535];
    while(1){
         memset(buf,0,sizeof(buf));
         memset(buf,0,sizeof(url));
         memset(buf,0,sizeof(path));
        
         charnum=get_line(client,buf,sizeof(buf));//读取一行数据到buf
         if(charnum == 0){
             printf("client close \n");
             close(client); //返回0 客户端关闭连接
             break;
         }
        
        if((p_info=strstr(buf,"GET"))==NULL) continue;

        else{
            p_info+=(strlen("GET"));
                while(isspace((int)*p_info)) p_info++;
                while(!isspace((int)*p_info) && i<sizeof(url)){
                       url[i] = *p_info;
                       i++;
                       p_info++;
                } 
           url[i] = '\0';
           if(strcmp(url,"/")==0){
                strcpy(url,"/index.html");
            }
            if(strstr(url,"?stream")){
                printf("send stream  to %d\n",client);
                send_stream(client);
            }
            else{ 
                sprintf(path,"%s%s",WEB_PATH,url); 
                printf("path:%s\n",path);
                printf("send file  to %d\n",client);
               if(send_file(client,path)) {
                    printf("send file completed\n");
                   // return 0;
                }
            }
            i=0;
        
        }
    }

    return 1;

}
/*************************************************
Function: send_error
Description: 发送error
Input:
Output: 
Return: 
Others:
*************************************************/

int send_error(int client,int error){
    int fd ;
    char buf[255];
    char error_str[255];
    char error_file_path[255];
    struct stat st;
    memset(error_str,0,sizeof(error_str));
    memset(error_file_path,0,sizeof(error_file_path));
    switch (error)
        {
        case 400:strcpy(error_str,"Bad Request");break;
        case 404:strcpy(error_str,"Not Found");break;
        case 501:strcpy(error_str,"Not Implemented");break;
        default:break;
        }
    sprintf(error_file_path,"%s/%d.html",ERROR_PATH,error);
    fd = open(error_file_path,0,'r');  
    if(fd < 0){
        memset(buf,0,sizeof(buf));        
        sprintf(buf,"HTTP/1.1 400 Bad Request\r\n" \
               "Content-Type:text/plain\r\n" \
               STD_HEADER\
                   "<HTML>"\
                         "<TITLE>SERVER ERROR</TITLE>"\
                         "<BODY>server error</BODY>"\
                   "</HTML>"

                );
        send(client,buf,strlen(buf),0);
        return 0;
    }
    fstat(fd, &st);

    sprintf(buf,"HTTP/1.1 %d %s\r\n" \
               "Content-Type:text/html\r\n" \
               STD_HEADER\
               "Content-Length:%ld\r\n"\
                "\r\n"
        ,error,error_str,st.st_size);  
    send(client,buf,strlen(buf),0);
  
    if(sendfile(client,fd,0,st.st_size)==-1){
        perror("send error file");
        return 0;
    }
    close(fd); 
    return st.st_size;
}

int send_file(int client,char *path){
    int fd;
    char buf[255];
    char *mimetype =NULL;
    struct stat st;
    
    fd = open(path,0,'r');
    if(fd < 0){
        send_error(client,404);
        perror("server send");
        return 0;
    }
    fstat(fd, &st);

    /* determine mime-type */
    for(int i = 0; i < sizeof(mimetypes)/sizeof(mimetypes[0]) ; i++) {
   
        if(strstr(path,mimetypes[i].dot_extension)) {
            mimetype = (char *)mimetypes[i].mimetype;
            break;
        }
    }
    
    if(mimetype){
      
        sprintf(buf,"HTTP/1.1 200 OK\r\n"\
                   "Content-Type:%s \r\n"\
                   STD_HEADER\
                   "Content-Length:%ld\r\n"\
                   "\r\n"\
            ,mimetype,st.st_size);
        send(client,buf,strlen(buf),0);
    }
    else{
        send_error(client,400);
        close(client);
        return 0;
    
    }
    
    if(sendfile(client,fd,0,st.st_size)==-1){
        perror("sendfile");
        return 0;
    }
    close(fd);   
    return st.st_size;
}

/*multipart/x-mixed-replace用于服务器推送server push和HTTP流 stream*/

int send_stream(int client){

    char buf[255];
    sprintf(buf, "HTTP/1.1 200 OK\r\n" \
            STD_HEADER \
            "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
            "\r\n" \
            "--" BOUNDARY "\r\n");

    if(write(client, buf, strlen(buf)) < 0) {
        return 0;
    }
      for(;;) { 
           // printf("sendjpegbuf\n");
           //内存空间出队列
            ioctl(fd, VIDIOC_DQBUF, &v4l2buf);

           //printf("jpeg buf index%d\n",v4l2buf.index);
            //sprintf(name,"./%d.jpg",v4l2buf.index);
           jpeg_size=compress_yuyv_to_jpeg(framebuf[v4l2buf.index].start,&jpeg_frame_buffer,80,mycamera.width,mycamera.height); //压缩为jpeg格式，压缩质量为80
          // printf("jpeg addr:%p\n",jpeg_frame_buffer);
          // printf("jpeg size:%d\n",jpeg_size);
    
            /* 内存重新入队列*/
          // memcpy(sendbuf,jpeg_frame_buffer,jpeg_size);
             
           send_jpeg_frame(client,jpeg_frame_buffer,jpeg_size);
           //free(jpeg_frame_buffer);
         
            ioctl(fd, VIDIOC_QBUF, &v4l2buf);
          
        }
  

}


int send_jpeg_frame(int client,uint8_t *jpegbuf,const int size){

    char buf[255];
    struct timeval timestamp;
      

    gettimeofday(&timestamp,NULL);
    sprintf(buf,"Content-Type:image/jpeg \r\n"\
               "Content-Length: %d\r\n"\
               "X-Timestamp: %d.%06d\r\n"\
               STD_HEADER\
               "\r\n"
       ,size,(int)timestamp.tv_sec,(int)timestamp.tv_usec);
   if( write(client,buf,strlen(buf))<0)
        perror("send buf");
    if(write(client,jpegbuf,size)<0)
         perror("send jpegbuf");
    sprintf(buf, "\r\n--" BOUNDARY "\r\n");//BOUNDARY边界以区分两帧图片
    send(client, buf, strlen(buf),0);
    return 1;
  
}

/*************************************************
Function: get_line
Description: 从socket获得一行数据
Input:
Output: 
Return: 
Others:读取以“\r\n”结尾的一行数据
*************************************************/

int get_line(int client_sock, char *buf, int size){
 int i = 0;
 char c = '\0';
 int n;

    while ((i < size - 1) && (c != '\n')) {
       n = recv(client_sock, &c, 1, 0);     //取一个字符到c，返回n
     
        if (n > 0)                    //如果有
        {
            if (c == '\r')               //如果是回车
            {
                n = recv(client_sock, &c, 1, MSG_PEEK); //MSG_PEEK标志，不移除socket中的buf
            if ((n > 0) && (c == '\n'))     //如果回车符后面还有数据并且为换行
                recv(client_sock, &c, 1, 0);      //读取换行
            else 
                c = '\n';                   //否则令c为换行符
            }
            buf[i] = c;                  //此时的第i+1字符为c
            i++;    
        }
        else
        c = '\n';                    //数据读完令c为换行符
    }
    buf[i] = '\0';                  //读完行，最后一个为0
    

 return(i);//返回行字符数
}





