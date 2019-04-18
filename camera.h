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
#include <linux/videodev2.h>
#include "jpeglib.h"

#define CAMERA_DEVICE "/dev/video0"
#define CAPTURE_FILE "./camera_file.yuv" 

#define BMP_FILE "./bmp_file.bmp" 
#define MJPG_FILE "./mjpg_file.mjpeg"

#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define BUFFER_COUNT 4
#define V4L2_PIX_FMT V4L2_PIX_FMT_YUYV//V4L2_PIX_FMT_YUYV
#define clip_8bit(val) (val) < 0 ? 0 :( (val) > 255 ? 255 : (val) )
#define FRAME_NUM 100

typedef struct VideoBuffer{
    void   *start;
    size_t  length;
}VideoBuffer;


//#pragma pack(1)
#ifdef BMP_FILE
typedef struct BmpFileHeader{
    uint16_t bfType;
    uint32_t bfSize;
    uint32_t  bfReserved;

    uint32_t  bfOffBits;

}BmpFileHeader;


typedef struct BmpInfoHeader{

    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;/* Number of color planes */
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;


}BmpInfoHeader;

void bmp_head_init(BmpFileHeader *bf,BmpInfoHeader *bi,const int width, const int height);

#endif
void yuyv_to_rgb (const uint8_t *src_ptr,uint8_t *rgb_ptr,const int width, const int height);

int v4l2_driver_info_get(int fd);
int v4l2_fmt_list(int fd);

int v4l2_fmt_get(int fd);
int v4l2_fmt_set(int fd);
int v4l2_buf_req(int fd,struct v4l2_requestbuffers *rebuf,const int bufnum);
int v4l2_buf_map(int fd,VideoBuffer *framebuf,struct v4l2_buffer *buf,const int bufnum);

int v4l2_steamon(int fd);


#endif
