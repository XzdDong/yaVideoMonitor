#ifndef V4L2_CTR_H
#define V4L2_CTR_H
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

#define DEBUG 


#ifndef DBG

#ifdef DEBUG
#define DBG(...) fprintf(stderr, " DBG(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#else
#define DBG(...)
#endif

#endif

#ifndef LOG
#define LOG(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stderr, "%s", _bf); syslog(LOG_INFO, "%s", _bf); }
#endif
typedef struct VideoBuffer{
    void   *start;
    size_t  length;
}VideoBuffer;

typedef struct CameraInfo{
    int      fd;
    int    width;
    int    height;
    int buffer_count;
    uint32_t pix_format;
    
    enum v4l2_field field;
    
}CameraInfo;


int v4l2_driver_info_get(CameraInfo camera);
int v4l2_fmt_list(CameraInfo camera);

int v4l2_fmt_get(CameraInfo camera);
int v4l2_fmt_set(CameraInfo camera);
int v4l2_buf_req(CameraInfo camera,struct v4l2_requestbuffers *rebuf);
int v4l2_buf_map(CameraInfo camera,VideoBuffer *framebuf,struct v4l2_buffer *buf);

int v4l2_steamon(CameraInfo camera);






#endif

