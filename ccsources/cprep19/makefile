include ../make.common

RELS = cp.o cp1.o cp2.o cp3.o cp4.o solve.o

CFLAGS = -g -I.
LFLAGS = -g

ifeq ($(TARGETOS),Linux)
	CFLAGS+=-mx32
	LFLAGS+=-mx32
endif

coprep: $(RELS)
	gcc $(LFLAGS) $(RELS) -o $@

$(RELS): cp.h

clean:
	rm -f *.o *.exe coprep

install:
	cp coprep ../../test/dd/cmds