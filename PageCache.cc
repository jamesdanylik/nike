#include "PageCache.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
  string hostname = request.GetHost();
  int port = request.GetPort();
  string port_string = to_string(port);
  string delim = ":";
  string path = request.GetPath();
  string url = hostname + delim + port_string  + path;

  int i = 0;
  cout << "Searching for cached page" << endl;
  while( cache[i].active ) 
  {
    if( cache[i].url == url )
    {
      return cache[i];
    }
    i++;
  }
  
  cout << "None found, allocating on free spot" << endl;
  if( i < PAGE_CACHE_SIZE)
  {
    cache[i] = fetchPage(request); 
    cache[i].url = url;
    cache[i].active = true;
    return cache[i];
  }

  evictOldest();
  cache[PAGE_CACHE_SIZE-1] = fetchPage(request);
  cache[PAGE_CACHE_SIZE-1].url = url;
  cache[PAGE_CACHE_SIZE-1].active = true;  
  return cache[PAGE_CACHE_SIZE-1];
}

pagecache_t PageCache::fetchPage(HttpRequest request)
{
  pagecache_t answer;

  string hostname = request.GetHost();
  int port = request.GetPort();
  string port_string = to_string(port);
  string delim = ":";
  string path = request.GetPath();
  string url = hostname + delim + port_string + path;

  char request_buffer[request.GetTotalLength()];
  request.FormatRequest(request_buffer);

  int socket = cxns.getCxn(hostname, port);
  write(socket, request_buffer, request.GetTotalLength());

  char* intrl_buffer = (char*) malloc(1);
  char* response_buffer = (char*) malloc(1);
  int intrl_bufferLen = 0, response_bufferLen = 0, response_bufferSize = 1;
  HttpResponse response;  

  while( (intrl_bufferLen = recv(socket, intrl_buffer,1,0)) > 0 )
  {
    if ( response_bufferLen + intrl_bufferLen >= response_bufferSize)
    {
      int oldsize = response_bufferSize;
      response_bufferSize += 1;
      char* new_buffer = (char*) malloc(response_bufferSize);
      memcpy(new_buffer, response_buffer, oldsize);
      free(response_buffer);
      response_buffer = new_buffer;
    }

    memcpy(response_buffer + response_bufferLen, intrl_buffer, intrl_bufferLen);
    response_bufferLen += intrl_bufferLen;

    if ( response_bufferLen >= 4 && 
         response_buffer[response_bufferLen-4] == '\r' && 
         response_buffer[response_bufferLen-3] == '\n' &&
         response_buffer[response_bufferLen-2] == '\r' && 
         response_buffer[response_bufferLen-1] == '\n' )
    {
      cout << "Received response from remote server!" << endl;
      try {
         response.ParseResponse(response_buffer, response_bufferLen);
      } catch (exception e) {
         perror("bad response from remote server");
         exit(1);
      }

      answer.header = response;
      
      intrl_bufferLen = 0;
      response_bufferLen = 0;
      int content_len = atoi(response.FindHeader("Content-Length").c_str());
      if( content_len > 0 )
      {
        while( (intrl_bufferLen = recv(socket, intrl_buffer,1,0)) > 0 )
        {
          if ( response_bufferLen + intrl_bufferLen >= response_bufferSize)
          {
            int oldsize = response_bufferSize;
            response_bufferSize += 1;
            char* new_buffer = (char*) malloc(response_bufferSize);
            memcpy(new_buffer, response_buffer, oldsize);
            free(response_buffer);
            response_buffer = new_buffer;
         }

         memcpy(response_buffer + response_bufferLen, intrl_buffer, intrl_bufferLen);
         response_bufferLen += intrl_bufferLen;
         
         if ( response_bufferLen == content_len)
         {
           answer.content = string(response_buffer, (size_t) response_bufferLen);
           return answer;
         }
        }
      }
      answer.content = "";
      return answer;
    }
  }
  return answer;
}

void PageCache::evictOldest()
{
  cache[0] = pagecache_t();
  for(int i=PAGE_CACHE_SIZE; i>0; i--)
  {
    if( cache[i].active )
    {
      cache[0] = cache[i];
      return;
    }
  } 
}
