//
//  pwm.c
//  
//
//  Created by ksy on 14/11/29.
//
//

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



int main(void)
{
    int fd;
    fd = open("/dev/pwm",O_RDONLY);
    if(fd == -1)
    {
        perror("open");
    }
    int i=0;
    for(i=0;i<5;i++)
    {
        ioctl(fd,1,i+i*2000);
	sleep(2);
        
    }

    close(fd);
    return 0;
}
