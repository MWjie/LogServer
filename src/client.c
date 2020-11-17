#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#define __USE_POSIX
#include <sched.h>
#include <pthread.h>

#include "log.h"
#include "release.h"


#define LOG_Test

#define LOG_DefaultAddrIP           ( "127.0.0.1" )
#define LOG_DefaultAddrPort         ( 32001 )
#define LOG_EpollRcvBufSize         ( 4 * 1024 )


STATIC VOID LOG_Version(VOID) 
{
    printf("LOG server v=%s sha=%s:%d bits=%d build=%s\n",
            LOG_VERSION,
            LOG_GIT_SHA1,
            atoi(LOG_GIT_DIRTY) > 0,
            sizeof(LONG) == 4 ? 32 : 64,
            LOG_BUILD_ID);
}

STATIC INT LOG_InitClientFd(VOID)
{
    INT socketfd    = -1;
    INT iErron      = 0;
    STATIC USHORT usClientPort = LOG_DefaultAddrPort;
    struct sockaddr_in clientaddr;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == socketfd)
    {
        iErron = errno;
        fprintf(stderr, "socket errno = %d", iErron);
        return -1;
    }
 
    bzero(&clientaddr, sizeof(struct sockaddr_in));
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(++usClientPort);
    clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (-1 == bind(socketfd, (struct sockaddr *)&clientaddr, sizeof(struct sockaddr)))
    {
        iErron = errno;
        fprintf(stderr, "bind errno = %d", iErron);
        close(socketfd);
        return -1;
    }

    return socketfd;
}

STATIC INT LOG_CreateClient(VOID)
{
    INT socketfd        = -1;
    INT iErron          = 0;
    INT iMsgLen         = 0;
    socklen_t iAddrLen  = sizeof(struct sockaddr);
    struct sockaddr_in serveraddr;
    CHAR *pcMsgBuf  = NULL;

    socketfd = LOG_InitClientFd();
    if (-1 == socketfd)
    {
        return -1;
    }

    bzero(&serveraddr, sizeof(struct sockaddr_in));
    serveraddr.sin_family       = AF_INET;
    serveraddr.sin_port         = htons(LOG_DefaultAddrPort);
    serveraddr.sin_addr.s_addr  = inet_addr(LOG_DefaultAddrIP);
        
    pcMsgBuf = (CHAR *)malloc(LOG_EpollRcvBufSize * sizeof(CHAR));
    if (NULL == pcMsgBuf)
    {
        close(socketfd);
        return -1;
    }

    iMsgLen = snprintf(pcMsgBuf, LOG_EpollRcvBufSize, "%d:%d", getpid(), getpid());
    sendto(socketfd, pcMsgBuf, iMsgLen, 0, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr));
    pcMsgBuf[0] = '\0';
    iMsgLen = recvfrom(socketfd, pcMsgBuf, LOG_EpollRcvBufSize, 0, (struct sockaddr *)&serveraddr, &iAddrLen);
    fprintf(stdout, "pid%d : recv = %s\n", getpid(), pcMsgBuf);

    close(socketfd);
    return 0;
}

INT main(IN INT argc, IN CHAR *argv[])
{
    LOG_Version();

#ifdef LOG_Test
    for (UINT uiIndex = 0; uiIndex < 5; uiIndex++)
    {
        LOG_CreateClient();
        if (0 < fork()) break;
    }
#endif
    

    return 0;

}


#ifdef __cplusplus
}
#endif


