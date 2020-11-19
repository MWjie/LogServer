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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#define __USE_GNU
#include <sched.h>
#include <pthread.h>

#include "log.h"
#include "util.h"

#define LOG_LocalSysLogBufSize      ( 512UL )


INT LOG_LocalSyslog(IN LOGLocalSyslog_S *pstLocalSyslog, IN CHAR *pcFunc, IN INT uiLine, IN CHAR *fmt, ...)
{
    va_list ap;
    struct timeval stTimeVal;
    struct tm stLocalTime;

    ULONG ulStrLen = 0;
    CHAR szLogStr[LOG_LocalSysLogBufSize];

    va_start(ap, fmt);
    gettimeofday(&stTimeVal, NULL);
    localtime_r(&stTimeVal.tv_sec, &stLocalTime);

    ulStrLen += strftime(szLogStr, sizeof(szLogStr), "%d %b %Y %H:%M:%S.", &stLocalTime);
    ulStrLen += snprintf(szLogStr + ulStrLen, sizeof(szLogStr) - ulStrLen, "%03d %s[%d]: ",
                         stTimeVal.tv_usec / 1000, pcFunc, uiLine);
    ulStrLen += vsnprintf(szLogStr + ulStrLen, sizeof(szLogStr) - ulStrLen, fmt, ap);

    if (1 == g_pstLogServerContext->bIsLocalSyslog)
    {
        pthread_mutex_lock(&pstLocalSyslog->mutex);
        write(pstLocalSyslog->fd, szLogStr, ulStrLen);
        pthread_mutex_unlock(&pstLocalSyslog->mutex);
    }
    else
    {
        fprintf(stdout, "%s", szLogStr);
    }

    va_end(ap);
    return 0;
}

VOID *LOG_WirteThread(VOID *arg)
{
    LOG_RawSysLog("Thread Create success\n");
    return (VOID *)0;
}


STATIC VOID LOG_CmdSetAddr(IN CHAR *pcArgvContent)
{
    CHAR *pcAddrIP   = NULL;
    CHAR *pcAddrPort = NULL;

    pcAddrIP   = pcArgvContent;
    pcAddrPort = strchr(pcArgvContent, ':');
    if (NULL == pcAddrPort)
    {
        LOG_RawSysLog("Set Address error!\n");
        return;
    }

    if (0 == LOG_CheckAddrIPv4(pcAddrIP) &&
        0 == LOG_CheckAddrPort(pcAddrPort + 1))
    {
        sscanf(pcArgvContent, "%[^:]:%hu", g_pstLogServerContext->szLogAddrIP, &g_pstLogServerContext->usLogAddrPort);
        LOG_RawSysLog("Set Address %s:%u\n", g_pstLogServerContext->szLogAddrIP, g_pstLogServerContext->usLogAddrPort);
    }

    return;
}

STATIC VOID LOG_CmdSetCPU(IN CHAR *pcArgvContent)
{
    UINT usTargeCPU;
    cpu_set_t mask;

    usTargeCPU = (USHORT)atoi(pcArgvContent);
    g_pstLogServerContext->usTargeCPU     = usTargeCPU;
    g_pstLogServerContext->bIsCPUAffinity = 1;

    CPU_ZERO(&mask);
    CPU_SET(usTargeCPU, &mask);
    if (-1 == sched_setaffinity(0, sizeof(cpu_set_t), &mask))
    {
        LOG_RawSysLog("Set CPU affinity error!\n");
        return;
    }
    LOG_RawSysLog("Set CPU%u affinity\n", usTargeCPU);

    return;
}

STATIC VOID LOG_CmdSetPATH(IN CHAR *pcArgvContent)
{
    strncpy(g_pstLogServerContext->szFilePath, pcArgvContent, strlen(pcArgvContent));
    LOG_RawSysLog("Set FilePath %s\n", pcArgvContent);

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


STATIC VOID LOG_Daemonize(VOID)
{
    INT fd;

    if (0 != fork()) 
    {
        exit(0); /* parent exits */
    }
    setsid(); /* create a new session */

    /* Every output goes to /dev/null. If Redis is daemonized but
     * the 'logfile' is set to 'stdout' in the configuration file
     * it will not log at all. */
    if (-1 != (fd = open("/dev/null", O_RDWR, 0)))
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO)
        {
            close(fd);
        }
    }
}


STATIC VOID LOG_CmdUsage(VOID)
{
    fprintf(stderr, "------------------------------------------------------------ \n");
    fprintf(stderr, "------------------------------------------------------------ \n");
    fprintf(stderr, "Usage: (1) ./logserver \n");
    fprintf(stderr, "       (2) ./logserver -h/-help \n");
    fprintf(stderr, "       (3) ./logserver -s/-set Address=127.0.0.1:32001 \n");
    fprintf(stderr, "           ./logserver -s/-set CPU=0 \n");
    fprintf(stderr, "           ./logserver -s/-set PATH=./Log/ \n");
    fprintf(stderr, "           ./logserver -s/-set Address=127.0.0.1:32001 CPU=0 \n");
    fprintf(stderr, "------------------------------------------------------------ \n\n");
    exit(1);
}


VOID LOG_ParsePara(IN INT argc, IN CHAR *argv[])
{
    if (2 <= argc)
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
}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

