#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

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
  int sockFD, maxFD = 0;
  int i = 0;

  char buf[bufSize];
  fd_set readFDs, allFDs; 

  if ((listenFD = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() error");
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

  maxFD = listenFD;  
  while (1) {
    allFDs = readFDs;
    printf("Select Wait %d\n", maxFD);
    FDNum = select(maxFD+1, &allFDs, (fd_set *)0, (fd_set *)0, NULL);
    
    if (FD_ISSET(listenFD, &allFDs)) {
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
      printf("accept OK\n");
      continue;
    }
    
    for (i=0;i<=maxFD;++i) {
      sockFD = i;
      if (FD_ISSET(sockFD, &allFDs)) {
        if (readn(sockFD, buf, 2) == 0) {
          break;
        }
        short recvLen = *(short *)&buf;
        if (readn(sockFD, buf, recvLen) == 0) {
          printf("close\n");
          close(sockFD);
          FD_CLR(sockFD, &readFDs);
        }
        else {
          buf[recvLen] = 0;
          write(sockFD, buf, strlen(buf));
        }
        if (--FDNum <= 0) {
          break;
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
    sp += readLen;
  }
  return 1;
}
