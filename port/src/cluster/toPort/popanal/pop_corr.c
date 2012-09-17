/*
**************************************************
DESCRIPTION:
popanal
    performs a variety of analyses on population activity.
    Note that this is a VERY picky program when it comes to 
    specifying the source of spike information. This is
    designed to work with some very specific data sets and
    needs to be reworked for more general application

AUTHOR:
    Written by Matthew Wilson
    ARL Division of Neural Systems, Memory, and Aging
    University of Arizona
    Tucson, AZ 85724
    wilson@nsma.arizona.edu

DATES:
    original 4/92
    last update	3/93

**************************************************
*/

#include "pop_ext.h"


int CheckForOverlap(result,bin,cluster1,cluster2)
Result	*result;
int	bin;
int	cluster1;
int	cluster2;
{
int	actualx,actualy;
int	gx,gy;

    actualx = result->position[bin].x;
    actualy = result->position[bin].y;
    if(actualx == 0 && actualy == 0){
	return(0);
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
	return(0);
    } 
    /*
    ** we now have the position at this time
    ** now determine whether there is mean firing at this
    ** location which overlaps
    */
    if((result->positiongrid[cluster1][gy][gx] > 0) &&
    (result->positiongrid[cluster2][gy][gx] > 0)){
	return(1);
    } else {
	return(0);
    }
}

/*
** returns the cross correlation array for the two clusters
** can perform a shuffle correlation by randomizing the spike
** train of a cluster 
*/

float *CorrelateSpikes(result,cluster1,cluster2,corr_result)
Result	*result;
int	cluster1;
int	cluster2;
CorrResult	*corr_result;
{
int 	i,j;
double 	time1;
int	zbin;
register float	*corrarray;
register double	interval;
register int	bin;
double	fbin;
int	split;
int	maxinterval;
int	start;
int	firsttime;
unsigned long	maxtime;
unsigned long	mintime;
long	*sarray1,*sarray2;
int	nbins;
int	nspikes1;
int	nspikes2;
float	distance;
float	zerocorr;
float	norm;
float	peakcorr;
int	peakphase;
int	first_time;
int	total_count;
int	startsearch;
int	endsearch;
float	meanphase;
double	sum;
unsigned char	*overlaparray;
int	noverlap;
int	nnonoverlap;

    /*
    ** calculate the number of bins based on the binsize
    ** and the max time
    */
    nspikes1 = result->spikecount[cluster1];
    nspikes2 = result->spikecount[cluster2];
    /*
    ** if either cluster does not contain spikes then return an invalid
    ** correlation
    */
    if(nspikes1 <= 0 || nspikes2 <= 0){
	corr_result->distance = -1;
	corr_result->valid = 0;
	return(NULL);
    }
    /*
    ** if either cluster contains less than the minimum number of
    ** spikes then return an invalid correlation
    */
    if((nspikes1 < result->minspikes) || (nspikes2 < result->minspikes)){
	corr_result->distance = -1;
	corr_result->valid = 0;
	return(NULL);
    }
    /*
    ** if either cluster contains more than the maximum number of
    ** spikes then return an invalid correlation
    */
    if((result->maxspikes > 0) &&
    ((nspikes1 > result->maxspikes) || (nspikes2 > result->maxspikes))){
	corr_result->distance = -1;
	corr_result->valid = 0;
	return(NULL);
    }
    /*
    ** if either cluster contains has more than the maximum firing rate
    ** then return an invalid correlation
    */
    if( (result->maxrate > 0) && 
    (((float)nspikes1/(result->tend - result->tstart) > result->maxrate) || 
    ((float)nspikes2/(result->tend - result->tstart) > result->maxrate))){
	corr_result->distance = -1;
	corr_result->valid = 0;
	return(NULL);
    }
    /*
    ** if either cluster contains has less than the minimum firing rate
    ** then return an invalid correlation
    */
    if((((float)nspikes1/(result->tend - result->tstart) < result->minrate) || 
    ((float)nspikes2/(result->tend - result->tstart) < result->minrate))){
	corr_result->distance = -1;
	corr_result->valid = 0;
	return(NULL);
    }
    sarray1 = result->spikearray[cluster1];
    sarray2 = result->spikearray[cluster2];
    if(nspikes1 > nspikes2){
	maxtime = sarray1[nspikes1-1];
    } else {
	maxtime = sarray2[nspikes2-1];
    }
    if(sarray1[0] > sarray2[0]){
	mintime = sarray1[0];
    } else {
	mintime = sarray2[0];
    }

    if(result->tmax > 0){
	maxinterval = result->tmax;
    } else {
	maxinterval = maxtime - mintime;
    }
    nbins = 2*(int)(0.5 + (maxinterval/result->binsize)) + 1;
    if(nbins > MAXTIMEBINS){
	nbins = MAXTIMEBINS;
    }
    /*
    ** locate the zero time bin
    */
    zbin = nbins/2;

    corrarray = (float *)calloc(nbins,sizeof(float));

    /*
    ** go through each time in 1 and correlate with times in 2
    */
    start = 0;
    total_count = 0;
    if(result->spatially_restrict_corr){
	noverlap = 0;
	nnonoverlap = 0;
	overlaparray = (unsigned char *) malloc(sizeof(unsigned char)*result->ntimebins);
	for(i=0;i<result->ntimebins;i++){
	    /*
	    ** compute the time corresponding to this bin
	    */
	    if(CheckForOverlap(result,i,cluster1,cluster2)){
		overlaparray[i] = 1;
		noverlap++;
	    } else {
		overlaparray[i] = 0;
		nnonoverlap++;
	    }
	}
	/*
	fprintf(stderr,"%d %d : ov=%d, non=%d\n",cluster1,cluster2,noverlap,nnonoverlap);
	*/
    }
    for(i=0;i<nspikes1;i++){
	time1 = sarray1[i];
	/*
	** if the correlation is to be restricted to spatially overlapping
	** domains of each cell then determine the time bin
	** during which this spike occurred so that the position
	** can be determined and the firing by place grid can be
	** checked
	*/
	if(result->spatially_restrict_corr){
	    bin = (time1 - result->tstart)/result->binsize;
	    if(overlaparray[bin] == 0) continue;
	}
	firsttime = 1;
	for(j=start;j<nspikes2;j++){
	    if(result->spatially_restrict_corr){ 
		bin = (sarray2[j] - result->tstart)/result->binsize;
		if(overlaparray[bin] == 0) continue;
	    }
	    /*
	    ** calculate the interval between time1 and this time
	    */
	    interval = sarray2[j] - time1;
	    if(interval < -maxinterval){
		continue;
	    } else {
		/*
		** next time around, start at this point
		*/
		if(firsttime){
		    firsttime = 0;
		    start = j;
		}
	    }
	    if(interval > maxinterval) break; 
	    /*
	    ** which bin?
	    ** things which fall on bin boundaries get
	    ** split between them
	    */
	    fbin = interval/result->binsize;
	    split = 0;
	    if(interval >= 0){
		if(fbin - (int)fbin == (0.5 - result->zerosqueeze) && 
		(int)fbin != 0){
		    /* split it */
		    split = -1;
		}
		bin = zbin + (int)(fbin + 0.5 + result->zerosqueeze);
	    } else {
		if(fbin - (int)fbin == (-0.5 + result->zerosqueeze) &&
		(int)fbin != 0){
		    /* split it */
		    split = 1;
		}
		bin = zbin + (int)(fbin - 0.5 - result->zerosqueeze);
	    }
	    if(bin < 0 || bin >= nbins) continue;
	    /*
	    ** increment the count in that bin
	    */
	    if(split == 0){
		corrarray[bin]++;
	    } else {
		corrarray[bin] += 0.5;
		corrarray[bin+split] += 0.5;
	    }
	    total_count++;
	}
    }
    if(!result->hasspatialfiring || (result->peakloc[cluster1].x < 0 || result->peakloc[cluster2].x < 0)){
	distance = -1;
    } else {
	distance = sqrt((double)(
	(result->peakloc[cluster1].x - result->peakloc[cluster2].x)*
	(result->peakloc[cluster1].x - result->peakloc[cluster2].x) +
	(result->peakloc[cluster1].y - result->peakloc[cluster2].y)*
	(result->peakloc[cluster1].y - result->peakloc[cluster2].y)));
    }
    /*
    ** if the histogram contains less than the minimum number of
    ** events then return an invalid correlation
    */
    if(total_count < result->min_totalcorr_events){
	corr_result->distance = -1;
	corr_result->valid = 0;
	return(NULL);
    }
    /*
    ** find the peak corr within the search window
    ** note that this is biased to positive phase peaks
    */
    first_time = 1;
    if(result->peak_search_window == 0){
	startsearch = 0;
	endsearch = nbins-1;
    } else {
	startsearch = zbin - result->peak_search_window;
	endsearch = zbin + result->peak_search_window;
    }
    meanphase = 0;
    sum = 0;
    for(i=startsearch;i<zbin;i++){
	if(i < 0) continue;
	if(first_time || ((corrarray[i] > peakcorr) && 
	(corrarray[i] >= result->min_corr_events))){
	    peakcorr = corrarray[i];
	    peakphase = i;
	}
	first_time = 0;
	meanphase += (zbin - i)*corrarray[i];
	sum += corrarray[i];
    }
    for(i=endsearch;i>=zbin;i--){
	if(i >= nbins) continue;
	if((corrarray[i] > peakcorr) &&
	(corrarray[i] >= result->min_corr_events)){
	    peakcorr = corrarray[i];
	    peakphase = i;
	}
	meanphase += (i - zbin)*corrarray[i];
	sum += corrarray[i];
    }
    if(sum != 0){
	meanphase /= sum;
    } else {
	meanphase = ERRORPHS;
    }

    /*
    ** fill the correlation results structure
    */
    corr_result->nbins = nbins;
    corr_result->zerocorr = corrarray[zbin];
    corr_result->zbin = zbin;
    corr_result->peakphase = peakphase;
    corr_result->peakcorr = peakcorr;
    corr_result->meanphase = meanphase;
    corr_result->distance = distance;
    corr_result->totalcount = total_count;
    corr_result->valid = 1;
    if(result->spatially_restrict_corr){
	free(overlaparray);
    }
    /*
    ** clear the correlation array
    if(corrarray){
	free(corrarray);
    }
    */
    return(corrarray);
}

float ComputeSpatialOverlap(result,cluster1,cluster2)
Result	*result;
int	cluster1;
int	cluster2;
{
float		**grid1;
float		**grid2;
double		overlap;
float		halfmax1;
float		halfmax2;
int		x,y;
int	nspikes1;
int	nspikes2;

    /*
    ** compute the overlap in fields if spatial firing
    ** info is available
    */
    overlap=0;
    if(result->hasspatialfiring){
	if((grid1 = result->positiongrid[cluster1]) == NULL){
	    return(-1.0);
	}
	halfmax1 = result->peak[cluster1]*result->peakfrac;
	if((grid2 = result->positiongrid[cluster2]) == NULL){
	    return(-1.0);
	}
	halfmax2 = result->peak[cluster2]*result->peakfrac;
	/*
	** compute overlap for clusters with valid fields
	*/
	if((result->peakloc[cluster1].x < 0) ||
	(result->peakloc[cluster2].x < 0)){
	    /*
	    ** one of the clusters does not have a valid field
	    ** therefore set an invalid overlap
	    */
	    return(-1.0);
	} else
	for(x=result->gxlo;x<result->gxhi;x++){
	    for(y=result->gylo;y<result->gyhi;y++){
		/*
		** count locations in which both fields are above the
		** halfmax cutoff
		overlap += grid1[y][x]*grid2[y][x];
		*/
		overlap += (grid1[y][x] > halfmax1)*(grid2[y][x] > halfmax2);
		/*
		if((grid1[y][x] > halfmax1) && (grid2[y][x] > halfmax2)){
		    overlap += grid1[y][x] + grid2[y][x];
		}
		*/
	    }
	}
    }
    /*
    ** normalize the overlap to the expected value
    overlap /= result->ntimebins;
    */
    /*
    ** normalize to the mean rate of the clusters
    nspikes1 = result->spikecount[cluster1];
    nspikes2 = result->spikecount[cluster2];
    overlap /= (float)(nspikes1 + nspikes2)/result->ntimebins;
    */
    return((float)overlap);
}

ComputeExpectedCorrelation(result,cluster1,cluster2,expected,sd,meanrate,minrate,maxrate)
Result	*result;
int	cluster1;
int	cluster2;
double	*expected;
double	*sd;
double	*meanrate;
double	*minrate;
double	*maxrate;
{
int	bin;
int	nspikes1;
int	nspikes2;
int	*binarray1;
int	*binarray2;
double	sum1;
double	sum2;
double	sum1sqr;
double	sum2sqr;
int	noverlap;
double	var1;
double	var2;
int	i;
unsigned long	*sarray1,*sarray2;

    nspikes1 = result->spikecount[cluster1];
    nspikes2 = result->spikecount[cluster2];
    /*
    ** compute the expected value of the correlation
    ** for normalization
    ** see  Palm et al. 1988 Biol. Cybernetics 59,1-11
    ** for more detail
    */
    if(result->spatially_restrict_corr){
	sarray1 = result->spikearray[cluster1];
	sarray2 = result->spikearray[cluster2];
	/*
	** if we are going to restrict the correlation to spatially overlapping
	** regions then identify those domains now
	** and compute the first order statistics of the spike trains during
	** these periods
	*/
	binarray1 = (int *)calloc(result->ntimebins,sizeof(int));
	binarray2 = (int *)calloc(result->ntimebins,sizeof(int));
	/*
	** bin all the spikes for each train
	*/
	for(i=0;i<nspikes1;i++){
	    bin = (sarray1[i] - result->tstart)/result->binsize;
	    binarray1[bin]++;
	}
	for(i=0;i<nspikes2;i++){
	    bin = (sarray2[i] - result->tstart)/result->binsize;
	    binarray2[bin]++;
	}
	/*
	** compute the mean and variance over the overlapping intervals only
	*/
	sum1 = 0;
	sum2 = 0;
	sum1sqr = 0;
	sum2sqr = 0;
	noverlap=0;
	for(i=0;i<result->ntimebins;i++){
	    /*
	    ** compute the time corresponding to this bin
	    */
	    if(CheckForOverlap(result,i,cluster1,cluster2)){
		if(binarray1[i] > 0){
		    sum1 += binarray1[i];
		    sum1sqr += binarray1[i]*binarray1[i];
		}
		if(binarray2[i] > 0){
		    sum2 += binarray2[i];
		    sum2sqr += binarray2[i]*binarray2[i];
		}
		noverlap++;
	    } 
	}
	if(noverlap > 1){
	    /*
	    ** note the multiplication of sd and expected by N.  This 
	    ** is due to the use of the cross correlogram which is
	    ** just SUM(xy) rather than (1/N)*SUM(xy)
	    ** Expected then becomes the expected zero bin value of
	    ** the cross correlogram and sd is N*sdx*sdy
	    */
	    *expected = sum1*sum2/noverlap;
	    var1 = (sum1sqr*(double)noverlap - sum1*sum1);
	    var2 = (sum2sqr*(double)noverlap - sum2*sum2);
	    *sd = noverlap*sqrt(var1*var2)/((double)noverlap*(noverlap-1));
	    *meanrate = (float)(sum1 + sum2)/(noverlap*result->binsize*1e-4);
	    if(nspikes1 > nspikes2){
		*maxrate = (float)(sum1)/(result->ntimebins*result->binsize*1e-4);
	    } else {
		*minrate = (float)(sum2)/(result->ntimebins*result->binsize*1e-4);
	    }
	} else {
	    *expected = 0;
	    *sd = 0;
	    *meanrate = 0;
	}
	free(binarray1);
	free(binarray2);
    } else {
	*expected = (float)nspikes1*nspikes2/result->ntimebins;
	*sd = result->ntimebins*result->binsd[cluster1]*
	    result->binsd[cluster2];
	*meanrate = (float)(nspikes1 + nspikes2)/(result->ntimebins*result->binsize*1e-4);
	if(nspikes1 > nspikes2){
	    *maxrate = (float)(nspikes1)/(result->ntimebins*result->binsize*1e-4);
	    *minrate = (float)(nspikes2)/(result->ntimebins*result->binsize*1e-4);
	} else {
	    *maxrate = (float)(nspikes2)/(result->ntimebins*result->binsize*1e-4);
	    *minrate = (float)(nspikes1)/(result->ntimebins*result->binsize*1e-4);
	}
    }
    /*
    ** modification to examine the effect of normalization
    *expected = 0;
    */
}

int FindPeak(result,corr_result,corrarray,expected_corr,sd_corr,bestphase,bestcorr,bestindex,bestcv)
Result	*result;
CorrResult	*corr_result;
float		*corrarray;
double		expected_corr;
double		sd_corr;
int	*bestphase;
float	*bestcorr;
float	*bestindex;
float	*bestcv;
{
float		bestzscore;
float		z_n;
float		z_alpha;
int		firsttime;
float		critical_value;
float		peakcount;
float		meancount;
float		peakwidth;
float		peakindex;
float		zscore;
float		total_peak;
int		zbin;
int		center;
int		count,i;
float		sd;
float		mean;
float		peakcorr;

    /*
    ** check for a non-empty correlation array
    */
    if(corr_result->totalcount == 0){
	*bestphase = 0;
	*bestcorr = 0;
	*bestindex = 0;
	*bestcv = 0;
	return(0);
    }
    bestzscore = 0;
    /*
    ** get the significance z value for the given number of bins
    */
    z_n = (log10((double)corr_result->nbins) - 1)*0.78 + 1.8;
    /*
    ** get significance cutoffs for 1-tailed test
    */
    z_alpha = -(log10((double)result->peakcutoff)+1)*0.83 + 1.4;
    /*
    ** now assess the significance of the peak and search for the
    ** maximum significance
    */
    firsttime = 1;
#ifdef OLD
    for(center = corr_result->zbin - result->peak_search_window;
    center<corr_result->zbin + result->peak_search_window;center++){
	if(center <0 || center >= corr_result->nbins) continue;
#else
	center = corr_result->peakphase;
#endif
	peakwidth = 0;
	count = 0;
	meancount = 0;
	/*
	** compute the mean correlation outside of the peak window
	*/
	for(i=0;i<corr_result->nbins;i++){
	    /*
	    ** skip the peak window
	    */
	    if((i >= center - result->peakwidth) && 
	    (i <= center + result->peakwidth)){
		peakwidth++;
		continue;
	    }
	    meancount += corrarray[i];
	    count++;
	}
	mean = meancount/count;
	/*
	** compute the peak correlation index
	** see Wiegner and Wierzbicka 1987, J. Neurosci. Methods,
	** 22,125-131
	*/
	peakcount = 0;
	count = 0;
	peakcorr = 0;
	for(i=center - result->peakwidth;i<center + 
	result->peakwidth;i++){
	    if(i < 0 || i >= corr_result->nbins){
		continue;
	    }
	    peakcount += corrarray[i] - mean;
	    peakcorr += corrarray[i];
	    count++;
	}
	peakindex = peakcount/corr_result->totalcount;
	peakcorr /= count;
	/*
	** compute the critical significance value for the index
	*/
	sd = sqrt((double)(peakwidth/((corr_result->nbins - 
	    peakwidth)*corr_result->totalcount))
	);
	/*
	** note that the z_n term can be dropped if the window is
	** taken around a fixed center i.e. no phase search window
	*/
	critical_value = z_n/sqrt((double)(corr_result->nbins*
	corr_result->totalcount)) + z_alpha*sd;

	/*
	** compute standard value for the index
	*/
	zscore = peakindex/sd;
	if(firsttime || (zscore > bestzscore)){
	    bestzscore = zscore;
	    *bestphase = center;
	    *bestcorr = (corrarray[center] - expected_corr)/
		sd_corr;
	    *bestindex = peakindex;
	    *bestcv = critical_value;
	}
	firsttime = 0;
#ifdef OLD
    }
#endif
    return(1);
    
}

CorrResult	*ComputeCorrelation(result,cluster1,cluster2,fpout,output)
Result	*result;
int	cluster1;
int	cluster2;
FILE	*fpout;
int	output;
{
float blah;
CorrResult	shuffled_corr_result;
static CorrResult	corr_result;
int		i;
float		distance;
int		peakphs;
int		center;
float		*corrarray;
int		count;
float		bestcorr;
int		bestphase;
float		bestindex;
float		bestcv;
double		expected_corr;
double		sd_corr;
double		meanrate;
double		minrate;
double		maxrate;
float		overlap;
int		fcolor;
float		cx1,cy1,cx2,cy2;
double		factor;
float		corr;
double		pos,neg;
int		flip,index;
int		bias;
int		negcount;
int		poscount;
double		mag;
float		skew;
float		corrval;
float		skew_sum;
float		skew_offset;
float		centerofmass;


    /*
    ** prepare the results data structure
    ** note that a pointer to corr_result is returned
    */
    bzero(&corr_result,sizeof(CorrResult));
    bzero(&shuffled_corr_result,sizeof(CorrResult));
    bestindex = 0;
    bestcv = 0;

    /*
    ** compute the amount of spatial overlap in the spatial firing
    ** patterns for the two clusters
    */
    overlap = ComputeSpatialOverlap(result,cluster1,cluster2);

    /*
    ** compute the expected correlation
    */
    corr_result.distance = -1;
    ComputeExpectedCorrelation(result,cluster1,cluster2,
	&expected_corr,&sd_corr,&meanrate,&minrate,&maxrate);
    corr_result.expected_corr = expected_corr;
    corr_result.sd_corr = sd_corr;
    corr_result.meanrate = meanrate;
    corr_result.minrate = minrate;
    corr_result.maxrate = maxrate;

    /*
    ** if an error condition occurred during calculation of the
    ** correlation normalization then return an invalid result
    */
    if(sd_corr <= 0){
	/*
	** a valid correlation array was not computed. 
	*/
	if(fpout && result->showallcorrs && output){
	    fprintf(fpout,"%d\t%d\t%g\t%g\t%g\t%d\t%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
	    cluster1,
	    cluster2,
	    corr_result.distance,
	    0.0,
	    0.0,
	    ERRORPHS,
	    ERRORPHS,
	    overlap,
	    0.0,
	    meanrate,
	    minrate,
	    maxrate,
	    0.0,
	    0.0,
	    0.0);
	}
	if(result->fpcorrhistout && result->showallcorrs && output){
	    fprintf(result->fpcorrhistout,"%d\t%d\t%g\t%g\t%g\t%d\t%d\t%d",
	    cluster1,cluster2,corr_result.distance,overlap,meanrate,0,0,0);
	    if(result->hasparameters){
		fprintf(result->fpcorrhistout,"\t%g\t%g",
		clusterdir[cluster1].parameter,
		clusterdir[cluster2].parameter);
	    }
	    fprintf(result->fpcorrhistout,"\n");
	}
	corr_result.valid = 0;
	return(&corr_result);
    }

    /*
    ** compute the actual correlation between the clusters
    */
    if((corrarray = 
    CorrelateSpikes(result,cluster1,cluster2, &corr_result)) == NULL){
	/*
	** a valid correlation array was not computed. One or more of
	** the conditions placed on validity of the clusters was
	** not met
	*/
	if(fpout && result->showallcorrs && output){
	    fprintf(fpout,"%d\t%d\t%g\t%g\t%g\t%d\t%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
	    cluster1,
	    cluster2,
	    corr_result.distance,
	    0.0,
	    0.0,
	    ERRORPHS,
	    ERRORPHS,
	    overlap,
	    expected_corr/sd_corr,
	    meanrate,
	    minrate,
	    maxrate,
	    expected_corr,
	    sd_corr,
	    0.0);
	}
	if(result->fpcorrhistout && result->showallcorrs && output){
	    fprintf(result->fpcorrhistout,"%d\t%d\t%g\t%g\t%g\t%d\t%d\t%d",
	    cluster1,cluster2,corr_result.distance,overlap,meanrate,0,0,0);
	    if(result->hasparameters){
		fprintf(result->fpcorrhistout,"\t%g\t%g",
		clusterdir[cluster1].parameter,
		clusterdir[cluster2].parameter);
	    }
	    fprintf(result->fpcorrhistout,"\n");
	}
	return(&corr_result);
    }
    /*
    ** if within or between probe options are selected then compare
    ** the cluster/probe names
    ** but dont skip autocorrs
    */
    if(result->betweenprobe && (cluster1 != cluster2)){
	/*
	** if the probe names are the same then
	** skip it
	*/
	if(strcmp(result->clustername[cluster1],
	    result->clustername[cluster2]) == 0){
	    /*
	    ** a valid correlation array was not computed. 
	    */
	    if(fpout && result->showallcorrs && output){
		fprintf(fpout,"%d\t%d\t%g\t%g\t%g\t%d\t%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
		cluster1,
		cluster2,
		corr_result.distance,
		0.0,
		0.0,
		ERRORPHS,
		ERRORPHS,
		overlap,
		0.0,
		meanrate,
		minrate,
		maxrate,
		0.0,
		0.0,
		0.0);
	    }
	    if(result->fpcorrhistout && result->showallcorrs && output){
		fprintf(result->fpcorrhistout,"%d\t%d\t%g\t%g\t%g\t%d\t%d\t%d",
		cluster1,cluster2,corr_result.distance,overlap,meanrate,0,0,0);
		if(result->hasparameters){
		    fprintf(result->fpcorrhistout,"\t%g\t%g",
		    clusterdir[cluster1].parameter,
		    clusterdir[cluster2].parameter);
		}
		fprintf(result->fpcorrhistout,"\n");
	    }
	    corr_result.valid = 0;
	    return(&corr_result);
	}
    }
    if(result->withinprobe){
	/*
	** if the probe names are different then
	** skip it
	*/
	if(strcmp(result->clustername[cluster1],
	    result->clustername[cluster2]) != 0){
	    /*
	    ** a valid correlation array was not computed. 
	    */
	    if(fpout && result->showallcorrs && output){
		fprintf(fpout,"%d\t%d\t%g\t%g\t%g\t%d\t%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
		cluster1,
		cluster2,
		corr_result.distance,
		0.0,
		0.0,
		ERRORPHS,
		ERRORPHS,
		overlap,
		0.0,
		meanrate,
		minrate,
		maxrate,
		0.0,
		0.0,
		0.0);
	    }
	    if(result->fpcorrhistout && result->showallcorrs && output){
		fprintf(result->fpcorrhistout,"%d\t%d\t%g\t%g\t%g\t%d\t%d\t%d",
		cluster1,cluster2,corr_result.distance,overlap,meanrate,0,0,0);
		if(result->hasparameters){
		    fprintf(result->fpcorrhistout,"\t%g\t%g",
		    clusterdir[cluster1].parameter,
		    clusterdir[cluster2].parameter);
		}
		fprintf(result->fpcorrhistout,"\n");
	    }
	    corr_result.valid = 0;
	    return(&corr_result);
	}
    }
    if(result->peakcutoff > 0){
	FindPeak(result,&corr_result,corrarray,expected_corr,sd_corr,
	&bestphase,&bestcorr,
	&bestindex,&bestcv);
	fprintf(stderr,"findpeak\n");
    } else {
	center = corr_result.peakphase;
	bestphase = center;
	bestcorr = (corrarray[center] - expected_corr)/
	    sd_corr;
	    if(bestphase > corr_result.nbins){
	    fprintf(stderr,"peak %d\n",bestphase);
	}
    }
    /*
    ** output the results for significant corrs
    */
    if((result->peakcutoff <= 0) || (bestindex > bestcv) ||
    result->showallcorrs){
	if(fpout && output){
	    fprintf(fpout,"%d\t%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
	    cluster1,
	    cluster2,
	    corr_result.distance,
	    (corr_result.zerocorr - expected_corr)/sd_corr,
	    bestcorr,
	    (bestphase - corr_result.zbin)*result->binsize/10.0,
	    corr_result.meanphase*result->binsize/10.0,
	    overlap,
	    expected_corr/sd_corr,
	    meanrate,
	    minrate,
	    maxrate,
	    expected_corr,
	    sd_corr,
	    corr_result.zerocorr);

	}
	/*
	** output to the correlation histogram file
	*/
	if(result->fpcorrhistout && output){
	    /*
	    ** if sign bias is selected then flip the direction of
	    ** the correlation depending on the weight of each
	    ** side of the distribution - place the heavy side on
	    ** the positive
	    ** ignore the zero bin
	    */
	    pos = neg = 0;
	    poscount = negcount = 0;
	    skew_offset = 0;
	    skew_sum = 0;
	    for(i=1;i<corr_result.zbin;i++){
		if(result->skew_window){
		    /*
		    ** skip portions of the histogram between zero
		    ** and the edge of the skew window
		    */
		    if( corr_result.zbin - i < result->skew_window)
			break;
		}
		/*
		** NOTE: the expected correlation must be corrected for
		** edge effects. The sd should also be corrected but
		** I will get to that later.
		*/
		factor = (result->ntimebins - (corr_result.zbin - i))/
		(double)result->ntimebins;
		if(result->skewnormalize){
		    corrval = (corrarray[i] - expected_corr*factor)/sd_corr;
		} else {
		    corrval = corrarray[i];
		}
		/*
		neg += corrval*(corr_result.zbin -i);
		*/
		neg += corrval;
		/*
		** look for positive peaks by  only 
		** looking at rectified correlation
		*/
		if(corrval > 0){
		    skew_offset += (i-corr_result.zbin)*corrval;
		    skew_sum += corrval;
		}
		negcount++;
	    }
	    for(i=corr_result.zbin+1;i<corr_result.nbins-1;i++){
		if(result->skew_window){
		    /*
		    ** skip portions of the histogram between zero
		    ** and the edge of the skew window
		    */
		    if( i - corr_result.zbin < result->skew_window)
			continue;
		}
		/*
		** NOTE: the expected correlation must be corrected for
		** edge effects. The sd should also be corrected but
		** I will get to that later.
		*/
		factor = (result->ntimebins - (i - corr_result.zbin))/
		(double)result->ntimebins;
		if(result->skewnormalize){
		    corrval = (corrarray[i] - expected_corr*factor)/sd_corr;
		} else {
		    corrval = corrarray[i];
		}
		/*
		pos += corrval*(i-corr_result.zbin);
		*/
		pos += corrval;
		/*
		** look for positive peaks by  only 
		** looking at rectified correlation
		*/
		if(corrval > 0){
		    skew_offset += (i-corr_result.zbin)*corrval;
		    skew_sum += corrval;
		}
		poscount++;
	    }
	    if(poscount != negcount){
		fprintf(stderr,
		"WARNING: unequal pos and neg skew contribution\n");
	    }
	    if(neg > pos){
		bias = -1;
	    } else {
		bias = 1;
	    }
	    if(neg == pos){
		bias = 0;
	    }
	    if(result->corrhistsignbias){
		/*
		** if the negative half of the histogram is heavier
		** then flip it
		*/
		if(bias < 0)
		    flip = 1;
		else
		    flip = 0;
	    } else {
		flip = 0;
	    }
	    mag = pos*pos + neg*neg;
	    if(mag == 0){
		skew = 0;
	    } else {
	    /*
		skew = (pos-neg)/sqrt(mag);
		*/
		if(result->skewcount){
		    skew = pos-neg;
		} else {
		    skew = (pos-neg)/(pos+neg);
		}
		/*
		if(fabs(pos) + fabs(neg) == 0){
		    skew = 0;
		} else {
		    skew = pos-neg/(fabs(pos) + fabs(neg));;
		}
		*/
	    }
	    if(skew_sum > 0){
		centerofmass = skew_offset/skew_sum;
	    } else {
		centerofmass = 0;
	    }
	    fprintf(result->fpcorrhistout,"%d\t%d\t%g\t%g\t%g\t%d\t%g\t%g\t",
	    cluster1,cluster2,corr_result.distance,overlap,meanrate,
	    bias,skew,centerofmass);
	    if(result->hasparameters){
		fprintf(result->fpcorrhistout,"%g\t%g\t",
		clusterdir[cluster1].parameter,
		clusterdir[cluster2].parameter);
	    }
	    /*
	    ** output all but the end bins which dont have valid
	    ** contents due to bin roundoff
	    */
	    for(i=1;i<corr_result.nbins-1;i++){
		if(flip){
		    index = corr_result.nbins-1 - i;
		} else {
		    index = i;
		}
		/*
		** NOTE: the expected correlation must be corrected for
		** edge effects. The sd should also be corrected but
		** I will get to that later.
		*/
		if(result->rawhist){
		    fprintf(result->fpcorrhistout,"%g\t", corrarray[index]);
		} else {
		    factor = (result->ntimebins - abs(index - corr_result.zbin))/
		    (double)result->ntimebins;
		    fprintf(result->fpcorrhistout,"%g\t",
		    (corrarray[index] - expected_corr*factor)/sd_corr);
		}
	    }
	    fprintf(result->fpcorrhistout,"\n");
	}
	/*
	** output to the correlation plot file
	*/
	if(result->fpcorrplotout && output){
	    corr = (corr_result.zerocorr - expected_corr)/sd_corr;
	    fcolor = (int)(215 + 500*corr);
	    if(fcolor > 250) {
		fcolor = 250;
	    }
	    /*
	    ** compute the location of the cell in the plot display
	    */
	    cx1 = cos((double)cluster1*M_PI*2/result->nclusters);
	    cy1 = sin((double)cluster1*M_PI*2/result->nclusters);
	    cx2 = cos((double)cluster2*M_PI*2/result->nclusters);
	    cy2 = sin((double)cluster2*M_PI*2/result->nclusters);
#ifdef OLD
	    fprintf(result->fpcorrplotout,"%d %d\n%d %d\n",
		result->peakloc[cluster1].x,
		result->peakloc[cluster1].y,
		result->peakloc[cluster2].x,
		result->peakloc[cluster2].y);
	    if(fcolor > 215)
	    fprintf(result->fpcorrplotout,
		"/setcolor %d\n/plotwline %g %g %g %g %g %g\n",
		fcolor,
		(float)result->peakloc[cluster1].x,
		(float)result->peakloc[cluster1].y,
		meanrate,
		(float)result->peakloc[cluster2].x,
		(float)result->peakloc[cluster2].y,
		meanrate);
#else
	    /*
	    ** only output positive correlations
	    */
	    if(fcolor > 215){
	    /*
		fprintf(result->fpcorrplotout,"%g %g\n%g %g\n",
		cx1,cy1,cx2,cy2);
		fprintf(result->fpcorrplotout,
		    "/setcolor %d\n/plotwline %g %g %g %g %g %g\n",
		    fcolor,
		    cx1,
		    cy1,
		    meanrate,
		    cx2,cy2,
		    meanrate);
		*/
		fprintf(result->fpcorrplotout, "/newplot\n");
#ifndef OLD
		fprintf(result->fpcorrplotout,"%g %g %g %d\n%g %g %g %d\n",
		cx1,cy1,corr,cluster1,cx2,cy2,corr,cluster2);
		fprintf(result->fpcorrplotout, "/color %d\n",
		    fcolor);
#else
		fprintf(result->fpcorrplotout,"%d %d %g %d\n%d %d %g %d\n",
		result->peakloc[cluster1].x,
		result->peakloc[cluster1].y,
		corr,cluster1,
		result->peakloc[cluster2].x,
		result->peakloc[cluster2].y,
		corr,cluster2);
		fprintf(result->fpcorrplotout, "/color %d\n",
		    fcolor);
#endif
	    }
#endif

	}
    }
    free(corrarray);
    return(&corr_result);
}

int corrcompare(i1,i2)
CorrList	*i1,*i2;
{
    return((int)(i1->corrsum - i2->corrsum > 0));
}

SortCorrelations(result)
Result	*result;
{
int	i,j;

    /*
    ** prepare the correlation list
    */
    result->corrlist = (CorrList *)calloc(sizeof(CorrList),result->nclusters);
    /*
    ** do the upper half
    */
    for(i=0;i<result->nclusters;i++){
	result->corrlist[i].index = i;
	for(j=i+1;j<result->nclusters;j++){
	    result->corrlist[i].corrsum += result->corrmatrix[i][j];
	}
    }
    /*
    ** do the lower half
    */
    for(i=0;i<result->nclusters;i++){
	for(j=0;j<i;j++){
	    result->corrlist[i].corrsum += result->corrmatrix[j][i];
	}
    }
    /*
    ** sort the list of indices
    */
    qsort(result->corrlist,result->nclusters,sizeof(CorrList),corrcompare);
}


