#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<math.h>
#include	<X11/Xlib.h>

/* MWL Libs */
#include 	<header.h>

#include 	"xview_defs.h"
#include 	"xview_struct.h"

int32_t	ncontours = 10;

int16_t	cumulative;
char    fname[80],
		ch[80],
		valstr[20],
		timestr[20];
char	*commandfile;
int32_t	commandsource;
char	*display;
Image	*image;
Image	*imageptr;
float	ys,lscale,xs,dscale;
int16_t manual_xsize,manual_ysize;
int16_t report_autoscale;
float
		xsize,
		ysize,
		zsize;
int32_t     display_mode,
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
int32_t		debug;
int32_t		backup;

int16_t	representation;
int16_t	title_mode;
char	title[MAXTITLES][100];
int16_t	ntitles;
int16_t	orient;
int16_t	speed;
int16_t	posneg;
char *geometry;
int16_t header;
int32_t signal_step;
int32_t got_signal;
int16_t color_mode;
int16_t no_lines;

GlobalContext	*G;
ViewContext	*V;
