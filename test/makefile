TARGET=texturebm

OBJS=texturebm.r gfxlib.r gpulib.r

$(TARGET): $(OBJS)
	echo cc $(OBJS) -f=$(TARGET)

gfxlib.r: gfxlib.c gfxlib.h types.h
	./ccf gfxlib.c

gpulib.r: gpulib.c gpulib.h types.h
	./ccf gpulib.c

texturebm.r: texturebm.c gfxlib.h gpulib.h types.h
	./ccf texturebm.c
