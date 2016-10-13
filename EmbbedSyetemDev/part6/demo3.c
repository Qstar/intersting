#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
                                                                                                                                                                                                                           
#include <linux/types.h>
//#include <linux/videodev.h>
#include <setjmp.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <asm/types.h>         
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/stat.h>
#include  "videodev.h"
#include  "jpeg_sample.c"

#define FB_DEV "/dev/fb0"
#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 240

int fb_open(char *fb_device);
int fb_close(int fd);
int fb_stat(int fd, int *width, int *height, int *depth);
void *fb_mmap(int fd, unsigned int screensize);
int fb_munmap(void *start, size_t length);
int fb_pixel(void *fbmem, int width, int height,int x, int y, unsigned short color);

int cam_fd;
struct v4l2_capability cam_cap;
struct v4l2_fmtdesc fmtdesc;
struct v4l2_format fmt;
struct v4l2_requestbuffers  req;
struct v4l2_buffer buf;

struct VideoBuffer{
	void *start;
	size_t length;
};
struct InputData
{
    unsigned int dummy1;
    unsigned int dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int value;
    unsigned int dummy3;
    unsigned int dummy4;
    unsigned int dummy5;
    unsigned int dummy6;
};
struct VideoBuffer *buffers;

int main(int argc,char **argv)
{
	int ret;
	int numBufs = 0;
	
	unsigned char *buffer;
    /*
     * declaration for framebuffer device
     */
    int fbdev;
    char *fb_device;
    unsigned char *fbmem;
    unsigned int screensize;
    unsigned int fb_width;
    unsigned int fb_height;
    unsigned int fb_depth;
    unsigned int x;
    unsigned int y;

    int fd;
    int nResult;
    struct InputData event;

	
	cam_fd = open("/dev/video0" ,O_RDWR);
	if(cam_fd < 0){
		printf("can not open driver!! \n");
		exit(1);
	}

	fd = open("/dev/input/event0", O_RDONLY|O_NONBLOCK);
	 if(fd==-1)
	 {
	        perror("open(event0)");
	        return 1;
	 }
			
	//设置帧格式
	memset(&fmt,0,sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  // 数据流类型，必须永远是V4L2_BUF_TYPE_VIDEO_CAPTURE
	fmt.fmt.pix.width = VIDEO_WIDTH;
	fmt.fmt.pix.height = VIDEO_HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565; //要与摄像头相对应， 视频数据存储类型，例如是YUV4：2：2还是RGB
	fmt.fmt.pix.depth = 16;
	if(ioctl(cam_fd, VIDIOC_S_FMT,&fmt) < 0){
		printf("set format is fail!! \n");
		exit(4);
	}

	/*
	* open framebuffer device
	*/
	if ((fb_device = getenv("FRAMEBUFFER")) == NULL)
		fb_device = FB_DEV;
	fbdev = fb_open(fb_device);
	/*
     * get status of framebuffer device
     */
    fb_stat(fbdev, &fb_width, &fb_height, &fb_depth);
    if ((VIDEO_WIDTH > fb_width) || (VIDEO_HEIGHT > fb_height))
        return (-1);
    /*
     * map framebuffer device to shared memory
     */
    screensize = fb_width * fb_height * fb_depth / 8;
    fbmem = fb_mmap(fbdev, screensize);
	
	/*以上步骤完成了视频采集的准备工作，但驱动还没有启动采集过程，
	应用程序需要调用VIDIOC_STREAMON ioctl 系统调用驱动才会开始采集数据。
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl (fd, VIDIOC_STREAMON, &type);*/
    //开始采集视频
	int buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(cam_fd,VIDIOC_STREAMON,&buf_type) < 0){
		printf("VIDIOC_STREAMON is fail!! \n");
	    exit(9);
	}
	
	/*采集过程开始以后，驱动会不停地将数据写入分配的缓冲区内，
	当一个缓冲区的数据准备就绪后，驱动就会将其放入输出队列，等待应用程序的处理。
	当所有的缓冲区都进入输出队列后，驱动将停止采集，并等待缓冲区重新放入采集队列。
    读取数据时，首先需要将一个缓冲区出队列：
         struct v4l2_buffer buf;
         ioctl (fd, VIDIOC_DQBUF, &buf)；*/
    //取出FIFO 缓存中已经采样的帧缓存,
    int i = 0;
    while(i < 1000){
	char*  buffer = (char*) calloc(VIDEO_WIDTH * VIDEO_HEIGHT*2 ,1);

	int count = read(cam_fd,buffer,VIDEO_WIDTH * VIDEO_HEIGHT *2);
	printf(">> %d\n",count);
	for(y = 0;y < VIDEO_HEIGHT;y++){
		memcpy(fbmem + y * fb_width * 2,buffer + y * VIDEO_WIDTH * 2,VIDEO_WIDTH * 2);
	}

	nResult = read(fd,&event,sizeof(event));
	if(nResult==sizeof(event)){
                 	printf("keycode:%d,keyvalue:%d\n",event.code,event.value);
                 	write_jpeg_file( "cemara.jpg");
                 	break;
                 }
	i++;
}
	
//停止视频的采集
if(ioctl(cam_fd,VIDIOC_STREAMOFF,&buf_type) < 0){
	printf("VIDIOC_STREAMOFF is fail!! \n");
  		exit(12);
}
close(cam_fd);
close(fbdev);
close(fd);

return 0;
}
/*
 * open framebuffer device.
 * return positive file descriptor if success,
 * else return -1.
 */
int fb_open(char *fb_device)
{
    int fd;
    if ((fd = open(fb_device, O_RDWR)) < 0) {
        perror(__func__);
        return (-1);
    }
    return (fd);
}
/*
 * get framebuffer's width,height,and depth.
 * return 0 if success, else return -1.
 */
int fb_stat(int fd, int *width, int *height, int *depth)
{
    struct fb_fix_screeninfo fb_finfo;
    struct fb_var_screeninfo fb_vinfo;
    if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_finfo)) {
        perror(__func__);
        return (-1);
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_vinfo)) {
        perror(__func__);
        return (-1);
    }
    *width = fb_vinfo.xres;
    *height = fb_vinfo.yres;
    *depth = fb_vinfo.bits_per_pixel;
    return (0);
}
/*
 * map shared memory to framebuffer device.
 * return maped memory if success,
 * else return -1, as mmap dose.
 */
void* fb_mmap(int fd, unsigned int screensize)
{
    caddr_t fbmem;
    if ((fbmem = mmap(0, screensize, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0)) == MAP_FAILED) {
        perror(__func__);
        return (void *) (-1);
    }
    return (fbmem);
}
/*
 * unmap map memory for framebuffer device.
 */
int fb_munmap(void *start, size_t length)
{
    return (munmap(start, length));
}
