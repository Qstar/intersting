#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>//fb_var_screeninfo
#include <sys/mman.h>

#define COLORDEPTH	16

typedef unsigned short ushort;
typedef unsigned int uint;

#if COLORDEPTH == 32
#define ARGB(A,R,G,B) ((A<<24)|(R<<16)|(G<<8)|B)
typedef uint COLORREF;
#elif COLORDEPTH == 24
#define RGB(R,G,B) ((R<<16)|(G<<8)|B)
typedef uint COLORREF;
#elif COLORDEPTH == 16
#define RGB(R,G,B) ((R&0xf8)<<8)|((G&0xfc)<<3)|((B&0xf8)>>3)
typedef ushort COLORREF;
#else
#error "Not Support this color depth."
#endif

char *fbmem_addr = NULL;
uint scrwidth, scrheight;
uint depth;

void draw_pixel(uint x, uint y, COLORREF color)
{
    *(ushort *)(fbmem_addr + scrwidth * y * depth + x * depth) = color;
}

void draw_line(uint ax, uint ay, uint bx, uint by, COLORREF color)
{
    double  m, c;
    double y;
    uint iy,x;
    
    if(bx != ax)
    {
        m = (double)(by-ay)/(bx-ax);
        c =  ay - m*ax;
        for (x=ax ; x <=bx ; x++) {
            y = m*x + c;
            y = y + 0.5;
            iy = (uint)y;
            draw_pixel (x, iy, color);
        }
    }
    else
    {
        for(iy = ay; iy <= by; ++iy){
            draw_pixel (ax, iy, color);
        }
    }
}

void draw_rect(uint x, uint y, uint w, uint h, COLORREF color)
{
    draw_line(x, y, x, y + h, color);
    draw_line(x, y, x + w, y, color);
    draw_line(x + w, y, x + w, y + h, color);
    draw_line(x, y + h, x + w, y + h, color);
}

void fill_rect(uint x, uint y, uint w, uint h, COLORREF color)
{
    int i;
    for(i = y; i < y + h; ++i)
    {
        draw_line(x, i, x + w, i, color);
    }
}


int main()
{
    int fd;
    struct fb_var_screeninfo modeinfo;
    size_t length;
    
    fd = open("/dev/fb0", O_RDWR);
    if(fd == -1)
    {
        perror("open /dev/fb0");
        return 1;
    }
    
    ioctl(fd, FBIOGET_VSCREENINFO, &modeinfo);
    
    scrwidth = modeinfo.xres;
    scrheight = modeinfo.yres;
    depth = modeinfo.bits_per_pixel / 8;
    
    length = scrwidth * scrheight * depth;
    fbmem_addr = mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (NULL == fbmem_addr)
    {
        perror("mmap");
        return 1;
    }
    
    //	printf("draw line\n");
    //	draw_line(10, 10, 50, 50, RGB(255, 0, 0));
    //	printf("draw rectangle\n");
    //	draw_rect(10, 60, 100, 50, RGB(0, 255, 0));
    //	printf("fill rectangle\n");
    //	fill_rect(10, 120, 100, 50, RGB(0, 0, 255));
    
    printf("draw******************************\n");
    fill_rect(0, 0, 10, 20, RGB(0, 0, 255));
    
    int i = 0;
    fill_rect(0, 0, scrwidth, scrheight, RGB(0, 0,0));
    for(i = 0;i<200;++i)
    {
        fill_rect(0+i-1, 0+i-1, 10, 20, RGB(0, 0, 0));
        fill_rect(0+i, 0+i, 10, 20, RGB(255, 0, 255));
        usleep(200);
    }
    
    close(fd);
    return 0;
}
