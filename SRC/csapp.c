#include "csapp.h"

ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while(nleft > 0)
    {
        nread = read(fd, bufp, nleft);

        if(nread < 0)  // 读取失败
        {
            if(errno == EINTR)  // 信号中断
                nread = 0;
            else
                return -1;
        }
        else if(nread == 0)
        {
            break;   // EOF
        }

        nleft -= nread;
        bufp += nread;
    }
    return (n-nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while(nleft > 0)
    {
        nwritten = write(fd, bufp, nleft);
        if(nwritten <= 0)
        {
            if(errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }

    return n;
}

void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    while(rp->rio_cnt <=0)
    {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0)
        {
            if(errno != EINTR)
                return -1;
        }
        else if(rp->rio_cnt == 0)
        {
            return 0;
        }
        else
        {
            rp->rio_bufptr = rp->rio_buf;
        }
    }

    int cnt = n;
    if(rp->rio_cnt < n)
        cnt = rp->rio_cnt;

    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;

    return cnt;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;
    for(n=1; n<maxlen; n++)
    {
        rc = rio_read(rp, &c,1);
        if(rc == 1)
        {
            *bufp++ = c;
            if(c == '\n')
            {
                n++;
                break;
            }
        }
        else if(rc == 0)
        {
            if(n == 1)
                return 0;
            else
                break;
        }
        else
        {
            return -1;
        }
    }
    *bufp = 0;
    return n-1;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0)
    {
        if ((nread = rio_read(rp, bufp, nleft)) < 0)
            return -1;          /* errno set by read() */
        else if (nread == 0)
            break;              /* EOF */

        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}

int open_clientfd(const char *host_name, const char *port)
{

    int clientfd;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM; // tcp
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG;
    getaddrinfo(host_name, port, &hints, &listp);

    // 遍历获取到的IP地址
    for(p = listp;p;p = p->ai_next)
    {
        // 创建套接字
        clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(clientfd < 0)
            continue;

        // 建立连接
        int con = connect(clientfd, p->ai_addr, p->ai_addrlen);
        if(con != -1)
            break;
        close(clientfd);
    }

    freeaddrinfo(listp);
    if(!p)
        return -1;
    else
        return clientfd;
}

int open_listenfd(const char *port)
{
    struct addrinfo hints, *listp, *p=NULL;
    int listenfd, optval = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; // tcp
    hints.ai_flags = AI_PASSIVE|AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    getaddrinfo(NULL, port, &hints, &listp);
     // 遍历获取到的IP地址
    for(p = listp;p;p = p->ai_next)
    {
        // 创建套接字
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(listenfd < 0)
            continue;

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
        // 绑定
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
        {
            printf("Bind port succeed!\n");
            break;
        }

        close(listenfd);
    }

    freeaddrinfo(listp);
    if(!p)
    {
        printf("No IP to use!\n");
        return -1;
    }

    if(listen(listenfd, LISTENQ) < 0)
    {
        printf("Listen port failed!\n");
        close(listenfd);
        return -1;
    }
    // Print Socket Info
    printf("Listen port succeed!\n");
    printf("port: %s\n",port);
    return listenfd;

}









































