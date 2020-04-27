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

CFLAGS    += -mx32
LFLAGS    += -mx32
BINARY	= rma
OBJS	= chcodes.o part2.o part3.o part4.o part5.o part6.o part7.o part8.o part9.o rma_main.o

$(BINARY):	$(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) ../../test/dd/cmds
