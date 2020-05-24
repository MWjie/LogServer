#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "api.h"

extern INT LOG_System_s(IN CHAR *pcCmd);
extern INT LOG_OpenShm(IN CHAR *pcShmName, IN ULONG ulShmSize);
extern INT LOG_CloseShm(IN CHAR *pcShmAddr, IN ULONG ulShmSize);
extern INT LOG_CheckAddrIPv4(IN CHAR *pcAddrIP);
extern INT LOG_CheckAddrPort(IN CHAR *pcAddrPort);


#ifdef __cplusplus
}
#endif

#endif //UTIL_H

