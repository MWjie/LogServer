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
#include <dirent.h>
#include <time.h>
#include <signal.h>
#define __USE_GNU
#include <sched.h>
#include <pthread.h>

#include "log.h"

#define LOG_LocalSysLogPath         ( "./LocalSysLog" )
#define LOG_DefualtLogPath          ( "./Log/" )

LOGServerContext_S *g_pstLogServerContext = NULL;


STATIC INT LOG_InitContext(IN INT argc, IN CHAR *argv[])
{
    CHAR szShellBuf[128];
    CHAR szFileName[128];
    CHAR szOldFile[128];
    cpu_set_t mask;
    DIR *dir;
    struct dirent *ptr;

    g_pstLogServerContext = (LOGServerContext_S *)malloc(sizeof(LOGServerContext_S));
    if (NULL == g_pstLogServerContext)
    {
        return -1;
    }
    memset(g_pstLogServerContext, 0x0, sizeof(LOGServerContext_S));

    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    g_pstLogServerContext->uiTargeCPU = 0;

    srand(time(NULL) ^ getpid());
    g_pstLogServerContext->pid = getpid();

    pthread_mutex_init(&g_pstLogServerContext->stLOGLocalSyslog.mutex, NULL);
    g_pstLogServerContext->stLOGLocalSyslog.fd = open(LOG_LocalSysLogPath, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (0 > g_pstLogServerContext->stLOGLocalSyslog.fd)
    {
        LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, __func__, __LINE__, "Open %s error!\n", LOG_LocalSysLogPath);
        return -1;
    }

    strncpy(g_pstLogServerContext->szFilePath, LOG_DefualtLogPath, strlen(LOG_DefualtLogPath));
    LOG_ParsePara(argc, argv);

    snprintf(szShellBuf, sizeof(szShellBuf), "mkdir -p %s", g_pstLogServerContext->szFilePath);
    system(szShellBuf);

    snprintf(szShellBuf, sizeof(szShellBuf), "find %s -name *.bak -exec rm -rf {} \\;", g_pstLogServerContext->szFilePath);
    system(szShellBuf);

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


INT main(IN INT argc, IN CHAR *argv[])
{

    if (0 != LOG_InitContext(argc, argv))
    {
        printf("LOG_InitContext error!\n");
        return -1;
    }

    return 0;

}


#ifdef __cplusplus
}
#endif


