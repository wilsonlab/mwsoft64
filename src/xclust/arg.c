#include "xclust_ext.h"
#include <signal.h>
#include <X11/Xutil.h>

static char *commandstr;
static int commandsource;

char *CommandStr()
{
    return(commandstr);
}

int CommandSource()
{
    return(commandsource);
}

ClearCommands()
{
    commandstr = NULL;
}

ExecuteCommands(graph)
Graph	*graph;
{
Plot		*plot;
    /*
    ** execute command file or line if any
    */
    if(CommandStr()){
	if(CommandSource() == FROM_FILE){
	    plot = SelectedPlot(graph);
	    DoFileCommands(graph,&plot,CommandStr());
	} else {
	    plot = SelectedPlot(graph);
	    DoCommand(graph,&plot,CommandStr());
	}
    }
}


ParseArgList(frame,argc,argv)
Frame 	*frame;
int	argc;
char	**argv;
{
int	nxtarg;
int	filecnt;
DataSource	*source;
Graph	*graph;
int	x,y,width,height;
int	status;
int	defloadmode;

    source = NULL;
    frame->color_mode = 1;
    frame->fontname = NULL;
    frame->inverse = -1;
    frame->display_name = NULL;		/* use the environ DISPLAY */
    frame->x = -1;			/* use default geometry */
    frame->y = -1;
    frame->width = -1;
    frame->height = -1;
    /* scale_type and colormaptype not used anymore */
/*     frame->scale_type = SPECTRALSCALE; */
/*     frame->colormaptype = PRIVATE; */

    graph = frame->graph;
    graph->dragx1 = -1;
    graph->dragx2 = -1;
    graph->resolution = 1;		/* hi res postscript output */
    graph->currentprojection[0] = 0;	/* default data vector components */
    graph->currentprojection[1] = 1;	/* for x,y,[z/error] */
    graph->currentprojection[2] = 2;
    nxtarg = 0;

    graph->xaxis.autoscale = TRUE;
    graph->yaxis.autoscale = TRUE;
    graph->yaxis.title_offset = 0;
    graph->xaxis.title_offset = 0;
    graph->xaxis.ticklabel_offset = 0;
    graph->yaxis.ticklabel_offset = 0;

    graph->yaxis.tickinc = 0;		/* automatic tick interval calc */
    graph->xaxis.tickinc = 0;

    graph->yaxis.nsubticks = 0;	
    graph->xaxis.nsubticks = 0;

    graph->xaxis.show = TRUE;
    graph->yaxis.show = TRUE;

    graph->xaxis.desired_nticks = -60;	/* 60 pixel spacing between xticks */
    graph->yaxis.desired_nticks = -40;	/* 40 pixel spacing between yticks */

    graph->ticksize = 3;		/* 3 pixel ticks */

    graph->xaxis.show_labels = 1;		/* show all labels initially */
    graph->yaxis.show_labels = 1;
    graph->show_legend = TRUE;
    graph->show_title = TRUE;
    graph->showbounds = TRUE;
    graph->quadrants = 0xf;		/* display all quadrants */
    graph->gridcolor = MININTERFACECOLOR + GRIDCOLOR;

    filecnt = 0;
    graph->xaxis.title = "";
    graph->yaxis.title = "";

    graph->xaxis.leftdp = 4;
    graph->yaxis.leftdp = 4;

    graph->xaxis.rightdp = -1;		/* use the %g formatting */
    graph->yaxis.rightdp = -1;

    graph->title = NULL;
    graph->optimize = 1;		/* dont draw plots that are out of 
					view of the window */
    graph->optimizepoints = 0;		/* dont draw overlapping points */
    graph->stepthrough = NULL;		/* single step through plots */

    graph->thetax = 0;			/* set 3d rotation angles */
    graph->thetay = 0;
    graph->thetaz = 0;
    graph->matrix = NULL;		/* turn off rotational transformation */
    graph->disparityscale = 0;		
    commandstr = NULL;
    defloadmode = PARTIAL_LOAD;
    suppresswarnings = 0;

    while( argc > ++nxtarg){
	if (arg_is("-usage") || arg_is("-help")){
	    fprintf(stderr,
	    "usage: %s file [file ..][-title name][-display name]\n",
	    argv[0]);
	    fprintf(stderr,
	    "\t[-bw][-inverse][-font name][-geometry str]\n");
	    fprintf(stderr,
	    "\t[-xlh # #][-ylh # #][-debug][-cscale #]\n");
	    fprintf(stderr,
	    "\t[-xtitle name][-ytitle name][-nomenu]\n");
	    fprintf(stderr,
	    "\t[-command str][-commandfile name]\n");
	    fprintf(stderr,
	    "\t[-restore graphfile][-fullload]\n");
	    fprintf(stderr,
	    "\t[-cmap file][-ocmap file][-partialload file start end]\n");
	    exit(0);
	} else
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
	if (arg_is("-display")){
	    frame->display_name = argv[++nxtarg];
/* 	} else */
/* 	if (arg_is("-cmap")){ */
/* 	    if(ReadColorMap(argv[++nxtarg])){ */
/* 		frame->scale_type = FILESCALE; */
/* 	    } */
	} else
	if (arg_is("-fullload")){
	    defloadmode = FULL_LOAD;
	} else
	if (arg_is("-nomenu")){
	    frame->menucontainer->mapped = 0;
/* 	} else */
/* 	if (arg_is("-cmaptype")){ */
/* 	    ++nxtarg; */
/* 	    if(strcmp(argv[nxtarg],"default") == 0){ */
/* 		frame->colormaptype = DEFAULT; */
/* 	    } else */
/* 	    if(strcmp(argv[nxtarg],"private") == 0){ */
/* 		frame->colormaptype = PRIVATE; */
/* 	    } else */
/* 	    if(strcmp(argv[nxtarg],"shared") == 0){ */
/* 		frame->colormaptype = SHARED; */
/* 	    } */
/* 	} else */
/* 	if (arg_is("-ocmap")){ */
/* 	    WriteColorMap(argv[++nxtarg]); */
/* 	    fprintf(stderr,"Done\n"); */
/* 	    exit(0); */
	} else
	if (arg_is("-restore")){
	    SetRestoreFile(argv[++nxtarg]);
	} else
	if (arg_is("-commandfile")){
	    commandstr = argv[++nxtarg];
	    commandsource = FROM_FILE;
	} else
	if (arg_is("-command")){
	    commandstr = argv[++nxtarg];
	    commandsource = FROM_OTHER;
	} else
	if (arg_is("-suppresswarnings")){
	    suppresswarnings = 1;
	} else
	if (arg_is("-bw")){
	    frame->color_mode = 0;
/* 	} else */
/* 	if (arg_is("-cscale")){ */
/* 	    frame->scale_type = atoi(argv[++nxtarg]); */
	} else
	if (arg_is("-inverse")){
	    frame->inverse = 1;
	} else
	if (arg_is("-title")){
	    graph->title = argv[++nxtarg];
	} else
	if (arg_is("-font")){
	    frame->fontname = argv[++nxtarg];
	} else
	if (arg_is("-xtitle")){
	    graph->xaxis.title = argv[++nxtarg];
	} else
	if (arg_is("-ytitle")){
	    graph->yaxis.title = argv[++nxtarg];
	} else
	if (arg_is("-nomap")){
	    frame->mapped = 0;
	} else
	if (arg_is("-debug")){
	    debug = TRUE;
	} else
	if (arg_is("-xlh")){
	    graph->xaxis.autoscale = FALSE;
	    graph->xmin = farg();
	    graph->xmax = farg();
	} else
	if (arg_is("-ylh")){
	    graph->yaxis.autoscale = FALSE;
	    graph->ymin = farg();
	    graph->ymax = farg();
	} else
	if (arg_is("-geometry")){
	    status = XParseGeometry(argv[++nxtarg],&x,&y,&width,&height);
	    if(status & XValue){
		frame->x = x;
	    }
	    if(status & YValue){
		frame->y = y;
	    }
	    if(status & WidthValue){
		frame->width = width;
		/*frame->wwidth = width;*/
	    }
	    if(status & HeightValue){
		frame->height = height;
		/*frame->wheight = height;*/
	    }
	} else
	if (arg_is("-partialplotload") ||
	    arg_is("-ppl")){
	    source = AddSource(graph);
	    source->type = FROM_FILE;
	    source->filename = argv[++nxtarg];
	    source->loadmode = PARTIAL_PLOT_LOAD;
	    source->startplot = iarg();
	    source->endplot = iarg();
	    source->startline = -1;
	    source->endline = -1;
	    filecnt++;
	} else
	if (arg_is("-partialload") ||
	    arg_is("-pl")){
	    source = AddSource(graph);
	    source->type = FROM_FILE;
	    source->filename = argv[++nxtarg];
	    source->loadmode = PARTIAL_LOAD;
	    source->startline = iarg();
	    source->endline = iarg();
	    filecnt++;
	} else
	if(!arg_starts_with('-')){
	    source = AddSource(graph);
	    source->loadmode = defloadmode;
	    source->type = FROM_FILE;
	    source->filename = argv[nxtarg];
	    switch(source->loadmode){
	    case FULL_LOAD:
		break;
	    case PARTIAL_LOAD:
		source->startline = 0;
		source->endline = -1;
		break;
	    }
	    filecnt++;
	} else {
	    fprintf(stderr,"%s: unknown option '%s'\n",argv[0],argv[nxtarg]);
	}
    }

    if(filecnt == 0) {
	/*
	** use stdin
	*/
	source = AddSource(graph);
	source->filename = "STDIN";
	source->type = FROM_FILE;
    }
}

