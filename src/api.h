#ifndef API_H
#define API_H

#ifdef __cplusplus
extern "C" {
#endif


/* type definitions */
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
#define STATIC              static
#define BOOL                USHORT
#define VOID                void

#ifndef FALSE
#define FALSE               0U
#endif

#ifndef TRUE
#define TRUE                1U
#endif

#ifndef NULL
#define NULL                ((VOID *)0)
#endif


typedef enum LOG_Level {
    LOG_ERROR = 0,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TRACE
} LOGLEVEL_E;

typedef struct LOG_ShmHeader {
    CHAR szFileName[64];        /* LOG Shmname and Log file name */
    UINT uiClientPid;           /* Cilent pid */
    UINT uiShmSize;             /* Create Shm Size */
    VOID *pShmStartOffset;      /* Shmaddr + sizeof(LOGShmHeader_S) */
    VOID *pShmEndOffset;        /* Last one log allow addr */
    VOID *pShmWriteOffset;      /* Client proess write pointer */
    VOID *pShmReadOffset;       /* Server proess read pointer */
} LOGShmHeader_S;

#ifdef __cplusplus
}
#endif

#endif //API_H

