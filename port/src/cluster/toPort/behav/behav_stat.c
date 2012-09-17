#include "behav_ext.h"

double ChiSquare(result,testmean,dchi,nchi)
Result	*result;
double	testmean;
double	*dchi;
int	*nchi;
{
int	i;
int	j;
int	n;
double	chi;
double	sum;
double	sumsqr;
double	mean;
double	var;
int	nmin;

    chi = 0;
    *dchi = 0;
    nmin = CHI_MINOCCUPANCY;
    *nchi = 0;
    for(i=0;i<result->xsize;i++){
	for(j=0;j<result->xsize;j++){
	    n = result->occupancy_gridn[i][j];
	    /*
	    ** only take points which have enough samples
	    */
	    if(n > nmin){
		sum = result->occupancy_grid[i][j];
		sumsqr = result->occupancy_gridsqr[i][j];
		mean = sum/n;
		var = (sumsqr - sum*sum/n)/(n-1);
		if(var < 0){
		    fprintf(stderr,"ERROR: invalid variance %g\n",var);
		}
		if(var < TINYVAR){
		    /*
		    chi = HUGECHI;
		    *dchi = -HUGECHI;
		    */
		} else {
		    (*nchi)++;
		    chi += (mean - testmean)*(mean - testmean)/var;
		    *dchi += -2*(mean - testmean)/var;
		}
	    }
	}
    }
    return(chi);
}

/* commented out by Tom D. 11/18/04--never called; empty */
/* void TestGridSignificance(result,x1,y1,x2,y2) */
/* Result	*result; */
/* int	x1,y1;		/\* grid locations *\/ */
/* int	x2,y2; */
/* { */
/* } */

void EvaluateSpatialFiring(result,spikearray,cell)
Result	*result;
SpikeList	*spikearray;
int	cell;
{
int	nspikes;
int	nsamples;
float	rate;
float	rate2;
int	i,j;
float	xscale,yscale;
int	nocc;
float	meanrate;
float	info;
float	meanx,meany;
float	fieldvarx,fieldvary;
float	absfieldvarx,absfieldvary;
float	peakfieldvarx,peakfieldvary;
double	sumx,sumy;
double	abssumx,abssumy;
double	peaksumx,peaksumy;
float	fieldsize;
float	absfieldsize;
float	peakfieldsize;
double	meanxsqr,meanysqr;
double	sumxsqr,sumysqr;
double	abssumxsqr,abssumysqr;
double	peaksumxsqr,peaksumysqr;
double  summeanrate, peakmeanrate, absmeanrate;
float	dmean;
int	dcount;
int	npsamples;
int	half;
int	index;
int	index2;
float	dimax = 0;
int	didir = INT_MIN;
float	dsum = 0;
float	spikemean;
float	peakrate;
float	peakthresh;
int	peakx,peaky;
double	weightedx, weightedy,totalrate,totalnsamples;

    /*
    ** scale factor
    */
    xscale = result->xres/result->xsize;
    yscale = result->yres/result->ysize;
    /*
    ** compute overall mean rate
    */
    info = 0;
    meanrate = 0;
    peakrate  = 0;
    nocc = 0;
    peakx = 0;
    peaky = 0;
    for(i=0;i<result->xsize;i++){
	for(j=0;j<result->ysize;j++){
	    /*
	    ** changed 12/20/00 to reflect minpsamples.
	    if(result->gridtotaln[i][j] > 0){
	    **
	    */
	    if(result->gridtotaln[i][j] > result->minpsamples){
		meanrate += (float)result->grid[i][j];
		nocc += result->gridtotaln[i][j];
		nspikes = result->grid[i][j];
		nsamples = result->gridtotaln[i][j];
		if(nsamples > 0){
		    rate = (float)nspikes/(result->pinterval*nsamples);
		    if(rate > peakrate){
			peakrate = rate;
			peakx = i;
			peaky = j;
		    }
		}
	    }
	}
    }
    if(nocc > 0){
	meanrate /= result->pinterval*nocc;
    }
    /*
    ** compute the mean spike rate by counting the number of spikes
    ** that fall between the start and end times
    */
    /*
    ** 2.28 modified mean rate in the status output to reflect total spikes
    ** between start and end time only
    */
    nspikes = 0;
    for(i=0;i<spikearray[cell].nspikes;i++){
    	if((spikearray[cell].data[i].timestamp >= starttime) &&
    	(spikearray[cell].data[i].timestamp <= endtime)){
	    nspikes++;
	}
    }
    spikemean = 1e4*nspikes/(endtime -starttime);
    peakthresh = 0.1*peakrate;
    meanx = 0;
    meany = 0;
    sumx = 0;
    sumy = 0;
    sumxsqr = 0;
    sumysqr = 0;
    abssumx = 0;
    abssumy = 0;
    abssumxsqr = 0;
    abssumysqr = 0;
    peaksumx = 0;
    peaksumy = 0;
    peaksumxsqr = 0;
    peaksumysqr = 0;
    meanxsqr = 0;
    meanysqr = 0;
    fieldvarx = 0;
    fieldvary = 0;
    absfieldvarx = 0;
    absfieldvary = 0;
    peakfieldvarx = 0;
    peakfieldvary = 0;
    fieldsize=0;
    absfieldsize=0;
    peakfieldsize=0;
    npsamples = 0;
    weightedx = 0;
    weightedy = 0;
    totalrate = 0;
    totalnsamples = 0;
    summeanrate = 0;
    absmeanrate = 0;
    peakmeanrate = 0;
    for(i=0;i<result->xsize;i++){
	for(j=0;j<result->ysize;j++){
	    nspikes = result->grid[i][j];
	    nsamples = result->gridtotaln[i][j];
	    /*
	    ** changed 12/20/00 to reflect minpsamples.
	    if(nsamples > 0){
	    */
	    if(nsamples > result->minpsamples){
		rate = (float)nspikes/(result->pinterval*nsamples);
		npsamples++;
		totalnsamples += nsamples;
	    } else {
		rate = 0;
	    }
	    weightedx += i*rate;
	    weightedy += j*rate;
	    totalrate += rate;
	    /*
	    ** now evaluate against adjacent points
	    */
	    /*
	    ** number of points above threshold
	    */
	    if(rate >= peakthresh){
		peakfieldsize++;
		peaksumx += i;
		peaksumy += j;
		peaksumxsqr += i*i;
		peaksumysqr += j*j;
		peakmeanrate += rate;
	    }
	    if(rate >= result->fieldthresh){
		absfieldsize++;
		abssumx += i;
		abssumy += j;
		abssumxsqr += i*i;
		abssumysqr += j*j;
		absmeanrate += rate;
	    }
	    if(rate >= 0.5*spikemean){
		fieldsize++;
		/*
		** field size and variance
		*/
		sumx += i;
		sumy += j;
		sumxsqr += i*i;
		sumysqr += j*j;
		summeanrate += rate;
		result->field_estimate[i][j] = 1;
	    }
	    /*
	    ** mean distance between suprathreshold points
	    */
	    /*
	    ** rate vs distance
	    */
	    /*
	    ** spatial coherence of points
	    */
	    /*
	    ** dispersion
	    */
	    /*
	    ** variance of rate vs distance
	    */
	    /*
	    ** spatial information
	    ** info = {p(s)/n}*log{p(s)/p(mean)}
	    */
	    if(rate > 0){
	    	/*
		info += rate*log(rate/meanrate);
		*/
		info += nsamples*rate*log(rate/meanrate);
	    }
	}
    }
    /*
    info /= npsamples*log(2.0);
    */
    info /= totalnsamples;
    if(totalrate > 0){
	weightedx /= totalrate;
	weightedy /= totalrate;
    }
    if(fieldsize > 0){
	meanx = sumx/fieldsize;
	meany = sumy/fieldsize;
	meanxsqr = sumxsqr/(fieldsize*fieldsize);
	meanysqr = sumxsqr/(fieldsize*fieldsize);
	fieldvarx = sqrt((fieldsize*sumxsqr - sumx*sumx)/(fieldsize*(fieldsize-1)));
	fieldvary = sqrt((fieldsize*sumysqr - sumy*sumy)/(fieldsize*(fieldsize-1)));
    }
    if(absfieldsize > 0){
	absfieldvarx = sqrt((absfieldsize*abssumxsqr - abssumx*abssumx)/
	    (absfieldsize*(absfieldsize-1)));
	absfieldvary = sqrt((absfieldsize*abssumysqr - abssumy*abssumy)/
	    (absfieldsize*(absfieldsize-1)));
    }
    if(peakfieldsize > 0){
	peakfieldvarx = sqrt((peakfieldsize*peaksumxsqr - peaksumx*peaksumx)/
	    (peakfieldsize*(peakfieldsize-1)));
	peakfieldvary = sqrt((peakfieldsize*peaksumysqr - peaksumy*peaksumy)/
	    (peakfieldsize*(peakfieldsize-1)));
    }
    if((result->behavmode == DIRECTION) && (result->savemode == VECTOR)){
	/*
	** find the axis of maximal directionality
	*/
	dimax = 0;
	didir = -1;
	for(j=0;j<result->dsize;j++){
	    /*
	    ** compute a directionality index based on mean difference
	    ** in opposing directions
	    ** only evaluate bins in which firing in either direction exceeds
	    ** the mean
	    */
	    dmean = 0;
	    dcount=0;
	    dsum = 0;
	    half = result->dsize/2;
	    for(i=0;i<half;i++){
		index = (i+j)%result->dsize;
		index2 = (i+half+j)%result->dsize;
		if((result->vectortotaln[index] > 0) && 
		    (result->vectortotaln[index2] > 0)){
		    rate = result->vector[index]/
			(result->vectortotaln[index]*result->pinterval);
		    rate2 = result->vector[index2]/
			(result->vectortotaln[index2]*result->pinterval);
		    if((rate >= 0.0*meanrate) || (rate2 >= 0.0*meanrate)){
			dmean += rate-rate2;
			dsum += rate + rate2;
			dcount++;
		    }
		}
	    }
	    if(dmean > dimax){
		dimax = dmean;
		didir = j*360.0/result->dsize;
	    }
	}
	if(dsum > 0){
	    dimax /= dsum;
	}
    } else {
	if(verbose)
	fprintf(stderr,"no directional analysis\n");
    }
    fprintf(result->fpreport,"%% \t%d,%d\t\t :Behavioral Spike Analysis Dimensions (x by y)\n",result->xsize,result->ysize);
    fprintf(result->fpreport,"\t%s\t\t :Cell\n",spikearray[cell].fname);
    fprintf(result->fpreport,"\t%g\t\t :Mean Field Center X (gridpix)\n",meanx);
    fprintf(result->fpreport,"\t%g\t\t :Mean Field Center y (gridpix)\n",meany);
    fprintf(result->fpreport,"\t%-7.4g\t\t :Mean Firing Rate (spikes/sec)\n",spikemean);
    fprintf(result->fpreport,"\t%-7.4g\t\t :Mean XY Firing Rate (spikes/sec)\n",meanrate);
    fprintf(result->fpreport,"\t%g\t\t :%5.3g Fixed Threshold Field size (gridpix)\n",absfieldsize,result->fieldthresh);
    fprintf(result->fpreport,"\t%g\t\t :%5.3g Fixed Threshold Mean Infield Rate (spikes/sec)\n",absmeanrate/absfieldsize,result->fieldthresh);
    fprintf(result->fpreport,"\t%g\t\t :%5.3g Mean-relative Threshold Field size (gridpix)\n",fieldsize,meanrate);
    fprintf(result->fpreport,"\t%g\t\t :%5.3g Mean-relative Threshold Mean Infield Rate (spikes/sec)\n",summeanrate/fieldsize,meanrate);
    fprintf(result->fpreport,"\t%g\t\t :%5.3g Peak Threshold Field size (gridpix)\n",peakfieldsize,peakthresh);
    fprintf(result->fpreport,"\t%g\t\t :%5.3g Peak Threshold Infield Mean Rate (spikes/sec)\n",peakmeanrate/peakfieldsize,peakthresh);
    fprintf(result->fpreport,"\t%g\t\t :Fixed Field size (percent of sampled space)\n",
	100.0*absfieldsize/npsamples);
    fprintf(result->fpreport,"\t%g\t\t :Field spread X (sd)\n", fieldvarx);
    fprintf(result->fpreport,"\t%g\t\t :Field spread Y (sd)\n", fieldvary);
    fprintf(result->fpreport,"\t%g\t\t :Fixed Field spread X (sd)\n", absfieldvarx);
    fprintf(result->fpreport,"\t%g\t\t :Fixed Field spread Y (sd)\n", absfieldvary);
    fprintf(result->fpreport,"\t%g\t\t :Peaked Field spread X (sd)\n", peakfieldvarx);
    fprintf(result->fpreport,"\t%g\t\t :Peaked Field spread Y (sd)\n", peakfieldvary);
    if((result->behavmode == DIRECTION) && (result->savemode == VECTOR)){
	fprintf(result->fpreport,"\t%g\t\t :Directional index\n", dimax);
	fprintf(result->fpreport,"\t%d\t\t :Directional bias (deg)\n", didir);
    }
    /*
    ** 6/15/01 added field peak statistics
    */
    fprintf(result->fpreport,"\t%d\t\t :Peak Field X (gridpix)\n",peakx);
    fprintf(result->fpreport,"\t%d\t\t :Peak Field Y (gridpix)\n",peaky);
    fprintf(result->fpreport,"\t%-7.4g\t\t :Peak Firing Rate (spikes/sec)\n",peakrate);
    fprintf(result->fpreport,"\t%g\t\t :Weighted Field X (gridpix)\n",weightedx);
    fprintf(result->fpreport,"\t%g\t\t :Weighted Field Y (gridpix)\n",weightedy);
    /*
    ** 12/10/02 added info printout
    */
    fprintf(result->fpreport,"\t%g\t\t :Spatial Information (bits/second)\n",info);
    fprintf(result->fpreport,"\t%g\t\t :Spatial Information Per Spike (bits/spike)\n",info/meanrate);
    /*
    ** evaluate connectivity of suprathreshold points to
    ** reach the final field estimate
    for(i=0;i<result->xsize;i++){
	for(j=0;j<result->ysize;j++){
	    if(result->field_estimate[i][j] > 0){
	    }
	}
    }
    */
}

