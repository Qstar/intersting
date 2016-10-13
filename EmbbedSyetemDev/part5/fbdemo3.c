#include <linux/fb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define RGB(R, G, B) (( R & 0xf8) << 8) | (( G & 0xfc ) << 3) | (( B & 0xf8 ) >> 3)

#pragma pack(1)
typedef struct
{
    unsigned short int	type;
    unsigned int		size;
    unsigned short int	reserved1, reserved2;
    unsigned int		offset;
} s_header;

typedef struct
{
    unsigned int		size;
    int			width, height;
    unsigned short int	planes;
    unsigned short int	bits;
    unsigned int		compression;
    unsigned int		imagesize;
    int			xresolution, yresolution;
    unsigned int		ncolours;
    unsigned int		importantcolours;
} s_info_header;

void	draw_pixel(int x, int y, unsigned short color, char *fbmem_addr, __u32 screen_width, __u32 bpp)
{
    *(unsigned short *)(fbmem_addr + screen_width * y * bpp + x * bpp) = color;
}

int				main(int argc, char *argv[])
{
    FILE				*bmp_stream;
    ssize_t			fd;
    struct fb_var_screeninfo	modeinfo;
    char				*fbmem_addr,*ptmpmem;
    __u32				screen_width;
    __u32				screen_height;
    __u32				bpp;
    s_header			bmp_header;
    s_info_header			bmp_info_header;
    char				pixel_address[3];
    int				i;
    __u32				j;

    if (argc == 1)
    {
        printf("usage : ./image picture.bmp\n");
        return (1);
    }
    fd = open("/dev/fb0", O_RDWR);
    if (fd == -1)
    {
        printf("open failed !");
        return (1);
    }
    ioctl(fd, FBIOGET_VSCREENINFO, &modeinfo);
    screen_width = modeinfo.xres;
    screen_height = modeinfo.yres;
    bpp = modeinfo.bits_per_pixel / 8;

		ptmpmem= mmap(0, screen_width * screen_height * bpp, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    fbmem_addr = (char*)malloc(screen_width * screen_height * bpp);
	memset(fbmem_addr, 0, screen_width * screen_height * bpp);
	if (NULL == fbmem_addr)
	{
		perror("mmap");
		return 1;
	}



/*    fbmem_addr = mmap(0, screen_width * screen_height * bpp, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);*/
    bmp_stream = fopen(argv[1], "r");
    if (bmp_stream == NULL)
    {
        printf("fopen failed !");
        return (1);
    }
    fread(&bmp_header, 1, sizeof(s_header), bmp_stream);
    fread(&bmp_info_header, 1, sizeof(s_info_header), bmp_stream);
    fseek(bmp_stream, bmp_header.offset, SEEK_SET);
    i = screen_height - 1;
    while (i != -1)
    {
        j = 0;
        while (j < screen_width)
        {
            fread(pixel_address, 3, 1, bmp_stream);
            draw_pixel(j, i, RGB(pixel_address[2], pixel_address[1], pixel_address[0]), fbmem_addr, screen_width, 2);
            ++j;
        }
        --i;
    }
		
		memcpy(ptmpmem, fbmem_addr, screen_width * screen_height * bpp);

    fclose(bmp_stream);
    close(fd);
    return (0);
}
