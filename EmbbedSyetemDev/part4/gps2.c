#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>

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

int		command_parser(char *cmd)
{
  char		*gps_output;
  char		*status;
  double	latitude;
  double	longitude;
  double	ground_speed;

  gps_output = strtok(cmd, ",");
  if (strcmp("$GPRMC", gps_output) != 0)
    return 1;
  strtok(NULL, ",");
  status = strtok(NULL, ",");
  if (strcmp("A", status) != 0)
    return 1;
  latitude = atof(strtok(NULL, ","));
  strtok(NULL, ",");
  longitude = atof(strtok(NULL, ","));
  strtok(NULL, ",");
  ground_speed = atof(strtok(NULL, ",")) * 1.85200;
  strtok(NULL, ",");
  strtok(NULL, ",");
  printf("latitude=North %f’\nlongitude=East %f’\nground speed = %f kph\n", latitude, longitude, ground_speed);
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
