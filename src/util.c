#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

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
#define __USE_POSIX
#include <signal.h>
#include <pthread.h>
#include <sys/mman.h>

#include "util.h"

CHAR *LOG_Strtok_r(IN CHAR *s, IN const CHAR *delim, OUT CHAR **save_ptr) 
{     
    CHAR *token;     
     
    if (s == NULL) s = *save_ptr;     
     
    /* Scan leading delimiters.  */     
    s += strspn(s, delim);
    if (*s == '/0')      
        return NULL;     
     
    /* Find the end of the token.  */     
    token = s;     
    s = strpbrk(token, delim);
    if (s == NULL)     
        /* This token finishes the string.  */     
        *save_ptr = strchr(token, '/0');     
    else {     
        /* Terminate the token and make *SAVE_PTR point past it.  */     
        *s = '/0';     
        *save_ptr = s + 1;     
    }     
     
    return token;     
}  

INT LOG_System_s(IN CHAR *pcCmd)
{
    INT iPid     = 0;
    INT iStatus  = -1;
    INT iWaitPid = 0;
    struct sigaction stIgnore, stSaveIntr, stSaveQuit;
    sigset_t stChildMask, stSaveMask;

    if (NULL == pcCmd)
    {
        return -1;
    }

    stIgnore.sa_handler = SIG_IGN;
    stIgnore.sa_flags   = 0;
    sigemptyset(&stIgnore.sa_mask);
    (VOID)sigaction(SIGINT,  &stIgnore, &stSaveIntr);
    (VOID)sigaction(SIGQUIT, &stIgnore, &stSaveQuit);
    sigemptyset(&stChildMask);
    sigaddset(&stChildMask, SIGCHLD);
    (VOID)sigprocmask(SIG_BLOCK, &stChildMask, &stSaveMask);

    iPid = vfork();
    if (0 > iPid)
    {
        iStatus = -1;
    }
    else if (0 == iPid)
    {
        (VOID)sigaction(SIGINT,  &stSaveIntr, NULL);
        (VOID)sigaction(SIGQUIT, &stSaveQuit, NULL);
        (VOID)sigprocmask(SIG_SETMASK, &stSaveMask, (sigset_t *)NULL);
        (VOID)execl("/bin/sh", "sh", "-c", pcCmd, (CHAR *)NULL);
        _exit(127);
    }
    else
    {
        iWaitPid = waitpid(iPid, (INT *)&iStatus, 0);
        if (0 > iWaitPid)
        {
            iStatus = -1;
        }
    }
    (VOID)sigaction(SIGINT,  &stSaveIntr, NULL);
    (VOID)sigaction(SIGQUIT, &stSaveQuit, NULL);
    (VOID)sigprocmask(SIG_SETMASK, &stSaveMask, (sigset_t *)NULL);

    return 0;
}


CHAR *LOG_OpenShm(IN CHAR *pcShmName, IN UINT uiShmSize)
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

    ftruncate(fd, uiShmSize);

    pcShmAddr = (CHAR *)mmap(NULL, uiShmSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(NULL == pcShmAddr)
    {
        close(fd);
        return NULL;
    }

    close(fd);
    return pcShmAddr;

}

INT LOG_CloseShm(IN CHAR *pcShmAddr, IN UINT uiShmSize)
{
    if(NULL == pcShmAddr || 0 == uiShmSize)
    {
        return -1;
    }

    if (-1 == munmap(pcShmAddr, uiShmSize))
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
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


