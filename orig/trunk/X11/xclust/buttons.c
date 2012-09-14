#include	"xclust_ext.h"

int snapmode = 1;
static int buttonmode = DATAMODE;
static int fastsearch = TRUE;
extern float GetZoomPercent();
extern int lastsavedview;
extern int ProcessSavedView();
/* extern void UpdateRotationMatrix(double ***, float, float, float); */
/* extern void SetRotationMatrix(double ***, float, float, float); */
extern void AffineRotationMatrixEuler(double ***, float, float, float);
extern void AffineRotationMatrixAngleVector(double ***, float, float, float, float);
extern double ***ApplyAffineRotation(double ***, float, float, float, float);
extern void ApplyAffineTranslation(double ***, float , float , float );
extern double ***AffineRotateUp(double ***, float);
extern double ***AffineRotateRight(double ***, float);
static Time lastmotionevent;
extern float rotpercent;
extern int skipevent;
extern Label *commandlabel;
extern Label *coordlabel;
extern Label *rotlabel;

/* temporary offsets during Wobble mode (interactive 3d rotation) */ 
static int xval,yval,zval;

static ClusterBounds *currentbound = NULL;
static int closestpoint = -1;;
/* a bit of a hack to remember the axis show states */
static old_axis_show_x, old_axis_show_y;

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
int	estimated_start_i;
int	ecount;

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

NBShowPoint2(label,x,y,pt)
Label *label;
float x,y;
int	pt;
{

    sprintf(label->u.string.line,"n=%d x=%-8.4g y=%-8.4g",pt,x,y);

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

    sprintf(label,"x=%-g y=%-g",x,y);
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
    AddLabelBox(basic,0,0,0,x,y,2.0,WORLD_LBL,TEMPORARY_LBL);
    sprintf(label,"x=%-g y=%-g",x,y);
    SetColor(basic->foreground);
    Text(basic,sx +3,sy,label);
    AddLabelString(basic,label,sx+3,sy,x,y,WORLD_LBL,TEMPORARY_LBL,basic->fontname);
}

ShowSlope2(label,x1,y1,x2,y2)
Label *label;
float x1,y1;
float x2,y2;
{
char label1[200];
/*char label[200];*/

    sprintf(label1,"dx= %-7.4g dy= %-7.4g l= %-7.4g ",
    x2-x1,
    y2-y1,
    sqrt(pow((double)(x2-x1),(double)2.0) + pow((double)(y2-y1),(double)2.0)));
    if(x2-x1 == 0){
	sprintf(label->u.string.line,"%s s= Infinity",label1);
    } else {
	sprintf(label->u.string.line,"%s s= %-7.4g",label1,
	(y2-y1)/(x2-x1));
    }
}

ShowSlopeCoords2(label,x1,y1,x2,y2)
Label *label;
float x1,y1;
float x2,y2;
{
char label1[200];
/*char label1[200];*/

    sprintf(label1,"x= %-8.4g y= %-8.4g dx= %-7.4g dy= %-7.4g l= %-7.4g",
    x2,
    y2,
    x2-x1,
    y2-y1,
    sqrt(pow((double)(x2-x1),(double)2.0) + pow((double)(y2-y1),(double)2.0)));
    if(x2-x1 == 0){
	sprintf(label->u.string.line,"%s s= Infinity",label1);
    } else {
	sprintf(label->u.string.line,"%s s= %-7.4g",label1,
	(y2-y1)/(x2-x1));
    }
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

/* ButtonPressAction(graph,buttonevent) */
/* Graph			*graph; */
/* XButtonPressedEvent	*buttonevent; */
/* { */
/* float	x,y; */
/* float	x1,y1; */
/* float	x2,y2; */
/* int	sx1,sy1; */
/* int	sx2,sy2; */
/* int	button; */
/*  int state; */
/* int	pt; */
/* Plot	*p; */
/* float	xoffset,yoffset; */

/*     sx1 = buttonevent->x; */
/*     sy1 = buttonevent->y; */
/*     button = buttonevent->button; */
/*     state = buttonevent->state; */
/*     if(graph->button != 0){ */
/* 	/\* */
/* 	** another button is already down so implement the abort action */
/* 	*\/ */
/* 	graph->button = 0; */
/* 	return; */
/*     } */
/*     graph->button = button; */
/*     if((p=SelectedPlot(graph)) != NULL){ */
/* 	xoffset = p->xoffset_axis; */
/* 	yoffset = p->yoffset_axis; */
/*     } else { */
/* 	xoffset = 0; */
/* 	yoffset = 0; */
/*     } */


/*     switch(ButtonMode()){ */
/*     case DATAMODE: */
/* 	switch(button){ */
/* 	case 1: */
/* 	    /\* */
/* 	    ** mark the start of the drag location */
/* 	    *\/ */
/* 	    graph->dragx1 = sx1; */
/* 	    graph->dragy1 = sy1; */
/* 	    break; */
/* 	case 2: */
/* 	    ClearWindow(graph->frame->text); */
/* 	    if(snapmode){ */
/* 		/\* */
/* 		** world coord xy readout of data  */
/* 		*\/ */
/* 		pt = Snap(graph,sx1,sy1,&x,&y); */
/* 		/\* */
/* 		** get the screen coords of the data point */
/* 		*\/ */
/* 		ScreenTransform(graph,x,y,&sx2,&sy2); */
/* 		/\* */
/* 		** added correction for axis offsets in the displayed coords */
/* 		ShowCoords(graph,sx2,sy2,x,y); */
/* 		*\/ */
/* 		ShowCoords(graph,sx2,sy2,x-xoffset,y-yoffset); */
/* 		/\* */
/* 		** mark the start of the drag location */
/* 		*\/ */
/* 		graph->dragx1 = sx2; */
/* 		graph->dragy1 = sy2; */
/* 	    } else { */
/* 		/\* */
/* 		** world coord xy readout of cursor location */
/* 		*\/ */
/* 		WorldTransform(graph,sx1,sy1,&x,&y); */
/* 		ShowCoords(graph,sx1,sy1,x,y); */
/* 		/\* */
/* 		** mark the start of the drag location */
/* 		*\/ */
/* 		graph->dragx1 = sx1; */
/* 		graph->dragy1 = sy1; */
/* 	    } */
/* 	    graph->dragwx1 = x; */
/* 	    graph->dragwy1 = y; */
/* 	    break; */
/* 	case 3: */
/* 	    /\* */
/* 	    ** world coord xy readout closest above point */
/* 	    *\/ */
/* 	    pt = Snap(graph,sx1,sy1,&x,&y); */
/* 	    /\* */
/* 	    ** draw a line to the point on the plot (sx2,sy2) */
/* 	    *\/ */
/* 	    ScreenTransform(graph,x,y,&sx2,&sy2); */
/* 	    /\* avoid writing on the last pixel *\/ */
/* 	    if(sy2 > sy1){ */
/* 		sy2 -= 1;  */
/* 	    } else { */
/* 		sy2 += 1; */
/* 	    } */
/* 	    graph->dragx1 = sx1; */
/* 	    graph->dragy1 = sy1; */
/* 	    graph->dragx2 = sx2; */
/* 	    graph->dragy2 = sy2; */
/* 	    DrawSuperLine(graph,sx2,sy1,sx2,sy2,graph->supercolor); */
/* 	    ClearWindow(graph->frame->text); */
/* 	    NBShowPoint(graph->frame->text,0,graph->fontheight,x-xoffset,y-yoffset,pt); */
/* 	} */
/* 	break; */
/*     case DRAWMODE: */
/* 	switch(button){ */
/* 	case 1: */
/* 	    /\* */
/* 	    ** mark the start of the line */
/* 	    *\/ */
/* 	    graph->dragx1 = sx1; */
/* 	    graph->dragy1 = sy1; */
/* 	    WorldTransform(graph,sx1,sy1,&x,&y); */
/* 	    graph->dragwx1 = x; */
/* 	    graph->dragwy1 = y; */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case POLYLINEMODE: */
/* 	switch(button){ */
/* 	case 1: */
/* 	    /\* */
/* 	    ** if this is a new line then */
/* 	    ** mark the start of the line */
/* 	    ** otherwise finish the previous and then mark the start */
/* 	    *\/ */
/* 	    if((graph->dragx1 != -1) && (graph->dragx2 != -1)){ */
/* 		/\* */
/* 		** mark the end of the drag line */
/* 		*\/ */
/* 		x1 = graph->dragwx1; */
/* 		y1 = graph->dragwy1; */
/* 		graph->dragx2 = sx1; */
/* 		graph->dragy2 = sy1; */
/* 		WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2); */
/* 		/\* */
/* 		** draw a line through the points */
/* 		*\/ */
/* 		AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0, */
/* 		    WORLD_LBL,TEMPORARY_LBL); */
/* 		/\* */
/* 		** and reset the end of drag point */
/* 		*\/ */
/* 		graph->dragx2 = -1; */
/* 	    } */
/* 	    graph->dragx1 = sx1; */
/* 	    graph->dragy1 = sy1; */
/* 	    WorldTransform(graph,sx1,sy1,&x,&y); */
/* 	    graph->dragwx1 = x; */
/* 	    graph->dragwy1 = y; */
/* 	    break; */
/* 	case 2:			/\* end draw line *\/ */
/* 	    /\* */
/* 	    ** mark the end of the drag line */
/* 	    *\/ */
/* 	    x1 = graph->dragwx1; */
/* 	    y1 = graph->dragwy1; */
/* 	    graph->dragx2 = sx1; */
/* 	    graph->dragy2 = sy1; */
/* 	    WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2); */
/* 	    /\* */
/* 	    ** draw a line through the points */
/* 	    *\/ */
/* 	    AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0, */
/* 		WORLD_LBL,TEMPORARY_LBL,graph->fontname); */
/* 	    /\* */
/* 	    ** set the drag coords to the invalid state */
/* 	    *\/ */
/* 	    graph->dragx1 = -1; */
/* 	    graph->dragx2 = -1; */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case ZAPMODE: */
/* 	switch(button){ */
/* 	case 1: */
/* 	    /\* */
/* 	    ** delete the label */
/* 	    *\/ */
/* 	    ZapLabel(graph,buttonevent); */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case PINCHMODE: */
/* 	switch(button){ */
/* 	case 1: */
/* 	    /\* */
/* 	    ** offset the plots */
/* 	    *\/ */
/* 	    OffsetPlotGraphically(graph,buttonevent->x,buttonevent->y,0); */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case CLUSTERBOXMODE: */
/* 	switch(button){ */
/* 	case 1: */
/* 	    /\* */
/* 	    ** mark the start of the drag location */
/* 	    *\/ */
/* 	    graph->dragx1 = sx1; */
/* 	    graph->dragy1 = sy1; */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case CLUSTERPOLYLINEMODE: */
/* 	switch(button){ */
/* 	case 1: */
/* 	    /\* */
/* 	    ** if this is a new line then */
/* 	    ** mark the start of the line */
/* 	    ** otherwise finish the previous and then mark the start */
/* 	    *\/ */
/* 	    if((graph->dragx1 != -1) && (graph->dragx2 != -1)){ */
/* 		/\* */
/* 		** mark the end of the drag line */
/* 		*\/ */
/* 		x1 = graph->dragwx1; */
/* 		y1 = graph->dragwy1; */
/* 		graph->dragx2 = sx1; */
/* 		graph->dragy2 = sy1; */
/* 		WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2); */
/* 		/\* */
/* 		** and reset the end of drag point */
/* 		*\/ */
/* 		graph->dragx2 = -1; */
/* 	    } */
/* 	    if(graph->dragx1 == -1){ */
/* 		/\* */
/* 		** make a backup of the current cluster bounds */
/* 		*\/ */
/* 		BackupClusterBounds(graph); */
/* 	    } */
/* 	    graph->dragx1 = sx1; */
/* 	    graph->dragy1 = sy1; */
/* 	    WorldTransform(graph,sx1,sy1,&x,&y); */
/* 	    graph->dragwx1 = x; */
/* 	    graph->dragwy1 = y; */
/* 	    AssignClusterBounds(graph, */
/* 		graph->selectedcluster, */
/* 		graph->currentprojection[0], */
/* 		graph->currentprojection[1], */
/* 		-1,-1,x,y,INCOMPLETE); */
/* 	    break; */
/* 	case 2:			/\* end draw line *\/ */
/* 	    /\* */
/* 	    ** mark the end of the drag line */
/* 	    *\/ */
/* 	    x1 = graph->dragwx1; */
/* 	    y1 = graph->dragwy1; */
/* 	    graph->dragx2 = sx1; */
/* 	    graph->dragy2 = sy1; */
/* 	    WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2); */
/* 	    AssignClusterBounds(graph, */
/* 		graph->selectedcluster, */
/* 		graph->currentprojection[0], */
/* 		graph->currentprojection[1], */
/* 		-1,-1,x2,y2,COMPLETE); */
/* 	    SetClusterModified(graph,graph->selectedcluster,1); */
/* 	    AssignClusterPointsToCluster(graph,graph->selectedcluster); */
/* 	    /\* */
/* 	    ** and refresh the display */
/* 	    *\/ */
/* 	    ScaleAndRefreshGraph(graph); */
/* 	    /\* */
/* 	    ** set the drag coords to the invalid state */
/* 	    *\/ */
/* 	    graph->dragx1 = -1; */
/* 	    graph->dragx2 = -1; */
/* 	    break; */
/* 	} */
/* 	break; */
/*     } */
/* } */

/* ButtonReleaseAction(graph,buttonevent) */
/* Graph			*graph; */
/* XButtonReleasedEvent	*buttonevent; */
/* { */
/* int	button; */
/* int	sx2,sy2; */
/* int	sx1,sy1; */
/* float	x1,y1,x2,y2; */
/* int	pt; */
/* Plot	*p; */
/* float	xoffset,yoffset; */

/*     button = buttonevent->button; */
/*     if(graph->button == 0){ */
/* 	graph->dragx1 = -1; */
/* 	graph->dragx2 = -1; */
/* 	/\* */
/* 	** this is the abort condition */
/* 	*\/ */
/* 	RefreshGraph(graph); */
/* 	return; */
/*     } */
/*     /\* */
/*     ** indicate that no button is being pressed */
/*     *\/ */
/*     graph->button = 0; */
/*     sx2 = buttonevent->x; */
/*     sy2 = buttonevent->y; */
/*     if((p=SelectedPlot(graph)) != NULL){ */
/* 	xoffset = p->xoffset_axis; */
/* 	yoffset = p->yoffset_axis; */
/*     } else { */
/* 	xoffset = 0; */
/* 	yoffset = 0; */
/*     } */
/*     switch(ButtonMode()){ */
/*     case DATAMODE: */
/* 	switch(button){ */
/* 	case 1:			/\* zoom box *\/ */
/* 	    /\* */
/* 	    ** mark the end of the drag and zoom */
/* 	    *\/ */
/* 	    graph->dragx2 = sx2; */
/* 	    graph->dragy2 = sy2; */
/* 	    WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1); */
/* 	    WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2); */
/* 	    /\* */
/* 	    ** dont perform a zero scale action */
/* 	    *\/ */
/* 	    if(x2 == x1 || y2 == y1) return; */
/* 	    if(x2 > x1){ */
/* 		graph->wxmin = x1; */
/* 		graph->wxmax = x2; */
/* 	    } else { */
/* 		graph->wxmin = x2; */
/* 		graph->wxmax = x1; */
/* 	    } */
/* 	    if(y2 > y1){ */
/* 		graph->wymin = y1; */
/* 		graph->wymax = y2; */
/* 	    } else { */
/* 		graph->wymin = y2; */
/* 		graph->wymax = y1; */
/* 	    } */
/* 	    ScaleAndRefreshGraph(graph); */
/* 	    /\* */
/* 	    ** set the drag coords back to the invalid state */
/* 	    *\/ */
/* 	    graph->dragx1 = -1; */
/* 	    graph->dragx2 = -1; */
/* 	    break; */
/* 	case 2:			/\* ruler line *\/ */
/* 	    /\* */
/* 	    ** get the world coordinates of the starting point */
/* 	    *\/ */
/* 	    x1 = graph->dragwx1; */
/* 	    y1 = graph->dragwy1; */
/* 	    if(sx2 != graph->dragx1 || sy2 != graph->dragy1){ */
/* 		if(snapmode){ */
/* 		    /\* */
/* 		    ** display the final coords */
/* 		    *\/ */
/* 		    pt = Snap(graph,sx2,sy2,&x2,&y2); */
/* 		    /\* */
/* 		    ** compute the screen coordinate of the data point */
/* 		    *\/ */
/* 		    ScreenTransform(graph,x2,y2,&sx1,&sy1); */
/* 		    ShowCoords(graph,sx1,sy1,x2-xoffset,y2-yoffset); */
/* 		} else { */
/* 		    /\* */
/* 		    ** display the final coords */
/* 		    *\/ */
/* 		    WorldTransform(graph,sx2,sy2,&x2,&y2); */
/* 		    ShowCoords(graph,sx2,sy2,x2,y2); */
/* 		} */
/* 		ClearWindow(graph->frame->text); */
/* 		ShowSlope(graph->frame->text,0,graph->fontheight,x1,y1,x2,y2); */
/* 		AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0,WORLD_LBL, */
/* 		TEMPORARY_LBL,graph->fontname); */
/* 	    } */
/* 	    /\* */
/* 	    ** set the drag coords back to the invalid state */
/* 	    *\/ */
/* 	    graph->dragx1 = -1; */
/* 	    graph->dragx2 = -1; */
/* 	    break; */
/* 	case 3:			/\* drag line *\/ */
/* 	    /\* */
/* 	    ** erase the old line  */
/* 	    *\/ */
/* 	    EraseSuperLine(graph,graph->dragx2,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor); */
/* 	    /\* */
/* 	    ** get the coords */
/* 	    *\/ */
/* 	    pt = Snap(graph,sx2,sy2,&x2,&y2); */
/* 	    ShowCoords(graph,graph->dragx2,graph->dragy2,x2-xoffset,y2-yoffset); */
/* 	    graph->dragx1 = -1; */
/* 	    graph->dragx2 = -1; */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case DRAWMODE: */
/* 	switch(button){ */
/* 	case 1:			/\* draw line *\/ */
/* 	    /\* */
/* 	    ** mark the end of the drag and zoom */
/* 	    *\/ */
/* 	    graph->dragx2 = sx2; */
/* 	    graph->dragy2 = sy2; */
/* 	    WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1); */
/* 	    WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2); */
/* 	    /\* */
/* 	    ** draw a line between the points */
/* 	    *\/ */
/* 	    AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0,WORLD_LBL, */
/* 	    TEMPORARY_LBL); */
/* 	    /\* */
/* 	    ** set the drag coords back to the invalid state */
/* 	    *\/ */
/* 	    graph->dragx1 = -1; */
/* 	    graph->dragx2 = -1; */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case POLYLINEMODE: */
/* 	switch(button){ */
/* 	case 1:			/\* draw line *\/ */
/* #ifdef OLD */
/* 	    /\* */
/* 	    ** mark the end of the drag line */
/* 	    *\/ */
/* 	    graph->dragx2 = sx2; */
/* 	    graph->dragy2 = sy2; */
/* 	    WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1); */
/* 	    WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2); */
/* 	    /\* */
/* 	    ** draw a line through the points */
/* 	    *\/ */
/* 	    AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0, */
/* 		WORLD_LBL,TEMPORARY_LBL,graph->fontname); */
/* 	    /\* */
/* 	    ** set the new start drag coords to the end of the prev drag line  */
/* 	    *\/ */
/* 	    graph->dragx1 = sx2; */
/* 	    graph->dragy1 = sy2; */
/* 	    graph->dragwx1 = x2; */
/* 	    graph->dragwy1 = y2; */
/* #endif */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case CLUSTERBOXMODE: */
/* 	switch(button){ */
/* 	case 1:			/\* cluster box *\/ */
/* 	    /\* */
/* 	    ** mark the end of the cluster box drag  */
/* 	    *\/ */
/* 	    graph->dragx2 = sx2; */
/* 	    graph->dragy2 = sy2; */
/* 	    WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1); */
/* 	    WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2); */
/* 	    /\* */
/* 	    ** dont make a zero sized box */
/* 	    *\/ */
/* 	    if(x2 == x1 || y2 == y1){ */
/* 		graph->dragx1 = -1; */
/* 		graph->dragx2 = -1; */
/* 		return; */
/* 	    } */
/* 	    /\* */
/* 	    ** make a backup of the current cluster bounds */
/* 	    *\/ */
/* 	    BackupClusterBounds(graph); */
/* 	    /\* */
/* 	    ** assign points to the cluster  */
/* 	    *\/ */
/* 	    AssignClusterBounds(graph,graph->selectedcluster, */
/* 		graph->currentprojection[0],graph->currentprojection[1], */
/* 		4,0,x1,y1,INCOMPLETE); */
/* 	    AssignClusterBounds(graph,graph->selectedcluster, */
/* 		graph->currentprojection[0],graph->currentprojection[1], */
/* 		4,1,x2,y1,INCOMPLETE); */
/* 	    AssignClusterBounds(graph,graph->selectedcluster, */
/* 		graph->currentprojection[0],graph->currentprojection[1], */
/* 		4,2,x2,y2,INCOMPLETE); */
/* 	    AssignClusterBounds(graph,graph->selectedcluster, */
/* 		graph->currentprojection[0],graph->currentprojection[1], */
/* 		4,3,x1,y2,COMPLETE); */
/* 	    SetClusterModified(graph,graph->selectedcluster,1); */
/* 	    AssignClusterPointsToCluster(graph,graph->selectedcluster); */
/* 	    /\* */
/* 	    ** and refresh the display */
/* 	    *\/ */
/* 	    ScaleAndRefreshGraph(graph); */
/* 	    /\* */
/* 	    ** set the drag coords back to the invalid state */
/* 	    *\/ */
/* 	    graph->dragx1 = -1; */
/* 	    graph->dragx2 = -1; */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case CLUSTERPOLYLINEMODE: */
/* 	switch(button){ */
/* 	case 1:			/\* draw line *\/ */
/* 	    break; */
/* 	} */
/* 	break; */
/*     } */
/* } */

/* PointerMotionAction(graph,motionevent) */
/* Graph			*graph; */
/* XPointerMovedEvent	*motionevent; */
/* { */
/* int	sx2,sy2; */
/* int	sx1,sy1; */
/* int	tmpx2,tmpy2; */
/* int	tmpx1,tmpy1; */
/* int	button; */
/* float	x1,y1; */
/* float	x2,y2; */
/* float	x,y; */
/* extern int text_entry; */
/* int	pt; */
/* float	xoffset,yoffset; */
/* Plot	*p; */

/*     /\* */
/*     ** get the location of the cursor */
/*     *\/ */
/*     sx2 = motionevent->x; */
/*     sy2 = motionevent->y; */
/*     if((p=SelectedPlot(graph)) != NULL){ */
/* 	xoffset = p->xoffset_axis; */
/* 	yoffset = p->yoffset_axis; */
/*     } else { */
/* 	xoffset = 0; */
/* 	yoffset = 0; */
/*     } */
/*     /\* */
/*     ** get the button pressed */
/*     *\/ */
/*     button = (motionevent->state & 0xFF00) >> 8; */
/*     switch(ButtonMode()){ */
/*     case PINCHMODE: */
/* 	/\* */
/* 	** pinch at the cursor location */
/* 	*\/ */
/* 	WorldTransform(graph,sx2,sy2,&x2,&y2); */
/* 	NBShowCoords(graph->frame->text,0,graph->fontheight,x2,y2); */
/* 	break; */
/*     case TEXTMODE: */
/* 	/\* */
/* 	** if its in text mode then read out the cursor location */
/* 	*\/ */
/* 	ShowCursorCoords(graph->frame->text,0,graph->fontheight,sx2,sy2); */
/* 	break; */
/*     case DRAWMODE: */
/* 	/\* */
/* 	** if its in screen coord mode then read out the cursor location */
/* 	ShowCursorCoords(graph->frame->text,0,graph->fontheight,sx2,sy2); */
/* 	*\/ */
/* 	switch(button){ */
/* 	case 1:				/\* draw line *\/ */
/* 	    if(graph->dragx1 != -1){ */
/* 		if(graph->dragx2 != -1){ */
/* 		    /\* */
/* 		    ** erase the old drag line from the starting drag coordinates */
/* 		    ** drag->x1,y1 to the previous end coordinates drag->x2,y2. */
/* 		    *\/ */
/* 		    EraseSuperLine(graph, */
/* 		    graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor); */
/* 		} */
/* 		/\* */
/* 		** get the world coordinates of the initial drag point */
/* 		*\/ */
/* 		x1 = graph->dragwx1; */
/* 		y1 = graph->dragwy1; */
/* 		/\* */
/* 		** get the world coordinates of the current cursor */
/* 		** location */
/* 		*\/ */
/* 		WorldTransform(graph,sx2,sy2,&x2,&y2); */
/* 		/\* */
/* 		** update the drag coordinates to the current location */
/* 		*\/ */
/* 		graph->dragx2 = sx2;  */
/* 		graph->dragy2 = sy2; */
/* 		/\* */
/* 		** display data about the line between the points in the */
/* 		** text window */
/* 		*\/ */
/* 		ShowSlopeCoords(graph->frame->text,0,graph->fontheight,x1,y1,x2,y2); */
/* 		/\* */
/* 		** draw the new drag line */
/* 		*\/ */
/* 		DrawSuperLine(graph, */
/* 		graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor); */
/* 	    } */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case POLYLINEMODE: */
/* 	/\* */
/* 	** if its in screen coord mode then read out the cursor location */
/* 	ShowCursorCoords(graph->frame->text,0,graph->fontheight,sx2,sy2); */
/* 	*\/ */
/* 	if(graph->dragx1 != -1){ */
/* 	    if(graph->dragx2 != -1){ */
/* 		/\* */
/* 		** erase the old drag line from the starting drag coordinates */
/* 		** drag->x1,y1 to the previous end coordinates drag->x2,y2. */
/* 		*\/ */
/* 		EraseSuperLine(graph, */
/* 		graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor); */
/* 	    } */
/* 	    /\* */
/* 	    ** get the world coordinates of the initial drag point */
/* 	    *\/ */
/* 	    x1 = graph->dragwx1; */
/* 	    y1 = graph->dragwy1; */
/* 	    /\* */
/* 	    ** get the world coordinates of the current cursor */
/* 	    ** location */
/* 	    *\/ */
/* 	    WorldTransform(graph,sx2,sy2,&x2,&y2); */
/* 	    /\* */
/* 	    ** update the drag coordinates to the current location */
/* 	    *\/ */
/* 	    graph->dragx2 = sx2;  */
/* 	    graph->dragy2 = sy2; */
/* 	    /\* */
/* 	    ** display data about the line between the points in the */
/* 	    ** text window */
/* 	    *\/ */
/* 	    ShowSlopeCoords(graph->frame->text,0,graph->fontheight,x1,y1,x2,y2); */
/* 	    /\* */
/* 	    ** draw the new drag line */
/* 	    *\/ */
/* 	    DrawSuperLine(graph, */
/* 	    graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor); */
/* 	} */
/* 	break; */
/*     case DATAMODE: */
/* 	switch(button){ */
/* 	case 1:				/\* zoom box *\/ */
/* 	    if(graph->dragx1 != -1){ */
/* 		if(graph->dragx2 != -1){ */
/* 		    /\* */
/* 		    ** erase the old drag rectangle */
/* 		    *\/ */
/* 		    if(graph->dragy1 > graph->dragy2){ */
/* 			tmpy1 = graph->dragy2; */
/* 			tmpy2 = graph->dragy1; */
/* 		    } else { */
/* 			tmpy2 = graph->dragy2; */
/* 			tmpy1 = graph->dragy1; */
/* 		    } */
/* 		    if(graph->dragx1 > graph->dragx2){ */
/* 			tmpx1 = graph->dragx2; */
/* 			tmpx2 = graph->dragx1; */
/* 		    } else { */
/* 			tmpx2 = graph->dragx2; */
/* 			tmpx1 = graph->dragx1; */
/* 		    } */
/* 		    EraseSuperBox(graph,tmpx1,tmpy1,tmpx2,tmpy2,graph->supercolor); */
/* 		} */
/* 		graph->dragx2 = sx2;  */
/* 		graph->dragy2 = sy2; */
/* 		/\* */
/* 		** draw the new drag rectangle */
/* 		*\/ */
/* 		if(graph->dragy1 > graph->dragy2){ */
/* 		    tmpy1 = graph->dragy2; */
/* 		    tmpy2 = graph->dragy1; */
/* 		} else { */
/* 		    tmpy2 = graph->dragy2; */
/* 		    tmpy1 = graph->dragy1; */
/* 		} */
/* 		if(graph->dragx1 > graph->dragx2){ */
/* 		    tmpx1 = graph->dragx2; */
/* 		    tmpx2 = graph->dragx1; */
/* 		} else { */
/* 		    tmpx2 = graph->dragx2; */
/* 		    tmpx1 = graph->dragx1; */
/* 		} */
/* 		DrawSuperBox(graph,tmpx1,tmpy1,tmpx2,tmpy2,graph->supercolor); */
/* 	    } */
/* 	    break; */
/* 	case 2:				/\* ruler line *\/ */
/* 	    if(graph->dragx1 != -1){ */
/* 		if(graph->dragx2 != -1){ */
/* 		    /\* */
/* 		    ** erase the old drag line from the starting drag coordinates */
/* 		    ** drag->x1,y1 to the previous end coordinates drag->x2,y2. */
/* 		    *\/ */
/* 		    EraseSuperLine(graph, */
/* 		    graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor); */
/* 		} */
/* 		/\* */
/* 		** get the world coordinates of the initial drag point */
/* 		*\/ */
/* 		x1 = graph->dragwx1; */
/* 		y1 = graph->dragwy1; */
/* 		if(snapmode){ */
/* 		    /\* */
/* 		    ** get the data coordinates closest to the current cursor */
/* 		    ** location */
/* 		    *\/ */
/* 		    pt = Snap(graph,sx2,sy2,&x2,&y2); */
/* 		    /\* */
/* 		    ** compute the screen coordinate of the data point */
/* 		    *\/ */
/* 		    ScreenTransform(graph,x2,y2,&sx1,&sy1); */
/* 		    /\* */
/* 		    ** update the drag coordinates to the current location */
/* 		    *\/ */
/* 		    graph->dragx2 = sx1;  */
/* 		    graph->dragy2 = sy1; */
/* 		} else { */
/* 		    /\* */
/* 		    ** get the world coordinates of the current cursor */
/* 		    ** location */
/* 		    *\/ */
/* 		    WorldTransform(graph,sx2,sy2,&x2,&y2); */
/* 		    /\* */
/* 		    ** update the drag coordinates to the current location */
/* 		    *\/ */
/* 		    graph->dragx2 = sx2;  */
/* 		    graph->dragy2 = sy2; */
/* 		} */
/* 		/\* */
/* 		** display data about the line between the points in the */
/* 		** text window */
/* 		*\/ */
/* 		ShowSlopeCoords(graph->frame->text,0,graph->fontheight,x1-xoffset,y1-yoffset,x2-xoffset,y2-yoffset); */
/* 		/\* */
/* 		** draw the new drag line */
/* 		*\/ */
/* 		DrawSuperLine(graph, */
/* 		graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor); */
/* 	    } */
/* 	    break; */
/* 	case 4: */
/* 	    /\* */
/* 	    ** get the data point whose screen x coordinate is closest to the */
/* 	    ** current cursor location */
/* 	    *\/ */
/* 	    pt = Snap(graph,sx2,sy2,&x,&y); */
/* 	    /\* */
/* 	    ** compute the screen coordinate of the data point */
/* 	    *\/ */
/* 	    ScreenTransform(graph,x,y,&sx1,&sy1); */
/* 	    /\* */
/* 	    ** erase the previous line */
/* 	    *\/ */
/* 	    EraseSuperLine(graph, */
/* 	    graph->dragx2,graph->dragy2,graph->dragx2,graph->dragy1,graph->supercolor); */

/* 	    /\* */
/* 	    ** and draw a new line using the x coordinate of the data point */
/* 	    ** and the y coordinates of the cursor and the data point */
/* 	    *\/ */
/* 	    graph->dragx1 = sx2; */
/* 	    graph->dragy1 = sy2; */
/* 	    graph->dragx2 = sx1; */
/* 	    graph->dragy2 = sy1; */
/* 	    DrawSuperLine(graph,sx1,sy1,sx1,sy2,graph->supercolor); */
/* 	    /\* */
/* 	    ** display the data coordinates in the text window */
/* 	    *\/ */
/* 	    NBShowPoint(graph->frame->text,0,graph->fontheight,x-xoffset,y-yoffset,pt); */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case CLUSTERBOXMODE: */
/* 	switch(button){ */
/* 	case 1:				/\* draw cluster box *\/ */
/* 	    if(graph->dragx1 != -1){ */
/* 		if(graph->dragx2 != -1){ */
/* 		    /\* */
/* 		    ** erase the old drag rectangle */
/* 		    *\/ */
/* 		    if(graph->dragy1 > graph->dragy2){ */
/* 			tmpy1 = graph->dragy2; */
/* 			tmpy2 = graph->dragy1; */
/* 		    } else { */
/* 			tmpy2 = graph->dragy2; */
/* 			tmpy1 = graph->dragy1; */
/* 		    } */
/* 		    if(graph->dragx1 > graph->dragx2){ */
/* 			tmpx1 = graph->dragx2; */
/* 			tmpx2 = graph->dragx1; */
/* 		    } else { */
/* 			tmpx2 = graph->dragx2; */
/* 			tmpx1 = graph->dragx1; */
/* 		    } */
/* 		    EraseSuperBox(graph,tmpx1,tmpy1, tmpx2,tmpy2,graph->supercolor); */
/* 		} */
/* 		graph->dragx2 = sx2;  */
/* 		graph->dragy2 = sy2; */
/* 		/\* */
/* 		** draw the new drag rectangle */
/* 		*\/ */
/* 		if(graph->dragy1 > graph->dragy2){ */
/* 		    tmpy1 = graph->dragy2; */
/* 		    tmpy2 = graph->dragy1; */
/* 		} else { */
/* 		    tmpy2 = graph->dragy2; */
/* 		    tmpy1 = graph->dragy1; */
/* 		} */
/* 		if(graph->dragx1 > graph->dragx2){ */
/* 		    tmpx1 = graph->dragx2; */
/* 		    tmpx2 = graph->dragx1; */
/* 		} else { */
/* 		    tmpx2 = graph->dragx2; */
/* 		    tmpx1 = graph->dragx1; */
/* 		} */
/* 		DrawSuperBox(graph,tmpx1,tmpy1, tmpx2,tmpy2,graph->supercolor); */
/* 	    } */
/* 	    break; */
/* 	} */
/* 	break; */
/*     case CLUSTERPOLYLINEMODE: */
/* 	/\* */
/* 	** if its in screen coord mode then read out the cursor location */
/* 	ShowCursorCoords(graph->frame->text,0,graph->fontheight,sx2,sy2); */
/* 	*\/ */
/* 	if(graph->dragx1 != -1){ */
/* 	    if(graph->dragx2 != -1){ */
/* 		/\* */
/* 		** erase the old drag line from the starting drag coordinates */
/* 		** drag->x1,y1 to the previous end coordinates drag->x2,y2. */
/* 		*\/ */
/* 		EraseSuperLine(graph, */
/* 		graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor); */
/* 	    } */
/* 	    /\* */
/* 	    ** get the world coordinates of the initial drag point */
/* 	    *\/ */
/* 	    x1 = graph->dragwx1; */
/* 	    y1 = graph->dragwy1; */
/* 	    /\* */
/* 	    ** get the world coordinates of the current cursor */
/* 	    ** location */
/* 	    *\/ */
/* 	    WorldTransform(graph,sx2,sy2,&x2,&y2); */
/* 	    /\* */
/* 	    ** update the drag coordinates to the current location */
/* 	    *\/ */
/* 	    graph->dragx2 = sx2;  */
/* 	    graph->dragy2 = sy2; */
/* 	    /\* */
/* 	    ** display data about the line between the points in the */
/* 	    ** text window */
/* 	    *\/ */
/* 	    ShowSlopeCoords(graph->frame->text,0,graph->fontheight,x1,y1,x2,y2); */
/* 	    /\* */
/* 	    ** draw the new drag line */
/* 	    *\/ */
/* 	    DrawSuperLine(graph, */
/* 	    graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor); */
/* 	} */
/* 	break; */
/*     } */
/* } */




ButtonPressAction(Graph *graph, XButtonPressedEvent *buttonevent)
{

  int sx1,sy1;
  int sx2,sy2;
  int button;
  int state;
/*   float xoffset,yoffset; */
  Plot *p;
  float	x,y;
  float	x1,y1;
  float	x2,y2;
  char *clmode;
  int rotpfreq;
  ClusterBounds *cb;
  int dd, pnt;
  Label *label;
  int pt;


  /* screen coordinates of mouse click */
  sx1 = buttonevent->x;
  sy1 = buttonevent->y;

  /* button pressed */
  button = buttonevent->button;
  /* modifier key state */
  state = buttonevent->state;

  /* already another button down? */
  if(graph->button != 0){
    return; /* don't process it */
  }

  /* remember the button pressed */
  graph->button = button;

  p=SelectedPlot(graph);

  /* ?? */
/*   if((p=SelectedPlot(graph)) != NULL){ */
/*     xoffset = p->xoffset_axis; */
/*     yoffset = p->yoffset_axis; */
/*   } else { */
/*     xoffset = 0; */
/*     yoffset = 0; */
/*   } */

  switch (button) {

  case 1: /*left mouse button*/
    switch (ButtonMode()) {
    case CLUSTERPOLYLINEMODE:
      if (debug) {
	fprintf(stderr, "clusterpolylinemode\n");
	fprintf(stderr, "next line segment\n");
      }

      /*
      ** mark the end of the drag line
      */
      x1 = graph->dragwx1;
      y1 = graph->dragwy1;
      graph->dragx2 = sx1;
      graph->dragy2 = sy1;
      WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2);
      /*
      ** and reset the end of drag point
      */
      graph->dragx2 = -1;

      graph->dragx1 = sx1;
      graph->dragy1 = sy1;
      WorldTransform(graph,sx1,sy1,&x,&y);
      graph->dragwx1 = x;
      graph->dragwy1 = y;
      AssignClusterBounds(graph,
			  graph->selectedcluster,
			  graph->currentprojection[0],
			  graph->currentprojection[1],
			  -1,-1,x,y,INCOMPLETE);
      
      break;
    case POLYLINEMODE:
      if (debug) {
	fprintf(stderr, "polylinemode\n");
	fprintf(stderr, "next line segment\n");
      }
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
		   WORLD_LBL,TEMPORARY_LBL);
      /*
      ** and reset the end of drag point
      */
      graph->dragx2 = -1;
      
      graph->dragx1 = sx1;
      graph->dragy1 = sy1;
      WorldTransform(graph,sx1,sy1,&x,&y);
      graph->dragwx1 = x;
      graph->dragwy1 = y;

      break;
    case DATAMODE:

      /* shift-left-click = edit a point, if it's close enough */
      if (state&1 && !(state&4)) { /* shift, not control */
	if (debug) {
	  fprintf(stderr, "shift, not control\n");
	  fprintf(stderr, "start edit bound\n");
	}
	if (graph->clusterbounds==NULL)
				   break;

	for(cb=graph->clusterbounds;cb;cb=cb->next){
	  if(cb->clusterid == graph->selectedcluster && 
	     ( (cb->projection[0] == graph->currentprojection[0] && 
	     cb->projection[1] == graph->currentprojection[1]) || (cb->projection[0] == graph->currentprojection[1] && 
	     cb->projection[1] == graph->currentprojection[0]) ) ) {

	    for(pnt = 0; pnt<cb->ncoords; pnt++) {
		dd = (sx1-cb->coord[pnt].x)*(sx1-cb->coord[pnt].x) + 
		  (sy1-cb->coord[pnt].y)*(sy1-cb->coord[pnt].y);
	      if (dd<25) {
		NormalSelectInputs(G);
		FleurCursor(F);
		SetButtonMode(EDITBOUNDMODE);
		currentbound = cb;
		closestpoint = pnt;
		graph->dragx1 = sx1;
		graph->dragy1 = sy1;

		if (pnt==0) {
		  EraseSuperLine(graph, 
				 cb->coord[pnt].x,
				 cb->coord[pnt].y,
				 cb->coord[cb->ncoords-1].x,
				 cb->coord[cb->ncoords-1].y,
				 graph->supercolor);
		  DrawSuperLine(graph, 
				sx1,
				sy1,
				cb->coord[cb->ncoords-1].x,
				cb->coord[cb->ncoords-1].y,
				graph->supercolor);
		}
		else {
		  EraseSuperLine(graph, 
				 cb->coord[pnt].x,
				 cb->coord[pnt].y,
				 cb->coord[pnt-1].x,
				 cb->coord[pnt-1].y,
				 graph->supercolor);
		  DrawSuperLine(graph, 
				sx1, 
				sy1,
				cb->coord[pnt-1].x,
				cb->coord[pnt-1].y,
				graph->supercolor);
		}
		if (pnt==cb->ncoords-1) {
		  EraseSuperLine(graph, 
				 cb->coord[pnt].x,
				 cb->coord[pnt].y,
				 cb->coord[0].x,
				 cb->coord[0].y,
				 graph->supercolor);
		  DrawSuperLine(graph, 
				sx1,
				sy1,
				cb->coord[0].x,
				cb->coord[0].y,
				graph->supercolor);
		}
		else {
		  EraseSuperLine(graph, 
				 cb->coord[pnt].x,
				 cb->coord[pnt].y,
				 cb->coord[pnt+1].x,
				 cb->coord[pnt+1].y,
				 graph->supercolor);
		  DrawSuperLine(graph,
				sx1,
				sy1,
				cb->coord[pnt+1].x,
				cb->coord[pnt+1].y,
				graph->supercolor);
		}


		return;
	      }
	    }
	  }
	}

	
      } else if ( !(state&1) && state&4 ) { /* control, not shift */

      /* ctrl-left-click: enter 3-D 'wobble' mode */
	if (debug) {
	  fprintf(stderr, "control, not shift\n");
	  fprintf(stderr, "wobble\n");
	}
	InteractiveSelectInputs(G);
	SetButtonMode(WOBBLEMODE);
	FleurCursor(F);

	lastmotionevent = buttonevent->time;

	/*for now only store/set the pointfreq of the current plot */
	rotpfreq = graph->rotpointfreq;
	graph->rotpointfreq = p->pointfreq;
	p->pointfreq = rotpfreq;
	/*AffineIdentityMatrix(&graph->matrix);*/
	AffineRotationMatrixEuler(&graph->matrix, graph->thetax, graph->thetay, graph->thetaz);

/* 	/\* turn off axes and shift origin to screen center *\/ */
	old_axis_show_x = graph->xaxis.show;
	old_axis_show_y = graph->yaxis.show;
	graph->xaxis.show = graph->yaxis.show = 0;

	label = (Label*) AddLabelLine(graph, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.1*(p->xmax-p->xmin), 0.0, 0.0,WORLD_LBL,TEMPORARY_LBL);
	label->color = MININTERFACECOLOR + XAXIS;
	label->name = "3daxis";
	label = (Label*) AddLabelLine(graph, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0,  0.1*(p->ymax-p->ymin), 0.0,WORLD_LBL,TEMPORARY_LBL);
	label->color = MININTERFACECOLOR + YAXIS;
	label->name = "3daxis";
	label = (Label*) AddLabelLine(graph, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0,  0.1*(p->zmax-p->zmin),WORLD_LBL,TEMPORARY_LBL);
	label->color = MININTERFACECOLOR + ZAXIS;
	label->name = "3daxis";

/* 	xval = (graph->wxmax + graph->wxmin) / 3; */
/* 	yval = (graph->wymax + graph->wymin) / 3; */
/* 	zval = xval; */

/* /\* 	fprintf(stderr, "xval: %d ; yval: %d ; zval: %d\n",xval,yval,zval); *\/ */

/* 	graph->plot->xoffset -= xval; */
/* 	graph->plot->yoffset -= yval; */
/* 	graph->plot->zoffset -= zval; */

/* 	graph->wxmax -= xval; */
/* 	graph->wxmin -= xval; */
/* 	graph->wymax -= yval; */
/* 	graph->wymin -= yval; */

	ScaleAndRefreshGraph(graph);
	    

	/* mark start of the drag location */
	graph->dragx1 = sx1;
	graph->dragy1 = sy1;
      } else if ( state&5 ) { /* shift and control */
	
	/* ctrl-shift-left-click: start polyline */
	if (debug) {
	  fprintf(stderr, "shift and control\n");
	  fprintf(stderr, "start polyline\n");
	}
	ClickSelectInputs(G);
	SetButtonMode(POLYLINEMODE);
	CrosshairCursor(F);

	graph->dragx1 = sx1;
	graph->dragy1 = sy1;
	WorldTransform(graph,sx1,sy1,&x,&y);
	graph->dragwx1 = x;
	graph->dragwy1 = y;

      } else { /* no modifier */
	
	/* plain left-click: start cluster boundary */
	if (debug) {
	  fprintf(stderr, "no modifier\n");
	  fprintf(stderr, "start cluster boundary\n");
	}
	
	clmode = GetItemValue("/clustermenu/clustermode");
	
	if ( strcmp(clmode, POLYGONSTRING)==0 ) {
	  ClickSelectInputs(G);
	  SetButtonMode(CLUSTERPOLYLINEMODE);
	  CrosshairCursor(F);
	  
	  BackupClusterBounds(graph);
	  
	  graph->dragx1 = sx1;
	  graph->dragy1 = sy1;
	  WorldTransform(graph,sx1,sy1,&x,&y);
	  graph->dragwx1 = x;
	  graph->dragwy1 = y;
	  AssignClusterBounds(graph,
			      graph->selectedcluster,
			      graph->currentprojection[0],
			      graph->currentprojection[1],
			      -1,-1,x,y,INCOMPLETE);
	} else if ( strcmp(clmode, BOXSTRING)==0 ) {
	  NormalSelectInputs(G);
	  SetButtonMode(CLUSTERBOXMODE);
	  CrosshairCursor(F);
	  /*
	  ** mark the start of the drag location
	  */
	  graph->dragx1 = sx1;
	  graph->dragy1 = sy1;
	}
      }
    }
    break;

  case 2: /*middle mouse button*/
    switch (ButtonMode()) {
    case CLUSTERPOLYLINEMODE:
      if (debug) {
	fprintf(stderr, "clusterpolylinemode\n");
	fprintf(stderr, "end poly line\n");
      }
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);
      /*
      ** mark the end of the drag line
      */
      x1 = graph->dragwx1;
      y1 = graph->dragwy1;
      graph->dragx2 = sx1;
      graph->dragy2 = sy1;
      WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2);
      AssignClusterBounds(graph,
			  graph->selectedcluster,
			  graph->currentprojection[0],
			  graph->currentprojection[1],
			  -1,-1,x2,y2,COMPLETE);
      SetClusterModified(graph,graph->selectedcluster,1);
      AssignClusterPointsToCluster(graph,graph->selectedcluster);
      /*
      ** and refresh the display
      */
      ScaleAndRefreshGraph(graph);
      /*
      ** set the drag coords to the invalid state
      */
      graph->dragx1 = -1;
      graph->dragx2 = -1;

      break;
    case POLYLINEMODE:
      if (debug) {
	fprintf(stderr, "polylinemode\n");
	fprintf(stderr, "end poly line\n");
      }
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);

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

    case DATAMODE:
      if (state&1 && !(state&4)) { /* shift, not control */
	if (debug) {
	  fprintf(stderr, "shift, not control\n");
	  fprintf(stderr, "start interactive zoom\n");
	}
	NormalSelectInputs(G);
	SetButtonMode(ZOOMINTERACTIVEMODE);
	/* mark start of the drag location */
	graph->dragx1 = sx1;
	graph->dragy1 = sy1;
      } else if ( !(state&1) && state&4 ) { /* control, not shift */
	if (debug) {
	  fprintf(stderr, "control, not shift\n");
	  fprintf(stderr, "start ruler line\n");
	}

	NormalSelectInputs(G);
	SetButtonMode(RULERLINEMODE);
	CrosshairCursor(F);
/* 	ClearWindow(graph->frame->text); */
	if(snapmode){
	  /*
	  ** world coord xy readout of data
	  */
	  pt = Snap(graph,sx1,sy1,&x,&y);
	  /*
	  ** get the screen coords of the data point
	  */
	  ScreenTransform(graph,x,y,&sx2,&sy2);
	  /*
	  ** added correction for axis offsets in the displayed coords
	  ShowCoords(graph,sx2,sy2,x,y);
	  */
 	  ShowCoords(graph,sx2,sy2,x-p->xoffset_axis,y-p->yoffset_axis);
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

      } else if ( state&5 ) { /* shift and control */
	if (debug) {
	  fprintf(stderr, "shift and control\n");
	  fprintf(stderr, "no action\n");
	}
      } else { /* no modifier */
	if (debug) {
	  fprintf(stderr, "no modifier\n");
	  fprintf(stderr, "start interactive pan\n");
	}
	NormalSelectInputs(G);
	SetButtonMode(PANINTERACTIVEMODE);
	FleurCursor(F);
	/* mark start of the drag location */
	graph->dragx1 = sx1;
	graph->dragy1 = sy1;
      }
    }
    break;
  case 3: /*right mouse button*/
    switch (ButtonMode()) {
    case CLUSTERPOLYLINEMODE:
    case POLYLINEMODE:
      break;
    case DATAMODE: /* = default mode */

      if (state&1 && !(state&4)) { /* shift, not control */
	if (debug) {
	  fprintf(stderr, "shift, not control\n");
	  fprintf(stderr, "start zoom out box\n");
	}
	NormalSelectInputs(G);
	SetButtonMode(ZOOMBOXMODE);
	CrosshairCursor(F);
	/* mark start of the drag location */
	graph->dragx1 = sx1;
	graph->dragy1 = sy1;
      } else if ( !(state&1) && state&4 ) { /* control, not shift */
	if (debug) {
	  fprintf(stderr, "control, not shift\n");
	  fprintf(stderr, "drag line\n");
	}
	NormalSelectInputs(G);
	SetButtonMode(DRAGLINEMODE);
	CrosshairCursor(F);
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
	DrawSuperLine(graph,sx2,sy1,sx2,sy2,graph->supercolor);
	NBShowPoint2(commandlabel,x-p->xoffset_axis,y-p->yoffset_axis,pt);
	RefreshText(graph->frame->text);

      } else if ( state&5 ) { /* shift and control */
	if (debug) {
	  fprintf(stderr, "shift and control\n");
	  fprintf(stderr, "draw line\n");
	}
	NormalSelectInputs(G);
	SetButtonMode(DRAWMODE);
	CrosshairCursor(F);
	/*
	** mark the start of the line
	*/
	graph->dragx1 = sx1;
	graph->dragy1 = sy1;
	WorldTransform(graph,sx1,sy1,&x,&y);
	graph->dragwx1 = x;
	graph->dragwy1 = y;

      } else { /* no modifier */
	if (debug) {
	  fprintf(stderr, "no modifier\n");
	  fprintf(stderr, "start zoom in box\n");
	}
	NormalSelectInputs(G);
	SetButtonMode(ZOOMBOXMODE);
	CrosshairCursor(F);
	/* mark start of the drag location */
	graph->dragx1 = sx1;
	graph->dragy1 = sy1;
      
      }
    }

    break;
  }
  
}


ButtonReleaseAction(Graph *graph, XButtonReleasedEvent *buttonevent)
{

int	button;
 int	state;
int	sx2,sy2;
int	sx1,sy1;
float	x1,y1,x2,y2;
int	pt;
Plot	*p;
/* float	xoffset,yoffset; */
 float val;
 long dx, dy;
 double dist, frac;
 float zoom;
 int sv;
 int rotpfreq;
 int i;

    /* button number */
    button = buttonevent->button;

    /* modifier key state */
    state = buttonevent->state;

    if (debug){
      fprintf(stderr,"got mouse button release: %d\n",button);
      fprintf(stderr,"got mouse button modifier state: %d\n",state);
    }

    /* Ignore 'intermediate' button presses (e.g. clicking button 5
       during a button 3 drag) */
    
    if (button != graph->button){ 
      return; }
    
    if(graph->button == 0){ 
	graph->dragx1 = -1;
	graph->dragx2 = -1;
	/*
	** this is the abort condition
	*/
	RefreshGraph(graph);
	return;
    }
    /*
    ** indicate that no button is being pressed
    */
    graph->button = 0;
    sx2 = buttonevent->x;
    sy2 = buttonevent->y;

    p=SelectedPlot(graph);
/*     if((p=SelectedPlot(graph)) != NULL){ */
/* 	xoffset = p->xoffset_axis; */
/* 	yoffset = p->yoffset_axis; */
/*     } else { */
/* 	xoffset = 0; */
/* 	yoffset = 0; */
/*     } */

    
    switch (ButtonMode()) {
    case CLUSTERPOLYLINEMODE:
    case POLYLINEMODE:
      if (debug) {
	fprintf(stderr, "clusterpolylinemode, polylinemode\n");
	fprintf(stderr, "nothing to be done\n");
      }
      break;
    case DRAWMODE:
      if (debug) {
	fprintf(stderr, "drawmode\n");
	fprintf(stderr, "end line drawing\n");
      }
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);

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
      AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0,WORLD_LBL,
		   TEMPORARY_LBL);
      /*
      ** set the drag coords back to the invalid state
      */
      graph->dragx1 = -1;
      graph->dragx2 = -1;

      break;
    case ZOOMBOXMODE:
      if (debug) {
	fprintf(stderr, "zoomboxmode\n");
	fprintf(stderr, "apply zoom\n");
      }
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);
      /*
      ** mark the end of the drag and zoom
      */
      graph->dragx2 = sx2;
      graph->dragy2 = sy2;
      WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1);
      WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2);

      zoom = (float) GetZoomPercent();

      if (debug) {
	fprintf(stderr, "zoom: %f\n", zoom);
      }

      if( ( (graph->dragx1-sx2)*(graph->dragx1-sx2) + (graph->dragy1-sy2)*(graph->dragy1-sy2)) < 25 ) {
	/* treat this as a zoom-click */

/* 	if (button == 1) { */
/* 	  /\* zoom in *\/ */
/* 	  val = (graph->wymax - graph->wymin)*(1-zoom); */
/* 	  graph->wymax = y1+0.5*val; */
/* 	  graph->wymin = y1-0.5*val; */
/* 	  val = (graph->wxmax - graph->wxmin)*(1-zoom); */
/* 	  graph->wxmax = x1+0.5*val; */
/* 	  graph->wxmin = x1-0.5*val; */

/* 	} else if (button == 3) { */
	  /* zoom out */
/* 	  val = (graph->wymax - graph->wymin)/(1-zoom); */
/* 	  graph->wymax = y1+0.5*val; */
/* 	  graph->wymin = y1-0.5*val; */
/* 	  val = (graph->wxmax - graph->wxmin)/(1-zoom); */
/* 	  graph->wxmax = x1+0.5*val; */
/* 	  graph->wxmin = x1-0.5*val; */

	ZoomOut(graph, zoom);
	/*
	val = (graph->wymax - graph->wymin) * zoom;
	graph->wymax += val;
	graph->wymin -= val;
	val = (graph->wxmax - graph->wxmin) * zoom;
	graph->wxmax += val;
	graph->wxmin -= val;
	*/

/* 	} */

      } else {
/* 	if (button == 1) { */
	  /* zoom in */
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
/* 	} else if (button == 3) { */
/* 	  /\* zoom out *\/ */
/* 	  val = 0.5 * (graph->wxmax-graph->wxmin)*(graph->wxmax-graph->wxmin) / fabs(x1-x2); */
/* 	  graph->wxmin = (x1+x2)*0.5 - val; */
/* 	  graph->wxmax = (x1+x2)*0.5 + val; */
/* 	  val = 0.5 * (graph->wymax-graph->wymin)*(graph->wymax-graph->wymin) / fabs(y1-y2); */
/* 	  graph->wymin = (y1+y2)*0.5 - val; */
/* 	  graph->wymax = (y1+y2)*0.5 + val; */
/* 	} */
	  ScaleAndRefreshGraph(graph);
      }

      /*
      ** set the drag coords back to the invalid state
      */
      graph->dragx1 = -1;
      graph->dragx2 = -1;
      break;
    case CLUSTERBOXMODE:
      if (debug) {
	fprintf(stderr, "clusterboxmode\n");
	fprintf(stderr, "end cluster box\n");
      }
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);

      /*
      ** mark the end of the cluster box drag
      */
      graph->dragx2 = sx2;
      graph->dragy2 = sy2;
      WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1);
      WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2);
      /*
      ** dont make a zero sized box
      */
      if(x2 == x1 || y2 == y1){
	graph->dragx1 = -1;
	graph->dragx2 = -1;
	return;
      }
      /*
      ** make a backup of the current cluster bounds
      */
      BackupClusterBounds(graph);
      /*
      ** assign points to the cluster
      */
      AssignClusterBounds(graph,graph->selectedcluster,
			  graph->currentprojection[0],graph->currentprojection[1],
			  4,0,x1,y1,INCOMPLETE);
      AssignClusterBounds(graph,graph->selectedcluster,
			  graph->currentprojection[0],graph->currentprojection[1],
			  4,1,x2,y1,INCOMPLETE);
      AssignClusterBounds(graph,graph->selectedcluster,
			  graph->currentprojection[0],graph->currentprojection[1],
			  4,2,x2,y2,INCOMPLETE);
      AssignClusterBounds(graph,graph->selectedcluster,
			  graph->currentprojection[0],graph->currentprojection[1],
			  4,3,x1,y2,COMPLETE);
      SetClusterModified(graph,graph->selectedcluster,1);
      AssignClusterPointsToCluster(graph,graph->selectedcluster);
      /*
      ** and refresh the display
      */
      ScaleAndRefreshGraph(graph);
      /*
      ** set the drag coords back to the invalid state
      */
      graph->dragx1 = -1;
      graph->dragx2 = -1;

      break;
    case ZOOMINTERACTIVEMODE:
      if (debug) {
	fprintf(stderr, "zoominteractivemode\n");
	fprintf(stderr, "end interactive mode\n");
      }
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);

      WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1);

      /* calculate new zoom fraction */
      dx = sx2 - graph->dragx1;
      dy = graph->dragy1 - sy2;
      dist = sqrt( (double)  (dx*dx + dy*dy) );
      frac = 1- dist / (1000); /* for now just an arbitrary normalization */

      if (frac < 0.01)
	frac = 0.01;

      if (dx<-dy)
	frac = 1/(frac);

      if (debug) {
	fprintf(stderr, "fraction: %f, dx: %d, dy: %d\n", frac, dx, dy);
      }

      /* zoom */
      graph->wymax = y1+frac*(graph->wymax-y1);
      graph->wymin = y1-frac*(y1-graph->wymin);
      graph->wxmax = x1+frac*(graph->wxmax-x1);
      graph->wxmin = x1-frac*(x1-graph->wxmin);
      
      ScaleAndRefreshGraph(graph);
      /*
      ** set the drag coords back to the invalid state
      */
      graph->dragx1 = -1;
      graph->dragx2 = -1;
      break;
    case PANINTERACTIVEMODE:
      if (debug) {
	fprintf(stderr, "paninteractivemode\n");
	fprintf(stderr, "end interactive mode\n");
      }
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);

      graph->dragx2 = sx2;
      graph->dragy2 = sy2;
      WorldTransform(graph,graph->dragx1,graph->dragy1,&x1,&y1);
      WorldTransform(graph,graph->dragx2,graph->dragy2,&x2,&y2);

      graph->wymax -= (y2-y1);
      graph->wymin -= (y2-y1);
      graph->wxmax -= (x2-x1);
      graph->wxmin -= (x2-x1);

      ScaleAndRefreshGraph(graph);
      /*
      ** set the drag coords back to the invalid state
      */
      graph->dragx1 = -1;
      graph->dragx2 = -1;
      break;
    case WOBBLEMODE:
      /* go back to previous rotation */
      /* for now just 0,0,0 */
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);
      /*reset pointfreq of the current plot */
      rotpfreq = graph->rotpointfreq;
      graph->rotpointfreq = p->pointfreq;
      p->pointfreq = rotpfreq;
      sprintf(rotlabel->u.string.line, "tx: %5d  ty: %5d  tz: %5d",
	      (int)(360*graph->thetax/(2*M_PI))%360,
	      (int)(360*graph->thetay/(2*M_PI))%360,
	      (int)(360*graph->thetaz/(2*M_PI))%360);
      RefreshText(graph->frame->text); 
/*       SetRotationMatrix(&graph->matrix, 0, 0, 0); */
      AffineRotationMatrixEuler(&graph->matrix, graph->thetax, graph->thetay, graph->thetaz);
      /* AffineIdentityMatrix(&graph->matrix);*/

/*       /\* undo our hackish translations (to get rotation about center) *\/ */
      graph->xaxis.show	= old_axis_show_x;
      graph->yaxis.show = old_axis_show_y;

      /* remove 3D axes */
      for (i=0; i<3; i++)
	DeleteLabel(graph, GetNamedLabel(graph, "3daxis"));

/*       graph->plot->xoffset += xval; */
/*       graph->plot->yoffset += yval; */
/*       graph->plot->zoffset += zval; */

/*       graph->wxmax += xval; */
/*       graph->wxmin += xval; */
/*       graph->wymax += yval; */
/*       graph->wymin += yval; */

      /* and refresh the graph */
      ScaleAndRefreshGraph(graph);
      graph->dragx1 = -1;
      break;
    case EDITBOUNDMODE:
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);
      BackupClusterBounds(graph);
      currentbound->coord[closestpoint].x = graph->dragx1;
      currentbound->coord[closestpoint].y = graph->dragy1;

      if (currentbound->projection[0] == graph->currentprojection[1] && 
	  currentbound->projection[1] == graph->currentprojection[0]) {
	WorldTransform(graph, graph->dragx1, graph->dragy1, &currentbound->fcoord[closestpoint].y, &currentbound->fcoord[closestpoint].x);
      } else {
	WorldTransform(graph, graph->dragx1, graph->dragy1, &currentbound->fcoord[closestpoint].x, &currentbound->fcoord[closestpoint].y);
      }
      currentbound = NULL;
      closestpoint = -1;
      graph->dragx1 = -1;

      SetClusterModified(graph,graph->selectedcluster,1);
      AssignClusterPoints(graph);
      ScaleAndRefreshGraph(graph);
      break;
    case RULERLINEMODE:
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);
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
 	  ShowCoords(graph,sx1,sy1,x2-p->xoffset_axis,y2-p->yoffset_axis);
	} else {
	  /*
	  ** display the final coords
	  */
	  WorldTransform(graph,sx2,sy2,&x2,&y2);
	  ShowCoords(graph,sx2,sy2,x2,y2);
	}
/* 	ClearWindow(graph->frame->text); */
	ShowSlope2(commandlabel,x1,y1,x2,y2);
	AddLabelLine(graph,0,0,0,0,x1,y1,0.0,x2,y2,0.0,WORLD_LBL,
		     TEMPORARY_LBL,graph->fontname);
      }
      /*
      ** set the drag coords back to the invalid state
      */
      graph->dragx1 = -1;
      graph->dragx2 = -1;
      break;
    case DRAGLINEMODE:
      NormalSelectInputs(G);
      SetButtonMode(DATAMODE);
      NormalCursor(F);
      /*
      ** erase the old line
      */
      EraseSuperLine(graph,graph->dragx2,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor);
      /*
      ** get the coords
      */
      pt = Snap(graph,sx2,sy2,&x2,&y2);
      ShowCoords(graph,graph->dragx2,graph->dragy2,x2-p->xoffset_axis,y2-p->yoffset_axis);
      graph->dragx1 = -1;
      graph->dragx2 = -1;
      break;
    case DATAMODE: /*default */

      /* 
      ** hide/show cluster zero on left side button 
      */
      if (button == 6) {
	CallMenuItem ("/clustermenu/hidecluster");
	return;
      }

      /*
      ** select an empty cluster on right side button
      */

      if (button == 7) {
	InterpretCommand(graph,p,"/selectemptycluster");
	return;
      }
    
      /* note no 'break' statement--this next block is to be executed
      ** in *both* DATAMODE and in FLASHSELECTMODE */

    case FLASHSELECTMODE:
      /*
      ** Process mouse wheel events: step through saved views 
      */
      if (button == 4) {
	if(state&ControlMask){
	  SetButtonMode(FLASHSELECTMODE);
	  WaitForKeyReleaseInputs(graph);
	  InterpretCommand(graph,p,"/flash 1");
	  InterpretCommand(graph,p,"/selectprevenabledcluster");
	  return;
	}
	if(state&ShiftMask){
	  return;
	}
	else { /* no shift, no control */
	  InterpretCommand(graph,p,"/prevsavedview");
	}
      }
    
      if (button == 5) {
	if(state&ControlMask){
	  SetButtonMode(FLASHSELECTMODE);
	  WaitForKeyReleaseInputs(graph);
	  InterpretCommand(graph,p,"/flash 1");
	  InterpretCommand(graph,p,"/selectnextenabledcluster");
	  return;
	}
	if(state&ShiftMask){
	  return;
	}
	else { /* no shift, no control */
	  InterpretCommand(graph,p,"/nextsavedview");
	  return;
	}
      }

    }
    
}


PointerMotionAction(Graph *graph, XPointerMovedEvent *motionevent)
{
int	sx2,sy2;
int	sx1,sy1;
int	button;
float	x1,y1;
float	x2,y2;
float	x,y;
extern int text_entry;
int	pt;
/* float	xoffset,yoffset; */
Plot	*p;
 XTimeCoord *xtc;
 int  nevents;
 unsigned long lasttime;

 int dd;
 int pnt;
 ClusterBounds *cb;
 char tmpstr[200];

 double dx, dy, lv, angle;

    /*
    ** get the location of the cursor
    */
    sx2 = motionevent->x;
    sy2 = motionevent->y;

    p=SelectedPlot(graph);
/*     if((p=SelectedPlot(graph)) != NULL){ */
/* 	xoffset = p->xoffset_axis; */
/* 	yoffset = p->yoffset_axis; */
/*     } else { */
/* 	xoffset = 0; */
/* 	yoffset = 0; */
/*     } */
    WorldTransform(graph, sx2, sy2, &x2, &y2);
    sprintf(coordlabel->u.string.line, "sx: %3d  sy: %3d  wx: %9.2f  wy: %9.2f", sx2, sy2, x2, y2);
    RefreshText(graph->frame->text);

    /*
    ** get the button pressed
    */
    button = (motionevent->state & 0xFF00) >> 8;

    switch (ButtonMode()) {
    case CLUSTERPOLYLINEMODE:
    case POLYLINEMODE:
    case DRAWMODE:
      if (debug) {
	fprintf(stderr, "clusterpolylinemode, polylinemode, drawmode\n");
	fprintf(stderr, "draw rubber line\n");
      }
      DrawRubberLine(graph, sx2, sy2);
      break;
    case ZOOMBOXMODE:
    case CLUSTERBOXMODE:
      if (debug) {
	fprintf(stderr, "zoomboxmode, clusterboxmode\n");
	fprintf(stderr, "rubber box\n");
      }
      DrawRubberBox(graph, sx2, sy2);
      break;
    case ZOOMINTERACTIVEMODE:
      if (debug) {
	fprintf(stderr, "zoominteractivemode\n");
	fprintf(stderr, "interactive zooming\n");
      }
      DrawRubberLine(graph, sx2, sy2);
      break;
    case PANINTERACTIVEMODE:
      if (debug) {
	fprintf(stderr, "paninteractivemode\n");
	fprintf(stderr, "interactive panning\n");
      }
      DrawRubberLine(graph, sx2, sy2);
      break;
    case DRAGLINEMODE:
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
		     graph->dragx2,graph->dragy2,graph->dragx2,graph->dragy1,graph->supercolor);

      /*
      ** and draw a new line using the x coordinate of the data point
      ** and the y coordinates of the cursor and the data point
      */
      graph->dragx1 = sx2;
      graph->dragy1 = sy2;
      graph->dragx2 = sx1;
      graph->dragy2 = sy1;
      DrawSuperLine(graph,sx1,sy1,sx1,sy2,graph->supercolor);
      /*
      ** display the data coordinates in the text window
      */
      NBShowPoint2(commandlabel,x-p->xoffset_axis,y-p->yoffset_axis,pt);
      break;
    case RULERLINEMODE:
      if(graph->dragx1 != -1){
	if(graph->dragx2 != -1){
	  /*
	  ** erase the old drag line from the starting drag coordinates
	  ** drag->x1,y1 to the previous end coordinates drag->x2,y2.
	  */
	  EraseSuperLine(graph,
			 graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor);
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
 	ShowSlopeCoords2(commandlabel,x1-p->xoffset_axis,y1-p->yoffset_axis,x2-p->xoffset_axis,y2-p->yoffset_axis);
	/*
	** draw the new drag line
	*/
	DrawSuperLine(graph,
		      graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor);
      }
      break;
    case WOBBLEMODE:
      if (debug) {
	fprintf(stderr, "wobbleinteractivemode\n");
	fprintf(stderr, "interactive wobbling\n");
	fprintf(stderr, "sx1: %d, sy1: %d, sx2: %d, sy2: %d\n", graph->dragx1, graph->dragy1, sx2, sy2);
      }


      x1 = 2*M_PI*(float)(sy2-graph->dragy1)/graph->wheight;
      y1 = 2*M_PI*(float)(sx2-graph->dragx1)/graph->wwidth;

      dx = sx2-graph->dragx1;
      dy = sy2-graph->dragy1;
      lv = dx*dx + dy*dy;
      angle = 4*M_PI*(sqrt(lv))/graph->wwidth;
      lv = sqrt(lv);
      dx = dx / lv;
      dy = dy / lv;
      /*angle = angle / lv;*/

      /*
      fprintf(stderr, "x1: %f, y1: %f, tx: %f, ty: %f, dragx: %d, dragy: %d\n", x1, y1, graph->thetax, graph->thetay, graph->dragx1, graph->dragy1);
      */
 
/*       sprintf(rotlabel->u.string.line, "tx: %5d  ty: %5d  tz: %5d", */
/* 	      (int)(360*(graph->thetax+x1)/(2*M_PI))%360, */
/* 	      (int)(360*(graph->thetay+y1)/(2*M_PI))%360, */
/* 	      (int)(360*(graph->thetaz)/(2*M_PI))%360); */
/*       RefreshText(graph->frame->text);  */


/*       SetRotationMatrix(&graph->matrix, (float) (graph->thetax+x1), (float) (graph->thetay+y1), 0.0); */

/*       AffineRotationMatrixEuler(&graph->matrix, (float) (graph->thetax+x1), (float) (graph->thetay+y1), 0.0); */
      /*    graph->matrix = *(ApplyAffineRotation(&graph->matrix, dy,-dx, 0.0, angle)); */

      if (graph->rotorigin)
	ApplyAffineTranslation(&graph->matrix, -(graph->wxmax+graph->wxmin)/2, -(graph->wymax+graph->wymin)/2,  -(graph->wxmax+graph->wxmin)/2);
      graph->matrix = *(ApplyAffineRotation(&graph->matrix, dy, dx, 0.0, angle));
      if (graph->rotorigin)
	ApplyAffineTranslation(&graph->matrix, (graph->wxmax+graph->wxmin)/2, (graph->wymax+graph->wymin)/2,(graph->wxmax+graph->wxmin)/2);

/*       graph->matrix = *(AffineRotateRight(&graph->matrix, (float) 4*M_PI*(sx2-graph->dragx1)/graph->wwidth)); */
/*       graph->matrix= *(AffineRotateUp(&graph->matrix, (float) 4*M_PI*(sy2-graph->dragy1)/graph->wheight)); */


      ScaleAndRefreshGraph(graph);
      graph->dragy1 = sy2;
      graph->dragx1 = sx2;
      break;
    case EDITBOUNDMODE:
      cb = currentbound;
      if (closestpoint==0) {
	EraseSuperLine(graph, graph->dragx1,graph->dragy1,cb->coord[cb->ncoords-1].x,cb->coord[cb->ncoords-1].y,graph->supercolor);
	DrawSuperLine(graph, sx2,sy2,cb->coord[cb->ncoords-1].x,cb->coord[cb->ncoords-1].y,graph->supercolor);
      }
      else {
	EraseSuperLine(graph, graph->dragx1,graph->dragy1,cb->coord[closestpoint-1].x,cb->coord[closestpoint-1].y,graph->supercolor);
	DrawSuperLine(graph, sx2, sy2,cb->coord[closestpoint-1].x,cb->coord[closestpoint-1].y,graph->supercolor);
      }
      
      if (closestpoint==cb->ncoords-1) {
	EraseSuperLine(graph, graph->dragx1,graph->dragy1,cb->coord[0].x,cb->coord[0].y,graph->supercolor);
	DrawSuperLine(graph, sx2,sy2,cb->coord[0].x,cb->coord[0].y,graph->supercolor);
      }
      else {
	EraseSuperLine(graph, graph->dragx1,graph->dragy1,cb->coord[closestpoint+1].x,cb->coord[closestpoint+1].y,graph->supercolor);
	DrawSuperLine(graph, sx2, sy2,cb->coord[closestpoint+1].x,cb->coord[closestpoint+1].y,graph->supercolor);
      }
      graph->dragx1 = sx2;
      graph->dragy1 = sy2;
      break;

    default:
      if (motionevent->state&1 && !(motionevent->state&4)) { /* shift, not control */
	for(cb=graph->clusterbounds;cb;cb=cb->next){
	  if(cb->clusterid == graph->selectedcluster &&
	     ( (cb->projection[0] == graph->currentprojection[0] && cb->projection[1] == graph->currentprojection[1]) ||
	       (cb->projection[0] == graph->currentprojection[1] && cb->projection[1] == graph->currentprojection[0]) ) ) {
	    for(pnt = 0; pnt<cb->ncoords; pnt++) {
	      dd = (sx2-cb->coord[pnt].x)*(sx2-cb->coord[pnt].x) + (sy2-cb->coord[pnt].y)*(sy2-cb->coord[pnt].y);
	      if (dd<25) {
		FleurCursor(F);
		return;
	      }
	    }
	  }
	}
      }
      NormalCursor(F);
      break;
    }

}

LeaveNotifyAction(Graph *graph, void *event)
{
  if(buttonmode == FLASHSELECTMODE){
    SetButtonMode(DATAMODE);
    NormalSelectInputs(G);
    InterpretCommand(graph,SelectedPlot(G),"/flash 0");
  }
}

DrawRubberBox(Graph *graph, int sx2, int sy2)
{
int	tmpx2,tmpy2;
int	tmpx1,tmpy1;


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
      EraseSuperBox(graph,tmpx1,tmpy1,tmpx2,tmpy2,graph->supercolor);
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
    DrawSuperBox(graph,tmpx1,tmpy1,tmpx2,tmpy2,graph->supercolor);
  }
}

DrawRubberLine(Graph *graph, int sx2, int sy2)
{
  if(graph->dragx1 != -1){
    if(graph->dragx2 != -1){
      /*
      ** erase the old drag line from the starting drag coordinates
      ** drag->x1,y1 to the previous end coordinates drag->x2,y2.
      */
      EraseSuperLine(graph,
		     graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor);
    }
    /*
    ** get the world coordinates of the initial drag point
    */
    /*
    x1 = graph->dragwx1;
    y1 = graph->dragwy1;
    */
    /*
    ** get the world coordinates of the current cursor
    ** location
    */
    /*
    WorldTransform(graph,sx2,sy2,&x2,&y2);
    */
    /*
    ** update the drag coordinates to the current location
    */
    graph->dragx2 = sx2;
    graph->dragy2 = sy2;
    /*
    ** display data about the line between the points in the
    ** text window
    */
    /*
    ShowSlopeCoords(graph->frame->text,0,graph->fontheight,x1,y1,x2,y2);
    */
    /*
    ** draw the new drag line
    */
    DrawSuperLine(graph,
		  graph->dragx1,graph->dragy1,graph->dragx2,graph->dragy2,graph->supercolor);
  }
}
