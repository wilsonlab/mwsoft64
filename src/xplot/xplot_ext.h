#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<math.h>
#include	<X11/Xlib.h>
#include	<X11/X.h>
#include	<X11/Xutil.h>
#include 	<header.h>
#include 	"xplot_defs.h"
#include	"xplot_struct.h"

extern int		debug;
extern Graph		*G;
extern char		*CopyString();
extern Plot		*AddPlot();
extern Plot		*SelectedPlot();
extern Frame		*F;
extern Label		*FindLabel();
extern XContext		datacontext;
