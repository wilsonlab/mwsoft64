#include	"xplot_ext.h"

int snapmode = 1;
static int buttonmode = DATAMODE;
static int fastsearch = TRUE;

FastSearchMode(i)
int	i;
{
    fastsearch = i;
}

ButtonMode()
{
    return(buttonmode);
}

SetButtonMode(mode)
int mode;
{
    buttonmode = mode;
}

ToggleSnapMode()
{
    snapmode = !snapmode;
}

SnapMode(mode)
int mode;
{
    snapmode = mode;
}

/*
** takes the screen coordinates given in sx,sy and locates the 
** data point with the closest x screen coordinate
** then returns those data values in wx,wy
*/
int Snap(graph,sx,sy,wx,wy)
Graph *graph;
int sx,sy;
float *wx,*wy;
{
float	x,y;
float	wz;
float	wx2,wy2;
int	i;
int	npoints;
Plot	*p;
Plot	*plot;
float	dist,min_dist;
int	min_pt;
int	ecount;
int	estimated_start_i;

    /*
    ** get the world coordinates of the cursor
    */
    WorldTransform(graph,sx,sy,&x,&y);
    /*
    ** get the currently selected plot
    ** use the first plot as the default
    */
    plot = graph->plot;
    for(p=graph->plot;p;p=p->next){
	if(p->selected){
	    plot = p;
	}
    }
    /*
    ** locate the data point with the closest x coord
    ** from the currently selected plot
    */
    npoints = plot->npoints;
    min_dist = -1;
    if(fastsearch){
	/*
	** check the endpoints
	*/
	if(npoints > 0){
	    /*
	    ** get the first data point
	    */
	    GetTransformedDataPoint(plot,wx,wy,&wz,0);
	    /*
	    ** if there is only one point then just return
	    ** return with wx,wy set to the data point
	    */
	    if(npoints < 2) return(npoints-1);
	    /*
	    ** otherwise get the last data point
	    */
	    GetTransformedDataPoint(plot,&wx2,&wy2,&wz,npoints-1);
	    /*
	    ** is it beyond the most positive data point?
	    */
	    if(x >= *wx && x >= wx2){
		/*
		** return with wx,wy set to the most positive data point
		*/
		if(wx2 > *wx){
		    *wx = wx2;
		    *wy = wy2;
		}
		return(npoints-1);
	    }
	    /*
	    ** is it beyond the most negative data point?
	    */
	    if(x <= *wx && x <= wx2){
		/*
		** return with wx,wy set to the most negative data point
		*/
		if(wx2 < *wx){
		    *wx = wx2;
		    *wy = wy2;
		}
		return(0);
	    }
	    /*
	    ** SPEED UP THE SEARCH
	    ** we can estimate the likely index from the current min/max window
	    ** index values. start a little bit ahead of the estimate.
	    */
	    if(plot->xmax > plot->xmin){
		estimated_start_i = plot->xlo + 
		(plot->xhi - plot->xlo)*(x - plot->xmin)/(plot->xmax - plot->xmin);
		/*
		** if the estimated start coord is significantly different
		** than the current coord then make an adjustment
		*/
		for(ecount=0;ecount<5;ecount++){
		    if(estimated_start_i > plot->npoints) {
			estimated_start_i = plot->npoints - 1;
		    }
		    if(estimated_start_i < 0) {
			estimated_start_i = 0;
		    }
		    GetTransformedDataPoint(plot,wx,wy,&wz,estimated_start_i);
		    estimated_start_i = estimated_start_i + 
		    0.5*(plot->xhi - plot->xlo)*(x-*wx)/(plot->xmax - plot->xmin);
		}
	    } else {
		estimated_start_i = 0;
	    }


	    if(estimated_start_i > plot->npoints) {
		estimated_start_i = plot->npoints - 1;
	    }
	    if(estimated_start_i < 0) {
		estimated_start_i = 0;
	    }

	    GetTransformedDataPoint(plot,wx,wy,&wz,estimated_start_i);
	    /*
	    ** now if the coordinate of the estimated start is greater than
	    ** the selected coord then start the search backward
	    ** else forward
	    */
	    if(*wx <= x){
		for(i=estimated_start_i+1;i<plot->npoints;i++){
		    GetTransformedDataPoint(plot,&wx2,&wy2,&wz,i);
		    /*
		    ** is x between this data point and the last?
		    */
		    if(wx2 >= *wx){
			if(x < wx2 && x >= *wx ){
			    return(i-1);
			}
		    } else {
			if(x < *wx && x >= wx2 ){
			    *wx = wx2;
			    *wy = wy2;
			    return(i-1);
			}
		    }
		    *wx = wx2;
		    *wy = wy2;
		}
		/*
		** if not found there then search back to the beginning
		*/
		GetTransformedDataPoint(plot,wx,wy,&wz,estimated_start_i);
		for(i=estimated_start_i-1;i>=0;i--){
		    GetTransformedDataPoint(plot,&wx2,&wy2,&wz,i);
		    /*
		    ** is x between this data point and the last?
		    */
		    if(wx2 >= *wx){
			if(x < wx2 && x >= *wx ){
			    return(i-1);
			}
		    } else {
			if(x < *wx && x >= wx2 ){
			    *wx = wx2;
			    *wy = wy2;
			    return(i-1);
			}
		    }
		    *wx = wx2;
		    *wy = wy2;
		}
		return(0);
	    } else {
		/*
		** if not found there then search back to the beginning
		*/
		for(i=estimated_start_i-1;i>=0;i--){
		    GetTransformedDataPoint(plot,&wx2,&wy2,&wz,i);
		    /*
		    ** is x between this data point and the last?
		    */
		    if(wx2 >= *wx){
			if(x < wx2 && x >= *wx ){
			    return(i-1);
			}
		    } else {
			if(x < *wx && x >= wx2 ){
			    *wx = wx2;
			    *wy = wy2;
			    return(i-1);
			}
		    }
		    *wx = wx2;
		    *wy = wy2;
		}
		GetTransformedDataPoint(plot,wx,wy,&wz,estimated_start_i);
		for(i=estimated_start_i+1;i<plot->npoints;i++){
		    GetTransformedDataPoint(plot,&wx2,&wy2,&wz,i);
		    /*
		    ** is x between this data point and the last?
		    */
		    if(wx2 >= *wx){
			if(x < wx2 && x >= *wx ){
			    return(i-1);
			}
		    } else {
			if(x < *wx && x >= wx2 ){
			    *wx = wx2;
			    *wy = wy2;
			    return(i-1);
			}
		    }
		    *wx = wx2;
		    *wy = wy2;
		}
		return(plot->npoints-1);
	    }

	    for(i=estimated_start_i+1;i<plot->npoints;i++){
		GetTransformedDataPoint(plot,&wx2,&wy2,&wz,i);
		/*
		** is x between this data point and the last?
		*/
		if(wx2 >= *wx){
		    if(x < wx2 && x >= *wx ){
			return(i-1);
		    }
		} else {
		    if(x < *wx && x >= wx2 ){
			*wx = wx2;
			*wy = wy2;
			return(i-1);
		    }
		}
		*wx = wx2;
		*wy = wy2;
	    }
	    /*
	    ** if not found there then search back to the beginning
	    */
	    GetTransformedDataPoint(plot,wx,wy,&wz,estimated_start_i);
	    for(i=estimated_start_i-1;i>=0;i--){
		GetTransformedDataPoint(plot,&wx2,&wy2,&wz,i);
		/*
		** is x between this data point and the last?
		*/
		if(wx2 >= *wx){
		    if(x < wx2 && x >= *wx ){
			return(i-1);
		    }
		} else {
		    if(x < *wx && x >= wx2 ){
			*wx = wx2;
			*wy = wy2;
			return(i-1);
		    }
		}
		*wx = wx2;
		*wy = wy2;
	    }
	    return(0);
#ifdef OLD
	    GetTransformedDataPoint(plot,wx,wy,&wz,0);
	    for(i=1;i<plot->npoints;i++){
		GetTransformedDataPoint(plot,&wx2,&wy2,&wz,i);
		/*
		** is x between this data point and the last?
		*/
		if(wx2 >= *wx){
		    if(x < wx2 && x >= *wx ){
			return(i-1);
		    }
		} else {
		    if(x < *wx && x >= wx2 ){
			*wx = wx2;
			*wy = wy2;
			return(i-1);
		    }
		}
		*wx = wx2;
		*wy = wy2;
	    }
	    return(npoints -1);
#endif
	} else {
	    return(-1);
	}
    } else {
	/*
	** just slog through all of the points
	*/
	for(i=0;i<plot->npoints;i++){
	    GetTransformedDataPoint(plot,&wx2,&wy2,&wz,i);
	    /*
	    ** compute sqr distance
	    */
	    dist = (x - wx2)*(x - wx2) + (y - wy2)*(y - wy2);
	    if(i == 0 || dist < min_dist) {
		min_dist = dist;
		min_pt = i;
		*wx = wx2;
		*wy = wy2;
	    }
	}
	return(min_pt);
    }
}

NBShowPoint(basic,sx,sy,x,y,pt)
BasicWindow	*basic;
int sx,sy;
float x,y;
int	pt;
{
char label[80];

    sprintf(label,"n=%d x=%-8.4g y=%-8.4g",pt,x,y);
    SetColor(basic->foreground);
    Text(basic,sx +3,sy,label);
    FreeLabels(basic);
    AddLabelString(basic,label,sx+3,sy,0.0,0.0,SCREEN_LBL,TEMPORARY_LBL,basic->fontname);
}

NBShowCoords(basic,sx,sy,x,y)
BasicWindow	*basic;
int sx,sy;
float x,y;
{
char label[80];

    sprintf(label,"x=%-8.4g y=%-8.4g",x,y);
    SetColor(basic->foreground);
    Text(basic,sx +3,sy,label);
    FreeLabels(basic);
    AddLabelString(basic,label,sx+3,sy,0.0,0.0,SCREEN_LBL,TEMPORARY_LBL,basic->fontname);
}

ShowCoords(basic,sx,sy,x,y)
BasicWindow	*basic;
int sx,sy;
float x,y;
{
char label[80];

    Box(sx-2,sy-2,sx+2,sy+2);
    AddLabelBox(basic,0,0,0,x,y,2.0,WORLD_LBL,TEMPORARY_LBL,basic->fontname);
    sprintf(label,"x=%-8.4g y=%-8.4g",x,y);
    SetColor(basic->foreground);
    Text(basic,sx +3,sy,label);
    AddLabelString(basic,label,sx+3,sy,x,y,WORLD_LBL,TEMPORARY_LBL,basic->fontname);
}

ShowSlope(basic,sx,sy,x1,y1,x2,y2)
BasicWindow	*basic;
int sx,sy;
float x1,y1;
float x2,y2;
{
char label1[200];
char label[200];

    SetColor(basic->foreground);
    sprintf(label1,"dx= %-7.4g dy= %-7.4g l= %-7.4g s= %-7.4g         ",
    x2-x1,
    y2-y1,
    sqrt(pow((double)(x2-x1),(double)2.0) + pow((double)(y2-y1),(double)2.0)));
    if(x2-x1 == 0){
	sprintf(label,"%s s= Infinity    ",label1);
    } else {
	sprintf(label,"%s s= %-7.4g         ",label1,
	(y2-y1)/(x2-x1));
    }
    Text(basic,sx +3,sy,label);
    FreeLabels(basic);
    AddLabelString(basic,label,sx+3,sy,0.0,0.0,SCREEN_LBL,TEMPORARY_LBL,basic->fontname);
}

ShowSlopeCoords(basic,sx,sy,x1,y1,x2,y2)
BasicWindow	*basic;
int sx,sy;
float x1,y1;
float x2,y2;
{
char label[200];
char label1[200];

    SetColor(basic->foreground);
    sprintf(label1,"x= %-8.4g y= %-8.4g dx= %-7.4g dy= %-7.4g l= %-7.4g",
    x2,
    y2,
    x2-x1,
    y2-y1,
    sqrt(pow((double)(x2-x1),(double)2.0) + pow((double)(y2-y1),(double)2.0)));
    if(x2-x1 == 0){
	sprintf(label,"%s s= Infinity    ",label1);
    } else {
	sprintf(label,"%s s= %-7.4g         ",label1,
	(y2-y1)/(x2-x1));
    }
    Text(basic,sx +3,sy,label);
    FreeLabels(basic);
    AddLabelString(basic,label,sx+3,sy,0.0,0.0,SCREEN_LBL,TEMPORARY_LBL,basic->fontname);
}

ShowCursorCoords(basic,sx,sy,x,y)
BasicWindow	*basic;
int sx,sy;
int x,y;
{
char label[200];

    SetColor(basic->foreground);
    sprintf(label,"%5d %5d         ", x,y);
    Text(basic,sx +3,sy,label);
    FreeLabels(basic);
    AddLabelString(basic,label,sx+3,sy,0.0,0.0,SCREEN_LBL,TEMPORARY_LBL,basic->fontname);
}

ButtonPressAction(graph,buttonevent)
Graph			*graph;
XButtonPressedEvent	*buttonevent;
{
float	x,y;
float	x1,y1;
float	x2,y2;
int	sx1,sy1;
int	sx2,sy2;
int	button;
int	pt;
Plot	*p;
float	xoffset,yoffset;

    sx1 = buttonevent->x;
    sy1 = buttonevent->y;
    button = buttonevent->button;
    if((p=SelectedPlot(graph)) != NULL){
	xoffset = p->xoffset_axis;
	yoffset = p->yoffset_axis;
    } else {
	xoffset = 0;
	yoffset = 0;
    }
    switch(ButtonMode()){
    case DATAMODE:
	switch(button){
	case 1:
	    /*
	    ** mark the start of the drag location
	    */
	    graph->dragx1 = sx1;
	    graph->dragy1 = sy1;
	    break;
	case 2:
	    ClearWindow(graph->frame->text);
	    if(snapmode){
		/*
		** world coord xy readout of data 
		*/
		pt = Snap(graph,sx1,sy1,&x,&y);
		/*
		** get the screen coords of the data point
		*/
		ScreenTransform(graph,x,y,&sx2,&sy2);
		ShowCoords(graph,sx2,sy2,x-xoffset,y-yoffset);
		/*
		** mark the start of the drag location
		*/
		graph->dragx1 = sx2;
		graph->dragy1 = sy2;
	    } else {
		/*
		** world coord xy readout of cursor location
		*/
		WorldTransform(graph,sx1,sy1,&x,&y);
		ShowCoords(graph,sx1,sy1,x,y);
		/*
		** mark the start of the drag location
		*/
		graph->dragx1 = sx1;
		graph->dragy1 = sy1;
	    }
	    graph->dragwx1 = x;
	    graph->dragwy1 = y;
	    break;
	case 3:
	    /*
	    ** world coord xy readout closest above point
	    */
	    pt = Snap(graph,sx1,sy1,&x,&y);
	    /*
	    ** draw a line to the point on the plot (sx2,sy2)
	    */
	    ScreenTransform(graph,x,y,&sx2,&sy2);
	    /* avoid writing on the last pixel */
	    if(sy2 > sy1){
		sy2 -= 1; 
	    } else {
		sy2 += 1;
	    }
	    graph->dragx1 = sx1;
	    graph->dragy1 = sy1;
	    graph->dragx2 = sx2;
	    graph->dragy2 = sy2;
	    DrawSuperLine(graph,sx2,sy1,sx2,sy2);
	    ClearWindow(graph->frame->text);
	    NBShowPoint(graph->frame->text,0,graph->fontheight,x-xoffset,y-yoffset,pt);
	}
	break;
    case DRAWMODE:
	switch(button){
	case 1:
	    /*
	    ** mark the start of the line
	    */
	    graph->dragx1 = sx1;
	    graph->dragy1 = sy1;
	    WorldTransform(graph,sx1,sy1,&x,&y);
	    graph->dragwx1 = x;
	    graph->dragwy1 = y;
	    break;
	}
	break;
    case POLYLINEMODE:
	switch(button){
	case 1:
	    /*
	    ** if this is a new line then
	    ** mark the start of the line
	    ** otherwise finish the previous and then mark the start
	    */
	    if((graph->dragx1 != -1) && (graph->dragx2 != -1)){
		/*
		** mark the end of the drag line
		*/
		x1 = graph->dragwx1;
		y1 = graph->dragwy1;
		graph->dragx2 = sx1;
		graph->dragy2 = sy1;
		WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2);
		/*
		** draw a line through the points
		*/
		AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0,
		    WORLD_LBL,TEMPORARY_LBL,graph->fontname);
		/*
		** and reset the end of drag point
		*/
		graph->dragx2 = -1;
	    }
	    graph->dragx1 = sx1;
	    graph->dragy1 = sy1;
	    WorldTransform(graph,sx1,sy1,&x,&y);
	    graph->dragwx1 = x;
	    graph->dragwy1 = y;
	    break;
	case 2:			/* end draw line */
	    /*
	    ** mark the end of the drag line
	    */
	    x1 = graph->dragwx1;
	    y1 = graph->dragwy1;
	    graph->dragx2 = sx1;
	    graph->dragy2 = sy1;
	    WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2);
	    /*
	    ** draw a line through the points
	    */
	    AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0,
		WORLD_LBL,TEMPORARY_LBL,graph->fontname);
	    /*
	    ** set the drag coords to the invalid state
	    */
	    graph->dragx1 = -1;
	    graph->dragx2 = -1;
	    break;
	}
	break;
    case ZAPMODE:
	switch(button){
	case 1:
	    /*
	    ** delete the label
	    */
	    ZapLabel(graph,buttonevent);
	    break;
	}
	break;
    case PINCHMODE:
	switch(button){
	case 1:
	    /*
	    ** offset the plots
	    */
	    OffsetPlotGraphically(graph,buttonevent->x,buttonevent->y,0);
	    break;
	}
	break;
    }
}

ButtonReleaseAction(graph,buttonevent)
Graph			*graph;
XButtonReleasedEvent	*buttonevent;
{
int	button;
int	sx2,sy2;
int	sx1,sy1;
float	x1,y1,x2,y2;
int	pt;
Plot	*p;
float	xoffset,yoffset;

    button = buttonevent->button;
    sx2 = buttonevent->x;
    sy2 = buttonevent->y;
    if((p=SelectedPlot(graph)) != NULL){
	xoffset = p->xoffset_axis;
	yoffset = p->yoffset_axis;
    } else {
	xoffset = 0;
	yoffset = 0;
    }
    switch(ButtonMode()){
    case DATAMODE:
	switch(button){
	case 1:			/* zoom box */
	    /*
	    ** mark the end of the drag and zoom
	    */
	    graph->dragx2 = sx2;
	    graph->dragy2 = sy2;
	    WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1);
	    WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2);
	    /*
	    ** dont perform a zero scale action
	    */
	    if(x2 == x1 || y2 == y1) return;
	    if(x2 > x1){
		graph->wxmin = x1;
		graph->wxmax = x2;
	    } else {
		graph->wxmin = x2;
		graph->wxmax = x1;
	    }
	    if(y2 > y1){
		graph->wymin = y1;
		graph->wymax = y2;
	    } else {
		graph->wymin = y2;
		graph->wymax = y1;
	    }
	    ScaleAndRefreshGraph(graph);
	    /*
	    ** set the drag coords back to the invalid state
	    */
	    graph->dragx1 = -1;
	    graph->dragx2 = -1;
	    break;
	case 2:			/* ruler line */
	    /*
	    ** get the world coordinates of the starting point
	    */
	    x1 = graph->dragwx1;
	    y1 = graph->dragwy1;
	    if(sx2 != graph->dragx1 || sy2 != graph->dragy1){
		if(snapmode){
		    /*
		    ** display the final coords
		    */
		    pt = Snap(graph,sx2,sy2,&x2,&y2);
		    /*
		    ** compute the screen coordinate of the data point
		    */
		    ScreenTransform(graph,x2,y2,&sx1,&sy1);
		    ShowCoords(graph,sx1,sy1,x2-xoffset,y2-yoffset);
		} else {
		    /*
		    ** display the final coords
		    */
		    WorldTransform(graph,sx2,sy2,&x2,&y2);
		    ShowCoords(graph,sx2,sy2,x2,y2);
		}
		ClearWindow(graph->frame->text);
		ShowSlope(graph->frame->text,0,graph->fontheight,x1,y1,x2,y2);
		AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0,WORLD_LBL,TEMPORARY_LBL,graph->fontname);
	    }
	    /*
	    ** set the drag coords back to the invalid state
	    */
	    graph->dragx1 = -1;
	    graph->dragx2 = -1;
	    break;
	case 3:			/* drag line */
	    /*
	    ** erase the old line 
	    */
	    EraseSuperLine(graph,graph->dragx2,graph->dragy1,graph->dragx2,graph->dragy2);
	    /*
	    ** get the coords
	    */
	    pt = Snap(graph,sx2,sy2,&x2,&y2);
	    ShowCoords(graph,graph->dragx2,graph->dragy2,x2,y2);
	    graph->dragx1 = -1;
	    graph->dragx2 = -1;
	    break;
	}
	break;
    case DRAWMODE:
	switch(button){
	case 1:			/* draw line */
	    /*
	    ** mark the end of the drag and zoom
	    */
	    graph->dragx2 = sx2;
	    graph->dragy2 = sy2;
	    WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1);
	    WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2);
	    /*
	    ** draw a line between the points
	    */
	    AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0,WORLD_LBL,TEMPORARY_LBL,graph->fontname);
	    /*
	    ** set the drag coords back to the invalid state
	    */
	    graph->dragx1 = -1;
	    graph->dragx2 = -1;
	    break;
	}
	break;
    case POLYLINEMODE:
	switch(button){
	case 1:			/* draw line */
#ifdef OLD
	    /*
	    ** mark the end of the drag line
	    */
	    graph->dragx2 = sx2;
	    graph->dragy2 = sy2;
	    WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1);
	    WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2);
	    /*
	    ** draw a line through the points
	    */
	    AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0,
		WORLD_LBL,TEMPORARY_LBL,graph->fontname);
	    /*
	    ** set the new start drag coords to the end of the prev drag line 
	    */
	    graph->dragx1 = sx2;
	    graph->dragy1 = sy2;
	    graph->dragwx1 = x2;
	    graph->dragwy1 = y2;
#endif
	    break;
	}
	break;
    }
}

PointerMotionAction(graph,motionevent)
Graph			*graph;
XPointerMovedEvent	*motionevent;
{
int	sx2,sy2;
int	sx1,sy1;
int	tmpx2,tmpy2;
int	tmpx1,tmpy1;
int	button;
float	x1,y1;
float	x2,y2;
float	x,y;
extern int text_entry;
int	pt;
float	xoffset,yoffset;
Plot	*p;

    /*
    ** get the location of the cursor
    */
    sx2 = motionevent->x;
    sy2 = motionevent->y;
    if((p=SelectedPlot(graph)) != NULL){
	xoffset = p->xoffset_axis;
	yoffset = p->yoffset_axis;
    } else {
	xoffset = 0;
	yoffset = 0;
    }
    /*
    ** get the button pressed
    */
    button = (motionevent->state & 0xFF00) >> 8;
    switch(ButtonMode()){
    case PINCHMODE:
	/*
	** pinch at the cursor location
	*/
	WorldTransform(graph,sx2,sy2,&x2,&y2);
	NBShowCoords(graph->frame->text,0,graph->fontheight,x2,y2);
	break;
    case TEXTMODE:
	/*
	** if its in text mode then read out the cursor location
	*/
	ShowCursorCoords(graph->frame->text,0,graph->fontheight,sx2,sy2);
	break;
    case DRAWMODE:
	/*
	** if its in screen coord mode then read out the cursor location
	ShowCursorCoords(graph->frame->text,0,graph->fontheight,sx2,sy2);
	*/
	switch(button){
	case 1:				/* draw line */
	    if(graph->dragx1 != -1){
		if(graph->dragx2 != -1){
		    /*
		    ** erase the old drag line from the starting drag coordinates
		    ** drag->x1,y1 to the previous end coordinates drag->x2,y2.
		    */
		    EraseSuperLine(graph,
		    graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2);
		}
		/*
		** get the world coordinates of the initial drag point
		*/
		x1 = graph->dragwx1;
		y1 = graph->dragwy1;
		/*
		** get the world coordinates of the current cursor
		** location
		*/
		WorldTransform(graph,sx2,sy2,&x2,&y2);
		/*
		** update the drag coordinates to the current location
		*/
		graph->dragx2 = sx2; 
		graph->dragy2 = sy2;
		/*
		** display data about the line between the points in the
		** text window
		*/
		ShowSlopeCoords(graph->frame->text,0,graph->fontheight,x1,y1,x2,y2);
		/*
		** draw the new drag line
		*/
		DrawSuperLine(graph,
		graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2);
	    }
	    break;
	}
	break;
    case POLYLINEMODE:
	/*
	** if its in screen coord mode then read out the cursor location
	ShowCursorCoords(graph->frame->text,0,graph->fontheight,sx2,sy2);
	*/
	if(graph->dragx1 != -1){
	    if(graph->dragx2 != -1){
		/*
		** erase the old drag line from the starting drag coordinates
		** drag->x1,y1 to the previous end coordinates drag->x2,y2.
		*/
		EraseSuperLine(graph,
		graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2);
	    }
	    /*
	    ** get the world coordinates of the initial drag point
	    */
	    x1 = graph->dragwx1;
	    y1 = graph->dragwy1;
	    /*
	    ** get the world coordinates of the current cursor
	    ** location
	    */
	    WorldTransform(graph,sx2,sy2,&x2,&y2);
	    /*
	    ** update the drag coordinates to the current location
	    */
	    graph->dragx2 = sx2; 
	    graph->dragy2 = sy2;
	    /*
	    ** display data about the line between the points in the
	    ** text window
	    */
	    ShowSlopeCoords(graph->frame->text,0,graph->fontheight,x1,y1,x2,y2);
	    /*
	    ** draw the new drag line
	    */
	    DrawSuperLine(graph,
	    graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2);
	}
	break;
    case DATAMODE:
	switch(button){
	case 1:				/* zoom box */
	    if(graph->dragx1 != -1){
		if(graph->dragx2 != -1){
		    /*
		    ** erase the old drag rectangle
		    */
		    if(graph->dragy1 > graph->dragy2){
			tmpy1 = graph->dragy2;
			tmpy2 = graph->dragy1;
		    } else {
			tmpy2 = graph->dragy2;
			tmpy1 = graph->dragy1;
		    }
		    if(graph->dragx1 > graph->dragx2){
			tmpx1 = graph->dragx2;
			tmpx2 = graph->dragx1;
		    } else {
			tmpx2 = graph->dragx2;
			tmpx1 = graph->dragx1;
		    }
		    EraseSuperBox(graph,tmpx1,tmpy1,tmpx2,tmpy2);
		}
		graph->dragx2 = sx2; 
		graph->dragy2 = sy2;
		/*
		** draw the new drag rectangle
		*/
		if(graph->dragy1 > graph->dragy2){
		    tmpy1 = graph->dragy2;
		    tmpy2 = graph->dragy1;
		} else {
		    tmpy2 = graph->dragy2;
		    tmpy1 = graph->dragy1;
		}
		if(graph->dragx1 > graph->dragx2){
		    tmpx1 = graph->dragx2;
		    tmpx2 = graph->dragx1;
		} else {
		    tmpx2 = graph->dragx2;
		    tmpx1 = graph->dragx1;
		}
		DrawSuperBox(graph,tmpx1,tmpy1,tmpx2,tmpy2);
	    }
	    break;
	case 2:				/* ruler line */
	    if(graph->dragx1 != -1){
		if(graph->dragx2 != -1){
		    /*
		    ** erase the old drag line from the starting drag coordinates
		    ** drag->x1,y1 to the previous end coordinates drag->x2,y2.
		    */
		    EraseSuperLine(graph,
		    graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2);
		}
		/*
		** get the world coordinates of the initial drag point
		*/
		x1 = graph->dragwx1;
		y1 = graph->dragwy1;
		if(snapmode){
		    /*
		    ** get the data coordinates closest to the current cursor
		    ** location
		    */
		    pt = Snap(graph,sx2,sy2,&x2,&y2);
		    /*
		    ** compute the screen coordinate of the data point
		    */
		    ScreenTransform(graph,x2,y2,&sx1,&sy1);
		    /*
		    ** update the drag coordinates to the current location
		    */
		    graph->dragx2 = sx1; 
		    graph->dragy2 = sy1;
		} else {
		    /*
		    ** get the world coordinates of the current cursor
		    ** location
		    */
		    WorldTransform(graph,sx2,sy2,&x2,&y2);
		    /*
		    ** update the drag coordinates to the current location
		    */
		    graph->dragx2 = sx2; 
		    graph->dragy2 = sy2;
		}
		/*
		** display data about the line between the points in the
		** text window
		*/
		ShowSlopeCoords(graph->frame->text,0,graph->fontheight,x1-xoffset,y1-yoffset,x2-xoffset,y2-yoffset);
		/*
		** draw the new drag line
		*/
		DrawSuperLine(graph,
		graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2);
	    }
	    break;
	case 4:
	    /*
	    ** get the data point whose screen x coordinate is closest to the
	    ** current cursor location
	    */
	    pt = Snap(graph,sx2,sy2,&x,&y);
	    /*
	    ** compute the screen coordinate of the data point
	    */
	    ScreenTransform(graph,x,y,&sx1,&sy1);
	    /*
	    ** erase the previous line
	    */
	    EraseSuperLine(graph,
	    graph->dragx2,graph->dragy2,graph->dragx2,graph->dragy1);

	    /*
	    ** and draw a new line using the x coordinate of the data point
	    ** and the y coordinates of the cursor and the data point
	    */
	    graph->dragx1 = sx2;
	    graph->dragy1 = sy2;
	    graph->dragx2 = sx1;
	    graph->dragy2 = sy1;
	    DrawSuperLine(graph,sx1,sy1,sx1,sy2);
	    /*
	    ** display the data coordinates in the text window
	    */
	    NBShowPoint(graph->frame->text,0,graph->fontheight,x-xoffset,y-yoffset,pt);
	    break;
	}
	break;
    }
}
