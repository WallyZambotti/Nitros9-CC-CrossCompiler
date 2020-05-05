TARGET=texturebm
CC=cc253
CFLAGS=-A -s -r
OBJS=texturebm.r gfxlib.r gpulib.r

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -f=$(TARGET)

gfxlib.r: gfxlib.c gfxlib.h types.h
	$(CC) $(CFLAGS) gfxlib.c

gpulib.r: gpulib.c gpulib.h types.h
	$(CC) $(CFLAGS) gpulib.c

texturebm.r: texturebm.c gfxlib.h gpulib.h types.h
	$(CC) $(CFLAGS) texturebm.c
