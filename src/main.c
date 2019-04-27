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


STATIC VOID LOG_ParsePara(IN CHAR *argv[])
{
	if (0 == strcasecmp(argv[1], "-s") ||
        0 == strcasecmp(argv[1], "-set"))
    {
        LOG_CmdSet(argv[2]);
    }
    else
    {
        LOG_CmdUsage();
    }
}


INT main(IN INT argc, IN CHAR *argv[])
{
    if (2 <= argc)
    {
        LOG_ParsePara(argv);
    }

    srand(time(NULL) ^ getpid());

    return 0;
}


#ifdef __cplusplus
}
#endif


