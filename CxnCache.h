#ifndef _CXN_CACHE_H_
#define _CXN_CACHE_H_

/* System Includes */
#include <mutex>
#include <string>

/* CxnCache Variables */
#define CXN_CACHE_SIZE 100

using namespace std;

/* A type to hold socket connections and data about them */
struct cxn_t
{
  cxn_t()
  {
    socket = -1;
    hostname = "";
    port = -1;  
  }
  int socket;
  string hostname;
  int port;
};

/* A class to cache host connections and attempt to reuse
 * them.  Also does basic error checking to make sure connections
 * are still open */
class CxnCache
{
public:
  CxnCache();
  ~CxnCache();
  int getCxn(string hostname, int port);
  void closeCxn(string hostname, int port);
private:
  cxn_t cache[CXN_CACHE_SIZE];
};

#endif // _CXN_CACHE_H_
