#include "CxnCache.h"

#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http-helpers.h"

using namespace std;

CxnCache::CxnCache()
{
}

CxnCache::~CxnCache()
{
}

int CxnCache::getCxn(string hostname, int port)
{
  int i = 0;
  while ( cache[i].port != -1 && i < CXN_CACHE_SIZE )
  { // While there's allocated connections, search them for the one we want
    if ( cache[i].hostname == hostname && cache[i].port == port)
    { // If we find it, check if its closed; return it if not, else close/retry
      cout << "Found existing connection" << endl;
      if ( !isClosed(cache[i].socket) )
      { 
        cout << "It was open" << endl;
        return cache[i].socket;
      } else {
        cout << "It was closed" << endl;
        closeCxn(hostname, port);
        return getCxn(hostname, port);        
      }    
    }
    i++;    
  }

  // If we get here, there was ni appropraite connection.  i is now the first
  // unallocated catch connection's index or two big if the cache was full
  if ( i < CXN_CACHE_SIZE ) 
  { // If i < the max size, this is a valid unallocated catch block;
    // allocate it and return it
    cout << "No connection found but there's space!" << endl;
    cache[i].socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if ( cache[i].socket == -1 ) 
    {
      perror("failed to allocate socket");
      exit(1);
    }

    int yes = 1;
    if ( setsockopt(cache[i].socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
    {
      perror("failed to set reuse on socket");
      exit(1);
    }
    if ( setsockopt(cache[i].socket, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) == -1 )
    {
      perror("failed to set keepalive on socket");
      exit(1);
    }
   
    struct sockaddr_in serv_addr;
    initSockAddr(&serv_addr, hostname.c_str(), (uint16_t) port);
    int serv_len = sizeof(serv_addr);
    if ( connect(cache[i].socket, (struct sockaddr*) &serv_addr, (socklen_t) serv_len) < 0)
    {
      perror("connection to remote host failed");
      exit(1);
    }
    cache[i].hostname = hostname;
    cache[i].port = port;
    cout << "Allocating in the middle" << endl;
    return cache[i].socket;
  }


  cout << "removing and allocating on the end" << endl;
  closeCxn(cache[0].hostname, cache[0].port);
  cache[CXN_CACHE_SIZE-1].socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if ( cache[CXN_CACHE_SIZE-1].socket == -1 )
  {
    perror("failed to allocate socket");
    exit(1);
  }

  int yes = 1;
  if ( setsockopt(cache[CXN_CACHE_SIZE-1].socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
  {
    perror("failed to set reuse on socket"); 
    exit(1);
  }
  if ( setsockopt(cache[CXN_CACHE_SIZE-1].socket, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) == -1 )
  {
    perror("failed to set keepalive on socket");
    exit(1);
  }

  struct sockaddr_in serv_addr;
  initSockAddr(&serv_addr, hostname.c_str(), (uint16_t) port);
  int serv_len = sizeof(serv_addr);
  if ( connect(cache[CXN_CACHE_SIZE-1].socket, (struct sockaddr*) &serv_addr, (socklen_t) serv_len) < 0)
  {
    perror("connection to remote host failed");
    exit(1);
  }
  cache[CXN_CACHE_SIZE-1].hostname = hostname;
  cache[CXN_CACHE_SIZE-1].port = port;
  return cache[CXN_CACHE_SIZE-1].socket;
  
}

void CxnCache::closeCxn(string hostname, int port)
{
  for( int i=0; i < CXN_CACHE_SIZE; i++)
  {
    if ( cache[i].hostname == hostname && cache[i].port == port )
    {
      close(cache[i].socket);
      cache[i].socket = -1;
      cache[i].hostname = "";
      cache[i].port = -1;
      for ( int j=CXN_CACHE_SIZE-1; j > i; j--)
      {
        if( cache[j].port != -1 )
        {
          cache[i].port = cache[j].port;
          cache[i].hostname = cache[j].hostname;
          cache[i].socket = cache[j].socket;
          return;
        }
      }
      return;
    }
  }
  return;
}
