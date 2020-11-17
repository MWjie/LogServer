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

#define LOG_LocalSysLogPath         ( "./LocalSysLog" )
#define LOG_DefualtLogPath          ( "./Log/" )
#define LOG_DefaultAddrIP           ( "127.0.0.1" )
#define LOG_DefaultAddrPort         ( 32001 )
#define LOG_SysLogSwitch            ( 1 )
#define LOG_MaxEpollNum             ( 8 )
#define LOG_EpollRcvBufSize         ( 4 * 1024 )


STATIC INT LOG_CreateEpollEvent(VOID)
{
    INT socketfd    = -1;
    INT epollfd     = -1;
    INT iErron      = 0;
    INT iNoblock    = 1;
    INT iReuseAddr  = 1;
    INT iRcvBuf     = LOG_EpollRcvBufSize;
    INT infds       = -1;
    INT iIndex      = 0;
    LONG lRecvLen   = 0;
    CHAR *pcRcvBuf  = NULL;

    struct sockaddr_in ClientAddr;
    socklen_t iAddrLen = sizeof(ClientAddr);
    struct sockaddr_in serveraddr;
    struct epoll_event ev;
    struct epoll_event events[LOG_MaxEpollNum];

    pcRcvBuf = (CHAR *)malloc(iRcvBuf * sizeof(CHAR));
    if (NULL == pcRcvBuf)
    {
        iErron = errno;
        LOG_RawSysLog("malloc error, erron: %d\n", iErron);
        return -1;
    }

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(g_pstLogServerContext->szLogAddrIP);
    serveraddr.sin_port = htons(g_pstLogServerContext->usLogAddrPort);

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == socketfd)
    {
        iErron = errno;
        LOG_RawSysLog("socket error, erron: %d\n", iErron);
        return -1;
    }

    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (const CHAR *)&iReuseAddr, sizeof(iReuseAddr));
    setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (const CHAR *)&iRcvBuf, sizeof(iRcvBuf));
    ioctl(socketfd, FIONBIO, &iNoblock);

    if (-1 == bind(socketfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr)))
    {
        iErron = errno;
        LOG_RawSysLog("bind error, erron: %d\n", iErron);
        close(socketfd);
        return -1;
    }

    epollfd    = epoll_create(LOG_MaxEpollNum);
    ev.events  = EPOLLIN;
    ev.data.fd = socketfd;

    if (0 > epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &ev))
    {
        iErron = errno;
        LOG_RawSysLog("epoll add error, erron: %d\n", iErron);
        close(socketfd);
        return -1;
    }

    for (;;)
    {
        infds = epoll_wait(epollfd, events, LOG_MaxEpollNum, -1);
        if (infds == -1)
        {
            continue;
        }

        for (iIndex = 0; iIndex < infds; iIndex++)
        {
            if (0 != (events[iIndex].events & (EPOLLERR | EPOLLHUP)))
            {
                iErron = errno;
                LOG_RawSysLog("epoll error, erron: %d\n", iErron);
                close(events[iIndex].data.fd);
                continue;
            }

            if (0 != (events[iIndex].events & EPOLLIN))
            {
                lRecvLen = recvfrom(events[iIndex].data.fd, pcRcvBuf, LOG_EpollRcvBufSize, 0,
                                    (struct sockaddr *)&ClientAddr, &iAddrLen);
                if (0 < lRecvLen)
                {

                }
            }
        }
    }

    close(epollfd);
    close(socketfd);

    return 0;
}

STATIC VOID LOG_Version(VOID) 
{
    printf("LOG server v=%s sha=%s:%d bits=%d build=%s\n",
            LOG_VERSION,
            LOG_GIT_SHA1,
            atoi(LOG_GIT_DIRTY) > 0,
            sizeof(LONG) == 4 ? 32 : 64,
            LOG_BUILD_ID);
    exit(0);
}

INT main(IN INT argc, IN CHAR *argv[])
{
    LOG_Version();

#ifdef LOG_Test
    LOG_System_s("uname -a");
    for (;;)
    {
        sleep(1);
    }
#endif

    LOG_CreateEpollEvent();

    return 0;

}


#ifdef __cplusplus
}
#endif

