#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "xplot_ext.h"

char	numstr[200];
char	labelstr[200];
char	tmpstr[200];
int	plot_select = 0;
int	command_entry = 0;
int	first_char = 1;
int	labelsx,labelsy,labelwx,labelwy;
short	labelcoord_mode = WINDOW_LBL;
static float plot_scale = 0.75;
static int box = FALSE;
static int header = TRUE;
int line_width = 1;
static escape_mode = 0;
static axis_mode = XMODE;
static global_operation=0;
static float shiftpercent = 0.05;
static float rotpercent = 0.05;
int shift_signal;


#define FORALLPLOTS for(plot=graph->plot;plot;plot=plot->next)
#define SQR(x)		((x)*(x))

void
sigshift(sig)
int sig;
{
   /* shift_signal = 1;*/
	RightShift();
	XFlush(G->display);
	signal(SIGUSR1,sigshift);
}

SetHeader(mode)
int mode;
{
    header = mode;
}

AutoBarWidth(plot,percent)
Plot	*plot;
float	percent;
{
    /*
    ** set bar width to a fraction of the distance between the first
    ** two points
    */
    if(plot->npoints > 2){
	plot->bar_width = plot->xscale*percent*fabs(plot->data[0].x - plot->data[1].x);
    } else {
	plot->bar_width = 0;
    }
}

ZapLabel(graph,event)
Graph	*graph;
XKeyEvent	*event;
{
Label	*label;
    label = FindLabel(graph,event->x,event->y);
    DeleteLabel(graph,label);
    RefreshGraph(graph);
}

RightShift()
{
float	val;
    /*
    ** right shift plot by a percentage of the window bounds
    */
    val = (G->wxmax - G->wxmin)*shiftpercent;
    G->wxmax += val;
    G->wxmin += val;
    ScaleAndRefreshGraph(G);
}

LeftShift()
{
float	val;
    /*
    ** left shift plot by a percentage of the window bounds
    */
    val = (G->wxmax - G->wxmin)*shiftpercent;
    G->wxmax -= val;
    G->wxmin -= val;
    ScaleAndRefreshGraph(G);
}

SetAxisMode(mode)
int mode;
{
    axis_mode = mode;
}

TogglePinchmode()
{
    if(ButtonMode() == PINCHMODE){
	NormalSelectInputs(G);
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	ClickSelectInputs(G);
	SetButtonMode(PINCHMODE);
	HandCursor(F);
    }
}

TogglePolyLineMode()
{
    if(ButtonMode() == POLYLINEMODE){
	NormalSelectInputs(G);
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	ClickSelectInputs(G);
	SetButtonMode(POLYLINEMODE);
	CrosshairCursor(F);
    }
}

ToggleDrawmode()
{
    if(ButtonMode() == DRAWMODE){
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	SetButtonMode(DRAWMODE);
	CrosshairCursor(F);
    }
}

ToggleZapmode()
{
    if(ButtonMode() == ZAPMODE){
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	SetButtonMode(ZAPMODE);
	TargetCursor(F);
    }
}

ShiftPlotList(graph)
Graph *graph;
{
Plot	*nextplot;
Plot	*headplot;
Plot	*plot;

    headplot = NULL;
    for(plot=graph->plot;plot;plot=plot->next){
	/*
	** swap
	*/
	if(plot->selected){
	    nextplot = plot->next;
	    if(nextplot == NULL){
		/*
		** end of the list
		*/
		if(headplot == NULL) return;	/* only entry in the list */
		nextplot = graph->plot;
		graph->plot = plot;
		plot->next = nextplot;
		headplot->next = NULL;
	    } else { 
		if(headplot == NULL){
		    /*
		    ** beginning of list
		    */
		    graph->plot = nextplot;
		} else {
		    headplot->next = nextplot;
		}
		plot->next = nextplot->next;
		nextplot->next = plot;
	    }
	    return;
	}
	headplot = plot;
    }
}

SetShiftPercent(val)
float val;
{
    shiftpercent = val;
}

PrintToFile(graph,fname,mode)
Graph *graph;
char *fname;
char *mode;
{
    /*
    ** create a postscript file
    */
    SetPSFileOutput(1);
    SetPSFilename(fname);
    SetPSFilemode(mode);
    PreparePS(graph->display,graph->window,plot_scale,box,header,
    graph->wwidth,graph->wheight);
    RefreshGraph(graph);
    FinishPS();
    SetPSFileOutput(0);
}

PrintOut(graph)
Graph *graph;
{
    PreparePS(graph->display,graph->window,plot_scale,box,header,
    graph->wwidth,graph->wheight);
    RefreshGraph(graph);
    FinishPS();
}

SetBox(mode)
int mode;
{
    box = mode;
}

SelectPlot(graph,ival)
Graph *graph;
int ival;
{
int	cnt;
Plot	*plot;

    /*
    ** clear previous selections
    */
    for(plot=graph->plot;plot;plot=plot->next){
	plot->selected = 0;
    }
    /*
    ** mark the new selection
    */
    cnt = 0;
    for(plot=graph->plot;plot;plot=plot->next){
	if(cnt == ival){
	    plot->selected = 1;
	    break;
	}
	if(ival < 0 && plot->next == NULL){		/* use the last plot */
	    plot->selected = 1;
	    break;
	}
	cnt++;
    }
    /*
    ** default to the first plot if an invalid selection 
    ** was made. This also gives wraparound if autoselection
    ** is used.
    */
    if(plot == NULL){
	graph->plot->selected = 1;
    }
    /*
    ** update the plot legends
    */
    cnt = 0;
    for(plot=graph->plot;plot;plot=plot->next){
	DrawPlotLegend(plot,cnt);
	cnt++;
    }
}

OffsetPlotGraphically(graph,cx,cy,val)
Graph *graph;
int	cx,cy;
float	val;
{
Plot	*plot;
int	closest;
float	distance;
int	i;
float	d;

    /*
    ** find the points closest to the current cursor
    */
    for(plot=graph->plot;plot;plot=plot->next){
	distance = 0;
	closest = 0;
	for(i=0;i<plot->npoints;i++){
	    d = abs(cx - plot->coord[i].x);
	    if(i == 0 || d < distance ){
		distance = d;
		closest = i;
	    }
	}
	/*
	** offset the plot by the data value
	*/
	plot->yoffset_axis = val - plot->data[closest].y;
    }
    ScaleAndRefreshGraph(graph);
}

SelectPlotGraphically(graph,cx,cy)
Graph *graph;
int	cx,cy;
{
int	cnt;
Plot	*plot;
Plot	*closest;
float	distance;
int	i;
float	d;

    /*
    ** clear previous selections
    */
    for(plot=graph->plot;plot;plot=plot->next){
	plot->selected = 0;
    }
    /*
    ** find the visible point closest to the current cursor
    */
    closest = NULL;
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->visible)
	for(i=0;i<plot->npoints;i++){
	    d = SQR(cx - plot->coord[i].x) + 
		SQR(cy - plot->coord[i].y);
	    if(closest == NULL || d < distance ){
		distance = d;
		closest = plot;
	    }
	}
    }
    /*
    ** mark the new selection
    */
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot == closest){
	    plot->selected = 1;
	    break;
	}
    }
    /*
    ** default to the first plot if an invalid selection 
    ** was made. This also gives wraparound if autoselection
    ** is used.
    */
    if(plot == NULL){
	fprintf(stderr,"");
    }
    /*
    ** update the plot legends
    */
    cnt = 0;
    for(plot=graph->plot;plot;plot=plot->next){
	DrawPlotLegend(plot,cnt);
	cnt++;
    }
}

Plot *SelectedPlot(graph)
Graph *graph;
{
Plot *plot;

    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->selected){
	    return(plot);
	}
    }
    return(NULL);
}

GlobalOperation()
{
    return(global_operation);
}

ResetGlobalOperation()
{
    global_operation = 0;
}

SetGlobalOperation()
{
    global_operation = 1;
}

long	fsize(name)
char	*name;
{
struct stat stbuf;

    if(stat(name,&stbuf) == -1) {
	fprintf(stderr,"fsize: can't find %s\n",name);
	return;
    }
    if((stbuf.st_mode & S_IFMT) == S_IFDIR) {
	fprintf(stderr,"fsize: %s is a directory\n",name);
	return;
    }
    return(stbuf.st_size);
}

ProcessSavedView(graph, mode, index)
Graph	*graph;
int	mode;
int	index;
{
char	tmpstr[80];

    if(mode){
	/* save view */
	graph->savedview[index].wxmin = graph->wxmin;
	graph->savedview[index].wymin = graph->wymin;
	graph->savedview[index].wxmax = graph->wxmax;
	graph->savedview[index].wymax = graph->wymax;
	if(index > 0){
	sprintf(tmpstr,"Current view saved in F%d              ",index);
	/* SetColor(graph->foreground); */
	Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);
	}
    } else {
	if(
	(graph->savedview[index].wxmin == 0) &&
	(graph->savedview[index].wymin == 0) &&
	(graph->savedview[index].wxmax == 0) &&
	(graph->savedview[index].wymax == 0)
	){
	    sprintf(tmpstr,"No view saved in F%d              ",index);
	    /* SetColor(graph->foreground); */
	    Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);
	    return;
	}
	/* restore stored view */
	graph->wxmin = graph->savedview[index].wxmin;
	graph->wymin = graph->savedview[index].wymin;
	graph->wxmax = graph->savedview[index].wxmax;
	graph->wymax = graph->savedview[index].wymax;
	ScaleAndRefreshGraph(graph);
	sprintf(tmpstr,"Restored view from F%d              ",index);
	/* SetColor(graph->foreground); */
	Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);
    }
}


KeyAction(graph,event)
Graph		*graph;
XKeyEvent	*event;
{
char	buffer[100];
int	nbytes;
char	c;
KeySym	key;
float	time;
float	val;
int	ival;
int	cnt;
Plot	*plot;
float	wx,wy;
Label	*label;
int	i;

    buffer[0] = '\0';
    /* 
    ** do key mapping to determine the actual key pressed
    */
    XLookupString(event,buffer,100, &key, NULL);
    if((c = *buffer) == '\0'){
	/*
	** check the key code for special keys
	*/
	if(debug){
	    printf("%s\n",XKeysymToString(key));
	}
	/*
	** carry out special function key operations
	*/
	if(strcmp(XKeysymToString(key),"F1")==0){
	    ProcessSavedView(graph,escape_mode,1);
	    escape_mode = 0;
	} else
	if(strcmp(XKeysymToString(key),"F2")==0){
	    ProcessSavedView(graph,escape_mode,2);
	    escape_mode = 0;
	} else
	if(strcmp(XKeysymToString(key),"F3")==0){
	    ProcessSavedView(graph,escape_mode,3);
	    escape_mode = 0;
	} else
	if(strcmp(XKeysymToString(key),"F4")==0){
	    ProcessSavedView(graph,escape_mode,4);
	    escape_mode = 0;
	} else
	if(strcmp(XKeysymToString(key),"F5")==0){
	    ProcessSavedView(graph,escape_mode,5);
	    escape_mode = 0;
	} else
	if(strcmp(XKeysymToString(key),"F6")==0){
	    ProcessSavedView(graph,escape_mode,6);
	    escape_mode = 0;
	} else
	if(strcmp(XKeysymToString(key),"F7")==0){
	    ProcessSavedView(graph,escape_mode,7);
	    escape_mode = 0;
	} else
	if(strcmp(XKeysymToString(key),"F8")==0){
	    ProcessSavedView(graph,escape_mode,8);
	    escape_mode = 0;
	} else
	if(strcmp(XKeysymToString(key),"F9")==0){
	    ProcessSavedView(graph,escape_mode,9);
	    escape_mode = 0;
	} else
	if(strcmp(XKeysymToString(key),"F10")==0){
	    ProcessSavedView(graph,escape_mode,10);
	    escape_mode = 0;
	} else
	if(strcmp(XKeysymToString(key),"F12")==0){
	    ProcessSavedView(graph,escape_mode,11);
	    escape_mode = 0;
	} 
	return;
    }
    if(command_entry){
	if(c == '\r'){
	    ClearWindow(graph->frame->text);
	    if(global_operation){
		FORALLPLOTS{
		    InterpretCommand(graph,&plot,numstr);
		}
	    } else
	    plot = SelectedPlot(graph);
	    if(InterpretCommand(graph,&plot,numstr) == 0){
		RefreshGraph(graph);
	    }
	    command_entry = 0;
	    numstr[0] = '\0';
	} else
	if(c == ''){
	    if(strlen(numstr) > 0){
		numstr[strlen(numstr)-1] = '\0';
		SetColor(graph->frame->text->foreground);
		sprintf(tmpstr,"%s ",numstr);
		Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);
	    }
	} else {
	    SetColor(graph->frame->text->foreground);
	    sprintf(tmpstr,"%s%c ",numstr,c);
	    Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);
	    sprintf(numstr,"%s%c",numstr,c);
	}
	return;
    }
    /*
    ** see if in raw text entry mode
    */
    if(ButtonMode() == TEXTMODE){
	if(c == '\r'){
	    NormalCursor(F);
	    if(labelcoord_mode == WORLD_LBL){
		WorldTransform(graph,labelsx,labelsy,&wx,&wy);
	    }
	    AddLabelString(graph,labelstr,labelsx,labelsy,
	    wx,wy, labelcoord_mode,PERMANENT_LBL,graph->fontname);
	    sprintf(tmpstr,"%s ",labelstr);
	    SetColor(graph->foreground);
	    Text(graph,labelsx,labelsy,tmpstr);
	    SetButtonMode(DATAMODE);
	    first_char = 1;
	    labelstr[0] = '\0';
	    /* RefreshGraph(graph); */
	    NormalSelectInputs(graph);
	} else
	if(c == ''){
	    if(labelcoord_mode == WORLD_LBL){
		WorldTransform(graph,labelsx,labelsy,&wx,&wy);
	    }
	    AddLabelString(graph,labelstr,labelsx,labelsy,
	    wx,wy, labelcoord_mode,PERMANENT_LBL,graph->fontname);
	    SetColor(graph->foreground);
	    sprintf(tmpstr,"%s ",labelstr);
	    Text(graph,labelsx,labelsy,tmpstr);

	    labelsy += graph->fontheight;
	    labelstr[0] = '\0';
	    SetColor(graph->foreground);
	    sprintf(tmpstr,"%s",labelstr);
	    Text(graph,labelsx,labelsy,tmpstr);
	} else
	if(c == ''){
	    if(strlen(labelstr) > 0){
		labelstr[strlen(labelstr)-1] = '\0';
		SetColor(graph->foreground);
		sprintf(tmpstr,"%s ",labelstr);
		Text(graph,labelsx,labelsy,tmpstr);
	    }
	} else {
	    if(first_char){
		labelsx = event->x;
		labelsy = event->y;
		first_char = 0;
		labelstr[0] = '\0';
	    }
	    sprintf(labelstr,"%s%c",labelstr,c);
	    SetColor(graph->foreground);
	    sprintf(tmpstr,"%s",labelstr);
	    Text(graph,labelsx,labelsy,tmpstr);
	}
	return;
    }
    if(c == ''){
	if(strlen(numstr) > 0){
	    numstr[strlen(numstr)-1] = '\0';
	    SetColor(graph->frame->text->foreground);
	    sprintf(tmpstr,"%s ",numstr);
	    Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);
	}
    } else {
	SetColor(graph->frame->text->foreground);
	sprintf(tmpstr,"%s%c ",numstr,c);
	Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);
    }
    /*
    ** check for numeric strings
    */
    if((c >= '0' && c <='9') || (c == '.') || (c == '-') || (c =='e')){
	sprintf(numstr,"%s%c",numstr,c);
    } else{
	/*
	** ESCAPE PREFIXED KEYS
	*/
	if(escape_mode){
	    switch(c){
	    case '': 
		FreeAllLabels(graph);
		RefreshGraph(graph);
		break;
	    case '\t':
		/*
		** toggle snapmode in the ruler line
		*/
		ToggleSnapMode();
		break;
	    case '=':
		/*
		** select the offset of the currently selected plot
		*/
		if(strlen(numstr) > 0){
		    if(global_operation){
			FORALLPLOTS{
			    if(axis_mode == XMODE){
				plot->xoffset_axis = atof(numstr);
			    } else
			    if(axis_mode == YMODE){
				plot->yoffset_axis = atof(numstr);
			    }
			}
		    } else
		    if(plot = SelectedPlot(graph)){
			if(axis_mode == XMODE){
			    plot->xoffset_axis = atof(numstr);
			} else
			if(axis_mode == YMODE){
			    plot->yoffset_axis = atof(numstr);
			}
		    }
		}
		numstr[0]='\0';
		ScaleAndRefreshGraph(graph);
		break;
	    case 'A':
		if(strlen(numstr) > 0){
		    if(axis_mode == XMODE){
			graph->xaxis.title_offset = atoi(numstr);
		    } else
		    if(axis_mode == YMODE){
			graph->yaxis.title_offset = atoi(numstr);
		    }
		}
		numstr[0]='\0';
		break;
	    case 'B':
		/*
		** toggle bar_graph fill mode 
		*/
		if(global_operation){
		    FORALLPLOTS{
			plot->plot_type = BAR_GRAPH;
			plot->bar_filled = !plot->bar_filled;
		    }
		} else
		if(plot = SelectedPlot(graph)){
		    plot->plot_type = BAR_GRAPH;
		    plot->bar_filled = !plot->bar_filled;
		}
		RefreshGraph(graph);
		break;
	    case 'D':
		ToggleDrawmode();
		break;
	    case 'L':
		/*
		** set the line_width
		*/
		if(strlen(numstr) > 0)
		    line_width = atoi(numstr);
		else
		    line_width = 1;
		numstr[0]='\0';
		break;
	    case 'M':
		/*
		** toggle the menu
		*/
		if(F->menu->mapped){
		    MapMenu(0);
		} else {
		    MapMenu(1);
		}
		break;
	    case 'O':
		AutoOrigin(graph);
		RefreshGraph(graph);
		break;
	    case 'P':
		TogglePolyLineMode();
		break;
	    case 'R':
		/*
		** fast raster specification 
		*/
		if(strlen(numstr) > 0)
		    val = atof(numstr);
		else
		    val = 0;
		numstr[0]='\0';
		ival = 0;
		for(plot=graph->plot;plot;plot=plot->next){
		    if(axis_mode == XMODE){
			plot->xoffset_axis = val*ival;
		    } else
		    if(axis_mode == YMODE){
			plot->yoffset_axis = val*ival;
		    }
		    ival++;
		}
		numstr[0]='\0';
		ScaleAndRefreshGraph(graph);
		break;
	    case 'T':
		if(strlen(numstr) > 0){
		    if(axis_mode == XMODE){
			graph->xaxis.ticklabel_offset = atoi(numstr);
		    } else
		    if(axis_mode == YMODE){
			graph->yaxis.ticklabel_offset = atoi(numstr);
		    }
		}
		numstr[0]='\0';
		break;
	    case 'a':
		/*
		** toggle axis display
		*/
		graph->xaxis.show = !graph->xaxis.show;
		graph->yaxis.show = !graph->yaxis.show;
		RefreshGraph(graph);
		break;
	    case 'b':
		if(strlen(numstr) > 0){
		    /*
		    ** set bar_graph display mode and bar width
		    */
		    if(global_operation){
			FORALLPLOTS{
			    plot->plot_type = BAR_GRAPH;
			    plot->bar_width = atof(numstr);
			}
		    } else
		    if(plot = SelectedPlot(graph)){
			plot->plot_type = BAR_GRAPH;
			plot->bar_width = atof(numstr);
		    }
		}  else {
		    /*
		    ** autosize the bar to the maximum
		    */
		    if(global_operation){
			FORALLPLOTS{
			    plot->plot_type = BAR_GRAPH;
			    AutoBarWidth(plot,1.0);
			}
		    } else
		    if(plot = SelectedPlot(graph)){
			plot->plot_type = BAR_GRAPH;
			AutoBarWidth(plot,1.0);
		    }
		}
		numstr[0]='\0';
		RefreshGraph(graph);
		break;
	    case 'c':
		/*
		** select the linestyle/color of the currently selected plot
		*/
		if(strlen(numstr) > 0){
		    if(plot = SelectedPlot(graph)){
			plot->linestyle = atoi(numstr);
		    }
		}
		numstr[0]='\0';
		RefreshGraph(graph);
		break;
	    case 'd':
		/*
		** toggle error bars
		*/
		if(global_operation){
		    FORALLPLOTS{
			plot->show_error = !plot->show_error;
		    }
		} else
		if(plot = SelectedPlot(graph)){
		    plot->show_error = !plot->show_error;
		}
		RefreshGraph(graph);
		break;
	    case 'f':
		/*
		** set the tick label precision 
		*/
		if(strlen(numstr) > 0){
		    if(axis_mode == XMODE){
			graph->xaxis.rightdp = atoi(numstr);
		    } else
		    if(axis_mode == YMODE){
			graph->yaxis.rightdp = atoi(numstr);
		    }
		}
		numstr[0]='\0';
		RefreshGraph(graph);
		break;
	    case 'i':
		/*
		** toggle inverse video
		*/
		graph->inverse = !graph->inverse;
		SetInverse(graph);
		RefreshGraph(graph);
		break;
	    case 'l':
		/*
		** set the axis style to log/linear
		*/
		if(strlen(numstr) > 0){
		    if(axis_mode == XMODE){
			graph->xaxis.type = atoi(numstr);
		    } else
		    if(axis_mode == YMODE){
			graph->yaxis.type = atoi(numstr);
		    }
		}
		numstr[0]='\0';
		ScaleAndRefreshGraph(graph);
		break;
	    case 'm':
		/*
		** set the plot type
		*/
		if(strlen(numstr) > 0){
		    if(global_operation){
			FORALLPLOTS{
			    plot->plot_type = atoi(numstr);
			}
		    } else
		    if(plot = SelectedPlot(graph)){
			plot->plot_type = atoi(numstr);
		    }
		} else {
		    if(plot = SelectedPlot(graph)){
			plot->plot_type = NORMAL_GRAPH;
		    }
		}
		numstr[0]='\0';
		RefreshGraph(graph);
		break;
	    case 'o':
		/*
		** set automatic axis placement mode
		*/
		if(axis_mode == YMODE){
		    graph->auto_yaxis = atoi(numstr);
		} else
		if(axis_mode == XMODE){
		    graph->auto_xaxis = atoi(numstr);
		}
		RefreshGraph(graph);
		break;
	    case 'p':
		/*
		** set the point style
		*/
		if(global_operation){
		    FORALLPLOTS{
			if(strlen(numstr) > 0){
			    plot->point_symbol = atoi(numstr);
			} else {
			    plot->point_symbol = BOX_PT;
			}
		    }
		}
		if(strlen(numstr) > 0){
		    if(plot = SelectedPlot(graph)){
			plot->point_symbol = atoi(numstr);
		    }
		} else {
		    if(plot = SelectedPlot(graph)){
			plot->point_symbol = BOX_PT;
		    }
		}
		numstr[0]='\0';
		RefreshGraph(graph);
		break;
	    case 'r':
		/*
		** toggle the showing of points
		*/
		if(global_operation){
		    FORALLPLOTS{
			plot->showpoints = !plot->showpoints;
		    }
		} else
		if(plot = SelectedPlot(graph)){
		    plot->showpoints = !plot->showpoints;
		}
		RefreshGraph(graph);
		break;
	    case 's':
		/*
		** select the scale of the currently selected plot
		*/
		if(strlen(numstr) > 0){
		    if(global_operation){
			FORALLPLOTS{
			    if(axis_mode == YMODE){
				plot->scale = atof(numstr);
			    } else
			    if(axis_mode == XMODE){
				plot->xscale = atof(numstr);
			    }
			}
		    } else
		    if(plot = SelectedPlot(graph)){
			if(axis_mode == YMODE){
			    plot->scale = atof(numstr);
			} else
			if(axis_mode == XMODE){
			    plot->xscale = atof(numstr);
			}
		    }
		}
		numstr[0]='\0';
		ScaleAndRefreshGraph(graph);
		break;
	    case 't':
		/*
		** set the number of ticks
		*/
		if(strlen(numstr) > 0){
		    if(axis_mode == XMODE){
			graph->xaxis.desired_nticks = atoi(numstr);
		    } else
		    if(axis_mode == YMODE){
			graph->yaxis.desired_nticks = atoi(numstr);
		    }
		}
		numstr[0]='\0';
		RefreshGraph(graph);
		break;
	    case 'x':
		/*
		** toggle axis placement mode
		*/
		if(global_operation){
		    FORALLPLOTS{
			if(axis_mode == XMODE){
			    plot->xaxis = !plot->xaxis;
			} else
			if(axis_mode == YMODE){
			    plot->yaxis = !plot->yaxis;
			}
		    }
		} else
		if(plot = SelectedPlot(graph)){
		    if(axis_mode == XMODE){
			plot->xaxis = !plot->xaxis;
		    } else
		    if(axis_mode == YMODE){
			plot->yaxis = !plot->yaxis;
		    }
		}
		RefreshGraph(graph);
		break;
	    }
	    escape_mode = 0;
	} else
	/*
	** SINGLE KEYS
	*/
	switch(c){
	case '?':
	    Help();
	    break;
	case '':
	    escape_mode = 1;
	    break;
	case '':
	    PrintToFile(graph,"xplot.ps","a");
	    RefreshGraph(graph);
	    break;
	case '': 
	    Quit();
	    break;
	case '':
	    ZapLabel(graph,event);
	    break;
	case '': 			/* toggle labels display */
	    graph->hide_labels = !graph->hide_labels;
	    RefreshGraph(graph);
	    break;
	case '': 
	    FreeLabels(graph);
	    RefreshGraph(graph);
	    break;
	case '':
	    /*
	    ** select the next plot
	    */
	    cnt = 0;
	    for(plot=graph->plot;plot;plot=plot->next){
		if(plot->selected){
		    ival = cnt +1;
		    break;
		}
		cnt++;
	    }
	    SelectPlot(graph,ival);
	    break;
	case 0x13:		/* ^S */
	    HandCursor(F);
	    SelectPlotGraphically(graph,event->x,event->y);
	    NormalCursor(F);
	    break;
	case '':
	    if(strlen(numstr) > 0){
		val = atof(numstr);
	    } else {
		val = 0;
	    }
	    HandCursor(F);
	    OffsetPlotGraphically(graph,event->x,event->y,val);
	    NormalCursor(F);
	    numstr[0]='\0';
	    break;
	case '': 
	    labelcoord_mode = WINDOW_LBL;	/* enter at window coords */
	    TextCursor(F);
	    TextSelectInputs(graph);
	    SetButtonMode(TEXTMODE);
	    break;
	case '':
	    /*
	    ** erase the current command string
	    */
	    numstr[0]='\0';
	    break;
	case '': 
	    labelcoord_mode = WORLD_LBL;	/* enter at world coords */
	    TextCursor(F);
	    TextSelectInputs(graph);
	    SetButtonMode(TEXTMODE);
	    break;
	case 'A':
	    if(axis_mode == XMODE){
		AutoScale(graph,1,0);
	    } else 
	    if(axis_mode == YMODE){
		AutoScale(graph,0,1);
	    }
	    ScaleAndRefreshGraph(graph);
	    break;
	case 'D':
	    /*
	    ** shift the selected plot down in the list
	    */
	    ival = 1;
	    if(strlen(numstr) > 0){
		ival = atoi(numstr);
	    }
	    numstr[0]='\0';
	    for(i=0;i<ival;i++){
		ShiftPlotList(graph);
	    }
	    /*
	    ** update the plot legends
	    */
	    cnt = 0;
	    for(plot=graph->plot;plot;plot=plot->next){
		DrawPlotLegend(plot,cnt);
		cnt++;
	    }
	    break;
	case 'F':
	    PrintToFile(graph,"xplot.ps","w");
	    RefreshGraph(graph);
	    break;
	case 'I':
	    /*
	    ** set the shift percentage
	    */
	    if(strlen(numstr) > 0){
		shiftpercent = atof(numstr);
	    }
	    numstr[0]='\0';
	    /*
	    ** up shift plot by a percentage of the window bounds
	    */
	    val = (graph->wymax - graph->wymin)*shiftpercent;
	    if(plot = SelectedPlot(graph)){
		plot->yoffset_axis += val;
		ScaleAndRefreshGraph(graph);
	    }
	    break;
	case 'J':
	    /*
	    ** set the shift percentage
	    */
	    if(strlen(numstr) > 0){
		shiftpercent = atof(numstr);
	    }
	    numstr[0]='\0';
	    /*
	    ** left shift plot by a percentage of the window bounds
	    */
	    val = (graph->wxmax - graph->wxmin)*shiftpercent;
	    if(plot = SelectedPlot(graph)){
		plot->xoffset_axis -= val;
		ScaleAndRefreshGraph(graph);
	    }
	    break;
	case 'K':
	    /*
	    ** set the shift percentage
	    */
	    if(strlen(numstr) > 0){
		shiftpercent = atof(numstr);
	    }
	    numstr[0]='\0';
	    /*
	    ** right shift plot by a percentage of the window bounds
	    */
	    val = (graph->wxmax - graph->wxmin)*shiftpercent;
	    if(plot = SelectedPlot(graph)){
		plot->xoffset_axis += val;
		ScaleAndRefreshGraph(graph);
	    }
	    break;
	case 'L':
	    /*
	    ** toggle legends
	    */
	    graph->show_legend = !graph->show_legend;
	    RefreshGraph(graph);
	    break;
	case 'M':
	    /*
	    ** set the shift percentage
	    */
	    if(strlen(numstr) > 0){
		shiftpercent = atof(numstr);
	    }
	    numstr[0]='\0';
	    /*
	    ** down shift plot by a percentage of the window bounds
	    */
	    val = (graph->wymax - graph->wymin)*shiftpercent;
	    if(plot = SelectedPlot(graph)){
		plot->yoffset_axis -= val;
		ScaleAndRefreshGraph(graph);
	    }
	    break;
	case 'P':
	    /*
	    ** do the postscript output at the desired scale
	    ** which blanks the screen
	    */
	    PrintOut(graph);
	    /*
	    ** redisplay the screen
	    */
	    RefreshGraph(graph);
	    break;
	case 'S':
	    /*
	    ** select the previous plot
	    */
	    cnt = 0;
	    for(plot=graph->plot;plot;plot=plot->next){
		if(plot->selected){
		    ival = cnt - 1;
		    break;
		}
		cnt++;
	    }
	    SelectPlot(graph,ival);
	    break;
	case 'T':
	    /*
	    ** toggle title
	    */
	    graph->show_title = !graph->show_title;
	    RefreshGraph(graph);
	    break;
	case 'V':
	    /*
	    ** make all plots visible
	    */
	    for(plot=graph->plot;plot;plot=plot->next){
		plot->visible = 1;
		SetPlotVisibility(plot,plot->visible);
	    }
	case '/':
	    ClearWindow(graph->frame->text);
	    command_entry = 1;
	    strcpy(numstr,"/");
	    break;
	case '[':
	    /*
	    ** set the lower bound 
	    */
	    if(strlen(numstr) > 0){
		if(axis_mode == XMODE){
		    graph->wxmin = atof(numstr);
		} else
		if(axis_mode == YMODE){
		    graph->wymin = atof(numstr);
		}
	    }
	    numstr[0]='\0';
	    ScaleAndRefreshGraph(graph);
	    break;
	case ']':
	    /*
	    ** set the upper bound 
	    */
	    if(strlen(numstr) > 0){
		if(axis_mode == XMODE){
		    graph->wxmax = atof(numstr);
		} else
		if(axis_mode == YMODE){
		    graph->wymax = atof(numstr);
		}
	    }
	    numstr[0]='\0';
	    ScaleAndRefreshGraph(graph);
	    break;
	case '}':
	    /*
	    ** rotate about the selected axis
	    */
	    if(strlen(numstr) > 0){
		rotpercent = atof(numstr);
	    }
	    numstr[0]='\0';
	    val = 2*M_PI*rotpercent;

	    switch(axis_mode){
	    case XMODE:
		graph->thetax += val;
		break;
	    case YMODE:
		graph->thetay += val;
		break;
	    case ZMODE:
		graph->thetaz += val;
		break;
	    }
	    sprintf(tmpstr,"tx = %4d ty = %4d tz = %4d",
	    (int)(360*graph->thetax/(2*M_PI))%360,
	    (int)(360*graph->thetay/(2*M_PI))%360,
	    (int)(360*graph->thetaz/(2*M_PI))%360);
	    Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);
	    UpdateRotationMatrix(graph);
	    ScaleAndRefreshGraph(graph);
	    break;
	case '{':
	    /*
	    ** rotate about the selected axis
	    */
	    if(strlen(numstr) > 0){
		rotpercent = atof(numstr);
	    }
	    numstr[0]='\0';
	    val = 2*M_PI*rotpercent;

	    switch(axis_mode){
	    case XMODE:
		graph->thetax -= val;
		break;
	    case YMODE:
		graph->thetay -= val;
		break;
	    case ZMODE:
		graph->thetaz -= val;
		break;
	    }
	    sprintf(tmpstr,"tx = %4d ty = %4d tz = %4d",
	    (int)(360*graph->thetax/(2*M_PI))%360,
	    (int)(360*graph->thetay/(2*M_PI))%360,
	    (int)(360*graph->thetaz/(2*M_PI))%360);
	    Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);
	    UpdateRotationMatrix(graph);
	    ScaleAndRefreshGraph(graph);
	    break;
	case '<':
	    /*
	    ** zoom out by a percentage of the window bounds
	    */
	    val = (graph->wymax - graph->wymin)*.05;
	    graph->wymax += val;
	    graph->wymin -= val;
	    val = (graph->wxmax - graph->wxmin)*.05;
	    graph->wxmax += val;
	    graph->wxmin -= val;
	    ScaleAndRefreshGraph(graph);
	    break;
	case '>':
	    /*
	    ** zoom in by a percentage of the window bounds
	    */
	    val = (graph->wymax - graph->wymin)*.05;
	    graph->wymax -= val;
	    graph->wymin += val;
	    val = (graph->wxmax - graph->wxmin)*.05;
	    graph->wxmax -= val;
	    graph->wxmin += val;
	    ScaleAndRefreshGraph(graph);
	    break;
	case 'a':
	    AutoScale(graph,graph->xaxis.autoscale,graph->yaxis.autoscale);
	    ScaleAndRefreshGraph(graph);
	    break;
	case 'g':
	    /*
	    ** toggle grid
	    */
	    if(axis_mode == XMODE){
		graph->xaxis.show_grid = !graph->xaxis.show_grid;
	    }
	    if(axis_mode == YMODE){
		graph->yaxis.show_grid = !graph->yaxis.show_grid;
	    }
	    RefreshGraph(graph);
	    break;
	case 'i':
	    /*
	    ** set the shift percentage
	    */
	    if(strlen(numstr) > 0){
		shiftpercent = atof(numstr);
	    }
	    numstr[0]='\0';
	    /*
	    ** up shift plot by a percentage of the window bounds
	    */
	    val = (graph->wymax - graph->wymin)*shiftpercent;
	    graph->wymax += val;
	    graph->wymin += val;
	    ScaleAndRefreshGraph(graph);
	    break;
	case 'j':
	    /*
	    ** set the shift percentage
	    */
	    if(strlen(numstr) > 0){
		shiftpercent = atof(numstr);
	    }
	    numstr[0]='\0';
	    LeftShift();
	    break;
	case 'k':
	    /*
	    ** set the shift percentage
	    */
	    if(strlen(numstr) > 0){
		shiftpercent = atof(numstr);
	    }
	    numstr[0]='\0';
	    RightShift();
	    break;
	case 'l':
	    /*
	    ** toggle labels
	    */
	    if(axis_mode == XMODE){
		graph->xaxis.show_labels = !graph->xaxis.show_labels;
	    } 
	    if(axis_mode == YMODE){
		graph->yaxis.show_labels = !graph->yaxis.show_labels;
	    }
	    RefreshGraph(graph);
	    break;
	case 'm':
	    /*
	    ** set the shift percentage
	    */
	    if(strlen(numstr) > 0){
		shiftpercent = atof(numstr);
	    }
	    numstr[0]='\0';
	    /*
	    ** down shift plot by a percentage of the window bounds
	    */
	    val = (graph->wymax - graph->wymin)*shiftpercent;
	    graph->wymax -= val;
	    graph->wymin -= val;
	    ScaleAndRefreshGraph(graph);
	    break;
	case 'n':		/* forward step through */
	    StepForward(graph);
	    break;
	case 'p':		/* backward step through */
	    StepBackward(graph);
	    break;
	case 'r':
	    ScaleAndRefreshGraph(graph);
	    break;
	case 's':
	    /*
	    ** select the plot
	    */
	    if(strlen(numstr) > 0){
		/*
		** get the number of the plot
		*/
		ival = atoi(numstr);
	    } else {
		/*
		** select the next plot
		*/
		cnt = 0;
		for(plot=graph->plot;plot;plot=plot->next){
		    if(plot->selected){
			ival = cnt +1;
			break;
		    }
		    cnt++;
		}
	    }
	    SelectPlot(graph,ival);
	    numstr[0]='\0';
	    break;
	case 'v':
	    /*
	    ** toggle visibility
	    */
	    if(global_operation){
		FORALLPLOTS{
		    plot->visible = !plot->visible;
		    SetPlotVisibility(plot,plot->visible);
		}
	    } else
	    if(plot = SelectedPlot(graph)){
		plot->visible = !plot->visible;
		SetPlotVisibility(plot,plot->visible);
	    }
	    numstr[0]='\0';
	    RefreshGraph(graph);
	    break;
	case 'x':
	    axis_mode = XMODE;
	    break;
	case 'y':
	    axis_mode = YMODE;
	    break;
	case 'z':
	    axis_mode = ZMODE;
	    break;
	case '|':
	    /*
	    ** set the axis intercept 
	    */
	    if(strlen(numstr) > 0){
		if(axis_mode == XMODE){
		    graph->xaxis.yintcpt = atof(numstr);
		} else
		if(axis_mode == YMODE){
		    graph->yaxis.xintcpt = atof(numstr);
		}
	    }
	    numstr[0]='\0';
	    RefreshGraph(graph);
	    break;
	case '=':
	    /*
	    ** select the offset of the currently selected plot
	    */
	    if(strlen(numstr) > 0){
		if(global_operation){
		    FORALLPLOTS{
			if(axis_mode == XMODE){
			    plot->xoffset = atof(numstr);
			} else
			if(axis_mode == YMODE){
			    plot->yoffset = atof(numstr);
			}
		    }
		} else
		if(plot = SelectedPlot(graph)){
		    if(axis_mode == XMODE){
			plot->xoffset = atof(numstr);
		    } else
		    if(axis_mode == YMODE){
			plot->yoffset = atof(numstr);
		    }
		}
	    }
	    numstr[0]='\0';
	    ScaleAndRefreshGraph(graph);
	    break;
	case '*':
	    /*
	    ** select global operation
	    */
	    global_operation = 1;
	    break;
	}
    }
}
