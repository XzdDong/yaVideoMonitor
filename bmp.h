#ifndef BMP_H
#define BMP_H
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




#define clip_8bit(val) (val) < 0 ? 0 :( (val) > 255 ? 255 : (val) )



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

void yuyv_to_bgr (const uint8_t *src_ptr,uint8_t *rgb_ptr,const int width, const int height);


#endif

