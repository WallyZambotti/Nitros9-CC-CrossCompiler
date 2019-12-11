#include <os9.h>
#include "gfxlib.h"

#define SS_AScrn 0x8B
#define SS_DScrn 0x8C
#define SS_FScrn 0x8D
#define SS_DFPal 0x97

int GFXAllocateScreen(int winpath, int winmode, int *screenid, uchar **screenmemaddr)
{
  struct registers regs;
  regs.rg_a = winpath; /* path number of /w */
  regs.rg_b = SS_AScrn ; /* Alloc and map high res screen into process */
  regs.rg_x = winmode; /* 320 x 192 x 16 colors */
  if (_os9(I_SETSTT, &regs) == -1)
  {
    return regs.rg_b & 0xff;
  }
  *screenid = regs.rg_y;
  *screenmemaddr = (uchar*)regs.rg_x;
  return 0;
}

int GFXDisplayScreen(int winpath, int screenid)
{
  struct registers regs;
  regs.rg_a = winpath;
  regs.rg_b = SS_DScrn;
  regs.rg_y = screenid;
  if (_os9(I_SETSTT, &regs) == -1)
  {
    return regs.rg_b & 0xff;
  }
  return 0;
}

int GFXFreeScreen(int winpath, int screenid)
{
  struct registers regs;
  regs.rg_a = winpath;
  regs.rg_b = SS_FScrn;
  regs.rg_y = screenid;
  if (_os9(I_SETSTT, &regs) == -1)
  {
    return regs.rg_b & 0xff;
  }
  return 0;
}

void GFXSetPaletteColor(int winpath, uchar paletteindex, uchar colorval)
{
  uchar ChgPalet[4] = { 0x1B, 0x31, 0x00, 0x00 };
 
  ChgPalet[2] = paletteindex;
  ChgPalet[3] = colorval;
  write(winpath, ChgPalet, 4);
}

void GFXSetPalette(int winpath, uchar colortable[])
{
  uchar paletteindex;
  for (paletteindex = 0 ; paletteindex < 16 ; paletteindex++)
  {
    GFXSetPaletteColor(winpath, paletteindex, colortable[paletteindex]);
  }
}
