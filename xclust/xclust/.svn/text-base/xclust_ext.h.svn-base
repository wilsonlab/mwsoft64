#include 	<stdio.h>
#include 	<math.h>
#include 	<stdlib.h>
#include	<X11/Xlib.h>
#include	<X11/X.h>
#include	<X11/Xutil.h>
#include	<glob.h>
#include	<sys/wait.h>

#include 	"header.h"
#include 	"iolib.h"
#include 	"xclust_defs.h"
#include	"xclust_struct.h"


/************/
/* Globals  */
/************/


/* job id globals */
extern int current_job_id;
extern int continuous_step_job_id;
extern int blockmovie_job_id;
extern int intermittent_save_job_id;

extern int		debug;
extern Graph		*G;
extern Frame		*F;
extern XContext		datacontext;
extern char	*standardargv[];
extern int	standardargc;
extern int	rotatecolor;
extern int	suppresswarnings;

extern char		*CopyString();
extern Plot		*AddPlot();
extern Plot		*ClosePlot();
extern Plot		*SelectedPlot();
extern Plot		*PartialLoadPlot();
extern Label		*FindLabel();
extern Plot		*LoadPlotFromSource();
extern DataSource	*AddSource();
extern float		Atof();
extern ProjectionInfo	*GetProjectionInfo();
extern ProjectionInfo	*GetProjectionInfoByName();
extern ProjectionInfo	*AddProjectionInfo();
extern Graph		*GetGraph();
extern MenuWindow	*GetMenu();
extern MenuItem		*LookupMenuItem();
extern Label		*GetNamedLabel();
extern Label		*AddLabelString();
extern char		*Setenv();
extern ClusterList	*GetClusterInfo();
extern char		*GetItemValue();
extern char		*strchr();
extern Pixmap GetPixmapByName();
extern double GetDoubleFromRawData();
extern int	WhitePixelIdx();
extern int	BlackPixelIdx();
extern	unsigned long GetPixel();
