#include <stdio.h>
#include <os9.h>
#include <signal.h>
#include "types.h"
#include "gpulib.h"
#include "gfxlib.h"

struct _character
{
  uint textureid;
  int direction;
  int xloc, yloc;
};

typedef struct _character Character;

#define MAX_CHARACTERS 1024
#define MAX_QUEUE_LEN 10

Character characters[MAX_CHARACTERS];
int maxCharCnt = 0;
Rect rect;
uint scrnid;
int frame = 0;
int looping = 1;

void RenderCharacter(int);
int LoadAllCharacterTextures(int, char **);
void LoadTextureFromFile(int);
void DestroyAllCharacterTextures();

/*
   CatchSig handles signals 2 & 3 clearing 'looping' variable
   causing the main rendering loop to exit.
*/

CatchSig(int sig)
{
  looping = 0;  
}

unsigned char cifbuffer[5184];

int main(int argc, char **argv)

{
  short delays[3] = { 17, 17, 16 }, timediff, recorddiff;
  ulong timestamp, currenttime;
  int wp, tp, bytes=1;
  int scrn, swamped=0;
  uint queueLen = 0;
  int charCnt = 0, charidx, frames60=0;
  uchar *scrnbase;
  char buffer[8];
  struct registers regs;
  unsigned char colors[16] = { 
    0x00,     /* Black */
    0x04,     /* Dark Red */
    0x09,     /* Bright Blue */
    0x05,     /* Purple */
    0x24,     /* Red */
    0x22,     /* Brown */
    0x02,     /* Dark Green */
    0x27,     /* Flesh */
    0x14,     /* Green */
    0x39,     /* Light Blue */
    0x3C,     /* Pink */
    0x35,     /* Orange */
    0x3B,     /* Cyan */
    0x3F,     /* White */
    0x36,     /* Yellow */
    0x07      /* Dark Grey */
  } ;
  int colorreg, col, i;
  unsigned char byte;

  /* Catch the standard interupts which are used to exit the main loop */
  
  signal(2, CatchSig);
  signal(3, CatchSig);

  /* Check for a file in the arguements */

  if (argc < 2)
  {
    fprintf(stderr, "disp_raw <raw_image_file(s)>...\n");
    return 0;
  }

  /* Wait for user to proceed */

  fprintf(stderr, "Hit a character key to begin\n");
  read(0, &buffer, 1);

  /* open a window */

  wp = open("/v1", 3);

  if (wp == -1)
  {
    fprintf(stderr, "Can't open /w\n");
    return -1;
  }

  /* Allocate and Map the window into this process space */

  if(GFXAllocateScreen(wp, 4, &scrn, &scrnbase))
  {
    fprintf(stderr, "Can't allocate and map screen\n");
    close(wp);
    return -1;
  }

  /* Display Screen */

  if (GFXDisplayScreen(wp, scrn))
  {
    fprintf(stderr, "Can't display screen\n");
    close(wp);
    return -1;
  }

  /* Set Color Palette */

  GFXSetPalette(wp, colors);

  /* Display Test Pattern */

  for(i = 0 ; i < 30720 ; i++)
  {
    col = i / 1920;
    *(scrnbase + i) = (col<<4) + col;
  }

  /* Setup GPU. Provide the GPU the memory address of the VDG screen */

  GPUNewScreen(&scrnid, (uint)scrnbase, 320, 192, 4);

  /* Load all character textures from files provided via command line (args) */

  if(LoadAllCharacterTextures(argc, argv) != 0)
  {
    DestroyAllCharacterTextures();
    GPUDestroyScreen(scrnid);    
    GFXDisplayScreen(wp, 0); /* Un-display the Screen */
    GFXFreeScreen(wp, scrn);
    close(wp);
    return -1;
  }

  /* Define Texture rectangle */

  rect.x = 0; rect.y = 0; rect.w = 18; rect.h = 18;

  /* Main Render loop */

  /* Each character has a start vector of x 0, y 0 and direction 3 */
  
  characters[charCnt].direction = 3;
  characters[charCnt].xloc = 0;
  characters[charCnt].yloc = 0;
  charCnt++;

  GPUGetTicks(&timestamp); /* Record a timestamp for frame timing purposes */
  while (looping)
  {
    int delay = delays[frame];

    for (charidx = 0 ; charidx < charCnt ; charidx++)
    {
      RenderCharacter(charidx);
    }

    GPUGetTicks(&currenttime);
    recorddiff = timediff = currenttime - timestamp;

    frames60 = ++frames60%60;

    if (queueLen > MAX_QUEUE_LEN) swamped++;

    /* 
    Every sixty frames (1 second) fire up a new character
    if we haven't run out of frame time, character array or
    swamped the GPU with render texture requests.
    */

    if(frames60 == 0)
    {
      if (timediff < delay && charCnt < MAX_CHARACTERS &&
        queueLen < MAX_QUEUE_LEN)
      {
        /* The first maxcharCnt characters already have textureid's
        reuse those textureid's for the subsequent characters */
        
        if (charCnt >= maxCharCnt)
        {
          characters[charCnt].textureid = 
            characters[charCnt%maxCharCnt].textureid;
        }
        /* Otherwise every new character has the same start vectors */
        characters[charCnt].direction = 3;
        characters[charCnt].xloc = 0;
        characters[charCnt].yloc = 0;
        charCnt++;
      }
      else if (charCnt > 0)
      {
        charCnt--;
      }
    }

    /* Delay/Poll the GPU timer until the frame time 16.6667ms has elapsed */

    while (timediff < delay)
    {
      GPUGetTicks(&currenttime);
      timediff = currenttime - timestamp;
    }

    /* Get the GPU queue length so the character fire up logic doesn't
    swamp the GPU with too many characters */
       
    GPUGetQueueLen(&queueLen);
    GPUGetTicks(&timestamp);
    frame = ++frame%3;
  }

  /* Allow user to see screen & output a few stats */
  
  fprintf(stderr, "Character count %d Queue Len %d Swamped %d time left %d\n",
    charCnt, queueLen, swamped, recorddiff);
  fprintf(stderr, "Hit a character key to proceed\n");
  read(0, &buffer, 1);

  /* Destroy Texture and GPU screen */

  DestroyAllCharacterTextures();
  GPUDestroyScreen(scrnid);

  /* Un-display the Screen */

  GFXDisplayScreen(wp, 0);

  if (GFXDisplayScreen(wp, 0))
  {
    fprintf(stderr, "Can't un-display screen\n");
  }

  /* Free Screen */

  if (GFXFreeScreen(wp, scrn))
  {
    fprintf(stderr, "Can't free screen %d\n", regs.rg_b & 0xff);
  }

  /* Close the vdg window */

  close(wp);
}

void RenderCharacter(int charidx)
{
    register Character *character = &characters[charidx];
    rect.x = 18 * character->direction + 144 * frame;
    GPURenderTexture(scrnid, character->textureid, character->xloc, character->yloc, &rect);

    GPUSetColor(scrnid, 15);
    switch(character->direction)
    {
      case 0: 
        GPUDrawLine(scrnid, character->xloc, character->yloc+17, 
          character->xloc+17, character->yloc+17);
        if (character->yloc > 0) --character->yloc;
        break;
      case 1:
        GPUDrawLine(scrnid, character->xloc, character->yloc+17,
          character->xloc+17, character->yloc+17);
        GPUDrawLine(scrnid, character->xloc, character->yloc,
          character->xloc, character->yloc+17);
        if (character->yloc > 0) --character->yloc; 
        else character->direction = 3;
        if (character->xloc < 302) ++character->xloc;
        else character->direction = 7;
        break;
      case 2:
        GPUDrawLine(scrnid, character->xloc, character->yloc,
          character->xloc, character->yloc+17);
        if (character->xloc < 302) ++character->xloc;
        break;
      case 3:
        GPUDrawLine(scrnid, character->xloc, character->yloc,
          character->xloc, character->yloc+17);
        GPUDrawLine(scrnid, character->xloc, character->yloc,
          character->xloc+17, character->yloc);
        if (character->yloc < 174) ++character->yloc;
        else character->direction = 1;
        if (character->xloc < 302) ++character->xloc;
        else character->direction = 5;
        break;
      case 4:
        GPUDrawLine(scrnid, character->xloc, character->yloc,
          character->xloc+17, character->yloc);
        if (character->yloc < 174) ++character->yloc;
        break;
      case 5:
        GPUDrawLine(scrnid, character->xloc, character->yloc,
          character->xloc+17, character->yloc);
        GPUDrawLine(scrnid, character->xloc+17, character->yloc,
          character->xloc+17, character->yloc+17);
        if (character->yloc < 174) ++character->yloc;
        else character->direction = 7;
        if (character->xloc > 0) --character->xloc;
        else character->direction =3;
        break;
      case 6:
        GPUDrawLine(scrnid, character->xloc+17, character->yloc,
          character->xloc+17, character->yloc+17);
        if (character->xloc > 0) --character->xloc;
        break;
      case 7:
        GPUDrawLine(scrnid, character->xloc+17, character->yloc,
          character->xloc+17, character->yloc+17);
        GPUDrawLine(scrnid, character->xloc, character->yloc+17,
          character->xloc+17, character->yloc+17);
        if (character->yloc > 0) --character->yloc;
        else character->direction = 5;
        if (character->xloc > 0) --character->xloc;
        else character->direction = 1;
        break;
    }
}

int LoadAllCharacterTextures(int argc, char *argv[])
{
  int cif, fileidx;

  for (fileidx = 1 ; fileidx < argc ; fileidx++)
  {
    cif = open(argv[fileidx], 1);

    if (cif == -1)
    {
      fprintf(stderr, "Cannot open %s\n", argv[1]);
      return -1;
    }

    LoadTextureFromFile(cif);
  }

  return 0;
}

void LoadTextureFromFile(int cif)
{
  /* Read character texture into temp buffer  */

  int bytes = read(cif, cifbuffer, 5184);
  close(cif);

  /* Define texture */

  GPUNewTexture(&characters[maxCharCnt].textureid, 576, 18, 4);

  /* Load Texture pixmap */

  GPULoadTexture(scrnid, characters[maxCharCnt].textureid, cifbuffer);
  
  /* Set Texture Transparency */

  GPUSetTextureTransparency(characters[maxCharCnt].textureid, 1, 15);

  maxCharCnt++;
}

void DestroyAllCharacterTextures()
{
  int idx;
  for(idx = 0 ; idx < maxCharCnt ; idx++)
  {
    GPUDestroyTexture(characters[idx].textureid);
  }
}
