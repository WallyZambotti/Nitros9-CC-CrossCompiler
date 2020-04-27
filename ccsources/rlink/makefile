# MERGE		= cat
# SOFTLINK	= ln -s
# RM		= rm
# MAKE		= make
CFLAGS		= -mx32 -Dunix -DUNIX -I. -g -I../include -Wall
# ASM		= rma

# %.l: %.r
# 	$(MERGE) $< > $@

# %.r: %.a
# 	$(ASM) $< -o=$@

#vpath %.c ../../../rlink
#vpath %.h ../../../rlink

# CFLAGS += -DDEBUG

BINARY = rlink
OBJS = rlink_main.o rl_pass1a.o rl_pass1b.o rl_pass2.o os9out.o decbout.o

$(BINARY):	$(OBJS)
	$(CC) -mx32 $(OBJS) -o $@

$(OBJS):	rlink.h out.h ../include/rof.h

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) ../../test/dd/cmds
