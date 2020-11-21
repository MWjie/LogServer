#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

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
#include "util.h"
#include "release.h"


//#define LOG_Test

#define LOG_LocalSysLogPath         ( "./LocalSysLog" )
#define LOG_DefualtLogPath          ( "./Log/" )
#define LOG_DefaultAddrIP           ( "127.0.0.1" )
#define LOG_DefaultAddrPort         ( 32001U )
#define LOG_SysLogSwitch            ( 0 )
#define LOG_MaxEpollNum             ( 8U )
#define LOG_EpollRcvBufSize         ( 256U )
#define LOG_ShmReserveMemery        ( 512U )


#define LOG_BIT_ROUNDUP(x, n)       ((x + (n - 1)) & (~(n - 1)))


LOGServerContext_S *g_pstLogServerContext = NULL;


STATIC INT LOG_InitContext(IN INT argc, IN CHAR *argv[])
{
    CHAR szShellBuf[128];
    CHAR szFileName[128];
    CHAR szOldFile[128];

    DIR *dir;
    struct dirent *ptr;

    g_pstLogServerContext = (LOGServerContext_S *)malloc(sizeof(LOGServerContext_S));
    if (NULL == g_pstLogServerContext)
    {
        return -1;
    }
    memset(g_pstLogServerContext, 0x0, sizeof(LOGServerContext_S));

    srand(time(NULL) ^ getpid());

    g_pstLogServerContext->pid = getpid();
    g_pstLogServerContext->stShmListHeader.uiNum     = 0;
    g_pstLogServerContext->stShmListHeader.pstHeader = NULL;

    g_pstLogServerContext->bIsLocalSyslog = LOG_SysLogSwitch;
    pthread_mutex_init(&g_pstLogServerContext->stLOGLocalSyslog.mutex, NULL);
    g_pstLogServerContext->stLOGLocalSyslog.fd = open(LOG_LocalSysLogPath, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (0 > g_pstLogServerContext->stLOGLocalSyslog.fd)
    {
        LOG_RawSysLog("Open %s error!\n", LOG_LocalSysLogPath);
        return -1;
    }

    g_pstLogServerContext->usLogAddrPort = LOG_DefaultAddrPort;
    strncpy(g_pstLogServerContext->szLogAddrIP, LOG_DefaultAddrIP,  sizeof(LOG_DefaultAddrIP));
    strncpy(g_pstLogServerContext->szFilePath,  LOG_DefualtLogPath, sizeof(LOG_DefualtLogPath));

    LOG_ParsePara(argc, argv);

    snprintf(szShellBuf, sizeof(szShellBuf), "mkdir -p %s", g_pstLogServerContext->szFilePath);
    LOG_System_s(szShellBuf);

    snprintf(szShellBuf, sizeof(szShellBuf), "find %s -name *.bak -exec rm -rf {} \\;", g_pstLogServerContext->szFilePath);
    LOG_System_s(szShellBuf);

    dir = opendir(g_pstLogServerContext->szFilePath);
    while (NULL != (ptr = readdir(dir)))
    {
        if (0 != strcmp(ptr->d_name, ".") && 0 != strcmp(ptr->d_name, ".."))
        {
            snprintf(szFileName, sizeof(szFileName), "%s%s.bak", g_pstLogServerContext->szFilePath, ptr->d_name);
            snprintf(szOldFile,  sizeof(szOldFile),  "%s%s",     g_pstLogServerContext->szFilePath, ptr->d_name);
            rename(szOldFile, szFileName);
        }
    }
    closedir(dir);

    return 0;
}



INT LOG_CheckShmConfig(IN CHAR *pcShmName, IN UINT uiShmPid)
{
    LOGShmListHeader_S *pstShmListHeader = &g_pstLogServerContext->stShmListHeader;
    LOGShmList_S *pstShmListNode = pstShmListHeader->pstHeader;

    for (; pstShmListNode != NULL; pstShmListNode = pstShmListNode->pstNext)
    {
        if (uiShmPid == pstShmListNode->pstShmHeader->uiClientPid && 
            0 == strcmp(pstShmListNode->pstShmHeader->szFileName, pcShmName))
        {
            LOG_RawSysLog("ShmName amd ShmPid exist\n");
            return -1;
        }
    }

    return 0;
}


STATIC CHAR *LOG_EpollCallback(IN CHAR *pcRcvBuf)
{
    pthread_t tid;
    INT iResult     = 0;
    UINT uiShmSize  = 0;
    UINT uiShmPid   = 0;
    CHAR *pcShmAddr = NULL;
    CHAR *pcShmName = NULL;
    CHAR *pcShmPid  = NULL;
    CHAR *pcStrTmp  = NULL;
    CHAR szRecBuf[LOG_EpollRcvBufSize];
    LOGShmHeader_S *pstShmHeader = NULL;
    LOGShmList_S *pstShmListNode = (LOGShmList_S *)malloc(sizeof(LOGShmList_S));

    strncpy(szRecBuf, pcRcvBuf, sizeof(szRecBuf));
    pcShmName = LOG_Strtok_r(szRecBuf, ":", &pcStrTmp);
    pcShmPid  = LOG_Strtok_r(NULL, ":", &pcStrTmp);
    if (NULL == pcShmName || NULL == pcShmPid || NULL == pcStrTmp)
    {
        LOG_RawSysLog("strtok error\n");
        free(pstShmListNode);
        pstShmListNode = NULL;
        return NULL;
    }

    uiShmSize = atoi(pcStrTmp);
    uiShmPid  = atoi(pcShmPid);
    if (0 != LOG_CheckShmConfig(pcShmName, uiShmPid))
    {
        free(pstShmListNode);
        pstShmListNode = NULL;
        return NULL;
    }

    pcShmAddr = LOG_OpenShm(pcShmName, uiShmSize);
    if (NULL == pcShmAddr)
    {
        LOG_RawSysLog("Shm open error\n");
        free(pstShmListNode);
        pstShmListNode = NULL;
        return NULL;
    }
    LOG_RawSysLog("/dev/shm/%s size:%u open\n", pcShmName, uiShmSize);

    pstShmHeader = (LOGShmHeader_S *)pcShmAddr;
    memset(pcShmAddr, 0x0, sizeof(LOGShmHeader_S));
    strncpy(pstShmHeader->szFileName, pcShmName, sizeof(pstShmHeader->szFileName));
    pstShmHeader->uiClientPid               = uiShmPid;
    pstShmHeader->uiShmSize                 = uiShmSize;
    pstShmHeader->pShmAddr_Server           = pcShmAddr;
    pstShmHeader->pShmStartOffset_Server    = pcShmAddr + sizeof(LOGShmHeader_S);
    pstShmHeader->pShmEndOffset_Server      = pcShmAddr + uiShmSize - LOG_ShmReserveMemery;
    pstShmHeader->pShmWriteOffset_Server    = pstShmHeader->pShmStartOffset_Server;
    pstShmHeader->pShmReadOffset_Server     = pstShmHeader->pShmStartOffset_Server;

    pstShmListNode->pstShmHeader    = pstShmHeader;
    pstShmListNode->pstNext         = g_pstLogServerContext->stShmListHeader.pstHeader;
    g_pstLogServerContext->stShmListHeader.uiNum++;
    g_pstLogServerContext->stShmListHeader.pstHeader = pstShmListNode;
    
    if (pthread_create(&tid, NULL, LOG_ReadThread, (VOID *)pcShmAddr) != 0)
    {
        LOG_CloseShm(pcShmAddr, uiShmSize);
        free(pstShmListNode);
        pstShmListNode = NULL;
        LOG_RawSysLog("pthread create error\n");
        return NULL;
    }

    return pcShmAddr;
}


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
                LOG_RawSysLog("recv = %s\n", pcRcvBuf);
                if (0 < lRecvLen)
                {
                    if (NULL != LOG_EpollCallback(pcRcvBuf))
                    {
                        LOG_RawSysLog("LOG_EpollCallback success\n");
                        sendto(events[iIndex].data.fd, pcRcvBuf, lRecvLen, 0, 
                                (struct sockaddr *)&ClientAddr, iAddrLen); /* success echo */
                    }
                    else
                    {
                        LOG_RawSysLog("LOG_EpollCallback error\n");
                        sendto(events[iIndex].data.fd, "Error!", strlen("Error!") + 1, 
                                0, (struct sockaddr *)&ClientAddr, iAddrLen); /* error */
                    }
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
}


STATIC VOID LOG_STOP(INT signo) 
{
    CHAR szFilePath[LOG_EpollRcvBufSize];
    LOGShmList_S *pstListNode = g_pstLogServerContext->stShmListHeader.pstHeader;

    for (; pstListNode != NULL; pstListNode = pstListNode->pstNext)
    {
        LOG_RawSysLog("/dev/shm/%s addr:%p size:%u closed\n", 
                        pstListNode->pstShmHeader->szFileName, 
                        pstListNode->pstShmHeader->pShmAddr_Server,
                        pstListNode->pstShmHeader->uiShmSize);

        szFilePath[0] = "\0";
        snprintf(szFilePath, sizeof(szFilePath), "rm -f /dev/shm/%s", pstListNode->pstShmHeader->szFileName);
        LOG_CloseShm(pstListNode->pstShmHeader->pShmAddr_Server, pstListNode->pstShmHeader->uiShmSize);
        LOG_System_s(szFilePath);
    }

    LOG_RawSysLog("Signal Ctrl + C, LogServer exit...\n");
    _exit(0);
}


INT main(IN INT argc, IN CHAR *argv[])
{
    LOG_Version();
    if (0 != LOG_InitContext(argc, argv))
    {
        printf("LOG_InitContext error!\n");
        return -1;
    }

#ifdef LOG_Test
    LOG_System_s("uname -a");
    for (;;)
    {
        sleep(1);
    }
#endif

    signal(SIGINT, LOG_STOP); 
    LOG_CreateEpollEvent();

    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

