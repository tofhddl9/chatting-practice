#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

static const int BUFSIZE = 1024;

int main(int argc,char *argv[])
{
  signal(SIGCHLD, SIG_IGN);
  fork();
  fork();
  fork();
  fork();
  fork();
  fork();
  fork();
  fork();
  fork();
  fork();

//  FILE *fp = fopen("output.txt", "w");
  char length[2], recvBuf[BUFSIZE];
  char buf[]="hello, world\0";
  short len = strlen(buf);
  sprintf(length,"%c",len);
  int client_sockfd, size, i, n, state;

  uint64_t delta_us = 0;

  struct sockaddr_in server_addr;
  struct timespec start, end;

  client_sockfd = socket(PF_INET, SOCK_STREAM, 0);
  memset(&server_addr, 0, sizeof server_addr);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(7777);

  inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);

  state = connect(client_sockfd, (struct sockaddr *)&server_addr, 
      sizeof server_addr);

  if (state < 0) {
    perror("connect err");
    exit(1);
  }

  for (i=0;i<10;i++) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    //printf("[%d]bf write\n",getpid());
    n = write(client_sockfd, length, sizeof length);
    if (n<=0) {
      perror("write err");
      exit(1);
    }
    printf("length : %d%d\n",length[1],length[0]);
    //printf("[%d]bf read1\n",getpid());
    n = write(client_sockfd, buf, (*(short *)length));
    if (n<=0) {
      perror("write err");
      exit(1);
    }
    printf("msg : %s\n",buf);

    //printf("[%d]bf read2\n",getpid());
    n = read(client_sockfd, recvBuf, (*(short *)length));
    if (n<=0) {
      perror("read err");
      exit(1);
    }
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    delta_us += (end.tv_sec - start.tv_sec) * 1000000 +
      (end.tv_nsec - start.tv_nsec)/1000;
    //printf("%lu\n", delta_us);
    sleep(1);
    
  }
  return 0;
}
