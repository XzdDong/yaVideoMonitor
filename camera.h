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
#define BUFFER_COUNT 4
#define V4L2_PIX_FMT V4L2_PIX_FMT_YUYV//V4L2_PIX_FMT_YUYV

#define FRAME_NUM 1



#endif
