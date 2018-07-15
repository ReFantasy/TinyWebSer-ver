#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
int main(int argc, char **argv, char **envp)
{
    printf("dynamic...\n\n");
    char buf[8192];
    char s[] =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta charset=\"utf-8\">"
    "<title>菜鸟教程(runoob.com)</title>"
    "</head>"
    "<body>"

    "<h1>我的第er个标题</h1>"
    "<p>我的第er个段落。</p>"

    "</body>"
    "</html>";

    // 发送报头给客户端
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, strlen(s));
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, "text/html");

    write(STDOUT_FILENO, buf, strlen(buf));


    // 发送请求内容
    write(STDOUT_FILENO, s, strlen(s));
}
