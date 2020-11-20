#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#include "api.h"

extern INT LOG_System_s(IN CHAR *pcCmd);
extern CHAR *LOG_OpenShm(IN CHAR *pcShmName, IN UINT uiShmSize);
extern INT LOG_CloseShm(IN CHAR *pcShmAddr, IN UINT uiShmSize);
extern INT LOG_CheckAddrIPv4(IN CHAR *pcAddrIP);
extern INT LOG_CheckAddrPort(IN CHAR *pcAddrPort);
extern CHAR *LOG_Strtok_r(IN CHAR *s, IN const CHAR *delim, OUT CHAR **save_ptr);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif //UTIL_H

