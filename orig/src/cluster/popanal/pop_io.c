#include "pop_ext.h"

int StripCR(line)
char	*line;
{
char	*ptr;

    if(ptr = strchr(line,'\n')){
	*ptr = '\0';
	return(1);
    } else {
	return(0);
    }
}

/*
** read spikes into individual spike arrays rather than
** binning them
*/
int LoadSpikes(fp,result,clusterid)
FILE	*fp;
Result	*result;
int	clusterid;
{
unsigned long	*sarray;
unsigned long	timestamp;
int	count;
char	**header;
char	**ReadHeader();
int 	size;
int	nspikes;
int	*binarray;
int	bin;
double	sum;
double	sumsqr;
double	var;
double	var1;
int	i;
char	*tstring;
int	convert;

    /*
    ** count the number of spikes
    */
    nspikes = 0;
    fseek(fp,0L,0L);		/* rewind spike file */
    header = ReadHeader(fp,&size);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
    } else {
	convert = 1;
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	    GetFileArchitectureStr(header),
	    GetLocalArchitectureStr());
    }

    /*
    ** use the input spike file field to match probes
    */
    if((tstring = GetHeaderParameter(header,"Input Spike file:")) != NULL){
	result->clustername[clusterid] = (char *)calloc(strlen(tstring) + 1,sizeof(char));;
	strcpy(result->clustername[clusterid],tstring);
/*
	if(verbose){
	    fprintf(stderr,"Loading spikes for %s cluster %d\n",
	    result->clustername[clusterid],clusterid);
	}
*/
    }
    /*
    ** compute the total number of spikes in the file from the file length
    */
    fseek(fp,0L,2);
    nspikes = (ftell(fp) - size)/sizeof(unsigned long);
    sarray = result->spikearray[clusterid] = 
    (unsigned long *)calloc(nspikes,sizeof(unsigned long));
    /*
    ** allocate the time bin array to be used to compute
    ** bin variance
    */
    binarray = (int *)calloc(result->ntimebins,sizeof(int));
    /*
    ** make the second pass to fill the spike array
    */
    count = 0;
    fseek(fp,size,0L);		/* rewind spike file */
    while(!feof(fp)){		/* scan the entire file */
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    break;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(!InRange(result,timestamp)) continue;
	if(timestamp < result->tstart){
	    continue;
	}
	if(timestamp > result->tend){
	    break;
	}
	if(count >= nspikes){
	    fprintf(stderr,"ERROR: incongruity in timestamp count\n"); 
	    break;
	}
	sarray[count] = timestamp;
	bin = (timestamp - result->tstart)/result->binsize;
	binarray[bin]++;
	count++;
    }
/*
    if(verbose){
	fprintf(stderr,"Loaded spikes from %s (%ld)",
	TimestampToString(sarray[0]),sarray[0]);
	fprintf(stderr," to %s (%ld)\n",
	TimestampToString(sarray[count-1]),sarray[count-1]);
    }
*/
    /*
    ** compute mean and sd of the binned spike array
    */
    sum = 0;
    sumsqr = 0;
    for(i=0;i<result->ntimebins;i++){
	if(binarray[i] > 0){
	    sum += binarray[i];
	    sumsqr += binarray[i]*binarray[i];
	}
    }
    result->binmean[clusterid] = sum/result->ntimebins;
    var1 = (sumsqr*(double)result->ntimebins - sum*sum);
    var = var1/
	((double)result->ntimebins*(result->ntimebins-1));
    result->binsd[clusterid] = sqrt(var);
    free(binarray);
    return(count);
}

LoadAndBinSpikes(fp,result,clusterid)
FILE	*fp;
Result	*result;
int	clusterid;
{
unsigned long timestamp;
float	*vector;
int	bin;
int	count;
int	headersize;
int	convert;
char	**header;

    fseek(fp,0L,0L);		/* rewind spike file */
    header = ReadHeader(fp,&headersize);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
    } else {
	convert = 1;
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	    GetFileArchitectureStr(header),
	    GetLocalArchitectureStr());
    }

    count = 0;
    while(!feof(fp)){
	/*
	** read a timestamp
	*/
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    break;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(!InRange(result,timestamp)) continue;
	if(timestamp < result->tstart){
	    continue;
	}
	if(timestamp > result->tend){
	    break;
	}
	/*
	** assign it to a bin
	*/
	bin = (timestamp - result->tstart)/result->binsize;
	if(bin < 0){
	    continue;
	}
	if(bin >= result->ntimebins){
	    continue;
	}
	/*
	** increment the time bin contents for the cluster
	*/
	result->datavector[bin][clusterid]++;
	result->timebinstatus[bin].valid = 1;
	count++;
    }
    return(count);
}

ReconLoadAndBinSpikes(fp,result,clusterid)
FILE	*fp;
Result	*result;
int	clusterid;
{
unsigned long timestamp;
float	*vector;
int	bin;
int	count;
int	headersize;
int	convert;
char	**header;

    fseek(fp,0L,0L);		/* rewind spike file */
    header = ReadHeader(fp,&headersize);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
    } else {
	convert = 1;
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	    GetFileArchitectureStr(header),
	    GetLocalArchitectureStr());
    }

    count = 0;
    while(!feof(fp)){
	/*
	** read a timestamp
	*/
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    break;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(!InRange(result,timestamp)) continue;
	if(timestamp < result->recontstart){
	    continue;
	}
	if(timestamp > result->recontend){
	    break;
	}
	/*
	** assign it to a bin
	*/
	bin = (timestamp - result->recontstart)/result->binsize;
	if(bin < 0){
	    continue;
	}
	if(bin >= result->nrecontimebins){
	    continue;
	}
	/*
	** increment the time bin contents for the cluster
	*/
	result->reconvector[bin][clusterid]++;
	count++;
    }
    return(count);
}

SmoothSpikes(fp,result,clusterid)
FILE	*fp;
Result	*result;
int	clusterid;
{
unsigned long timestamp;
float	*vector,sum;
int	count,i,j;

    vector = (float *)malloc(sizeof(float)*result->ntimebins);
    for(i=0;i<result->ntimebins;i++){
	count = 0;
	sum = 0;
	for(j=i-result->smoothspikes;j<i+result->smoothspikes;j++){
	    if((j< 0) || (j>= result->ntimebins)) continue;
	    /*
	    ** increment the time bin contents for the cluster
	    */
	    sum += result->datavector[j][clusterid];
	    count++;
	}
	vector[i] = sum/count;
    }
    /*
    ** load the smoothed result into the data vector
    */
    for(i=0;i<result->ntimebins;i++){
	result->datavector[i][clusterid] = vector[i];
    }
    free(vector);
}

WriteGerstein(fp,result,clusterid)
FILE	*fp;
Result	*result;
int	clusterid;
{
unsigned long timestamp;
float	*vector;
int	bin;
int	count;

    count = 0;
    while(!feof(fp)){
	/*
	** read a timestamp
	*/
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    break;
	}
	/*
	** write it out to the output file with the cluster id
	*/
	fprintf(result->fpout,"%d\t%u\n",clusterid,timestamp);
	count++;
    }
    return(count);
}

WriteCorrelationMatrix(result)
Result	*result;
{
int	i,j;
int	index;

    for(i=0;i<result->nclusters;i++){
	if(result->sortcorr){
	    index = result->corrlist[i].index;
	} else {
	    index = i;
	}
	fwrite(result->corrmatrix[index],sizeof(float),result->nclusters,
	result->fpcorrmatrixout);
    }
}

WriteVectors(result)
Result	*result;
{
int	ival;
float	fval;
int	i,j;
double	sum;
FILE	*fp;

    if(verbose){
	fprintf(stderr,"writing %d clusters, %d timebins\n",
	result->nclusters,result->ntimebins);
    }
    fp = result->fpout;
    if(result->lump){
	for(i=0;i<result->ntimebins;i++){
	    sum = 0;
	    for(j=0;j<result->nclusters;j++){
		sum += result->datavector[i][j];
	    }
	    fprintf(fp,"%g\n",sum/result->nclusters);
	}
    } else {
	if(result->asciiout){
	    /*
	    ** go through each time bin and output the data vector
	    */
	    for(i=0;i<result->ntimebins;i++){
		fprintf(fp,"%lu",(unsigned long)(i*result->binsize +result->tstart));
		for(j=0;j<result->nclusters;j++){
		    fprintf(fp,"\t%6.2g",result->datavector[i][j]);
		}
		fprintf(fp,"\n");
	    }
	} else {
	    /*
	    ** write the binary xview header
	    */
	    ival = result->nclusters-1;
	    fwrite(&ival,sizeof(int),1,fp);
	    /* ival = result->ntimebins-1; */
	    ival = 0;
	    fwrite(&ival,sizeof(int),1,fp);
	    fval = 1;
	    fwrite(&fval,sizeof(float),1,fp);
	    ival = FLOAT;
	    fwrite(&ival,sizeof(int),1,fp);
	    /*
	    ** go through each time bin and output the data vector
	    */
	    for(i=0;i<result->ntimebins;i++){
		fwrite(result->datavector[i],sizeof(float),result->nclusters,fp);
	    }
	}
    }
}

float	**FillGrid(fp,result,clusterid)
FILE	*fp;
Result	*result;
int	clusterid;
{
int	xmax,ymax;
int	datatype;
float	dt;
float	**newgrid;
float	**smoothgrid;
int	i,j;
int	x,y,x2,y2,count;
double	sum;
char	**header;
int	headersize;
int	convert;

    fseek(fp,0L,0L);		/* rewind spike file */
    header = ReadHeader(fp,&headersize);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
    } else {
	convert = 1;
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	    GetFileArchitectureStr(header),
	    GetLocalArchitectureStr());
    }

    /*
    ** read in the xview format information
    */
    if(fread(&xmax,sizeof(int),1,fp) != 1){
	fprintf(stderr,
	"ERROR: unable to read position file for cluster %d\n", clusterid);
	return(NULL);
    }
    if(convert){
	ConvertData(&xmax,sizeof(int));
    }
    if(fread(&ymax,sizeof(int),1,fp) != 1){
	fprintf(stderr,
	"ERROR: unable to read position file for cluster %d\n", clusterid);
	return(NULL);
    }
    if(convert){
	ConvertData(&ymax,sizeof(int));
    }
    if(fread(&dt,sizeof(float),1,fp) != 1){
	fprintf(stderr,
	"ERROR: unable to read position file for cluster %d\n", clusterid);
	return(NULL);
    }
    if(convert){
	ConvertData(&dt,sizeof(float));
    }
    if(fread(&datatype,sizeof(int),1,fp) != 1){
	fprintf(stderr,
	"ERROR: unable to read position file for cluster %d\n", clusterid);
	return(NULL);
    }
    if(convert){
	ConvertData(&datatype,sizeof(int));
    }
    if(datatype != FLOAT){
	fprintf(stderr,
	"ERROR: unable to handle position file of datatype %d\n",
	datatype);
	return(NULL);
    }
    /*
    ** allocate the grid
    */
    result->xmax = xmax;
    result->ymax = ymax;
    /*
    ** if bounds were not explicitly specified then use the
    ** full range
    */
    if(result->xlo == -1){
	result->xlo = 0;
	result->ylo = 0;
	result->xhi = xmax;
	result->yhi = ymax;
    }
    if(result->gxlo == -1){
	result->gxlo = 0;
	result->gylo = 0;
	result->gxhi = xmax;
	result->gyhi = ymax;
    }
    /*
    ** check the specified bounds against the actual bounds
    */
    if(result->xhi > xmax || result->yhi > ymax || result->xlo < 0
    || result->ylo < 0){
	fprintf(stderr,"ERROR: grid dimension mismatch: specified x=(%d %d) y=(%d %d), actual x=(%d %d) y=(%d %d)\n",
	result->xlo,result->xhi,
	result->ylo,result->yhi,
	0,xmax,0,ymax);

	exit(-1);
    }
    result->datatype = datatype;
    newgrid = (float **)malloc((ymax+1)*sizeof(float *));
    for(i=0;i<=ymax;i++){
	newgrid[i] = (float *)malloc((xmax+1)*sizeof(float));
    }
    /*
    ** read the data into the grid
    */
    for(i=0;i<=ymax;i++){
	if(fread(newgrid[i],sizeof(float),xmax+1,fp) != xmax+1){
	    fprintf(stderr,
	    "ERROR: reading data from position file for cluster %d\n",
	    clusterid);
	    return(NULL);
	}
	if(convert){
	    for(j=0;j<=xmax;j++){
		ConvertData(&newgrid[i][j],sizeof(float));
	    }
	}
    }
    if(result->smoothgrid){
	smoothgrid = (float **)malloc((ymax+1)*sizeof(float *));
	for(i=0;i<=ymax;i++){
	    smoothgrid[i] = (float *)malloc((xmax+1)*sizeof(float));
	}
	/*
	** smooth the rate grid
	*/
	for(y=result->ylo;y<=result->yhi;y++){
	    for(x=result->xlo;x<=result->xhi;x++){
		/*
		** smooth the grid by summing adjacent bins
		*/
		sum = 0;
		count = 0;
		for(y2=y-result->smoothgrid;y2<=y+result->smoothgrid;y2++){
		    /*
		    ** dont include points outside the bounds
		    ** in the smoothing
		    */
		    if((y2 < result->ylo) || (y2 > result->yhi)){
			continue;
		    }
		    for(x2=x-result->smoothgrid;x2<=x+result->smoothgrid;x2++){
			/*
			** dont include points outside the bounds
			** in the smoothing
			*/
			if((x2 >= result->xlo) && (x2 <= result->xhi)){
			    sum += newgrid[y2][x2];
			    count++;
			} 
		    }
		}
		/*
		** fill the smoothed matrix
		*/
		if(count > 0){
		    smoothgrid[y][x] = sum/count;
		} else {
		    smoothgrid[y][x] = 0;
		}
	    }
	}
	return(smoothgrid);
    }
    return(newgrid);
}

WriteSparsity(result)
Result	*result;
{
int	y;

    if(verbose){
	fprintf(stderr,"writing sparsity grid\n");
    }
    for(y=0;y<=result->ymax;y++){
	fwrite(result->sparsegrid[y],sizeof(float),result->xmax+1,
	result->fpsparsegridout);
    }
    fclose(result->fpsparsegridout);
}

WritePositionErrorGrid(result)
Result	*result;
{
double	sum;
int	count;
int	x,y;
int	x2,y2;

    /*
    ** if grid position error output is selected then dump it
    */
    if(result->fpperrgrid){
	if(verbose){
	    fprintf(stderr,"writing position error grid\n");
	}
	for(y=result->ylo;y<=result->yhi;y++){
	    for(x=result->xlo;x<=result->xhi;x++){
		/*
		** normalize position error by occupancy
		*/
		if(result->occgrid[y][x] > 0){
		    result->perrgrid[y][x] /= result->occgrid[y][x];
		}
	    }
	}
	/*
	** if smoothing is selected then smooth it
	*/
	if(result->smoothcorr){
	    /*
	    ** initialize the error grid so that unprocessed
	    ** regions are can be distinguised (used by xview display program) 
	    */
	    for(y=0;y<=result->ymax;y++){
		for(x=0;x<=result->xmax;x++){
		    result->smoothperrgrid[y][x] = -999;
		}
	    }
	    /*
	    ** smooth the position error grid
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
				sum += result->perrgrid[y2][x2];
				count++;
			    } 
			}
		    }
		    /*
		    ** fill the smoothed matrix
		    */
		    if(count > 0){
			result->smoothperrgrid[y][x] = sum/count;
		    } else {
			result->smoothperrgrid[y][x] = 0;
		    }
		}
	    }
	    for(y=0;y<=result->ymax;y++){
		fwrite(result->smoothperrgrid[y],sizeof(float),result->xmax+1,
		result->fpperrgrid);
	    }
	} else {
	    for(y=0;y<=result->ymax;y++){
		fwrite(result->perrgrid[y],sizeof(float),result->xmax+1,
		result->fpperrgrid);
	    }
	}
    }
}

WriteGridMagnitude(result)
Result	*result;
{
int	x,y;
float	fval;

    if(verbose){
	fprintf(stderr,"writing magnitude grid\n");
    }
    for(y=0;y<=result->ymax;y++){
	for(x=0;x<=result->xmax;x++){
	    /*
	    ** if an occupancy cutoff has been specified then test for it
	    */
	    if(result->trueoccgrid[y][x] > result->occcutoff){
		fval = result->truemaggrid[y][x];
	    } else {
		fval = -999;
	    }
	    fwrite(&fval,sizeof(float),1, result->fpmagout);
	}
    }
    fclose(result->fpmagout);
}


CommonHeader(fp,result)
FILE	*fp;
Result	*result;
{
int	i;

    fprintf(fp,"%% Version:\t%s\n",VERSION);
    fprintf(fp,"%% Start time:\t%s\n",
	TimestampToString(result->tstart));
    fprintf(fp,"%% End time:\t%s\n",
	TimestampToString(result->tend));
    fprintf(fp,"%% Binsize:\t%u\n",result->binsize);
    fprintf(fp,"%% Ntimebins:\t%u\n",result->ntimebins);
    fprintf(fp,"%% Nx:\t%d\n",result->xmax+1);
    fprintf(fp,"%% Ny:\t%d\n",result->ymax+1);
    fprintf(fp,"%% Nclusters:\t%d\n",result->nactivec);
    for(i=0;i<result->nclusters;i++){
	if(clusterdir[i].ignore) continue;
	if(result->useprefix){
	    fprintf(fp,"%% %d:\t%s/t%d\t%d spikes\n",
	    i,clusterdir[i].dirname,
	    clusterdir[i].clusterid,
	    result->spikecount[i]);
	} else {
	    fprintf(fp,"%% %d:\t%s/%s\t%d spikes\n",
	    i,clusterdir[i].dirname,
	    clusterdir[i].filename,
	    result->spikecount[i]);
	}
    }
    if(result->hasspatialfiring){
	for(i=0;i<result->nclusters;i++){
	    if(result->pdir[i].ignore) continue;
	    fprintf(fp,"%% %d:\t%s\n",i,
		result->pdir[i].path);
	}
    }
}

WriteOutputHeaders(result,argc,argv)
Result	*result;
int	argc;
char	**argv;
{
int	i;
float	fval;
FILE	*fp;
int	datatype;

    /*
    ** write out the standard header to the cluster stats file
    ** file
    */
    if(result->fpout){
	/*
	** write out the standard header
	*/
	fp = result->fpout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Xview");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tPopulation vectors\n");
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the spike train 
    ** file
    */
    if(result->fpspiketrainout){
	/*
	** write out the standard header
	*/
	fp = result->fpspiketrainout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tSpike train\n");
	fprintf(fp,"%% Fields:\t%s\t%s\t%s\t%s\n",
	"time","clusterid","peakx","peaky");
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the population spike train 
    ** file
    */
    if(result->fppoptrainout){
	/*
	** write out the standard header
	*/
	fp = result->fppoptrainout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tPopulation spike train\n");
	fprintf(fp,"%% Fields:\t%s\n",
	"spikes");
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the population spike train 
    ** file
    */
    if(result->fppopvecout){
	/*
	** write out the standard header
	*/
	fp = result->fppopvecout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tPopulation vector\n");
	if(result->hasposition){
	    fprintf(fp,"%% Fields:\t%s\t%s\t%s\n",
	    "posx","posy","spike_rates");
	} else {
	    fprintf(fp,"%% Fields:\t%s\n",
	    "spike_rates");
	}
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the correlation plot
    ** file
    */
    if(result->fpcorrplotout){
	/*
	** write out the standard header
	*/
	fp = result->fpcorrplotout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tCorrelation matrix plot\n");
	fprintf(fp,"%% Fields:\t%s\t%s\t%s\t%s\n",
	"x","y","corr","cluster");
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the population vector
    ** file
    */
    if(result->fpstatout){
	/*
	** write out the standard header
	*/
	fp = result->fpstatout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tCluster stats\n");
	fprintf(fp,"%% Fields:\t%s\t%s\t%s\t%s\n",
	"cluster","rate","peakx","peaky");
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the correlation matrix
    ** file
    */
    if(result->fpcorrmatrixout){
	/*
	** write out the standard header
	*/
	fp = result->fpcorrmatrixout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Xview");
	fprintf(fp,"%% Xsize:\t%d\n",result->nclusters);
	fprintf(fp,"%% Ysize:\t%d\n",result->nclusters);
	fprintf(fp,"%% Fields:\t%s,%d,%d,%d\n",
	    "correlations",
	    FLOAT,sizeof(float),1);
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tCorrelation matrix\n");
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the correlation histogram
    ** file
    */
    if(result->fpcorrhistout){
	fp = result->fpcorrhistout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tCorrelation histogram\n");
	fprintf(fp,"%% Fields: vecid1\tvecid2\tdist\toverlap\tmeanrate\tbias\tskew\tshift");
	if(result->hasparameters){
	    fprintf(fp,"\tparm1\tparm2");
	}
	fprintf(fp,"\tcorrdata\n");
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the 
    ** distance file
    */
    if(result->fpdout){
	fp = result->fpdout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tPosition reconstruction\n");
	fprintf(fp,"%% Fields: timestamp\txest\tyest\tdist\tinterval\n");
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the position reconstruction
    ** file
    */
    if(result->fppout){
	fp = result->fppout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tPosition reconstruction\n");
	fprintf(fp,"%% Fields: timestamp\tmeanrate\tspeed\tsparsity\txest\tyest\t1-corr\tnties\txact\tyact\terror\t\n");
	EndStandardHeader(fp);
    }

    /*
    ** write out the standard header to the field replay output file
    ** file
    */
    if(result->fpspikecorr){
	fp = result->fpspikecorr;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tSpike correlation analysis\n");
	fprintf(fp,"%% Fields:\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
	    "vecid1","vecid2","dist","zerocorr","peakcorr","peakphs","meanphs","overlap",
	    "expectcorr","meanrate","minrate","maxrate","expected","sd","zero");
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the field replay output file
    ** file
    */
    if(result->fpreplayout){
	fp = result->fpreplayout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tSpike spatial field replay\n");
	fprintf(fp,"%% Fields:\t%s\t%s\t%s\t%s\t\n",
	    "x","y","rate","cluster");
	EndStandardHeader(fp);
    }

    /*
    ** write out the standard header to the replay correlation output file
    ** file
    */
    if(result->fpreplaydot){
	fp = result->fpreplaydot;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	if(result->position_bin_avg != 0){
	    fprintf(fp,"%% Analysis type:\tReplay dot product output - avg position\n");
	    fprintf(fp,"%% Fields:\t%s\t%s\t%s\t%s\t\n",
	    "bin","dotprod","netdist","fracdist");
	} else
	if(result->position_bin_search > 0){
	    fprintf(fp,"%% Analysis type:\tReplay dot product output - optimal search\n");
	    fprintf(fp,"%% Fields:\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t\n",
	    "bin","bestlag","dotprod","dist","netdist","fracdist","speed");
	}
	EndStandardHeader(fp);
    }
    /*
    ** write out the standard header to the trajectory tree
    ** file
    */
    if(result->fptreeout){
	fp = result->fptreeout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Analysis type:\tSpike trajectory tree\n");
	switch(result->treeoutputformat){
	case XPLOT:
	    if(result->hasposition && result->hasspatialfiring){
	    fprintf(fp,"%% Fields:\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t\n",
		"node id","ntraverse","nodex","nodey","distance",
		"cumdistance","actualx","actualy","adistance","depth");
	    } else 
	    if(result->hasspatialfiring){
	    fprintf(fp,"%% Fields:\t%s\t%s\t%s\t%s\t%s\t%s\t\n",
		"node id","ntraverse","nodex","nodey","distance",
		"cumdistance","depth");
	    } else {
	    fprintf(fp,"%% Fields:\t%s\t%s\t%s\t\n",
		"node id","ntraverse","depth");
	    }
	}
	EndStandardHeader(fp);
    }

    /*
    ** write out the standard header to the correlation grid output
    ** file
    */
    if(result->fpcorrout){
	fp = result->fpcorrout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Xview");
	CommonHeader(fp,result);
	fprintf(fp,
	    "%% Analysis type:\tPosition reconstruction correlation grid\n");
	EndStandardHeader(fp);
	/*
	** write out the old xview header
	*/
	fwrite(&(result->xmax),sizeof(int),1,fp);
	fwrite(&(result->ymax),sizeof(int),1,fp);
	fval = 1.0;
	fwrite(&fval,sizeof(float),1,fp);
	datatype = FLOAT;
	fwrite(&datatype,sizeof(int),1,fp);
    }
    /*
    ** write out the standard header to the correlation grid output
    ** file
    */
    if(result->fpperrgrid){
	fp = result->fpperrgrid;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Xview");
	CommonHeader(fp,result);
	fprintf(fp,
	    "%% Analysis type:\tPosition reconstruction error grid\n");
	EndStandardHeader(fp);
	/*
	** write out the old xview header
	*/
	fwrite(&(result->xmax),sizeof(int),1,fp);
	fwrite(&(result->ymax),sizeof(int),1,fp);
	fval = 1.0;
	fwrite(&fval,sizeof(float),1,fp);
	datatype = FLOAT;
	fwrite(&datatype,sizeof(int),1,fp);
    }
    /*
    ** write out the standard header to the grid magnitude output
    ** file
    */
    if(result->fpmagout){
	fp = result->fpmagout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Xview");
	CommonHeader(fp,result);
	fprintf(fp,
	    "%% Analysis type:\tPopulation vector magnitude grid\n");
	EndStandardHeader(fp);
	/*
	** write out the old xview header
	*/
	fwrite(&(result->xmax),sizeof(int),1,fp);
	fwrite(&(result->ymax),sizeof(int),1,fp);
	fval = 1.0;
	fwrite(&fval,sizeof(float),1,fp);
	fwrite(&(result->datatype),sizeof(int),1,fp);
    }
    /*
    ** write out the standard header to the grid sparsity output
    ** file
    */
    if(result->fpsparsegridout){
	fp = result->fpsparsegridout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Xview");
	CommonHeader(fp,result);
	fprintf(fp,"%% Sparsity cutoff:\t%g\n",
	    result->sparsity_cutoff);
	fprintf(fp,
	    "%% Analysis type:\tPopulation vector sparsity grid\n");
	EndStandardHeader(fp);
	/*
	** write out the old xview header
	*/
	fwrite(&(result->xmax),sizeof(int),1,fp);
	fwrite(&(result->ymax),sizeof(int),1,fp);
	fval = 1.0;
	fwrite(&fval,sizeof(float),1,fp);
	fwrite(&(result->datatype),sizeof(int),1,fp);
    }
    /*
    ** write out the standard header to the vector sparsity output
    ** file
    */
    if(result->fpsparsevecout){
	fp = result->fpsparsevecout;
	BeginStandardHeader(fp,argc,argv,VERSION);
	fprintf(fp,"%% File type:\t%s\n","Ascii");
	CommonHeader(fp,result);
	fprintf(fp,"%% Sparsity cutoff:\t%g\n",
	    result->sparsity_cutoff);
	fprintf(fp,
	    "%% Analysis type:\tPopulation vector sparsity vs time\n");
	EndStandardHeader(fp);
    }
}

int ReadPositions(result)
Result	*result;
{
int	i;
unsigned long tstart;
unsigned long tend;
int	bin;
FILE	*fp;
unsigned long timestamp;
unsigned long ptimestamp;
short x1;
short y1;
short x2;
short y2;
unsigned char	cval;
int	gx,gy;
int	px1;
int	py1;
int	mx;
int	my;
int	mx2;
int	my2;
float	mspeed;
float	speed;
int	nxy;
int	nxy2;
int	nsp;
int	pbin;
int	count;
int	ninterpol;
int	ibin;
int	headersize;
char	**header;
int	convert;
char	*typestr;
int	p2format;
int	dformat;
int	direction;

    fp = result->fppfile;
    p2format = 0;
    dformat = 0;
    fseek(fp,0L,0L);
    header = ReadHeader(fp,&headersize);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
    } else {
	convert = 1;
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	    GetFileArchitectureStr(header),
	    GetLocalArchitectureStr());
    }
    if((typestr = GetHeaderParameter(header,"Record type:")) != NULL){
	/*
	** look for the D type
	*/
	if(strncmp(typestr,"D",strlen("D")) == 0){
	    dformat = 1;
	} else {
	    dformat = 0;
	}
	/*
	** look for the T type
	*/
	if(strncmp(typestr,"T",strlen("T")) == 0){
	    p2format = 1;
	} else {
	    p2format = 0;
	}
    }
    mx = 0;
    my = 0;
    mx2 = 0;
    my2 = 0;
    mspeed = 0;
    px1 = -1;
    py1 = -1;
    ptimestamp = 0;
    nxy = 0;
    nxy2 = 0;
    nsp = 0;
    pbin = -1;
    count = 0;
    result->trueoccgrid = (float **)malloc((result->ymax+1)*sizeof(float *));
    for(i=0;i<=result->ymax;i++){
	result->trueoccgrid[i] = (float *)calloc((result->xmax+1),sizeof(float));
    }
    /*
    ** compute the number of bins per position interval
    ** note that binsize is in 100 usec intervals
    ** while the position interval is in sec
    */
    ninterpol = (int)(POSITION_INTERVAL*1e4/result->binsize + 0.5);
    while(!feof(fp)){
	/*
	** read in each position and fill the position bins
	*/
	fread(&timestamp,sizeof(unsigned long),1,fp);
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	/*
	** check for the end of the interval
	*/
	if(timestamp > result->tend){
	    break;
	}
	if(p2format){
	    /*
	    ** read in the front and back diode positions
	    */
	    fread(&x1,sizeof(short),1,fp);
	    fread(&y1,sizeof(short),1,fp);
	    fread(&x2,sizeof(short),1,fp);
	    fread(&y2,sizeof(short),1,fp);
	    if(convert){
		ConvertData(&x1,sizeof(short));
		ConvertData(&y1,sizeof(short));
		ConvertData(&x2,sizeof(short));
		ConvertData(&y2,sizeof(short));
	    }
	} else {
	    /*
	    ** front diode position
	    */
	    fread(&cval,sizeof(unsigned char),1,fp);
	    if(convert){
		ConvertData(&cval,sizeof(unsigned char));
	    }
	    x1 = cval;
	    fread(&cval,sizeof(unsigned char),1,fp);
	    if(convert){
		ConvertData(&cval,sizeof(unsigned char));
	    }
	    y1 = cval;
	    /*
	    ** back diode position
	    */
	    fread(&cval,sizeof(unsigned char),1,fp);
	    if(convert){
		ConvertData(&cval,sizeof(unsigned char));
	    }
	    x2 = cval;
	    fread(&cval,sizeof(unsigned char),1,fp);
	    if(convert){
		ConvertData(&cval,sizeof(unsigned char));
	    }
	    y2 = cval;
	    if(dformat){
		/*
		** read in the direction
		*/
		if(fread(&direction, sizeof(int),1,fp) != 1){
		    break;
		}
		if(convert){
		    ConvertData(&direction,sizeof(int));
		}
	    }
	}
	/*
	** check for valid positions
	*/
	if((x1 == 0) && (y1 == 0)){
	    continue;
	}
	/*
	** assign it to a bin
	*/
	bin = (timestamp - result->tstart)/result->binsize;
	/*
	** ignore points outside of the interval
	*/
	if(bin < 0){
	    continue;
	}
	if(bin >= result->ntimebins){
	    continue;
	}
	/*
	** transform the actual coordinate into grid
	** coordinates 
	*/
	gy = y1*(result->ymax+1)/result->yresolution;
	gx = x1*(result->xmax+1)/result->xresolution;
	/*
	** keep track of occupancies
	*/
	result->trueoccgrid[gy][gx]++;
	/*
	** keep a running mean of positions and speed within the bin
	*/
	if(bin == pbin){
	    /*
	    ** mean position
	    */
	    mx += x1;
	    my += y1;
	    nxy++;
	    if(x2 != 0 || y2 != 0){
		mx2 += x2;
		my2 += y2;
		nxy2++;
	    }
	    /*
	    ** mean speed
	    */
	    if(px1 != -1 && (timestamp != ptimestamp)){
		speed = sqrt((double)(((int)x1-px1)*((int)x1-px1) + 
			((int)y1-py1)*((int)y1 - py1)))/
		    ((timestamp - ptimestamp)*1e-4);
		mspeed += speed;
		nsp++;
	    }
	} else {
	    /*
	    ** save the previous bin contents
	    */
	    if(pbin != -1){
		/*
		** if the binsize is below the
		** time resolution of the position info
		** then fill in the missing bins
		*/
		for(ibin = 0;ibin<=ninterpol;ibin++){
		    if(pbin+ibin >= result->ntimebins){
			break;
		    }
		    result->position[pbin+ibin].x = mx/nxy;
		    result->position[pbin+ibin].y = my/nxy;
		    if(nsp>0){
			result->speed[pbin+ibin] = mspeed/nsp;
		    } else {
			result->speed[pbin+ibin] = 0;
		    }
		    if(nxy2 > 0){
			result->position[pbin+ibin].x2 = mx2/nxy2;
			result->position[pbin+ibin].y2 = my2/nxy2;
		    } else {
			result->position[pbin+ibin].x2 = 0;
			result->position[pbin+ibin].y2 = 0;
		    }
		}
		count++;
	    }
	    /*
	    ** reset the bin count and content
	    */
	    mx = x1;
	    my = y1;
	    mspeed = 0;
	    nsp = 0;
	    nxy = 1;
	    if(x2 != 0 || y2 != 0){
		mx2 = x2;
		my2 = y2;
		nxy2 = 1;
	    } else {
		mx2 = 0;
		my2 = 0;
		nxy2 = 0;
	    }
	    pbin = bin;
	}
	/*
	** update the previous state variables
	*/
	ptimestamp = timestamp;
	px1 = x1;
	py1 = y1;
    }
    return(count);
}

int ComputeBayesianDistributions(result)
Result	*result;
{
int	i;
int	j;
unsigned long tstart;
unsigned long tend;
int	bin;
FILE	*fp;
unsigned long timestamp;
unsigned long ptimestamp;
short x1;
short y1;
short x2;
short y2;
unsigned char	cval;
int	gx,gy;
int	px1;
int	py1;
int	mx;
int	my;
int	mx2;
int	my2;
float	mspeed;
float	speed;
int	nxy;
int	nxy2;
int	nsp;
int	pbin;
int	count;
int	ninterpol;
int	ibin;
int	headersize;
char	**header;
int	convert;
char	*typestr;
int	p2format;
int	dformat;
int	direction;

    fp = result->fppfile;
    p2format = 0;
    dformat = 0;
    fseek(fp,0L,0L);
    header = ReadHeader(fp,&headersize);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
    } else {
	convert = 1;
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	    GetFileArchitectureStr(header),
	    GetLocalArchitectureStr());
    }
    if((typestr = GetHeaderParameter(header,"Record type:")) != NULL){
	/*
	** look for the D type
	*/
	if(strncmp(typestr,"D",strlen("D")) == 0){
	    dformat = 1;
	} else {
	    dformat = 0;
	}
	/*
	** look for the T type
	*/
	if(strncmp(typestr,"T",strlen("T")) == 0){
	    p2format = 1;
	} else {
	    p2format = 0;
	}
    }
    mx = 0;
    my = 0;
    mx2 = 0;
    my2 = 0;
    mspeed = 0;
    px1 = -1;
    py1 = -1;
    ptimestamp = 0;
    nxy = 0;
    nxy2 = 0;
    nsp = 0;
    pbin = -1;
    count = 0;
    result->ntotalocc=0;
    /*
    ** allocate grids
    */
    result->zerop = (float *)calloc(result->nclusters,sizeof(float));
    result->nonzerop = (float *)calloc(result->nclusters,sizeof(float));
    result->trueoccgrid = (float **)malloc((result->ymax+1)*sizeof(float *));
    result->occgrid = (float **)malloc((result->ymax+1)*sizeof(float *));
    for(j=0;j<=result->ymax;j++){
	result->trueoccgrid[j] = (float *)calloc((result->xmax+1),sizeof(float));
	result->occgrid[j] = (float *)calloc((result->xmax+1),sizeof(float));
    }
    result->positiongrid = (float ***)malloc(result->nclusters*sizeof(float **));
    result->positiongridzero = (float ***)malloc(result->nclusters*sizeof(float **));
    for(i=0;i<result->nclusters;i++){
	result->positiongrid[i]= (float **)malloc((result->ymax+1)*sizeof(float *));
	result->positiongridzero[i]= (float **)malloc((result->ymax+1)*sizeof(float *));
	for(j=0;j<=result->ymax;j++){
	    result->positiongrid[i][j] = (float *)calloc((result->xmax+1),sizeof(float));
	    result->positiongridzero[i][j] = (float *)calloc((result->xmax+1),sizeof(float));
	}
    }
    /*
    ** compute the number of bins per position interval
    ** note that binsize is in 100 usec intervals
    ** while the position interval is in sec
    */
    while(!feof(fp)){
	/*
	** read in each position and fill the position bins
	*/
	fread(&timestamp,sizeof(unsigned long),1,fp);
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	/*
	** check for the end of the interval
	*/
	if(timestamp > result->tend){
	    break;
	}
	if(p2format){
	    /*
	    ** read in the front and back diode positions
	    */
	    fread(&x1,sizeof(short),1,fp);
	    fread(&y1,sizeof(short),1,fp);
	    fread(&x2,sizeof(short),1,fp);
	    fread(&y2,sizeof(short),1,fp);
	    if(convert){
		ConvertData(&x1,sizeof(short));
		ConvertData(&y1,sizeof(short));
		ConvertData(&x2,sizeof(short));
		ConvertData(&y2,sizeof(short));
	    }
	} else {
	    /*
	    ** front diode position
	    */
	    fread(&cval,sizeof(unsigned char),1,fp);
	    if(convert){
		ConvertData(&cval,sizeof(unsigned char));
	    }
	    x1 = cval;
	    fread(&cval,sizeof(unsigned char),1,fp);
	    if(convert){
		ConvertData(&cval,sizeof(unsigned char));
	    }
	    y1 = cval;
	    /*
	    ** back diode position
	    */
	    fread(&cval,sizeof(unsigned char),1,fp);
	    if(convert){
		ConvertData(&cval,sizeof(unsigned char));
	    }
	    x2 = cval;
	    fread(&cval,sizeof(unsigned char),1,fp);
	    if(convert){
		ConvertData(&cval,sizeof(unsigned char));
	    }
	    y2 = cval;
	    if(dformat){
		/*
		** read in the direction
		*/
		if(fread(&direction, sizeof(int),1,fp) != 1){
		    break;
		}
		if(convert){
		    ConvertData(&direction,sizeof(int));
		}
	    }
	}
	/*
	** check for valid positions
	*/
	if((x1 == 0) && (y1 == 0)){
	    continue;
	}
	/*
	** assign it to a bin
	*/
	bin = (timestamp - result->tstart)/result->binsize;
	/*
	** ignore points outside of the interval
	*/
	if(bin < 0){
	    continue;
	}
	if(bin >= result->ntimebins){
	    continue;
	}
	/*
	** transform the actual coordinate into grid
	** coordinates 
	*/
	gy = y1*(result->ymax+1)/result->yresolution;
	gx = x1*(result->xmax+1)/result->xresolution;
	/*
	** keep track of occupancies
	*/
	result->trueoccgrid[gy][gx]++;
	/*
	** keep track of total position samples
	*/
	result->ntotalocc++;

	/*
	** scan the binned spike trains to determine which cells were active
	** during this positional bin
	** note that the spike binsize must be greater than the position binsize
	*/
	for(i=0;i<result->nclusters;i++){
	    if(result->datavector[bin][i] > result->zerolevel){
		result->positiongrid[i][gy][gx]++;
		result->nonzerop[i]++;
	    } else {
		result->positiongridzero[i][gy][gx]++;
		result->zerop[i]++;
	    }
	}

	/*
	** update the previous state variables
	*/
	ptimestamp = timestamp;
	px1 = x1;
	py1 = y1;
    }
    return(count);
}

ReadPositionDirectoryHeader(result)
Result	*result;
{
char	**header;
int	headersize;
char	*tstring;

    fseek(result->fppdir,0L,0L);
    header = ReadHeader(result->fppdir,&headersize);
    /*
    ** look for the bounding region
    */
    if((result->xlo == -1) && 
    (tstring = GetHeaderParameter(header,"Bounds:")) != NULL){
	/*
	** get the bounds
	*/
	sscanf(tstring,"%d%d%d%d",
	    &result->xlo, &result->ylo, &result->xhi, &result->yhi);
    }
    if(verbose){
	if(result->xlo != -1){
	    fprintf(stderr,"using bounding region %d %d to %d %d\n",
	    result->xlo, result->ylo, result->xhi, result->yhi);
	} else {
	    fprintf(stderr,"using complete region. No bounds specified\n");
	}
    }
    if((result->gxlo == -1) && 
    (tstring = GetHeaderParameter(header,"PosBounds:")) != NULL){
	/*
	** get the bounds
	*/
	sscanf(tstring,"%d%d%d%d",
	    &result->gxlo, &result->gylo, &result->gxhi, &result->gyhi);
    } else {
	result->gxlo = result->xlo;
	result->gxhi = result->xhi;
	result->gylo = result->ylo;
	result->gyhi = result->yhi;
    }
    if(verbose){
	if(result->gxlo != -1){
	    fprintf(stderr,"using position bounding region %d %d to %d %d\n",
	    result->gxlo, result->gylo, result->gxhi, result->gyhi);
	} else {
	    fprintf(stderr,"using complete region. No bounds specified\n");
	}
    }
}

int ParseId(path)
char	*path;
{
int	val;
char	*fptr;
char	*lptr;
char	tmpstr[100];

    strcpy(tmpstr,path);
    /*
    ** first find the first and last numeric character in the string
    */
    fptr = tmpstr;
    while(fptr && *fptr != '\0' && (*fptr < '0' || *fptr > '9')){
	fptr++;
    }
    lptr = fptr;
    while(lptr && *lptr != '\0' && (*lptr >= '0' && *lptr <= '9')){
	lptr++;
    }
    *lptr = '\0';
    if(lptr != fptr){
	val = atoi(fptr);
    } else {
	val = -1;
    }
    return(val);
}

float ParseParameter(path)
char	*path;
{
float	val;
char	*fptr;
char	*lptr;
char	tmpstr[100];

    strcpy(tmpstr,path);
    /*
    ** first find the first whitespace in the string
    */
    fptr = tmpstr;
    while(fptr && *fptr != '\0' && *fptr != ' ' && *fptr != '\t'){
	fptr++;
    }
    val = atof(fptr);
    return(val);
}


ReadParameterDirectory(result)
Result	*result;
{
int	i;
char	line[1001];
char	*ptr;
char	*start;
char	dirname[100];
int	clusterid;
FILE	*fp;

    if(verbose){
	fprintf(stderr,"\nProcessing parameter list\n");
    }
    i = 0;
    fp = result->fpparms;
    /*
    ** read the entries of the directory file
    */
    while(!feof(fp)){
	/*
	** read the line and extract the directory and clusterid information
	*/
	if(fgets(line,1000,fp) == NULL){
	    break;
	}
	if(line[0] == '%') continue;
	if(clusterdir[i].ignore){
	    i++;
	    continue;
	}
	/*
	** skip over the ignore character
	*/
	if(line[0] == '#'){
	    start = line+1;
	} else {
	    start = line;
	}
	/*
	** assume the cluster directory specification is of the 
	** form dir/???n where dir is a string not containing
	** any additional path specifications
	*/
	ptr = start;
	while(strchr(ptr+1,'/') != NULL){
	    ptr = strchr(ptr+1,'/');
	}
	if(*ptr != '/'){
	    fprintf(stderr,
	    "ERROR: unable to parse directory from '%s' in dirfile\n",
	    line);
	    continue;
	}
	*ptr = '\0';
	/*
	** copy the directory portion of the string
	*/
	strcpy(dirname,start);
	/*
	** compare with the existing cluster dir name
	*/
	if(strcmp(clusterdir[i].dirname,dirname) != 0){
	    fprintf(stderr,
	    "ERROR: cluster/parameter directory mismatch '%s' '%s'\n",
	    clusterdir[i].dirname,dirname);
	    exit(-1);
	}
	/*
	** then read the cluster number portion of the string
	** assume the cluster index file specification is of the form
	** ???n where n is the clusterid
	** e.g. cl-1, cl-2, ...
	*/
	clusterid = ParseId(ptr+1);
	clusterdir[i].parameter = ParseParameter(ptr+1);
	/*
	** compare with existing id
	*/
	if(clusterid != clusterdir[i].clusterid){
	    fprintf(stderr,
	    "ERROR: cluster/parameter id mismatch '%s' '%s'\n",
	    clusterdir[i].clusterid,clusterid);
	    exit(-1);
	}
	if(verbose){
	    fprintf(stderr,"%d: Parameters for cluster %d in %s :\t%g\n",
	    i,clusterdir[i].clusterid,clusterdir[i].dirname,
	    clusterdir[i].parameter);
	}
	i++;
    }
    fclose(fp);
}

ReadClusterDirectory(result)
Result	*result;
{
int	i;
char	line[1001];
char	*ptr;
char	*start;

    if(verbose){
	fprintf(stderr,"\nProcessing cluster list\n");
    }
    i = 0;
    /*
    ** read the entries of the directory file
    */
    while(!feof(result->fpdir)){
	/*
	** read the line and extract the directory and clusterid information
	*/
	if(fgets(line,1000,result->fpdir) == NULL){
	    break;
	}
	if(line[0] == '%') continue;
	/*
	** check for the ignore cluster character at the
	** beginning of the line
	*/
	if((line[0] == '#') && (result->useallclusters != 1)){
	    clusterdir[i].ignore = 1;
	} else {
	    /*
	    ** implement fractional inclusion
	    */
	    if((result->fraction > 0) && (frandom(0,1) >= result->fraction)){
		clusterdir[i].ignore = 1;
	    } else {
		clusterdir[i].ignore = 0;
	    }
	}
	if(clusterdir[i].ignore){
	    i++;
	    continue;
	}
	/*
	** skip over the ignore character
	*/
	if(line[0] == '#'){
	    start = line+1;
	} else {
	    start = line;
	}
	/*
	** assume the cluster directory specification is of the 
	** form dir/???n where dir is a string not containing
	** any additional path specifications
	*/
	ptr = start;
	while(strchr(ptr+1,'/') != NULL){
	    ptr = strchr(ptr+1,'/');
	}
	if(*ptr != '/'){
	    fprintf(stderr,
	    "ERROR: unable to parse directory from '%s' in dirfile\n",
	    line);
	    continue;
	}
	*ptr = '\0';
	/*
	** copy the directory portion of the string
	*/
	strcpy(clusterdir[i].dirname,start);
	/*
	** then read the cluster number portion of the string
	** assume the cluster index file specification is of the form
	** ???n where n is the clusterid
	** e.g. cl-1, cl-2, ...
	*/
	clusterdir[i].clusterid = ParseId(ptr+1);
	/*
	** get the filename for use with the noprefix option
	*/
	strcpy(clusterdir[i].filename,ptr+1);
	if((ptr=strchr(clusterdir[i].filename,'\n')) != NULL){
	    *ptr = '\0';
	}
	/*
	if(verbose){
	    fprintf(stderr,"%d: Cluster %d in %s\n",
	    i,clusterdir[i].clusterid,clusterdir[i].dirname);
	}
	*/
	i++;
    }
    fclose(result->fpdir);
    result->nclusters = i;
}

ReadReconClusterDirectoryHeader(result,has_tstart,has_tend)
Result	*result;
int	has_tstart;
int	has_tend;
{
char	**header;
int	headersize;
char	*tstring;

    fseek(result->fpdirrecon,0L,0L);
    header = ReadHeader(result->fpdirrecon,&headersize);
    /*
    ** look for the start and end time parameters in the header
    */
    if(!has_tstart && 
    ((tstring = GetHeaderParameter(header,"Start time:")) != NULL) ||
    ((tstring = GetHeaderParameter(header,"Start Time:")) != NULL)
    ){
	if(verbose){
	    fprintf(stderr,"from recon cluster dir file, tstart=%s\n",tstring);
	}
	/*
	** set the start time
	*/
	result->recontstart = ParseTimestamp(tstring);
    }
    if(!has_tend && 
    ((tstring = GetHeaderParameter(header,"End time:")) != NULL) ||
    ((tstring = GetHeaderParameter(header,"End Time:")) != NULL)){
	if(verbose){
	    fprintf(stderr,"from recon cluster dir file, tend=%s\n",tstring);
	}
	/*
	** set the end time
	*/
	result->recontend = ParseTimestamp(tstring);
    }

    if(verbose){
	fprintf(stderr,"processing recon data from ");
	fprintf(stderr,"%s to ", TimestampToString(result->recontstart));
	fprintf(stderr,"%s\n",TimestampToString(result->recontend));
    }
}

ReadReconClusterDirectory(result)
Result	*result;
{
int	i;
char	line[1001];
char	*ptr;
char	*start;

    if(verbose){
	fprintf(stderr,"\nProcessing recon cluster list\n");
    }
    i = 0;
    /*
    ** read the entries of the directory file
    */
    while(!feof(result->fpdirrecon)){
	/*
	** read the line and extract the directory and clusterid information
	*/
	if(fgets(line,1000,result->fpdirrecon) == NULL){
	    break;
	}
	if(line[0] == '%') continue;
	/*
	** check for the ignore cluster character at the
	** beginning of the line
	*/
	if((line[0] == '#') && (result->useallclusters != 1)){
	    reconclusterdir[i].ignore = 1;
	} else {
	    /*
	    ** implement fractional inclusion
	    */
	    if((result->fraction > 0) && (frandom(0,1) >= result->fraction)){
		reconclusterdir[i].ignore = 1;
	    } else {
		reconclusterdir[i].ignore = 0;
	    }
	}
	if(reconclusterdir[i].ignore){
	    i++;
	    continue;
	}
	/*
	** skip over the ignore character
	*/
	if(line[0] == '#'){
	    start = line+1;
	} else {
	    start = line;
	}
	/*
	** assume the cluster directory specification is of the 
	** form dir/???n where dir is a string not containing
	** any additional path specifications
	*/
	ptr = start;
	while(strchr(ptr+1,'/') != NULL){
	    ptr = strchr(ptr+1,'/');
	}
	if(*ptr != '/'){
	    fprintf(stderr,
	    "ERROR: unable to parse directory from '%s' in dirfile\n",
	    line);
	    continue;
	}
	*ptr = '\0';
	/*
	** copy the directory portion of the string
	*/
	strcpy(reconclusterdir[i].dirname,start);
	/*
	** then read the cluster number portion of the string
	** assume the cluster index file specification is of the form
	** ???n where n is the clusterid
	** e.g. cl-1, cl-2, ...
	*/
	reconclusterdir[i].clusterid = ParseId(ptr+1);
	/*
	** get the filename for use with the noprefix option
	*/
	strcpy(reconclusterdir[i].filename,ptr+1);
	if((ptr=strchr(reconclusterdir[i].filename,'\n')) != NULL){
	    *ptr = '\0';
	}
	/*
	if(verbose){
	    fprintf(stderr,"%d: Cluster %d in %s\n",
	    i,reconclusterdir[i].clusterid,reconclusterdir[i].dirname);
	}
	*/
	i++;
    }
    fclose(result->fpdirrecon);
    result->nreconclusters = i;
}

ReadPositionDirectory(result)
Result	*result;
{
int	i;
char	line[1001];
char	*ptr;
char	*start;

    /*
    ** read the entries of the position directory file
    */
    if(verbose){
	fprintf(stderr,"\nProcessing position list\n");
    }
    i = 0;
    while(!feof(result->fppdir)){
	/*
	** read the line and extract the directory and clusterid information
	*/
	if(fgets(line,1000,result->fppdir) == NULL){
	    break;
	}
	if(line[0] == '%') continue;
	/*
	** blow the CR at the end
	*/
	StripCR(line);
	/*
	** check for the ignore cluster character at the
	** beginning of the line
	*/
	if((line[0] == '#') && (result->useallclusters != 1)){
	    result->pdir[i].ignore = 1;
	} else {
	    /*
	    ** implement fractional inclusion
	    ** by using the fraction established in the 
	    ** cluster list
	    */
	    if(result->fraction > 0){
		result->pdir[i].ignore = clusterdir[i].ignore;
	    } 
	}
	if(result->pdir[i].ignore){
	    i++;
	    continue;
	}
	/*
	** skip over the ignore character
	*/
	if(line[0] == '#'){
	    start = line+1;
	} else {
	    start = line;
	}
	/*
	** copy the path portion of the string
	*/
	strcpy(result->pdir[i].path,start);
	/*
	** assume the cluster directory specification is of the 
	** form dir/?n where dir is a string not containing
	** any additional path specifications
	*/
	ptr = start;
	while(strchr(ptr+1,'/') != NULL){
	    ptr = strchr(ptr+1,'/');
	}
	if(*ptr != '/'){
	    fprintf(stderr,
	    "ERROR: unable to parse directory from '%s' in dirfile\n",
	    line);
	    continue;
	}
	*ptr = '\0';
	/*
	** copy the directory portion of the string
	strcpy(result->pdir[i].dirname,start);
	*/
	/*
	** then read the cluster number portion of the string
	** assume the cluster index file specification is of the form
	** ?n where n is the clusterid
	** e.g. p1, p2, ...
	*/
	result->pdir[i].clusterid = ParseId(ptr+1);
	if(verbose){
	    fprintf(stderr,"%d: Spatial file %s : %d\n",
	    i,
	    result->pdir[i].path,
	    result->pdir[i].clusterid);
	}
	i++;
    }
    fclose(result->fppdir);
    /*
    ** confirm that this is the same as the number of timestamp
    ** clusters
    */
    if(result->nclusters != i){
	fprintf(stderr,
	"ERROR: mismatch in cluster count for timestamps and positions\n");
	exit(-1);
    }
}

ReadClusterDirectoryHeader(result,has_tstart,has_tend)
Result	*result;
int	has_tstart;
int	has_tend;
{
char	**header;
int	headersize;
char	*tstring;

    fseek(result->fpdir,0L,0L);
    header = ReadHeader(result->fpdir,&headersize);
    /*
    ** look for the start and end time parameters in the header
    */
    if(!has_tstart && 
    ((tstring = GetHeaderParameter(header,"Start time:")) != NULL) ||
    ((tstring = GetHeaderParameter(header,"Start Time:")) != NULL)
    ){
	if(verbose){
	    fprintf(stderr,"from cluster dir file, tstart=%s\n",tstring);
	}
	/*
	** set the start time
	*/
	result->tstart = ParseTimestamp(tstring);
    }
    if(!has_tend && 
    ((tstring = GetHeaderParameter(header,"End time:")) != NULL) ||
    ((tstring = GetHeaderParameter(header,"End Time:")) != NULL)){
	if(verbose){
	    fprintf(stderr,"from cluster dir file, tend=%s\n",tstring);
	}
	/*
	** set the end time
	*/
	result->tend = ParseTimestamp(tstring);
    }

    if(verbose){
	fprintf(stderr,"processing data from ");
	fprintf(stderr,"%s to ", TimestampToString(result->tstart));
	fprintf(stderr,"%s\n",TimestampToString(result->tend));
    }
}

WriteClusterStats(result)
Result	*result;
{
int	i;
FILE	*fp;
    
    if(result->fpstatout == NULL) return;
    for(i=0;i<result->nclusters;i++){
	/*
	** write out the firing rate of the cluster
	*/
	fprintf(result->fpstatout,"%d\t%g\t%d\t%d\n",
	i,
	(float)result->spikecount[i]/result->ntimebins,
	result->peakloc[i].x,
	result->peakloc[i].y);
    }
}


ProcessSpikeTrain(result)
Result	*result;
{
int	i,j;
FILE	*fp;
    
    if(result->fpspiketrainout == NULL) return;
    for(i=0;i<result->nclusters;i++){
	for(j=0;j<result->spikecount[i];j++){
	    /*
	    ** write out the firing rate of the cluster
	    */
	    fprintf(result->fpspiketrainout,"%u\t%d\t%d\t%d\n",
	    result->spikearray[i][j],
	    i,
	    result->peakloc[i].x,
	    result->peakloc[i].y);
	}
    }
}


