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
  CxnCache test1;
  PageCache test2;

  test1.getCxn("www.xkcd.com", 80);
  sleep(30);
  test1.getCxn("www.xkcd.com", 80);
  return 0;
}
