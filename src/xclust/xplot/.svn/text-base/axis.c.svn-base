#include "xplot_ext.h"

DrawAxes(graph)
Graph *graph;
{

    /*
    ** label the axes
    */
    if(graph->yaxis.show_labels && (graph->xaxis.title != NULL)){
	DrawYAxisLabel(graph);
    }
    if(graph->xaxis.show_labels && (graph->xaxis.title != NULL)){
	DrawXAxisLabel(graph);
    }
    if(graph->yaxis.show){
	DrawYTicks(graph);
    }
    if(graph->xaxis.show){
	DrawXTicks(graph);
    }
    /*
    ** draw the axis labels
    */
    if(graph->yaxis.show){
	DrawYAxis(graph);
    }
    if(graph->xaxis.show){
	DrawXAxis(graph);
    }
}

CalculateAxisParameters(graph)
Graph *graph;
{
    if(graph->auto_xaxis){
	AutoXOrigin(graph);
    }
    if(graph->auto_yaxis){
	AutoYOrigin(graph);
    }
    CalculateYTicks(graph);
    CalculateXTicks(graph);
    /*
    ** calculate the axis scale factor
    */
    CalculateAxisScale(graph->wxmin,graph->wxmax,
	graph->xaxis.leftdp,graph->xaxis.rightdp,&(graph->xaxis.exponent),
	graph->xaxis.nticks);
    CalculateAxisScale(graph->wymin,graph->wymax,
	graph->yaxis.leftdp,graph->yaxis.rightdp,&(graph->yaxis.exponent),
	graph->yaxis.nticks);
}

CalculateAxisScale(wmin,wmax,left,right,exponent,nticks)
float wmin,wmax;
int left,right;
int *exponent;
int nticks;
{
int scale_factor;

    /*
    ** determine the scale factor which will allow
    ** for unique values for each tick with the given precision
    */
    scale_factor = log10((double)((wmax-wmin)/nticks));
    /*
    ** if log is negative then check against right hand size
    */
    if(scale_factor < 0){
	/*
	** only scale if the exponent exceeds the available field size 
	*/
	/*
	** check for auto field formatting
	*/
	if(right < 0){
	    if(-scale_factor > -right ){
		*exponent = scale_factor;
	    } else {
		*exponent = 0;
	    }
	} else {
	    if(-scale_factor > right){
		*exponent = scale_factor;
	    } else {
		*exponent = 0;
	    }
	}
    } else  {
	/*
	** if log is positive then check against left hand size
	*/
	/*
	** only scale if the exponent exceeds the available field size 
	*/
	if(scale_factor > left){
	    *exponent = scale_factor;
	} else {
	    *exponent = 0;
	}
    } 
}


DrawXAxis(graph)
Graph *graph;
{
int	sy;
int	tmp;
int	sx1;
int	sx2;
int	axisx;
Plot	*plot;

    /*
    ** get the screen coordinates of the axis
    ** should probably make sure they are reasonable screen
    ** coords since X has such problems if they are not
    */
    SetColor(graph->foreground);
    ScreenTransform(graph,graph->wxmax,graph->xaxis.yintcpt,&sx2,&sy);
    ScreenTransform(graph,graph->yaxis.xintcpt,0.0,&axisx,&tmp);
    ScreenTransform(graph,graph->wxmin,0.0,&sx1,&tmp);
    /*
    ** draw the axis line
    */
    /*
    ** check for quadrant I 
    */
    if(graph->quadrants & 0x1){
	DrawLine(axisx,sy,sx2,sy);
    }
    /*
    ** check for quadrant III
    */
    if(graph->quadrants & 0x4){
	DrawLine(sx1,sy,axisx,sy);
    }
    /*
    ** check each plot to see whether it has its own axis
    */
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->xaxis && plot->visible){
	    ScreenTransform(graph,graph->wxmax,graph->xaxis.yintcpt+plot->yoffset_axis,&tmp,&sy);
	    if(graph->quadrants & 0x1){
		DrawLine(axisx,sy,sx2,sy);
	    }
	    if(graph->quadrants & 0x4){
		DrawLine(sx1,sy,axisx,sy);
	    }
	}
    }
}

DrawYAxis(graph)
Graph *graph;
{
int	sx;
int	sy1;
int	sy2;
int	tmp;
int	axisy;
Plot	*plot;

    /*
    ** draw the axis through the desired intercept
    */
    SetColor(graph->foreground);
    ScreenTransform(graph,graph->yaxis.xintcpt,graph->wymin,&sx,&sy1);
    ScreenTransform(graph,0.0,graph->xaxis.yintcpt,&tmp,&axisy);
    ScreenTransform(graph,0.0,graph->wymax,&tmp,&sy2);
    /*
    ** draw the axis line
    */
    /*
    ** check for quadrant II
    */
    if(graph->quadrants & 0x2){
	DrawLine(sx,axisy,sx,sy2);
    }
    /*
    ** check for quadrant IV
    */
    if(graph->quadrants & 0x8){
	DrawLine(sx,sy1,sx,axisy);
    }
    /*
    ** check each plot to see whether it has its own axis
    */
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->yaxis && plot->visible){
	    ScreenTransform(graph,graph->yaxis.xintcpt+plot->xoffset_axis,0.0,&sx,&tmp);
	    if(graph->quadrants & 0x2){
		DrawLine(sx,axisy,sx,sy2);
	    }
	    if(graph->quadrants & 0x8){
		DrawLine(sx,sy1,sx,axisy);
	    }
	}
    }
}

