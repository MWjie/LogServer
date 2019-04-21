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


CHAR szLogAddrIP[16] = "127.0.0.1";   //Log Server IP Address
USHORT usLogAddrPort = 32001;         //Log Server IP Port


VOID LOG_CmdUsage(VOID)
{
    fprintf(stderr, "Usage: (1) ./logserver \n");
    fprintf(stderr, "       (2) ./logserver -h/-help \n");
    fprintf(stderr, "       (3) ./logserver -s/-set 127.0.0.1:32001 \n");
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
        sscanf(pcAddrStr, "%[^:]:%u", szLogAddrIP, usLogAddrPort);
    }

    return;
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

    if (4 == sscanf(pcAddrIP, "%u.%u.%u.%u", uiAddr1, uiAddr2, uiAddr3, uiAddr4))
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

#ifdef __cplusplus
}
#endif

