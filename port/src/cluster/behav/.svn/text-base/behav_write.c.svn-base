#include "behav_ext.h"

void WriteXviewHeader(result)
Result	*result;
{
int	ival;
float	fval;

    if(result->fp == NULL) return;
    if(result->savemode == VECTOR){
	ival = 359;
	fwrite(&ival,sizeof(int),1,result->fp);
	ival = 0;
	fwrite(&ival,sizeof(int),1,result->fp);
	if(result->binsize > 0){
	    fval = result->binsize/10.0;
	} else {
	    fval = 1.0;
	}
	fwrite(&fval,sizeof(float),1,result->fp);
	ival = FLOAT;
	fwrite(&ival,sizeof(int),1,result->fp);
    } else {
	ival = result->xsize-1;
	fwrite(&ival,sizeof(int),1,result->fp);
	ival = result->ysize-1;
	fwrite(&ival,sizeof(int),1,result->fp);
	if(result->binsize > 0){
	    fval = result->binsize/10.0;
	} else {
	    fval = 1.0;
	}
	fwrite(&fval,sizeof(float),1,result->fp);
	ival = FLOAT;
	fwrite(&ival,sizeof(int),1,result->fp);
	if(result->fpvar){
	    ival = result->xsize-1;
	    fwrite(&ival,sizeof(int),1,result->fpvar);
	    ival = result->ysize-1;
	    fwrite(&ival,sizeof(int),1,result->fpvar);
	    fval = 1;
	    fwrite(&fval,sizeof(float),1,result->fpvar);
	    ival = FLOAT;
	    fwrite(&ival,sizeof(int),1,result->fpvar);
	}
    }
}

void WriteBinaryHeader(result)
Result	*result;
{
int	ival;

    if(result->fp == NULL) return;
    if(result->savemode == VECTOR){
	ival = 2;
	fwrite(&ival,sizeof(int),1,result->fp);
	ival = FLOAT;
	fwrite(&ival,sizeof(int),1,result->fp);
	fwrite(&ival,sizeof(int),1,result->fp);
    } else {
	ival = 3;
	fwrite(&ival,sizeof(int),1,result->fp);
	ival = FLOAT;
	fwrite(&ival,sizeof(int),1,result->fp);
	fwrite(&ival,sizeof(int),1,result->fp);
	fwrite(&ival,sizeof(int),1,result->fp);
	if(result->fpvar){
	    ival = 3;
	    fwrite(&ival,sizeof(int),1,result->fpvar);
	    ival = FLOAT;
	    fwrite(&ival,sizeof(int),1,result->fpvar);
	    fwrite(&ival,sizeof(int),1,result->fpvar);
	    fwrite(&ival,sizeof(int),1,result->fpvar);
	}
    }
}

void WriteGrid(result)
Result	*result;
{
int	i,j;
float	se;
float	mean;
int	n;
float	fval;
int	x,y;
float	smoothgrid;
float	smoothgridtotaln;
int	count;
double	cov;
double	meansd;
double	meanrate;
int	ncov;
double	var;

    cov = 0;
    meansd = 0;
    meanrate = 0;
    ncov = 0;
    for(j=0;j<result->ysize;j++){
	/*
	** grid sum
	*/
	if(result->fp){
	    for(i=0;i<result->xsize;i++){
		/*
		** if smoothing is selected then smooth the various
		** measures
		*/
		if(result->smooth > 0){
		    smoothgrid = 0;
		    smoothgridtotaln = 0;
		    count = 0;
		    if(BoundsTest(result,i,j)){
			for(y=j-result->smooth;y<=j+result->smooth;y++){
			    if((y < 0) || (y >= result->ysize)){
				continue;
			    }
			    for(x=i-result->smooth;x<=i+result->smooth;x++){
				if((x >= 0) && (x < result->xsize) && 
				BoundsTest(result,x,y) &&
				(((x-i)*(x-i) + (y-j)*(y-j)) <=
				result->smooth*result->smooth)
				){
				    /*
				    ** only include points with non-zero
				    ** occupancy in the smoothed result
				    */
				    if(result->gridtotaln[x][y] > result->minpsamples){
					smoothgrid += result->grid[x][y];
					smoothgridtotaln += result->gridtotaln[x][y];
					count++;
				    }
				}
			    }
			}
		    }
		    if(count > 0){
			smoothgrid /= count;
			smoothgridtotaln /= count;
		    } else {
			smoothgrid = -999;
			smoothgridtotaln = 1;
		    }
		} else {
		    if(result->gridtotaln[i][j] > result->minpsamples){
			smoothgrid = result->grid[i][j];
			smoothgridtotaln = result->gridtotaln[i][j];
		    } else {
			smoothgrid = 0;
			smoothgridtotaln = 0;
		    }
		}
		if(result->resulttype == RELIABILITY_RESULT){
		    if(result->gridtotaln[i][j] == 0){
			mean = inf;
		    } else {
			mean = (float)result->gridn[i][j]/result->gridtotaln[i][j];
		    }
		} else
		if(result->resulttype == SAMPLEBIAS_RESULT){
		    mean = result->grid[i][j];
		} else
		if(result->resulttype == NONZFIRING_RESULT){
		    mean = result->gridn[i][j];
		} else
		if(result->resulttype == ZFIRING_RESULT){
		    mean = result->gridtotaln[i][j] - result->gridn[i][j];
		} else
		if(result->resulttype == POSITION_SAMPLES){
		    mean = result->gridtotaln[i][j];
		} else
		if(result->resulttype == OCC_RESULT){
		    mean = result->occupancy_gridn[i][j];
		} else
		if(result->resulttype == OCCRATE_RESULT){
		    if(result->normalize){
			n = result->occupancy_gridn[i][j];
			if(n == 0){
			    mean = inf;
			} else {
			    /*
			    ** compute the average rate for position i,j
			    ** by dividing the cumulative correlated events 
			    ** for the position
			    ** the total occupancy for the position.
			    */
			    mean = result->occupancy_grid[i][j]/n;
			}
		    } else {
			mean = result->occupancy_grid[i][j];
		    }
		} else
		if(result->resulttype == CORR_RESULT){
		    if(result->normalize){
			n = smoothgridtotaln;
			if(n == 0){
			    mean = inf;
			} else {
			    /*
			    ** compute the average rate for position i,j
			    ** by dividing the cumulative correlated events for 
			    ** the position by
			    ** the total occupancy for the position.
			    */
			    mean = smoothgrid/n;
			    /*
			    ** convert to 1/sec frequency measure
			    */
			    mean /= result->pinterval;
			}
		    } else {
			mean = smoothgrid;
		    }
		} else
		if(result->resulttype == CUM_RESULT){
		    if(result->normalize){
			n = smoothgridtotaln;
			if(n == 0){
			    mean = inf;
			} else {
			    /*
			    ** compute the average rate for position i,j
			    ** by dividing the cumulative correlated events for 
			    ** the position by
			    ** the total occupancy for the position.
			    */
			    mean = smoothgrid/n;
			}
		    } else {
			mean = smoothgrid;
		    }
		} else {
		    fprintf(stderr,"ERROR: unknown grid output type\n");
		    break;
		}
		if(result->xviewformat){
		    fwrite(&mean,sizeof(float),1,result->fp);
		} else 
		if(result->ascii){
		    fprintf(result->fp,"%d \t%d \t%g\n",i,j,mean);
		} else {
		    fval = i;
		    fwrite(&fval,sizeof(float),1,result->fp);
		    fval = j;
		    fwrite(&fval,sizeof(float),1,result->fp);
		    fwrite(&mean,sizeof(float),1,result->fp);
		}
	    }
	}
	/*
	** grid variance
	*/
	if(result->normalize && result->fpvar){
	    for(i=0;i<result->xsize;i++){
		if(result->resulttype == OCCRATE_RESULT){
		    /*
		    ** note that the occupancy_grid values
		    ** are firing rates not spike counts. i.e. they
		    ** are already normalized for time interval
		    */
		    n = result->occupancy_gridn[i][j];
		    /*
		    ** what to do about 0 occupancy
		    */
		    if(n == 0 || n == 1){
			se = inf;
		    } else {
			/*
			** compute the variance
			*/
			var = ((double)(result->occupancy_gridsqr[i][j] - 
			    result->occupancy_grid[i][j]*
			    result->occupancy_grid[i][j]/n)/(n-1));
			/*
			** compute the standard deviation of the mean
			*/
			if(var > 0){
			    se = sqrt(var);
			} else {
			    se = 0;
			}
			/*
			** compute the mean COV
			*/
			if(((float)result->occupancy_grid[i][j]/n) >
			result->fieldthresh){
			    cov += se*n/result->occupancy_grid[i][j];
			    meansd += se;
			    meanrate += (float)result->occupancy_grid[i][j]/n;
			    ncov++;
			}
		    }
		} else 
		if(result->resulttype == CORR_RESULT){
		    n = result->gridtotaln[i][j];
		    /*
		    ** what to do about 0 occupancy
		    */
		    if(n == 0 || n == 1){
			se = inf;
		    } else {
			/*
			** convert to 1/sec frequency measure
			** and normalize by mean
			*/
			var = ((result->gridsumsqr[i][j] - 
			result->grid[i][j]*result->grid[i][j]/
			n)/(n*(n-1)*result->pinterval*result->pinterval));
			if(var > 0){
			    se = sqrt(var);
			} else {
			    se = 0;
			}
			/*
			** compute the mean COV
			*/
			if((float)result->grid[i][j]/(n*result->pinterval) > 
			result->fieldthresh){
			    cov += se/
			    ((float)result->grid[i][j]/(n*result->pinterval));
			    meansd += se;
			    meanrate += (float)result->grid[i][j]/(n*result->pinterval);
			    ncov++;
			}
		    }
		} else {
		    fprintf(stderr,"ERROR: unable to compute variance for selected output\n");
		    break;
		}
		if(result->xviewformat){
		    fwrite(&se,sizeof(float),1,result->fpvar);
		} else {
		    fval = i;
		    fwrite(&fval,sizeof(float),1,result->fpvar);
		    fval = j;
		    fwrite(&fval,sizeof(float),1,result->fpvar);
		    fwrite(&se,sizeof(float),1,result->fpvar);
		}
	    }
	}
    }
    if(result->fpvar && ncov > 0){
	fprintf(stdout,"%% COV \tsd \tmean \tn\n");
	fprintf(stdout,"%g\t%g\t%g\t%d\n",cov/ncov,meansd/ncov,meanrate/ncov,ncov);
    }
}

void WriteVector(result)
Result	*result;
{
int	i;
float	se;
float	mean;
int	n;

    /*
    ** output count vs degrees
    */
    for(i=0;i<result->dsize;i++){
	if(result->resulttype == SAMPLEBIAS_RESULT){
		mean = result->vectortotaln[i];
	} else
	if(result->resulttype == RELIABILITY_RESULT){
	    if(result->vectortotaln[i] == 0){
		mean = inf;
	    } else {
		mean = (float)result->vectorn[i]/result->vectortotaln[i];
	    }
	} else
	if(result->resulttype == NONZFIRING_RESULT){
	    mean = result->vectorn[i];
	} else
	if(result->resulttype == ZFIRING_RESULT){
	    mean = result->vectortotaln[i] - result->vectorn[i];
	} else
	if(result->resulttype == CORR_RESULT){
	    if(result->normalize){
		n = result->vectortotaln[i];
		if(n == 0){
		    mean = inf;
		} else {
		    mean = (float)result->vector[i]/n;
		    mean /= result->pinterval;
		}
	    } else { 
		mean = result->vector[i];
	    }
	} else {
	    fprintf(stderr,"ERROR: unknown vector output type\n");
	}
	if(result->format == BINARY){
	    fwrite(&mean,sizeof(float),1,result->fp);
	} else {
	    switch(result->plotmode){
	    case LINEAR:
		fprintf(result->fp,"%g %g\n",
		    (float)(i*MAXTHETA)/result->dsize,mean);
		break;
	    case POLAR:
		fprintf(result->fp,"%g %g\n",
		    (float)mean*cos(2*M_PI*i*MAXTHETA/(360*result->dsize)),
		    (float)mean*sin(2*M_PI*i*MAXTHETA/(360*result->dsize)));
		break;
	    }
	}
    }
    /*
    ** vector variance
    */
    if(result->normalize && result->fpvar){
	for(i=0;i<result->dsize;i++){
	    if(result->resulttype == CORR_RESULT){
		n = result->vectortotaln[i];
		if(n == 0 || n == 1){
		    se = inf;
		} else {
		    /*
		    ** convert to 1/sec frequency measure
		    */
		    se = sqrt((result->vectorsumsqr[i] -
		    result->vector[i]*result->vector[i]/n)/
		    (n*(n-1)*result->pinterval*result->pinterval));
		}
		if(result->format == BINARY){
		    fwrite(&se,sizeof(float),1,result->fpvar);
		} else {
		    fprintf(result->fpvar,"%g %g\n",
		    (float)(i*MAXTHETA)/result->dsize,se);
		}
	    } else {
		fprintf(stderr,"ERROR: unable to compute variance of selected output\n");
		break;
	    }
	}
    }
}


void PrintResults(result,plist)
Result	*result;
PositionList	*plist;
{
float tmp, num;

    fprintf(stderr,"positions processed =\t\t%d\n", 
	plist->nprocessedpositions);
    fprintf(stderr,"missing positions =\t\t%d \t(%.2g%%)\n",
	result->positionmissing,
	100.0*result->positionmissing/plist->nprocessedpositions);
#ifdef OLD
    fprintf(stderr,"missing directions =\t\t%d \t(%.2g%%)\n", 
	result->directionmissing, 
	100.0*result->directionmissing/plist->nprocessedpositions);
#endif
    fprintf(stderr,"valid positions =\t\t%d \t(%.2g%%)\n", 
	result->validposition, 
	100.0*result->validposition/plist->nprocessedpositions);
#ifdef OLD
    fprintf(stderr,"directions processed =\t\t%d \t(%.2g%%)\n", 
	result->validdirection, 
	100.0*result->validdirection/plist->nprocessedpositions);
#endif
    fprintf(stderr,"front diode occlusions =\t%d \t(%.2g%%)\n",
	result->nofrontdiode,
	100.0*result->nofrontdiode/plist->nprocessedpositions);
    fprintf(stderr,"back diode occlusions =\t\t%d \t(%.2g%%)\n", 
	result->nobackdiode, 
	100.0*result->nobackdiode/plist->nprocessedpositions);
    fprintf(stderr,"significant gaps =\t\t%d \t(%.2g%%)\n",
	result->gapflip, 100.0*result->gapflip/plist->nprocessedpositions);
    fprintf(stderr,"total gap flips =\t\t%d \t(%.2g%%)\n",
	result->corrected_gapflips, 
	100.0*result->corrected_gapflips/plist->nprocessedpositions);
    fprintf(stderr,"total flips =\t\t\t%d \t(%.2g%%)\n",
	result->corrected_flips, 
	100.0*result->corrected_flips/plist->nprocessedpositions);
    fprintf(stderr,"failed radius test =\t\t%d \t(%.2g%%)\n",
	result->exceededradius, 
	100.0*result->exceededradius/plist->nprocessedpositions);
    fprintf(stderr,"direction gaps =\t\t%d\n", 
	result->ndirectiongaps);
    fprintf(stderr,"max gap length =\t\t%d (%g msec)\n", 
	result->maxconsecutive,result->maxconsecutive*result->pinterval*1e3);
     tmp = (result->validdirection - result->corrected_gapflips);
     num = (tmp == 0) ? 0 : (float)result->directionsum / tmp;
     fprintf(stderr,"avg delta direction (total) =\t%g deg\n", num);

     tmp = (result->corrected_flips - result->corrected_gapflips);
     num = (tmp == 0) ? 0 : (float)result->flipdirectionsum / tmp;
     fprintf(stderr,"avg delta direction (flip) =\t%g deg\n", num);

    if(result->nvelocityang > 0){
	fprintf(stderr,"avg velocity discrepancy =\t%g deg\n",
	(float)result->sumvelocitydiff/result->nvelocityang);
    }
    if(result->validdirection){
	fprintf(stderr,"vel/dir discrepancies =\t\t%d\t(%.2g%%)\n",
	result->nvangdiff,100.0*result->nvangdiff/result->validdirection);
    }
#ifdef OLD
    fprintf(stderr,"diode angle discrepancies =\t%d\t(%.2g%%)\n",
	result->nvmagdiff,100.0*result->nvmagdiff/
	(result->nvmagdiff + result->nvmagok));
#endif
    fprintf(stderr,"flip lookahead =\t\t%d valid intervals\n", 
	maxlookahead);
    fprintf(stderr,"max acceptable gap =\t\t%d (%g msec)\n", 
	max_consecutive_miss,(float)maxgaplen/10);
    fprintf(stderr,"radius test (min/max) =\t\t%d/%d pts\n", 
	min_rad,max_rad);
    fprintf(stderr,"flip criterion =\t\t%d deg\n", 
	FLIPANG);
    fprintf(stderr,"position sample interval =\t%g msec\n", 
	result->pinterval*1e3);
    fprintf(stderr,"tracker frame resolution =\t%g x %g pixels\n", 
	result->xres, result->yres);
    fprintf(stderr,"Total occupancy time =\t%s (%g msec)\n",
	TimestampToString((long)(result->totaltime*result->pinterval*1e4)),
	result->totaltime*result->pinterval*1e3);
}

void WritePositionFile(result,timestamp,x1,y1,x2,y2)
Result	*result;
unsigned long timestamp;
int	x1,y1;
int	x2,y2;
{
short	sval;

    if(result->fpfix == NULL){
	return;
    }
    if(fwrite(&timestamp,sizeof(unsigned long),1,result->fpfix) != 1){
	fprintf(stderr,"ERROR: writing to fix file\n");
	exit(-1);
    }
    sval = x1;
    if(fwrite(&sval,sizeof(short),1,result->fpfix) != 1){
	fprintf(stderr,"ERROR: writing to fix file\n");
	exit(-1);
    }
    sval = y1;
    if(fwrite(&sval,sizeof(short),1,result->fpfix) != 1){
	fprintf(stderr,"ERROR: writing to fix file\n");
	exit(-1);
    }
    sval = x2;
    if(fwrite(&sval,sizeof(short),1,result->fpfix) != 1){
	fprintf(stderr,"ERROR: writing to fix file\n");
	exit(-1);
    }
    sval = y2;
    if(fwrite(&sval,sizeof(short),1,result->fpfix) != 1){
	fprintf(stderr,"ERROR: writing to fix file\n");
	exit(-1);
    }
}

void  WriteRateFile(result,timestamp,x,y,dir,rate)
Result	*result;
unsigned long timestamp;
int	x,y,dir;
float	rate;
{
unsigned char	cval;
short	sval;
float	fval;
unsigned long	lval;

    if(result->fprate == NULL){
	return;
    }
    lval = timestamp;
    if(fwrite(&lval,sizeof(unsigned long),1,result->fprate) != 1){
	fprintf(stderr,"ERROR: writing to rate file\n");
	exit(-1);
    }
    cval = x;
    if(fwrite(&cval,sizeof(unsigned char),1,result->fprate) != 1){
	fprintf(stderr,"ERROR: writing to rate file\n");
	exit(-1);
    }
    cval = y;
    if(fwrite(&cval,sizeof(unsigned char),1,result->fprate) != 1){
	fprintf(stderr,"ERROR: writing to rate file\n");
	exit(-1);
    }
    sval = dir;
    if(fwrite(&sval,sizeof(short),1,result->fprate) != 1){
	fprintf(stderr,"ERROR: writing to rate file\n");
	exit(-1);
    }
    fval = rate;
    if(fwrite(&fval,sizeof(float),1,result->fprate) != 1){
	fprintf(stderr,"ERROR: writing to rate file\n");
	exit(-1);
    }
}


void OutputFileHeader(fp,result,spikearray,filename,formatstr,argc,argv)
FILE	*fp;
Result	*result;
SpikeList	*spikearray;
char		*filename;
char		*formatstr;
int		argc;
char		**argv;
{
int	i;

    if(fp == NULL) return;
    BeginStandardHeader(fp,argc,argv,VERSION);
    fprintf(fp,"%% Current file: \t%s\n",filename);
    fprintf(fp,"%% File type: \t%s\n",formatstr);
    fprintf(fp,"%% Position file: \t%s\n",result->pfname);
    for(i=0;i<result->nclusters;i++){
	fprintf(fp,"%% Spike time file: \t%s",
	spikearray[i].fname);
	if(spikearray[i].timeshift != 0){
	    fprintf(fp," \t%g msec timeshift\n",
	    spikearray[i].timeshift/10.0);
	} else {
	    fprintf(fp,"\n");
	}
    }
    fprintf(fp,"%% Output file: \t%s\n",result->fpoutname);
    fprintf(fp,"%% Occupancy stat file: \t");
    if(result->fpocc){
	fprintf(fp,"%s\n",result->fpoccname);
    } else {
	fprintf(fp,"%s\n","none");
    }
    fprintf(fp,"%% Variance output file: \t");
    if(result->fpvar){
	fprintf(fp,"%s\n",result->fpvarname);
    } else {
	fprintf(fp,"%s\n","none");
    }
    fprintf(fp,"%% Dir/vel output file: \t");
    if(result->fpdir){
	fprintf(fp,"%s\n",result->fpdirname);
    } else {
	fprintf(fp,"%s\n","none");
    }
    fprintf(fp,"%% Event type: \t");
    switch(result->spatial_measure){
    case EVENTS:
	fprintf(fp,"%s\n","Events");
	break;
    case INTERVALS:
	fprintf(fp,"%s\n","Intervals");
	break;
    case INTERVALTHRESH:
	fprintf(fp,"%s\n","Threshold interval");
	break;
    }
    fprintf(fp,"%% Behavioral correlate: \t");
    switch(result->behavmode){
    case POSITION:
	fprintf(fp,"%s\n","Position");
	break;
    case DIRECTION:
	fprintf(fp,"%s\n","Direction");
	break;
    }
    fprintf(fp,"%% Position evaluation: \t");
    switch(result->positioneval){
    case CENTROID:
	fprintf(fp,"%s\n","Centroid");
	break;
    case FRONT:
	fprintf(fp,"%s\n","Front diode");
	break;
    }
    fprintf(fp,"%% Analysis format: \t");
    switch(result->savemode){
    case GRID:
	fprintf(fp,"%s\n","Grid");
	if(result->fpangularrange){
	    fprintf(fp,"%% Angular range map: \t%s",result->fpangularrangename);
	} else {
	    fprintf(fp,"%% Mintheta: \t%d\n",result->mintheta);
	    fprintf(fp,"%% Maxtheta: \t%d\n",result->maxtheta);
	}
	if(result->xviewformat){
	    fprintf(fp,"%% File type: \t%s\n","Xview");
	}
	fprintf(fp,"%% Xsize: \t%d\n",result->xsize);
	fprintf(fp,"%% Ysize: \t%d\n",result->ysize);
	fprintf(fp,"%% Xoffset: \t%d\n",result->xoffset);
	fprintf(fp,"%% Yoffset: \t%d\n",result->yoffset);
	break;
    case VECTOR:
	fprintf(fp,"%s\n","Vector");
	fprintf(fp,"%% Dsize: \t%d\n",result->dsize);
	break;
    }
    fprintf(fp,"%% Analysis type: \t");
    switch(result->resulttype){
    case CORR_RESULT:
	fprintf(fp,"%s\n","Correlation");
	break;
    case RELIABILITY_RESULT:
	fprintf(fp,"%s\n","Reliability");
	break;
    case SAMPLEBIAS_RESULT:
	fprintf(fp,"%s\n","Samplebias");
	break;
    case ZFIRING_RESULT:
	fprintf(fp,"%s\n","Zero firing");
	break;
    case NONZFIRING_RESULT:
	fprintf(fp,"%s\n","Non-zero firing");
	break;
    case OCC_RESULT:
	fprintf(fp,"%s\n","Occupancy");
	break;
    case OCCRATE_RESULT:
	fprintf(fp,"%s\n","Occupancy firing rate");
	break;
    }
    fprintf(fp,"%% Normalize: \t%s\n",TFstr(result->normalize));
    fprintf(fp,"%% Occ radius: \t%g pixels\n",result->occradius);
    if(result->enable_occdirection_change){
	fprintf(fp,"%% Occupancy changes with directional shifts\n");
	fprintf(fp,"%% Occ dirangle: \t%g degrees\n",result->occdirangle);
    }
    if(result->limited_occupancy != 0){
	fprintf(fp,"%% Occ limit: \t%g msec\n",
	result->limited_occupancy/10.0);
    }
    if(starttime != 0){
	fprintf(fp,"%% Start time: \t%s\n",TimestampToString(starttime));
    }
    if(endtime != 0){
	fprintf(fp,"%% End time: \t%s\n",TimestampToString(endtime));
    }
    if(endspike != 0){
	fprintf(fp,"%% End spike: \t%d\n",endspike);
    }
    if(startspike != 0){
	fprintf(fp,"%% Start spike: \t%d\n",startspike);
    }
    if(result->binsize != 0){
	fprintf(fp,"%% Binsize: \t%g msec\n",result->binsize/10.0);
    }
    fprintf(fp,"%% Maxlookahead: \t%d samples\n",maxlookahead);
    fprintf(fp,"%% Maxgap: \t%d samples\n",max_consecutive_miss);
    fprintf(fp,"%% Min/max radius: \t%d/%d pixels\n",min_rad,max_rad);
    fprintf(fp,"%% Infinity: \t%d\n",inf);
}

void PrepareOutputFiles(result,spikearray,argc,argv)
Result	*result;
SpikeList	*spikearray;
int	argc;
char	**argv;
{
char	*formatstr = NULL;

    /*
    ** write out the output file headers
    */
    switch(result->format){
    case BINARY:
	formatstr = "Binary";
	break;
    case ASCII:
	formatstr = "Ascii";
	break;
    }

    OutputFileHeader(result->fp,result,spikearray,result->fpoutname,
	formatstr,argc,argv);
    /*
    ** add the fields strings for the various file types
    ** -- incomplete at this point
    */
    if((result->behavmode == VELOCITY) && (result->savemode == GRID) && (result->format == ASCII)){
	fprintf(result->fp,"%% Fields :\t x \ty \tvel\n");
    }
    if((result->behavmode == POSITION) && (result->savemode == GRID) && (result->format == ASCII)){
	fprintf(result->fp,"%% Fields :\t x \ty \tmean\n");
    }
    EndStandardHeader(result->fp);

    if(result->fpreport){
	OutputFileHeader(result->fpreport,result,spikearray,"",
	    "Ascii",argc,argv);
	EndStandardHeader(result->fpreport);
    }

    if(result->fpdir){
	OutputFileHeader(result->fpdir,result,spikearray,result->fpdirname,
	    "Ascii",argc,argv);
	fprintf(result->fpdir,"%% Data fields :\t [LINE+1]\n");
	fprintf(result->fpdir,"%% timestamp \tx1 \ty1 \tx2 \ty2 \tdir \tangv \tvmag \tvang \tflip \tgapflip\n");
	EndStandardHeader(result->fpdir);
    }

    OutputFileHeader(result->fpvar,result,spikearray,result->fpvarname,
	formatstr,argc,argv);
    EndStandardHeader(result->fpvar);

    OutputFileHeader(result->fpocc,result,spikearray,result->fpoccname,
	"Ascii",argc,argv);
    EndStandardHeader(result->fpocc);

    OutputFileHeader(result->fprate,result,spikearray,result->fpratename,
	"Binary",argc,argv);
    EndStandardHeader(result->fprate);

    if(result->fpfix){
	OutputFileHeader(result->fpfix,result,spikearray,result->fpfixname,
	"Binary",argc,argv);
	fprintf(result->fpfix,
	"%% Fields: \ttimestamp,%d,%d,%d \tx1,%d,%d,%d \ty1,%d,%d,%d \tx2,%d,%d,%d \ty2,%d,%d,%d\n",
	INT,sizeof(unsigned long),1,
	SHORT,sizeof(short),1,
	SHORT,sizeof(short),1,
	SHORT,sizeof(short),1,
	SHORT,sizeof(short),1);
	fprintf(result->fpfix,
	"%% Record type: \tP\n");
	fprintf(result->fpfix,
	"%% Extraction type: \textended dual diode positions\n");
	EndStandardHeader(result->fpfix);
    }

    if(result->format == BINARY){
	if(result->xviewformat){
	    WriteXviewHeader(result);
	} else {
	    WriteBinaryHeader(result);
	}
    }
}
