#include "pop_ext.h"

/*
** evaluate the population vector over time
*/
ProcessPopulationSpikeTrain(result)
Result	*result;
{
double	totalpositionerror;
int	npositionsamples;
int	i,j;
int	x,y;
int	x2,y2;
int	invalidposition;
int	actualx,actualy;
double	magnitude;
Position	*tielist;
int	gx,gy;
float	vectorsum;
float	vectorsumsqr;
double	vectorval;
int	count;
float	**grid;
float	vectormean;
float	vectormag;
float	vectorsparsity;
float	*gridvec;
float	*corrvec;
float	*magvec;
int	bestx,besty;
unsigned long	timestamp;
double	maxcorr;
double	positionerror;
double	sum;
int	nties;
int	choice;
int	timebin;
int	ptimebin;
int	interval;
float	speed;
double	dataval;
int	shufindex;
int	*shuffleflags;
float	distance;
int	prevx,prevy;

    ptimebin = -1;
    if(result->fppoptrainout || result->fppopvecout){
	for(i=0;i<result->ntimebins;i++){
	    count = 0;
	    if(verbose){
		if(i%(result->ntimebins/100) == 0){
		    fprintf(stderr,"%4d%%\b\b\b\b\b",100*i/result->ntimebins);
		}
	    }
	    /*
	    ** compute the time bin for the spike information by adding
	    ** any offset that was requested
	    */
	    timebin = i+result->spike_bin_offset;
	    if(timebin < 0 || timebin >= result->ntimebins){
		/*
		** only process spikes that are in range
		*/
		continue;
	    }
	    /*
	    ** and that are valid
	    if(!result->timebinstatus[timebin].valid) continue;
	    */
	    vectorsum = 0;
	    vectorsumsqr = 0;
	    if(result->hasposition && result->fppopvecout){
		fprintf(result->fppopvecout,"%d\t%d\t",
		    result->position[i].x,
		    result->position[i].y);
	    }
	    /*
	    ** go through the population
	    */
	    for(j=0;j<result->nclusters;j++){
		if(clusterdir[j].ignore) continue;
		count++;
		dataval = result->datavector[timebin][j];
		if(result->fppopvecout){
			if(result->popthresh > 0){
			fprintf(result->fppopvecout,"%d\t",
			    dataval > result->popthresh);
			} else {
			fprintf(result->fppopvecout,"%g\t",
			    dataval);
			}
		}
		vectorsum += dataval;
		vectorsumsqr += dataval*dataval;
	    }
	    if(result->fppoptrainout){
		fprintf(result->fppopvecout,"\n");
	    }
	    if(count > 0){
		/*
		** compute sparsity as the cos of the angle between
		** the vector and the unit vector
		** <x,1>/|x||1|
		*/
		vectormean = vectorsum/count;
		vectormean = vectorsum;
		vectormag = sqrt(vectorsumsqr);
		if(vectormag > TINYVAL){
		    vectorsparsity = vectorsum/(vectormag * sqrt((double)count));
		} else {
		    vectorsparsity = 0;
		}
	    } else {
		vectormean = 0;
		vectormag = 0;
		vectorsparsity = 0;
	    }
	    /*
	    ** output the xy coord of the peak correlation vector
	    */
	    timestamp = result->tstart + result->binsize*i;
	    if(ptimebin > 0){
		interval = timebin - ptimebin;
	    } else {
		interval = -1;
	    }
	    ptimebin = timebin;
	    if(result->fppoptrainout){
		fprintf(result->fppoptrainout,"%g\n",
		    vectormean);
	    }
	}
    }
}

