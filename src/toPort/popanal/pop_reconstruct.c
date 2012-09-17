#include "pop_ext.h"

/*
** evaluate correlation of each timestamp population vector
** with position grid vectors
*/
ReconstructPosition(result)
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

    /*
    ** allocate the tie list. Make it large enough to hold the maximum
    ** number of tied reconstructed positions.
    */
    tielist = (Position *)malloc((result->yhi - result->ylo + 1)*
	(result->xhi - result->xlo + 1)*sizeof(Position));
    if(result->vshuffle){
	shuffleflags = (int *)malloc(sizeof(int)*result->nclusters);
    }
    /*
    ** evaluate correlation of each timestamp population vector
    ** with position grid vectors
    */
    totalpositionerror = 0;
    npositionsamples = 0;
    result->ambiguous = 0;
    result->nambiguous = 0;
    ptimebin = -1;
    prevx = -1;
    if(result->fppout || result->fpcorrout){
	for(i=0;i<result->ntimebins;i++){
	    if(result->position){
		if(result->posinterpolate){
			actualx = (result->position[i].x + result->position[i].x2)/2;
			actualy = (result->position[i].y + result->position[i].y2)/2;
		} else {
			actualx = result->position[i].x;
			actualy = result->position[i].y;
		}
		if(actualx == 0 && actualy == 0){
		    invalidposition = 1;
		} else {
		    invalidposition = 0;
		}
		/*
		** transform the actual coordinate into grid
		** coordinates 
		*/
		gy = actualy*(result->ymax+1)/result->yresolution;
		gx = actualx*(result->xmax+1)/result->xresolution;
		/*
		** dont process positions outside of the 
		** bounds
		*/
		if((result->gxlo >= 0) && 
		(gx < result->gxlo || gx > result->gxhi || 
		gy < result->gylo || gy  > result->gyhi)){
		    invalidposition = 1;
		} 
		/*
		** keep track of occupancies
		*/
		result->occgrid[gy][gx]++;
	    }
	    count = 0;
	    if(verbose){
		if(i%(result->ntimebins/10) == 0){
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
	    */
	    if(!result->timebinstatus[timebin].valid) continue;
	    /*
	    ** clear the correlation grid
	    */
	    for(y=0;y<=result->ymax;y++){
		bzero(result->corrgrid[y],(result->xmax+1)*sizeof(float));
	    }
	    /*
	    ** fill the correlation grid
	    */
	    magnitude = 0;
	    vectorsum = 0;
	    vectorsumsqr = 0;
	    if(result->vshuffle){
		/*
		** prepare the array to keep track of the shuffled
		** vector components that are left for placement
		*/
		for(j=0;j<result->nclusters;j++){
		/*
		    if(result->pdir[j].ignore || result->peak[j] <= 0 ){
		    */
		    if(result->pdir[j].ignore){
			shuffleflags[j] = 1;
		    } else {
			shuffleflags[j] = 0;
		    }
		}
	    }
	    for(j=0;j<result->nclusters;j++){
		if(result->pdir[j].ignore) continue;
		if(result->spikecount[j] < result->minspikes) continue;
		if((result->maxspikes > 0) &&
		(result->spikecount[j] > result->maxspikes)) continue;
		/*
		** dont process zero rate clusters
		if(result->peak[j] <= 0) continue;
		*/
		/*
		** the result of these conditions is to process only
		** clusters which have some spatial firing
		*/
		count++;
		/*
		** if looking at shuffled vectors then shuffle them
		** here
		*/
		if(result->vshuffle){
		    /*
		    ** pick one at random to fill this vector location
		    */
		    shufindex = (int)(frandom(0,result->nclusters))%
		    result->nclusters;
		    /*
		    ** if it is in use then scan for the next free one
		    */
		    while(shuffleflags[shufindex]){
			shufindex = (shufindex+1)%result->nclusters;
		    }
		    /*
		    ** mark the component as used
		    */
		    shuffleflags[shufindex] = 1;
		    dataval = result->datavector[timebin][shufindex];
		} else {
		    dataval = result->datavector[timebin][j];
		}
		vectorsum += dataval;
		vectorsumsqr += dataval*dataval;
		if(result->peaknormalize){
		    if(result->peak[j] != 0){
			/*
			** normalize the vector component by the position peak 
			** rate
			*/
			vectorval = dataval/result->peak[j];
		    }
		} else {
		    vectorval = dataval;
		}
		grid = result->positiongrid[j];
		/*
		** keep track of the magnitude of the data vector
		*/
		magnitude += vectorval*vectorval;
		for(y=result->ylo;y<=result->yhi;y++){
		    gridvec = grid[y];
		    corrvec = result->corrgrid[y];
		    for(x=result->xlo;x<=result->xhi;x++){
			/*
			** compute the correlation of the pop vector and the
			** position grid vectors
			** Normalize each component by the cluster peak rate
			*/
			corrvec[x] += vectorval*gridvec[x];
		    }
		}
	    }
	    if(count > 0){
		/*
		** compute sparsity as the cos of the angle between
		** the vector and the unit vector
		** <x,1>/|x||1|
		*/
		vectormean = vectorsum/count;
		vectormag = sqrt(vectorsumsqr);
		if(vectormag > TINYVAL){
		    vectorsparsity = vectorsum/(vectormag * sqrt(count));
		} else {
		    vectorsparsity = 0;
		}
	    } else {
		vectormean = 0;
		vectormag = 0;
		vectorsparsity = 0;
	    }
	    /*
	    ** normalize the corr grid
	    */
	    for(y=result->ylo;y<=result->yhi;y++){
		corrvec = result->corrgrid[y];
		magvec = result->maggrid[y];
		for(x=result->xlo;x<=result->xhi;x++){
		    /*
		    ** normalize by the magnitudes of the data and grid
		    ** vectors
		    */
		    if(magvec[x] > 0 && magnitude > 0){
			corrvec[x] /= (sqrt((double)magnitude)*magvec[x]);
		    }
		}
	    }
	    if(result->smoothcorr){
		/*
		** smooth the corr grid
		*/
		for(y=result->ylo;y<=result->yhi;y++){
		    for(x=result->xlo;x<=result->xhi;x++){
			/*
			** smooth the grid by summing adjacent bins
			*/
			sum = 0;
			count = 0;
			for(y2=y-result->smoothcorr;y2<=y+result->smoothcorr;y2++){
			    /*
			    ** dont include points outside the bounds
			    ** in the smoothing
			    */
			    if((y2 < result->ylo) || (y2 > result->yhi)){
				continue;
			    }
			    for(x2=x-result->smoothcorr;x2<=x+result->smoothcorr;x2++){
				/*
				** dont include points outside the bounds
				** in the smoothing
				*/
				if((x2 >= result->xlo) && (x2 <= result->xhi)){
				    sum += result->corrgrid[y2][x2];
				    count++;
				} 
			    }
			}
			/*
			** fill the smoothed matrix
			*/
			if(count > 0){
			    result->smoothcorrgrid[y][x] = sum/count;
			} else {
			    result->smoothcorrgrid[y][x] = 0;
			}
		    }
		}
	    }
	    /*
	    ** evaluate the max corr grid position
	    */
	    maxcorr = -1;
	    prevx = bestx;
	    prevy = besty;
	    bestx = 0;
	    besty = 0;
	    nties = 0;
	    for(y=result->ylo;y<=result->yhi;y++){
		/*
		** use the smoothed or unsmoothed correlation grid
		*/
		if(result->smoothcorr){
		    corrvec = result->smoothcorrgrid[y];
		} else {
		    corrvec = result->corrgrid[y];
		}
		for(x=result->xlo;x<=result->xhi;x++){
		    if(corrvec[x] > maxcorr){
			maxcorr = corrvec[x];
			bestx = x*result->xresolution/(result->xmax+1);
			besty = y*result->yresolution/(result->ymax+1);
			nties = 0;
		    } else 
		    if(corrvec[x] == maxcorr){
			/*
			** in case of a tie, keep track of
			** the ties so that a random
			** choice can be made later
			*/
			tielist[nties].x = x;
			tielist[nties].y = y;
			nties++;
		    } 

		}
	    }
	    if(nties > 0){
		/*
		** pick one of the tied positions at random
		*/
		choice = frandom(0,nties);
		if((choice < 0) || (choice >= nties)){
		    fprintf(stderr,"bad unresolved tie due to bad random #\n");
		} else {
		    bestx = tielist[choice].x*result->xresolution/(result->xmax+1);
		    besty = tielist[choice].y*result->yresolution/(result->ymax+1);
		}
		result->nambiguous++;
		result->ambiguous += nties;
	    }
	    /*
	    ** output the xy coord of the peak correlation vector
	    */
	    timestamp = result->tstart + result->binsize*i;
	    if(result->hasposition){
		speed = result->speed[i];
	    } else {
		speed = -1;
	    }
	    if(bestx > 0 && prevx > 0){
		distance = sqrt((double)((bestx - prevx)*(bestx - prevx) + 
		    (besty - prevy)*(besty - prevy)));
	    } else {
		distance = -1;
	    }
	    if(ptimebin > 0){
		interval = timebin - ptimebin;
	    } else {
		interval = -1;
	    }
	    ptimebin = timebin;
	    if(result->fpdout){
		fprintf(result->fpdout,"%u\t%d\t%d\t%g\t%d\n",
		    timestamp,
		    bestx,
		    besty,
		    distance,
		    interval);
	    }
	    if(result->fppout){
		fprintf(result->fppout,"%u\t%g\t%g\t%g\t%d\t%d\t%9.6g\t%d",
		    timestamp,
		    vectormean,
		    speed,
		    vectormean/vectormag,
		    bestx,
		    besty,
		    1-maxcorr,
		    nties);
	    }
	    if(result->hasposition){
		/*
		** store the reconstructed position for later
		** use in error distribution evaluation
		*/
		result->calcposition[i].x = bestx;
		result->calcposition[i].y = besty;
		if(result->position){
		    /*
		    ** and the error from the actual position if
		    ** available
		    */
		    if(invalidposition){
			/*
			** dont evaluate invalid positional samples
			*/
			positionerror = 0;
		    } else {
			positionerror = sqrt((double)(
				((actualx - bestx)*(actualx - bestx)) +
				((actualy - besty)*(actualy - besty))));
			/*
			** keep track of the cumulative position error
			*/
			totalpositionerror += positionerror;
			npositionsamples++;
		    }
		}
		if(result->comparepositions){
		    fprintf(result->fppout,"\t%d\t%d",
			actualx,actualy);
		    if(invalidposition){
			fprintf(result->fppout,"\t%s",
			    "NA");
		    } else {
			fprintf(result->fppout,"\t%g",
			    positionerror);
		    }
		}
		fprintf(result->fppout,"\n");
		fflush(result->fppout);
		/*
		** keep track of the error grid?
		*/
		if(result->fpperrgrid){
		    /*
		    ** sum the positionerror
		    */
		    result->perrgrid[gy][gx] += positionerror;
		}
	    } else {
		fprintf(result->fppout,"\n");
		fflush(result->fppout);
	    }
	    /*
	    ** if correlation grid output is selected then dump the grid for
	    ** this time slice
	    */
	    if(result->fpcorrout && magnitude > 0){
		for(y=0;y<=result->ymax;y++){
		    if(result->smoothcorr){
			fwrite(result->smoothcorrgrid[y],sizeof(float),
			    result->xmax+1,result->fpcorrout);
		    } else {
			fwrite(result->corrgrid[y],sizeof(float),
			    result->xmax+1,result->fpcorrout);
		    }
		}
	    }
	}
    }
    if(npositionsamples > 0){
	result->meanpositionerror = totalpositionerror/npositionsamples;
    } else {
	result->meanpositionerror = 0;
    }
}


#define NSHIFTS		500
EvaluateErrors(result,argc,argv)
int	argc;
char	**argv;
Result	*result;
{
int	i;
int	n;
double	positionerror;
double	totalerror;
int	shiftval;
int	goodcount;
int	ishift;
double	xerr;
double	yerr;
int	npositionsamples;

    /*
    ** evaluate the mean positional error for shifted versions of the
    ** reconstructed position vector
    */
    goodcount = 0;
    BeginStandardHeader(result->fppouterr,argc,argv,VERSION);
    fprintf(result->fppouterr,
	"%% Analysis type: \tShifted positional error distribution\n");
    fprintf(result->fppouterr,"%% Nshifts: \t%d\n",NSHIFTS);
    fprintf(result->fppouterr,"%% Mean unshifted positional error: \t%g\n",
	result->meanpositionerror);
    fprintf(result->fppouterr,"%% Fields: \t%s\t%s\t\n","shift","error");
    EndStandardHeader(result->fppouterr);
    totalerror = 0;
    for(n=0;n<NSHIFTS;n++){
	/*
	** compute a random shift 
	** dont use net zero shifts
	shiftval = frandom(1,result->ntimebins-1);
	*/
	shiftval = n - NSHIFTS/2;
	if(shiftval == 0) continue;
	positionerror = 0;
	npositionsamples = 0;
	/*
	** compute the mean positional error of the actual position
	** vector with a time shifted version of the calculated vector
	*/
	for(i=0;i<result->ntimebins;i++){
	    ishift = i+shiftval;
	    if(ishift < 0 || ishift >= result->ntimebins) continue;
	    /*
	    ishift = (i+shiftval)%result->ntimebins;
	    ** dont process invalid positions
	    */
	    if(result->position[i].x != 0 || result->position[i].y != 0){
		xerr = result->position[i].x - 
		    result->calcposition[ishift].x;
		yerr = result->position[i].y - 
		    result->calcposition[ishift].y;
		positionerror += sqrt((double)((xerr*xerr) + (yerr*yerr)));
		npositionsamples++;
	    }
	}
	/*
	** keep track of the shifts which produced total errors as good
	** or better than the unshifted version
	*/
	if((positionerror/npositionsamples) <= result->meanpositionerror){
	    goodcount++;
	}
	totalerror += positionerror/npositionsamples;
	/*
	** output the mean error value
	*/
	fprintf(result->fppouterr,"%d\t%g\n",
	shiftval,positionerror/npositionsamples);
    }
    /*
    ** output the significance level
    */
    fprintf(result->fppouterr,"%% Summary:\n");
    fprintf(result->fppouterr,"%% Ambiguous positions:\t%d\n",
	result->nambiguous);
    if(result->nambiguous > 0){
	fprintf(result->fppouterr,"%% Mean choices per ambiguity :\t%g\n",
	    (float)result->ambiguous/result->nambiguous);
    }
    fprintf(result->fppouterr,"%% Mean shifted positional error:\t%g\n",
	totalerror/NSHIFTS);
    fprintf(result->fppouterr,"%% Mean unshifted positional error:\t%g\n",
	result->meanpositionerror);
    fprintf(result->fppouterr,
    "%% Number of shifted configurations with lower error:\t%d\n",
	goodcount);
    fprintf(result->fppouterr,"%% p < %g\n",(float)(goodcount+1)/NSHIFTS);
    fclose(result->fppouterr);
}

