include ../make.common

# MERGE		= cat
# SOFTLINK	= ln -s
# RM		= rm
# MAKE		= make
CFLAGS		= -Dunix -DUNIX -I. -g -I../include -Wall
LFLAGS		= -g
# ASM		= rma

# %.l: %.r
# 	$(MERGE) $< > $@

# %.r: %.a
# 	$(ASM) $< -o=$@

#vpath %.c ../../../rlink
#vpath %.h ../../../rlink

# CFLAGS += -DDEBUG

ifeq ($(TARGETOS),Linux)
	CFLAGS+=-mx32
	LFLAGS+=-mx32
endif

ifeq ($(TARGETOS),Mingw)
	LIBS+=-lws2_32
endif

BINARY = rlink
OBJS = rlink_main.o rl_pass1a.o rl_pass1b.o rl_pass2.o os9out.o decbout.o

$(BINARY):	$(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@ $(LIBS)
	
$(OBJS):	rlink.h out.h ../include/rof.h

clean:
	rm -f $(BINARY) *.o *.exe

install: $(BINARY)
	cp $(BINARY) ../../test/dd/cmds
