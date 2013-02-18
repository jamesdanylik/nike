#include "CxnCache.h"

#include <string>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


using namespace std;

CxnCache::CxnCache()
{
}

CxnCache::~CxnCache()
{
}

cxn_t CxnCache::getCxn(string hostname, int port)
{
  cxn_t none;
  return none;
}

bool CxnCache::isClosed(int socket)
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

