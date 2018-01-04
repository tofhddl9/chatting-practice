#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <netinet/in.h>
#include <arpa/inet.h>

static const int bufSize = 1024;

int readn(int fd, char *buf, short readLen);
int main(void)
{
  int listenFD, connectFD;
  struct sockaddr_in serverAddr, clientAddr;
  int clientAddrLen;
  int FDNum;
  int i;

  char buf[bufSize];
  fd_set readFDs, copyFDs; 

  if ((listenFD = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() error");
    exit(1);
  }

  if(setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
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
  
  FD_ZERO(&readFDs);
  FD_SET(listenFD, &readFDs);
  int maxFD = listenFD;

  while (1) {
    copyFDs = readFDs;
    
    FDNum = select(maxFD+1, &copyFDs, NULL, NULL, NULL);
   
    if (FDNum == 0)
      continue;
     
    if (FD_ISSET(listenFD, &copyFDs)) {
      clientAddrLen = sizeof(clientAddr);
      if ((connectFD = accept(listenFD, (struct sockaddr *)&clientAddr,
        &clientAddrLen)) < 0) {
          perror("accept() error");
          exit(1);
      }
      FD_SET(connectFD, &readFDs);
      if (connectFD > maxFD) {
        maxFD = connectFD;
      }
    }
    
    for (i=0;i<=maxFD;++i) {
      if (FD_ISSET(i, &copyFDs)) {
        memset(&buf, 0, sizeof(buf));
        if (readn(i, buf, 2) == 0) {
          break;
        }
        short recvLen = *(short *)&buf;
        printf("[%d] recvLen : %d\n",i,recvLen);
        if (readn(i, buf, recvLen) == 0) {
          printf("close\n");
          close(i);
          FD_CLR(i, &readFDs);
        }
        else {
          buf[recvLen] = 0;
          write(i, buf, strlen(buf));
        }
      }
    }
  }
  return 0;
}

int readn(int fd, char *buf, short readLen)
{
  short sp = 0, readed;
  while (readLen) {
    readed = read(fd, buf + sp, readLen);
    if (readed <= 0) {
      return 0;
    }
    readLen -= readed;
    sp += readed;
  }
  return 1;
}
