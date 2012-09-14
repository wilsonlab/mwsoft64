#include "xclust_ext.h"

static int showclusters=1;
extern int rotatecolor;

SetShowClusters(state)
int	state;
{
    showclusters=state;
}

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

static ProjectionInfo	*xpinfo;

int plotcompare(f1,f2)
DataCoord	*f1,*f2;
{
    return((int)(GetDoubleFromRawData(f1->data,xpinfo) - GetDoubleFromRawData(f2->data,xpinfo)));
}

Sort(graph)
Graph 	*graph;
{
Plot	*plot;
int	xproject;

    xproject = graph->currentprojection[0];
    xpinfo = GetProjectionInfo(graph,xproject);
    /*
    fprintf(stderr,"sorting on projection %s (%d)\n",xpinfo->name,xproject);
    */

    for(plot=graph->plot;plot;plot=plot->next){
	if(xproject >= 0 && xproject < plot->ndatafields){
	    qsort(plot->rawdata,plot->npoints,sizeof(DataCoord),plotcompare);
	}
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
    if(!graph->overlay && (!rotatecolor || graph->changed)){
	ClearWindow(graph);
    }
    DisplayLabels(graph);
    DrawGraph(graph); 
    if(showclusters && graph->showbounds){
      DrawAllClusterBounds(graph);
    }
    graph->changed = 0;
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
float	scale;
int	ival;

    scale = plot->pointscale;
    if(scale == 0) scale = 1;
    switch(plot->point_symbol){
    case BOX_PT:
	/*
	** place a 5x5 pixel box around each point
	*/
	ival = 2*scale;
	Box(x-ival,y-ival,x+ival,y+ival);
	break;
    case FBOX_PT:
	/*
	** place a solid 5x5 pixel box around each point
	*/
	ival = 3*scale;
	FilledBox(x-ival,y-ival,(int)(x+2*scale),y+ival);
	break;
    case CIRCLE_PT:
	/*
	** place a 5 pixel circle around each point
	*/
	Circle(x,y,(int)(5*scale));
	break;
    case FCIRCLE_PT:
	/*
	** place a solid 5 pixel circle around each point
	*/
	FilledCircle(x,y,(int)(7*scale));
	break;
    case TRIANGLE_PT:
	/*
	** place a 5 pixel triangle around each point
	*/
	Triangle(x,y,(int)(5*scale));
	break;
    case FTRIANGLE_PT:
	/*
	** place a solid 5 pixel triangle around each point
	*/
	FilledTriangle(x,y,(int)(7*scale));
	break;
    case X_PT:
	/*
	** place a 5 pixel X at each point
	*/
	ival = 2*scale;
	DrawLine(x-ival,y-ival, x+ival,y+ival);
	DrawLine(x-ival,y+ival, x+ival,y-ival);
	break;
    case CROSS_PT:
	/*
	** place a 5 pixel X at each point
	*/
	ival = 2*scale;
	DrawLine(x-ival,y, x+ival,y);
	DrawLine(x,y+ival, x,y-ival);
	break;
    case VLINE_PT:
	/*
	** place a 5 point vertical line  at each point
	*/
	ival = 2*scale;
	DrawLine(x,y-ival, x,y+ival);
	break;
    case DOT_PT:
	/*
	** place a 1 pixel dot at each point
	ival = scale;
	DrawLine(x,y, x+ival,y);
	*/
	_DrawPoint(G,x,y);
	break;
    case LINE_PT:
	/*
	** place a 2 pixel line at each point
	*/
	ival = scale;
	DrawLine(x,y, x+ival,y);
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
int	prevcolor;
int	color;
int	delay;

    /*
    ** draw an icon for each point
    */
    prevcolor = -1;
    for(i=plot->xlo;i<=plot->xhi;i+=plot->pointfreq){
	if(plot->refreshtime > 0){
	    delay = plot->refreshtime*1e6/(plot->xhi - plot->xlo);
	    if(delay > 0){
		Usleep(delay);
	    }
	}
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
	    for(j=i-1;j>=start;j-=plot->pointfreq){
		if((plot->coord[i].x == plot->coord[j].x) && 
		(plot->coord[i].y == plot->coord[j].y)){
		    break;
		}
	    }
	} else {
	    j = start-1;
	}
	if((j < start)) {/* && (i%plot->pointfreq == 0)){*/
	    /*
	    ** get the color from the cluster info if available
	    */
	    if(showclusters && plot->rawdata) {
		if(plot->graph->hiddencluster[abs(plot->rawdata[i].clusterid)]) {
		    /*
		    ** ignore hidden clusters
		    */
		    continue;
		} else
/* 		if(plot->rawdata[i].clusterid == 0){ */
/* 		  if(prevcolor != plot->linestyle){ */
/* 		    SetColor(plot->linestyle); */
/* 		    prevcolor = plot->linestyle; */
/* 		  } */
/* 		} else  */
		if(plot->rawdata[i].clusterid >= 0){
		    color = LookupClusterColor(plot->graph,
			plot->rawdata[i].clusterid);
		    if(prevcolor != color){
			SetColor(color);
			prevcolor = color;
		    }
		} 
		if((rotatecolor && !plot->graph->changed) && (plot->graph->selectedcluster != plot->rawdata[i].clusterid)){
			/*
			** only draw points in the selected cluster when in rotate color mode
			*/
			continue;
		} else {	
		    DrawPointIcon(plot,plot->coord[i].x,plot->coord[i].y);
		}
	    } else {
		DrawPointIcon(plot,plot->coord[i].x,plot->coord[i].y);
	    }
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
	FloatScreenTransform(plot->graph,plot->bar_width,
	plot->graph->xaxis.yintcpt+plot->yoffset_axis,
	&sx1,&sy);
    } else {
	FloatScreenTransform(plot->graph,plot->bar_width,
	plot->graph->xaxis.yintcpt,
	&sx1,&sy);
    }
    FloatScreenTransform(plot->graph,0.0,0.0, &sx2,&sy2);
    width = fabs(sx1 -sx2);
    /*
    ** draw a box from the data point to the  x axis
    */
    for(i=plot->xlo;i<=plot->xhi;i++){
	/*
	** optimize for points out of view
	*/
	if(plot->fcoord[i].x < -10 || plot->fcoord[i].x > plot->graph->wwidth
	+10) continue;
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
double	nx,ny,nz;
int	p2;
ProjectionInfo	*pinfo;
double	p2val;

    if(plot->zmode == ERRORNONE){
	return;
    }
    p2 = plot->graph->currentprojection[2];
    if(p2 < 0 || p2 >= plot->ndatafields){
	return;
    }
    if((pinfo=GetProjectionInfo(plot->graph,p2)) == NULL){
	fprintf(stderr,"ERROR: Unable to to find info on field %d\n",p2);
	return;
    }
    for(i=plot->xlo;i<=plot->xhi;i++){
	if(i%plot->pointfreq == 0){
	    sx = plot->coord[i].x;
	    p2val = GetDoubleFromRawData(plot->rawdata[i],pinfo);
	    /*
	    ** get the screen coords of the error bar endpoints 
	    */
	    GetTransformedDataPoint(plot,&wx,&wy,&wz,i);
	    switch(plot->zmode){
	    case ERRORBAR:
		ScreenTransform(plot->graph,
		    0.0,
		    wy - plot->escale*p2val,
		    &tmp,&sy1);
		ScreenTransform(plot->graph,
		    0.0,
		    wy + plot->escale*p2val,
		    &tmp,&sy2);
		DrawLine(sx,sy1,sx,sy2);		/* error bar */
		DrawLine(sx-2,sy1,sx+2,sy1);	/* ticks at the endpoints */
		DrawLine(sx-2,sy2,sx+2,sy2);
		break;
            case ERRORBOX:
                ScreenTransform(plot->graph,
                    wx - plot->escale*p2val,
                    wy - plot->escale*p2val,
                    &sx1,&sy1);
                ScreenTransform(plot->graph,
                    wx + plot->escale*p2val,
                    wy + plot->escale*p2val,
                    &sx2,&sy2);
                if((sx1 == sx2) && (sy1 == sy2)) break;
                Box(sx1,sy1,sx2,sy2);
                break;
            case ERRORFILLEDBOX:
                ScreenTransform(plot->graph,
                    wx - plot->escale*p2val,
                    wy - plot->escale*p2val,
                    &sx1,&sy1);
                ScreenTransform(plot->graph,
                    wx + plot->escale*p2val,
                    wy + plot->escale*p2val,
                    &sx2,&sy2);
                if((sx1 == sx2) && (sy1 == sy2)) break;
                FilledBox(sx1,sy1,sx2,sy2);
                break;
            case ERRORCOLORBOX:
                ScreenTransform(plot->graph,
                    wx - plot->escale*p2val,
                    wy - plot->escale*p2val,
                    &sx1,&sy1);
                ScreenTransform(plot->graph,
                    wx + plot->escale*p2val,
                    wy + plot->escale*p2val,
                    &sx2,&sy2);
                if((sx1 == sx2) && (sy1 == sy2)) break;
		SetColor((int)(plot->cscale*p2val) +
		    plot->coffset);
                Box(sx1,sy1,sx2,sy2);
                break;
            case ERRORFILLEDCOLORBOX:
                ScreenTransform(plot->graph,
                    wx - plot->escale*p2val,
                    wy - plot->escale*p2val,
                    &sx1,&sy1);
                ScreenTransform(plot->graph,
                    wx + plot->escale*p2val,
                    wy + plot->escale*p2val,
                    &sx2,&sy2);
                if((sx1 == sx2) && (sy1 == sy2)) break;
		SetColor((int)(plot->cscale*p2val) +
		    plot->coffset);
                FilledBox(sx1,sy1,sx2,sy2);
                break;
            case ERRORCIRCLE:
                ScreenTransform(plot->graph,
                    0.0,
                    wy,
                    &tmp,&sy);
                ScreenTransform(plot->graph,
                    0.0,
                    wy + 2*plot->escale*p2val,
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
		    wy - plot->escale*p2val,
		    &tmp,&sy1);
		ScreenTransform(plot->graph,
		    0.0,
		    wy + plot->escale*p2val,
		    &tmp,&sy2);
		DrawLine(sx,sy1,sx,sy2);		/* error line */
		break;
            case FILLEDCOLORBOX:		/* fixed size color box */
                ScreenTransform(plot->graph,
                    wx - plot->escale,
                    wy - plot->escale,
                    &sx1,&sy1);
                ScreenTransform(plot->graph,
                    wx + plot->escale,
                    wy + plot->escale,
                    &sx2,&sy2);
                if((sx1 == sx2) && (sy1 == sy2)) break;
		SetColor((int)(plot->cscale*p2val) +
		    plot->coffset);
                FilledBox(sx1,sy1,sx2,sy2);
                break;
	    case ZLINE:
		/*
		** draw a line from the xy plane to the point
		*/
		ScreenTransform(plot->graph, wx, wy, &sx,&sy);
		/*
		** compute the point of intersection of a line drawn
		** from the point normal to the xy plane
		*/
		GetDataPoint(plot,&wx,&wy,&wz,i);
		MatrixTransform(plot->graph,(double)wx,(double)wy,(double)0.0,
		    &nx,&ny,&nz);
		ScreenTransform(plot->graph, nx, ny, &sx1,&sy1);
		DrawLine(sx1,sy1,sx,sy);
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
int	blockstart;
int	previd;

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
    case LINE_GRAPH:

	/*
	** draw the lines
	*/
	/*
	** go through the coordinates and draw lines with common ids (colors)
	** using single multiple-line draw commands to speed things up
	*/
	previd = 0;
	blockstart = plot->xlo;
	if(plot->rawdata){
	    previd = plot->rawdata[blockstart].clusterid;
	    if(previd == 0){
		SetColor(plot->linestyle);
	    } else {
		SetColor(LookupClusterColor(plot->graph,previd));
	    }
	}
	for(i=plot->xlo;i<=plot->xhi;i++){
	    /*
	    ** plot blocks of points with continuous clusterids.
	    ** is the id of the current point different from the 
	    ** previous? If yes then terminate the current block
	    ** and display it.
	    */
	    if((i > plot->xlo) && plot->rawdata && 
	    ((plot->rawdata[i].clusterid != previd) || (i == plot->xhi))
	    ){
		/*
		** is the cluster visible? (not hidden and not negative)
		*/
	       if(previd >= 0 &&
		  !plot->graph->hiddencluster[previd]  &&  /* used to be:
							      !plot->graph->hiddencluster[abs(previd)] &&  
							   */
		  blockstart < i)
		 {
		    /*
		    ** and set the color for the block
		    */
		    if(previd == 0){
			SetColor(plot->linestyle);
		    } else {
			SetColor(LookupClusterColor(plot->graph,previd));
		    }
		    /*
		    ** if so then draw the current block of lines 
		    */
		    if(plot->graph->resolution == 1){
			/*
			** use floating point resolution for the coordinates
			** so that if Postscript output is being generated we
			** can take full advantage of the printer resolution
			*/
			FloatMultipleLines(plot->coord+blockstart,i-blockstart,
			plot->fcoord+blockstart);
		    } else {
			/*
			** just use integer coordinates which is fine for scree
			** display
			*/
			MultipleLines(plot->coord+blockstart,i - blockstart);
		    }
		}
		/*
		** and start a new block with the current id
		*/
		blockstart = i;
		previd = plot->rawdata[i].clusterid;
		/*
		** and set the color for the block
		if(previd == 0){
		    SetColor(plot->linestyle);
		} else {
		    SetColor(LookupClusterColor(plot->graph,previd));
		}
		*/
	    }
	    previd = plot->rawdata[i].clusterid;
	}
	if(plot->showpoints){
	    DrawPoints(plot);
	}
	DrawErrorBars(plot);
	break;
    case BAR_GRAPH:
	DrawBars(plot);
	DrawErrorBars(plot);
	break;
    case SCATTER_GRAPH:
	DrawPoints(plot);
	DrawErrorBars(plot);
	break;
    case STEP_GRAPH:
	DrawSteps(plot);
	DrawErrorBars(plot);
	break;
    }
    /*
    ** draw any labels associated with the plot
    */
    DisplayPlotLabels(plot);
    /* SetLinestyle(plot->graph,SOLID_LINE); */
}


MenuDeletePlot(item)
MenuItem	*item;
{
    DeletePlot(SelectedPlot(G));
    RefreshGraph(G);
}

FreePlot(plot)
Plot *plot;
{
int	i;

    if(plot == NULL) return;
    if(plot->coord){
	free(plot->coord);
    }
    if(plot->fcoord){
	free(plot->fcoord);
    }
    if(plot->rawdata){
	for(i=0;i<plot->npoints;i++){
	    if(plot->rawdata[i].data){
		free(plot->rawdata[i].data);
	    }
	}
	free(plot->rawdata);
    }
    free(plot);
}

DeletePlot(plot)
Plot *plot;
{
Plot *p;
Graph *graph;
Plot *sp;
Plot *spnext;

    if(plot == NULL) return;
    graph = plot->graph;
    if((sp = SelectedPlot(graph)) != NULL){
	spnext = sp->next;
    } else {
	spnext = NULL;
    }
    /*
    ** is the plot to be deleted the first on the list
    */
    if(graph->plot == plot){
	graph->plot = plot->next;
	DeleteAllPlotLabels(plot);
	FreePlot(plot);
    } else {
	/*
	** search for it
	*/
	for(p=graph->plot;p;p=p->next){
	    if(p->next == plot){
		/*
		** found it so delete it
		*/
		p->next = plot->next;
		DeleteAllPlotLabels(plot);
		FreePlot(plot);
		break;
	    }
	}
    }
    /*
    ** if the deleted plot had been the currently selected on the
    ** select a different plot
    */
    if(sp == plot){
	if(spnext != NULL){
	    SelectPlot(spnext);
	} else {
	    SelectPlotByNumber(graph,0);
	}
    }
}

DeleteAllPlotLabels(plot)
Plot	*plot;
{
Label	*label;
Label	*nextlabel;

    if(plot == NULL) return;
    nextlabel = NULL;
    for(label = plot->label;label;label=nextlabel){
	nextlabel = label->next;
	free(label);
    }
    plot->label = NULL;
}
