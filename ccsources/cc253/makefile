include ../make.common

OBJS=cc253.o

CFLAGS=-g
LFLAGS=-g

ifeq ($(TARGETOS),Linux)
	CFLAGS+=-mx32  -DUNIX
	LFLAGS+=-mx32
endif

cc253: $(OBJS)
	cc $(LFLAGS) -o cc253 $(OBJS)

clean:
	rm -f *.o cc253 *.exe
	
install:
	cp cc253 ../../test/dd/cmds