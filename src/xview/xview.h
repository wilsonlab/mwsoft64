#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<math.h>
#include	<X11/Xlib.h>

/* MWL Libs */
#include 	<header.h>

#include 	"xview_defs.h"
#include 	"xview_struct.h"

int	ncontours = 10;

short	cumulative;
char    fname[80],
		ch[80],
		valstr[20],
		timestr[20];
char	*commandfile;
int	commandsource;
char	*display;
Image	*image;
Image	*imageptr;
float	ys,lscale,xs,dscale;
short manual_xsize,manual_ysize;
short report_autoscale;
float
		xsize,
		ysize,
		zsize;
int     display_mode,
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
		autoscale = 1,
		command,
		recnum = 0,
		col = 0;

float   	count,
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
FILE	*fp;
int		debug;
int		backup;

short	representation;
short	title_mode;
char	title[MAXTITLES][100];
short	ntitles;
short	orient;
short	speed;
short	posneg;
char *geometry;
short header;
int signal_step;
int got_signal;
short color_mode;
short no_lines;

GlobalContext	*G;
ViewContext	*V;
