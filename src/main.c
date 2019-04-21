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

#include "log.h"


STATIC INT LOG_ParsePara(IN INT argc, IN const CHAR *argv[])
{
    if (0 == strcasecmp(argv[1], "?")  ||
        0 == strcasecmp(argv[1], "-h") ||
        0 == strcasecmp(argv[1], "-help"))
    {
        return LOG_CmdUsage();
    }
    else if (0 == strcasecmp(argv[1], "-s") ||
             0 == strcasecmp(argv[1], "-set"))
    {
        return LOG_CmdSet(argv[2]);
    }
    else
    {
        return LOG_CmdUsage();
    }
}


INT main(IN INT argc, IN const CHAR *argv[])
{
    if (2 <= argc)
    {
        (VOID)LOG_ParsePara(argc, argv);
    }

    srand(time(NULL) ^ getpid());

    return 0;
}


#ifdef __cplusplus
}
#endif


