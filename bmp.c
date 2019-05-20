/*****************************************************************************
File name: bmp.c
Description: BMP格式转换
Author: xzdong
*****************************************************************************/

#include "bmp.h"
 



void bmp_head_init(BmpFileHeader *bf,BmpInfoHeader *bi,const int width, const int height){
    /*Set BITMAPFILEHEADER 设置BMP文件头*/
    bf->bfType = 0x4d42;
    bf->bfSize = 54 + bi->biSizeImage;     
    bf->bfReserved = 0;
    bf->bfOffBits = 54;
    /*设置BMP格式头*/
    bi->biSize = 40;
    bi->biWidth = width;
    bi->biHeight = height;
    bi->biPlanes = 1;
    bi->biBitCount = 24;
    bi->biCompression = 0;
    bi->biSizeImage = width*height*3;
    bi->biXPelsPerMeter = 0;
    bi->biYPelsPerMeter = 0;
    bi->biClrUsed = 0;
    bi->biClrImportant = 0;


}





/*************************************************
Function: yuyv_to_bgr
Description: 转化yuyv为bgr格式，由下至上，即yuyv最后一行为bgr首行
Input: const uint8_t *src_ptr   源buf指针   
    const int width  图像宽度
    const int height  图像高度
Output: uint8_t *rgb_ptr 输出buf指针
Return: void
Others: convert yuyv yto rgb yuyv, only for bmp 转换为bgr,仅适用于bmp格式
*************************************************/

void yuyv_to_bgr (const uint8_t *src_ptr,uint8_t *rgb_ptr,const int width, const int height)
{
    int r,g,b;
    int y,u,v;
    int i,j;
    printf("start convert\n");
    printf("rgb_ptr start address:0x%p\n",rgb_ptr);
   /*定位到最后一行*/
    rgb_ptr=rgb_ptr+width*(height-1)*3;
    
    printf("rgb_ptr first address:0x%p\n",rgb_ptr);
    /*逐行进行转换*/
    for (i = 0; i < height; i++)
    {
        /*每四个字节即两个像素点进行一次转换*/
     //printf("%d: rgb_ptr  address:0x%p\n",i, (unsigned int)rgb_ptr);
      for(j=0;j<width/2;j++)
      {
  
           /*取YUV三个值*/
          y = src_ptr[0];
          u = src_ptr[1];
          v = src_ptr[3];
         /*转换第一个像素点*/
          r = y +  1.042*(v-128);
          g = y - 0.34414*(u-128) - 0.71414*(v-128);
          b = y +  1.772*(u-128);
          /*data legalized数据合法化,按bgr格式写入buf*/

          *rgb_ptr++ = clip_8bit (b);               /*B*/
          *rgb_ptr++ = clip_8bit (g);               /* G */
          *rgb_ptr++ = clip_8bit (r);               /* R*/
          /*取第二个Y值*/
          y = src_ptr[2];
          /*转换第二个像素点*/
          r = y +  1.042*(v-128);
          g = y - 0.34414*(u-128) - 0.71414*(v-128);
          b = y +  1.772*(u-128);
         /*data legalized数据合法化*/
          *rgb_ptr++ = clip_8bit (b);               /*B*/
          *rgb_ptr++ = clip_8bit (g);               /* G */
          *rgb_ptr++ = clip_8bit (r);               /* R*/

           src_ptr += 4;//源指针指向下一yuv点
      }
      rgb_ptr=rgb_ptr-width*3*2;
     // printf("next  line  rgb_ptr address:0x%p\n", rgb_ptr);
    }
}


/*
#ifdef BMP_FILE
    yuyv_to_rgb(framebuf[buf.index].start,rgb_frame_buffer,VIDEO_WIDTH,VIDEO_HEIGHT);
    printf("yuyv to rgb success!\n");
    fwrite(&bf, 14, 1, bmp);
    fwrite(&bi, 40, 1, bmp);
    fwrite(rgb_frame_buffer, bi.biSizeImage, 1, bmp);
    printf("save bmp file  success\n"); 
#endif
*/



