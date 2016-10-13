//
//  leddemo.c
//  
//
//  Created by ksy on 14/11/29.
//
//

#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define oops(m,x) {perror(x);exit(x);}
#define CMD_LEDON 0
#define CMD_LEDOFF 1

int main(void)
{
    int fd;
    fd = open("/dev/leds",O_WRONLY);
    if(fd == -1)
    {
        oops("open",1);
    }
    ioctl(fd,CMD_LEDON,0);
    ioctl(fd,CMD_LEDON,1);
    ioctl(fd,CMD_LEDON,2);
    ioctl(fd,CMD_LEDON,3);
    sleep(3);
    ioctl(fd,CMD_LEDOFF,0);
    ioctl(fd,CMD_LEDOFF,1);
    ioctl(fd,CMD_LEDOFF,2);
    ioctl(fd,CMD_LEDOFF,3);
    sleep(3);
    ioctl(fd,CMD_LEDON,0);
    sleep(1);
    ioctl(fd,CMD_LEDON,1);
    sleep(1);
    ioctl(fd,CMD_LEDON,2);
    sleep(1);
    ioctl(fd,CMD_LEDON,3);
    sleep(1);
    ioctl(fd,CMD_LEDOFF,0);
    ioctl(fd,CMD_LEDOFF,1);
    ioctl(fd,CMD_LEDOFF,2);
    ioctl(fd,CMD_LEDOFF,3);
    sleep(1);
    close(fd);
    return 0;
}
