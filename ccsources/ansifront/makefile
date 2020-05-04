include ../make.common

OBJS=ansifront.o binexpr.o expr.o token.o decl.o poexpr.o util.o
CFLAGS=-g #-I/dd/defs

ifeq ($(TARGETOS),Linux)
	CFLAGS+=-mx32
	LFLAGS+=-mx32
endif

ansifront: $(OBJS)
	cc $(LFLAGS) -o ansifront $(OBJS)

install:
	cp ansifront ../../test/dd/cmds

clean:
	rm -f ansifront *.o *.exe