#include "xview_ext.h"
#include <signal.h>

error()
{
    fprintf(stderr,"\nusage: xview file [file ..][-a][-step][-wait #][-display_mode #]\n");
    fprintf(stderr,"\t[-maxmin # #][-scale #][-xs #][-ys #][-add]\n");
    fprintf(stderr,"\t[-plainformat xmax ymax dt datatype]\n");
    fprintf(stderr,"\t[-nomap][-rep 0/1][-debug][-signal]\n");
    fprintf(stderr,"\t[-gray][-rgray][-cscale #][-command str]\n");
    fprintf(stderr,"\t[-commandfile file]\n");
    fprintf(stderr,"\t[-noheader]\n");
    fprintf(stderr,"\t[-nolines]\n");
    fprintf(stderr,"\t[-cmap file][-ocmap file]\n");
    fprintf(stderr,"\t[-inverse]\n");
    fprintf(stderr,"\t[-bw]\n");
    fprintf(stderr,"\t[-getminmax]\n");
    fprintf(stderr,"\t[-nomap]\n");
    fprintf(stderr,"\t[-posdata]\n");
    exit (0);
}

ParseArgList(argc,argv)
int	argc;
char	**argv;
{
int		nxtarg;
int		filecnt;
extern	void	sigadvance(), sigtoggle();
char	*type;

    if (argc < 2)  {
	error();
    }
    nxtarg = 0;
    display = NULL;		/* use the environ DISPLAY */
    commandfile = NULL;
    signal(SIGUSR1,sigadvance);
    signal(SIGUSR2,sigtoggle);

    /* if autoscaling is off then assume nominal intracellular bound */
    ybase_size = xbase_size = 15;
    maxval = DEFAULTMAX;
    minval = DEFAULTMIN;
    autoscale = TRUE;
    report_autoscale = FALSE;
    waitval = 0;

    V->display_mode = COLORBOX;
    V->xmax = -1;
    V->ymax = -1;
    V->singlestep = TRUE;
    G->scale_type = SPECTRALSCALE;
    G->inverse = FALSE;
    V->plain = 0;
    V->posdata = 0;

    cumulative = FALSE;
    backup = FALSE;
    manual_xsize = 0;
    manual_ysize = 0;
    ys = xs = 1;
    representation = 0;
    title_mode = 0;
    speed = 0;
    ntitles=0;
    posneg = 0;
    geometry = NULL;
    header = 1;
    signal_step = 0;
    color_mode = 1;
    no_lines = 0;

    filecnt = 0;

    while( argc > ++nxtarg){
	if (arg_is("-version")){
	    fprintf(stderr,"%s : version %s : updated %s\n",
		argv[0],VERSION,DATE);

/* passed in from makefile when building an RPM*/
#ifdef MWSOFT_RPM_STRING 
	    fprintf(stderr,"From RPM: %s\n",
		    MWSOFT_RPM_STRING);
#endif

	    fprintf(stderr,"%s\n",COPYRIGHT);
	    exit(0);
	} else
	if (arg_is("-d")){
	    display = argv[++nxtarg];
	} else
	if (arg_is("-bw")){
	    color_mode = 0;
	} else
	if (arg_is("-nomap")){
	    G->mapped = FALSE;
	} else
	if (arg_is("-inverse")){
	    G->inverse = TRUE;
	} else
	if (arg_is("-command")){
	    commandfile = argv[++nxtarg];
	    commandsource = 1;
	} else
	if (arg_is("-posdata")){
	    V->posdata = 1;
	    autoscale = FALSE;
	    maxval = 1;
	    minval = 0;
	    scale = 1;
	    V->display_mode = POINT;
	} else
	if (arg_is("-commandfile")){
	    commandfile = argv[++nxtarg];
	    commandsource = 0;
	} else
	if (arg_is("-getminmax")){
	    report_autoscale = TRUE;
	} else
	if (arg_is("-rep")){
	    representation = iarg(); 
	} else
	if (arg_is("-xsize")){
	    manual_xsize =1 ;
	    xs = farg(); 
	} else
	if (arg_is("-ysize")){
	    manual_ysize =1 ;
	    ys =  farg();
	} else
	if (arg_is("-t")){
	    strcpy(title[ntitles++],argv[++nxtarg]);
	} else
	if (arg_is("-add")){
	    cumulative = TRUE;
	} else
	if (arg_is("-debug")){
	    debug = TRUE;
	} else
	if (arg_is("-signal")){
	    signal_step = TRUE;
	} else
	if (arg_is("-maxmin")){
	    autoscale = FALSE;
	    maxval = farg();
	    minval = farg();
	    if((scale = maxval - minval) == 0){
		scale = 1;
	    }
	} else
	if (arg_is("-speed")){
	    speed = iarg();
	} else
	if (arg_is("-zsize")){
	    zsize = farg();
	} else
	if (arg_is("-xmax")){
	    V->xmax = iarg();
	} else
	if (arg_is("-ymax")){
	    V->ymax = iarg();
	} else
	if (arg_is("-plainformat")){
	    V->xmax = iarg();
	    V->ymax = iarg();
	    V->dt = farg();
	    type = argv[++nxtarg];
	    if(strcmp(type,"float") == 0){
		V->datatype = FLOAT;
	    } else
	    if(strcmp(type,"char") == 0){
		V->datatype = CHAR;
	    } else
	    if(strcmp(type,"short") == 0){
		V->datatype = SHORT;
	    } else
	    if(strcmp(type,"int") == 0){
		V->datatype = INT;
	    } else
	    if(strcmp(type,"double") == 0){
		V->datatype = DOUBLE;
	    } else {
		fprintf(stderr,"invalid type specification\n");
		V->datatype = INVALID;
	    }
	    V->headersize = 0;
	    V->plain = 1;
	} else
	if (arg_is("-noheader")){
	    header = 0;
	} else
	if (arg_is("-nolines")){
	    no_lines = 1;
	} else
	if (arg_is("-scale")){
	    ybase_size = xbase_size = iarg();
	} else
	if (arg_is("-adjust")){
	    adjusttype(iarg());
	} else
	if (arg_is("-geometry")){
	    geometry = (char *)malloc(strlen(argv[++nxtarg]));
	    strcpy(geometry,argv[nxtarg]);
	} else
	if (arg_is("-cmap")){
	    if(ReadColorMap(argv[++nxtarg])){
		fprintf(stderr,"using colormap file\n");
		G->scale_type = FILESCALE;
	    }
	} else
	if (arg_is("-cscale")){
	    G->scale_type = iarg();
	} else
	if (arg_is("-gray")){
	    G->scale_type = GRAYSCALE;
	} else
	if (arg_is("-rgray")){
	    G->scale_type = RGRAYSCALE;
	} else
	if (arg_is("-mode")){
	    V->display_mode = iarg();
	} else
	if (arg_is("-box")){
	    V->display_mode = BOX;
	} else
	if (arg_is("-filledbox")){
	    V->display_mode = FILLEDBOX;
	} else
	if (arg_is("-colorbox")){
	    V->display_mode = COLORBOX;
	} else
	if (arg_is("-surface")){
	    V->display_mode = SURFACE;
	} else
	if (arg_is("-filledsurface")){
	    V->display_mode = FILLEDSURFACE;
	} else
	if (arg_is("-colorfilledsurface")){
	    V->display_mode = COLORFILLEDSURFACE;
	} else
	if(!arg_starts_with('-')){
	    strcpy(fname,argv[nxtarg]);
	    if((fp = fopen(fname,"r")) == NULL){
		fprintf(stderr,"xview: unable to open %s\n",fname);
	    } else {
		filecnt++;
	    }
	} else
	    error();
    }

    if(filecnt == 0) {
	printf("xview: must specify a file\n");
	exit(0);
    }
    V->filename = fname;
    if(ntitles == 0){
	strcpy(title[0],fname);
    }
    if(cumulative){
	strcat(title[0],"    <ADD>");
    }
    if(header == 0){
	strcat(title[0],"    <NH>");
    }
}
