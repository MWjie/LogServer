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

extern LOGServerContext_S *g_pstLogServerContext;


INT LOG_CheckAddrIPv4(IN CHAR *pcAddrIP)
{
    UINT uiAddr1 = 0;
    UINT uiAddr2 = 0;
    UINT uiAddr3 = 0;
    UINT uiAddr4 = 0;

    if (NULL == pcAddrIP)
    {
        LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Check IP error!\n");
        return -1;
    }

    if (4 == sscanf(pcAddrIP, "%u.%u.%u.%u", &uiAddr1, &uiAddr2, &uiAddr3, &uiAddr4))
    {
        if (255 >= uiAddr1 || 255 >= uiAddr2 || 255 >= uiAddr3 || 255 >= uiAddr4)
        {
            LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Check IP %s success\n", pcAddrIP);
            return 0;
        }
    }

    LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Check IP error!\n");
    return -1;
}


INT LOG_CheckAddrPort(IN CHAR *pcAddrPort)
{
    UINT uiAddrPort = 0;

    if (NULL == pcAddrPort)
    {
        LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Check port error!\n");
        return -1;
    }

    uiAddrPort = atoi(pcAddrPort);
    if (65535 >= uiAddrPort)
    {
        LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Check port %s success\n", pcAddrPort);
        return 0;
    }

    LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, "Check port error!\n");
	return -1;
}



#ifdef __cplusplus
}
#endif

