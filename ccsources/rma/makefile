include ../make.common

# MERGE		= cat
# SOFTLINK	= ln -s
# RM		= rm
# MAKE		= make
CFLAGS		= -I. -g -fsigned-char -I../include
LFLAGS		= -g
# ASM		= rma

ifeq ($(TARGETOS),Linux)
	CFLAGS+=-mx32
	LFLAGS+=-mx32
endif

# %.l: %.r
# 	$(MERGE) $< > $@

# %.r: %.a
# 	$(ASM) $< -o=$@

#vpath %.c ../../../rlink
#vpath %.h ../../../rlink

BINARY	= rma
OBJS	= chcodes.o part2.o part3.o part4.o part5.o part6.o part7.o part8.o part9.o rma_main.o

$(BINARY):	$(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

clean:
	rm -f $(BINARY) *.o *.exe

install: $(BINARY)
	cp $(BINARY) ../../test/dd/cmds
