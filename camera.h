#ifndef CAMERA_H
#define CAMERA_H
#include <stdio.h>
#include <unistd.h>
#include <linux/fb.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <asm/types.h>        


#include "jpeg.h"
#include "bmp.h"
#include "v4l2_ctr.h"
#include "httpd.h"

#define CAMERA_DEVICE "/dev/video0"
#define CAPTURE_FILE "./camera_file.yuv" 

//#define BMP_FILE "./bmp_file.bmp" 
#define MJPG_FILE "./mjpg_file.mjpeg"
#define JPEG_FILE "./jpeg_file.jpg"

#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define BUFFER_COUNT 5
#define V4L2_PIX_FMT V4L2_PIX_FMT_YUYV//V4L2_PIX_FMT_YUYV

extern uint8_t *jpeg_frame_buffer;
extern int jpeg_size;
extern int fd;
extern struct v4l2_requestbuffers reqbuf;//请求内存结构体
extern struct v4l2_buffer v4l2buf;//缓冲区结构体
extern VideoBuffer *framebuf;
extern struct CameraInfo mycamera;

void http_thread(void *arg);



#endif
