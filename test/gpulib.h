#include "types.h"

#ifndef _gpulib_
#define _gpulib_ 1
struct _rect
{
  int x, y, w, h;
};

typedef struct _rect Rect;
#endif

void GPUGetTicks(ulong*);
void GPUNewScreen(uint*, uint, uint, uint, uint); 
void GPUDestroyScreen(uint);
void GPUNewTexture(uint*, uint, uint, uint);
void GPUDestroyTexture(uint);
void GPUSetColor(uint, uint);
void GPUSetPixel(uint, uint, uint);
void GPUDrawLine(uint, uint, uint, uint, uint);
void GPULoadTexture(uint, uint, uchar*);
void GPURenderTexture(uint, uint, uint, uint, Rect*);
