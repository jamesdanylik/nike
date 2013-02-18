#include "PageCache.h"

#include <mutex>

#include "http-request.h"

using namespace std;

PageCache::PageCache()
{
  cacheLock.unlock();
}

PageCache::~PageCache()
{
}

pagecache_t PageCache::getPage(HttpRequest request)
{
  pagecache_t none;
  return none;
}

void PageCache::cachePage(pagecache_t page)
{
}

