#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define LOG_LocalSysLogBufSize      ( 512UL )

CHAR szLogAddrIP[16] = "127.0.0.1";   //Log Server IP Address
USHORT usLogAddrPort = 32001;         //Log Server IP Port

INT LOG_LocalSyslog(FILE *stream, CHAR *fmt, ...)
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
                          stLocalTime->tm_year + 1900, stLocalTime->tm_mon + 1, stLocalTime->tm_mday,
                          stLocalTime->tm_hour, stLocalTime->tm_min, stLocalTime->tm_sec, __FUNCTION__, __LINE__);
    ulStrLen += scnprintf(szLogStr + ulStrLen, sizeof(szLogStr) - ulStrLen, fmt, ap);


    fwrite(szLogStr, sizeof(szLogStr), 1, stream);


    va_end(ap);
}

INT LOG_CheckAddrIPv4(IN CHAR *pcAddrIP)
{
    UINT uiAddr1 = 0;
    UINT uiAddr2 = 0;
    UINT uiAddr3 = 0;
    UINT uiAddr4 = 0;

    if (NULL == pcAddrIP)
    {
        return -1;
    }

    if (4 == sscanf(pcAddrIP, "%u.%u.%u.%u", &uiAddr1, &uiAddr2, &uiAddr3, &uiAddr4))
    {
        if (255 >= uiAddr1 || 255 >= uiAddr2 || 255 >= uiAddr3 || 255 >= uiAddr4)
        {
            return 0;
        }
    }

    return -1;
}


INT LOG_CheckAddrPort(IN CHAR *pcAddrPort)
{
    UINT uiAddrPort = 0;

    if (NULL == pcAddrPort)
    {
        return -1;
    }

    uiAddrPort = atoi(pcAddrPort);
    if (65535 >= uiAddrPort)
    {
        return 0;
    }

	return -1;
}

VOID LOG_CmdUsage(VOID)
{
    fprintf(stderr, "Usage: (1) ./logserver \n");
    fprintf(stderr, "       (2) ./logserver -h/-help \n");
    fprintf(stderr, "       (3) ./logserver -s/-set Address=127.0.0.1:32001 \n");
    fprintf(stderr, "       (4) ./logserver -s/-set CPU=0 \n");
    exit(1);
}

VOID LOG_CmdSet(IN CHAR *pcAddrStr)
{
    CHAR *pcAddrIP   = NULL;
    CHAR *pcAddrPort = NULL;

    if (NULL == pcAddrStr)
    {
        return;
    }

    pcAddrIP   = pcAddrStr;
    pcAddrPort = strchr(pcAddrStr, ':');
    if (NULL == pcAddrPort)
    {
        return;
    }

    if (0 == LOG_CheckAddrIPv4(pcAddrIP) &&
        0 == LOG_CheckAddrPort(pcAddrPort + 1))
    {
        sscanf(pcAddrStr, "%[^:]:%hu", szLogAddrIP, &usLogAddrPort);
    }

    return;
}


#ifdef __cplusplus
}
#endif

