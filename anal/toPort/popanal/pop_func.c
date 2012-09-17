#include "pop_ext.h"

ComputeComponentSums(result)
Result	*result;
{
int	i,j;

    for(j=0;j<result->nclusters;j++){
	for(i=0;i<result->ntimebins;i++){
	    result->datasum[j] += result->datavector[i][j];
	    result->datasqr[j] += result->datavector[i][j]*result->datavector[i][j];
	}
    }
}

NormalizeVectors(result)
Result	*result;
{
int	i,j;
double	var;

    for(j=0;j<result->nclusters;j++){
	var = (result->ntimebins*result->datasqr[j] - 
	    result->datasum[j]*result->datasum[j])/
	    ((double)result->ntimebins*(result->ntimebins-1));
	if(var <= TINYVAL) continue;
	for(i=0;i<result->ntimebins;i++){
	    result->datavector[i][j] /= sqrt(var);
	}
    }
}

AllocateCorrGrids(result)
Result	*result;
{
int	i;

    /*
    ** allocate correlation grid
    */
    result->corrmatrix = (float **)malloc((result->nclusters)*sizeof(float *));
    for(i=0;i<result->nclusters;i++){
	result->corrmatrix[i] = (float *)calloc(result->nclusters,sizeof(float));
    }
}

AllocateSpatialGrids(result)
Result	*result;
{
int	i;

    /*
    ** allocate correlation grid
    */
    result->corrgrid = (float **)calloc((result->ymax+1),sizeof(float *));
    result->smoothcorrgrid = (float **)calloc((result->ymax+1),sizeof(float *));
    result->truemaggrid = (float **)calloc((result->ymax+1),sizeof(float *));
    result->maggrid = (float **)calloc((result->ymax+1),sizeof(float *));
    result->sparsegrid = (float **)calloc((result->ymax+1),sizeof(float *));
    result->perrgrid = (float **)calloc((result->ymax+1),sizeof(float *));
    result->smoothperrgrid = (float **)calloc((result->ymax+1),sizeof(float *));
    result->occgrid = (float **)calloc((result->ymax+1),sizeof(float *));
    for(i=0;i<=result->ymax;i++){
	result->corrgrid[i] = (float *)calloc((result->xmax+1),sizeof(float));
	result->smoothcorrgrid[i] = (float *)calloc((result->xmax+1),sizeof(float));
	result->maggrid[i] = (float *)calloc((result->xmax+1),sizeof(float));
	result->truemaggrid[i] = (float *)calloc((result->xmax+1),sizeof(float));
	result->sparsegrid[i] = (float *)calloc((result->xmax+1),sizeof(float));
	result->perrgrid[i] = (float *)calloc((result->xmax+1),sizeof(float));
	result->smoothperrgrid[i] = (float *)calloc((result->xmax+1),sizeof(float));
	result->occgrid[i] = (float *)calloc((result->xmax+1),sizeof(float));
    }
    /*
    ** allocate peak vector
    */
    result->peak = (float *)calloc(result->nclusters,sizeof(float));
    result->peakloc = (GridPosition *)calloc(result->nclusters,sizeof(GridPosition));
}

ComputeMagnitudeGrid(result)
Result	*result;
{
int	j;
int	x,y;
double	magnitude;

    /*
    ** initialize the magnitude grid so that unprocessed
    ** regions are can be distinguised (used by xview display program) 
    */
    for(y=0;y<=result->ymax;y++){
	for(x=0;x<=result->xmax;x++){
	    result->truemaggrid[y][x] = -999;
	}
    }
    for(y=result->ylo;y<=result->yhi;y++){
	for(x=result->xlo;x<=result->xhi;x++){
	    magnitude = 0;
	    for(j=0;j<result->nclusters;j++){
		if(result->pdir[j].ignore) continue;
		magnitude += result->positiongrid[j][y][x] *
		    result->positiongrid[j][y][x];
	    }
	    result->truemaggrid[y][x] = sqrt((double)magnitude);
	}
    }
}

ComputePositionPeak(result)
Result	*result;
{
int	j;
int	x,y;
float	peak;
float	**grid;
float	*gridvec;
int	xpeak,ypeak;

    /*
    ** compute the peak of the position grid vectors
    */
    for(j=0;j<result->nclusters;j++){
	if(result->pdir[j].ignore) continue;
	grid = result->positiongrid[j];
	peak = 0;
	xpeak = -1;
	ypeak = -1;
	for(y=result->ylo;y<=result->yhi;y++){
	    /*
	    ** dont look at locations outside of the bounds
	    */
	    if(result->gylo >= 0 && (y > result->gyhi || y < result->gylo)){
		continue;
	    }
	    gridvec = grid[y];
	    for(x=result->xlo;x<=result->xhi;x++){
		if(result->gxlo >= 0 && (x > result->gxhi || x < result->gxlo)){
		    continue;
		}
		if(gridvec[x] > peak){
		    /*
		    ** keep track of the peak rate and location
		    */
		    peak = gridvec[x];
		    xpeak = x;
		    ypeak = y;
		}
	    }
	}
	/*
	** if a minimum rate has been specified then check to see
	** whether the peak is less than this
	*/
	if(peak > result->minpeak){ 
	    result->peak[j] = peak;
	    result->peakloc[j].x = xpeak;
	    result->peakloc[j].y = ypeak;
	} else {
	    /*
	    ** use the min peak and assign an invalid peak location
	    */
	    result->peak[j] = result->minpeak;
	    result->peakloc[j].x = -1;
	    result->peakloc[j].y = -1;
	}
	/*
	** check the bounds
	*/
	if(result->gxlo >= 0 && 
	(xpeak > result->gxhi ||
	xpeak < result->gxlo ||
	ypeak > result->gyhi ||
	ypeak < result->gylo)){
	    /*
	    ** out of bounds
	    */
	    result->peakloc[j].x = -1;
	    result->peakloc[j].y = -1;
	}

	if((result->peakloc[j].x != -1) && (result->fpfieldcenter)){
	    /*
	    ** write out the field centers
	    */
	    fprintf(result->fpfieldcenter,"%d\t%d\n",
	    result->peakloc[j].x,
	    result->peakloc[j].y);
	}
    }
    if(result->fpfieldcenter){
	fclose(result->fpfieldcenter);
    }
}

NormalizePositionGridVectors(result)
Result	*result;
{
int	j;
int	x,y;
float	peak;
float	**grid;

    /*
    ** normalize the each component of the position grid vectors by
    ** the peak rate
    */
    for(j=0;j<result->nclusters;j++){
	if(result->pdir[j].ignore) continue;
	grid = result->positiongrid[j];
	peak = result->peak[j];
	/*
	** dont process zero peak rate clusters
	*/
	if(peak <= 0) continue;
	for(y=result->ylo;y<=result->yhi;y++){
	    for(x=result->xlo;x<=result->xhi;x++){
		grid[y][x] /= peak;
	    }
	}
    }
}

ComputeNormalizedMagnitudeGrid(result)
Result	*result;
{
int	j;
int	x,y;
double	magnitude;

    /*
    ** compute the normalized magnitude of the position grid vectors
    */
    for(y=result->ylo;y<=result->yhi;y++){
	for(x=result->xlo;x<=result->xhi;x++){
	    magnitude = 0;
	    for(j=0;j<result->nclusters;j++){
		if(result->pdir[j].ignore) continue;
		magnitude += result->positiongrid[j][y][x] *
		    result->positiongrid[j][y][x];
	    }
	    result->maggrid[y][x] = sqrt((double)magnitude);
	}
    }
}

ComputeSparsity(result)
Result	*result;
{
int	j;
int	x,y;
double	magnitude;

    /*
    ** compute the sparsity of the position grid vectors
    */
    /*
    ** initialize the sparsity grid so that unprocessed
    ** regions are can be distinguised (used by xview display program) 
    */
    for(y=0;y<=result->ymax;y++){
	for(x=0;x<=result->xmax;x++){
	    result->sparsegrid[y][x] = -999;
	}
    }
    for(y=result->ylo;y<=result->yhi;y++){
	for(x=result->xlo;x<=result->xhi;x++){
	    magnitude = 0;
	    /*
	    ** count the number of components which exceed the
	    ** sparsity cutoff ratio of peak rate
	    */
	    for(j=0;j<result->nclusters;j++){
		if(result->pdir[j].ignore) continue;
		/*
		** note that the position grid has already been normalized
		** to the peak rate
		*/
		if(result->sparsity_cutoff > 0){
		    /*
		    ** use a hard thresholded sparsity measure
		    */
		    if(result->positiongrid[j][y][x] > result->sparsity_cutoff){
			magnitude++;
		    }
		} else {
		    /*
		    ** use a continuous sparsity measure
		    */
		    magnitude += result->positiongrid[j][y][x];
		}
	    }
	    /*
	    ** compute sparsity as the fraction of suprathreshold
	    ** vector components
	    */
	    result->sparsegrid[y][x] = magnitude/result->nactivep;
	}
    }
}


ProcessGerstein(result)
Result	*result;
{
int	i;
FILE	*fpin;
int	count;
char	dirname[80];
int	headersize;

    /*
    ** this is a special format for G. Gerstein's analysis
    */
    if(result->fpout == NULL){
	fprintf(stderr,"must specify an output file\n");
	exit(-1);
    }
    /*
    ** open each timestamp file and bin the rate over the desired interval
    */
    for(i=0;i<result->nclusters;i++){
	if(clusterdir[i].ignore) continue;
	/*
	** open the binary timestamp file
	** assume that the timestamp file is of the form dir/tn where n is
	** the cluster number
	*/
	sprintf(dirname,"%s/t%d",clusterdir[i].dirname,clusterdir[i].clusterid);
	if((fpin = fopen(dirname,"r")) == NULL){
	    fprintf(stderr,"ERROR: unable to open cluster timestamp file '%s'\n",
	    dirname);
	    fprintf(stderr,"ignoring cluster\n");
	    clusterdir[i].ignore = 1;
	    continue;
	}
	result->nactivec++;
	ReadHeader(fpin,&headersize);
	/*
	** write out the cluster indexed timestamps. Note that the
	** output cluster ids must start at 3 (ask George)
	*/
	count = WriteGerstein(fpin,result,i+3);
	/*
	** close the data file and go the the next
	*/
	fclose(fpin);
	if(verbose){
	    fprintf(stderr,"%d spikes in %s cluster %d\n",
	    count,dirname,clusterdir[i].clusterid);
	}
    }
}

GetConvertedPosition(result,bin,px,py,px2,py2)
Result	*result;
int	bin;
float	*px,*py;
float	*px2,*py2;
{
    /*
    ** get the position of the front diode
    */
    if(bin < result->ntimebins && bin >= 0 &&
    result->position[bin].x > 0){
	*px = result->position[bin].x*
	(result->xmax+1)/result->xresolution;
	*py = result->position[bin].y*
	(result->ymax+1)/result->yresolution;
    } else {
	*px  = -1;
	*py  = -1;
    }
    /*
    ** try to get the position of the back diode
    */
    if(bin < result->ntimebins && bin >= 0 &&
    result->position[bin].x2 > 0){
	*px2 = result->position[bin].x2*
	(result->xmax+1)/result->xresolution;
	*py2 = result->position[bin].y2*
	(result->ymax+1)/result->yresolution;
    } else {
	*px2  = -1;
	*py2  = -1;
    }

}

