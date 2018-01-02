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

int main(void)
{
  int cnt = 0;
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

  if (listen(listenFD, 1) < 0) {
    perror("listen() error\n");
    exit(0);
  }
  
  signal(SIGCHLD, SIG_IGN);

  int connectSocketLen;
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
    cnt++;
    if (pid == 0) {
      //close(listenFD);

      while (1) {
        memset(buffer, 0x00, BUFSIZE);
        printf("[%d] : bf read1\n",cnt);
        if (readn(connectFD, buffer, 2) == 0) {
          printf("[%d] Bye1\n", cnt);
          break;
        }
        readLen = *(short *)&buffer;
        printf("[%d] : bf read2\n",cnt);

        if (readn(connectFD, buffer, readLen-2) == 0) {
          printf("[%d] Bye2\n", cnt);
          break;
        }
        buffer[readLen-2] = 0;
        int n;
        printf("[%d] : bf write\n", cnt);
        if ((n = write(connectFD, buffer, readLen-2)) <= 0) {
          perror("!!");
        }
        sleep(0);
      }
      //close(connectFD);
      exit(0);
    }
    
    else if (pid > 0) {
      //close(connectFD);
      printf("[%d] hello i'm parent\n", cnt);
    }

    else {
      perror("fork() error\n");
      exit(0);
    }
  }
  //close(listenFD);

  return 0;
}

int readn(int fd, char *buf, short n)
{
  short sp = 0, readed;
  while (n) {
    readed = read(fd, buf + sp, n);
    if (readed <= 0) {
      return 0;
    }
    n -= readed;
    sp += n;
  }
  return 1;
}
