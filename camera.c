/*****************************************************************************
File name: camera.c
Description: 
Author: xzdong
Date: 2019年5月9日 21:08:25
*****************************************************************************/

#include "camera.h"
 
#ifdef BMP_FILE

/*初始化bmp文件格式结构体*/

BmpInfoHeader bi;
BmpFileHeader bf;

#endif



VideoBuffer *framebuf;
uint8_t rgb_frame_buffer[VIDEO_WIDTH*VIDEO_HEIGHT*3];
uint8_t *jpeg_frame_buffer;

int main()
{
	printf("_____Camera Test______\n");
    int i, ret;  
    FILE *fp,*jpeg_fp;
    /* 打开摄像头设备*/
    int fd;
    int jpeg_size;
    fd = open(CAMERA_DEVICE,O_RDWR, 0);
    if (fd < 0) {
        perror("Open\n");
        return -1;
    }
    struct v4l2_requestbuffers reqbuf;//请求内存结构体
    struct v4l2_buffer buf;//缓冲区结构体

    //char *jpeg_frame_buffer_pt=jpeg_frame_buffer;
   
   
#ifdef BMP_FILE
    //Set BITMAPHEADER
     FILE *bmp;
    bmp_head_init(&bf,&bi,VIDEO_WIDTH,VIDEO_HEIGHT);
#endif
    v4l2_driver_info_get(fd);
    v4l2_fmt_list(fd);
    v4l2_fmt_set(fd);
    v4l2_fmt_get(fd);
    v4l2_buf_req(fd,&reqbuf,BUFFER_COUNT);
    
    framebuf = malloc(BUFFER_COUNT*sizeof (*framebuf));/*为缓冲区映射空间的结构体申请内存,大小为缓冲区个数×结构体长度*/
    v4l2_buf_map(fd,framebuf,&buf,reqbuf.count);
    
    v4l2_steamon(fd);
    
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

    for(i=0;i<FRAME_NUM;i++)
    {
     //内存空间出队列
    ioctl(fd, VIDIOC_DQBUF, &buf);
    printf("buf length:%d  bytesused:%d\n",buf.length,buf.bytesused);
    fwrite(framebuf[buf.index].start, buf.bytesused, 1, fp);//原始数据写入fp文件中
    printf("save one frame success.\n"); //CAPTURE_FILE
    printf("jpeg buf address:%p\n",jpeg_frame_buffer);
    jpeg_size=compress_yuyv_to_jpeg(framebuf[buf.index].start,&jpeg_frame_buffer,80,jpeg_fp); //压缩为jpeg格式，压缩质量为80
    printf("jpeg buf address:%p\n",jpeg_frame_buffer);
    printf("compress yuyv to jpeg successfully!   size:%d\n",jpeg_size);
    fwrite(jpeg_frame_buffer,jpeg_size,1,jpeg_fp);//写入到jpeg文件中
   
    printf("save one  jpeg frame\n");
    
#ifdef BMP_FILE
    yuyv_to_rgb(framebuf[buf.index].start,rgb_frame_buffer,VIDEO_WIDTH,VIDEO_HEIGHT);
    printf("yuyv to rgb success!\n");
    fwrite(&bf, 14, 1, bmp);
    fwrite(&bi, 40, 1, bmp);
    fwrite(rgb_frame_buffer, bi.biSizeImage, 1, bmp);
    printf("save bmp file  success\n"); 
#endif



    /* 内存重新入队列*/
    ret = ioctl(fd, VIDIOC_QBUF, &buf);
    }
 
   
    /*释放资源*/
    for (i=0; i< BUFFER_COUNT; i++) 
    {
        munmap(framebuf[i].start, framebuf[i].length);
    }


    /*关闭设备*/
    free(jpeg_frame_buffer);
    close(fd);
    fclose(fp);
    fclose(jpeg_fp);
    return 0;
}
#ifdef BMP_FILE

void bmp_head_init(BmpFileHeader *bf,BmpInfoHeader *bi,const int width, const int height){

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
    //Set BITMAPFILEHEADER
    bf->bfType = 0x4d42;
    bf->bfSize = 54 + bi->biSizeImage;     
    bf->bfReserved = 0;
    bf->bfOffBits = 54;

}

#endif

/*打印驱动信息*/
/*    struct v4l2_capability  {
        u8 driver[16]; // 驱动名字
        u8 card[32]; // 设备名字
        u8 bus_info[32]; // 设备在系统中的位置
        u32 version; // 驱动版本号
        u32 capabilities; // 设备支持的操作
        u32 reserved[4]; // 保留字段 };
    };*/


int v4l2_driver_info_get(int fd){
        
        /*获取驱动信息*/
        struct v4l2_capability cap;//驱动信息结构体
        /*VIDIOC_QUERYCAP信息查询请求*/
        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
            perror("VIDIOC_QUERYCAP ");
            return -1;
        }

        printf("Capability Informations:\n");
        printf(" driver: %s\n", cap.driver);
        printf(" card: %s\n", cap.card);
        printf(" bus_info: %s\n", cap.bus_info);
        printf(" version: %u.%u.%u\n", (cap.version>>16)&0XFF, (cap.version>>8)&0XFF,cap.version&0XFF);
        printf(" capabilities: %08X\n", cap.capabilities);
        return 0;
}

/*show all supported format.显示所有支持的格式*/
 /***********************
    struct v4l2_fmtdesc  {
        u32 index; // 要查询的格式序号，应用程序设置
        enum v4l2_buf_type type; // 帧类型，数据流类型，必须永远是V4L2_BUF_TYPE_VIDEO_CAPTURE
        u32 flags; // 是否为压缩格式
        u8 description[32]; // 格式名称
        u32 pixelformat; // 格式
        u32 reserved[4]; // 保留
    };
***********************/
int v4l2_fmt_list(int fd){	
	struct v4l2_fmtdesc fmtdesc;
	fmtdesc.index=0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//frame type  
	while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc) != -1){  
        
            printf("VIDIOC_ENUM_FMT success fmt.fmt.pix.pixelformat:%s\n",fmtdesc.description);
            fmtdesc.index++;

    }
    return 0;
}
/*设置视频格式*/

int v4l2_fmt_set(int fd){

 
    /* 设置视频格式*/
    struct v4l2_format fmt; //视频格式信息结构体
    memset(&fmt, 0, sizeof(fmt));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;//图像捕获格式
    fmt.fmt.pix.width       = VIDEO_WIDTH;//宽度
    fmt.fmt.pix.height      = VIDEO_HEIGHT;//长度
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT;//像素格式
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;//扫描方式
    /*VIDIOC_S_FMT表示set  format*/
    if (ioctl(fd, VIDIOC_S_FMT, &fmt)< 0) {
        perror("VIDIOC_S_FMT");
        return -1;
    }
    return 0;

}
/*获取视频格式*/
int v4l2_fmt_get(int fd)
{
     struct v4l2_format fmt;
   /*VIDIOC_G_FMT表示get format*/
    if (ioctl(fd, VIDIOC_G_FMT, &fmt)< 0) {
        printf("VIDIOC_G_FMT failed");
        return -1;
    }
    // Print Stream Format
    printf("Stream Format Informations:\n");
    printf(" type: %d\n", fmt.type);
    printf(" width: %d\n", fmt.fmt.pix.width);
    printf(" height: %d\n", fmt.fmt.pix.height);
    char fmtstr[8];
    memset(fmtstr, 0, 8);
    memcpy(fmtstr, &fmt.fmt.pix.pixelformat, 4);
    printf(" pixelformat: %s\n", fmtstr);
    printf(" field: %d\n", fmt.fmt.pix.field);
    printf(" bytesperline: %d\n", fmt.fmt.pix.bytesperline);
    printf(" sizeimage: %d\n", fmt.fmt.pix.sizeimage);
    printf(" colorspace: %d\n", fmt.fmt.pix.colorspace);
    printf(" priv: %d\n", fmt.fmt.pix.priv);
    printf(" raw_date: %s\n", fmt.fmt.raw_data);
    return 0;
}

/*请求分配内存*/
int v4l2_buf_req(int fd,struct v4l2_requestbuffers *reqbuf,const int bufnum){

    reqbuf->count = bufnum;//帧个数
    reqbuf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//类型为视频捕获
    reqbuf->memory = V4L2_MEMORY_MMAP;//内存映射

    if(ioctl(fd , VIDIOC_REQBUFS, reqbuf) < 0) {
        perror("VIDIOC_REQBUFS ");
        return -1;
    }
    return 0;
}
int v4l2_buf_map(int fd,VideoBuffer *framebuf,struct v4l2_buffer *buf,const int bufnum){

  
    int i;
    memset(buf,0,sizeof(*buf));//初始化
    for (i = 0; i < bufnum; i++) 
    {
        buf->index = i;//序号
        buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//类型为视频捕获
        buf->memory = V4L2_MEMORY_MMAP;//内存映射
        /*获取缓冲区信息到buf中*/
        if(ioctl(fd , VIDIOC_QUERYBUF, buf)< 0) {
            printf("VIDIOC_QUERYBUF (%d) failed\n", i);
            return -1;
        }

        /*映射到用户空间*/
       framebuf[i].length = buf->length;//设置缓冲帧长度
       framebuf[i].start = (char *) mmap(NULL, buf->length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf->m.offset);//起始地址,protect保护方式设置为可读写,共享映射
        if ( framebuf[i].start == MAP_FAILED) {
            printf("mmap (%d) failed: %s\n", i, strerror(errno));
            return -1;
        }
        /*内存入队列queue buffer*/
        if ( ioctl(fd , VIDIOC_QBUF, buf) < 0) {
            printf("VIDIOC_QBUF (%d) failed\n", i);
            return -1;
        }
        printf("Frame buffer %d: address=0x%p, length=%d\n", i, framebuf[i].start,(unsigned int)framebuf[i].length);
      
    } 
     return 0;
}

/*启动视频流*/

int v4l2_steamon(int fd){
    
      enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      /*开启视频流*/
      if (ioctl(fd, VIDIOC_STREAMON, &type)< 0) {
          printf("VIDIOC_STREAMON\n");
          return -1;
      }
      printf("start camera testing....................\n");
      return 0;
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

int compress_yuyv_to_jpeg(unsigned char *src_ptr,unsigned char **outbuffer, int quality,FILE *jpegfile)
{
    struct jpeg_compress_struct cinfo;      //压缩结构体
    struct jpeg_error_mgr jerr;             //错误信息
    JSAMPROW row_pointer[1];
    unsigned char *line_buffer, *yuyv;      //buf
    int z;
    static long unsigned int outSize;
   // unsigned char *outbuffer=NULL;
    line_buffer = calloc(VIDEO_WIDTH*3, 1);
    yuyv = src_ptr;

    cinfo.err = jpeg_std_error(&jerr);      //错误信息
    jpeg_create_compress(&cinfo);       //创建压缩对象
    printf("jpeg cinfo created\n");
   //jpeg_stdio_dest(&cinfo, jpegfile);  //输出到文件
    jpeg_mem_dest(&cinfo, outbuffer, &outSize);//输出到目标内存
    printf("outbuffer pt address:%p\n outbuffer address:%p \n",outbuffer,*outbuffer);
    cinfo.image_width = VIDEO_WIDTH;        //设置宽度
    cinfo.image_height = VIDEO_HEIGHT;      //设置高度
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;         //设置输入图片格式，支持RGB，YUV等等，YUYV需要进行转换

    jpeg_set_defaults(&cinfo);              //其他参数为默认
    jpeg_set_quality(&cinfo, quality, TRUE);  //设置转化图片质量 0-100

    jpeg_start_compress(&cinfo, TRUE);      //开始压缩

    z = 0;
    /*转换为rgb*/
    int r, g, b;
    int y, u, v;
    while(cinfo.next_scanline < VIDEO_HEIGHT) {
        int j;
        unsigned char *rgb_ptr = line_buffer;
        for(j=0;j<VIDEO_WIDTH/2;j++)
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



