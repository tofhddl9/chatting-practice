#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <arpa/inet.h>

static const int BUFSIZE = 1024;

int readn(int fd, char *buf, short n);
int writen(int fd, char *buf, short n);

int main(void)
{
  int listenFD, connectFD;
  struct sockaddr_in listenSocket, connectSocket;
  char buffer [BUFSIZE];
  
  if ((listenFD = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() error\n");
    exit(0);
  }
  
  if (setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
    perror("sockopt error\n");
    exit(0);
  }

  memset(&listenSocket, 0, sizeof(listenSocket));
  listenSocket.sin_family = AF_INET;
  listenSocket.sin_addr.s_addr = inet_addr("0.0.0.0");
  listenSocket.sin_port = htons(7777);

  if (bind(listenFD, (struct sockaddr *)&listenSocket, sizeof(listenSocket)) < 0) {
    perror("bind() error\n");
    exit(0);
  }

  if (listen(listenFD, 128) < 0) {
    perror("listen() error\n");
    exit(0);
  }
  
  signal(SIGCHLD, SIG_IGN);

  int connectSocketLen, n;
  short readLen;
  pid_t pid;

  while (1) {
    connectSocketLen = sizeof(connectSocket);
    if ((connectFD = accept(listenFD, (struct sockaddr *)&connectSocket,
            &connectSocketLen)) < 0) {
      perror("accept() error\n");
      exit(0);
    }
    pid = fork();
    if (pid == 0) {
      close(listenFD);
      while (1) {
        memset(buffer, 0, BUFSIZE);
        if ((n = readn(connectFD, buffer, 2)) == 0) {
          printf("Bye Client\n");
          close(connectFD);
          break;
        }
        readLen = (*(short *)&buffer);
        
        if ((n = readn(connectFD, buffer, readLen)) == 0) {
          close(connectFD);
          break;
        }
        write(connectFD, &readLen, 2);
        writen(connectFD, buffer, readLen);
        sleep(0);
      }
      if (n == 0) {
        close(connectFD);
        break;
      }
    }
    
    else if (pid > 0) {
      close(connectFD);
    }

    else {
      perror("fork() error\n");
      exit(0);
    }

  }
  close(listenFD);
  return 0;
}

int readn(int fd, char *buf, short n)
{
  short sp = 0, readed;
  while (n) {
    readed = read(fd, buf + sp, n);
    if (readed < 0) {
      perror("read() error");
      exit(1);
    }
    else if (readed == 0) {
      return 0;
    }
    n -= readed;
    sp += readed;
  }
  if (readed == 0) {
    return 0;
  }

  return 1;
}

int writen(int fd, char *buf, short n)
{
  short sp = 0, wroted;
  while (n) {
    wroted = write(fd, buf + sp, n);
    if (wroted < 0) {
      perror("write() error");
      exit(1);
    }
    else if (wroted == 0) {
      return 0;
    }
    n -= wroted;
    sp += wroted;
  }
  if (wroted == 0) {
    return 0;
  }
  return 1;
}
