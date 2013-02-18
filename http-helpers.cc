#include "http-helpers.h"

#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void* connectionHandler(void* args)
{
  exit(0);
}

bool isClosed(int socket)
{
  fd_set rfd;
  FD_ZERO(&rfd);
  FD_SET(socket, &rfd);
  timeval tv = { 0 };
  select(socket+1, &rfd, 0, 0, &tv);
  if ( !FD_ISSET(socket, &rfd) )
    return false;
  int n = 0;
  ioctl(socket, FIONREAD, &n);
  return n == 0;
}
