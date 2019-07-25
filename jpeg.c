/*****************************************************************************
File name: jpeg.c
Description:jpeg处理程序 
Author: xzdong
*****************************************************************************/

#include "jpeg.h"
 

/*************************************************
Function: compress_yuyv_to_jpeg
Description: 压缩yuyv为jpeg格式
Input: unsigned char *src_ptr    源buf指针
        unsigned char *outbuffer 输出buf指针
        int quality   压缩质量参数，范围0-100

Output: unsigned char *outbuffer 输出buf指针
       FILE *jpegfile 输出文件
Return: 返回压缩大小
Others: // 其它说明
*************************************************/



int compress_yuyv_to_jpeg(uint8_t *src_ptr,uint8_t **outbuffer, int quality,const int width,const int height)
{
    struct jpeg_compress_struct cinfo;      //压缩结构体
    struct jpeg_error_mgr jerr;             //错误信息
    JSAMPROW row_pointer[1];
    unsigned char *line_buffer;      //buf
    static long unsigned int outSize;
  
    line_buffer = calloc(width*3, 1);


    cinfo.err = jpeg_std_error(&jerr);      //错误信息
    jpeg_create_compress(&cinfo);           //创建压缩对象
    //printf("jpeg cinfo created\n");
   //jpeg_stdio_dest(&cinfo, jpegfile);    //输出到文件
    jpeg_mem_dest(&cinfo, outbuffer, &outSize);//输出到目标内存，动态分配outbuffer内存空间
    //printf("outbuffer pt address:%p\n outbuffer address:%p \n",outbuffer,*outbuffer);
    //printf("width:%d\n height:%d \n",width,height);
    cinfo.image_width = width;             //设置宽度
    cinfo.image_height = height;           //设置高度
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;         //设置输入图片格式，支持RGB，YUV等等，YUYV需要进行转换

    jpeg_set_defaults(&cinfo);              //其他参数为默认
    jpeg_set_quality(&cinfo, quality, TRUE);  //设置转化图片质量 0-100

    jpeg_start_compress(&cinfo, TRUE);      //开始压缩

    /*转换为rgb*/
    int r, g, b;
    int y, u, v;
    while(cinfo.next_scanline < height) {
        int j;
        unsigned char *rgb_ptr = line_buffer;
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

          *rgb_ptr++ = clip_8bit (r);               /*R*/
          *rgb_ptr++ = clip_8bit (g);               /* G */
          *rgb_ptr++ = clip_8bit (b);               /* B*/
          /*取第二个Y值*/
          y = src_ptr[2];
          /*转换第二个像素点*/
          r = y +  1.042*(v-128);
          g = y - 0.34414*(u-128) - 0.71414*(v-128);
          b = y +  1.772*(u-128);
         /*data legalized数据合法化*/
          *rgb_ptr++ = clip_8bit (r);               /*R*/
          *rgb_ptr++ = clip_8bit (g);               /* G */
          *rgb_ptr++ = clip_8bit (b);               /* B*/

           src_ptr += 4;//源指针指向下一yuv点
      }


        row_pointer[0] = line_buffer;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);//单行图片压缩
    }
   // dest_ptr=malloc(outSize);
    //memcpy(dest_ptr,outbuffer,outSize);

    jpeg_finish_compress(&cinfo);//结束压缩
    jpeg_destroy_compress(&cinfo);//摧毁压缩

    free(line_buffer);//释放资源
   
    return (outSize);//返回压缩大小
}

