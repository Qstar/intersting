#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

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
int ledfd;
void Led1(int stat){
    ioctl(ledfd,stat,0);
}
void Led2(int stat){
    ioctl(ledfd,stat,1);
}
void Led3(int stat){
    ioctl(ledfd,stat,2);
}
void Led4(int stat){
    ioctl(ledfd,stat,3);
}
struct CtrlTbl
{
    int keycode;
    void (*Action)(int stat);
}ActionTable[]=
{
    {103,Led1},
    {108,Led2},
    {105,Led3},
    {106,Led4}
};
int main()
{
    int	i;
    int	fd;
    int nResult;
    struct InputData event;
    fd = open("/dev/input/event0", O_RDONLY|O_NONBLOCK);
    if(fd==-1)
    {
        perror("open(event0)");
        return 1;
    }
    ledfd = open("/dev/leds", O_WRONLY);
    if(ledfd==-1){
        close(fd);
        perror("open(leds)");
        return 1;
    }
    while (1)
    {
        nResult=read(fd,&event,sizeof(event));
        if(nResult==sizeof(event)){
            printf("keycode:%d,keyvalue:%d\n",event.code,event.value);
            for(i=0;i<sizeof(ActionTable)/sizeof(ActionTable[0]);++i){
                if(ActionTable[i].keycode==event.code){
                    ActionTable[i].Action(event.value);
                    break;
                }
            }
        }usleep(200);
    }
    close(fd);
    close(ledfd);
    return 0;
}
