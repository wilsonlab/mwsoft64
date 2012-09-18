#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<math.h>
#include	<X11/Xlib.h>
#include	<X11/Xutil.h>

/* MWL libs */
#include 	<header.h>
#include 	<iolib.h>

#include 	"xview_defs.h"
#include	"xview_struct.h"

extern int32_t	ncontours;

extern	char    fname[80],
		ch[80],
		valstr[20],
		timestr[20];
char	*display;
extern char	*commandfile;
extern	int32_t	commandsource;
Image	*image, *imageptr;
extern	float	xs,lscale,ys,dscale;
extern int16_t manual_xsize,manual_ysize;
extern int16_t report_autoscale;
extern	float 	xsize,
		ysize,
		zsize;
extern	int32_t     display_mode,
		legend_base,
		xbase_size,
		ybase_size,
		zbase_size,
		waitval,
		indx,
		height,
		width,
		placed,
		cell,
		yb,
		show,
		mod,
		autoscale,
		command,
		recnum,
		col;
extern		float count,
		val,
		inc,
		maxval,
		minval,
		base,
		color_scale,
		color_min,
		scale,
	   *tmpdata,
	   *curdata,
	   *data;
extern	int16_t	cumulative;
extern	FILE	*fp;
extern	int32_t		debug;
extern	char	**global_envp;
extern	int32_t		backup;
extern	int16_t	representation;
extern  int16_t	title_mode;
extern  char	title[MAXTITLES][100];
extern	int16_t	ntitles;
extern	Display	*x_display;
extern	int16_t	orient;
extern	int16_t	speed;
extern	int16_t	posneg;
extern char *geometry;
extern int16_t header;
extern int32_t background;
extern int32_t foreground;
extern int16_t color_mode;
extern int16_t no_lines;
extern int32_t signal_step;
extern int32_t got_signal;

extern GlobalContext	*G;
extern ViewContext	*V;
