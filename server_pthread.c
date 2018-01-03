#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#include <pthread.h>
#include <arpa/inet.h>

int threadNum = 1024;
int bufSize = 1024;

int readn(int fd, char *buf, short n);
void *ThFunc(void *a);

int main(void)
{
  int listenFD, connectFD[threadNum];
  struct sockaddr_in serverAddr, clientAddr;
  pthread_t th[threadNum];

  if ((listenFD = socket(PF_INET, SOCK_STREAM, 0)) < 0){
    perror("socket() error");
    exit(1);
  }
  
  if (setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
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

  signal(SIGCHLD, SIG_IGN);

  int status, thIdx = 0;
  while (1) {
    int clientAddrLen = sizeof(clientAddr);
    if ((connectFD[thIdx] = accept(listenFD, (struct sockaddr *)&clientAddr
            , &clientAddrLen)) < 0) {
      perror("accept() error");
      exit(1);
    }
    
    if ((status = pthread_create(&th[thIdx], NULL, &ThFunc ,
            (void *)&connectFD[thIdx])) != 0) {
      printf("%d thread create error\n", thIdx);
      exit(1);
    }
    pthread_detach(th[thIdx]);

    ++thIdx;
  }

  return 0;
}

void *ThFunc(void *a)
{
  int fd = (*(int *)a), readLen;
  char recvBuf[bufSize];
  while (1) {
    memset(recvBuf, 0, bufSize);
    if (readn(fd, recvBuf, 2) == 0) {
      break;
    }
    readLen = *(short *)&recvBuf;
    printf("readLen : %d\n",readLen);

    if (readn(fd, recvBuf, readLen) == 0) {
      break;
    }
    recvBuf[readLen] = 0;

    write(fd, recvBuf, readLen);
  }
}

int readn(int fd, char *buf, short n)
{
  short sp = 0, readed;
  while (n) {
    readed = read(fd, buf + sp, n);
    if(readed <= 0) {
      return 0;
    }

    n-= readed;
    sp += readed;
  }

  return 1;
}
