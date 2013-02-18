/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <arpa/inet.h>
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

using namespace std;

int main (int argc, char *argv[])
{
  // command line parsing
  PageCache test;

  HttpRequest req;
  req.SetHost("www.whitehouse.gov");
  req.SetPort(80);
  req.SetPath("/");
  req.SetMethod(HttpRequest::MethodEnum::GET); 
  req.SetVersion ("1.1");
  req.AddHeader ("Accept-Language", "en-US"); 
  pagecache_t res;
  res = test.getPage(req);

  char buffer[res.header.GetTotalLength()];
  res.header.FormatResponse(buffer);
  string response_string = string(buffer,res.header.GetTotalLength());

  cout << response_string;
  cout << res.content;

  test.getPage(req);
  return 0;
}
