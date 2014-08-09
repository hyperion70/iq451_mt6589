/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * cfb_console.c
 *
 * Color Framebuffer Console driver for 8/15/16/24/32 bits per pixel.
 *
 * At the moment only the 8x16 font is tested and the font fore- and
 * background color is limited to black/white/gray colors. The Linux
 * logo can be placed in the upper left corner and additional board
 * information strings (that normaly goes to serial port) can be drawed.
 *
 * The console driver can use the standard PC keyboard interface (i8042)
 * for character input. Character output goes to a memory mapped video
 * framebuffer with little or big-endian organisation.
 * With environment setting 'console=serial' the console i/o can be
 * forced to serial port.

 The driver uses graphic specific defines/parameters/functions:

 (for SMI LynxE graphic chip)

 CONFIG_VIDEO_SMI_LYNXEM - use graphic driver for SMI 710,712,810
 VIDEO_FB_LITTLE_ENDIAN	 - framebuffer organisation default: big endian
 VIDEO_HW_RECTFILL	 - graphic driver supports hardware rectangle fill
 VIDEO_HW_BITBLT	 - graphic driver supports hardware bit blt

 Console Parameters are set by graphic drivers global struct:

 VIDEO_VISIBLE_COLS	     - x resolution
 VIDEO_VISIBLE_ROWS	     - y resolution
 VIDEO_PIXEL_SIZE	     - storage size in byte per pixel
 VIDEO_DATA_FORMAT	     - graphical data format GDF
 VIDEO_FB_ADRS		     - start of video memory

 CONFIG_I8042_KBD	     - AT Keyboard driver for i8042
 VIDEO_KBD_INIT_FCT	     - init function for keyboard
 VIDEO_TSTC_FCT		     - keyboard_tstc function
 VIDEO_GETC_FCT		     - keyboard_getc function

 CONFIG_CONSOLE_CURSOR	     - on/off drawing cursor is done with delay
			       loop in VIDEO_TSTC_FCT (i8042)
 CONFIG_SYS_CONSOLE_BLINK_COUNT     - value for delay loop - blink rate
 CONFIG_CONSOLE_TIME	     - display time/date in upper right corner,
			       needs CONFIG_CMD_DATE and CONFIG_CONSOLE_CURSOR
 CONFIG_VIDEO_LOGO	     - display Linux Logo in upper left corner
 CONFIG_VIDEO_BMP_LOGO	     - use bmp_logo instead of linux_logo
 CONFIG_CONSOLE_EXTRA_INFO   - display additional board information strings
			       that normaly goes to serial port. This define
			       requires a board specific function:
			       video_drawstring (VIDEO_INFO_X,
						 VIDEO_INFO_Y + i*VIDEO_FONT_HEIGHT,
						 info);
			       that fills a info buffer at i=row.
			       s.a: board/eltec/bab7xx.
CONFIG_VGA_AS_SINGLE_DEVICE  - If set the framebuffer device will be initialised
			       as an output only device. The Keyboard driver
			       will not be set-up. This may be used, if you
			       have none or more than one Keyboard devices
			       (USB Keyboard, AT Keyboard).

CONFIG_VIDEO_SW_CURSOR:	     - Draws a cursor after the last character. No
			       blinking is provided. Uses the macros CURSOR_SET
			       and CURSOR_OFF.
CONFIG_VIDEO_HW_CURSOR:	     - Uses the hardware cursor capability of the
			       graphic chip. Uses the macro CURSOR_SET.
			       ATTENTION: If booting an OS, the display driver
			       must disable the hardware register of the graphic
			       chip. Otherwise a blinking field is displayed
*/

/*****************************************************************************/
/* Console device defines with SMI graphic				     */
/* Any other graphic must change this section				     */
/*****************************************************************************/

#ifdef	CONFIG_VIDEO_SMI_LYNXEM

#define VIDEO_FB_LITTLE_ENDIAN
#define VIDEO_HW_RECTFILL
#define VIDEO_HW_BITBLT
#endif

/*****************************************************************************/
/* Defines for the CT69000 driver					     */
/*****************************************************************************/
#ifdef	CONFIG_VIDEO_CT69000

#define VIDEO_FB_LITTLE_ENDIAN
#define VIDEO_HW_RECTFILL
#define VIDEO_HW_BITBLT
#endif

/*****************************************************************************/
/* Defines for the SED13806 driver					     */
/*****************************************************************************/
#ifdef CONFIG_VIDEO_SED13806

#ifndef CONFIG_TOTAL5200
#define VIDEO_FB_LITTLE_ENDIAN
#endif
#define VIDEO_HW_RECTFILL
#define VIDEO_HW_BITBLT
#endif

/*****************************************************************************/
/* Defines for the SED13806 driver					     */
/*****************************************************************************/
#ifdef CONFIG_VIDEO_SM501

#ifdef CONFIG_HH405
#define VIDEO_FB_LITTLE_ENDIAN
#endif
#endif

/*****************************************************************************/
/* Defines for the MB862xx driver					     */
/*****************************************************************************/
#ifdef CONFIG_VIDEO_MB862xx

#ifdef CONFIG_VIDEO_CORALP
#define VIDEO_FB_LITTLE_ENDIAN
#endif
#ifdef CONFIG_VIDEO_MB862xx_ACCEL
#define VIDEO_HW_RECTFILL
#define VIDEO_HW_BITBLT
#endif
#endif

/*****************************************************************************/
/* Include video_fb.h after definitions of VIDEO_HW_RECTFILL etc	     */
/*****************************************************************************/
#include <video_fb.h>
#include <video_font.h>
#include <target/board.h>
#include <stdarg.h>
/*****************************************************************************/
/* some Macros								     */
/*****************************************************************************/
#define VIDEO_VISIBLE_COLS	(pGD->winSizeX)
#define VIDEO_VISIBLE_ROWS	(pGD->winSizeY)
#define VIDEO_PIXEL_SIZE	(pGD->gdfBytesPP)
#define VIDEO_DATA_FORMAT	(pGD->gdfIndex)
#define VIDEO_FB_ADRS		(pGD->frameAdrs)
#define VIDEO_FB_SIZE       (pGD->memSize)

/*****************************************************************************/
/* Console device defines with i8042 keyboard controller		     */
/* Any other keyboard controller must change this section		     */
/*****************************************************************************/
#include <platform/mt_disp_drv.h>
/*****************************************************************************/
/* Cursor definition:							     */
/* CONFIG_CONSOLE_CURSOR:  Uses a timer function (see drivers/input/i8042.c) */
/*                         to let the cursor blink. Uses the macros	     */
/*                         CURSOR_OFF and CURSOR_ON.			     */
/* CONFIG_VIDEO_SW_CURSOR: Draws a cursor after the last character. No	     */
/*			   blinking is provided. Uses the macros CURSOR_SET  */
/*			   and CURSOR_OFF.				     */
/* CONFIG_VIDEO_HW_CURSOR: Uses the hardware cursor capability of the	     */
/*			   graphic chip. Uses the macro CURSOR_SET.	     */
/*			   ATTENTION: If booting an OS, the display driver   */
/*			   must disable the hardware register of the graphic */
/*			   chip. Otherwise a blinking field is displayed     */
/*****************************************************************************/
#if !defined(CONFIG_CONSOLE_CURSOR) && \
    !defined(CONFIG_VIDEO_SW_CURSOR) && \
    !defined(CONFIG_VIDEO_HW_CURSOR)
/* no Cursor defined */
#define CURSOR_ON
#define CURSOR_OFF
#define CURSOR_SET
#endif

#ifdef	CONFIG_CONSOLE_CURSOR
#ifdef	CURSOR_ON
#error	only one of CONFIG_CONSOLE_CURSOR,CONFIG_VIDEO_SW_CURSOR,CONFIG_VIDEO_HW_CURSOR can be defined
#endif
void	console_cursor (int state);
#define CURSOR_ON  console_cursor(1);
#define CURSOR_OFF console_cursor(0);
#define CURSOR_SET
#ifndef CONFIG_I8042_KBD
#warning Cursor drawing on/off needs timer function s.a. drivers/input/i8042.c
#endif
#else
#ifdef	CONFIG_CONSOLE_TIME
#error	CONFIG_CONSOLE_CURSOR must be defined for CONFIG_CONSOLE_TIME
#endif
#endif /* CONFIG_CONSOLE_CURSOR */

#ifdef	CONFIG_VIDEO_SW_CURSOR
#ifdef	CURSOR_ON
#error	only one of CONFIG_CONSOLE_CURSOR,CONFIG_VIDEO_SW_CURSOR,CONFIG_VIDEO_HW_CURSOR can be defined
#endif
#define CURSOR_ON
#define CURSOR_OFF video_putchar(console_col * VIDEO_FONT_WIDTH,\
				 console_row * VIDEO_FONT_HEIGHT, ' ');
#define CURSOR_SET video_set_cursor();
#endif /* CONFIG_VIDEO_SW_CURSOR */


#ifdef CONFIG_VIDEO_HW_CURSOR
#ifdef	CURSOR_ON
#error	only one of CONFIG_CONSOLE_CURSOR,CONFIG_VIDEO_SW_CURSOR,CONFIG_VIDEO_HW_CURSOR can be defined
#endif
#define CURSOR_ON
#define CURSOR_OFF
#define CURSOR_SET video_set_hw_cursor(console_col * VIDEO_FONT_WIDTH, \
		  (console_row * VIDEO_FONT_HEIGHT) + video_logo_height);
#endif	/* CONFIG_VIDEO_HW_CURSOR */

#ifdef	CONFIG_VIDEO_LOGO
#define VIDEO_INFO_X		(VIDEO_LOGO_WIDTH)
#define VIDEO_INFO_Y		(VIDEO_FONT_HEIGHT/2)
#else	/* CONFIG_VIDEO_LOGO */
#define VIDEO_LOGO_WIDTH	0
#define VIDEO_LOGO_HEIGHT	0
#endif	/* CONFIG_VIDEO_LOGO */

#define VIDEO_COLS		VIDEO_VISIBLE_COLS
#define VIDEO_ROWS		VIDEO_VISIBLE_ROWS
#define VIDEO_SIZE		(VIDEO_ROWS*VIDEO_COLS*VIDEO_PIXEL_SIZE)
#define VIDEO_PIX_BLOCKS	(VIDEO_SIZE >> 2)
#define VIDEO_LINE_LEN		(VIDEO_COLS*VIDEO_PIXEL_SIZE)
#define VIDEO_BURST_LEN		(VIDEO_COLS/8)

#ifdef	CONFIG_VIDEO_LOGO
#define CONSOLE_ROWS		((VIDEO_ROWS - video_logo_height) / VIDEO_FONT_HEIGHT)
#else
#define CONSOLE_ROWS		(VIDEO_ROWS / VIDEO_FONT_HEIGHT)
#endif

#define CONSOLE_COLS		(VIDEO_COLS / VIDEO_FONT_WIDTH)
#define CONSOLE_ROW_SIZE	(VIDEO_FONT_HEIGHT * VIDEO_LINE_LEN)
#define CONSOLE_ROW_FIRST	(video_console_address)
#define CONSOLE_ROW_SECOND	(video_console_address + CONSOLE_ROW_SIZE)
#define CONSOLE_ROW_LAST	(video_console_address + CONSOLE_SIZE - CONSOLE_ROW_SIZE)
#define CONSOLE_SIZE		(CONSOLE_ROW_SIZE * CONSOLE_ROWS)
#define CONSOLE_SCROLL_SIZE	(CONSOLE_SIZE - CONSOLE_ROW_SIZE)

/* Macros */
#ifdef	VIDEO_FB_LITTLE_ENDIAN
#define SWAP16(x)	 ((((x) & 0x00ff) << 8) | ( (x) >> 8))
#define SWAP32(x)	 ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8)|\
			  (((x) & 0x00ff0000) >>  8) | (((x) & 0xff000000) >> 24) )
#define SHORTSWAP32(x)	 ((((x) & 0x000000ff) <<  8) | (((x) & 0x0000ff00) >> 8)|\
			  (((x) & 0x00ff0000) <<  8) | (((x) & 0xff000000) >> 8) )
#else
#define SWAP16(x)	 (x)
#define SWAP32(x)	 (x)
#if defined(VIDEO_FB_16BPP_WORD_SWAP)
#define SHORTSWAP32(x)	 ( ((x) >> 16) | ((x) << 16) )
#else
#define SHORTSWAP32(x)	 (x)
#endif
#endif

#if defined(DEBUG) || defined(DEBUG_CFB_CONSOLE)
#define PRINTD(x)	  printf(x)
#else
#define PRINTD(x)
#endif


#ifdef CONFIG_CONSOLE_EXTRA_INFO
extern void video_get_info_str (    /* setup a board string: type, speed, etc. */
    int line_number,	    /* location to place info string beside logo */
    char *info		    /* buffer for info string */
    );

#endif

/* Locals */
static GraphicDevice *pGD;	/* Pointer to Graphic array */

static void *video_fb_address;		/* frame buffer address */
static void *video_console_address;	/* console buffer start address */

static int video_logo_height = VIDEO_LOGO_HEIGHT;

static int console_col = 0; /* cursor col */
static int console_row = 0; /* cursor row */

static u32 eorx, fgx, bgx;  /* color pats */

static const int video_font_draw_table8[] = {
	    0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
	    0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
	    0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
	    0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff };

static const int video_font_draw_table15[] = {
	    0x00000000, 0x00007fff, 0x7fff0000, 0x7fff7fff };

static const int video_font_draw_table16[] = {
#if 0
	    0x00000000, 0x0000ffff, 0xffff0000, 0xffffffff };
#else   // Jett Patch: RGB565 with Little Endian Table
        0x00000000, 0xffff0000, 0x0000ffff, 0xffffffff };
#endif

static const int video_font_draw_table24[16][3] = {
	    { 0x00000000, 0x00000000, 0x00000000 },
	    { 0x00000000, 0x00000000, 0x00ffffff },
	    { 0x00000000, 0x0000ffff, 0xff000000 },
	    { 0x00000000, 0x0000ffff, 0xffffffff },
	    { 0x000000ff, 0xffff0000, 0x00000000 },
	    { 0x000000ff, 0xffff0000, 0x00ffffff },
	    { 0x000000ff, 0xffffffff, 0xff000000 },
	    { 0x000000ff, 0xffffffff, 0xffffffff },
	    { 0xffffff00, 0x00000000, 0x00000000 },
	    { 0xffffff00, 0x00000000, 0x00ffffff },
	    { 0xffffff00, 0x0000ffff, 0xff000000 },
	    { 0xffffff00, 0x0000ffff, 0xffffffff },
	    { 0xffffffff, 0xffff0000, 0x00000000 },
	    { 0xffffffff, 0xffff0000, 0x00ffffff },
	    { 0xffffffff, 0xffffffff, 0xff000000 },
	    { 0xffffffff, 0xffffffff, 0xffffffff } };

static const int video_font_draw_table32[16][4] = {
	    { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
	    { 0x00000000, 0x00000000, 0x00000000, 0x00ffffff },
	    { 0x00000000, 0x00000000, 0x00ffffff, 0x00000000 },
	    { 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff },
	    { 0x00000000, 0x00ffffff, 0x00000000, 0x00000000 },
	    { 0x00000000, 0x00ffffff, 0x00000000, 0x00ffffff },
	    { 0x00000000, 0x00ffffff, 0x00ffffff, 0x00000000 },
	    { 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff },
	    { 0x00ffffff, 0x00000000, 0x00000000, 0x00000000 },
	    { 0x00ffffff, 0x00000000, 0x00000000, 0x00ffffff },
	    { 0x00ffffff, 0x00000000, 0x00ffffff, 0x00000000 },
	    { 0x00ffffff, 0x00000000, 0x00ffffff, 0x00ffffff },
	    { 0x00ffffff, 0x00ffffff, 0x00000000, 0x00000000 },
	    { 0x00ffffff, 0x00ffffff, 0x00000000, 0x00ffffff },
	    { 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00000000 },
	    { 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff } };


/******************************************************************************/

static void video_drawchars (int xx, int yy, unsigned char *s, int count)
{
	u8 *cdat, *dest, *dest0;
	int rows, offset, c;

	offset = yy * VIDEO_LINE_LEN + xx * VIDEO_PIXEL_SIZE;
	dest0 = video_fb_address + offset;

	switch (VIDEO_DATA_FORMAT) {
	case GDF__8BIT_INDEX:
	case GDF__8BIT_332RGB:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--;
			     dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] = (video_font_draw_table8[bits >> 4] & eorx) ^ bgx;
				((u32 *) dest)[1] = (video_font_draw_table8[bits & 15] & eorx) ^ bgx;
			}
			dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
			s++;
		}
		break;

	case GDF_15BIT_555RGB:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--;
			     dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] = SHORTSWAP32 ((video_font_draw_table15 [bits >> 6] & eorx) ^ bgx);
				((u32 *) dest)[1] = SHORTSWAP32 ((video_font_draw_table15 [bits >> 4 & 3] & eorx) ^ bgx);
				((u32 *) dest)[2] = SHORTSWAP32 ((video_font_draw_table15 [bits >> 2 & 3] & eorx) ^ bgx);
				((u32 *) dest)[3] = SHORTSWAP32 ((video_font_draw_table15 [bits & 3] & eorx) ^ bgx);
			}
			dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
			s++;
		}
		break;

	case GDF_16BIT_565RGB:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--;
			     dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] = SHORTSWAP32 ((video_font_draw_table16 [bits >> 6] & eorx) ^ bgx);
				((u32 *) dest)[1] = SHORTSWAP32 ((video_font_draw_table16 [bits >> 4 & 3] & eorx) ^ bgx);
				((u32 *) dest)[2] = SHORTSWAP32 ((video_font_draw_table16 [bits >> 2 & 3] & eorx) ^ bgx);
				((u32 *) dest)[3] = SHORTSWAP32 ((video_font_draw_table16 [bits & 3] & eorx) ^ bgx);
			}
			dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
			s++;
		}
		break;

	case GDF_32BIT_X888RGB:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--;
			     dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] = SWAP32 ((video_font_draw_table32 [bits >> 4][0] & eorx) ^ bgx);
				((u32 *) dest)[1] = SWAP32 ((video_font_draw_table32 [bits >> 4][1] & eorx) ^ bgx);
				((u32 *) dest)[2] = SWAP32 ((video_font_draw_table32 [bits >> 4][2] & eorx) ^ bgx);
				((u32 *) dest)[3] = SWAP32 ((video_font_draw_table32 [bits >> 4][3] & eorx) ^ bgx);
				((u32 *) dest)[4] = SWAP32 ((video_font_draw_table32 [bits & 15][0] & eorx) ^ bgx);
				((u32 *) dest)[5] = SWAP32 ((video_font_draw_table32 [bits & 15][1] & eorx) ^ bgx);
				((u32 *) dest)[6] = SWAP32 ((video_font_draw_table32 [bits & 15][2] & eorx) ^ bgx);
				((u32 *) dest)[7] = SWAP32 ((video_font_draw_table32 [bits & 15][3] & eorx) ^ bgx);
			}
			dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
			s++;
		}
		break;

	case GDF_24BIT_888RGB:
		while (count--) {
			c = *s;
			cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
			for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
			     rows--;
			     dest += VIDEO_LINE_LEN) {
				u8 bits = *cdat++;

				((u32 *) dest)[0] = (video_font_draw_table24[bits >> 4][0] & eorx) ^ bgx;
				((u32 *) dest)[1] = (video_font_draw_table24[bits >> 4][1] & eorx) ^ bgx;
				((u32 *) dest)[2] = (video_font_draw_table24[bits >> 4][2] & eorx) ^ bgx;
				((u32 *) dest)[3] = (video_font_draw_table24[bits & 15][0] & eorx) ^ bgx;
				((u32 *) dest)[4] = (video_font_draw_table24[bits & 15][1] & eorx) ^ bgx;
				((u32 *) dest)[5] = (video_font_draw_table24[bits & 15][2] & eorx) ^ bgx;
			}
			dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
			s++;
		}
		break;
	}
}

/*****************************************************************************/

static inline void video_drawstring (int xx, int yy, unsigned char *s)
{
	video_drawchars (xx, yy, s, strlen ((char *)s));
}

/*****************************************************************************/

static void video_putchar (int xx, int yy, unsigned char c)
{
	video_drawchars (xx, yy + video_logo_height, &c, 1);
}

/*****************************************************************************/
#if defined(CONFIG_CONSOLE_CURSOR) || defined(CONFIG_VIDEO_SW_CURSOR)
static void video_set_cursor (void)
{
	/* swap drawing colors */
	eorx = fgx;
	fgx = bgx;
	bgx = eorx;
	eorx = fgx ^ bgx;
	/* draw cursor */
	video_putchar (console_col * VIDEO_FONT_WIDTH,
		       console_row * VIDEO_FONT_HEIGHT,
		       ' ');
	/* restore drawing colors */
	eorx = fgx;
	fgx = bgx;
	bgx = eorx;
	eorx = fgx ^ bgx;
}
#endif
/*****************************************************************************/
#ifdef CONFIG_CONSOLE_CURSOR
void console_cursor (int state)
{
	static int last_state = 0;

#ifdef CONFIG_CONSOLE_TIME
	struct rtc_time tm;
	char info[16];

	/* time update only if cursor is on (faster scroll) */
	if (state) {
		rtc_get (&tm);

		sprintf (info, " %02d:%02d:%02d ", tm.tm_hour, tm.tm_min,
			 tm.tm_sec);
		video_drawstring (VIDEO_VISIBLE_COLS - 10 * VIDEO_FONT_WIDTH,
				  VIDEO_INFO_Y, (uchar *)info);

		sprintf (info, "%02d.%02d.%04d", tm.tm_mday, tm.tm_mon,
			 tm.tm_year);
		video_drawstring (VIDEO_VISIBLE_COLS - 10 * VIDEO_FONT_WIDTH,
				  VIDEO_INFO_Y + 1 * VIDEO_FONT_HEIGHT, (uchar *)info);
	}
#endif

	if (state && (last_state != state)) {
		video_set_cursor ();
	}

	if (!state && (last_state != state)) {
		/* clear cursor */
		video_putchar (console_col * VIDEO_FONT_WIDTH,
			       console_row * VIDEO_FONT_HEIGHT,
			       ' ');
	}

	last_state = state;
}
#endif

/*****************************************************************************/

#ifndef VIDEO_HW_RECTFILL
static void memsetl (int *p, int c, int v)
{
	while (c--)
		*(p++) = v;
}
#endif

/*****************************************************************************/

#ifndef VIDEO_HW_BITBLT
static void memcpyl (int *d, int *s, int c)
{
	while (c--)
		*(d++) = *(s++);
}
#endif

/*****************************************************************************/

static void console_scrollup (void)
{
	/* copy up rows ignoring the first one */

#ifdef VIDEO_HW_BITBLT
	video_hw_bitblt (VIDEO_PIXEL_SIZE,	/* bytes per pixel */
			 0,	/* source pos x */
			 video_logo_height + VIDEO_FONT_HEIGHT, /* source pos y */
			 0,	/* dest pos x */
			 video_logo_height,	/* dest pos y */
			 VIDEO_VISIBLE_COLS,	/* frame width */
			 VIDEO_VISIBLE_ROWS - video_logo_height - VIDEO_FONT_HEIGHT	/* frame height */
		);
#else
	memcpyl (CONSOLE_ROW_FIRST, CONSOLE_ROW_SECOND,
		 CONSOLE_SCROLL_SIZE >> 2);
#endif

	/* clear the last one */
#ifdef VIDEO_HW_RECTFILL
	video_hw_rectfill (VIDEO_PIXEL_SIZE,	/* bytes per pixel */
			   0,	/* dest pos x */
			   VIDEO_VISIBLE_ROWS - VIDEO_FONT_HEIGHT,	/* dest pos y */
			   VIDEO_VISIBLE_COLS,	/* frame width */
			   VIDEO_FONT_HEIGHT,	/* frame height */
			   CONSOLE_BG_COL	/* fill color */
		);
#else
	memsetl (CONSOLE_ROW_LAST, CONSOLE_ROW_SIZE >> 2, CONSOLE_BG_COL);
#endif
}

/*****************************************************************************/

static void console_back (void)
{
	CURSOR_OFF console_col--;

	if (console_col < 0) {
		console_col = CONSOLE_COLS - 1;
		console_row--;
		if (console_row < 0)
			console_row = 0;
	}
	video_putchar (console_col * VIDEO_FONT_WIDTH,
		       console_row * VIDEO_FONT_HEIGHT,
		       ' ');
}

/*****************************************************************************/

static void console_newline (void)
{
	/* Check if last character in the line was just drawn. If so, cursor was
	   overwriten and need not to be cleared. Cursor clearing without this
	   check causes overwriting the 1st character of the line if line lenght
	   is >= CONSOLE_COLS
	 */
	if (console_col < CONSOLE_COLS)
		CURSOR_OFF
	console_row++;
	console_col = 0;

	/* Check if we need to scroll the terminal */
	if (console_row >= CONSOLE_ROWS) {
		/* Scroll everything up */
		console_scrollup ();

		/* Decrement row number */
		console_row--;
	}
}

static void console_cr (void)
{
	CURSOR_OFF console_col = 0;
}

/*****************************************************************************/

void video_putc (const char c)
{
	static int nl = 1;

        // Jett: check newline here in order to 
        //       scroll the screen immediately for the first time video_printf()
        //
        if (console_col >= CONSOLE_COLS)
          console_newline ();

	switch (c) {
	case 13:		/* back to first column */
		console_cr ();
		break;

	case '\n':		/* next line */
		if (console_col || (!console_col && nl))
			console_newline ();
		nl = 1;
		break;

	case 9:		/* tab 8 */
		CURSOR_OFF console_col |= 0x0008;
		console_col &= ~0x0007;

		if (console_col >= CONSOLE_COLS)
			console_newline ();
		break;

	case 8:		/* backspace */
		console_back ();
		break;

	default:		/* draw the char */
		video_putchar (console_col * VIDEO_FONT_WIDTH,
			       console_row * VIDEO_FONT_HEIGHT,
			       c);
		console_col++;

		/* check for newline */
		if (console_col >= CONSOLE_COLS) {
			console_newline ();
			nl = 0;
		}
	}
CURSOR_SET}


/*****************************************************************************/

void video_puts (const char *s)
{
	int count = strlen (s);

	while (count--)
		video_putc (*s++);
	
	mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);	
}

/*****************************************************************************/

void video_printf (const char *fmt, ...)//copy from mtk u-boot-1.1.6
{
#ifdef CFB_CONSOLE_ON	
	va_list args;
	unsigned int i;
	char printbuffer[CONFIG_SYS_PBSIZE];

	va_start (args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf (printbuffer, fmt, args);
	va_end (args);

	/* Print the string */
	video_puts (printbuffer);
#else
  printf("%s\n", fmt);
#endif	
}

/*****************************************************************************/

#if defined(CONFIG_CMD_BMP) || defined(CONFIG_SPLASH_SCREEN)

#define FILL_8BIT_332RGB(r,g,b)	{			\
	*fb = ((r>>5)<<5) | ((g>>5)<<2) | (b>>6);	\
	fb ++;						\
}

#define FILL_15BIT_555RGB(r,g,b) {			\
	*(unsigned short *)fb = SWAP16((unsigned short)(((r>>3)<<10) | ((g>>3)<<5) | (b>>3))); \
	fb += 2;					\
}

#define FILL_16BIT_565RGB(r,g,b) {			\
	*(unsigned short *)fb = SWAP16((unsigned short)((((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3))); \
	fb += 2;					\
}

#define FILL_32BIT_X888RGB(r,g,b) {			\
	*(unsigned long *)fb = SWAP32((unsigned long)(((r<<16) | (g<<8) | b))); \
	fb += 4;					\
}

#ifdef VIDEO_FB_LITTLE_ENDIAN
#define FILL_24BIT_888RGB(r,g,b) {			\
	fb[0] = b;					\
	fb[1] = g;					\
	fb[2] = r;					\
	fb += 3;					\
}
#else
#define FILL_24BIT_888RGB(r,g,b) {			\
	fb[0] = r;					\
	fb[1] = g;					\
	fb[2] = b;					\
	fb += 3;					\
}
#endif
#endif

static int video_init (void)
{
	unsigned char color8;

	if ((pGD = video_hw_init ()) == NULL)
		return -1;

	video_fb_address = (void *) VIDEO_FB_ADRS;

	/* Init drawing pats */
	switch (VIDEO_DATA_FORMAT) {
	case GDF__8BIT_INDEX:
		video_set_lut (0x01, CONSOLE_FG_COL, CONSOLE_FG_COL, CONSOLE_FG_COL);
		video_set_lut (0x00, CONSOLE_BG_COL, CONSOLE_BG_COL, CONSOLE_BG_COL);
		fgx = 0x01010101;
		bgx = 0x00000000;
		break;
	case GDF__8BIT_332RGB:
		color8 = ((CONSOLE_FG_COL & 0xe0) |
			  ((CONSOLE_FG_COL >> 3) & 0x1c) | CONSOLE_FG_COL >> 6);
		fgx = (color8 << 24) | (color8 << 16) | (color8 << 8) | color8;
		color8 = ((CONSOLE_BG_COL & 0xe0) |
			  ((CONSOLE_BG_COL >> 3) & 0x1c) | CONSOLE_BG_COL >> 6);
		bgx = (color8 << 24) | (color8 << 16) | (color8 << 8) | color8;
		break;
	case GDF_15BIT_555RGB:
		fgx = (((CONSOLE_FG_COL >> 3) << 26) |
		       ((CONSOLE_FG_COL >> 3) << 21) | ((CONSOLE_FG_COL >> 3) << 16) |
		       ((CONSOLE_FG_COL >> 3) << 10) | ((CONSOLE_FG_COL >> 3) << 5) |
		       (CONSOLE_FG_COL >> 3));
		bgx = (((CONSOLE_BG_COL >> 3) << 26) |
		       ((CONSOLE_BG_COL >> 3) << 21) | ((CONSOLE_BG_COL >> 3) << 16) |
		       ((CONSOLE_BG_COL >> 3) << 10) | ((CONSOLE_BG_COL >> 3) << 5) |
		       (CONSOLE_BG_COL >> 3));
		break;
	case GDF_16BIT_565RGB:
		fgx = (((CONSOLE_FG_COL >> 3) << 27) |
		       ((CONSOLE_FG_COL >> 2) << 21) | ((CONSOLE_FG_COL >> 3) << 16) |
		       ((CONSOLE_FG_COL >> 3) << 11) | ((CONSOLE_FG_COL >> 2) << 5) |
		       (CONSOLE_FG_COL >> 3));
		bgx = (((CONSOLE_BG_COL >> 3) << 27) |
		       ((CONSOLE_BG_COL >> 2) << 21) | ((CONSOLE_BG_COL >> 3) << 16) |
		       ((CONSOLE_BG_COL >> 3) << 11) | ((CONSOLE_BG_COL >> 2) << 5) |
		       (CONSOLE_BG_COL >> 3));
		break;
	case GDF_32BIT_X888RGB:
		fgx = (CONSOLE_FG_COL << 16) | (CONSOLE_FG_COL << 8) | CONSOLE_FG_COL;
		bgx = (CONSOLE_BG_COL << 16) | (CONSOLE_BG_COL << 8) | CONSOLE_BG_COL;
		break;
	case GDF_24BIT_888RGB:
		fgx = (CONSOLE_FG_COL << 24) | (CONSOLE_FG_COL << 16) |
			(CONSOLE_FG_COL << 8) | CONSOLE_FG_COL;
		bgx = (CONSOLE_BG_COL << 24) | (CONSOLE_BG_COL << 16) |
			(CONSOLE_BG_COL << 8) | CONSOLE_BG_COL;
		break;
	}
	eorx = fgx ^ bgx;

	video_console_address = video_fb_address;

	/* Initialize the console */
	console_col = 0;
	console_row = 0;

	return 0;
}


/*****************************************************************************/

/*
 * Implement a weak default function for boards that optionally
 * need to skip the video initialization.
 */
int __board_video_skip(void)
{
	/* As default, don't skip test */
	return 0;
}
int board_video_skip(void) __attribute__((weak, alias("__board_video_skip")));

int drv_video_init (void)
{
	int skip_dev_init;
//	struct stdio_dev console_dev;

	/* Check if video initialization should be skipped */
	if (board_video_skip())
		return 0;

	/* Init video chip - returns with framebuffer cleared */
	skip_dev_init = (video_init () == -1);

#if 0   // Jett: don't register the vga device
#if !defined(CONFIG_VGA_AS_SINGLE_DEVICE)
	PRINTD ("KBD: Keyboard init ...\n");
	skip_dev_init |= (VIDEO_KBD_INIT_FCT == -1);
#endif

	if (skip_dev_init)
		return 0;

	/* Init vga device */
	memset (&console_dev, 0, sizeof (console_dev));
	strcpy (console_dev.name, "vga");
	console_dev.ext = DEV_EXT_VIDEO;	/* Video extensions */
	console_dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_SYSTEM;
	console_dev.putc = video_putc;	/* 'putc' function */
	console_dev.puts = video_puts;	/* 'puts' function */
	console_dev.tstc = NULL;	/* 'tstc' function */
	console_dev.getc = NULL;	/* 'getc' function */

#if !defined(CONFIG_VGA_AS_SINGLE_DEVICE)
	/* Also init console device */
	console_dev.flags |= DEV_FLAGS_INPUT;
	console_dev.tstc = VIDEO_TSTC_FCT;	/* 'tstc' function */
	console_dev.getc = VIDEO_GETC_FCT;	/* 'getc' function */
#endif /* CONFIG_VGA_AS_SINGLE_DEVICE */
#endif //don't register the vga device

  // Jett: set cursor to the right-bottom corner
  //       scroll screen immediately for the first time video_printf()
  //
  video_set_cursor(CONSOLE_ROWS - 1, CONSOLE_COLS);
  return 0;

	//if (stdio_register (&console_dev) != 0)
	//	return 0;

	/* Return success */
	//return 1;
}

void video_clean_screen(void)
{
    memset(VIDEO_FB_ADRS, 0, VIDEO_FB_SIZE);
}


void video_set_cursor(int row, int col)
{
    if (row >= 0 && row < CONSOLE_ROWS &&
        col >= 0 && col <= CONSOLE_COLS)
    {
        console_row = row;
        console_col = col;
    }
}


int video_get_rows(void)
{
    return CONSOLE_ROWS;
}


int video_get_colums(void)
{
    return CONSOLE_COLS;
}
