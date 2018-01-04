#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>

static const int maxConn = 1024;
static const int bufSize = 1024;

int readn(int fd, char *buf, short n);

int main(void)
{
  int listenFD, connectFD;
  struct sockaddr_in serverAddr, clientAddr;
  struct pollfd clientFDs[maxConn];
  char buf[bufSize];
  int i, FDNum = 1;
  int readLen, clientAddrLen;

  if ((listenFD = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() error");
    exit(1);
  }

  if (setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR,  &(int){1}, sizeof(int)) < 0) {
    perror("setsockopt() error");
    exit(1);
  }

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
  serverAddr.sin_port = htons(7777);

  if (bind(listenFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    perror("bind() error");
    exit(1);    
  }

  if (listen(listenFD, 5) < 0) {
    perror("listen() error");
    exit(1);
  }

  memset(&clientFDs, 0, sizeof(clientFDs));
  clientFDs[0].fd = listenFD;
  clientFDs[0].events = POLLIN;

  for (i=1;i<maxConn;++i) {
    clientFDs[i].fd = -1;
    clientFDs[i].events = POLLIN;
  }

  while (1) {
    if (poll(clientFDs, FDNum, 1000) < 0) {
      perror("poll() error");
      exit(1);
    }
    
    if (clientFDs[0].revents == POLLIN) {
      if ((connectFD = accept(listenFD, (struct sockaddr *)&clientAddr,
          &clientAddrLen)) < 0) {
            perror("accept error");
            exit(1);
      }
        
      for (i=0;i<FDNum;++i) {
        if (clientFDs[i].fd == -1) {
          clientFDs[i].fd = connectFD;
          clientFDs[i].revents = POLLIN;
          printf("new client! %d\n",connectFD);
          break;
        }
      }
      if(i == FDNum) {
        clientFDs[i].fd = connectFD;
        clientFDs[i].events = POLLIN;
        FDNum++;
      } 
    }

    for (i=1;i<FDNum;++i) {
      if (clientFDs[i].revents == POLLIN) {
        memset(&buf, 0, bufSize);
        if (readn(clientFDs[i].fd, buf, 2) == 0) {
          break;
        }
        readLen = *(short *)buf;
        printf("readLen : %d\n", readLen);
  
        if (readn(clientFDs[i].fd, buf, readLen) == 0) {
          close(clientFDs[i].fd);
          break;
        }
        
        else {
          buf[readLen] = 0;
          write(clientFDs[i].fd, buf, strlen(buf));
        }
      }
    }
  }
  return 0;
}


int readn(int fd, char* buf, short n)
{
  short sp = 0, readed;
  while (n) {
    if ((readed = read(fd, buf+sp, n)) <= 0) {
      return 0;
    }
    sp += readed;
    n -= readed;
  }
  return 1;
}
