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
 
#define CAMERA_DEVICE "/dev/video0"
#define CAPTURE_FILE "./camera_file.yuv" 
#define BMP_FILE "./bmp_file.bmp" 

#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define BUFFER_COUNT 4
#define clip_8bit(val) (val) < 0 ? 0 :( (val) > 255 ? 255 : (val) )

#pragma pack(1)

typedef struct BmpFileHeader{
    uint16_t bfType;
    uint32_t bfSize;
    uint32_t  bfReserved;

    uint32_t  bfOffBits;

}BmpFileHeader;


typedef struct VideoBuffer{
    void   *start;
    size_t  length;
}VideoBuffer;



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



VideoBuffer *framebuf;
unsigned char rgb_frame_buffer[VIDEO_WIDTH*VIDEO_HEIGHT*3];//rgb格式缓存
void yuyv_to_rgb (const uint8_t *src_ptr,uint8_t *rgb_ptr,const int width, const int height);



int main()
{
	printf("_____Camera Test______\n");
    int i, ret;  
    FILE *fp,*bmp;
    /* 打开摄像头设备*/
    int fd;
    fd = open(CAMERA_DEVICE,O_RDWR, 0);
    if (fd < 0) {
        perror("Open\n");
        return -1;
    }
    /*初始化bmp文件格式结构体*/
    BmpInfoHeader bi;
    BmpFileHeader bf;
     //Set BITMAPINFOHEADER
    bi.biSize = 40;
    bi.biWidth = VIDEO_WIDTH;
    bi.biHeight = VIDEO_HEIGHT;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = 0;
    bi.biSizeImage = VIDEO_HEIGHT*VIDEO_WIDTH*3;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;
 

    //Set BITMAPFILEHEADER
    bf.bfType = 0x4d42;
    bf.bfSize = 54 + bi.biSizeImage;     
    bf.bfReserved = 0;
    bf.bfOffBits = 54;
    
    
    /*获取驱动信息*/
    struct v4l2_capability cap;//驱动信息结构体
    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);//VIDIOC_QUERYCAP信息查询请求
    if (ret < 0) {
        printf("VIDIOC_QUERYCAP failed (%d)\n", ret);
        return ret;
    }
	/*打印驱动信息*/
/*    struct v4l2_capability  {
        u8 driver[16]; // 驱动名字
        u8 card[32]; // 设备名字
        u8 bus_info[32]; // 设备在系统中的位置
        u32 version; // 驱动版本号
        u32 capabilities; // 设备支持的操作
        u32 reserved[4]; // 保留字段 };
    };*/

    printf("Capability Informations:\n");
    printf(" driver: %s\n", cap.driver);
    printf(" card: %s\n", cap.card);
    printf(" bus_info: %s\n", cap.bus_info);
    printf(" version: %u.%u.%u\n", (cap.version>>16)&0XFF, (cap.version>>8)&0XFF,cap.version&0XFF);
    printf(" capabilities: %08X\n", cap.capabilities);

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
	
	struct v4l2_fmtdesc fmtdesc;
	fmtdesc.index=0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//frame type  
	while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc) != -1){  
        
            printf("VIDIOC_ENUM_FMT success.->fmt.fmt.pix.pixelformat:%s\n",fmtdesc.description);
            fmtdesc.index++;
   
   
    }

 
    /* 设置视频格式*/
    struct v4l2_format fmt; //视频格式信息结构体
    memset(&fmt, 0, sizeof(fmt));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;//图像捕获格式
    fmt.fmt.pix.width       = VIDEO_WIDTH;//宽度
    fmt.fmt.pix.height      = VIDEO_HEIGHT;//长度
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;//像素格式
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;//扫描方式
    ret = ioctl(fd, VIDIOC_S_FMT, &fmt);//VIDIOC_S_FMT表示set  format
    if (ret < 0) {
        printf("VIDIOC_S_FMT failed (%d)\n", ret);
        return ret;
    }
 
    //获取视频格式
    ret = ioctl(fd, VIDIOC_G_FMT, &fmt);//VIDIOC_G_FMT表示get format
    if (ret < 0) {
        printf("VIDIOC_G_FMT failed (%d)\n", ret);
        return ret;
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

  
    //请求分配内存
    struct v4l2_requestbuffers reqbuf;//请求内存结构体
    
    reqbuf.count = BUFFER_COUNT;//帧个数
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//类型为视频捕获
    reqbuf.memory = V4L2_MEMORY_MMAP;//内存映射
    
    ret = ioctl(fd , VIDIOC_REQBUFS, &reqbuf);
    if(ret < 0) {
        printf("VIDIOC_REQBUFS failed (%d)\n", ret);
        return ret;
    }
    
    framebuf = malloc(BUFFER_COUNT*sizeof (*framebuf));//为缓冲区映射空间的结构体申请内存,大小为缓冲区个数×结构体长度
   // VideoBuffer*  buffers = calloc( reqbuf.count, sizeof(*buffers) );//申请总的缓冲区内存空间
    struct v4l2_buffer buf;//缓冲区结构体
    memset(&buf,0,sizeof(buf));//初始化
    for (i = 0; i < reqbuf.count; i++) 
    {
        buf.index = i;//序号
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//类型为视频捕获
        buf.memory = V4L2_MEMORY_MMAP;//内存映射
        //获取缓冲区信息到buf中
        ret = ioctl(fd , VIDIOC_QUERYBUF, &buf);
        if(ret < 0) {
            printf("VIDIOC_QUERYBUF (%d) failed (%d)\n", i, ret);
            return ret;
        }
 
        //映射到用户空间
        framebuf[i].length = buf.length;//设置缓冲帧长度
        framebuf[i].start = (char *) mmap(NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);//起始地址,protect保护方式设置为可读写,共享映射
        if (framebuf[i].start == MAP_FAILED) {
            printf("mmap (%d) failed: %s\n", i, strerror(errno));
            return -1;
        }
    
        //内存入队列
        ret = ioctl(fd , VIDIOC_QBUF, &buf);//queue buffer
        if (ret < 0) {
            printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
            return -1;
        }
 
        printf("Frame buffer %d: address=0x%x, length=%d\n", i, (unsigned int)framebuf[i].start, framebuf[i].length);
    }
 
    // 启动视频流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd, VIDIOC_STREAMON, &type);//开启视频流
    if (ret < 0) {
        printf("VIDIOC_STREAMON failed (%d)\n", ret);
        return ret;
    }
 
 
    printf("start camera testing...\n");
    fp = fopen(CAPTURE_FILE, "wb");//打开存储文件
    bmp = fopen(BMP_FILE, "wb");//打开bmp存储文件
 
    //开始视频测试
    if (ret < 0) 
     {
        printf("VIDIOC_DQBUF failed (%d)\n", ret);
        return ret;
     }
    /*Process the frame.处理这一帧*/
    
    if (fp==NULL) {
        printf("open frame-data-file failed.\n");
        return -1;
    }
      if (bmp==NULL) {
        printf("open bmp-file failed.\n");
       
    }
    printf("open file success.\n");

    for(i=0;i<1;i++)
    {
        //内存空间出队列
    ret = ioctl(fd, VIDIOC_DQBUF, &buf);
    fwrite(framebuf[buf.index].start, buf.bytesused, 1, fp);
    printf("save one frame success.\n"); //CAPTURE_FILE
    yuyv_to_rgb(framebuf[buf.index].start,rgb_frame_buffer,VIDEO_WIDTH,VIDEO_HEIGHT);
    printf("yuyv to rgb success!\n");
    fwrite(&bf, 14, 1, bmp);
    fwrite(&bi, 40, 1, bmp);
    fwrite(rgb_frame_buffer, bi.biSizeImage, 1, bmp);
    printf("save bmp file  success.\n"); //CAPTURE_FILE


    // 内存重新入队列
    ret = ioctl(fd, VIDIOC_QBUF, &buf);
    }
    if (ret < 0) 
      {
        printf("VIDIOC_QBUF failed (%d)\n", ret);
        return ret;
      }
 
   
    //释放资源
    for (i=0; i< BUFFER_COUNT; i++) 
    {
        munmap(framebuf[i].start, framebuf[i].length);
    }

    //关闭设备

    close(fd);
    fclose(fp);
    return 0;
}



/*convert yuyv yto rgb yuyv, only for bmp 转换为rgb格式,仅适用于bmp格式*/
void yuyv_to_rgb (const uint8_t *src_ptr,uint8_t *rgb_ptr,const int width, const int height)
{
    int r,g,b;
    int y,u,v;
    int i,j;
    printf("start convert\n");
    printf("rgb_ptr start address:0x%x\n", (unsigned int)rgb_ptr);
   /*定位到最后一行*/
    rgb_ptr=rgb_ptr+width*(height-1)*3;
    
    printf("rgb_ptr first address:0x%x\n", (unsigned int)rgb_ptr);
    /*逐行进行转换*/
    for (i = 0; i < height; i++)
    {
        /*每四个字节即两个像素点进行一次转换*/
     //printf("%d: rgb_ptr  address:0x%x\n",i, (unsigned int)rgb_ptr);
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
     // printf("next  line  rgb_ptr address:0x%x\n", (unsigned int)rgb_ptr);
    }
}



