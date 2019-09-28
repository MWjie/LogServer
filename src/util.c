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
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/mman.h>

#include "util.h"


INT LOG_OpenShm(IN CHAR *pcShmName, IN ULONG ulShmSize)
{
    INT fd = 0;
    CHAR szShmPath[128];
    CHAR *pcShmAddr = NULL;

    if (NULL == pcShmName)
    {
        return NULL;
    }

    snprintf(szShmPath, sizeof(szShmPath), "/dev/shm/%s", pcShmName);
    fd = open(szShmPath, O_RDWR | O_CREAT | O_EXCL, 0644);
    if (fd < 0)
    {
        fd = open(szShmPath, O_RDWR | O_CREAT, 0644);
        if (fd < 0)
        {
            return NULL;
        }
    }

    ftruncate(fd, ulShmSize);

    pcShmAddr = (CHAR *)mmap(NULL, ulShmSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(NULL == pcShmAddr)
    {
        close(fd);
        return NULL;
    }

    close(fd);
    return pcShmAddr;

}

INT LOG_CloseShm(IN CHAR *pcShmAddr, IN ULONG ulShmSize)
{
    if(NULL == pcShmAddr || 0 == ulShmSize)
    {
        return -1;
    }

    if (-1 == munmap(pcShmAddr, ulShmSize))
    {
        return -1;
    }

    return 0;
}



INT LOG_CheckAddrIPv4(IN CHAR *pcAddrIP)
{
    UINT uiAddr1 = 0;
    UINT uiAddr2 = 0;
    UINT uiAddr3 = 0;
    UINT uiAddr4 = 0;

    if (NULL == pcAddrIP)
    {
        return -1;
    }

    if (4 == sscanf(pcAddrIP, "%u.%u.%u.%u", &uiAddr1, &uiAddr2, &uiAddr3, &uiAddr4))
    {
        if (255 >= uiAddr1 && 255 >= uiAddr2 && 255 >= uiAddr3 && 255 >= uiAddr4)
        {
            return 0;
        }
    }

    return -1;
}


INT LOG_CheckAddrPort(IN CHAR *pcAddrPort)
{
    UINT uiAddrPort = 0;

    if (NULL == pcAddrPort)
    {
        return -1;
    }

    uiAddrPort = atoi(pcAddrPort);
    if (65535 >= uiAddrPort)
    {
        return 0;
    }

	return -1;
}



#ifdef __cplusplus
}
#endif

