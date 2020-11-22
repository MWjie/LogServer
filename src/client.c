#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <sys/time.h>
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


#define LOG_Test

#define LOG_DefaultAddrIP           ( "127.0.0.1" )
#define LOG_DefaultAddrPort         ( 32001U )
#define LOG_MsgBufSize              ( 256U )
#define LOG_LocalSysLogBufSize      ( 512UL )
#define LOG_ShmReserveMemery        ( 512U )

volatile LOGShmHeader_S *g_pstShmHeader = NULL;
volatile CHAR g_szLogStr[LOG_LocalSysLogBufSize];

STATIC LOGLEVELSTR_S g_stLogLevel[] = {
    {LOG_ERROR,     "[ERROR]"},
    {LOG_WARN,      "[WARN] "},
    {LOG_INFO,      "[INFO] "},
    {LOG_DEBUG,     "[DEBUG]"},
    {LOG_TRACE,     "[TRACE]"}
};


INT LOG_InitClientFd(VOID)
{
    INT socketfd    = -1;
    INT iErron      = 0;
    STATIC USHORT usClientPort = LOG_DefaultAddrPort;
    struct sockaddr_in clientaddr;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == socketfd)
    {
        iErron = errno;
        fprintf(stderr, "socket errno = %d\n", iErron);
        return -1;
    }
 
    bzero(&clientaddr, sizeof(struct sockaddr_in));
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(++usClientPort);
    clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (-1 == bind(socketfd, (struct sockaddr *)&clientaddr, sizeof(struct sockaddr)))
    {
        iErron = errno;
        fprintf(stderr, "bind errno = %d\n", iErron);
        close(socketfd);
        return -1;
    }

    return socketfd;
}

INT LOG_CreateClient(VOID)
{
    INT socketfd        = -1;
    INT iErron          = 0;
    INT iMsgLen         = 0;
    CHAR szSendBuf[LOG_MsgBufSize] = "";
    CHAR szRecvBuf[LOG_MsgBufSize] = "";
    socklen_t iAddrLen  = sizeof(struct sockaddr);
    struct sockaddr_in serveraddr;

    socketfd = LOG_InitClientFd();
    if (-1 == socketfd)
    {
        return -1;
    }

    bzero(&serveraddr, sizeof(struct sockaddr_in));
    serveraddr.sin_family       = AF_INET;
    serveraddr.sin_port         = htons(LOG_DefaultAddrPort);
    serveraddr.sin_addr.s_addr  = inet_addr(LOG_DefaultAddrIP);

    /* Msg ShmFilename:pid:ShmSize */
    iMsgLen = snprintf(szSendBuf, LOG_MsgBufSize, "%d:%d:65536", getpid(), getpid());
    sendto(socketfd, szSendBuf, iMsgLen, 0, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr));
    fprintf(stdout, "send = %s\n", szSendBuf);

    iMsgLen = recvfrom(socketfd, szRecvBuf, LOG_MsgBufSize, 0, (struct sockaddr *)&serveraddr, &iAddrLen);
    if (0 != strncmp(szSendBuf, szRecvBuf, strlen(szSendBuf)))
    {
        fprintf(stdout, "send = %s : recv = %s\n", szSendBuf, szRecvBuf);
    }

    close(socketfd);
    return 0;
}

LOGShmHeader_S *LOG_GetShmAddr(pid_t pid, UINT uiShmSize)
{
    LOGShmHeader_S *pstShmHeader = NULL;
    CHAR szShmName[32] = "";
    CHAR *pcShmAddr = NULL;

    snprintf(szShmName, sizeof(szShmName), "%d", pid);
    pcShmAddr = LOG_OpenShm(szShmName, uiShmSize);
    if (NULL == pcShmAddr)
    {
        return NULL;
    }

    pstShmHeader = (LOGShmHeader_S *)pcShmAddr;
    pstShmHeader->pShmAddr_Client           = pcShmAddr;
    pstShmHeader->pShmStartOffset_Client    = pcShmAddr + sizeof(LOGShmHeader_S);
    pstShmHeader->pShmEndOffset_Client      = pcShmAddr + uiShmSize - LOG_ShmReserveMemery;
    pstShmHeader->pShmWriteOffset_Client    = pstShmHeader->pShmStartOffset_Client;
    pstShmHeader->pShmReadOffset_Client     = pstShmHeader->pShmStartOffset_Client;

    return pstShmHeader;
}


CHAR *LOG_WriteLog(IN LOGLEVEL_E enLogLevel, IN CHAR *pcFunc, IN INT uiLine, IN CHAR *fmt, ...)
{
    va_list ap;
    struct timeval stTimeVal;
    struct tm stLocalTime;
    ULONG ulStrLen    = 0;
    ULONG ulStrLenRes = 0;

    va_start(ap, fmt);
    gettimeofday(&stTimeVal, NULL);
    localtime_r(&stTimeVal.tv_sec, &stLocalTime);

    ulStrLen += snprintf(g_szLogStr, sizeof(g_szLogStr), "%s ", g_stLogLevel[enLogLevel].pcLevelStr);
    ulStrLen += strftime(g_szLogStr + ulStrLen, sizeof(g_szLogStr) - ulStrLen, "%d %b %Y %H:%M:%S.", &stLocalTime);
    ulStrLen += snprintf(g_szLogStr + ulStrLen, sizeof(g_szLogStr) - ulStrLen, "%03d %s[%d]: ",
                         stTimeVal.tv_usec / 1000, pcFunc, uiLine);
    ulStrLen += vsnprintf(g_szLogStr + ulStrLen, sizeof(g_szLogStr) - ulStrLen, fmt, ap);

    memcpy(g_pstShmHeader->pShmWriteOffset_Client, g_szLogStr, ulStrLen);
//    strncpy(g_pstShmHeader->pShmWriteOffset_Client, g_szLogStr, sizeof(g_szLogStr));
    if ((g_pstShmHeader->pShmWriteOffset_Client + ulStrLen) >= g_pstShmHeader->pShmEndOffset_Client)
    {
        /* 如果最后部分超出保留地址范围，将多余部分写入头部地址 */
        ulStrLenRes = (g_pstShmHeader->pShmWriteOffset_Client + ulStrLen - g_pstShmHeader->pShmEndOffset_Client);
        memcpy(g_pstShmHeader->pShmStartOffset_Client, g_szLogStr + ulStrLen - ulStrLenRes, ulStrLenRes);
        g_pstShmHeader->pShmWriteOffset_Client = g_pstShmHeader->pShmStartOffset_Client + ulStrLenRes;
        g_pstShmHeader->pShmWriteOffset_Server = g_pstShmHeader->pShmStartOffset_Server + ulStrLenRes;
    }
    else
    {
        g_pstShmHeader->pShmWriteOffset_Client += ulStrLen;
        g_pstShmHeader->pShmWriteOffset_Server += ulStrLen;
    }
  
    va_end(ap);
    return 0;
}


INT main(IN INT argc, IN CHAR *argv[])
{
    UINT uiCount = 0;
    UINT uiIndex = 0;
#ifdef LOG_Test
    for (uiIndex = 0; uiIndex < 5; uiIndex++)
    {
        LOG_CreateClient();
        if ((g_pstShmHeader = LOG_GetShmAddr(getpid(), 65536)) == NULL)
            return -1;
        LOG_ErrorLog("hello\n");
        if (0 < fork()) break;
    }

    if (uiIndex == 5) return 0;

    while (1)
    {
        uiCount++;
        LOG_ErrorLog("(%u) test pid = %u %p:%s\n", uiCount, getpid(), (CHAR *)g_pstShmHeader, (CHAR *)g_pstShmHeader);
        LOG_WarnLog("(%u) test pid = %u %p:%s\n", uiCount, getpid(), (CHAR *)g_pstShmHeader, (CHAR *)g_pstShmHeader);
        LOG_InfoLog("(%u) test pid = %u %p:%s\n", uiCount, getpid(), (CHAR *)g_pstShmHeader, (CHAR *)g_pstShmHeader);
        LOG_DebugLog("(%u) test pid = %u %p:%s\n", uiCount, getpid(), (CHAR *)g_pstShmHeader, (CHAR *)g_pstShmHeader);
        LOG_TraceLog("(%u) test pid = %u %p:%s\n", uiCount, getpid(), (CHAR *)g_pstShmHeader, (CHAR *)g_pstShmHeader);
        usleep(100000);
    }

#endif
    


    return 0;

}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


