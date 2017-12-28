#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

static const int BUFSIZE = 1024;

int main(void)
{
  int serv_fd, clnt_fd;
  struct sockaddr_in serv_addr, clnt_addr;
  char buffer [BUFSIZE];

  if ((serv_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket() error");
    exit(0);
  }
  
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(7777);

  if (bind(serv_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("bind() error");
    exit(0);
  }

  if (listen(serv_fd, 5) < 0) {
    perror("listen() error");
    exit(0);
  }
  
  int pid, clnt_addr_len, read_len;

  while (1) {
    clnt_addr_len = sizeof(clnt_addr);
    if ((clnt_fd = accept(serv_fd, (struct sockaddr *)&clnt_addr, &clnt_addr_len)) < 0) {
      perror("accept() error");
      exit(0);
    }

    pid = fork();
    if (pid ==0) {
      while (1) {
        memset(buffer, 0, BUFSIZE);
        if ((read_len = read(clnt_fd, buffer, BUFSIZE)) <= 0) {
          close(clnt_fd);
          exit(0);
        }
        printf("client say : %s\n", buffer);
        write(clnt_fd, buffer, read_len);
      }
    }
    if (pid < 0) {
      perror("fork error");
      return 1;
    }
  }
  close(clnt_fd);
}
