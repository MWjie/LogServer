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
#define STATIC  static


typedef struct LOG_LocalSyslog {
    INT fd;                                 /* File descriptor */
    pthread_mutex_t mutex;                  /* Mutex */
} LOGLocalSyslog_S;

typedef struct LOG_ServerContext {
    /* General */
    pid_t pid;                              /* Main process pid. */
    UINT uiTargeCPU;                        /* CPU affinity */
    CHAR szFilePath[128];                   /* Log Path */
    CHAR szLogAddrIP[16];                   /* Log Server IP Address */
    USHORT usLogAddrPort;                   /* Log Server IP Port */
    LOGLocalSyslog_S stLOGLocalSyslog;      /* Local Syslog */
} LOGServerContext_S;


extern INT LOG_LocalSyslog(IN LOGLocalSyslog_S *pstLocalSyslog, IN CHAR *fmt, ...);
extern VOID LOG_ParsePara(IN INT argc, IN CHAR *argv[]);


#ifdef __cplusplus
}
#endif

#endif //LOG_H

