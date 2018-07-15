#include "tinywebserver.h"

int Dup2(int fd1, int fd2)
{
    int rc;

    if ((rc = dup2(fd1, fd2)) < 0)
	printf("Dup2 error");
    return rc;
}
int main(int argc, char *argv[])
{
    RunWebServer(argc, argv);
    return 0;
}
