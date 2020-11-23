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

#include "api.h"


/* 测试LogServer读取日志 */
INT main(IN INT argc, IN CHAR *argv[])
{
    UINT uiCount = 0;
    UINT uiIndex = 0;

    for (uiIndex = 0; uiIndex < 5; uiIndex++)
    {
        if (LOG_CreateClient(getpid(), 65536) == NULL)
            return -1;
        if (0 < fork()) break;
    }

    if (uiIndex == 5) return 0;

    while (1)
    {
        uiCount++;
        LOG_ErrorLog("(%u) test pid = %u\n", uiCount, getpid());
        LOG_WarnLog("(%u) test pid = %u\n", uiCount, getpid());
        LOG_InfoLog("(%u) test pid = %u\n", uiCount, getpid());
        LOG_DebugLog("(%u) test pid = %u\n", uiCount, getpid());
        LOG_TraceLog("(%u) test pid = %u\n", uiCount, getpid());
        usleep(100000);
    }

    return 0;

}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


