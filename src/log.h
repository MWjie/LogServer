#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void                VOID;
typedef char                CHAR;
typedef short               SHORT;
typedef int                 INT;
typedef long                LONG;
typedef long long           INT64;
typedef unsigned char       UCHAR;
typedef unsigned short      USHORT;
typedef unsigned int        UINT;
typedef unsigned long       ULONG;
typedef unsigned long long  UINT64;


#define IN
#define OUT
#define INOUT
#define STATIC      static
#define BOOL        USHORT


typedef enum LOG_Level {
    LOG_ERROR = 0,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TRACE
} LOGLEVEL_E;

typedef struct LOG_LocalSyslog {
    INT fd;                                 /* File descriptor */
    pthread_mutex_t mutex;                  /* Mutex */
} LOGLocalSyslog_S;

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


#define LOG_RawSysLog(fmt, args...) \
    do \
    { \
        if (1 == g_pstLogServerContext->bIsLocalSyslog) \
        { \
            LOG_LocalSyslog(&g_pstLogServerContext->stLOGLocalSyslog, __func__, __LINE__, fmt, ##args); \
        } \
    } while (0)


#ifdef __cplusplus
}
#endif

#endif //LOG_H

