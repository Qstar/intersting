#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

int			set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
  struct termios	newtio, oldtio;

  if (tcgetattr(fd, &oldtio) != 0)
    {
      perror("SetupSerial 1");
      return (-1);
    }
  bzero(&newtio, sizeof (newtio));
  newtio.c_cflag |= CLOCAL | CREAD;
  newtio.c_cflag &= ~CSIZE;
  switch (nBits)
    {
    case 7:
      newtio.c_cflag |= CS7;
      break;
    case 8:
      newtio.c_cflag |= CS8;
      break;
    }
  switch (nEvent)
    {
    case 'O':
      newtio.c_cflag |= PARENB;
      newtio.c_cflag |= PARODD;
      newtio.c_iflag |= (INPCK | ISTRIP);
      break;
    case 'E': 
      newtio.c_iflag |= (INPCK | ISTRIP);
      newtio.c_cflag |= PARENB;
      newtio.c_cflag &= ~PARODD;
      break;
    case 'N':  
      newtio.c_cflag &= ~PARENB;
      break;
    }
  switch (nSpeed)
    {
    case 2400:
      cfsetispeed(&newtio, B2400);
      cfsetospeed(&newtio, B2400);
      break;
    case 4800:
      cfsetispeed(&newtio, B4800);
      cfsetospeed(&newtio, B4800);
      break;
    case 9600:
      cfsetispeed(&newtio, B9600);
      cfsetospeed(&newtio, B9600);
      break;
    case 115200:
      cfsetispeed(&newtio, B115200);
      cfsetospeed(&newtio, B115200);
      break;
    case 460800:
      cfsetispeed(&newtio, B460800);
      cfsetospeed(&newtio, B460800);
      break;
    default:
      cfsetispeed(&newtio, B9600);
      cfsetospeed(&newtio, B9600);
      break;
    }
  if (nStop == 1)
    newtio.c_cflag &= ~CSTOPB;
  else if (nStop == 2)
    newtio.c_cflag |= CSTOPB;
  newtio.c_cc[VTIME] = 0;
  newtio.c_cc[VMIN] = 0;
  tcflush(fd, TCIFLUSH);
  if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
      perror("com set error");
      return (-1);
    }
  return (0);
}


int			command_parser(char *cmd)
{
  char			*gps_output;
  char			*time;
  char			*status;
  char			*date;
  char			buf[3];
  struct tm		tm;
  time_t		timep;
  struct timeval	tv;

  gps_output = strtok(cmd, ",");
  if (strcmp("$GPRMC", gps_output) != 0)
    return 1;
  time = strtok(NULL, ",");
  status = strtok(NULL, ",");
  if (strcmp("A", status) != 0)
    return 1;
  strtok(NULL, ",");
  strtok(NULL, ",");
  strtok(NULL, ",");
  strtok(NULL, ",");
  strtok(NULL, ",");
  strtok(NULL, ",");
  date =  strtok(NULL, ",");
  buf[2] = '\0';
  buf[1] = time[5];
  buf[0] = time[4];
  tm.tm_sec = atoi(buf);
  buf[1] = time[3];
  buf[0] = time[2];
  tm.tm_min = atoi(buf);
  buf[1] = time[1];
  buf[0] = time[0];
  tm.tm_hour = atoi(buf) + 8;
  buf[1] = date[5];
  buf[0] = date[4];
  tm.tm_year = atoi(buf) + 100;
  buf[1] = date[3];
  buf[0] = date[2];
  tm.tm_mon = atoi(buf) - 1;
  buf[1] = date[1];
  buf[0] = date[0];
  tm.tm_mday = atoi(buf);
  timep = mktime(&tm);
  tv.tv_sec = timep;
  printf("%s\n", asctime(localtime(&timep)));
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);
  return (0);
}

int	concat_command(char *cmd, char c)
{
  int	i;

  i = 0;
  if (c == '\n')
  {
    command_parser(cmd);
    memset(cmd, '\0', 256);
    return (0);
  }
  while (cmd[i] != '\0')
    {
      i = i + 1;
    }
  cmd[i] = c;
  return (0);
}

int	main()
{
  int	fd;
  int	nread;
  char	command[256];
  char	buf;
  int	nset;

  fd = open("/dev/ttySAC1", O_RDONLY);
  if (fd == -1)
    {
      printf("open failed\n");
      exit(1);
    }
  nset = set_opt(fd, 4800, 8, 'N', 1);
  if (nset == -1)
    {
      printf("set_opt failed\n");
      exit(1);
    }
  memset(command, '\0', 256);
  while	(1)
    {
      nread = read(fd, &buf, 1);
      if (nread > 0)
	{
	  concat_command(command, buf);
	  if (buf == 'q')
	    break;
	}
    }
  close(fd);
  return (0);
}
