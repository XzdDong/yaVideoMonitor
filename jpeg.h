#ifndef JPEG_H
#define JPEG_H
#include <stdio.h>
#include <unistd.h>
#include <linux/fb.h>
#include <stdlib.h>
#include <string.h>
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


#include "jpeg-ubuntu/include/jpeglib.h"
#include "v4l2_ctr.h"


#define clip_8bit(val) (val) < 0 ? 0 :( (val) > 255 ? 255 : (val) )

int compress_yuyv_to_jpeg(uint8_t *src_ptr,uint8_t **outbuffer, int quality,const int width,const int height);




#endif

