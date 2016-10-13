#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

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

int main()
{
   int fd;
   int nResult;
   struct InputData event;
   fd = open("/dev/input/event0",O_RDONLY|O_NONBLOCK);
   if(fd == -1)
   {
      perror("open");
      return 1;
   }

   while(1)
  {
     nResult = read(fd,&event,sizeof(event));
     if(nResult == sizeof(event))
    {
       printf("keycode:%d,keyvalue:%d\n",event.code,event.value);

    }
    sleep(1);
  }

  close(fd);
  return 0;
}
