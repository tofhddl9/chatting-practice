#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static const int BUFSIZE = 1024;

int main(void)
{
  int sock_fd;
  struct sockaddr_in serv_addr;
  char recv_buffer[BUFSIZE], send_buffer[BUFSIZE];

  if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() error");
    exit(0);
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(7777);

  if ((connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
    perror("connect() error");
    exit(0);
  }

  int read_len;

  while (1) {
    scanf("%s", send_buffer);
    write(sock_fd, send_buffer, strlen(send_buffer));
    
    memset(&recv_buffer, 0, BUFSIZE);
    read_len = read(sock_fd, recv_buffer, BUFSIZE-1);
    recv_buffer[read_len] = '\0';
    printf("server reply : %s\n", recv_buffer);
  }

  close(sock_fd);
  return 0;
}
