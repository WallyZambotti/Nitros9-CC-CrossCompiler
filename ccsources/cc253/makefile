include ../make.common

OBJS=cc253.o

CFLAGS=-g -fsigned-char 
LFLAGS=-g

ifeq ($(TARGETOS),Linux)
	CFLAGS+=-mx32 -DUNIX
	LFLAGS+=-mx32
endif

ifeq ($(TARGETOS),LinuxHF)
	CFLAGS+=-DUNIX
endif

cc253: $(OBJS)
	$(CC) $(LFLAGS) -o cc253 $(OBJS)

clean:
	rm -f *.o cc253 *.exe
	
install:
	cp cc253 ../../test/dd/cmds