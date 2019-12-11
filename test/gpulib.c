#include <os9.h>
#include "gpulib.h"

#define GPU_GetQueueLen 62
#define GPU_GetTicks 63
#define GPU_NewScreen 64
#define GPU_DestroyScreen 65
#define GPU_SetColor 66
#define GPU_SetPixel 67
#define GPU_DrawLine 68
#define GPU_NewTexture 69
#define GPU_DestroyTexture 70
#define GPU_SetTextureTransparency 71
#define GPU_LoadTexture 72
#define GPU_RenderTexture 73

uchar *GPUcmd = (uchar*)0xFF60;
uint *GPUi1 = (uint*)0xFF61;
uint **GPUip1 = (uint**)0xFF61;
ulong **GPUlp1 = (ulong**)0xFF61;
uint *GPUi2 = (uint*)0xFF63;
uint *GPUi3 = (uint*)0xFF65;
uchar **GPUcp3 = (uchar**)0xFF65;
uint *GPUi4 = (uint*)0xFF67;
uint *GPUi5 = (uint*)0xFF69;
uint **GPUip5 = (uint**)0xFF69;
Rect **GPUrp5 = (Rect**)0xFF69;

void GPUGetQueueLen(uint *lenref)
{
  *GPUip1 = lenref;
  *GPUcmd = GPU_GetQueueLen;
}

void GPUGetTicks(ulong *clockref)
{
  *GPUlp1 = clockref;
  *GPUcmd = GPU_GetTicks;
}

void GPUNewScreen(
  uint *screenref, uint screenmembase, uint width, uint height, uint bpp)
{
  *GPUip1 = screenref; 
  *GPUi2 = screenmembase; 
  *GPUi3 = width; 
  *GPUi4 = height;
  *GPUi5 = bpp; 
  *GPUcmd = GPU_NewScreen;
}

void GPUDestroyScreen(uint screenid)
{
  *GPUi1 = screenid; 
  *GPUcmd = GPU_DestroyScreen;
}

void GPUNewTexture(uint *textureid, uint width, uint height, uint bpp)
{
  *GPUip1 = textureid; 
  *GPUi2 = width; 
  *GPUi3 = height; 
  *GPUi4 = bpp;
  *GPUcmd = GPU_NewTexture;
}

void GPUDestroyTexture(uint textureid)
{
  *GPUi1 = textureid; 
  *GPUcmd = GPU_DestroyTexture;
}

void GPUSetColor(uint screenid, uint color)
{
  *GPUi1 = screenid;
  *GPUi2 = color; 
  *GPUcmd = GPU_SetColor;
}

void GPUSetPixel(uint screenid, uint x, uint y)
{
  *GPUi1 = screenid;
  *GPUi2 = x; 
  *GPUi3 = y; 
  *GPUcmd = GPU_SetPixel;
}

void GPUDrawLine(uint screenid, uint x1, uint y1, uint x2, uint y2)
{
  *GPUi1 = screenid;
  *GPUi2 = x1; 
  *GPUi3 = y1; 
  *GPUi4 = x2;
  *GPUi5 = y2;
  *GPUcmd = GPU_DrawLine;
}

void GPULoadTexture(uint screenid, uint textureid, uchar *textureaddr)
{
  *GPUi1 = screenid; 
  *GPUi2 = textureid; 
  *GPUcp3 = textureaddr;
  *GPUcmd = GPU_LoadTexture;
}

void GPUSetTextureTransparency(uint textureid, uint onoff, uint color)
{
  *GPUi1 = textureid;
  *GPUi2 = onoff;
  *GPUi3 = color;
  *GPUcmd = GPU_SetTextureTransparency;
}

void GPURenderTexture(
  uint screenid, uint textureid, uint screenx, uint screeny, Rect *texturerect)
{
  *GPUi1 = screenid; 
  *GPUi2 = textureid; 
  *GPUi3 = screenx; 
  *GPUi4 = screeny;
  *GPUrp5 = texturerect; 
  *GPUcmd = GPU_RenderTexture;
}
