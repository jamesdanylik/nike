#ifndef _PAGE_CACHE_H_
#define _PAGE_CACHE_H_

/* System Includes */
#include <mutex>
#include <string>

/* Program Includes */
#include "http-response.h"
#include "http-request.h"
#include "CxnCache.h"

/* Page Cache Variables */
#define PAGE_CACHE_SIZE 50

using namespace std;

/* A type for holding http responses and data about them */
struct pagecache_t
{
  pagecache_t() // Default constructor
  {
    active = false;
    content = "";
    url = "";
  }
  // Structure fields
  string url;
  HttpResponse header;
  string       content;
  bool         active;
};

class PageCache
{
public:
  // Methods
  PageCache();
  ~PageCache();
  pagecache_t getPage(HttpRequest request);
  pagecache_t fetchPage(HttpRequest request);
  void        evictOldest();  
  void        cachePage(pagecache_t page);
  // Class fields
  pagecache_t cache[PAGE_CACHE_SIZE];
  CxnCache    cxns;
  mutex       cacheLock;
};

#endif // _PAGE_CACHE_H_
