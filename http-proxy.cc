/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http-request.h"
#include "http-response.h"
#include "http-helpers.h"
#include "CxnCache.h"
#include "PageCache.h"

#define LISTEN_PORT 14805

using namespace std;

void* cxnHandler(void* args)
{
  struct thread_data* my_data;
  my_data = (struct thread_data*) args;

  int socket = my_data->socket;
  int* numClients = my_data->numClients;
  mutex* numClientsLock = my_data->numClientsLock;
  PageCache* pc = my_data->pc;

  char* intrl_buffer = (char*) malloc(1);
  char* response_buffer = (char*) malloc(1);
  int intrl_bufferLen = 0, response_bufferLen = 0, response_bufferSize = 1;
  HttpRequest response;

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
      cout << "Received response from client!" << endl;
      try {
         response.ParseRequest(response_buffer, response_bufferLen);
      } catch (exception e) {
         perror("bad response message from client");
         exit(1);
      }
      pagecache_t rep;
      (*pc).cacheLock.lock();
      rep = (*pc).getPage(response);
      (*pc).cacheLock.unlock();

      char out_buffer[rep.header.GetTotalLength()];
      rep.header.FormatResponse(out_buffer);
      string out_string = string(out_buffer,rep.header.GetTotalLength());

      write(socket,out_string.data(), out_string.length());
      write(socket,rep.content.data(), rep.content.length());
      intrl_bufferLen = 0;
      response_bufferLen = 0;
    }
  }
  (*numClientsLock).lock();
  (*numClients)++;
  (*numClientsLock).unlock();
  exit(0);
}

int main (int argc, char *argv[])
{
  int listenFD, clientFD;
  struct sockaddr_in listenAddr, clientAddr;
  PageCache pc;
  int numClients = 0;
  mutex numClientsLock;
  numClientsLock.unlock();


  if ( -1 == (listenFD = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) ) 
  {
    perror("socket creation failed");
    return(1);
  }

  memset(&listenAddr, 0, sizeof(listenAddr));
  listenAddr.sin_family = AF_INET;
  listenAddr.sin_port = htons(LISTEN_PORT);
  listenAddr.sin_addr.s_addr = INADDR_ANY;

  int yes = 1;
  if ( setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
  {
    perror("failed to set reuse on socket");
  }

  if ( -1 == bind(listenFD, (struct sockaddr *) &listenAddr, sizeof(listenAddr))) 
  {
    perror("binding socket failed");
    close(listenFD);
    return(1);
  }

  if ( -1 == listen(listenFD, 10))
  {
    perror("socket failed to listen");
    close(listenFD);
    return(1);
  }

  int clientAddrSize = sizeof(clientAddr);
  thread_data pass_data[10];
  while(1)
  {
    clientFD = accept(listenFD, (struct sockaddr *) &clientAddr,(socklen_t *) &clientAddrSize);
    if ( 0 > clientFD )
    {
      perror("failed to accept connection");
      close(listenFD);
      return(EXIT_FAILURE);
    }
    
    pthread_t thread_id;
    pass_data[numClients].numClients = &numClients;
    pass_data[numClients].numClientsLock = &numClientsLock;
    pass_data[numClients].pc = &pc;
    pass_data[numClients].socket = clientFD;
    pthread_create(&thread_id, NULL, cxnHandler, &pass_data[numClients]);
    numClients++;
  }
}
