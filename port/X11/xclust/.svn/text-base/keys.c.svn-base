#include <sys/types.h>
#include <sys/stat.h>
#include "xclust_ext.h"

#define NUMSTRLEN 200
char	numstr[NUMSTRLEN+1];
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
static float zoompercent = 0.1;
float rotpercent = 0.05;
int	rotatecolor = 0;

/* extern void SetRotationMatrix(double***, float, float, float); */
extern void AffineRotationMatrixEuler(double ***, float , float , float );
extern Label *commandlabel;
extern Label *coordlabel;
extern Label *rotlabel;

#define FORALLPLOTS for(plot=graph->plot;plot;plot=plot->next)

SetHeader(mode)
int mode;
{
    header = mode;
}

AutoBarWidth(plot,percent)
Plot	*plot;
float	percent;
{
int	p0;
ProjectionInfo	*pinfo;
double	p0val1;
double	p0val0;

    /*
    ** set bar width to a fraction of the distance between the first
    ** two points
    */
    p0 = plot->graph->currentprojection[0];
    if((pinfo=GetProjectionInfo(plot->graph,p0)) == NULL){
	fprintf(stderr,"ERROR: Unable to find info on field %d\n",p0);
	return;
    }
    p0val0 = GetDoubleFromRawData(plot->rawdata[0].data,pinfo);
    p0val1 = GetDoubleFromRawData(plot->rawdata[1].data,pinfo);
    if(plot->npoints > 2 && p0 >= 0 && p0 < plot->ndatafields){
	plot->bar_width = plot->xscale*percent*
	fabs(p0val0 - p0val1);
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

MenuSetPinchMode(item)
MenuItem	*item;
{
    if(item->state == 0){
	NormalSelectInputs(G);
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	ClickSelectInputs(G);
	SetButtonMode(PINCHMODE);
	HandCursor(F);
    }
    if(item->state == 1){
	MenuClassSetState(item,item->menu,item->class,0);
	/*
	** refresh the menu display
	*/
	DrawMenu(item->menu);
    } 
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

MenuSetClusterBoxMode(item)
MenuItem	*item;
{
    if(item->state == 0){
	NormalSelectInputs(G);
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	ClickSelectInputs(G);
	SetButtonMode(CLUSTERBOXMODE);
	CrosshairCursor(F);
    }
    if(item->state == 1){
	MenuClassSetState(item,item->menu,item->class,0);
	/*
	** refresh the menu display
	*/
	DrawMenu(item->menu);
    } 
}

ToggleClusterBoxMode()
{
    if(ButtonMode() == CLUSTERBOXMODE){
	NormalSelectInputs(G);
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	ClickSelectInputs(G);
	SetButtonMode(CLUSTERBOXMODE);
	CrosshairCursor(F);
    }
}

MenuSetPolyLineMode(item)
MenuItem	*item;
{
    if(item->state == 0){
	NormalSelectInputs(G);
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	ClickSelectInputs(G);
	SetButtonMode(POLYLINEMODE);
	CrosshairCursor(F);
    }
    if(item->state == 1){
	MenuClassSetState(item,item->menu,item->class,0);
	/*
	** refresh the menu display
	*/
	DrawMenu(item->menu);
    } 
}

MenuSetClusterMode(MenuItem *item)
{

  if (strcmp(item->value, BOXSTRING)==0)
    item->value = POLYGONSTRING;
  else
    item->value = BOXSTRING;

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

MenuSetClusterPolyLineMode(item)
MenuItem	*item;
{
    if(item->state == 0){
	NormalSelectInputs(G);
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	ClickSelectInputs(G);
	SetButtonMode(CLUSTERPOLYLINEMODE);
	CrosshairCursor(F);
    }
    if(item->state == 1){
	MenuClassSetState(item,item->menu,item->class,0);
	/*
	** refresh the menu display
	*/
	DrawMenu(item->menu);
    } 
}

ToggleClusterPolyLineMode()
{
    if(ButtonMode() == CLUSTERPOLYLINEMODE){
	NormalSelectInputs(G);
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	ClickSelectInputs(G);
	SetButtonMode(CLUSTERPOLYLINEMODE);
	CrosshairCursor(F);
    }
}


MenuSetDrawmode(item)
MenuItem	*item;
{
    if(item->state == 0){
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	SetButtonMode(DRAWMODE);
	CrosshairCursor(F);
    }
    if(item->state == 1){
	MenuClassSetState(item,item->menu,item->class,0);
	/*
	** refresh the menu display
	*/
	DrawMenu(item->menu);
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

MenuSetZapmode(item)
MenuItem	*item;
{
    if(item->state == 0){
	SetButtonMode(DATAMODE);
	NormalCursor(F);
    } else {
	SetButtonMode(ZAPMODE);
	TargetCursor(F);
    }
    if(item->state == 1){
	MenuClassSetState(item,item->menu,item->class,0);
	/*
	** refresh the menu display
	*/
	DrawMenu(item->menu);
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

SetTextEntryMode(graph,mode)
Graph	*graph;
int	mode;
{
    labelcoord_mode = mode;	
    TextCursor(F);
    TextSelectInputs(graph);
    SetButtonMode(TEXTMODE);
}

MenuWorldTextEntry(item)
MenuItem	*item;
{
    SetTextEntryMode(G,WORLD_LBL);
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
  if (val>=0 && val<1)
    shiftpercent = val;
  else
    fprintf(stderr, "ERROR: Invalid shiftpercent: %f\n", val);
}

SetZoomPercent(val)
float val;
{
  if (val>=0 && val<1)
    zoompercent = val;
  else
    fprintf(stderr, "ERROR: Invalid zoompercent: %f\n", val);
}

float GetZoomPercent()
{
  return (float) zoompercent;
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

SelectPlot(plot)
Plot	*plot;
{
Graph 	*graph;
Plot	*p;

    if(plot == NULL || plot->graph == NULL) return;
    graph = plot->graph;
    /*
    ** clear previous selections
    */
    for(p=graph->plot;p;p=p->next){
	p->selected = 0;
    }
    /*
    ** mark the new selection
    */
    plot->selected = 1;
}

SelectPlotByNumber(graph,ival)
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
	if(graph->plot){
	    graph->plot->selected = 1;
	}
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
int	p1;
ProjectionInfo	*pinfo;
double	p1val;

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
	p1 = graph->currentprojection[1];
	if((pinfo=GetProjectionInfo(plot->graph,p1)) == NULL){
	    fprintf(stderr,"ERROR: Unable to find info on field %d\n",p1);
	    return;
	}
	p1val = GetDoubleFromRawData(plot->rawdata[closest].data,pinfo);
	if(p1 >= 0 && p1 < plot->ndatafields){
	    plot->xoffset_axis = val - p1val;
	}
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
    distance = -1;
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

    if(graph == NULL) return(NULL);
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
	return(0);
    }
    if((stbuf.st_mode & S_IFMT) == S_IFDIR) {
	fprintf(stderr,"fsize: %s is a directory\n",name);
	return(0);
    }
    return(stbuf.st_size);
}

MenuZoomOut(item)
MenuItem	*item;
{
    ZoomOut(GetGraph("/graph"),zoompercent);
}

MenuZoomIn(item)
MenuItem	*item;
{
    ZoomIn(GetGraph("/graph"),zoompercent);
}

ZoomOut(graph,fract)
Graph	*graph;
float	fract;
{
float	val;

    /*
    ** zoom out by a percentage of the window bounds
    */
    val = (graph->wymax - graph->wymin)*fract;
    graph->wymax += val;
    graph->wymin -= val;
    val = (graph->wxmax - graph->wxmin)*fract;
    graph->wxmax += val;
    graph->wxmin -= val;
    ScaleAndRefreshGraph(graph);
}

ZoomIn(graph,fract)
Graph	*graph;
float	fract;
{
float	val;

    /*
    ** zoom in by a percentage of the window bounds
    */
    val = (graph->wymax - graph->wymin)*fract;
    graph->wymax -= val;
    graph->wymin += val;
    val = (graph->wxmax - graph->wxmin)*fract;
    graph->wxmax -= val;
    graph->wxmin += val;
    ScaleAndRefreshGraph(graph);
}

/* returns 0 on successful saving/loading of saved view, 1 otherwise */

#define SV graph->savedview[index]

int ProcessSavedView(graph, savemode, index)
Graph	*graph;
int	savemode;
int	index;
{
ProjectionInfo	*pinfo;
char	tmpstr[80];


     /* test if index in bounds */ 
     if (index < 0 || index > 12){   
       fprintf(stderr,"ERROR: Can't save/load view. Slot %d out of range 0-12. (ProcessSavedView)\n", index);
       return(0);
     }

     if(savemode){

       /*
       ** save current view in a F-key slot
       */

       /* flag that this F-key slot is now being used */
       SV.used = 1;

       /* save the data range */
       SV.wxmin = graph->wxmin;
       SV.wymin = graph->wymin;
       SV.wxmax = graph->wxmax;
       SV.wymax = graph->wymax;
    
       /* save the projection numbers */
       SV.projection[0] = graph->currentprojection[0];
       SV.projection[1] = graph->currentprojection[1];
       SV.projection[2] = graph->currentprojection[2];

       /* print status message */
       if (index > 1) { /* don't issue messages on rescale-driven savedviews */
	 sprintf(commandlabel->u.string.line, "Current view saved in F%d",index);
	 RefreshText(graph->frame->text);
	 if(debug)
	   fprintf(stderr,strcat(commandlabel->u.string.line, "\n"));
       }

       /* save current savedview in lastsavedview */
       if (index>1)
	 graph->lastsavedview = index;
       
    } else { 

      /*
      ** load saved view from F-key slot
      */

      /* is there a view to load? */
      if(!SV.used){
	sprintf(commandlabel->u.string.line, "No view saved in F%d",index);
	RefreshText(graph->frame->text);
	if (debug)
	  fprintf(stderr,strcat(commandlabel->u.string.line, "\n"));
	return(1);
      }
	
      /* test whether the projections we want to switch to all exist */
      /* proj # of -1 means leave projection unchanged, so we don't
      ** have to test for existence of new projection (used, e.g. for
      ** z-axis in legacy 2-projection /loadview command) */
      if ((SV.projection[0] >= 0 && !GetProjectionInfo(G,SV.projection[0])) ||
	  (SV.projection[1] >= 0 && !GetProjectionInfo(G,SV.projection[1])) ||
	  (SV.projection[2] >= 0 && !GetProjectionInfo(G,SV.projection[2]))){
	return(1);
      }

      /*
      ** before switching projections save the data ranges 
      ** for the x and y  projections in the projection info
      */
      if(pinfo = GetProjectionInfo(G,G->currentprojection[0])){
	pinfo->wmin = G->wxmin;
	pinfo->wmax = G->wxmax;
      }
      if(pinfo = GetProjectionInfo(G,G->currentprojection[1])){
	pinfo->wmin = G->wymin;
	pinfo->wmax = G->wymax;
      }

      /* restore stored view */
      /* proj # of -1 means leave projection unchanged (see /loadview command) */
      if (SV.projection[0] >= 0) {
	graph->currentprojection[0] = SV.projection[0];  
	graph->selectedprojection[0] = -1;
      }
      if (SV.projection[1] >= 0) {
	graph->currentprojection[1] = SV.projection[1];  
	graph->selectedprojection[1] = -1;
      }
      if (SV.projection[2] >= 0) {
	graph->currentprojection[2] = SV.projection[2];
	graph->selectedprojection[2] = -1;
      }

      /* update labels, etc */
      UpdateProjections();

      /* negative projection # means leave it alone */
      if (SV.projection[0] >= 0) {
	/* use provided data range or... */
	if (SV.wxmin != -1 ||
	    SV.wxmax != -1){
	  graph->wxmin = SV.wxmin;
	  graph->wxmax = SV.wxmax;
	} else {
	  /* ... autoscale the X axis on initial pass only*/

	  /* we have to rescale first, (after setting new projections)
	     or else AutoScale uses the data from the last view */
	  RescaleGraph(graph);
	  AutoScale(graph,1,0);

	  /* set this as the new range */
	  SV.wxmin = graph->wxmin;
	  SV.wxmax = graph->wxmax;
	}
      }
	  
      /* negative projection # means leave it alone */
      if (SV.projection[1] >= 0) {
	/* use provided data range or... */
	if (SV.wymin != -1 ||
	    SV.wymax != -1){
	  graph->wymin = SV.wymin;
	  graph->wymax = SV.wymax;
	} else {
	  /* ... autoscale the Y axis on initial pass only */

	  /* we have to rescale first, (after setting new projections)
	     or else AutoScale uses the data from the last view */
	  RescaleGraph(graph);  
	  AutoScale(graph,0,1);

	  /* set this as the new range */
	  SV.wymin = graph->wymin;
	  SV.wymax = graph->wymax;
	}	
      }
	  
      /* refresh graph */
      ScaleAndRefreshGraph(graph);

      /* set button states to reflect new view */
      AssignProjectionButtons();


      /* print success  message */
      sprintf(commandlabel->u.string.line, "Restored view from F%d",index);
      RefreshText(graph->frame->text);
      if(debug)
	fprintf(stderr,strcat(commandlabel->u.string.line, "\n"));

      /* save current view number in lastsavedview */
      if (index>1)
	graph->lastsavedview = index;
    }
     return(0);
}
#undef SV



/* load saved views into Graph */

/* a negative # for any projection means that view doesn't specify
   that projection (and it will not be changed by switching to that
   view */

/* a negative # for any data range value means to autoscale on
   loading in the view */

LoadViewDefaults(graph,index,p0,p1,p2,wxmin,wymin,wxmax,wymax)
Graph	*graph;
int	index;
int	p0,p1,p2;
float	wxmin,wymin,wxmax,wymax;
{

  /* test for valid view # (F1 reserved) */
  if(index < 2 || index > 12){
    fprintf(stderr,"attempt to load invalid view index %d\n",index);
    return(1);
  }

  /* test that all non-negative projections exist */
  /* (negative projections are 'unset') */
  if ((p0 >= 0 && !GetProjectionInfo(G,p0)) ||
      (p1 >= 0 && !GetProjectionInfo(G,p1)) ||
      (p2 >= 0 && !GetProjectionInfo(G,p2))){
    if(!suppresswarnings)
      fprintf(stderr,"WARNING: Projections in saved view F%d do not exist; skipping.(LoadViewDefaults)\n",index);
    return(1);
  }
	
  graph->savedview[index].projection[0] = p0;
  graph->savedview[index].projection[1] = p1;
  graph->savedview[index].projection[2] = p2;
  graph->savedview[index].wxmin = wxmin;
  graph->savedview[index].wymin = wymin;
  graph->savedview[index].wxmax = wxmax;
  graph->savedview[index].wymax = wymax;
  graph->savedview[index].used = 1;
  return(0);
}

SaveViewDefaults(graph,fp)
Graph	*graph;
FILE	*fp;
{
int	index;
char	*s;

    for(index=2;index<=12;index++){
	if(graph->savedview[index].used != 0)
	  {
	    fprintf(fp,"/loadview %d %d %d %d %g %g %g %g\n",
		    index,
		    graph->savedview[index].projection[0],
		    graph->savedview[index].projection[1],
		    graph->savedview[index].projection[2],
		    graph->savedview[index].wxmin,
		    graph->savedview[index].wymin,
		    graph->savedview[index].wxmax,
		    graph->savedview[index].wymax);
	  }
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

    /* to avoid segfaults, catch weird keystrokes (like blue 'Fn' key
    ** on IBM thinkpad) that have no string associated with them */
    
    if (XKeysymToString(key) == NULL)
      return; 
    
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
	if(strcmp(XKeysymToString(key),"F11")==0){
	    ProcessSavedView(graph,escape_mode,11);
	    escape_mode = 0;
	} 
	if(strcmp(XKeysymToString(key),"F12")==0){
	    ProcessSavedView(graph,escape_mode,12);
	    escape_mode = 0;
	} 
	return;
    }
    if(command_entry){
	if(debug){
	    printf("%0x\n",c);
	}
	if(c == '\r'){
	  /*ClearWindow(graph->frame->text);*/
	  sprintf(commandlabel->u.string.line, "%s","Done.");
	  RefreshText(graph->frame->text);
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
	    
	    /* overwrite the just-used command line with 'null' terminators */
	    /*for (i = 0; i <= NUMSTRLEN; i++)*/
	    numstr[0] = '\0';
	} else
	if(c == '' || c== ''){
	    if(strlen(numstr) > 0){
		numstr[strlen(numstr)-1] = '\0';
		SetColor(graph->frame->text->foreground);
		/*sprintf(tmpstr,"%s ",numstr);
		  Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);*/
		sprintf(commandlabel->u.string.line, "%s",numstr);
		RefreshText(graph->frame->text);

	    }
	} else {
	    SetColor(graph->frame->text->foreground);
	    /*sprintf(tmpstr,"%s%c ",numstr,c);
	      Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);*/
	    sprintf(commandlabel->u.string.line, "%s%c",numstr,c);
	    RefreshText(graph->frame->text);
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
	    /*sprintf(tmpstr,"%s ",numstr);
	      Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);*/
	    sprintf(commandlabel->u.string.line, "%s",numstr);
	    RefreshText(graph->frame->text);
	}
    } else {
	SetColor(graph->frame->text->foreground);
	/*sprintf(tmpstr,"%s%c ",numstr,c);
	   Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);*/
	sprintf(commandlabel->u.string.line, "%s%c",numstr,c);
	RefreshText(graph->frame->text);
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
	    case 'C':
		ToggleClusterBoxMode();
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
		if(F->menucontainer->mapped){
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
		SetInverse(graph,-1,-1);
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
			plot->plot_type = LINE_GRAPH;
		    }
		}
		numstr[0]='\0';
		/*
		** set the source default
		*/
		SetSourceDefault(plot->source,PLOTTYPE,Atoi(numstr));
		RefreshGraph(graph);
		break;
	    case 'o':
		/*
		** toggle automatic axis placement mode
		*/
		graph->auto_axes = !graph->auto_axes;
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
			    plot->source->defaults[POINTTYPE] = atoi(numstr);
			} else {
			    plot->point_symbol = BOX_PT;
			    plot->source->defaults[POINTTYPE] = BOX_PT;
			}
		    }
		}
		if(strlen(numstr) > 0){
		    if(plot = SelectedPlot(graph)){
			plot->point_symbol = atoi(numstr);
			plot->source->defaults[POINTTYPE] = atoi(numstr);
		    }
		} else {
		    if(plot = SelectedPlot(graph)){
			plot->point_symbol = BOX_PT;
			plot->source->defaults[POINTTYPE] = BOX_PT;
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
	    sprintf(commandlabel->u.string.line, "%s","ESC-");
	    RefreshText(graph->frame->text);
	    return;
	    break;
	case '':
	    PrintToFile(graph,"xclust.ps","a");
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
	    SelectPlotByNumber(graph,ival);
	    break;
	case '':
	    rotatecolor = !rotatecolor;
	    if(rotatecolor == 1){
		/*
		** save the current value of the pixel
		*/
		SavePixel();
	    } else {
		/*
		** restore the previous value of the pixel
		*/
		RestorePixel();
	    }
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
	    SetTextEntryMode(graph,WINDOW_LBL); /* enter at screen cooords */
	    break;
	case '':
	    /*
	    ** erase the current command string
	    */
	    numstr[0]='\0';
	    break;
	case '': 
	    SetTextEntryMode(graph,WORLD_LBL); /* enter at world coords */
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
	    PrintToFile(graph,"xclust.ps","w");
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
	    SelectPlotByNumber(graph,ival);
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
	  /* ClearWindow(graph->frame->text);*/
	    command_entry = 1;
	    strcpy(numstr,"/");
	    sprintf(commandlabel->u.string.line, "%s", numstr);
	    RefreshText(graph->frame->text);
	    return;
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
	    sprintf(rotlabel->u.string.line, "tx: %5d  ty: %5d  tz: %5d",
	    (int)(360*graph->thetax/(2*M_PI))%360,
	    (int)(360*graph->thetay/(2*M_PI))%360,
	    (int)(360*graph->thetaz/(2*M_PI))%360);
	    RefreshText(graph->frame->text);
	    /*Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);*/
/* 	    SetRotationMatrix(&graph->matrix, graph->thetax, graph->thetay, graph->thetaz); */
	    AffineRotationMatrixEuler(&graph->matrix, graph->thetax, graph->thetay, graph->thetaz);
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
	    sprintf(rotlabel->u.string.line, "tx: %5d  ty: %5d  tz: %5d",
	    (int)(360*graph->thetax/(2*M_PI))%360,
	    (int)(360*graph->thetay/(2*M_PI))%360,
	    (int)(360*graph->thetaz/(2*M_PI))%360);
	    RefreshText(graph->frame->text);
	    /*Text(graph->frame->text,0,graph->frame->text->fontheight,tmpstr);*/
/* 	    SetRotationMatrix(&graph->matrix, graph->thetax, graph->thetay, graph->thetaz); */
	    AffineRotationMatrixEuler(&graph->matrix, graph->thetax, graph->thetay, graph->thetaz);
	    ScaleAndRefreshGraph(graph);
	    break;
	case '<':
	    ZoomOut(graph,zoompercent);
	    break;
	case '>':
	    ZoomIn(graph,zoompercent);
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
	    SelectPlotByNumber(graph,ival);
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
    sprintf(commandlabel->u.string.line, "%s","Done.");
    RefreshText(graph->frame->text);
}

KeyReleaseAction(Graph *graph,XKeyEvent* event)
{
char	buffer[100];
KeySym	key;

    buffer[0] = '\0';
    /* 
    ** do key mapping to determine the actual key pressed
    */
    XLookupString(event,buffer,100, &key, NULL);

    /* to avoid segfaults, catch weird keystrokes (like blue 'Fn' key
    ** on IBM thinkpad) that have no string associated with them */
    if (XKeysymToString(key) == NULL)
      return; 

    /* if we are in FLASHSELECTMODE turn off flashing on release of ctrl-key */
    if ((strcmp(XKeysymToString(key),"Control_L") == 0) ||
	(strcmp(XKeysymToString(key),"Control_R") == 0)) {
      if (ButtonMode() == FLASHSELECTMODE){
	SetButtonMode(DATAMODE);
	NormalSelectInputs(G);
	InterpretCommand(graph,SelectedPlot(G),"/flash 0");
      }
    }
}
	
