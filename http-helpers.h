#ifndef _HTTP_HELPERS_H_
#define _HTTP_HELPERS_H_

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

void* cxnHandler(void* args);
bool isClosed(int socket);
void initSockAddr( struct sockaddr_in *name,
                   const char * hostname,
                   uint16_t port);

#endif // _HTTP_HELPERS_H_
