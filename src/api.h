#ifndef API_H
#define API_H

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



#ifdef __cplusplus
}
#endif

#endif //API_H

