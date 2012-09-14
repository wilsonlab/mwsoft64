#include "behav_ext.h"

void ComputeOccupancy(result,x,y,direction,timestamp,gap)
Result	*result;
int	x,y,direction;
unsigned long	timestamp;
int	gap;
{
int	interval;
float	rate;

    /*
    ** how much time has been spent in the current location
    */
    interval = timestamp - result->occupancystart;
    /*
    ** check for a limit on the occupancy time
    */
    if(result->limited_occupancy){
	if(interval > result->limited_occupancy){
	    interval = result->limited_occupancy;
	}
    }
    /*
    ** compute the firing rate over the occupancy interval
    */
    rate = 1.0e4*result->occupancyspikes/interval;
#if(STANDARD_OCCUPANCY == 1) 
    /*
    ** keep track of the total firing rate at this location
    */
    result->occupancy_grid[x][y] += rate;
    result->occupancy_gridsqr[x][y] += rate*rate;
    /*
    ** keep track of the number of occupancies at this location
    */
    result->occupancy_gridn[x][y]++;
    if(result->fpocc){
	fprintf(result->fpocc,"%d\t%d\t%d",
	    x,y,direction/DIRBINSIZE);
	if(rate>result->minrate){
	    fprintf(result->fpocc,"\t%g\t%d\n", rate, interval);
	} else  {
	    fprintf(result->fpocc,"\t%g\t%d\n", 0.0, interval);
	}
	if(result->directionline && rate > result->minrate){
	    /*
	    ** add xplot commands to draw a line to represent
	    ** head direction
	    */
	    fprintf(result->fpocc,"/wline %d\t%d\t%d\t%g\t%g\t%d\n",
	    x,y,0,x+LINELEN*cos((double)(2*M_PI*direction/360)),
	    y+LINELEN*sin((double)(2*M_PI*direction/360)),0);
	}
    }
#else
    /*
    ** occupancy can/should also be output with uniform 
    ** occupancy time intervals.
    ** this is accomplished by splitting an interval into
    ** multiple occupancies of length result->pinterval
    */
    n = interval/(1e4*result->pinterval) + .5;
    for(i=0;i<n;i++){
	if(result->fpocc){
	    fprintf(result->fpocc,"%d\t%d\t%d",
		x,y,direction/DIRBINSIZE);
	    if(rate>result->minrate){
		fprintf(result->fpocc,"\t%g\n", rate);
	    } else  {
		fprintf(result->fpocc,"\t%g\n", 0.0);
	    }
	    if(result->directionline && rate> result->minrate){
		/*
		** add xplot commands to draw a line to represent
		** head direction
		*/
		fprintf(result->fpocc,"/wline %d\t%d\t%g\t%g\n",
		x,y,x+LINELEN*cos((double)(2*M_PI*direction/360)),
		y+LINELEN*sin((double)(2*M_PI*direction/360)));
	    }
	}
	result->occupancy_grid[x][y] += rate;
	result->occupancy_gridsqr[x][y] += rate*rate;
	result->occupancy_gridn[x][y]++;
    }
#endif
}

double MeanOccupancyRate(result)
Result	*result;
{
int	i;
int	j;
int	n;
double	sum;
double	mean;
int	nmin;
int	count;

    count = 0;
    mean = 0;
    nmin = 5;
    for(i=0;i<result->xsize;i++){
	for(j=0;j<result->xsize;j++){
	    n = result->occupancy_gridn[i][j];
	    /*
	    ** only take points which have enough samples
	    */
	    if(n > nmin){
		count++;
		sum = result->occupancy_grid[i][j];
		mean += sum/n;
	    }
	}
    }
    if(count == 0){
	return(0);
    } else {
	return(mean/count);
    }
}

