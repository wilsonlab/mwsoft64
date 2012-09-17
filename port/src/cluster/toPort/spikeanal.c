/*
*******************************************************************
PROGRAM:
    spikeanal

DESCRIPTION:
    Performs various spike train analyses such as normal and log
    interspike interval, auto/cross correlation, rate, 
    spike triggered avg with continous recorded data

AUTHOR:
    Written by Matthew Wilson
    ARL Division of Neural Systems, Memory, and Aging
    University of Arizona
    Tucson, AZ 85724
    wilson@nsma.arizona.edu

DATES:
    original 5/91
    program update 9/92
*******************************************************************
*/

#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <header.h>
#include <iolib.h>


#define VERSION "1.29"

#define MAXBINS	500000
#define MAXRANGES 10000
#define ISIMODE	0
#define CORRMODE 1
#define RATEMODE 2
#define INTERVAL 3
#define STAMODE 4
#define DUMPMODE 5

#define MAXCRDATA	100000

typedef struct range_type {
    unsigned long	stime;	
    unsigned long	etime;	
} Range;

typedef struct result_type {
    long	headersize;
    long	headersize2;
    int		nspikefiles;
    long	nbins;
    double	binsize;		/* binsize in 100 usec */
    int		binmax;
    int		binmin;
    int		analysis_mode;
    int		logtime;
    float	*ratearray;
    float	*corrarray;
    long		nspikes;
    long		nspikes2;
    long		nprocessed;
    long		nprocessed2;
    unsigned long	*spikearray;
    unsigned long	*spikearray2;
    unsigned long	*isiarray;
    int		nranges;
    int		maxranges;
    Range	*range;
    int		outofrange;
    int		cumspikes;
    int		tmax;
    int		sparse;
    char	*fnameout;
    char	*fnamecr;
    char	*fnamein[2];
    FILE	*fpin[2];
    FILE	*fpout;
    FILE	*fpcr;
    int		norm1;
    int		norm2;
    int		suppresszero;
    int		ignoremultiple;
    int		normcorr;
    unsigned long starttime;
    unsigned long endtime;
    int		eegthresh;
    int		edge;
    int		binaryout;
} Result;

int	verbose = 0;
short	crdata[MAXCRDATA];
long	maxbins = MAXBINS; 

int InRange(result,timestamp,edgeoffset)
Result	*result;
unsigned long	timestamp;
unsigned long	edgeoffset;
{
int	inrange;
int	j;

    if(result->nranges == 0) return(1);
    inrange = 0;
    for(j=0;j<result->nranges;j++){
	/*
	** is the spike within the specified range?
	*/
	if((timestamp >= (result->range[j].stime + edgeoffset)) &&
	((result->range[j].etime == 0) ||
	(timestamp <= (result->range[j].etime - edgeoffset)))){
	    inrange = 1;
	    break;
	}
    }
    return(inrange);
}

unsigned long *LoadSpikes(fp,nspikes,size)
FILE	*fp;
int	*nspikes;
long 	*size;
{
unsigned long	*sarray;
unsigned long	timestamp;
int	count;
char	**header;
char	**ReadHeader();
int	convert;

    /*
    ** count the number of spikes
    */
    *nspikes = 0;
    fseek(fp,0L,0L);		/* rewind spike file */
    header = ReadHeader(fp,size);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
	fprintf(stderr,"No data conversion file from %s architecture.\n",
	GetFileArchitectureStr(header));
    } else {
	convert = 1;
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	     GetFileArchitectureStr(header),
	     GetLocalArchitectureStr());
    }

#ifdef OLD
    if(verbose){
	DisplayHeader(stderr,header,*size);
    }
    while(!feof(fp)){		/* scan the entire file */
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    break;
	}
	(*nspikes)++;
    }
#endif
    /*
    ** compute the number of spikes in the file from the file length
    */
    fseek(fp,0L,2);
    *nspikes = (ftell(fp) - *size)/sizeof(unsigned long);
    sarray = (unsigned long *)malloc((*nspikes)*sizeof(unsigned long));
    /*
    ** make the second pass to fill the spike array
    */
    count = 0;
    fseek(fp,*size,0L);		/* rewind spike file */
    while(!feof(fp)){		/* scan the entire file */
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    break;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(count >= *nspikes){
	    fprintf(stderr,"ERROR: incongruity in timestamp count\n"); 
	    break;
	}
	sarray[count] = timestamp;
	count++;
    }
    if(verbose){
	fprintf(stderr,"Loaded %d spikes from %s (%ld)",
	*nspikes,
	TimestampToString(sarray[0]),sarray[0]);
	fprintf(stderr," to %s (%ld)\n",
	TimestampToString(sarray[count-1]),sarray[count-1]);
    }
    return(sarray);
}

void CountProcessedSpikes(result)
Result	*result;
{
int	i;

    result->nprocessed = 0;
    for(i=0;i<result->nspikes;i++){
	if(InRange(result,result->spikearray[i],0)){
	    result->nprocessed++;
	    /*
	    ** check for min and max time ranges
	    */
	    if(result->nprocessed == 0 || 
	    (result->spikearray[i] < result->starttime)){
		result->starttime = result->spikearray[i];
	    }
	    if(result->nprocessed == 0 || 
	    (result->spikearray[i] > result->endtime)){
		result->endtime = result->spikearray[i];
	    }
	}
    }
    result->nprocessed2 = 0;
    for(i=0;i<result->nspikes2;i++){
	if(InRange(result,result->spikearray2[i],0)){
	    result->nprocessed2++;
	    /*
	    ** check for min and max time ranges
	    */
	    if(result->spikearray2[i] < result->starttime){
		result->starttime = result->spikearray2[i];
	    }
	    if(result->spikearray2[i] > result->endtime){
		result->endtime = result->spikearray2[i];
	    }
	}
    }
}

void DumpSpikes(result)
Result	*result;
{
int	i;

    /*
    ** dump the spike array
    */
    for(i=0;i<result->nspikes;i++){
	if(verbose){
	    if((result->nspikes > 100) && (i%(result->nspikes/100) == 0)){
		fprintf(stderr," %3ld%%\b\b\b\b\b",
		(100*i)/(result->nspikes));
	    }
	}
	/*
	** check to make sure the spikes to be analyzed are within
	** a specified timestamp range
	*/
	if(!InRange(result,result->spikearray[i],0)) continue;
	/*
	** output the spike time
	*/
	if(result->binaryout){
	    fwrite(result->spikearray+i,sizeof(unsigned long),1, result->fpout);
	} else {
	    fprintf(result->fpout,"%lu\n",result->spikearray[i]);
	}
    }
}

void Rate(result)
Result	*result;
{
int	bin;
int	i;
int	maxinterval;

    if(result->tmax > 0){
	maxinterval = result->tmax+result->spikearray[0];
    } else {
	maxinterval = result->spikearray[result->nspikes-1] - 
	    result->spikearray[0];
    }
    result->nbins = maxinterval/result->binsize + 1;
    if(result->nbins > maxbins){
	result->nbins = maxbins;
    }
    if(result->nbins <= 0){
	result->nbins = 0;
	fprintf(stderr,
	"Warning: nothing to bin. nbins = %ld\n",
	result->nbins);
	return;
    }
    result->ratearray = (float *)calloc(result->nbins,sizeof(float));
    /*
    ** fill the rate array
    */
    for(i=0;i<result->nspikes;i++){
	if(verbose){
	    if((result->nspikes > 100) && (i%(result->nspikes/100) == 0)){
		fprintf(stderr," %3ld%%\b\b\b\b\b",
		(100*i)/(result->nspikes));
	    }
	}
	/*
	** check to make sure the spikes to be analyzed are within
	** a specified timestamp range
	*/
	if(!InRange(result,result->spikearray[i],0)) continue;
	/*
	** test for output bound range
	*/
	if((result->spikearray[i] - result->spikearray[0]) > maxinterval) break;
	/*
	** assign the bin based on the time from beginning of the data set
	*/
	bin = (result->spikearray[i] - result->spikearray[0])/result->binsize;
	if(bin < 0){
	    fprintf(stderr,"ERROR: invalid spike at timestamp %lu\n",
		result->spikearray[i]);
	    continue;
	}
	if(bin >= result->nbins){
	    result->outofrange++;
	    continue;
	}
	if(result->logtime){
	    if(i>0){
		result->ratearray[bin]+=
		    1.0/log10((double)(result->spikearray[i] -
		    result->spikearray[i-1]));
	    }
	} else {
	    result->ratearray[bin]++;
	}
    }
}

void Interval(result)
Result	*result;
{
int	i;
double	interval;

    for(i=0;i<result->nspikes;i++){
	if(verbose){
	    if((result->nspikes > 100) && (i%(result->nspikes/100) == 0)){
		fprintf(stderr," %3ld%%\b\b\b\b\b",
		(100*i)/(result->nspikes));
	    }
	}
	/*
	** check to make sure the spikes to be analyzed are within
	** a specified timestamp range
	*/
	if(!InRange(result,result->spikearray[i],0)) continue;
	if(i>0){
	    if(result->logtime){
		interval = 1.0/log10((double)(result->spikearray[i] -
		    result->spikearray[i-1]));
	    } else {
		interval = 1.0/(double)(result->spikearray[i] -
		    result->spikearray[i-1]);
	    }
	    fprintf(result->fpout,"%g\t%g\n",
		result->spikearray[i]*1e-4,interval);
	}
    }
}

void Correlate(result)
Result	*result;
{
int 	i,j;
double 	time1;
int	zbin;
register float	*ca;
register double	interval;
register int	bin;
double	fbin;
int	split;
int	maxinterval;
int	start;
int	firsttime;
unsigned long	maxtime;
unsigned long	mintime;
unsigned long 	toffset;

    /*
    ** calculate the number of bins based on the binsize
    ** and the max time
    */
    if(result->spikearray[result->nspikes-1] > 
    result->spikearray2[result->nspikes2 -1]){
	maxtime = result->spikearray[result->nspikes-1];
    } else {
	maxtime = result->spikearray2[result->nspikes2-1];
    }
    if(result->spikearray[0] > result->spikearray2[0]){
	mintime = result->spikearray[0];
    } else {
	mintime = result->spikearray2[0];
    }

    if(result->tmax > 0){
	maxinterval = result->tmax;
    } else {
	maxinterval = maxtime - mintime;
    }
    result->nbins = 2*(int)(0.5 + (maxinterval/result->binsize)) + 1;
    if(result->nbins > maxbins){
	result->nbins = maxbins;
    }
    /*
    ** locate the zero time bin
    */
    /* zbin = result->nbins/2+1; */
    zbin = result->nbins/2;


    /*
    ** clear the correlation array
    */
    if((result->corrarray = (float *)calloc(result->nbins,sizeof(float))) == NULL){
	fprintf(stderr,"ERROR: unable to allocate %ld bins\n",result->nbins);
	exit(-1);
    }

    /*
    ** go through each time in 1 and correlate with times in 2
    */
    start = 0;
    if(result->edge){
	toffset = result->tmax;
    } else {
	toffset = 0;
    }
    for(i=0;i<result->nspikes;i++){
	/*
	** check to make sure the spikes to be analyzed are within
	** a specified timestamp range. 
	** If edge treatment is specified then dont process spikes
	** at the beginning and end of the data
	*/
	if(!InRange(result,result->spikearray[i],toffset)) continue;
	time1 = result->spikearray[i];
	ca = result->corrarray;
	if(verbose){
	    if((result->nspikes > 100) && (i%(result->nspikes/100) == 0)){
		fprintf(stderr," %3ld%%\b\b\b\b\b",
		(100*i)/(result->nspikes));
	    }
	}
	firsttime = 1;
	for(j=start;j<result->nspikes2;j++){
	    /*
	    ** check to make sure the spikes to be analyzed are within
	    ** a specified timestamp range
	    */
	    if(!InRange(result,result->spikearray2[j],0)) continue;

	    /*
	    ** calculate the interval between time1 and this time
	    */
	    interval = result->spikearray2[j] - time1;
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
	    if(result->suppresszero && (interval == 0)){
		/*
		** skip the zero offset correlations
		*/
		continue;
	    }
	    /*
	    ** which bin?
	    ** things which fall on bin boundaries get
	    ** split between them
	    */
	    fbin = interval/result->binsize;
	    split = 0;
	    if(interval >= 0){
		if((fbin - (int)fbin) == 0.5){
		    /*
		    ** randomly assign it to one side or the other
		    */
		    if(frandom(0,1) > 0.5){
			split = 0.25;
		    } else {
			split = -0.25;
		    }
		} else {
		    split = 0;
		}
		bin = zbin + (int)(fbin + 0.5 + split);
	    } else {
		if((fbin - (int)fbin) == -0.5){
		    /*
		    ** randomly assign it to one side or the other
		    */
		    if(frandom(0,1) > 0.5){
			split = 0.25;
		    } else {
			split = -0.25;
		    }
		} else {
		    split = 0;
		}
		bin = zbin + (int)(fbin - 0.5 + split);
	    }
	    if(bin < 0 || bin >= result->nbins) continue;
	    /*
	    ** increment the count in that bin
	    */
	    ca[bin]++;
	}
    }
}

void ISI(result)
Result	*result;
{
unsigned long	timestamp;
unsigned long	ptimestamp;
int	bin;
int	firstflag;
int	min_isi = INT_MIN;
int	max_isi = INT_MAX;
int	i;

    /*
    ** get the min and max isi values
    */
 if(result->nspikes > 0){  /* any spikes in ttfile? */
    ptimestamp = 0;
    firstflag = 1;
    for(i=0;i<result->nspikes;i++){
	/*
	** check to make sure the spikes to be analyzed are within
	** a specified timestamp range
	*/
	if(!InRange(result,result->spikearray[i],0)){
	    /*
	    ** reset the previous timestamp variable since it is no
	    ** longer valid following a skipped spike
	    */
	    ptimestamp = 0;
	    continue;
	}
	timestamp = result->spikearray[i];
	/*
	** keep track of the min/max interspike interval
	*/
	if(ptimestamp == 0){
	    ptimestamp = timestamp;
	    continue;
	} 
	if(firstflag){
	    min_isi = timestamp - ptimestamp;
	    max_isi = timestamp - ptimestamp;
	    firstflag = 0;
	} else {
	    if(timestamp - ptimestamp < min_isi){
		min_isi = timestamp - ptimestamp;
	    }
	    if(timestamp - ptimestamp > max_isi){
		max_isi = timestamp - ptimestamp;
	    }
	}
	ptimestamp = timestamp;
    }
    /*
    ** check the bounds
    */
    if(!result->logtime && (min_isi < 0)){
	fprintf(stderr,"Warning: invalid interspike interval found (%d)\n",
	min_isi);
    }
 } else{   /* No spikes in ttfile */
   min_isi = 0;
   max_isi = 0;
 }
    /*
    ** allocate the isi array based on the tmax and the desired
    ** bin size
    */
 if(result->logtime){
   result->binmax = log10((double)max_isi/10.0)/(result->binsize/10.0);
   result->binmin = log10((double)min_isi/10.0)/(result->binsize/10.0);
   if(result->binmin > 0){
     /*
     ** put in the origin even if the min bin isnt 0
     */
     result->binmin = 0;
   }
 } else {
   result->binmax = max_isi/(result->binsize);
   result->binmin = 0;
 }
 result->nbins = result->binmax - result->binmin + 1;
 /*
 ** lets be reasonable now
 */
 if(result->nbins > maxbins){
   result->nbins = maxbins;
   fprintf(stderr,
	   "Warning: exceeded maximum number of bins (%ld). Using %ld instead\n",
	   maxbins,result->nbins);
 }
 if(result->nbins <= 0){
   result->nbins = 0;
   fprintf(stderr,
	   "Warning: nothing to bin. nbins = %ld\n",
	   result->nbins);
	return;
    }
    /*
    ** allocate the interval array
    */
    result->isiarray = (unsigned long *)calloc(result->nbins,
	sizeof(unsigned long));
    /*
    ** make the second pass to fill the isi array
    */
    ptimestamp = 0;
    for(i=0;i<result->nspikes;i++){
	if(verbose){
	    if((result->nspikes > 100) && (i%(result->nspikes/100) == 0)){
		fprintf(stderr," %3ld%%\b\b\b\b\b",
		(100*i)/(result->nspikes));
	    }
	}
	/*
	** check to make sure the spikes to be analyzed are within
	** a specified timestamp range
	*/
	if(!InRange(result,result->spikearray[i],0)){
	    ptimestamp = 0;
	    continue;
	}
	timestamp = result->spikearray[i];
	if(ptimestamp == 0){
	    ptimestamp = timestamp;
	    continue;
	} 
	if(result->logtime){
	    bin = log10((double)(timestamp - ptimestamp)/10.0)/
		(result->binsize/10.0) + result->binmin;
	} else{
	    bin = (timestamp - ptimestamp)/result->binsize + result->binmin;
	}
	ptimestamp = timestamp;

	if(!result->logtime && bin < 0){
	    fprintf(stderr,"ERROR: invalid isi  of %d\n",bin);
	    continue;
	}
	if(bin >= result->nbins){
	    result->outofrange++;
	    continue;
	}
	result->isiarray[bin]++;
    }
}

int WriteRate(result)
Result	*result;
{
int	i;
int	first = 1;
double	totalspikes;
double  totaltime;
double	sumsqr;
double	totalbins;
char	line[100];
char	headerappend[1000];
struct stat buf;

    /*
    ** output the overall mean rate
    */
    /*
    ** if input ranges were specified then use them to compute the
    ** time over which rate is to be averaged
    */
    totalspikes = 0;
    sumsqr = 0;
    for(i=0;i<result->nbins;i++){
	/*
	** sum all spikes
	*/
	totalspikes += result->ratearray[i];
	sumsqr += result->ratearray[i]*result->ratearray[i];
    }
    /*
    ** normalize to spikes^2/sec^2
    */
    sumsqr /= result->binsize*result->binsize*1e-8;
    /*
    ** compute total time
    */
    if(result->nranges > 0){
	totaltime = 0;
	for(i=0;i<result->nranges;i++){
	    /*
	    ** 
	    */
	    if(result->range[i].etime > 0){
		totaltime = 1e-4*result->nbins*result->binsize
		    - result->range[i].stime;
	    } else {
		totaltime += result->range[i].etime - result->range[i].stime;
	    }
	}
	/*
	** convert to sec
	*/
	totaltime *= 1e-4;
    } else {
	/*
	** total time in sec
	*/
	totaltime = 1e-4*result->nbins*result->binsize;
    }
    totalbins = 1e4*totaltime/result->binsize;
    /*
    ** output overall rate statistics
    */
    sprintf(headerappend,
	"%% Mean rate (spikes/sec): \t%g\n",
	totalspikes/totaltime);
    sprintf(line,
	"%% Standard error of the mean rate (spikes/sec): \t%g\n",
	sqrt(
	    (sumsqr - 
	    (totalspikes/totaltime)*(totalspikes/totaltime)/totalbins)/
	    (totalbins*(totalbins -1))
	)
    );
    strcat(headerappend,line);
    sprintf(line,
	"%% Standard deviation of the mean rate (spikes/sec): \t%g\n",
	sqrt(
	    (sumsqr - 
	    (totalspikes/totaltime)*(totalspikes/totaltime)/totalbins)/
	    (totalbins -1)
	)
    );
    strcat(headerappend,line);
    /*
    ** check the file mode to see whether it is seekable
    */
    fstat(fileno(result->fpout),&buf);
    if(S_ISFIFO(buf.st_mode)){	/* is this a pipe */
	fprintf(result->fpout,"%s",headerappend);
    } else {
	/*
	** insert the data into the header
	*/
	fclose(result->fpout);
	/*
	** add the new information to the header
	*/
	if(!AppendToHeader(result->fnameout,headerappend)){
	    fprintf(stderr,"ERROR: Error appending to file header\n");
	}
	if((result->fpout = fopen(result->fnameout,"a")) == NULL){
	    fprintf(stderr,"ERROR: unable to reopen output file '%s'\n",
	    result->fnameout);
	    return(0);
	}
	fseek(result->fpout,0,2);	/* go to end of file */
    }
    /*
    ** output rate per bin
    */
    for(i=0;i<result->nbins;i++){
	/*
	** output in spikes/sec vs sec
	*/
	if((result->tmax > 0) && (i*result->binsize > result->tmax)) break;
	if(result->sparse && result->ratearray[i] == 0){
	    if(first){
		/*
		** put in one zero. Allows proper autoscaling later on
		*/
		first = 0;
	    } else
		continue;
	}
	fprintf(result->fpout,"%g %g\n",
	1e-4*(i*result->binsize + result->spikearray[0]),
	10000*(float)result->ratearray[i]/result->binsize);
    }
    return(1);
}

void WriteISI(result)
Result	*result;
{
int	i;
int	first = 1;

    for(i=0;i<result->nbins;i++){
	/*
	** count vs msec
	*/
	if((result->tmax > 0) && (i*result->binsize > result->tmax)) break;
	if(result->sparse && result->isiarray[i] == 0){ 
	    if(first){
		/*
		** put in one zero. Allows proper autoscaling later on
		*/
		first = 0;
	    } else
		continue;
	}
	if(result->logtime){
	    fprintf(result->fpout,
	    "%g %ld\n", 
	    pow(10.0,0.1*(i-result->binmin)*result->binsize), result->isiarray[i]);
	} else {
	    fprintf(result->fpout,
	    "%g %ld\n", 
	    0.1*(i-result->binmin)*result->binsize, result->isiarray[i]);
	}
    }
}

void WriteCorr(result)
Result	*result;
{
int	i;
int	first = 1;
int	norm;
double	expected;
double	offset;
int	ntimebins = 0;

    norm = 1;

    if(result->norm1){
	norm *= result->nprocessed;
    }
    if(result->norm2){
	norm *= result->nprocessed2;
    }
    if(result->normcorr){
	ntimebins = (result->endtime - result->starttime)/result->binsize;
	offset = (float)result->nprocessed*result->nprocessed2/ntimebins;
    } else {
	offset = 0;
    }
    /*
    ** dont normalize by zero
    */
    if(norm == 0){
	norm = 1;
    }
    for(i=0;i<result->nbins;i++){
	/*
	** count vs msec
	*/
	if(result->sparse && result->corrarray[i] == 0){
	    if(first){
		/*
		** put in one zero. Allows proper autoscaling later on
		*/
		first = 0;
	    } else
		continue;
	}
	if(result->normcorr){
	    /*
	    ** compute the correction factor for edge effects
	    */
	    expected = offset*(ntimebins - abs(i - (result->nbins - 1)/2))/
	    ntimebins;
	} else {
	    expected = 0;
	}
	fprintf(result->fpout,"%g %g\n",
	0.1*result->binsize*(i-(result->nbins-1)/2),
	(result->corrarray[i] - expected)/norm);
    }
}

void ReadRange(fp,result)
FILE	*fp;
Result  *result;
{
long     size;
char    **header;
char	*filetype;
int	binaryformat;
char	line[201];
char	ef0[10];
char	ef1[10];
char	startstr[30];
char	endstr[30];
unsigned long tstart;
unsigned long tend;
int	convert;

    fseek(fp,0L,0L);            /* rewind range file */
    /*
    ** read in the header
    */
    header = ReadHeader(fp,&size);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
	fprintf(stderr,"No data conversion file from %s architecture.\n",
	GetFileArchitectureStr(header));
    } else {
	convert = 1;
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	     GetFileArchitectureStr(header),
	     GetLocalArchitectureStr());
    }
    /*
    ** is this binary or ascii format
    */
    if((filetype = GetHeaderParameter(header,"File type:")) != NULL){
	/*
	** look for the binary
	*/
	if(strncmp(filetype,"Binary",strlen("Binary")) == 0){
	    binaryformat = 1;
	} else {
	    binaryformat = 0;
	}
    } else {
	binaryformat = 0;
    }
    while(!feof(fp)){           /* scan the entire file */
	if(binaryformat){
	    /*
	    ** read the starting timestamp
	    */
	    if(fread(&tstart,sizeof(unsigned long),1,fp) != 1){
		break;
	    }
	    /*
	    ** read the ending timestamp
	    */
	    if(fread(&tend,sizeof(unsigned long),1,fp) != 1){
		break;
	    }
	    if(convert){
		ConvertData(&tstart,sizeof(unsigned long));
		ConvertData(&tend,sizeof(unsigned long));
	    }
	} else {
	    if(fgets(line,200,fp) == NULL){
		break;
	    }
	    /*
	    ** ignore comments and commands
	    */
	    if(line[0] == '%') continue;
	    if(line[0] == '/') continue;
	    /*
	    ** parse the line for timestamps
	    */
	    if(sscanf(line,"%s%s%s%s",ef0,ef1,startstr,endstr) != 4) continue;
	    tstart = ParseTimestamp(startstr);
	    tend = ParseTimestamp(endstr);
	    if(strncmp(endstr,"end",3) == 0){
		tend = 0;
	    }
	}
	result->range[result->nranges].stime = tstart;
	result->range[result->nranges].etime = tend;
        /*
        ** increment range count
        */
        result->nranges++;
    }
}

void ReadWindow(fp,result,windowmin,windowmax)
FILE	*fp;
Result  *result;
unsigned long windowmin;
unsigned long windowmax;
{
long     size;
char    **header;
unsigned long   timestamp;
int	convert;

    fseek(fp,0L,0L);            /* rewind timestamp file */
    /*
    ** read in the header
    */
    header = ReadHeader(fp,&size);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
	fprintf(stderr,"No data conversion file from %s architecture.\n",
	GetFileArchitectureStr(header));
    } else {
	convert = 1;
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	     GetFileArchitectureStr(header),
	     GetLocalArchitectureStr());
    }
    while(!feof(fp)){           /* scan the entire file */
        /*
        ** read the timestamp
        */
        if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
            break;
        }
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
        result->range[result->nranges].stime = timestamp - windowmin;
        result->range[result->nranges].etime = timestamp + windowmax;
        /*
        ** increment range count
        */
        result->nranges++;
    }
}

void DeleteSpike(spikearray,index,nspikes)
unsigned long *spikearray;
int	index;
int	nspikes;
{
int	i;

    /*
    ** shift the spikes down
    */
    for(i=index;i<nspikes-1;i++){
	spikearray[i] = spikearray[i+1];
    }
}


/*
** TestSpikes returns the number of improper spike times
*/
int TestSpikes(result,spikearray,nspikes)
Result	*result;
unsigned long *spikearray;
int	nspikes;
{
int	i;
int	count;

    if(result->ignoremultiple) return(0);
    count = 0;
    for(i=0;i<nspikes-1;i++){
	/*
	** test for identical timestamps
	*/
	if(spikearray[i] == spikearray[i+1]){
	    fprintf(stderr,
	    "\tmultiple timestamp %lu for spike %d\n",spikearray[i],i);
	    count++;
	    /*
	    ** delete the offending spike
	    */
	    DeleteSpike(spikearray,i,nspikes);
	    nspikes--;
	}
    }
    return(count);
}


int intcompare(i1,i2)
unsigned long	*i1,*i2;
{
    return(*i1 - *i2);
}

void PrepareSpikeArrays(result)
Result	*result;
{
int	count;

    if(result->nspikefiles > 1){
	result->spikearray = LoadSpikes(result->fpin[0],&result->nspikes,
	    &result->headersize);
	qsort(result->spikearray,result->nspikes,sizeof(unsigned long),
	    intcompare);
	/*
	** test the spike array for proper form
	*/
	if((count = TestSpikes(result,result->spikearray,result->nspikes)) != 0){
	    fprintf(stderr,
	    "ERROR: redundant spike times in timestamp file '%s'\n",
	    result->fnamein[0]);
	    result->nspikes -= count;
	}
	result->spikearray2 = LoadSpikes(result->fpin[1],&result->nspikes2,
	    &result->headersize2);
	qsort(result->spikearray2,result->nspikes2,sizeof(unsigned long),
	    intcompare);
	/*
	** test the spike array for proper form
	*/
	if((count = TestSpikes(result,result->spikearray2,result->nspikes2)) != 0){
	    fprintf(stderr,
	    "ERROR: redundant spike times in timestamp file '%s'\n",
	    result->fnamein[1]);
	    result->nspikes2 -= count;
	}
    } else {
	result->spikearray = LoadSpikes(result->fpin[0],&result->nspikes,
	    &result->headersize);
	qsort(result->spikearray,result->nspikes,sizeof(unsigned long),
	    intcompare);
	/*
	** test the spike array for proper form
	*/
	if((count = TestSpikes(result,result->spikearray,result->nspikes)) != 0){
	    fprintf(stderr,
	    "ERROR: redundant spike times in timestamp file '%s'\n",
	    result->fnamein[0]);
	    result->nspikes -= count;
	}
	result->spikearray2 = result->spikearray;
	result->nspikes2 = result->nspikes;
	result->headersize2 = result->headersize;
    }
}

void WriteOutputHeader(result,argc,argv)
int	argc;
char	**argv;
Result	*result;
{
int	i;

    /*
    ** write the output file header
    */
    BeginStandardHeader(result->fpout,argc,argv,VERSION);

    switch(result->analysis_mode){
    case STAMODE:
	fprintf(result->fpout,"%% Analysis: \t%s\n","STA");
	break;
    case ISIMODE:
	fprintf(result->fpout,"%% Analysis: \t%s\n","ISI");
	break;
    case CORRMODE:
	fprintf(result->fpout,"%% Analysis: \t%s\n","Cross correlation");
	break;
    case RATEMODE:
	fprintf(result->fpout,"%% Analysis: \t%s\n","Rate");
	break;
    case INTERVAL:
	fprintf(result->fpout,"%% Analysis: \t%s\n","Interval");
	break;
    case DUMPMODE:
    	fprintf(result->fpout,"%% Analysis: \t%s\n","Timestamp");
    	fprintf(result->fpout,"%% File type: \t%s\n","Binary");
	fprintf(result->fpout,"%% Fields: \t%s,%d,%d,%d\n",
	"timestamp",ULONG,sizeof(unsigned long),1);
	break;
    }
    fprintf(result->fpout,"%% Input file 1: \t%s\n",result->fnamein[0]);
    if(result->nspikefiles > 1){
	fprintf(result->fpout,"%% Input file 2: \t%s\n",result->fnamein[1]);
    }
    fprintf(result->fpout,"%% Output file: \t%s\n",result->fnameout);
    fprintf(result->fpout,"%% Binsize: \t%g msec\n",result->binsize/10.0);
    fprintf(result->fpout,"%% Tmax: \t%g msec\n",result->tmax/10.0);
    fprintf(result->fpout,"%% Logtime: \t%s\n",TFstr(result->logtime));
    fprintf(result->fpout,"%% Norm1: \t%s\n",TFstr(result->norm1));
    fprintf(result->fpout,"%% Norm2: \t%s\n",TFstr(result->norm2));
    /*
    ** report range limits placed on the input data 
    */
    if(result->nranges > 0){
	fprintf(result->fpout,"%% Input data timestamp range restrictions:\n");
	for(i=0;i<result->nranges;i++){
	    fprintf(result->fpout,"%% \t%s (%lu) to",
	    TimestampToString(result->range[i].stime),result->range[i].stime);
	    fprintf(result->fpout," %s (%lu)\n",
	    TimestampToString(result->range[i].etime),result->range[i].etime);
	}
    }
    EndStandardHeader(result->fpout);
}


void STA(result)
Result	*result;
{
int 	i,j;
double 	time1;
int	zbin;
register float	*ca = NULL;
register double	interval;
register int	bin;
double	fbin;
int	split;
int	maxinterval;
int	start;
int	firsttime;
unsigned long	startstamp;
unsigned short	nsamples;
unsigned long	freq;
double 	dt;
char	**header;
long	size;
int	*ncorr;
int	convert;
int	cdat;


    ncorr = NULL;
    maxinterval = result->tmax;
    result->nbins = 2*(int)(0.5 + (maxinterval/result->binsize)) + 1;
    if(result->nbins > maxbins){
	result->nbins = maxbins;
    }
    /*
    ** locate the zero time bin
    */
    /* zbin = result->nbins/2+1; */
    zbin = result->nbins/2;

    /*
    ** clear the correlation array
    */
    if((result->corrarray = (float *)calloc(result->nbins,sizeof(float))) == NULL){
	fprintf(stderr,"ERROR: unable to allocate %ld bins\n",result->nbins);
	exit(-1);
    }
    ncorr = (int *)calloc(result->nbins,sizeof(int));

    /*
    ** read the cr file header
    */
    header = ReadHeader(result->fpcr,&size);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
        (GetFileArchitecture(header) == ARCH_UNKNOWN)) {
        convert = 0;
        fprintf(stderr,"No data conversion file from %s architecture.\n"
,
                GetFileArchitectureStr(header));
    } else {
        convert = 1;
        fprintf(stderr,"Converting data from %s to %s architectures.\n",
            GetFileArchitectureStr(header),
            GetLocalArchitectureStr());
    }
    /*
    ** get sampling rate and buffer size information from the header
    */


    /*
    ** go through the cr data file and correlate with spike times 
    */
    start = 0;
    while(!feof(result->fpcr)){
	/*
	** read in data from the cr file
	*/
	if(fread(&startstamp,sizeof(unsigned long),1,result->fpcr) != 1) break;
	if(fread(&nsamples,sizeof(unsigned short),1,result->fpcr) != 1) break;
	if(fread(&freq,sizeof(unsigned long),1,result->fpcr) != 1) break;
	if(fread(crdata,sizeof(short),nsamples,result->fpcr) != nsamples) break;
	if(convert){
	    ConvertData(&startstamp,sizeof(unsigned long));
	    ConvertData(&nsamples,sizeof(unsigned short));
	    ConvertData(&freq,sizeof(unsigned long));
	    for(i=0;i<nsamples;i++){
		ConvertData(crdata+i,sizeof(short));
	    }
	}
	if(verbose){
	    /*fprintf(stderr,"%10lu %6d %6d\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",startstamp,nsamples,freq);*/
	    fprintf(stderr,"%10lu %6d %6ld\n",startstamp,nsamples,freq);
	}
	dt = 1e4/freq;
	/*
	** check the eeg for amplitude criteria
	*/
	for(i=0;i<nsamples;i++){
	    cdat = crdata[i] - 2048;
	    if(cdat > result->eegthresh || cdat < -result->eegthresh){
		break;
	    }
	}
	if(i < nsamples) continue;
	/*
	** process the data points
	*/
	for(i=0;i<nsamples;i++){
	    /*
	    ** get the timestamp of the cr data point
	    */
	    time1 = startstamp + i*dt;
	    /*
	    ** check to make sure the data point to be analyzed is within
	    ** the specified timestamp range
	    */
	    if(!InRange(result,(unsigned long)time1,0)) continue;
	    ca = result->corrarray;
	    firsttime = 1;
	    for(j=start;j<result->nspikes;j++){
		/*
		** check to make sure the spikes to be analyzed are within
		** a specified timestamp range
		*/
		if(!InRange(result,result->spikearray[j],0)) continue;

		/*
		** calculate the interval between cr timestamp and this time
		*/
		interval = time1 - result->spikearray[j];
		if(interval > maxinterval){
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
		if(interval < -maxinterval) break; 
		if(result->suppresszero && (interval == 0)){
		    /*
		    ** skip the zero offset correlations
		    */
		    continue;
		}
		/*
		** which bin?
		** things which fall on bin boundaries get
		** split between them
		*/
		fbin = interval/result->binsize;
		split = 0;
		if(interval >= 0){
		    if((fbin - (int)fbin) == 0.5){
			/*
			** randomly assign it to one side or the other
			*/
			if(frandom(0,1) > 0.5){
			    split = 0.25;
			} else {
			    split = -0.25;
			}
		    } else {
			split = 0;
		    }
		    bin = zbin + (int)(fbin + 0.5 + split);
		} else {
		    if((fbin - (int)fbin) == -0.5){
			/*
			** randomly assign it to one side or the other
			*/
			if(frandom(0,1) > 0.5){
			    split = 0.25;
			} else {
			    split = -0.25;
			}
		    } else {
			split = 0;
		    }
		    bin = zbin + (int)(fbin - 0.5 + split);
		}
		if(bin < 0 || bin >= result->nbins) continue;
		/*
		** add the cr data to that bin
		*/
		ca[bin] += crdata[i];
		ncorr[bin]++;
	    }
	}
    }
    if(ncorr){
	free(ncorr);
    }
    /*
    ** normalize the corr
    */
    for(i=0;i<result->nbins;i++){
	if(ncorr[i] > 0){
	    ca[i] /= ncorr[i];
	}
    }
    if(verbose){
	fprintf(stderr,"\n");
    }
}


int CheckSettings(result)
Result	*result;
{
    /*
    ** check for spike time files
    */
    if(result->nspikefiles == 0){
	fprintf(stderr,"ERROR: Must specify a time file\n");
	return(0);
    }

    /*
    ** check for cr data file if spike triggered averaging is selected
    */
    if(result->analysis_mode == STAMODE){
	if(result->fpcr == NULL){
	    fprintf(stderr,"ERROR: Must specify a cr data file\n");
	    return(0);
	}
	if(result->tmax <= 0){
	    fprintf(stderr,
		"ERROR: Must specify nonzero tmax for sta analysis\n");
	    return(0);
	}
    }

    /*
    ** use the default binsize is unspecified
    */
    if(result->binsize == 0){
	result->binsize = 1;
    }
    return(1);

}

int main(argc,argv)
int argc;
char **argv;
{
int	nxtarg;
FILE	*fptrange;
char	*fnametrange;
Result	result;
unsigned long windowmin;
unsigned long windowmax;

    nxtarg = 0;
    bzero(&result,sizeof(Result));
    result.nspikefiles = 0;
    result.binaryout = 0;
    result.fpout = stdout;
    result.binsize = 10;	/* 1 msec */
    result.ratearray = NULL;
    result.spikearray = NULL;
    result.nranges = 0;
    result.maxranges = MAXRANGES;
    result.range = (Range *)calloc(result.maxranges,sizeof(Range));
    result.analysis_mode = ISIMODE;
    result.tmax = 0;
    result.logtime = 0;
    result.sparse = FALSE;
    result.cumspikes = 0;
    result.outofrange = 0;
    result.norm1 = 0;
    result.norm2 = 0;
    result.normcorr = 0;
    result.edge = 0;
    result.ignoremultiple = 0;
    result.eegthresh = 4096;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,
	    "usage: %s tfile [tfile2] [-o file][-corr][-logtime][-bin[size] msec]\n",
	    argv[0]);
	    fprintf(stderr,"\t[-tmax msec][-v][-rate][-int][-sparse][-edge]\n");
	    fprintf(stderr,"\t[-sta crfile][-isi][-norm1][-norm2][-suppresszero]\n");
	    fprintf(stderr,
	    "\t[-trange start_timestamp end_timestamp [-trange..]]\n");
	    fprintf(stderr,
	    "\t[-trangefile file][-twindow file windowmin windowmax]\n");
	    fprintf(stderr,"\t[-normcorr]\t\tcompute normalized correlation\n");
	    fprintf(stderr,"\t[-dump]\t\toutput spike times\n");
	    fprintf(stderr,"\t[-binaryout]\t\toutput spike times in binary for dump\n");
	    fprintf(stderr,"\t[-allowmultiple]\t\tallow multiple redundant spikes\n");
	    exit(0);
	} else
	if(strcmp(argv[nxtarg],"-version") == 0){
	    fprintf(stderr,"%s : version %s : updated %s\n",
		argv[0],VERSION,DATE);

/* passed in from makefile when building an RPM*/
#ifdef MWSOFT_RPM_STRING 
	    fprintf(stderr,"From RPM: %s\n",
		    MWSOFT_RPM_STRING);
#endif

	    fprintf(stderr,"%s\n",COPYRIGHT); 
	    exit(0);
	} else
	if(strcmp(argv[nxtarg],"-logtime") == 0){
	    result.logtime = 1;
	} else
	if(strcmp(argv[nxtarg],"-norm1") == 0){
	    result.norm1 = 1;
	} else
	if(strcmp(argv[nxtarg],"-norm2") == 0){
	    result.norm2 = 1;
	} else 
	if(strcmp(argv[nxtarg],"-normcorr") == 0){
	    result.normcorr = 1;
	} else 
	if(strcmp(argv[nxtarg],"-allowmultiple") == 0){
	    result.ignoremultiple = 1;
	} else 
	if(strcmp(argv[nxtarg],"-v") == 0){
	    verbose = 1;
	} else
	if(strcmp(argv[nxtarg],"-tmax") == 0){
	    result.tmax  = 10*atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-sparse") == 0){
	    result.sparse = TRUE;
	} else
	if(strcmp(argv[nxtarg],"-suppresszero") == 0){
	    result.suppresszero = 1;
	} else
	if(strcmp(argv[nxtarg],"-edge") == 0){
	    result.edge = 1;
	} else
	if(strcmp(argv[nxtarg],"-binaryout") == 0){
	    result.binaryout = 1;
	} else
	if(strcmp(argv[nxtarg],"-dump") == 0){
	    result.analysis_mode = DUMPMODE;
	} else
	if(strcmp(argv[nxtarg],"-isi") == 0){
	    result.analysis_mode = ISIMODE;
	} else
	if(strcmp(argv[nxtarg],"-int") == 0){
	    result.analysis_mode = INTERVAL;
	} else
	if(strcmp(argv[nxtarg],"-rate") == 0){
	    result.analysis_mode = RATEMODE;
	} else
	if(strcmp(argv[nxtarg],"-corr") == 0){
	    result.analysis_mode = CORRMODE;
	} else
	if(strcmp(argv[nxtarg],"-eegthresh") == 0){
	    result.eegthresh = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-bin") == 0 ||
	(strcmp(argv[nxtarg],"-binsize") == 0)){
	    result.binsize  = 10*atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-o") == 0){
	    result.fnameout = argv[++nxtarg];
	    if((result.fpout = fopen(result.fnameout,"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open output file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	} else
        if(strcmp(argv[nxtarg],"-trange") == 0){
            /*
            ** get the start and end timestamps for the range
            */
            result.range[result.nranges].stime  = ParseTimestamp(argv[++nxtarg]);
            result.range[result.nranges].etime  = ParseTimestamp(argv[++nxtarg]);
	    if(strncmp(argv[nxtarg],"end",3) == 0){
		result.range[result.nranges].etime  = 0;
	    }
            result.nranges++;
        } else
        if(strcmp(argv[nxtarg],"-sta") == 0){
            result.fnamecr = argv[++nxtarg];
            if((result.fpcr = fopen(result.fnamecr,"r")) == NULL){
                fprintf(stderr,"ERROR: unable to open cr file '%s' \n",
		result.fnamecr);
                exit(0);
            }
	    result.analysis_mode = STAMODE;
	} else
        if(strcmp(argv[nxtarg],"-trangefile") == 0){
            fnametrange = argv[++nxtarg];
            if((fptrange = fopen(fnametrange,"r")) == NULL){
                fprintf(stderr,"ERROR: unable to open trange file '%s' \n",
		fnametrange);
                exit(0);
            }
            /*
            ** read in the time ranges
            */
            ReadRange(fptrange,&result);
	    fclose(fptrange);
	} else
        if(strcmp(argv[nxtarg],"-twindow") == 0){
            fnametrange = argv[++nxtarg];
	    windowmin = ParseTimestamp(argv[++nxtarg]);
	    windowmax = ParseTimestamp(argv[++nxtarg]);
            if((fptrange = fopen(fnametrange,"r")) == NULL){
                fprintf(stderr,"ERROR: unable to open timestamp file '%s' \n",
		fnametrange);
                exit(0);
            }
            /*
            ** read in the time ranges
            */
            ReadWindow(fptrange,&result,windowmin,windowmax);
	    fclose(fptrange);
	} else
	if(argv[nxtarg][0] != '-'){
	    if(result.nspikefiles > 1){
		fprintf(stderr,"ERROR: maximum of 2 input files allowed\n");
		exit(-1);
	    }
	    result.fnamein[result.nspikefiles] = argv[nxtarg];
	    if((result.fpin[result.nspikefiles] = fopen(argv[nxtarg],"r")) == 
	    NULL){
		fprintf(stderr,"%s: ERROR: unable to open time file '%s' \n",
		argv[0],argv[nxtarg]);
		exit(0);
	    }
	    result.nspikefiles++;
	} else {
	    fprintf(stderr,"ERROR: invalid option '%s'\n",argv[nxtarg]);
	    exit(0);
	}
    }

    if(CheckSettings(&result) == 0){
	fprintf(stderr,"analysis aborted\n");
	exit(0);
    };

    /*
    ** fill the spike time arrays from the spike time files
    */
    if(verbose){
	fprintf(stderr,"Loading spikes...");
    }
    PrepareSpikeArrays(&result);

    /*
    ** determine how many of the spikes are to be processed
    */
    CountProcessedSpikes(&result);

    /*
    ** prepare the output file header
    */
    WriteOutputHeader(&result,argc,argv);

    /*
    ** perform the analyses
    */
    switch(result.analysis_mode){
    case STAMODE:
	if(verbose){
	    fprintf(stderr,"Computing STA...");
	}
	STA(&result);
	if(verbose){
	    fprintf(stderr,"Saving...");
	}
	WriteCorr(&result);
	break;
    case ISIMODE:
	if(verbose){
	    fprintf(stderr,"Computing ISI...");
	}
	ISI(&result);
	if(verbose){
	    fprintf(stderr,"Saving...");
	}
	WriteISI(&result);
	break;
    case CORRMODE:
	if(verbose){
	    fprintf(stderr,"Computing correlation...");
	}
	Correlate(&result);
	if(verbose){
	    fprintf(stderr,"Saving...");
	}
	WriteCorr(&result);
	break;
    case RATEMODE:
	if(verbose){
	    fprintf(stderr,"Computing rate...");
	}
	Rate(&result);
	if(verbose){
	    fprintf(stderr,"Saving...");
	}
	if(WriteRate(&result) == 0){
	    fprintf(stderr,"ERROR: Error in isi output\n");
	    exit(-1);
	}
	break;
    case INTERVAL:
	if(verbose){
	    fprintf(stderr,"Computing interval...");
	}
	Interval(&result);
	break;
    case DUMPMODE:
	if(verbose){
	    fprintf(stderr,"Dumping spike times...");
	}
	DumpSpikes(&result);
	break;
    }
    /*
    ** analyses complete
    */
    if(verbose){
	fprintf(stderr,"Done. Binned %ld x %ld spikes",
	result.nspikes,result.nspikes2);
	if(result.outofrange > 0){
	    fprintf(stderr," : %d out of range\n",result.outofrange);
	} else {
	    fprintf(stderr,"\n");
	}
    }
    exit(0);
}

