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
