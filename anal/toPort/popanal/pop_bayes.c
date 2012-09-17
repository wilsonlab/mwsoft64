#include "pop_ext.h"

/*
** evaluate correlation of each timestamp population vector
** with position grid vectors
*/
ComputeBayesianReconstruction(result)
Result	*result;
{
double	totalpositionerror;
int	npositionsamples;
int	i,j;
int	x,y;
int	x2,y2;
int	invalidposition;
int	actualx,actualy;
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

    speed = 0;
    /*
    ** allocate the tie list. Make it large enough to hold the maximum
    ** number of tied reconstructed positions.
    */
    tielist = (Position *)malloc((result->yhi - result->ylo + 1)*
	(result->xhi - result->xlo + 1)*sizeof(Position));
    result->corrgrid = (float **)malloc((result->ymax+1)*sizeof(float *));
    result->smoothcorrgrid = (float **)malloc((result->ymax+1)*sizeof(float *));
    for(j=0;j<=result->ymax;j++){
	result->corrgrid[j] = (float *)calloc((result->xmax+1),sizeof(float));
	result->smoothcorrgrid[j] = (float *)calloc((result->xmax+1),sizeof(float));
    }
    for(i=0;i<result->nrecontimebins;i++){
	count = 0;
	if(verbose){
	    if(i%(result->nrecontimebins/10) == 0){
		fprintf(stderr,"%4d%%\b\b\b\b\b",100*i/result->nrecontimebins);
	    }
	}
	if(result->position && result->comparepositions){
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
	/*
	** clear the correlation grid
	*/
	for(y=0;y<=result->ymax;y++){
	    bzero(result->corrgrid[y],(result->xmax+1)*sizeof(float));
	}
	/*
	** compute the total conditional probability based on the contribution
	** of each cell
	*/
	for(j=0;j<result->nreconclusters;j++){
	    dataval = result->reconvector[i][j];
	    for(y=result->ylo;y<=result->yhi;y++){
		for(x=result->xlo;x<=result->xhi;x++){
		    /*
		    ** determine the state
		   */
		   if(dataval > result->zerolevel){
			/*
			** P(Xj|S1)
			*/
			if(result->nonzerop[j] > 0){
			    result->corrgrid[y][x] += 
			    result->positiongrid[j][y][x]/(result->nonzerop[j]);;
			}
		    } else {
#ifdef OLD
			/*
			** P(Xj|S0)
			*/
			if(result->zerop[j] > 0){
			    result->corrgrid[y][x] += 
			    result->positiongridzero[j][y][x]/(result->zerop[j]*result->nreconclusters);;
			}
#endif
		    }
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
	timestamp = result->recontstart + result->binsize*i;
	if(bestx > 0 && prevx > 0){
	    distance = sqrt((double)((bestx - prevx)*(bestx - prevx) + 
		(besty - prevy)*(besty - prevy)));
	} else {
	    distance = -1;
	}
	if(ptimebin > 0){
	    interval = i - ptimebin;
	} else {
	    interval = -1;
	}
	ptimebin = i;
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
	if(result->comparepositions){
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
	}
	/*
	** if correlation grid output is selected then dump the grid for
	** this time slice
	*/
	if(result->fpcorrout){
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
    if(npositionsamples > 0){
	result->meanpositionerror = totalpositionerror/npositionsamples;
    } else {
	result->meanpositionerror = 0;
    }
}


