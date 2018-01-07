#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>

#include <arpa/inet.h>

static const int bufSize = 1024;
static const int epollSize = 50;

int readn(int fd, char *buf, short n);
int writen(int fd, char *buf, short n);

int main(void)
{
  int listenFD, connectFD;
  struct sockaddr_in serverAddr, clientAddr;
  char buf[bufSize];

  struct epoll_event event, *events;
  int epollFD, eventNum;
  int i,n,clientAddrLen;

  if ((listenFD = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
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

  if (listen(listenFD, 128) < 0) {
    perror("listen() error");
    exit(1);
  }

  if ((epollFD = epoll_create(epollSize)) < 0) {
    perror("epoll_create() error");
    exit(1);
  }
  
  events = malloc(sizeof(struct epoll_event) * epollSize);

  event.events = EPOLLIN;
  event.data.fd = listenFD;
  epoll_ctl(epollFD, EPOLL_CTL_ADD, listenFD, &event);

  while (1) {
    if ((eventNum = epoll_wait(epollFD, events, epollSize, -1)) < 0) {
      perror("epoll_wait() error");
      exit(1);
    }

    for (i=0; i<eventNum; ++i) {
      if (events[i].data.fd == listenFD) {
        clientAddrLen = sizeof(clientAddr);
        if ((connectFD = accept(listenFD, (struct sockaddr *)&clientAddr,
          &clientAddrLen)) < 0) {
            perror("accept() error");
            exit(1);
        }

        event.events = EPOLLIN;
        event.data.fd = connectFD;
        epoll_ctl(epollFD, EPOLL_CTL_ADD, connectFD, &event);
      }

      else {
        memset(&buf, 0, bufSize);
        if ((n = readn(events[i].data.fd, buf, 2)) == 0) {
          printf("closed client : %d\n",events[i].data.fd);
          epoll_ctl(epollFD, EPOLL_CTL_DEL, events[i].data.fd, NULL);
          close(events[i].data.fd); 
          break;
        }
        short recvLen = *(short *)&buf;
        printf("recvLen : %d\n", recvLen);

        if((n = readn(events[i].data.fd, buf, recvLen)) == 0) {
          printf("closed client : %d\n",events[i].data.fd);
          epoll_ctl(epollFD, EPOLL_CTL_DEL, events[i].data.fd, NULL);
          close(events[i].data.fd);
          //break;
        }

        //else {
          write(events[i].data.fd, &recvLen, 2);
          buf[recvLen] = 0;
          writen(events[i].data.fd, buf, recvLen);
        //}

      }

    }
  }

  return 0;
}

int readn(int fd, char *buf, short n)
{
  short sp = 0, readed;
  while (n) {
    readed = read(fd, buf+sp, n);
    if (readed < 0) {
      perror("read error");
      exit(1);
    }
    n -= readed;
    sp += readed;
  }
  if (readed == 0) {
    close(fd);
    return 0;
  }
  return 1;
}

int writen(int fd, char *buf, short n)
{
  short sp = 0, wroted;
  while (n) {
    wroted = write(fd, buf+sp, n);
    if (wroted < 0) {
      perror("write error");
      exit(1);
    }
    n -= wroted;
    sp += wroted;
  }
  if (wroted == 0) {
    close(fd);
    return 0;
  }
  return 1;
}
