#ifndef __CSAPP_H__
#define __CSAPP_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RIO_BUFSIZE 8192
#define LISTENQ 1024
#define MAXLINE 1024
typedef struct sockaddr SA;
/*********************************
*RIO的无缓冲输入输出函数         *
*********************************/
ssize_t rio_readn(int fd, void *usrbuf, size_t n);    // 遇到EOF返回不足值
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

/*********************************
*RIO的带缓冲输入输出函数         *
*********************************/
typedef struct
{
    int rio_fd;
    int rio_cnt;
    char *rio_bufptr;
    char rio_buf[RIO_BUFSIZE];
}rio_t;

void rio_readinitb(rio_t *rp, int fd);    // 将fd与缓冲区rit_t 联系起来
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

/*********************************
*套接字包装函数                  *
*********************************/
// 与监听在端口port上的主机host_name建立连接，并返回套接字描述符,
int open_clientfd(const char *host_name, const char *port);

// 打开和返回一个监听描述符，描述符子在端口port上接收连接请求
int open_listenfd(const char *port);








































#endif /* __CSAPP_H__ */

