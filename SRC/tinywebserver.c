#include "tinywebserver.h"
extern char **environ;
void RunWebServer(int argc, const char *argv[])
{
    int listenfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    // 检查命令行参数
    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // 打开监听端口
    listenfd = open_listenfd(argv[1]);
    if(listenfd == -1)
    {
        printf("Web Server couldn't listen port\n");
        return;
    }
    // 循环接收并处理请求
    while(1)
    {
        // 接收http请求
        clientlen = sizeof(clientaddr);
        int connfd = accept(listenfd, (SA*)&clientaddr, &clientlen);

        if(fork() == 0)
        {
             // 获取客户端信息并打印
            getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
            printf("Accept connection from (%s,%s)\n", hostname, port);

            // 执行请求
            doit(connfd);

            // 关闭连接
            close(connfd);
        }
    }
}

void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE],uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    // 读取并解析请求行
    // HTTP 请求行格式： method URI versin
    rio_readinitb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method,uri, version);

    // 检查请求类型
    if(strcasecmp(method, "GET"))
    {
        clienterror(fd, method, "501", "Not implemented","Tiny Web Server does not implement thid method");
        return;
    }

    // 读并忽略请求报头
    // 请求报头为服务器提供了额外的信息，如浏览器的商标等
    read_requesthdrs(&rio);

    // 从GET请求中解析URI,然后请求的是静态内容还是动态内容
    is_static = parse_uri(uri, filename, cgiargs);
    // 判断请求文件（静态文件或者动态内容生成文件）是否存在
    if(stat(filename, &sbuf) < 0)
    {
        clienterror(fd, filename, "404", "Not found", "Tiny Web Server couldn't find this file");
        return;
    }

    // 向客户端（浏览器）提供请求内容
    if(is_static)  // 提供静态内容
    {
        // 判断读权限
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            clienterror(fd, filename, "403", "Forbidden", "Tiny Web Server couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);

    }
    else  // 提供动态内容
    {
        // 判断可执行权限
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        {
            clienterror(fd, filename, "403", "Forbidden", "Tiny Web Server couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}

// 发送一个HTTP响应到客户端，响应中包含状态码和状态消息
void clienterror(int fd, const char *cause, const char *errnum, const char *shortmsg, const char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    // 构建html响应页面
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web Server</em>\r\n", body);

    // 返回响应
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));
}

// 不使用请求报头的任何信息，请求报头以 "\r\n" 与下面的信息分隔
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];
    rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf,"\r\n"))
    {
        rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

// 假设静态内容的目录就是它的当前目录，可执行文件的目录是 ./cgi-bin
// 任何包含字符串 cgi-bin 的 URI 都被认为是对动态内容的请求
// 默认静态主页 ./home.html
// 返回 1 静态请求， 0 动态请求
int parse_uri(const char *uri, const char *filename, const char *cgiargs)
{
    char *ptr;
    if(!strstr(uri, "cgi-bin"))  // 静态请求
    {
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if(uri[strlen(uri)-1] == '/')
            strcat(filename, "home.html");
        return 1;
    }
    else  // 动态请求
    {
        ptr = index(uri, '?');
        if(ptr)
        {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }
        else
        {
            strcpy(cgiargs, "");
        }
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}


void get_filetype(const char *filename, const char *filetype)
{
    if(strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if(strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if(strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if(strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}


// 提供五种常见类型的静态内容：
// HTML文件、无格式的文本文件、GIF/PNG/JPG 图片格式
void serve_static(int fd, const char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
    // 发送报头给客户端
    get_filetype(filename, filetype);

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    printf("filetype: %s\n", filetype);
    rio_writen(fd, buf, strlen(buf));


    // 发送请求内容
    srcfd = open(filename, O_RDONLY, 0);
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    rio_writen(fd, srcp, filesize);
    munmap(srcp, filesize);
}

void serve_dynamic(int fd, const char *filename, const char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL};

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    rio_writen(fd, buf, strlen(buf));
    printf("dynamic....\n\n");
    if(fork() == 0)
    {
        setenv("QUERY_STRING", cgiargs, 1);
        dup2(fd, STDOUT_FILENO);
        execve(filename, emptylist, environ);
    }
    wait(NULL);
}



























