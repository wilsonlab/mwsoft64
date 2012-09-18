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

extern int	ncontours;

extern	char    fname[80],
		ch[80],
		valstr[20],
		timestr[20];
char	*display;
extern char	*commandfile;
extern	int	commandsource;
Image	*image, *imageptr;
extern	float	xs,lscale,ys,dscale;
extern short manual_xsize,manual_ysize;
extern short report_autoscale;
extern	float 	xsize,
		ysize,
		zsize;
extern	int     display_mode,
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
extern	short	cumulative;
extern	FILE	*fp;
extern	int		debug;
extern	char	**global_envp;
extern	int		backup;
extern	short	representation;
extern  short	title_mode;
extern  char	title[MAXTITLES][100];
extern	short	ntitles;
extern	Display	*x_display;
extern	short	orient;
extern	short	speed;
extern	short	posneg;
extern char *geometry;
extern short header;
extern int background;
extern int foreground;
extern short color_mode;
extern short no_lines;
extern int signal_step;
extern int got_signal;

extern GlobalContext	*G;
extern ViewContext	*V;
