#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#include "api.h"


typedef struct LOG_LocalSyslog {
    INT fd;                                 /* File descriptor */
    pthread_mutex_t mutex;                  /* Mutex */
} LOGLocalSyslog_S;

typedef struct LOG_ShmList {
    LOGShmHeader_S stShmHeader;             /* File descriptor */
    struct LOG_ShmList *pstNext;                  /* Mutex */
} LOGShmList_S;

typedef struct LOG_ShmListHeader {
    UINT uiNum;             /* File descriptor */
    LOGShmList_S *pstNext;                  /* Mutex */
} LOGShmListHeader_S;

typedef struct LOG_ServerContext {
    /* General */
    pid_t pid;                              /* Main process pid. */
    USHORT usTargeCPU;                      /* CPU affinity */
    BOOL bIsCPUAffinity;                    /* CPU affinity flag */
    CHAR szFilePath[128];                   /* Log Path */
    CHAR szLogAddrIP[16];                   /* Log Server IP Address */
    USHORT usLogAddrPort;                   /* Log Server IP Port */
    BOOL bIsLocalSyslog;                    /* Local Syslog flag */
    LOGLocalSyslog_S stLOGLocalSyslog;      /* Local Syslog */
} LOGServerContext_S;

extern LOGServerContext_S *g_pstLogServerContext;


extern INT LOG_LocalSyslog(IN LOGLocalSyslog_S *pstLocalSyslog, IN CHAR *pcFunc, IN INT uiLine, IN CHAR *fmt, ...);
extern VOID LOG_ParsePara(IN INT argc, IN CHAR *argv[]);


#define LOG_RawSysLog(fmt, ...) \
    do \
    { \
        LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, __func__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif //LOG_H

