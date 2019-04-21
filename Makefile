release_hdr := $(shell sh -c 'chmod 777 mkreleasehdr.sh || ./mkreleasehdr.sh')
uname_S 	:= $(shell sh -c 'uname -s 2>/dev/null 		|| echo not')
uname_M 	:= $(shell sh -c 'uname -m 2>/dev/null 		|| echo not')

# Default settings
OPTIMIZATION?=-O2

WARN	= -Wall -W -Wno-missing-field-initializers
OPT		= $(OPTIMIZATION)
CC 		= gcc
DEBUG	= -g -ggdb

STD		= -std=c99 -pedantic -DREDIS_STATIC=''
ifneq (,$(findstring clang,$(CC)))
ifneq (,$(findstring FreeBSD,$(uname_S)))
	STD+=-Wno-c11-extensions
endif
endif

# Default allocator defaults to Jemalloc if it's not an ARM
MALLOC=libc
ifneq ($(uname_M),armv6l)
ifneq ($(uname_M),armv7l)
ifeq ($(uname_S),Linux)
	MALLOC=jemalloc
endif
endif
endif

# To get ARM stack traces if Redis crashes we need a special C flag.
ifneq (,$(filter aarch64 armv,$(uname_M)))
        CFLAGS+=-funwind-tables
else
ifneq (,$(findstring armv,$(uname_M)))
        CFLAGS+=-funwind-tables
endif
endif

# Backwards compatibility for selecting an allocator
ifeq ($(USE_TCMALLOC),yes)
	MALLOC=tcmalloc
endif

ifeq ($(USE_TCMALLOC_MINIMAL),yes)
	MALLOC=tcmalloc_minimal
endif

ifeq ($(USE_JEMALLOC),yes)
	MALLOC=jemalloc
endif

ifeq ($(USE_JEMALLOC),no)
	MALLOC=libc
endif

FINAL_CFLAGS	= $(STD) $(WARN) $(OPT) $(DEBUG) $(CFLAGS)
FINAL_LDFLAGS	= $(LDFLAGS) $(REDIS_LDFLAGS) $(DEBUG)
FINAL_LIBS		= -lm


ifeq ($(uname_S),SunOS)
	# SunOS
        ifneq ($(@@),32bit)
		CFLAGS+= -m64
		LDFLAGS+= -m64
	endif
	DEBUG=-g
	DEBUG_FLAGS=-g
	export CFLAGS LDFLAGS DEBUG DEBUG_FLAGS
	INSTALL=cp -pf
	FINAL_CFLAGS+= -D__EXTENSIONS__ -D_XPG6
	FINAL_LIBS+= -ldl -lnsl -lsocket -lresolv -lpthread -lrt
else
ifeq ($(uname_S),Darwin)
	# Darwin
	FINAL_LIBS+= -ldl
else
ifeq ($(uname_S),AIX)
        # AIX
        FINAL_LDFLAGS+= -Wl,-bexpall
        FINAL_LIBS+=-ldl -pthread -lcrypt -lbsd
else
ifeq ($(uname_S),OpenBSD)
	# OpenBSD
	FINAL_LIBS+= -lpthread
	ifeq ($(USE_BACKTRACE),yes)
	    FINAL_CFLAGS+= -DUSE_BACKTRACE -I/usr/local/include
	    FINAL_LDFLAGS+= -L/usr/local/lib
	    FINAL_LIBS+= -lexecinfo
    	endif
else
ifeq ($(uname_S),FreeBSD)
	# FreeBSD
	FINAL_LIBS+= -lpthread -lexecinfo
else
ifeq ($(uname_S),DragonFly)
	# FreeBSD
	FINAL_LIBS+= -lpthread -lexecinfo
else
	# All the other OSes (notably Linux)
	FINAL_LDFLAGS+= -rdynamic
	FINAL_LIBS+=-ldl -pthread -lrt
endif
endif
endif
endif
endif
endif



DIR_SRC 		= ./src
DIR_OBJ 		= ./obj
DIR_BIN 		= ./bin

CCCOLOR			= "\033[34m"
LINKCOLOR		= "\033[34;1m"
SRCCOLOR		= "\033[33m"
BINCOLOR		= "\033[37;1m"
MAKECOLOR		= "\033[32;1m"
ENDCOLOR		= "\033[0m"

QUIET_CC      	= @printf '    %b %b\n' $(CCCOLOR)CC$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_LINK    	= @printf '    %b %b\n' $(LINKCOLOR)LINK$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_INSTALL 	= @printf '    %b %b\n' $(LINKCOLOR)INSTALL$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR) 1>&2;

LOG_CC			= $(QUIET_CC)$(CC) $(FINAL_CFLAGS)
LOG_LD			= $(QUIET_LINK)$(CC) $(FINAL_LDFLAGS)
LOG_INSTALL		= $(QUIET_INSTALL)$(INSTALL)

SERVER_NAME		= LogServer
BIN_TARGET 		= ${DIR_BIN}/${SERVER_NAME}
OBJ_TARGET 		= $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir $(wildcard ${DIR_SRC}/*.c)))

${DIR_OBJ}/%.o: ${DIR_SRC}/%.c
	$(LOG_CC) $(CFLAGS) -c $< -o $@

${BIN_TARGET}: ${OBJ_TARGET)
	$(LOG_LD) $(OBJ_TARGET) -o $@ $^ $(FINAL_LIBS)

.PHONY: clean
clean:
	rm -rf ${DIR_OBJ}/*.o

.PHONY: distclean
distclean:
	rm -rf ${DIR_OBJ} ${DIR_BIN}
