/*****************************************************************************
File name: camera.c
Description: 主程序
Author: xzdong
Date: 2019年5月10日 20:20:55
*****************************************************************************/

#include "camera.h"
 
#ifdef BMP_FILE

/*初始化bmp文件格式结构体*/

BmpInfoHeader bi;
BmpFileHeader bf;

#endif
pthread_t pthread_id[10];
static int client_sock[10];
static int client_sock_num=0;
pthread_t http_pid;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;





VideoBuffer *framebuf;
uint8_t rgb_frame_buffer[VIDEO_WIDTH*VIDEO_HEIGHT*3];
uint8_t *jpeg_frame_buffer;
int jpeg_size;
int fd;
struct v4l2_requestbuffers reqbuf;//请求内存结构体
struct v4l2_buffer v4l2buf;//缓冲区结构体
CameraInfo mycamera;



int main()
{
	printf("_____Camera Test______\n");
    int i;  
    FILE *fp,*jpeg_fp;
   
    int server_sock;
  //  CameraInfo mycamera;

    
    /* 打开摄像头设备*/
    //int fd;
    fd = open(CAMERA_DEVICE,O_RDWR, 0);
    if (fd < 0) {
        perror("Open\n");
        return -1;
    }
    mycamera.buffer_count=BUFFER_COUNT;
    mycamera.fd=fd;
    mycamera.pix_format=V4L2_PIX_FMT;
    mycamera.width=VIDEO_WIDTH;
    mycamera.height=VIDEO_HEIGHT;
    mycamera.field=V4L2_FIELD_INTERLACED;
    printf("videowidth:%d\n",mycamera.width);
    
    //struct v4l2_requestbuffers reqbuf;//请求内存结构体
   // struct v4l2_buffer buf;//缓冲区结构体

    //char *jpeg_frame_buffer_pt=jpeg_frame_buffer;
   
   
#ifdef BMP_FILE
    //Set BITMAPHEADER
     FILE *bmp;
    bmp_head_init(&bf,&bi,VIDEO_WIDTH,VIDEO_HEIGHT);
#endif
    v4l2_driver_info_get(mycamera);
    v4l2_fmt_list(mycamera);
    v4l2_fmt_set(mycamera);
    v4l2_fmt_get(mycamera);
    v4l2_buf_req(mycamera,&reqbuf);
    
    framebuf = malloc(BUFFER_COUNT*sizeof (*framebuf));/*为缓冲区映射空间的结构体申请内存,大小为缓冲区个数×结构体长度*/
    v4l2_buf_map(mycamera,framebuf,&v4l2buf);
    
    v4l2_steamon(mycamera);
    
   /*打开存储文件*/
    if ((fp = fopen(CAPTURE_FILE, "wb"))==NULL) {
        perror("open frame-data-file failed\n");
        return -1;
    }
     if ((jpeg_fp = fopen(JPEG_FILE, "wb"))==NULL) {
        perror("open jpeg file  failed\n");
        return -1;
    }
    

#ifdef BMP_FILE
    /*打开bmp存储文件*/
    if ((bmp = fopen(BMP_FILE, "wb"))==NULL) {
        perror("open bmp-file failed.\n");  
    }
#endif

//    for(i=0;i<FRAME_NUM;i++)
 /*   while(1)
    {
     //内存空间出队列
    ioctl(fd, VIDIOC_DQBUF, &buf);
   printf("buf length:%d  bytesused:%d\n",buf.length,buf.bytesused);
    fwrite(framebuf[buf.index].start, buf.bytesused, 1, fp);//原始数据写入fp文件中
   printf("save one frame success.\n"); //CAPTURE_FILE
    printf("jpeg buf address:%p\n",jpeg_frame_buffer);
    jpeg_size=compress_yuyv_to_jpeg(framebuf[buf.index].start,&jpeg_frame_buffer,80,jpeg_fp,mycamera.width,mycamera.height); //压缩为jpeg格式，压缩质量为80
    printf("jpeg buf address:%p\n",jpeg_frame_buffer);
    printf("compress yuyv to jpeg successfully!   size:%d\n",jpeg_size);
    fwrite(jpeg_frame_buffer,jpeg_size,1,jpeg_fp);//写入到jpeg文件中
   
    printf("save one  jpeg frame\n");
    //内存重新入队列
    ret = ioctl(fd, VIDIOC_QBUF, &buf);
    }*/

     pthread_create(&http_pid,NULL,(void*)&http_thread,(void*)&server_sock);
     while(1){
        //printf("main thread\n");

    }
   
    /*释放资源*/
    printf("free resouce\n");
    for (i=0; i< BUFFER_COUNT; i++) 
    {
        munmap(framebuf[i].start, framebuf[i].length);
    }


    /*关闭设备*/
   // free(jpeg_frame_buffer);
    close(fd);
    fclose(fp);
    fclose(jpeg_fp);
    
    return 0;
}
void http_thread(void *arg){
    int *server_sock = (int*)arg;
    *server_sock = server_init( &server_addr,&client_addr);

      if( accept_loop(*server_sock,&client_addr,&client_sock[client_sock_num],pthread_id[client_sock_num]) ){
          client_sock_num++;
      }




}







