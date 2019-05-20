/*****************************************************************************
File name: v4l2_ctr.c
Description: v4l2驱动程序
Author: xzdong
Date: 2019年5月10日 16:28:18
*****************************************************************************/

#include "v4l2_ctr.h"
 

/*打印驱动信息*/
/*   
struct v4l2_capability  {
        u8 driver[16]; // 驱动名字
        u8 card[32]; // 设备名字
        u8 bus_info[32]; // 设备在系统中的位置
        u32 version; // 驱动版本号
        u32 capabilities; // 设备支持的操作
        u32 reserved[4]; // 保留字段 };
    };*/
int v4l2_driver_info_get(CameraInfo camera){
        
    struct v4l2_capability cap;//驱动信息结构体
    /*VIDIOC_QUERYCAP信息查询请求*/
    if (ioctl(camera.fd, VIDIOC_QUERYCAP, &cap) < 0) {
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
int v4l2_fmt_list(CameraInfo camera){	
	struct v4l2_fmtdesc fmtdesc;
	fmtdesc.index=0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//frame type  
	while(ioctl(camera.fd,VIDIOC_ENUM_FMT,&fmtdesc) != -1){  
        
            printf("VIDIOC_ENUM_FMT success fmt.fmt.pix.pixelformat:%s\n",fmtdesc.description);
            fmtdesc.index++;

    }
    return 0;
}
/*设置视频格式*/

int v4l2_fmt_set(CameraInfo camera){

    struct v4l2_format fmt; //视频格式信息结构体
    memset(&fmt, 0, sizeof(fmt));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;//图像捕获格式
    fmt.fmt.pix.width       = camera.width;              //宽度
    fmt.fmt.pix.height      = camera.height;             //长度
    fmt.fmt.pix.pixelformat = camera.pix_format;             //像素格式
    fmt.fmt.pix.field       = camera.field;    //扫描方式
    /*VIDIOC_S_FMT表示set format*/
    if (ioctl(camera.fd, VIDIOC_S_FMT, &fmt)< 0) {
        perror("video set format");
        return -1;
    }
    return 0;

}
/*获取视频格式*/
int v4l2_fmt_get(CameraInfo camera){

    struct v4l2_format fmt;
   /*VIDIOC_G_FMT表示get format*/
    if (ioctl(camera.fd, VIDIOC_G_FMT, &fmt)< 0) {
        printf("video get format failed");
        return -1;
    }
    /* Print Stream Format*/
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
int v4l2_buf_req(CameraInfo camera,struct v4l2_requestbuffers *reqbuf){

    reqbuf->count = camera.buffer_count;//帧个数
    reqbuf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//类型为视频捕获
    reqbuf->memory = V4L2_MEMORY_MMAP;//内存映射

    if(ioctl(camera.fd , VIDIOC_REQBUFS, reqbuf) < 0) {
        perror("VIDIOC_REQBUFS ");
        return -1;
    }
    return 0;
}
int v4l2_buf_map(CameraInfo camera,VideoBuffer *framebuf,struct v4l2_buffer *buf){

  
    int i;
    memset(buf,0,sizeof(*buf));                 //初始化
    for (i = 0; i < camera.buffer_count; i++) 
    {
        buf->index = i;                         //序号
        buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  //类型为视频捕获
        buf->memory = V4L2_MEMORY_MMAP;         //内存映射
        /*获取缓冲区信息到buf中*/
        if(ioctl(camera.fd , VIDIOC_QUERYBUF, buf)< 0) {
            printf("VIDIOC_QUERYBUF (%d) failed\n", i);
            return -1;
        }

        /*映射到用户空间*/
       framebuf[i].length = buf->length;        //设置缓冲帧长度
       framebuf[i].start = (char *) mmap(NULL, buf->length, PROT_READ|PROT_WRITE, MAP_SHARED, camera.fd, buf->m.offset);//起始地址,protect保护方式设置为可读写,共享映射
        if ( framebuf[i].start == MAP_FAILED) {
            printf("mmap (%d) failed: %s\n", i, strerror(errno));
            return -1;
        }
        /*内存入队列queue buffer*/
        if ( ioctl(camera.fd , VIDIOC_QBUF, buf) < 0) {
            printf("VIDIOC_QBUF (%d) failed\n", i);
            return -1;
        }
        printf("Frame buffer %d: address=0x%p, length=%d\n", i, framebuf[i].start,(unsigned int)framebuf[i].length);
      
    } 
     return 0;
}

/*启动视频流*/

int v4l2_steamon(CameraInfo camera){
    
      enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (ioctl(camera.fd, VIDIOC_STREAMON, &type)< 0) {
          printf("VIDIOC_STREAMON\n");
          return -1;
      }
      printf("start camera testing....................\n");
      return 0;
}












