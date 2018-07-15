#ifndef __TINYWEBSERVER_H__
#define __TINYWEBSERVER_H__
#include "csapp.h"
#define MAXBUF 8192  /* Max I/O buffer size */
extern char **environ;  // 全局变量environ, 环境变量指针数组

void RunWebServer(int argc, const char *argv[]);
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(const char *uri, const char *filename, const char *cgiargs);
void serve_static(int fd, const char *filename, int filesize);
void get_filetype(const char *filename, const char *filetype);
void serve_dynamic(int fd, const char *filename, const char *cgiargs);
void clienterror(int fd, const char *cause, const char *errnum, const char *shortmsg, const char *longmsg);


#endif //__TINYWEBSERVER_H__
