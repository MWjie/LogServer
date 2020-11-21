#ifndef API_H
#define API_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


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

typedef struct LOG_LevelStr {
    LOGLEVEL_E enLogLevel;
    CHAR *pcLevelStr;
} LOGLEVELSTR_S;


extern CHAR *LOG_WriteLog(IN LOGLEVEL_E enLogLevel, IN CHAR *pcFunc, IN INT uiLine, IN CHAR *fmt, ...);

#define LOG_ErrorLog(fmt, ...) \
    do \
    { \
        LOG_WriteLog(LOG_ERROR, __func__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)

#define LOG_WarnLog(fmt, ...) \
    do \
    { \
        LOG_WriteLog(LOG_WARN, __func__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)

#define LOG_InfoLog(fmt, ...) \
    do \
    { \
        LOG_WriteLog(LOG_INFO, __func__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)

#define LOG_DebugLog(fmt, ...) \
    do \
    { \
        LOG_WriteLog(LOG_DEBUG, __func__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)

#define LOG_TraceLog(fmt, ...) \
    do \
    { \
        LOG_WriteLog(LOG_TRACE, __func__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (0)




#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif //API_H

