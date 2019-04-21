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
typedef unsigned long long  UINT64;


#define IN
#define OUT
#define INOUT
#define STATIC  static


extern VOID LOG_CmdUsage(VOID);
extern VOID LOG_CmdSet(IN CHAR *pcAddrStr);
extern VOID LOG_CheckAddrIPv4(VOID);
extern INT LOG_CheckAddrPort(IN CHAR *pcAddrPort);




#ifdef __cplusplus
}
#endif

#endif //LOG_H

