release_hdr := $(shell sh -c 'chmod 777 mkreleasehdr.sh && ./mkreleasehdr.sh')
uname_S 	:= $(shell sh -c 'uname -s 2>/dev/null 		|| echo not')
uname_M 	:= $(shell sh -c 'uname -m 2>/dev/null 		|| echo not')

# Default settings
OPTIMIZATION?=-O2

WARN	= -Wall -W -Wno-missing-field-initializers
OPT		= $(OPTIMIZATION)
CC 		= gcc
DEBUG	= -g

STD		= -std=c99 -pedantic
ifneq (,$(findstring clang,$(CC)))
ifneq (,$(findstring FreeBSD,$(uname_S)))
	STD+=-Wno-c11-extensions
endif
endif

MALLOC=libc

# To get ARM stack traces if Redis crashes we need a special C flag.
ifneq (,$(filter aarch64 armv,$(uname_M)))
        CFLAGS+=-funwind-tables
else
ifneq (,$(findstring armv,$(uname_M)))
        CFLAGS+=-funwind-tables
endif
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
	FINAL_LIBS+=-ldl -lpthread -lrt
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

LOG_SERVER_NAME	= $(DIR_BIN)/LogServer
LOG_CLI_NAME	= $(DIR_BIN)/LogClient
LOG_SERVER_OBJ	= $(DIR_OBJ)/log.o $(DIR_OBJ)/util.o $(DIR_OBJ)/server.o
LOG_CLI_OBJ		= $(DIR_OBJ)/client.o $(DIR_OBJ)/util.o 
OBJ_TARGET 		= $(patsubst %.c,$(DIR_OBJ)/%.o,$(notdir $(wildcard ${DIR_SRC}/*.c)))

all: $(LOG_SERVER_NAME) $(LOG_CLI_NAME)
	@echo ""
	@echo "Hint: It's a good idea to run 'make test' ;)"
	@echo ""

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.c
	$(LOG_CC) $(CFLAGS) -c $< -o $@

# log-server
$(LOG_SERVER_NAME): $(LOG_SERVER_OBJ)
	$(LOG_LD) -o $@ $^ $(FINAL_LIBS)

# log-cli
$(LOG_CLI_NAME): $(LOG_CLI_OBJ)
	$(LOG_LD) -o $@ $^ $(FINAL_LIBS)


.PHONY: clean
clean:
	#rm -rf ${DIR_OBJ}/*.o
	rm -rf ${DIR_OBJ} ${DIR_BIN} ./src/release.h LocalSysLog LOG/

.PHONY: distclean
distclean:
	rm -rf ${DIR_OBJ} ${DIR_BIN} ./src/release.h LocalSysLog LOG/
