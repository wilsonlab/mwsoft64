#include "xclust_ext.h"

CalculateTickCount(interval,origin,wmin,wmax,start,nticks)
float 	interval;
float 	origin;
float 	wmin,wmax;
float 	*start;
int	*nticks;
{
    /*
    ** find the number of intervals which will fit in wmax-wmin
    */
    *nticks = (wmax - wmin)/(interval) + 1.5;

    /*
    ** the starting tick is the first multiple of the interval
    ** from the origin that appears after wmin
    */
    if(wmin - origin > 0){
	*start = origin + (int)(1 + (wmin - origin)/(interval))*(interval);
    } else {
	*start = origin + (int)((wmin - origin)/(interval))*(interval);
    }
}

float CalculateTickInterval(desired_nticks,origin,wmin,wmax)
int 	desired_nticks;
float 	origin;
float 	wmin,wmax;
{
float	base_interval;
float	interval_hint;
int	total_ticks;
int	tickscale;
float	logscale;
float	interval;

    /*
    ** find an even spacing of ticks over the interval wmin to wmax
    */
    interval_hint = (wmax -wmin)/desired_nticks;

    /*
    ** find the base interval
    */
    logscale = log10((double)interval_hint);
    /*
    ** always go to the finer resolution
    */
    if(logscale < 0){
	/* push it down to the next log interval */
	base_interval = pow((double)10.0,(double)((int)(logscale - 1)));
    } else {
	/* rounds down to the next log interval */
	base_interval = pow((double)10.0,(double)((int)logscale));
    }

    if(base_interval == 0){
	base_interval = 1;
    }
    /*
    ** calculate the integer number of base intervals that will fit in the
    ** total_interval
    */
    total_ticks = (wmax - wmin)/base_interval;
    if(total_ticks > desired_nticks){
	/*
	** find the multiple of the total_ticks which is closest to the
	** desired ticks
	*/
	tickscale = (float)total_ticks/desired_nticks +.5;
	interval = base_interval*tickscale;
    } else {
	/*
	** find the multiple of the total_ticks which is closest to the
	** desired ticks
	*/
	tickscale = (float)desired_nticks/total_ticks +.5;
	interval = base_interval/tickscale;
    }
    return(interval);
}

DrawXTickLabel(graph,sx,sy,val,left,right)
Graph *graph;
int sx,sy;
float val;
int left,right;
{
char format[80];
char label[80];
int 	tw,th;

    SetColor(MININTERFACECOLOR + AXESTICKLABELS);
    if(right < 0){
	sprintf(format,"%%%dg",-right);
    } else {
	sprintf(format,"%%%d.%df",left,right);
    }
    sprintf(label,format,val);
    TextExtent(label,&tw,&th);
    Text(graph,sx - tw/2,
    (int)(sy + 2*graph->ticksize + th +graph->xaxis.ticklabel_offset),label);
}

DrawYTickLabel(graph,sx,sy,val,left,right)
Graph *graph;
int sx,sy;
float val;
int left,right;
{
char format[80];
char label[80];
int 	tw,th;

    SetColor(MININTERFACECOLOR + AXESTICKLABELS);
    if(right < 0){
	sprintf(format,"%%%dg",-right);
    } else {
	sprintf(format,"%%%d.%df",left,right);
    }
    sprintf(label,format,val);
    TextExtent(label,&tw,&th);
    Text(graph,sx -tw - 2*graph->ticksize +graph->yaxis.ticklabel_offset,
    sy + th/2,label);
}

CalculateYTicks(graph)
Graph *graph;
{
int	desired_nticks;

    /*
    ** if the desired tick count is less than 0 then
    ** use the tick count as a screen coordinate spacing
    ** between ticks
    */
    if(graph->yaxis.desired_nticks < 0){
	desired_nticks = -((float)(graph->wheight)/graph->yaxis.desired_nticks) +.5;
    } else {
	desired_nticks = graph->yaxis.desired_nticks;
    }
    /*
    ** get reasonable tick intervals for the given number of ticks
    */
    if(graph->yaxis.desired_tickinc > 0){
	graph->yaxis.tickinc = graph->yaxis.desired_tickinc;
    } else {
	/*
	** calculate tick spacing
	*/
	graph->yaxis.tickinc = 
	CalculateTickInterval(desired_nticks,graph->xaxis.yintcpt,
	graph->wymin,graph->wymax);
    }
    CalculateTickCount(graph->yaxis.tickinc,graph->xaxis.yintcpt,graph->wymin,
    graph->wymax,&(graph->yaxis.tickstart),&(graph->yaxis.nticks));
}

CalculateXTicks(graph)
Graph *graph;
{
int	desired_nticks;

    /*
    ** if the desired tick count is less than 0 then
    ** use the tick count as a screen coordinate spacing
    ** between ticks
    */
    if(graph->xaxis.desired_nticks < 0){
	desired_nticks = -((float)(graph->wwidth)/graph->xaxis.desired_nticks) +.5;
    } else {
	desired_nticks = graph->xaxis.desired_nticks;
    }
    if(graph->xaxis.desired_tickinc > 0){
	graph->xaxis.tickinc = graph->xaxis.desired_tickinc;
    } else {
	/*
	** calculate tick spacing
	*/
	graph->xaxis.tickinc = 
	CalculateTickInterval(desired_nticks,graph->yaxis.xintcpt,
	graph->wxmin,graph->wxmax);
    }
    CalculateTickCount(graph->xaxis.tickinc,graph->yaxis.xintcpt,graph->wxmin,
    graph->wxmax,&(graph->xaxis.tickstart),&(graph->xaxis.nticks));
}

DrawYTicks(graph)
Graph *graph;
{
int	sx1,sy1;
int	sx2,sy2;
int	i;
float	xint;
float	val;
float	scale;
int	tmp;
int	axisy;
int	j;
double	ts,te;

    if(graph->yaxis.desired_nticks == 0){
	graph->yaxis.nticks = 1;
	return;
    }
    /*
    ** get the screen coordinates of the axis
    */
    SetColor(MININTERFACECOLOR + AXESTICKS);
    xint = graph->yaxis.xintcpt;
    ScreenTransform(graph,xint,graph->wymin,&sx1,&sy1);
    ScreenTransform(graph,xint,graph->wymax,&sx2,&sy2);
    /*
    ** get the axis exponent
    */
    scale = 1.0/pow((double)10.0,(double)graph->yaxis.exponent);
    ScreenTransform(graph,0.0,graph->xaxis.yintcpt,&tmp,&axisy);
    /*
    ** loop over all the tick marks
    */
    for(i=0;i<graph->yaxis.nticks;i++){
	for(j=0;j<graph->yaxis.nsubticks+1;j++){
	  SetColor(MININTERFACECOLOR + AXESTICKS);
	    /*
	    ** get the actual value of the tick
	    */
            switch(graph->yaxis.type){
            case LINEAR_AXIS:
                val = i*graph->yaxis.tickinc + graph->yaxis.tickstart +
                j*graph->yaxis.tickinc/(graph->yaxis.nsubticks +1);
                break;
            case LOG10_AXIS:
                ts = pow(10.0,(double)(i*graph->yaxis.tickinc +
                        graph->yaxis.tickstart));
                te = pow(10.0,(double)((i+1)*graph->yaxis.tickinc +
                        graph->yaxis.tickstart));
                val = log10(ts + (te -ts)*j/(graph->yaxis.nsubticks + 1));
                break;
            }

	    /*
	    ** locate its screen position in the window
	    */
	    ScreenTransform(graph,graph->yaxis.xintcpt,val,&sx1,&sy1);
	    /*
	    ** either draw a grid or just tick marks at the tick intervals
	    */
	    if(graph->yaxis.show_grid){
		SetColor(graph->gridcolor);
		SetLinestyle(graph,1);
		DrawLine(0,sy1,graph->wwidth,sy1);
		SetLinestyle(graph,0);
	    } 
	    if(!(graph->quadrants & 0x2) && !(graph->quadrants & 0x8)){
		continue;
	    }
	    /*
	    ** check for quadrants II
	    */
	    if(!(graph->quadrants & 0x2) && sy1 < axisy){
		continue;
	    }
	    /*
	    ** check for quadrants IV
	    */
	    if(!(graph->quadrants & 0x8) && sy1 > axisy){
		continue;
	    }
	    if(j==0){
		DrawLine(sx1-graph->ticksize,sy1,sx2+graph->ticksize,sy1);
		/*
		** label the ticks
		*/
		if(graph->yaxis.show_labels){
                    switch(graph->yaxis.type){
                    case LINEAR_AXIS:
                        DrawYTickLabel(graph,sx1,sy1,
                        val*scale,graph->yaxis.leftdp,graph->yaxis.rightdp);
                        break;
                    case LOG10_AXIS:
                        DrawYTickLabel(graph,sx1,sy1,pow(10.0,val*scale),
                        graph->yaxis.leftdp,graph->yaxis.rightdp);
                        break;
                    }
		}
	    } else {
		DrawLine(sx1- (int)(0.5*graph->ticksize),sy1,
		sx2+(int)(0.5*graph->ticksize), sy1);
	    }
	}
    }
}

DrawXTicks(graph)
Graph *graph;
{
int	sx1,sy1;
int	sx2,sy2;
int	i;
float	yint;
float	val;
char	label[100];
float	scale;
int	tmp;
int	axisx;
int	j;
float	tinc;
float	tstart;
double	ts,te;

    if(graph->xaxis.desired_nticks == 0){
	graph->xaxis.nticks = 1;
	return;
    }
    /*
    ** get the screen coordinates of the axis
    */
    SetColor(MININTERFACECOLOR + AXESTICKS);
    yint = graph->xaxis.yintcpt;
    ScreenTransform(graph,graph->wxmin,yint,&sx1,&sy1);
    ScreenTransform(graph,graph->wxmax,yint,&sx2,&sy2);

    scale = 1.0/pow((double)10.0,(double)graph->xaxis.exponent);
    ScreenTransform(graph,graph->yaxis.xintcpt,0.0,&axisx,&tmp);
    for(i=0;i<graph->xaxis.nticks;i++){
	for(j=0;j<graph->xaxis.nsubticks+1;j++){

	  SetColor(MININTERFACECOLOR + AXESTICKS);
            switch(graph->xaxis.type){
            case LINEAR_AXIS:
                val = i*graph->xaxis.tickinc + graph->xaxis.tickstart +
                j*graph->xaxis.tickinc/(graph->xaxis.nsubticks +1);
                break;
            case LOG10_AXIS:
                ts = pow(10.0,(double)(i*graph->xaxis.tickinc +
                        graph->xaxis.tickstart));
                te = pow(10.0,(double)((i+1)*graph->xaxis.tickinc +
                        graph->xaxis.tickstart));
                val = log10(ts + (te -ts)*j/(graph->xaxis.nsubticks + 1));
                break;
            }
	    ScreenTransform(graph,val,graph->xaxis.yintcpt,&sx1,&sy1);
	    /*
	    ** either draw a grid or just tick marks at the tick intervals
	    */
	    if(graph->xaxis.show_grid){
		SetColor(graph->gridcolor);
		SetLinestyle(graph,1);
		DrawLine(sx1,0,sx1,graph->wheight);
		SetLinestyle(graph,0);
	    }
	    if(!(graph->quadrants & 0x1) && !(graph->quadrants & 0x4)){
		continue;
	    }
	    /*
	    ** check for quadrants I 
	    */
	    if(!(graph->quadrants & 0x1) && sx1 > axisx){
		continue;
	    }
	    /*
	    ** check for quadrants III
	    */
	    if(!(graph->quadrants & 0x4) && sx1 <axisx){
		continue;
	    }
	    if(j==0){
		DrawLine(sx1,sy1-graph->ticksize,sx1,sy2+graph->ticksize);
		/*
		** label the ticks
		*/
		if(graph->xaxis.show_labels){
                    switch(graph->xaxis.type){
                    case LINEAR_AXIS:
                        DrawXTickLabel(graph,sx1,sy1,
                        val*scale,graph->xaxis.leftdp,graph->xaxis.rightdp);
                        break;
                    case LOG10_AXIS:
                        DrawXTickLabel(graph,sx1,sy1,pow(10.0,val*scale),
                        graph->xaxis.leftdp,graph->xaxis.rightdp);
                        break;
                    }
		}
	    } else {
		DrawLine(sx1,sy1- (int)(0.5*graph->ticksize),sx1,
		sy2+(int)(0.5*graph->ticksize));
	    }
	}
    }
}

