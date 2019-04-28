#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#include "log.h"
#include "util.h"


#define LOG_LocalSysLogBufSize      ( 512UL )


CHAR g_szLogAddrIP[16] = "127.0.0.1";       /* Log Server IP Address */
USHORT g_usLogAddrPort = 32001;             /* Log Server IP Port */
extern LOGServerContext_S *g_pstLogServerContext;



INT LOG_LocalSyslog(IN LOGLocalSyslog_S *pstLocalSyslog, IN CHAR *fmt, ...)
{
    va_list ap;
    struct timeval stTimeVal;
    struct tm stLocalTime;

    ULONG ulStrLen = 0;
    CHAR szLogStr[LOG_LocalSysLogBufSize];

    va_start(ap, fmt);
    gettimeofday(&stTimeVal, NULL);
    localtime_r(&stTimeVal->tv_sec, &stLocalTime);

    ulStrLen += scnprintf(szLogStr, sizeof(szLogStr), "%4d-%02d-%02d %2d:%2d:2d %s[%d]: ",
                          stLocalTime.tm_year + 1900, stLocalTime.tm_mon + 1, stLocalTime.tm_mday,
                          stLocalTime.tm_hour, stLocalTime.tm_min, stLocalTime.tm_sec, __FUNCTION__, __LINE__);
    ulStrLen += scnprintf(szLogStr + ulStrLen, sizeof(szLogStr) - ulStrLen, fmt, ap);

    pthread_mutex_tlock(&pstLocalSyslog->mutex);
    write(pstLocalSyslog->fd, szLogStr, ulStrLen);
    pthread_mutex_unlock(&pstLocalSyslog->mutex);

    va_end(ap);
    return 0;
}


STATIC VOID LOG_CmdSetAddr(IN CHAR *pcArgvContent)
{
    CHAR *pcAddrIP   = NULL;
    CHAR *pcAddrPort = NULL;

    pcAddrIP   = pcArgvContent;
    pcAddrPort = strchr(pcArgvContent, ':');
    if (NULL == pcAddrPort)
    {
        LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Set Address error!\n");
        return;
    }

    if (0 == LOG_CheckAddrIPv4(pcAddrIP) &&
        0 == LOG_CheckAddrPort(pcAddrPort + 1))
    {
        sscanf(pcArgvContent, "%[^:]:%hu", g_szLogAddrIP, &g_usLogAddrPort);
        g_pstLogServerContext->usLogAddrPort = g_usLogAddrPort;
        strlcpy(g_pstLogServerContext->szLogAddrIP, g_szLogAddrIP, sizeof(g_szLogAddrIP);
        LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Set Address %s:%us\n", g_szLogAddrIP, g_usLogAddrPort);
    }

    return;
}

STATIC VOID LOG_CmdSetCPU(IN CHAR *pcArgvContent)
{
    UINT uiTargeCPU;
    cpu_set_t mask;

    uiTargeCPU = (UINT)atoi(pcArgvContent);
    g_pstLogServerContext->uiTargeCPU = uiTargeCPU;

    CPU_ZERO(&mask);
    CPU_SET(uiTargeCPU, &mask);
    if (-1 == sched_setaffinity(0, sizeof(cpu_set_t), &mask))
    {
        LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Set CPU affinity error!\n");
    }
    LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Set CPU%u affinity\n", uiTargeCPU);

    return;
}

STATIC VOID LOG_CmdSetPATH(IN CHAR *pcArgvContent)
{
    strlcpy(g_pstLogServerContext->szFilePath, pcArgvContent, strlen(pcArgvContent);
    LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Set FilePath %s\n", pcArgvContent);

    return;
}


STATIC VOID LOG_CmdSet(IN INT argc, IN CHAR *argv[])
{
    UINT uiIndex;
    CHAR szArgvHeader[64];
    CHAR szArgvContent[64];

    for (uiIndex = 0; uiIndex < (UINT)argc; uiIndex++)
    {
        szArgvHeader[0]  = '\0';
        szArgvContent[0] = '\0';
        sscanf(argv[uiIndex], "%[^=]=%s", szArgvHeader, szArgvContent);

        if (0 == strcmp(szArgvHeader, "Address"))
        {
            LOG_CmdSetAddr(szArgvContent);
        }
        else if (0 == strcmp(szArgvHeader, "CPU"))
        {
            LOG_CmdSetCPU(szArgvContent);
        }
        else if (0 == strcmp(szArgvHeader, "PATH"))
        {
            LOG_CmdSetPATH(szArgvContent);
        }
    }

    return;
}


STATIC VOID LOG_CmdUsage(VOID)
{
    fprintf(stderr, "------------------------------------------------------------ \n");
    fprintf(stderr, "------------------------------------------------------------ \n");
    fprintf(stderr, "Usage: (1) ./logserver \n");
    fprintf(stderr, "       (2) ./logserver -h/-help \n");
    fprintf(stderr, "       (3) ./logserver -s/-set Address=127.0.0.1:32001 \n");
    fprintf(stderr, "           ./logserver -s/-set CPU=0 \n");
    fprintf(stderr, "           ./logserver -s/-set PATH=./log \n");
    fprintf(stderr, "           ./logserver -s/-set Address=127.0.0.1:32001 CPU=0 \n");
    fprintf(stderr, "------------------------------------------------------------ \n");
    exit(1);
}


VOID LOG_ParsePara(IN INT argc, IN CHAR *argv[])
{
	if (3 <= argc && (0 == strcasecmp(argv[1], "-s") || 0 == strcasecmp(argv[1], "-set")))
    {
        LOG_CmdSet(argc - 2, &argv[2]);
    }
    else
    {
        LOG_CmdUsage();
    }
}


#ifdef __cplusplus
}
#endif

