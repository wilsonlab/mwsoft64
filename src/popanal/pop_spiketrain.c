#include "pop_ext.h"

/*
** evaluate the population vector over time
*/
ProcessPopulationSpikeTrain(result)
Result	*result;
{
double	totalpositionerror;
int32_t	npositionsamples;
int32_t	i,j;
int32_t	x,y;
int32_t	x2,y2;
int32_t	invalidposition;
int32_t	actualx,actualy;
double	magnitude;
Position	*tielist;
int32_t	gx,gy;
float	vectorsum;
float	vectorsumsqr;
double	vectorval;
int32_t	count;
float	**grid;
float	vectormean;
float	vectormag;
float	vectorsparsity;
float	*gridvec;
float	*corrvec;
float	*magvec;
int32_t	bestx,besty;
uint32_t	timestamp;
double	maxcorr;
double	positionerror;
double	sum;
int32_t	nties;
int32_t	choice;
int32_t	timebin;
int32_t	ptimebin;
int32_t	interval;
float	speed;
double	dataval;
int32_t	shufindex;
int32_t	*shuffleflags;
float	distance;
int32_t	prevx,prevy;

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

