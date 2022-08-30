#include "Intellisense.h"
#include "gba.h"

// list defines for the screen register at 0x4000000
#define REG_DISPCNT *(unsigned short *)0x4000000
#define MODE_0 0x0		  // set mode 0 - tiled, bkgs 0123, no rot/scale
#define MODE_1 0x1		  // set mode 1 - tiled, bkgs 012, rot/scale 2
#define MODE_2 0x2		  // set mode 2 - tiled, bkgs 23,  rot/scale 23
#define MODE_3 0x3		  // set mode 3 - 16bit buffer (enable bkg 2 to use)
#define MODE_4 0x4		  // set mode 4 - 8bit buffer, double bufferable
#define MODE_5 0x5		  // set mode 5 - 16bit buffer, double bufferable at 160x128
#define BACKBUFFER 0x10	  // controls which buffer is active in mode 4 or 5
#define BKG0_ENABLE 0x100 // enable bkg 0
#define BKG1_ENABLE 0x200 // enable bkg 1
#define BKG2_ENABLE 0x400 // enable bkg 2
#define BKG3_ENABLE 0x800 // enable bkg 3
#define OBJ_ENABLE 0x1000 // enable objects
// define vblank reg, contains line vblank currently at
#define VBLANK_CNT *(volatile unsigned short *)0x4000006 // the vertical line being draw, volatile as changed outside of code
// list defines for the Video ram buffer at 0x6000000
#define VRAM 0x6000000		   // start of VRAM front buffer
#define BACK_VRAM_M5 0x600A000 // start of VRAM back buffer

// globals
unsigned short *VideoBuffer; // location we are currently drawing to, flips between front and back buffer

void SwapBuffer(void) // this swaps the locations of the current display and current writing location
{
	if (REG_DISPCNT & BACKBUFFER) // backbuffer is being displayed so swap
	{
		REG_DISPCNT &= ~BACKBUFFER;					  // make display show the frontbuffer (remove backbuffer using it's bitwise compliement)
		VideoBuffer = (unsigned short *)BACK_VRAM_M5; // make backbuffer the one we draw to
	}
	else // frontbuffer is being displayed so swap
	{
		REG_DISPCNT |= BACKBUFFER;			  // make display show the backbuffer (add backbuffer)
		VideoBuffer = (unsigned short *)VRAM; // make frontbuffer the one we draw to
	}
}

void WaitVBlank(void) // waits for vblank to complete
{
	while (VBLANK_CNT < 160)
	{
	} // do nothing untill vblank has completed
}

typedef struct Point
{
	unsigned char X;
	unsigned char Y;
} Point;

void BlankScreen(unsigned short *ptr, unsigned short Colour)
{
	unsigned short counter = 0;

	while (counter++ < 20480)
		*ptr++ = Colour;
}

void DrawRect(unsigned short *ptr, Point TopLeft, Point BottomRight, unsigned short Colour)
{
	unsigned char x, y;

	for (x = TopLeft.X; x < BottomRight.X; x++)
	{
		for (y = TopLeft.Y; y < BottomRight.Y; y++)
		{
			*(ptr + y * 160 + x) = Colour;
		}
	}
}

int main()
{
	unsigned short counter = 0;

	REG_DISPCNT = (MODE_5 | BKG2_ENABLE);
	VideoBuffer = (unsigned short *)BACK_VRAM_M5;

	Point P_Loc = {0, 0};

	while (1)
	{
		if (KEY_R)
		{
			P_Loc.X+=3;
			if (P_Loc.X > 160)
				P_Loc.X = 160;
		}

		if (KEY_L)
		{
			P_Loc.X-=3;
			if (P_Loc.X < 0)
				P_Loc.X = 160;
		}
		BlankScreen(VideoBuffer, 31);

		Point B = {P_Loc.X + 10, P_Loc.Y + 10};

		DrawRect(VideoBuffer, P_Loc, B, 31 << 10);

		WaitVBlank();
		SwapBuffer();

		BlankScreen(VideoBuffer, 31 << 10);

		WaitVBlank();
		SwapBuffer();
	}

	return 0;
}