#include "xplot_ext.h"

AssignColors(graph,start,increment)
Graph	*graph;
int	start;
int	increment;
{
int	color;
Plot	*plot;
    
    color = start;
    for(plot=graph->plot;plot;plot=plot->next){
	plot->linestyle = color;
	color += increment;
    }
}

int plotcompare(f1,f2)
FCoord	*f1,*f2;
{
    return((int)(f1->x - f2->x));
}

Sort(graph)
Graph 	*graph;
{
Plot	*plot;
    for(plot=graph->plot;plot;plot=plot->next){
	qsort(plot->data,plot->npoints,sizeof(FCoord),plotcompare);
    }
}

ScaleAndRefreshGraph(graph)
Graph 	*graph;
{
    RescaleGraph(graph);
    RefreshGraph(graph);
}

RefreshGraph(graph)
Graph	*graph;
{
    ResetGlobalOperation();
    ClearWindow(graph);
    DisplayLabels(graph);
    DrawGraph(graph); 
}

DrawGraph(graph)
Graph	*graph;
{
Plot	*plot;
int	cnt;

    CalculateAxisParameters(graph);
    DrawAxes(graph);
    /*
    ** draw each plot of the graph
    */
    cnt = 0;
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->graph->show_legend){
	    DrawPlotLegend(plot,cnt);
	}
	cnt++;
	if(plot->visible){
	    if(!graph->stepthrough || (graph->stepthrough == plot)){
		DrawPlot(plot);
	    }
	}
    }
    if(graph->show_title){
	DrawGraphTitle(graph);
    }
}

ResetStepThrough(graph)
Graph	*graph;
{
    graph->stepthrough = NULL;
}

StepForward(graph)
Graph	*graph;
{
Plot	*plot;
int	savecolor;

    if(graph->stepthrough){
	/*
	** draw over the previous
	*/
	DrawPlot(graph->stepthrough);
	/*
	** advance to the next plot
	*/
	if(graph->stepthrough->next){
	    graph->stepthrough = graph->stepthrough->next;
	    /*
	    ** change the color
	    */
	    savecolor = graph->stepthrough->linestyle;
	    graph->stepthrough->linestyle = graph->foreground;
	    DrawPlot(graph->stepthrough);
	    /*
	    ** restore the color
	    */
	    graph->stepthrough->linestyle = savecolor;
	}
    } else {
	return;
    }
}

StepBackward(graph)
Graph	*graph;
{
Plot	*plot;
int	savecolor;

    if(graph->stepthrough){
	/*
	** draw over the previous
	*/
	DrawPlot(graph->stepthrough);
	/*
	** go back to the previous plot
	*/
	for(plot=graph->plot;plot;plot=plot->next){
	    if(plot->next == graph->stepthrough){
		graph->stepthrough = plot;
	    }
	}
	/*
	** change the color
	*/
	savecolor = graph->stepthrough->linestyle;
	graph->stepthrough->linestyle = graph->foreground;
	DrawPlot(graph->stepthrough);
	/*
	** restore the color
	*/
	graph->stepthrough->linestyle = savecolor;
    } else {
	return;
    }
}

DrawPointIcon(plot,x,y)
Plot *plot;
int x,y;
{
Pixmap	bitmap;
short	w,h;

    switch(plot->point_symbol){
    case BOX_PT:
	/*
	** place a 5x5 pixel box around each point
	*/
	Box(x-2,y-2,x+2,y+2);
	break;
    case FBOX_PT:
	/*
	** place a solid 5x5 pixel box around each point
	*/
	FilledBox(x-3,y-3,x+2,y+3);
	break;
    case CIRCLE_PT:
	/*
	** place a 5 pixel circle around each point
	*/
	Circle(x,y,5);
	break;
    case FCIRCLE_PT:
	/*
	** place a solid 5 pixel circle around each point
	*/
	FilledCircle(x,y,7);
	break;
    case TRIANGLE_PT:
	/*
	** place a 5 pixel triangle around each point
	*/
	Triangle(x,y,5);
	break;
    case FTRIANGLE_PT:
	/*
	** place a solid 5 pixel triangle around each point
	*/
	FilledTriangle(x,y,7);
	break;
    case X_PT:
	/*
	** place a 5 pixel X at each point
	*/
	DrawLine(x-2,y-2, x+2,y+2);
	DrawLine(x-2,y+2, x+2,y-2);
	break;
    case CROSS_PT:
	/*
	** place a 5 pixel X at each point
	*/
	DrawLine(x-2,y, x+2,y);
	DrawLine(x,y+2, x,y-2);
	break;
    case VLINE_PT:
	/*
	** place a 5 pixel X at each point
	*/
	DrawLine(x,y+2, x,y-2);
	break;
    case DOT_PT:
	/*
	** place a 1 pixel dot at each point
	*/
	DrawLine(x,y, x+1,y);
	break;
    default:
	/*
	** try to find the bitmap
	*/
	if(plot->point_symbol > 100){
	    if((bitmap = GetBitmap(plot->point_symbol - 100,&w,&h))){
		DrawBitmap(G,bitmap,x-w/2,y-h/2,w,h);
	    }
	}
    }
}

DrawPoints(plot)
Plot	*plot;
{
int	i;
int	j;
int	start = 0;

    /*
    ** draw an icon for each point
    */
    for(i=plot->xlo;i<=plot->xhi;i++){
	/*
	** optimize for points out of view
	*/
	if(plot->coord[i].x < -10 || plot->coord[i].x > plot->graph->wwidth
	+10) continue;
	/*
	** optimize for overlapping points
	*/
	if(plot->graph->optimizepoints > 0){
	    /*
	    ** dont look too far back or it will take forever
	    */
	    if(i-plot->xlo > plot->graph->optimizepoints){
		start = i - plot->graph->optimizepoints;
	    } else {
		start = plot->xlo;
	    }
	    for(j=i-1;j>=start;j--){
		if((plot->coord[i].x == plot->coord[j].x) && 
		(plot->coord[i].y == plot->coord[j].y)){
		    break;
		}
	    }
	} else {
	    j = start-1;
	}
	if((j < start) && (i%plot->pointfreq == 0)){
	    DrawPointIcon(plot,plot->coord[i].x,plot->coord[i].y);
	}
    }
}

DrawSteps(plot)
Plot	*plot;
{
int	i;

    /*
    ** draw an step between each point
    */
    for(i=plot->xlo;i<plot->xhi;i++){
	/*
	** optimize for points out of view
	*/
	if(plot->coord[i].x < -10 || plot->coord[i].x > plot->graph->wwidth
	+10) continue;
	DrawLine(plot->coord[i].x,plot->coord[i].y,
	plot->coord[i].x,plot->coord[i+1].y);
	DrawLine(plot->coord[i].x,plot->coord[i+1].y,
	plot->coord[i+1].x,plot->coord[i+1].y);
    }
}

DrawBars(plot)
Plot	*plot;
{
int	i;
float	width;
float	sx1,sx2;
float	sy,sy2;

    /*
    ** calculate the screen fcoordinates of the x axis
    ** and the bar width
    */
    if(plot->xaxis){
	FloatScreenTransform(plot->graph,plot->bar_width + plot->xmin,
	plot->graph->xaxis.yintcpt+plot->yoffset_axis,
	&sx1,&sy);
    } else {
	FloatScreenTransform(plot->graph,plot->bar_width + plot->xmin,
	plot->graph->xaxis.yintcpt,
	&sx1,&sy);
    }
    FloatScreenTransform(plot->graph,plot->xmin,0.0, &sx2,&sy2);
    width = fabs(sx1 -sx2);
    fprintf(stderr,"%g %g %g\n",width,sx1,sx2);
    /*
    ** draw a box from the data point to the  x axis
    */
    for(i=plot->xlo;i<=plot->xhi;i++){
	/*
	** optimize for points out of view
	*/
	if((i+1 <= plot->xhi) && (i-1 >=plot->xlo) && (plot->fcoord[i+1].x < -10) || (plot->fcoord[i-1].x > plot->graph->wwidth +10)) continue;
	if(plot->bar_filled){
	    FilledBox((int)(plot->fcoord[i].x -width/2.0 +.5),
	    (int)plot->fcoord[i].y,
	    (int)(plot->fcoord[i].x+width/2.0 +.5),
	    (int)sy);
	} else {
	    Box((int)(plot->fcoord[i].x-width/2.0 +.5),
	    (int)plot->fcoord[i].y,
	    (int)(plot->fcoord[i].x+width/2.0 +.5),
	    (int)sy);
	}
    }
}

DrawErrorBars(plot)
Plot	*plot;
{
int	i;
int	sx;
int	tmp;
int	sy;
int	sx1,sx2,sy1,sy2;
float	wx,wy,wz;

    if(plot->edata == NULL){
	return;
    }
    for(i=plot->xlo;i<=plot->xhi;i++){
	if(i%plot->pointfreq == 0){
	    sx = plot->coord[i].x;
	    /*
	    ** get the screen coords of the error bar endpoints 
	    */
	    GetTransformedDataPoint(plot,&wx,&wy,&wz,i);
	    switch(plot->errorbartype){
	    case ERRORBAR:
		ScreenTransform(plot->graph,
		    0.0,
		    wy - plot->escale*plot->edata[i].value,
		    &tmp,&sy1);
		ScreenTransform(plot->graph,
		    0.0,
		    wy + plot->escale*plot->edata[i].value,
		    &tmp,&sy2);
		DrawLine(sx,sy1,sx,sy2);		/* error bar */
		DrawLine(sx-2,sy1,sx+2,sy1);		/* ticks at the endpoints */
		DrawLine(sx-2,sy2,sx+2,sy2);
		break;
	    case ERRORBOX:
		ScreenTransform(plot->graph,
		    wx - plot->escale*plot->edata[i].value,
		    wy - plot->escale*plot->edata[i].value,
		    &sx1,&sy1);
		ScreenTransform(plot->graph,
		    wx + plot->escale*plot->edata[i].value,
		    wy + plot->escale*plot->edata[i].value,
		    &sx2,&sy2);
		if((sx1 == sx2) && (sy1 == sy2)) break;
		Box(sx1,sy1,sx2,sy2);
		break;
	    case ERRORCIRCLE:
		ScreenTransform(plot->graph,
		    0.0,
		    wy,
		    &tmp,&sy);
		ScreenTransform(plot->graph,
		    0.0,
		    wy + 2*plot->escale*plot->edata[i].value,
		    &tmp,&sy2);
		    if(sy-sy2 == 0) break;
		    if(sy > sy2){
			Circle(sx,sy,sy-sy2);
		    } else {
			Circle(sx,sy,sy2-sy);
		    }
		break;
	    case ERRORLINE:
		ScreenTransform(plot->graph,
		    0.0,
		    wy - plot->escale*plot->edata[i].value,
		    &tmp,&sy1);
		ScreenTransform(plot->graph,
		    0.0,
		    wy + plot->escale*plot->edata[i].value,
		    &tmp,&sy2);
		DrawLine(sx,sy1,sx,sy2);		/* error bar */
		break;
	    }
	}
    }
}

DrawPlot(plot)
Plot	*plot;
{
int minus_x;
int minus_y;
int plus_x;
int plus_y;
int i;

    if(plot->graph->optimize){
	minus_x = 1;
	minus_y = 1;
	plus_x = 1;
	plus_y = 1;
	for(i=0;i<plot->npoints;i++){
	    /*
	    ** optimize for points out of view
	    */
	    if(plot->coord[i].y > -10) minus_y = 0;
	    if(plot->coord[i].x > -10) minus_x = 0; 
	    if(plot->coord[i].y < plot->graph->wheight + 10) plus_y = 0;
	    if(plot->coord[i].x < plot->graph->wwidth + 10) plus_x = 0;
	}
	/*
	** test for all points out of view in the same quadrant
	*/
	if(minus_x || minus_y || plus_x || plus_y){
	    /*
	    ** out of sight
	    */
	    return;
	}
    }
    SetColor(plot->linestyle);
    /* SetLinestyle(plot->graph,plot->linestyle); */
    switch(plot->plot_type){
    case NORMAL_GRAPH:

	if(plot->graph->resolution == 1){
	    FloatMultipleLines(plot->coord+plot->xlo,plot->xhi-plot->xlo +1,
	    plot->fcoord+plot->xlo);
	} else {
	    MultipleLines(plot->coord+plot->xlo,plot->xhi - plot->xlo + 1);
	}
	if(plot->showpoints){
	    DrawPoints(plot);
	}
	if(plot->show_error){
	    DrawErrorBars(plot);
	}
	break;
    case BAR_GRAPH:
	DrawBars(plot);
	if(plot->show_error){
	    DrawErrorBars(plot);
	}
	break;
    case SCATTER_GRAPH:
	DrawPoints(plot);
	if(plot->show_error){
	    DrawErrorBars(plot);
	}
	break;
    case STEP_GRAPH:
	DrawSteps(plot);
	if(plot->show_error){
	    DrawErrorBars(plot);
	}
	break;
    }
    /* SetLinestyle(plot->graph,SOLID_LINE); */
}

