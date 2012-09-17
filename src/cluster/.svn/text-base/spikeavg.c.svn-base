/*
*******************************************************************
PROGRAM:
    spikeavg

DESCRIPTION:
    Spikeavg computes average spike waveforms with std dev or sems
    by reading an index file extracted from a cluster tree.
    It can also output the individual spike waveforms listed in an index file. 
    Spikeavg can also generate a list of spike timestamps, in either
    ascii or binary format, from an index file

AUTHOR:
    Written by Matthew Wilson
    Departments of Brain and Cognitive Sciences, and Biology
    Center for Learning and Memory
    Massachussetts Institute of Technology
    Cambridge MA 02139
    wilson@ai.mit.edu

DATES:
    original program 4/91
    program update 6/96

MODIFICATIONS
    1.27 changed headers to include Fields information for spiketimes
*******************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef SUNAUDIO
#include "multimedia/ulaw2linear.h"
#include "multimedia/audio_device.h"
#endif
#include <iolib.h>
#include <header.h>


#define VERSION "1.29"

/*
******************************************
**              DEFINES
******************************************
*/
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define MAX_SPIKE 64
#define TIMESTAMP_SIZE 4
#define ST_REC_SIZE 64
#define TT_REC_SIZE 128
#define MAXTEMPLATE 128
#define PEAKTIME 6
#define MAXCHANNELS 32

#define AUDIO_RATE	8000
#define SAMPLED_RATE	33000

#define AUDIO	0
#define ASCIIPLAYBACK	1

#define GAPLEN 15

#define ADVOLTAGE 10
#define ADRESOLUTION 2048

#define MAXBURSTLEN 20

/*
******************************************
**              STRUCTURES
******************************************
*/
typedef struct result_type {
    int		testthresh;
    int		testlthresh;
    float	threshold;
    float	lthreshold;
    long	toffset;
    int		usegains;
    int		nchannels;
    double	vscale[MAXCHANNELS];
} Result;

typedef struct spike_type {
    float	avgx[MAX_SPIKE];
    float	sdx[MAX_SPIKE];
    float	avgy[MAX_SPIKE];
    float	sdy[MAX_SPIKE];
    float	avga[MAX_SPIKE];
    float	sda[MAX_SPIKE];
    float	avgb[MAX_SPIKE];
    float	sdb[MAX_SPIKE];
    int		n;
} Spike;

typedef struct template_type {
    char			filename[100];
    FILE			*fp;
    int				npts;
    double			*value;
    int				*mask;
    float			magnitude;
    struct template_type 	*next;
} Template;


typedef struct index_type {
    int		*index;
    int		n;
} Index;

typedef struct timeindex_type {
    int			index;
    unsigned long	timestamp;
} TimeIndex;

/*
******************************************
**              GLOBALS
******************************************
*/
Spike	spike;
int	stdev = 0;
int	start;
int	peak_align;
int	signalpid;
int	playbin;
float	warpfactor;
int	playmode;
int	allspikes;
int	verbose;
int	relative_cov;
long	stheadersize;
unsigned long	tstart;
unsigned long	tend;
float	samplerate;
int	spkvoffset;
int	convert;

short	showbursts;
short	showsinglespikes;

/*
******************************************
**              BASIC FUNCTIONS
******************************************
*/
int tindexcompare(p1,p2)
TimeIndex	*p1,*p2;
{
    return(p1->timestamp - p2->timestamp);
}

int indexcompare(p1,p2)
int	*p1,*p2;
{
    return(*p1 - *p2);
}

void SpikeSound(n,t,fp)
int	n,t;
FILE	*fp;
{
int	j,k;
char	cval;

    for(k=0;k<n;k++){
	for(j=0;j<t;j++){
	    cval = ~127;
	    fwrite(&cval,sizeof(char),1,fp);
	}
	for(j=0;j<t;j++){
	    cval = ~-127;
	    fwrite(&cval,sizeof(char),1,fp);
	}
    }
    fflush(fp);
}

/*
******************************************
**              INPUT ROUTINES
******************************************
*/
void ScanIndices(fp,idx)
FILE	*fp;
Index	*idx;
{
int	count;
char	line[1000];

    count = 0;
    while(!feof(fp)){
	if(fgets(line,1000,fp) == NULL) break;
	if(line[0] == '%') continue;
	count++;
    }
    idx->n = count;
}
void ReadIndices(fp,idx)
FILE	*fp;
Index	*idx;
{
int	count;
char	line[1000];
double	fval;
int	nargs;;

    count = 0;
    fseek(fp,0L,0L);
    while(!feof(fp)){
	if(fgets(line,1000,fp) == NULL) break;
	if(line[0] == '%') continue;
	nargs = sgetargs(line,1,&fval);
	idx->index[count++] = (int)(fval+0.5);
    }
    idx->n = count;
}

void DetectBursts(result,fp,idx,st,fpout,binary,min_burst_interval,max_burst_interval,showtimes,min_ss_interval,max_ss_interval)
Result	*result;
FILE	*fp;
Index	*idx;
Spike	*st;
FILE	*fpout;
int	max_burst_interval;
int	min_burst_interval;
int	binary;
int	showtimes;
int	min_ss_interval;
int	max_ss_interval;
{
int	i,j;
short	tmp[TT_REC_SIZE];
int	maxpeak = 0;
int	loc;
unsigned long timestamp;
unsigned long ptimestamp = 0;
unsigned long stimestamp;
int	nburst_events = 0;
short	in_burst = 0;
short	end_burst = 0;
short	in_nonsingle = 0;
short	single_spike = 0;
int	duration;
int	sid;
int	speak = 0;
int	epeak = 0;
int	ppeak = 0;
int	ploc = 0;
int	val;
unsigned long	bursttime[MAXBURSTLEN];
int		burstpeak[MAXBURSTLEN];

    for(i=0;i<idx->n;i++){
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	/*
	** seek to the spike location
	*/
	fseek(fp,(start + loc)*(sizeof(short)*TT_REC_SIZE+TIMESTAMP_SIZE) +
	stheadersize,0L);
	/*
	** read the timestamp
	*/
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	/*
	** and the spike waveform
	*/
	if(fread(tmp,sizeof(short),TT_REC_SIZE,fp) != TT_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	for(j=0;j<TT_REC_SIZE;j++){
	    if(convert){
		ConvertData(tmp+j,sizeof(short));
	    }
	    tmp[j] -= spkvoffset;
	    /*
	    ** scale if necessary
	    */
	    if(result->usegains){
		val = tmp[j]*result->vscale[(j%4)+((result->usegains-1)*4)];
	    } else {
		val = tmp[j];
	    }
	    /*
	    ** get the peak amplitudes
	    */
	    if((j==0)||(val > maxpeak)){
		maxpeak = val;
	    }
	}
	end_burst = 0;
	/*
	** now check for maximum interspike interval to satisfy
	** burst event requirements
	*/
	if((ptimestamp > 0) && (timestamp - ptimestamp <= max_burst_interval) &&
		((min_burst_interval < 0) || (timestamp - ptimestamp >= min_burst_interval))){
	    /*
	    ** is this the beginning of a burst event?
	    */
	    if(!in_burst){
		stimestamp = ptimestamp;
		sid = ploc;
		speak = ppeak;
		nburst_events = 1;
	    }
	    bursttime[nburst_events-1] = ptimestamp;
	    burstpeak[nburst_events-1] = ppeak;
	    in_burst = 1;
	    nburst_events++;
	} else {
	    /*
	    ** is this the end of a burst event?
	    */
	    if(in_burst){
		end_burst = 1;
		epeak = ppeak;
		bursttime[nburst_events-1] = ptimestamp;
		burstpeak[nburst_events-1] = ppeak;
	    }
	    in_burst = 0;
	}

	if(end_burst){
	    /*
	    ** if this is the end of a burst then output the timestamp
	    ** of the beginning of the event, the number of events
	    ** and the total duration
	    */
	    duration = ptimestamp - stimestamp;
	    if(binary){
		if(!showtimes){
		    if(fwrite(&sid,sizeof(int),1,fpout) != 1){
			fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		    }
		}
		if(fwrite(&stimestamp,sizeof(unsigned long),1,fpout) != 1){
		    fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		}
		if(!showtimes){
		    if(fwrite(&nburst_events,sizeof(int),1,fpout) != 1){
			fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		    }
		    if(fwrite(&duration,sizeof(int),1,fpout) != 1){
			fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		    }
		}
	    } else {
		fprintf(fpout,"%d\t%lu\t%d\t%d\t%d\t%d\t%d",
		sid,stimestamp,nburst_events,duration,speak,epeak,speak-epeak);
		/*
		** write out all of the burst times and peaks
		*/
		for(j=1;j<nburst_events;j++){
		    fprintf(fpout,"\t%d\t%d",(int)(bursttime[j]-bursttime[0]),burstpeak[j]);
		}
		/*
		** pad out the rest
		*/
		for(;j<MAXBURSTLEN;j++){
		    fprintf(fpout,"\t0\t0");
		}
		fprintf(fpout,"\n");
	    }
	    nburst_events = 0;
	}
	if(showsinglespikes){
	    /*
	    ** now check for maximum interspike interval to satisfy
	    ** burst event requirements
	    */
	    if((ptimestamp > 0) && (timestamp - ptimestamp > min_ss_interval) &&
		    ((max_ss_interval < 0) || (timestamp - ptimestamp < max_ss_interval))){
		/*
		** the requirement for a single spike is two consecutive 
		** intervals greater than the specified burst interval
		*/
		if(!in_nonsingle){
		    single_spike = 1;
		}
		in_nonsingle = 0;
	    } else {
		/*
		** this is not a single spike
		*/
		single_spike = 0;
		in_nonsingle = 1;
	    }

	    if(single_spike){
		if(binary){
		    if(!showtimes){
			if(fwrite(&ploc,sizeof(int),1,fpout) != 1){
			    fprintf(stderr,"ERROR: unable to write record %d\n",loc);
			}
		    }
		    if(fwrite(&ptimestamp,sizeof(unsigned long),1,fpout) != 1){
			fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		    }
		} else {
		    fprintf(fpout,"%d\t%lu\t%d\t%d\t%d\t%d\t%d\n",
		    ploc,ptimestamp,0,0,ppeak,ppeak,0);
		}
		single_spike = 0;
	    }
	}
	ptimestamp = timestamp;
	ppeak = maxpeak;
	ploc = loc;
    }
}

void DetectSingleSpikes(result,fp,idx,st,fpout,binary,min_ss_interval,max_ss_interval,showtimes)
Result	*result;
FILE	*fp;
Index	*idx;
Spike *st;
FILE	*fpout;
int	min_ss_interval;
int	max_ss_interval;
int	binary;
int	showtimes;
{
int	i,j;
short	tmp[TT_REC_SIZE];
int	loc;
unsigned long timestamp;
unsigned long ptimestamp = 0;
short	in_burst = 0;
int	maxpeak = 0;
int	ppeak = 0;
int	ploc = 0;
int	single_spike = 0;
int	val;

    for(i=0;i<idx->n;i++){
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	/*
	** seek to the spike location
	*/
	fseek(fp,(start + loc)*(sizeof(short)*TT_REC_SIZE+TIMESTAMP_SIZE) +
	stheadersize,0L);
	/*
	** read the timestamp
	*/
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	/*
	** and the spike waveform
	*/
	if(fread(tmp,sizeof(short),TT_REC_SIZE,fp) != TT_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	for(j=0;j<TT_REC_SIZE;j++){
	    if(convert){
		ConvertData(tmp+j,sizeof(short));
	    }
	    tmp[j] -= spkvoffset;
	    /*
	    ** scale if necessary
	    */
	    if(result->usegains){
		val = tmp[j]*result->vscale[(j%4)+((result->usegains-1)*4)];
	    } else {
		val = tmp[j];
	    }
	    /*
	    ** get the peak amplitudes
	    */
	    if((j==0)||(val > maxpeak)){
		maxpeak = val;
	    }
	}
	/*
	** now check for maximum interspike interval to satisfy
	** burst event requirements
	*/
	if((ptimestamp > 0) && (timestamp - ptimestamp > min_ss_interval) &&
		((max_ss_interval < 0) || (timestamp - ptimestamp < max_ss_interval))){
	    /*
	    ** the requirement for a single spike is two consecutive 
	    ** intervals greater than the specified burst interval
	    */
	    if(!in_burst){
		single_spike = 1;
	    }
	    in_burst = 0;
	} else {
	    /*
	    ** this is not a single spike
	    */
	    single_spike = 0;
	    in_burst = 1;
	}

	if(single_spike){
	    /*
	    ** if this is the end of a burst then output the timestamp
	    ** of the beginning of the event, the number of events
	    ** and the total duration
	    */
	    if(binary){
		if(!showtimes){
		    if(fwrite(&ploc,sizeof(int),1,fpout) != 1){
			fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		    }
		}
		if(fwrite(&ptimestamp,sizeof(unsigned long),1,fpout) != 1){
		    fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		}
	    } else {
		fprintf(fpout,"%d\t%lu\t%d\n",ploc,ptimestamp,ppeak);
	    }
	    single_spike = 0;
	}
	ptimestamp = timestamp;
	ppeak = maxpeak;
	ploc = loc;
    }
}

#ifdef SUNAUDIO
PlayTetrodeSpikes(result,fp,idx,st,fpout,channel)
Result	*result;
FILE	*fp;
Index	*idx;
Spike *st;
FILE	*fpout;
int	channel;
{
int	i,j;
int	cindex;
unsigned long timestamp;
unsigned long otimestamp;
short	tmp[TT_REC_SIZE];
FILE	*audio;
unsigned char	cval;
TimeIndex	*tindex;
unsigned long gatetime;
unsigned long tottime;
unsigned long delay;
unsigned long tdelay;
unsigned	tsleep;
double		dval;
int		n;
int		loc;
int		spikereclen;

    /*
    ** read in the timestamps
    */
    if((tindex = (TimeIndex *)malloc(idx->n*sizeof(TimeIndex))) == NULL){
	fprintf(stderr,"MEMORY ERROR: unable to allocate timestamp array\n");
	return;
    }
    tottime = 0;
    gatetime = 0;
    n = idx->n;
    spikereclen = TT_REC_SIZE/4;
    if(n > 0){
	/*
	** if an index list is given then just read the spikes on the list
	*/
	for(i=0;i<n;i++){
	    if(allspikes){
		loc = i;
	    } else {
		loc = idx->index[i];
	    }
	    fseek(fp,(start + loc)*(sizeof(short)*TT_REC_SIZE
	    +TIMESTAMP_SIZE) + stheadersize,0L);
	    if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
		fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
		return;
	    }
	    if(convert){
		ConvertData(&timestamp,sizeof(unsigned long));
	    }
	    if(result->toffset != 0){
		timestamp += result->toffset;
	    }
	    if(timestamp < tstart) continue;
	    if((tend > 0) && (timestamp > tend)) continue;
	    tindex[i].index = i;
	    tindex[i].timestamp = timestamp;
	}
	qsort(tindex,n,sizeof(TimeIndex),tindexcompare);
    }

    if(playmode == AUDIO){
	if((audio = fopen("/dev/audio","w")) == NULL){
	    fprintf(stderr,"ERROR: unable to open audio device\n");
	    exit(-1);
	}
	audio_get_play_gain(fileno(audio),&dval);
	/*
	fprintf(stderr,"Current Gain = %g\n",dval);
	*/
    }
    i = 0;
    fseek(fp,start*(sizeof(short)*TT_REC_SIZE+TIMESTAMP_SIZE) + stheadersize,0L);
    while(!feof(fp)){
	/*
	** if a certain number of spikes were to be played then
	** check the count
	*/
	if(n > 0 && i > n) break;
	otimestamp = timestamp;
	/*
	*/
	if(n > 0){
	    /*
	    ** seek to the location of the listed spike
	    */
	    fseek(fp,(start + tindex[i].index)*(sizeof(short)*TT_REC_SIZE
	    +TIMESTAMP_SIZE) + TIMESTAMP_SIZE + stheadersize,0L);
	    timestamp = tindex[i].timestamp;
	    if(timestamp < tstart) continue;
	    if((tend > 0) && (timestamp > tend)) continue;
	    if(allspikes){
		cindex = i;
	    } else {
		cindex = idx->index[i];
	    }
	} else {
	    /*
	    ** read the next spike timestamp
	    */
	    cindex = i;
	    if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
		fprintf(stderr,"ERROR: illegal read from record %d\n",cindex);
	    }
	    if(convert){
		ConvertData(&timestamp,sizeof(unsigned long));
	    }
	    if(result->toffset != 0){
		timestamp += result->toffset;
	    }
	}
	/*
	** read in the spike waveform
	*/
	if(fread(tmp,sizeof(short),TT_REC_SIZE,fp) != TT_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",cindex);
	}
	if(convert){
	    for(j=0;j<TT_REC_SIZE;j++){
		ConvertData(tmp+j,sizeof(short));
	    }
	}
	if(i != 0){
	    /* time in usec */
	    delay = 100*(timestamp - otimestamp); 
	    tottime += delay;
	    /*
	    ** break up the delay into playbin chunks
	    */
	    if(signalpid > 0 && delay > playbin*1000){
		tdelay = 0;
		while(tdelay < delay){
		    tsleep = (float)playbin*1000/warpfactor;
		    if(tsleep < 1) tsleep = 1;
		    usleep(tsleep);
		    if(signalpid > 0 && tottime >= gatetime){
			gatetime += playbin*1000;
			kill(signalpid,SIGUSR1);
		    }
		    tdelay += playbin*1000;
		}
		/*
		** get the remainder of the delay
		*/
		tsleep = ((float)delay - (float)tdelay + 
		    (float)playbin*1000 - 1e4*spikereclen/samplerate)/warpfactor;
		if(tsleep < 1) tsleep = 1;
		usleep(tsleep);
	    } else {
		tsleep = ((float)delay -1e4*spikereclen/samplerate)/warpfactor;
		if(tsleep < 1) tsleep = 1;
		usleep(tsleep);
		if(signalpid > 0 && tottime >= gatetime){
		    gatetime += playbin*1000;
		    kill(signalpid,SIGUSR1);
		}
	    }
	}
	/*
	** play the spike
	*/
	switch(playmode){
	case AUDIO:
	    /*
	    SpikeSound(3,6,audio);
	    */
	    for(j=0;j<spikereclen;j++){
		if(j%((int)(samplerate/AUDIO_RATE)) != 0) continue;
		cval = audio_s2u(32*(tmp[4*j+channel]-spkvoffset));
		fwrite(&cval,sizeof(char),1,audio);
	    }
	    fflush(audio);
	    /*
	    for(j=0;j<spikereclen;j++){
		fprintf(fpout,"%d %d\n",j+spikereclen,tmp[4*j+1]-spkvoffset);
	    }
	    */
	    break;
	case ASCIIPLAYBACK:
	    fprintf(fpout,"******\b\b\b\b\b\b");
	    fflush(fpout);
	    usleep(1000);
	    fprintf(fpout,"      \b\b\b\b\b\b");
	    fflush(fpout);
	    break;
	}
	i++;
    }
    if(playmode == AUDIO){
	fclose(audio);
    }
    free(tindex);
}
#endif
#ifdef SB_AUDIO
PlayTetrodeSpikes(result,fp,idx,st,fpout,channel)
Result	*result;
FILE	*fp;
Index	*idx;
Spike *st;
FILE	*fpout;
int	channel;
{
int	i,j;
int	cindex;
unsigned long timestamp;
unsigned long otimestamp;
short	tmp[TT_REC_SIZE];
FILE	*audio;
unsigned char	cval;
TimeIndex	*tindex;
unsigned long gatetime;
unsigned long tottime;
unsigned long delay;
unsigned long tdelay;
unsigned	tsleep;
double		dval;
int		n;
int		loc;
int		spikereclen;

    fprintf(stderr,"Play %d\n",channel);
    /*
    ** read in the timestamps
    */
    if((tindex = (TimeIndex *)malloc(idx->n*sizeof(TimeIndex))) == NULL){
	fprintf(stderr,"MEMORY ERROR: unable to allocate timestamp array\n");
	return;
    }
    sb_dsp_ioctl(1,1,1,1);
    tottime = 0;
    gatetime = 0;
    n = idx->n;
    spikereclen = TT_REC_SIZE/4;
    if(n > 0){
	/*
	** if an index list is given then just read the spikes on the list
	*/
	for(i=0;i<n;i++){
	    if(allspikes){
		loc = i;
	    } else {
		loc = idx->index[i];
	    }
	    fseek(fp,(start + loc)*(sizeof(short)*TT_REC_SIZE
	    +TIMESTAMP_SIZE) + stheadersize,0L);
	    if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
		fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
		return;
	    }
	    if(convert){
		ConvertData(&timestamp,sizeof(unsigned long));
	    }
	    if(result->toffset != 0){
		timestamp += result->toffset;
	    }
	    if(timestamp < tstart) continue;
	    if((tend > 0) && (timestamp > tend)) continue;
	    tindex[i].index = i;
	    tindex[i].timestamp = timestamp;
	}
	qsort(tindex,n,sizeof(TimeIndex),tindexcompare);
    }

    if((audio = fopen("/dev/dsp","w")) == NULL){
	fprintf(stderr,"ERROR: unable to open dsp device\n");
	exit(-1);
    }
    /*
    fprintf(stderr,"Current Gain = %g\n",dval);
    */
    i = 0;
    fseek(fp,start*(sizeof(short)*TT_REC_SIZE+TIMESTAMP_SIZE) + stheadersize,0L);
    while(!feof(fp)){
	fprintf(stderr,"%7d\n\n\n\n\n\n\n",i);
	/*
	** if a certain number of spikes were to be played then
	** check the count
	*/
	if(n > 0 && i > n) break;
	otimestamp = timestamp;
	/*
	*/
	if(n > 0){
	    /*
	    ** seek to the location of the listed spike
	    */
	    fseek(fp,(start + tindex[i].index)*(sizeof(short)*TT_REC_SIZE
	    +TIMESTAMP_SIZE) + TIMESTAMP_SIZE + stheadersize,0L);
	    timestamp = tindex[i].timestamp;
	    if(timestamp < tstart) continue;
	    if((tend > 0) && (timestamp > tend)) continue;
	    if(allspikes){
		cindex = i;
	    } else {
		cindex = idx->index[i];
	    }
	} else {
	    /*
	    ** read the next spike timestamp
	    */
	    cindex = i;
	    if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
		fprintf(stderr,"ERROR: illegal read from record %d\n",cindex);
	    }
	    if(convert){
		ConvertData(&timestamp,sizeof(unsigned long));
	    }
	    if(result->toffset != 0){
		timestamp += result->toffset;
	    }
	}
	/*
	** read in the spike waveform
	*/
	if(fread(tmp,sizeof(short),TT_REC_SIZE,fp) != TT_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",cindex);
	}
	if(convert){
	    for(j=0;j<TT_REC_SIZE;j++){
		ConvertData(tmp+j,sizeof(short));
	    }
	}
	if(i != 0){
	    /* time in usec */
	    delay = 100*(timestamp - otimestamp); 
	    tottime += delay;
	    /*
	    ** break up the delay into playbin chunks
	    */
	    if(signalpid > 0 && delay > playbin*1000){
		tdelay = 0;
		while(tdelay < delay){
		    tsleep = (float)playbin*1000/warpfactor;
		    if(tsleep < 1) tsleep = 1;
		    usleep(tsleep);
		    if(signalpid > 0 && tottime >= gatetime){
			gatetime += playbin*1000;
			kill(signalpid,SIGUSR1);
		    }
		    tdelay += playbin*1000;
		}
		/*
		** get the remainder of the delay
		*/
		tsleep = ((float)delay - (float)tdelay + 
		    (float)playbin*1000 - 1e4*spikereclen/samplerate)/warpfactor;
		if(tsleep < 1) tsleep = 1;
		usleep(tsleep);
	    } else {
		tsleep = ((float)delay -1e4*spikereclen/samplerate)/warpfactor;
		if(tsleep < 1) tsleep = 1;
		usleep(tsleep);
		if(signalpid > 0 && tottime >= gatetime){
		    gatetime += playbin*1000;
		    kill(signalpid,SIGUSR1);
		}
	    }
	}
	/*
	** play the spike
	*/
	/*
	SpikeSound(3,6,audio);
	*/
	for(j=0;j<spikereclen;j++){
	    if(j%((int)(samplerate/AUDIO_RATE)) != 0) continue;
	    cval = 255*(tmp[4*j+channel]-spkvoffset);
	    fwrite(&cval,sizeof(char),1,audio);
	}
	fflush(audio);
	/*
	for(j=0;j<spikereclen;j++){
	    fprintf(fpout,"%d %d\n",j+spikereclen,tmp[4*j+1]-spkvoffset);
	}
	*/
	i++;
    }
    fclose(audio);
    free(tindex);
}
#endif

/*
******************************************
**              OUTPUT ROUTINES
******************************************
*/
#ifdef SUNAUDIO
PlaySpikes(fp,idx,st,fpout,channel)
FILE	*fp;
Index	*idx;
Spike *st;
FILE	*fpout;
int	channel;
{
int	i,j;
int	cindex;
unsigned long timestamp;
unsigned long otimestamp;
short	tmp[ST_REC_SIZE];
FILE	*audio;
unsigned char	cval;
TimeIndex	*tindex;
unsigned long gatetime;
unsigned long tottime;
unsigned long delay;
unsigned long tdelay;
unsigned	tsleep;
double		dval;
int		n;
int		loc;
int		spikereclen;

    /*
    ** read in the timestamps
    */
    if((tindex = (TimeIndex *)malloc(idx->n*sizeof(TimeIndex))) == NULL){
	fprintf(stderr,"MEMORY ERROR: unable to allocate timestamp array\n");
	return;
    }
    tottime = 0;
    gatetime = 0;
    n = idx->n;
    spikereclen = ST_REC_SIZE/2;
    if(n > 0){
	/*
	** if an index list is given then just read the spikes on the list
	*/
	for(i=0;i<n;i++){
	    if(allspikes){
		loc = i;
	    } else {
		loc = idx->index[i];
	    }
	    fseek(fp,(start + loc)*(sizeof(short)*ST_REC_SIZE
	    +TIMESTAMP_SIZE) + stheadersize,0L);
	    if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
		fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
		return;
	    }
	    if(convert){
		ConvertData(&timestamp,sizeof(unsigned long));
	    }
	    if(result->toffset != 0){
		timestamp += result->toffset;
	    }
	    if(timestamp < tstart) continue;
	    if((tend > 0) && (timestamp > tend)) continue;
	    tindex[i].index = i;
	    tindex[i].timestamp = timestamp;
	}
	qsort(tindex,n,sizeof(TimeIndex),tindexcompare);
    }

    if(playmode == AUDIO){
	if((audio = fopen("/dev/audio","w")) == NULL){
	    fprintf(stderr,"ERROR: unable to open audio device\n");
	    exit(-1);
	}
	audio_get_play_gain(fileno(audio),&dval);
	/*
	fprintf(stderr,"Current Gain = %g\n",dval);
	*/
    }
    i = 0;
    fseek(fp,start*(sizeof(short)*ST_REC_SIZE+TIMESTAMP_SIZE) + stheadersize,0L);
    while(!feof(fp)){
	/*
	** if a certain number of spikes were to be played then
	** check the count
	*/
	if(n > 0 && i > n) break;
	otimestamp = timestamp;
	/*
	*/
	if(n > 0){
	    /*
	    ** seek to the location of the listed spike
	    */
	    fseek(fp,(start + tindex[i].index)*(sizeof(short)*ST_REC_SIZE
	    +TIMESTAMP_SIZE) + TIMESTAMP_SIZE + stheadersize,0L);
	    timestamp = tindex[i].timestamp;
	    if(timestamp < tstart) continue;
	    if((tend > 0) && (timestamp > tend)) continue;
	    if(allspikes){
		cindex = i;
	    } else {
		cindex = idx->index[i];
	    }
	} else {
	    /*
	    ** read the next spike timestamp
	    */
	    cindex = i;
	    if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
		fprintf(stderr,"ERROR: illegal read from record %d\n",cindex);
	    }
	    if(convert){
		ConvertData(&timestamp,sizeof(unsigned long));
	    }
	    if(result->toffset != 0){
		timestamp += result->toffset;
	    }
	}
	/*
	** read in the spike waveform
	*/
	if(fread(tmp,sizeof(short),ST_REC_SIZE,fp) != ST_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",cindex);
	}
	if(convert){
	    for(j=0;j<ST_REC_SIZE;j++){
		ConvertData(tmp+j,sizeof(short));
	    }
	}
	if(i != 0){
	    /* time in usec */
	    delay = 100*(timestamp - otimestamp); 
	    tottime += delay;
	    /*
	    ** break up the delay into playbin chunks
	    */
	    if(signalpid > 0 && delay > playbin*1000){
		tdelay = 0;
		while(tdelay < delay){
		    tsleep = (float)playbin*1000/warpfactor;
		    if(tsleep < 1) tsleep = 1;
		    usleep(tsleep);
		    if(signalpid > 0 && tottime >= gatetime){
			gatetime += playbin*1000;
			kill(signalpid,SIGUSR1);
		    }
		    tdelay += playbin*1000;
		}
		/*
		** get the remainder of the delay
		*/
		tsleep = ((float)delay - (float)tdelay + 
		    (float)playbin*1000 - 1e4*spikereclen/samplerate)/warpfactor;
		if(tsleep < 1) tsleep = 1;
		usleep(tsleep);
	    } else {
		tsleep = ((float)delay -1e4*spikereclen/samplerate)/warpfactor;
		if(tsleep < 1) tsleep = 1;
		usleep(tsleep);
		if(signalpid > 0 && tottime >= gatetime){
		    gatetime += playbin*1000;
		    kill(signalpid,SIGUSR1);
		}
	    }
	}
	/*
	** play the spike
	*/
	switch(playmode){
	case AUDIO:
	    /*
	    SpikeSound(3,6,audio);
	    */
	    for(j=0;j<spikereclen;j++){
		if(j%((int)(samplerate/AUDIO_RATE)) != 0) continue;
		cval = audio_s2u(32*(tmp[2*j+channel]-spkvoffset));
		fwrite(&cval,sizeof(char),1,audio);
	    }
	    fflush(audio);
	    /*
	    for(j=0;j<spikereclen;j++){
		fprintf(fpout,"%d %d\n",j+spikereclen,tmp[2*j+1]-spkvoffset);
	    }
	    */
	    break;
	case ASCIIPLAYBACK:
	    fprintf(fpout,"******\b\b\b\b\b\b");
	    fflush(fpout);
	    usleep(1000);
	    fprintf(fpout,"      \b\b\b\b\b\b");
	    fflush(fpout);
	    break;
	}
	i++;
    }
    if(playmode == AUDIO){
	fclose(audio);
    }
    free(tindex);
}
#endif

void TemplateMatch(result,fp,idx,st,fpout,template,show,corr_cutoff,lcorr_cutoff)
Result	*result;
FILE	*fp;
Index	*idx;
Spike 	*st;
FILE	*fpout;
Template	*template;
int	show;
float	corr_cutoff;
float	lcorr_cutoff;
{
Template	*t;
int	i,j;
short	tmp[ST_REC_SIZE];
float 	magy;
float 	magx;
float	corr;
float	maxcorr = 0;
float	corrx,corry;
int	first;
int	loc;
int	spikereclen;
unsigned long timestamp;

    spikereclen = ST_REC_SIZE/2;
    for(i=0;i<idx->n;i++){
	if(verbose){
	    if((idx->n > 100) && (i%(idx->n/100) == 0)){
		fprintf(stderr," %3d%%\b\b\b\b\b",
		(100*i)/(idx->n));
	    }
	}
	/*
	** read into the spike file
	*/
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	fseek(fp,(start + loc)*(sizeof(short)*ST_REC_SIZE + TIMESTAMP_SIZE) +
	stheadersize,0L);
	fread(&timestamp,sizeof(unsigned long),1,fp);
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	if(fread(tmp,sizeof(short),ST_REC_SIZE,fp) != ST_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	for(j=0;j<ST_REC_SIZE;j++){
	    if(convert){
		ConvertData(tmp+j,sizeof(short));
	    }
	    tmp[j] -= spkvoffset;
	}
	/*
	** if thresholding is selected then test the waveform
	*/
	if(result->testthresh){
	    for(j=0;j<ST_REC_SIZE;j++){
		if(tmp[j] >= result->threshold) break;
	    }
	    if(j == ST_REC_SIZE) continue;
	}
	if(result->testlthresh){
	    for(j=0;j<ST_REC_SIZE;j++){
		if(tmp[j] >= result->lthreshold) break;
	    }
	    if(j != ST_REC_SIZE) continue;
	}
	/*
	** compute the magnitude of the waveform
	** To get the correlation coefficient then normalize with
	** the waveform magnitude.
	** 
	**  r = (W dot T)/(mag(W)*mag(T))
	**
	** to get covariance relative to the template then use
	**
	**  Ct = (W dot T)/(mag(T)*mag(T))
	*/
	first = 1;
	for(t = template;t != NULL; t = t->next){
	    if(relative_cov){
		if(t->npts > spikereclen){
		    magx = t->magnitude;
		    magy = 0;
		} else {
		    magx = t->magnitude;
		    magy = t->magnitude;
		}
	    } else {
		magx = 0;
		magy = 0;
		for(j=0;j<spikereclen;j++){
		    magx += tmp[2*j]*tmp[2*j]*t->mask[j];
		    if(t->npts > spikereclen){
			magy += tmp[2*j+1]*tmp[2*j+1]*t->mask[j+spikereclen];
		    } else {
			magy += tmp[2*j+1]*tmp[2*j+1]*t->mask[j];
		    }
		}
	    }
	    corrx = 0;
	    corry = 0;
	    corr = 0;
	    /*
	    ** template match x component
	    */
	    for(j=0;j<spikereclen;j++){
		corrx += tmp[2*j]*t->value[j]*t->mask[j];
	    }
	    /*
	    ** template match y component
	    */
	    if(t->npts > spikereclen){
		/*
		** use y template values
		*/
		for(j=0;j<spikereclen;j++){
		    corry += tmp[2*j+1]*t->value[j+spikereclen]*
		    t->mask[j+spikereclen];
		}
	    } else {
		/*
		** reuse x template values
		*/
		for(j=0;j<spikereclen;j++){
		    corry += tmp[2*j+1]*t->value[j]*t->mask[j];
		}
	    }
	    /*
	    ** use the total correlation
	    */
	    if(t->npts > spikereclen){
		corr = (corrx + corry)/
		(sqrt(magx + magy)*sqrt(t->magnitude));
	    } else {
		/*
		** use the max correlation
		*/
		if(corrx > corry){
		    corr = corrx/(sqrt(magx)*sqrt(t->magnitude));
		} else {
		    corr = corry/(sqrt(magy)*sqrt(t->magnitude));
		}
	    }
	    if(first || corr > maxcorr) {
		maxcorr = corr;
		first = 0;
	    }
	}
	if(show){
	    if((maxcorr > corr_cutoff) && (maxcorr < lcorr_cutoff)){
		fprintf(fpout,"%d\n",loc);
	    }
	} else {
	    fprintf(fpout,"%g\n",maxcorr);
	}
    }
}

void TetrodeTemplateMatch(result,fp,idx,st,fpout,template,show,corr_cutoff,lcorr_cutoff)
Result	*result;
FILE	*fp;
Index	*idx;
Spike 	*st;
FILE	*fpout;
Template	*template;
int	show;
float	corr_cutoff;
float	lcorr_cutoff;
{
Template	*t;
int	i,j;
short	spikedata[TT_REC_SIZE];
float 	magy;
float 	magx;
float 	maga;
float 	magb;
float	corr;
float	maxcorr = 0;
float	corrx,corry;
float	corra,corrb;
int	first;
int	loc;
int	spikereclen;
unsigned long timestamp;

    spikereclen = TT_REC_SIZE/4;
    for(i=0;i<idx->n;i++){
	if(verbose){
	    if((idx->n > 100) && (i%(idx->n/100) == 0)){
		fprintf(stderr," %3d%%\b\b\b\b\b",
		(100*i)/(idx->n));
	    }
	}
	/*
	** read into the spike file
	*/
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	fseek(fp,(start + loc)*(sizeof(short)*TT_REC_SIZE + TIMESTAMP_SIZE) +
	stheadersize,0L);
	fread(&timestamp,sizeof(unsigned long),1,fp);
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	if(fread(spikedata,sizeof(short),TT_REC_SIZE,fp) != TT_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    for(j=0;j<TT_REC_SIZE;j++){
		ConvertData(spikedata+j,sizeof(short));
	    }
	}
	for(j=0;j<TT_REC_SIZE;j++){
	    spikedata[j] -= spkvoffset;
	}
	if(result->testthresh){
	    for(j=0;j<TT_REC_SIZE;j++){
		if(spikedata[j] >= result->threshold) break;
	    }
	    if(j == TT_REC_SIZE) continue;
	}
	if(result->testlthresh){
	    for(j=0;j<TT_REC_SIZE;j++){
		if(spikedata[j] < result->lthreshold) break;
	    }
	    if(j == TT_REC_SIZE) continue;
	}
	/*
	** compute the magnitude of the waveform
	** To get the correlation coefficient then normalize with
	** the waveform magnitude.
	** 
	**  r = (W dot T)/(mag(W)*mag(T))
	**
	** to get covariance relative to the template then use
	**
	**  Ct = (W dot T)/(mag(T)*mag(T))
	*/
	first = 1;
	for(t = template;t != NULL; t = t->next){
	    if(relative_cov){
		if(t->npts > spikereclen){
		    magx = t->magnitude;
		    magy = 0;
		    maga = 0;
		    magb = 0;
		} else {
		    magx = t->magnitude;
		    magy = t->magnitude;
		    maga = t->magnitude;
		    magb = t->magnitude;
		}
	    } else {
		magx = 0;
		magy = 0;
		maga = 0;
		magb = 0;
		for(j=0;j<spikereclen;j++){
		    magx += spikedata[4*j]*spikedata[4*j]*t->mask[j];
		    if(t->npts > spikereclen){
			magy += spikedata[4*j+1]*spikedata[4*j+1]*
				t->mask[j+spikereclen];
			maga += spikedata[4*j+2]*spikedata[4*j+2]*
				t->mask[j+2*spikereclen];
			magb += spikedata[4*j+3]*spikedata[4*j+3]*
				t->mask[j+3*spikereclen];
		    } else {
			magy += spikedata[4*j+1]*spikedata[4*j+1]*t->mask[j];
			maga += spikedata[4*j+2]*spikedata[4*j+2]*t->mask[j];
			magb += spikedata[4*j+3]*spikedata[4*j+3]*t->mask[j];
		    }
		}
	    }
	    corrx = 0;
	    corry = 0;
	    corra = 0;
	    corrb = 0;
	    corr = 0;
	    /*
	    ** template match x component
	    */
	    for(j=0;j<spikereclen;j++){
		corrx += spikedata[4*j]*t->value[j]*t->mask[j];
	    }
	    /*
	    ** template match y,a,b component
	    */
	    if(t->npts > spikereclen){
		/*
		** use y template values
		*/
		for(j=0;j<spikereclen;j++){
		    corry += spikedata[4*j+1]*t->value[j+spikereclen]*
		    t->mask[j+spikereclen];
		    corra += spikedata[4*j+2]*t->value[j+2*spikereclen]*
		    t->mask[j+2*spikereclen];
		    corrb += spikedata[4*j+3]*t->value[j+3*spikereclen]*
		    t->mask[j+3*spikereclen];
		}
	    } else {
		/*
		** reuse x template values
		*/
		for(j=0;j<spikereclen;j++){
		    corry += spikedata[4*j+1]*t->value[j]*t->mask[j];
		    corra += spikedata[4*j+2]*t->value[j]*t->mask[j];
		    corrb += spikedata[4*j+3]*t->value[j]*t->mask[j];
		}
	    }
	    /*
	    ** use the total correlation
	    */
	    if(t->npts > spikereclen){
		corr = (corrx + corry + corra + corrb)/
		(sqrt(magx + magy + maga + magb)*sqrt(t->magnitude));
	    } else {
		/*
		** use the max correlation
		*/
		if((corrx > corry) && (corrx > corra) && (corrx > corrb)){
		    corr = corrx/(sqrt(magx)*sqrt(t->magnitude));
		} else 
		if((corry > corrx) && (corry > corra) && (corry > corrb)){
		    corr = corry/(sqrt(magy)*sqrt(t->magnitude));
		} else
		if((corra > corry) && (corra > corrx) && (corra > corrb)){
		    corr = corra/(sqrt(maga)*sqrt(t->magnitude));
		} else {
		    corr = corrb/(sqrt(magy)*sqrt(t->magnitude));
		}
	    }
	    if(first || corr > maxcorr) {
		maxcorr = corr;
		first = 0;
	    }
	}
	if(show){
	    if((maxcorr > corr_cutoff) && (maxcorr < lcorr_cutoff)){
		fprintf(fpout,"%d\n",loc);
	    }
	} else {
	    fprintf(fpout,"%g\n",maxcorr);
	}
    }
}

void ShowSpikes(result,fp,idx,st,fpout,names,fpstout)
Result	*result;
FILE	*fp;
Index	*idx;
Spike 	*st;
FILE	*fpout;
int	names;
FILE	*fpstout;
{
int	i,j;
short	tmp[ST_REC_SIZE];
unsigned long timestamp;
int	loc;
int	spikereclen;

    spikereclen = ST_REC_SIZE/2;
    for(i=0;i<idx->n;i++){
	if(verbose){
	    if((idx->n > 100) && (i%(idx->n/100) == 0)){
		fprintf(stderr," %3d%%\b\b\b\b\b",
		(100*i)/(idx->n));
	    }
	}
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	fseek(fp,(start + loc)*(sizeof(short)*ST_REC_SIZE +TIMESTAMP_SIZE) +
	stheadersize,0L);
	/*
	** read in the timestamp
	*/
	fread(&timestamp,sizeof(unsigned long),1,fp);
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	/*
	** read in the waveform
	*/
	if(fread(tmp,sizeof(short),ST_REC_SIZE,fp) != ST_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    for(j=0;j<ST_REC_SIZE;j++){
		ConvertData(tmp+j,sizeof(short));
	    }
	}
	if(fpstout){
	    fwrite(&timestamp,sizeof(unsigned long),1,fpstout);
	    if(fwrite(tmp,sizeof(short),ST_REC_SIZE,fpstout) != ST_REC_SIZE){
		fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		return;
	    }
	} else {
	    for(j=0;j<ST_REC_SIZE;j++){
		tmp[j] -= spkvoffset;
	    }
	    fprintf(fpout,"/newplot\n");
	    fprintf(fpout,"/plotname %d\n",loc);
	    fprintf(fpout,"/line\n");
	    for(j=0;j<spikereclen;j++){
		fprintf(fpout,"%d\t%d\n",j,tmp[2*j]);
	    }
	    for(j=0;j<spikereclen;j++){
		fprintf(fpout,"%d\t%d\n",j+spikereclen,tmp[2*j+1]);
	    }
	    if(names){
		fprintf(fpout,"/wtext %d\t%d\t%d\n",
		2*spikereclen,tmp[2*spikereclen-1],
		loc);
	    }
	}
    }
}

int InsertGap(fp,start,len,val)
FILE	*fp;
int	start;
int	len;
int	val;
{
    fprintf(fp,"/defaultcluster -1\n");
    fprintf(fp,"%d\t%d\n",start,val);
    fprintf(fp,"%d\t%d\n",start+len-1,val);
    fprintf(fp,"/defaultcluster 0\n");
    /*
    ** return the number of points inserted
    */
    return(len);
}

void ShowTetrodeSpikes(result,fp,idx,st,fpout,names,fpstout,timech)
Result	*result;
FILE	*fp;
Index	*idx;
Spike 	*st;
FILE	*fpout;
int	names;
FILE	*fpstout;
int	timech;
{
int	i,j;
short	tmp[TT_REC_SIZE];
int	itmp[TT_REC_SIZE];
unsigned long timestamp;
int	loc;
int	spikereclen;
double	dt;
int	count;

    spikereclen = TT_REC_SIZE/4;
    for(i=0;i<idx->n;i++){
	if(verbose){
	    if((idx->n > 100) && (i%(idx->n/100) == 0)){
		fprintf(stderr," %3d%%\b\b\b\b\b",
		(100*i)/(idx->n));
	    }
	}
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	fseek(fp,(start + loc)*(sizeof(short)*TT_REC_SIZE +TIMESTAMP_SIZE) +
	stheadersize,0L);
	/*
	** read in the timestamp
	*/
	fread(&timestamp,sizeof(unsigned long),1,fp);
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	/*
	** read in the waveform
	*/
	if(fread(tmp,sizeof(short),TT_REC_SIZE,fp) != TT_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    for(j=0;j<TT_REC_SIZE;j++){
		ConvertData(tmp+j,sizeof(short));
	    }
	}
	if(result->testthresh){
	    for(j=0;j<TT_REC_SIZE;j++){
		if(tmp[j] - spkvoffset >= result->threshold) break;
	    }
	    if(j == TT_REC_SIZE) continue;
	}
	if(result->testlthresh){
	    for(j=0;j<TT_REC_SIZE;j++){
		if(tmp[j] - spkvoffset < result->lthreshold) break;
	    }
	    if(j == TT_REC_SIZE) continue;
	}
	if(fpstout){
	    fwrite(&timestamp,sizeof(unsigned long),1,fpstout);
	    if(fwrite(tmp,sizeof(short),TT_REC_SIZE,fpstout) != TT_REC_SIZE){
		fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		return;
	    }
	} else {
	    for(j=0;j<TT_REC_SIZE;j++){
		tmp[j] -= spkvoffset;
		/*
		** scale if necessary
		*/
		if(result->usegains){
		    itmp[j] = tmp[j]*result->vscale[(j%4)+((result->usegains-1)*4)];
		} else {
		    itmp[j] = tmp[j];
		}
	    }
	    if(timech >= 0){
		dt = 1.0e4/(double)samplerate;
		/*
		** put the first pt at zero to minimize distracting 
		** connecting lines between spike events
		*/
		fprintf(fpout,"%-25.14g\t0\n",(double)timestamp);
		/*
		** display channel timech
		*/
		for(j=0;j<spikereclen;j++){
		    fprintf(fpout,"%-25.14g\t%d\n",
		    (double)timestamp + j*dt,itmp[4*j + timech]);
		}
		/*
		** put the final pt at zero to minimize distracting 
		** connecting lines between spike events
		*/
		fprintf(fpout,"%-25.14g\t0\n",(double)timestamp + spikereclen*dt);
	    } else {
		fprintf(fpout,"/newplot\n");
		fprintf(fpout,"/plotname %d\n",loc);
		fprintf(fpout,"/line\n");
		fprintf(fp,"/clustercolor 1 1\n");
		count = 0;
		for(j=0;j<spikereclen;j++){
		    fprintf(fpout,"%d\t%d\n",count,itmp[4*j]);
		    count++;
		}
		count += InsertGap(fpout,count,GAPLEN,0);
		for(j=0;j<spikereclen;j++){
		    fprintf(fpout,"%d\t%d\n",count,itmp[4*j+1]);
		    count++;
		}
		count += InsertGap(fpout,count,GAPLEN,0);
		for(j=0;j<spikereclen;j++){
		    fprintf(fpout,"%d\t%d\n",count,itmp[4*j+2]);
		    count++;
		}
		count += InsertGap(fpout,count,GAPLEN,0);
		for(j=0;j<spikereclen;j++){
		    fprintf(fpout,"%d\t%d\n",count,itmp[4*j+3]);
		    count++;
		}
	    }
	    if(names){
		fprintf(fpout,"/wtext %d\t%d\t%d\n",
		2*spikereclen,itmp[2*spikereclen-1],
		loc);
	    }
	}
    }
}

void ShowSpikeTimes(result,fp,idx,st,fpout,binary)
Result	*result;
FILE	*fp;
Index	*idx;
Spike *st;
FILE	*fpout;
int	binary;
{
int	i;
unsigned long	timestamp;
int	loc;

    for(i=0;i<idx->n;i++){
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	fseek(fp,(start + loc)*(sizeof(short)*ST_REC_SIZE +TIMESTAMP_SIZE) +
	stheadersize,0L);
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	if(binary){
	    if(fwrite(&timestamp,sizeof(unsigned long),1,fpout) != 1){
		fprintf(stderr,"ERROR: unable to write record %d\n",loc);
	    }
	} else {
	    fprintf(fpout,"%lu 1\n",timestamp);
	}
    }
}

void ShowSingleSpikeTimes(result,fp,idx,st,fpout,binary,mingap)
Result	*result;
FILE	*fp;
Index	*idx;
Spike *st;
FILE	*fpout;
int	binary;
int	mingap;
{
int	i;
unsigned long	timestamp;
unsigned long	ptimestamp;
unsigned char	probe;
int	loc;

    timestamp = 0;
    for(i=0;i<idx->n;i++){
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	ptimestamp = timestamp;
	fseek(fp,(start + loc)*(sizeof(unsigned char)+sizeof(short)+
	TIMESTAMP_SIZE) + stheadersize,0L);
	if(fread(&probe,sizeof(unsigned char),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	if(ptimestamp > 0 && (timestamp - ptimestamp < mingap)){
	    continue;
	}
	if(binary){
	    if(fwrite(&timestamp,sizeof(unsigned long),1,fpout) != 1){
		fprintf(stderr,"ERROR: unable to write record %d\n",loc);
	    }
	} else {
	    fprintf(fpout,"%lu 1\n",timestamp);
	}
    }
}

void ShowTetrodeSpikeTimes(result,fp,idx,st,fpout,binary)
Result	*result;
FILE	*fp;
Index	*idx;
Spike *st;
FILE	*fpout;
int	binary;
{
short	spikedata[TT_REC_SIZE];
int	i,j;
unsigned long	timestamp;
int	loc;

    for(i=0;i<idx->n;i++){
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	fseek(fp,(start + loc)*(sizeof(short)*TT_REC_SIZE +TIMESTAMP_SIZE) +
	stheadersize,0L);
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	if(result->testthresh || result->testlthresh){
	    if(fread(spikedata,sizeof(short),TT_REC_SIZE,fp) != TT_REC_SIZE){
		fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
		return;
	    }
	    if(convert){
		for(j=0;j<TT_REC_SIZE;j++){
		    ConvertData(spikedata+j,sizeof(short));
		}
	    }
	    if(result->testthresh){
		for(j=0;j<TT_REC_SIZE;j++){
		    if(spikedata[j] -spkvoffset>= result->threshold) break;
		}
		if(j == TT_REC_SIZE) continue;
	    }
	    if(result->testlthresh){
		for(j=0;j<TT_REC_SIZE;j++){
		    if(spikedata[j] -spkvoffset< result->lthreshold) break;
		}
		if(j == TT_REC_SIZE) continue;
	    }
	}
	if(binary){
	    if(fwrite(&timestamp,sizeof(unsigned long),1,fpout) != 1){
		fprintf(stderr,"ERROR: unable to write record %d\n",loc);
	    }
	} else {
	    fprintf(fpout,"%lu\t1\n",timestamp);
	}
    }
}

void WriteSpike(fp,st)
FILE	*fp;
Spike *st;
{
int	i;
int	spikereclen;

    spikereclen = ST_REC_SIZE/2;
    fprintf(fp,"/newplot\n");
    fprintf(fp,"/line\n");
    for(i=0;i<spikereclen;i++){
	fprintf(fp,"%d\t%g\t%g\n",i,st->avgx[i],st->sdx[i]);
    }
    for(i=0;i<spikereclen;i++){
	fprintf(fp,"%d\t%g\t%g\n",i+spikereclen,st->avgy[i],st->sdy[i]);
    }
}

void WriteTetrodeSpike(fp,st)
FILE	*fp;
Spike *st;
{
int	i;
int	spikereclen;
int	count;

    spikereclen = TT_REC_SIZE/4;
    fprintf(fp,"/newplot\n");
    fprintf(fp,"/line\n");
    count = 0;
    for(i=0;i<spikereclen;i++){
	fprintf(fp,"%d\t%g\t%g\n",count,st->avgx[i],st->sdx[i]);
	count++;
    }
    count += InsertGap(fp,count,GAPLEN,0);
    for(i=0;i<spikereclen;i++){
	fprintf(fp,"%d\t%g\t%g\n",count,st->avgy[i],st->sdy[i]);
	count++;
    }
    count += InsertGap(fp,count,GAPLEN,0);
    for(i=0;i<spikereclen;i++){
	fprintf(fp,"%d\t%g\t%g\n",count,st->avga[i],st->sda[i]);
	count++;
    }
    count += InsertGap(fp,count,GAPLEN,0);
    for(i=0;i<spikereclen;i++){
	fprintf(fp,"%d\t%g\t%g\n",count,st->avgb[i],st->sdb[i]);
	count++;
    }
}


/*
******************************************
**              CORE ROUTINE
******************************************
*/
void AvgSpikes(result,fp,idx,st)
Result	*result;
FILE	*fp;
Index	*idx;
Spike *st;
{
int	i,j;
short	tmp[ST_REC_SIZE];
double	sumsqr[ST_REC_SIZE];
double	sum[ST_REC_SIZE];
int	n;
int	align = 0;
int	pt;
int	tpeak_x;
int	tpeak_y;
int	peak_x;
int	peak_y;
int	loc;
int	spikereclen;
unsigned long timestamp;

    spikereclen = ST_REC_SIZE/2;
    for(j=0;j<spikereclen;j++){
	sum[2*j] = 0;
	sum[2*j+1] = 0;
	sumsqr[2*j] = 0;
	sumsqr[2*j+1] = 0;
    }
    st->n = 0;
    for(i=0;i<idx->n;i++){
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	fseek(fp,(start + loc)*(sizeof(short)*ST_REC_SIZE+TIMESTAMP_SIZE) +
	stheadersize,0L);
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	if(fread(tmp,sizeof(short),ST_REC_SIZE,fp) != ST_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	for(j=0;j<ST_REC_SIZE;j++){
	    if(convert){
		ConvertData(tmp+j,sizeof(short));
	    }
	    tmp[j] -= spkvoffset;
	}
	if(peak_align){
	    /*
	    ** shift the record over to position the peak at time PEAKTIME
	    */
	    peak_x = tmp[0];
	    peak_y = tmp[1];
	    tpeak_x = 0;
	    tpeak_y = 0;
	    for(j=1;j<spikereclen;j++){
		if(tmp[2*j] > peak_x){
		    tpeak_x = j;
		    peak_x = tmp[2*j];
		}
		if(tmp[2*j+1] > peak_y){
		    tpeak_y = j;
		    peak_y = tmp[2*j+1];
		}
		/*
		** pick the shift of the max peak
		*/
		if(peak_x > peak_y){
		    align = PEAKTIME - tpeak_x;
		} else {
		    align = PEAKTIME - tpeak_y;
		}
	    }
	} else {
	    align = 0;
	}
	for(j=0;j<spikereclen;j++){
	    pt = 2*(j+align);
	    if(pt < 0 || pt >= ST_REC_SIZE) continue;
	    sum[2*j] += tmp[pt];
	    sumsqr[2*j] += tmp[pt]*tmp[pt];
	    pt = 2*(j+align)+1;
	    if(pt < 0 || pt >= ST_REC_SIZE) continue;
	    sum[2*j+1] += tmp[pt];
	    sumsqr[2*j+1] += tmp[pt]*tmp[pt];
	}
	st->n++;
    }
    if(st->n > 0){
	n = st->n;
	for(j=0;j<spikereclen;j++){
	    st->avgx[j] = sum[2*j]/n;
	    st->avgy[j] = sum[2*j+1]/n;
	    if(st->n > 1){
		if(stdev){
		    /* 
		    ** standard deviation of the mean
		    */
		    st->sdx[j] =
		    sqrt((sumsqr[2*j]-sum[2*j]*sum[2*j]/n)/((n-1)));
		    st->sdy[j] =
		    sqrt((sumsqr[2*j+1]-sum[2*j+1]*sum[2*j+1]/n)/((n-1)));
		} else {
		    /*
		    ** standard error of the mean
		    */
		    st->sdx[j] = sqrt((sumsqr[2*j]-sum[2*j]*sum[2*j]/n)/
		    ((float)n*(n-1)));
		    st->sdy[j] =
		    sqrt((sumsqr[2*j+1]-sum[2*j+1]*sum[2*j+1]/n)/
		    ((float)n*(n-1)));
		}
	    } else {
		st->sdx[j] = 0;
		st->sdy[j] = 0;
	    }
	}
    }
}

void AvgTetrodeSpikes(result,fp,idx,st)
Result	*result;
FILE	*fp;
Index	*idx;
Spike *st;
{
int	i,j;
short	tmp[TT_REC_SIZE];
int	itmp[TT_REC_SIZE];
double	sumsqr[TT_REC_SIZE];
double	sum[TT_REC_SIZE];
int	n;
int	align = 0;
int	pt;
int	tpeak_x;
int	tpeak_y;
int	tpeak_a;
int	tpeak_b;
int	peak_x;
int	peak_y;
int	peak_a;
int	peak_b;
int	loc;
int	spikereclen;
unsigned long timestamp;

    spikereclen = TT_REC_SIZE/4;
    for(j=0;j<TT_REC_SIZE;j++){
	sum[j] = 0;
	sumsqr[j] = 0;
    }
    st->n = 0;
    for(i=0;i<idx->n;i++){
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	fseek(fp,(start + loc)*(sizeof(short)*TT_REC_SIZE+TIMESTAMP_SIZE) +
	stheadersize,0L);
	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	if(result->toffset != 0){
	    timestamp += result->toffset;
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	if(fread(tmp,sizeof(short),TT_REC_SIZE,fp) != TT_REC_SIZE){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	for(j=0;j<TT_REC_SIZE;j++){
	    if(convert){
		ConvertData(tmp+j,sizeof(short));
	    }
	    tmp[j] -= spkvoffset;
	    /*
	    ** scale if necessary
	    */
	    if(result->usegains){
		itmp[j] = tmp[j]*result->vscale[(j%4)+((result->usegains-1)*4)];
	    } else {
		itmp[j] = tmp[j];
	    }
	}
	if(peak_align){
	    /*
	    ** shift the record over to position the peak at time PEAKTIME
	    */
	    peak_x = itmp[0];
	    peak_y = itmp[1];
	    peak_a = itmp[2];
	    peak_b = itmp[3];
	    tpeak_x = 0;
	    tpeak_y = 0;
	    tpeak_a = 0;
	    tpeak_b = 0;
	    for(j=1;j<spikereclen;j++){
		if(itmp[4*j] > peak_x){
		    tpeak_x = j;
		    peak_x = itmp[4*j];
		}
		if(itmp[4*j+1] > peak_y){
		    tpeak_y = j;
		    peak_y = itmp[4*j+1];
		}
		if(itmp[4*j+2] > peak_a){
		    tpeak_a = j;
		    peak_a = itmp[4*j+2];
		}
		if(itmp[4*j+3] > peak_b){
		    tpeak_b = j;
		    peak_b = itmp[4*j+3];
		}
		/*
		** pick the shift of the max peak
		*/
		if((peak_x > peak_y) && (peak_x > peak_a) && (peak_x > peak_b)){
		    align = PEAKTIME - tpeak_x;
		} else 
		if((peak_y > peak_x) && (peak_y > peak_a) && (peak_y > peak_b)){
		    align = PEAKTIME - tpeak_y;
		} else
		if((peak_a > peak_y) && (peak_a > peak_x) && (peak_a > peak_b)){
		    align = PEAKTIME - tpeak_a;
		} else {
		    align = PEAKTIME - tpeak_b;
		}
	    }
	} else {
	    align = 0;
	}
	for(j=0;j<spikereclen;j++){
	    /*
	    ** xchannel
	    */
	    pt = 4*(j+align);
	    if(pt < 0 || pt >= TT_REC_SIZE) continue;
	    sum[4*j] += itmp[pt];
	    sumsqr[4*j] += itmp[pt]*itmp[pt];

	    /*
	    ** ychannel
	    */
	    pt = 4*(j+align)+1;
	    if(pt < 0 || pt >= TT_REC_SIZE) continue;
	    sum[4*j+1] += itmp[pt];
	    sumsqr[4*j+1] += itmp[pt]*itmp[pt];

	    /*
	    ** achannel
	    */
	    pt = 4*(j+align)+2;
	    if(pt < 0 || pt >= TT_REC_SIZE) continue;
	    sum[4*j+2] += itmp[pt];
	    sumsqr[4*j+2] += itmp[pt]*itmp[pt];

	    /*
	    ** bchannel
	    */
	    pt = 4*(j+align)+3;
	    if(pt < 0 || pt >= TT_REC_SIZE) continue;
	    sum[4*j+3] += itmp[pt];
	    sumsqr[4*j+3] += itmp[pt]*itmp[pt];
	}
	st->n++;
    }
    if(st->n > 0){
	n = st->n;
	for(j=0;j<spikereclen;j++){
	    st->avgx[j] = sum[4*j]/n;
	    st->avgy[j] = sum[4*j+1]/n;
	    st->avga[j] = sum[4*j+2]/n;
	    st->avgb[j] = sum[4*j+3]/n;
	    if(st->n > 1){
		if(stdev){
		    /* 
		    ** standard deviation of the mean
		    */
		    st->sdx[j] =
		    sqrt((sumsqr[4*j]-sum[4*j]*sum[4*j]/n)/((n-1)));
		    st->sdy[j] =
		    sqrt((sumsqr[4*j+1]-sum[4*j+1]*sum[4*j+1]/n)/((n-1)));
		    st->sda[j] =
		    sqrt((sumsqr[4*j+2]-sum[4*j+2]*sum[4*j+2]/n)/((n-1)));
		    st->sdb[j] =
		    sqrt((sumsqr[4*j+3]-sum[4*j+3]*sum[4*j+3]/n)/((n-1)));
		} else {
		    /*
		    ** standard error of the mean
		    */
		    st->sdx[j] = sqrt((sumsqr[4*j]-sum[4*j]*sum[4*j]/n)/
		    ((float)n*(n-1)));
		    st->sdy[j] =
		    sqrt((sumsqr[4*j+1]-sum[4*j+1]*sum[4*j+1]/n)/
		    ((float)n*(n-1)));
		    st->sda[j] =
		    sqrt((sumsqr[4*j+2]-sum[4*j+2]*sum[4*j+2]/n)/
		    ((float)n*(n-1)));
		    st->sdb[j] =
		    sqrt((sumsqr[4*j+3]-sum[4*j+3]*sum[4*j+3]/n)/
		    ((float)n*(n-1)));
		}
	    } else {
		st->sdx[j] = 0;
		st->sdy[j] = 0;
		st->sda[j] = 0;
		st->sdb[j] = 0;
	    }
	}
    }
}

Template *AddTemplate(template,fp,filename)
Template	*template;
FILE		*fp;
char		*filename;
{
Template	*newtemplate;
Template	*t;
double		fval;
char		line[1000];
int		nargs;
double		tmpdata[MAXTEMPLATE];
int		tmpmask[MAXTEMPLATE];
int		npts;
int		j;

    /*
    ** allocate and initialize the template
    */
    newtemplate = (Template *)calloc(1,sizeof(Template));
    newtemplate->fp = fp;
    strcpy(newtemplate->filename,filename);

    /*
    ** read in the template data
    */
    newtemplate->npts = 0;
    npts = 0;
    while(!feof(fp)){
	if(fgets(line,1000,fp) == NULL){		/* read in a line of data */
	    break;
	}
	if(line[0] == '%') continue;		/* ignore comments */
	/* 
	** template point to ignore 
	*/
	if(line[0] == '@') {
		tmpmask[npts] = 0;
	} else {
	    /* get the data point */
	    nargs = sscanf(line,"%lf",&fval);
	    if(nargs > 0) {
		tmpdata[npts] = fval;
		tmpmask[npts] = 1;
	    } else {
		continue;
	    }
	}
	npts++;
	if(npts > MAXTEMPLATE){	/* check for template size */
	    fprintf(stderr,
	    "ERROR: number of points in template exceeded maximum (%d)\n",MAXTEMPLATE);
	    return(NULL);
	}
    }
    newtemplate->npts = npts;
    newtemplate->value = (double *)malloc(npts*sizeof(double));
    newtemplate->mask = (int *)malloc(npts*sizeof(int));
    bcopy(tmpdata,newtemplate->value,npts*sizeof(double));
    bcopy(tmpmask,newtemplate->mask,npts*sizeof(int));
    newtemplate->magnitude = 0;
    for(j=0;j<newtemplate->npts;j++){
	newtemplate->magnitude += newtemplate->value[j]*newtemplate->value[j]*
	newtemplate->mask[j];
    }

    /* 
    ** add the new template to the template list
    */
    if(template == NULL){		/* start a new list */
	template = newtemplate;
    } else {				/* go to the end of the list */
	for(t=template;t->next;t=t->next);
	t->next = newtemplate;
    }
    return(template);
}


/*
******************************************
**              MAIN
******************************************
*/
int main(argc,argv)
int argc;
char **argv;
{
int	i;
int	nxtarg;
FILE	*fpindex;
FILE	*fpst;
FILE	*fpstout;
FILE	*fptemplate;
FILE	*fpout;
int	showspikes;
int	showavg;
int	binary;
int	showspiketimes;
int	playspikes;
int	tetrode;
int	names;
char	*template_fname;
int	templatematch;
Template	*template;
float	corrcutoff;
float	lcorrcutoff;
struct stat stbuf;
int	timech;
char	**header;
char	*outname;
char	*stname = NULL;
char	*indexname= NULL;
char	*spiketype;
char	*tmpstr;
int	singletrode;
int	mingap;
Index	index;
int	max_burst_interval;
int	min_ss_interval;
int	min_burst_interval;
int	max_ss_interval;
Result	result;
char	line[100];



    showbursts = 0;
    showsinglespikes = 0;
    max_burst_interval = 0;
    min_ss_interval = 0;
    max_ss_interval = -1;
    min_burst_interval = -1;
    nxtarg = 0;
    mingap = 5000;		/* 500 msec min gap between se timestamps */
    spkvoffset = 0;
    verbose = 0;
    result.testthresh = 0;
    result.testlthresh = 0;
    result.threshold = 0;
    result.lthreshold = 0;
    result.toffset = 0;
    result.usegains = 0;
    index.n = 0;
    fpout = stdout;
    outname = "STDOUT";
    fpst = NULL;
    showavg = 1;
    showspikes = 0;
    peak_align = 0;
    start = 0;
    binary = 0;
    showspiketimes = 0;
    playspikes = 0;
    signalpid = 0;
    warpfactor = 1;
    playmode = AUDIO;
    fpindex = NULL;
    fpstout = NULL;
    names = FALSE;
    templatematch = 0;
    template = NULL;
    corrcutoff = -1;
    lcorrcutoff = 1;
    allspikes = 0;
    tetrode = 0;
    singletrode = 0;
    relative_cov = 0;
    timech = -1;
    tstart = 0;
    tend = 0;
    samplerate = SAMPLED_RATE;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,
	    "usage: %s spikefile [-iall][-i index][-if indexfile][-v][-align][-std]\n",
	    argv[0]);
	    fprintf(stderr,
	    "\t[-template file][-o file][-show][-showtimes][-binary][-play x/y]\n");
	    fprintf(stderr,
	    "\t[-signal pid bin(msec)][-warp timefactor][-hipass corr][-lowpass corr]\n");
	    fprintf(stderr,
	    "\t[-ttout file][-cov][-tetrode][-tstart timestamp][-tend timestamp]\n");
	    fprintf(stderr,"\t[-timech ch][-samplerate Hz]\n");
	    fprintf(stderr,
	    "\t[-showavg][-noavg][-higherthan threshold][-lowerthan threshold]\n");
	    fprintf(stderr,"\t[-spkvoffset val]\n");
	    fprintf(stderr,"\t[-applygains probe (0-1)]\n");
	    fprintf(stderr,"\t[-bursts max isi (msec)]\n");
	    fprintf(stderr,"\t[-singlespikes min isi (msec)]\n");
	    fprintf(stderr,"\t[-burstwin min maxisi (msec)]\n");
	    fprintf(stderr,"\t[-singlespikewin min maxisi (msec)]\n");
	    fprintf(stderr,"\t[-toffset time]\tadd 'time' to all spikefile timestamps\n");
	    fprintf(stderr,"\t[-negtoffset time]\tsubtract 'time' to all spikefile timestamps\n");
	    fprintf(stderr,"\t\t\t\t'time' MUST be positive\n");
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
	if(strcmp(argv[nxtarg],"-toffset") == 0){
	    result.toffset = ParseTimestamp(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-negtoffset") == 0){
	    result.toffset = -ParseTimestamp(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-applygains") == 0){
	    result.usegains = atoi(argv[++nxtarg])+1;
	} else
	if(strcmp(argv[nxtarg],"-bursts") == 0){
	    min_burst_interval = -1;
	    max_burst_interval = atof(argv[++nxtarg])*10;
	    showspiketimes = 0;
	    showavg = 0;
	    showbursts = 1;
	} else
	if(strcmp(argv[nxtarg],"-singlespikes") == 0){
	    min_ss_interval = atof(argv[++nxtarg])*10;
	    max_ss_interval = -1;
	    showspiketimes = 0;
	    showavg = 0;
	    showsinglespikes = 1;
	} else
	if(strcmp(argv[nxtarg],"-burstwin") == 0){
	    min_burst_interval = atof(argv[++nxtarg])*10;
	    max_burst_interval = atof(argv[++nxtarg])*10;
	    showspiketimes = 0;
	    showavg = 0;
	    showbursts = 1;
	} else
	if(strcmp(argv[nxtarg],"-singlespikewin") == 0){
	    min_ss_interval = atof(argv[++nxtarg])*10;
	    max_ss_interval = atof(argv[++nxtarg])*10;
	    showspiketimes = 0;
	    showavg = 0;
	    showsinglespikes = 1;
	} else
	if(strcmp(argv[nxtarg],"-noavg") == 0){
	    showavg = 0;
	} else
	if(strcmp(argv[nxtarg],"-showavg") == 0){
	    showavg = 1;
	} else
	if(strcmp(argv[nxtarg],"-show") == 0){
	    showspikes = 1;
	} else
	if(strcmp(argv[nxtarg],"-spkvoffset") == 0){
	    spkvoffset = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-timech") == 0){
	    showspikes = 1;
	    timech = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-iall") == 0){
	    allspikes = 1;
	} else
	if(strcmp(argv[nxtarg],"-tetrode") == 0){
	    tetrode = 1;
	} else
	if(strcmp(argv[nxtarg],"-cov") == 0){
	    relative_cov = 1;
	} else
	if(strcmp(argv[nxtarg],"-showtimes") == 0){
	    showspiketimes = 1;
	    showavg = 0;
	} else
	if(strcmp(argv[nxtarg],"-tstart") == 0){
	    tstart = ParseTimestamp(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-tend") == 0){
	    tend = ParseTimestamp(argv[++nxtarg]);
	    if(strncmp(argv[nxtarg],"end",3) == 0){
		tend = 0;
	    }
	} else
	if(strcmp(argv[nxtarg],"-samplerate") == 0){
	    samplerate = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-higherthan") == 0){
	    /* templatematch = 1; */
	    result.testthresh = 1;
	    result.threshold = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-lowerthan") == 0){
	    /* templatematch = 1; */
	    result.testlthresh = 1;
	    result.lthreshold = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-lowpass") == 0){
	    lcorrcutoff = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-hipass") == 0){
	    corrcutoff = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-template") == 0){
	    templatematch = 1;
	    template_fname = argv[++nxtarg];
	    if((fptemplate = fopen(template_fname,"r")) == NULL){
		fprintf(stderr,"ERROR: unable to open template file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    template = AddTemplate(template,fptemplate,template_fname);
	    showavg = 0;
	} else
	if(strcmp(argv[nxtarg],"-binary") == 0){
	    binary = 1;
	} else
	if(strcmp(argv[nxtarg],"-warp") == 0){
	    warpfactor = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-names") == 0){
	    names = TRUE;
	} else
	if(strcmp(argv[nxtarg],"-start") == 0){
	    start = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-signal") == 0){
	    signalpid = atoi(argv[++nxtarg]);	/* pid to gate */
	    playbin = atoi(argv[++nxtarg]);	/* binsize in msec */
	} else
	if(strcmp(argv[nxtarg],"-align") == 0){
	    peak_align = 1;
	} else
	if(strcmp(argv[nxtarg],"-std") == 0){
	    stdev = 1;
	} else
	if(strcmp(argv[nxtarg],"-play") == 0){
	    playspikes = atoi(argv[++nxtarg]);
	    showavg = 0;
	} else
	if(strcmp(argv[nxtarg],"-v") == 0){
	    verbose = 1;
	} else
	if(strcmp(argv[nxtarg],"-i") == 0){
	    if(index.n == 0){
		if((index.index = (int *)malloc(sizeof(int))) == NULL){
		    fprintf(stderr,"MEMORY ERROR: unable to allocate index array\n");
		    exit(-1);
		}
	    } else 
	    if((index.index = (int *)
		realloc(index.index,(index.n +1)*sizeof(int))) == NULL){
		fprintf(stderr,"MEMORY ERROR: unable to allocate index array\n");
		exit(-1);
	    }
	    index.index[index.n] = atoi(argv[++nxtarg]);
	    index.n++;
	} else
	if(strcmp(argv[nxtarg],"-if") == 0){
	    indexname = argv[++nxtarg];
	    if((fpindex = fopen(indexname,"r")) == NULL){
		fprintf(stderr,"ERROR: unable to open index file '%s'\n",indexname);
		exit(-1);
	    }
	} else
	if((strcmp(argv[nxtarg],"-stout") == 0 ) ||
	(strcmp(argv[nxtarg],"-ttout") == 0)){
	    if((fpstout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else
	if(strcmp(argv[nxtarg],"-o") == 0){
	    outname = argv[++nxtarg];
	    if((fpout = fopen(outname,"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open output file '%s'\n",outname);
		exit(-1);
	    }
	} else
	if(argv[nxtarg][0] != '-'){
	    stname = argv[nxtarg];
	    if((fpst = fopen(stname,"r")) == NULL){
		fprintf(stderr,"ERROR: unable to open spike file '%s'\n",stname);
		exit(-1);
	    }
	} else {
	    fprintf(stderr,"ERROR: %s: invalid option '%s'\n",
	    argv[0],argv[nxtarg]);
	    exit(-1);
	}
    }
    if(fpst == NULL){
	fprintf(stderr,"ERROR: Must provide a spike file\n");
	exit(0);
    }
    /*
    ** read the header of the spike file
    */
    header = ReadHeader(fpst,&stheadersize);
    if(verbose){
	DisplayHeader(stderr,header,stheadersize);
    }
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
    ** try to read some parameters from the header
    */
    if((spiketype = GetHeaderParameter(header,"Extraction type:")) != NULL){
	if(strncmp(spiketype,"tetrode waveforms",strlen("tetrode waveforms")) ==
	0){
	    tetrode = 1;
	}
	if(strncmp(spiketype,"single electrode",strlen("single electrode")) ==
	0){
	    singletrode = 1;
	}
    }
    /*
    ** try to read the gains for each channel
    */
    if((tmpstr = GetHeaderParameter(header,"nchannels:")) != NULL){
	result.nchannels = atoi(tmpstr);
    } else {
	fprintf(stderr,"Unable to determine number of channels from header. Using default (8)\n");
	result.nchannels = 8;
    }
    /*
    ** now go through each channel
    */
    for(i=0;i<result.nchannels;i++){
	sprintf(line,"channel %d ampgain:",i);
	if((tmpstr = GetHeaderParameter(header,line)) != NULL){
	    if(atoi(tmpstr) > 0){
		/*
		** scale in microvolts
		*/
		result.vscale[i] = 1.0e6*ADVOLTAGE/(atoi(tmpstr)*ADRESOLUTION);
		if(verbose){
		    fprintf(stderr,"ch %d ampgain %d : %g uV/unit\n",i,atoi(tmpstr),result.vscale[i]);
		}
	    } else {
		result.vscale[i] = 0;
	    }
	} else {
	    fprintf(stderr,"Unable to determine amp gain for channel %d from header.\n",i);
	    exit(-1);
	}
	sprintf(line,"channel %d adgain:",i);
	if((tmpstr = GetHeaderParameter(header,line)) != NULL){
	    result.vscale[i] /= pow(2.0,(double)(atoi(tmpstr)));
	    if(verbose){
		fprintf(stderr,"ch %d adgain %d : %g uV/unit\n",i,atoi(tmpstr),result.vscale[i]);
	    }
	} else {
	    fprintf(stderr,"Unable to determine amp gain for channel %d from header.\n",i);
	    exit(-1);
	}
    }
    if(fpstout){
	BeginStandardHeader(fpstout,argc,argv,VERSION);
	fprintf(fpstout,"%%\n%% Beginning of header from spike file: \t%s\n",stname);
	DisplayHeader(fpstout,header,stheadersize);
	EndStandardHeader(fpstout);
    }
    /*
    ** write the output file header
    */
    BeginStandardHeader(fpout,argc,argv,VERSION);
    if(result.usegains){
	fprintf(fpout,"%% Units: \t%s\n","uV");
    } else {
	fprintf(fpout,"%% Units: \t%s\n","AD");
    }
    fprintf(fpout,"%% Input Spike file: \t%s\n",stname);
    if(allspikes){
	fprintf(fpout,"%% Spike Indices: \t%s\n","Allspikes");
    } else
    if(fpindex){
	fprintf(fpout,"%% Spike Index file: \t%s\n",indexname);
    } else
    if(index.n != 0){
	fprintf(fpout,"%% Spike Indices: ");
	for(i=0;i<index.n;i++){
	    fprintf(fpout,"%d ",index.index[i]);
	}
	fprintf(fpout,"\n");
    }
    fprintf(fpout,"%% Output file: \t%s\n",outname);
    if(binary){
	fprintf(fpout,"%% File type: \t%s\n","Binary");
    } else {
	fprintf(fpout,"%% File type: \t%s\n","Ascii");
    }
    if(tetrode){
	fprintf(fpout,"%% Spike type: \t%s\n","tetrode");
    } else 
    if(singletrode){
	fprintf(fpout,"%% Spike type: \t%s\n","single electrode");
    } else {
	fprintf(fpout,"%% Spike type: \t%s\n","stereotrode");
    }
    if(showbursts){
	fprintf(fpout,"%% Burst detection interval: \t%d\n",max_burst_interval);
	if(binary){
	    if(showspiketimes){
		fprintf(fpout,"%% Fields: \t%s,%d,%d,%d\n",
		"timestamp",ULONG,sizeof(unsigned long),1);
	    } else
	    fprintf(fpout,
"%% Fields: \t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\n",
	    "id",INT,sizeof(int),1,
	    "timestamp",ULONG,sizeof(unsigned long),1,
	    "nevents",INT,sizeof(int),1,
	    "duration",INT,sizeof(int),1,
	    "peak1",INT,sizeof(int),1,
	    "peakn",INT,sizeof(int),1,
	    "delpeak",INT,sizeof(int),1);
	} else {
	    fprintf(fpout,
"%% Fields: \t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d",
	    "id",INT,sizeof(int),1,
	    "timestamp",ULONG,sizeof(unsigned long),1,
	    "nevents",INT,sizeof(int),1,
	    "duration",INT,sizeof(int),1,
	    "peak1",INT,sizeof(int),1,
	    "peakn",INT,sizeof(int),1,
	    "delpeak",INT,sizeof(int),1);
	    for(i=1;i<MAXBURSTLEN;i++){
		fprintf(fpout,"\t%s%d\t%s%d","t",i+1,"p",i+1);
	    }
	    fprintf(fpout,"\n");
	}
    } else
    if(showsinglespikes){
	fprintf(fpout,"%% Single spike detection interval: \t%d\n",max_burst_interval);
	if(binary){
	    if(showspiketimes){
		fprintf(fpout,"%% Fields: \t%s,%d,%d,%d\n",
		"timestamp",ULONG,sizeof(unsigned long),1);
	    } else
	    fprintf(fpout,"%% Fields: \t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\n",
	    "id",INT,sizeof(int),1,
	    "timestamp",ULONG,sizeof(unsigned long),1,
	    "delpeak",INT,sizeof(int),1);
	} else {
	    fprintf(fpout,"%% Fields: \t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\n",
	    "id",INT,sizeof(int),1,
	    "timestamp",ULONG,sizeof(unsigned long),1,
	    "delpeak",INT,sizeof(int),1);
	}
    } else 
    if(showspiketimes){
	fprintf(fpout,"%% Fields: \t%s,%d,%d,%d\n",
	"timestamp",ULONG,sizeof(unsigned long),1);
    }
    EndStandardHeader(fpout);
    /*
    ** end output file header
    */

    /*
    ** check to see if any indices were manually specified
    ** If not then try to read them from the index file.
    ** This should be changed to allow combining multiple file and
    ** manual specifications
    */
    if(index.n == 0){
	if(fpindex != NULL){
	    ScanIndices(fpindex,&index);
	    if((index.index = (int *)malloc(index.n*sizeof(int))) == NULL){
		fprintf(stderr,"MEMORY ERROR: unable to allocate index array\n");
		exit(-1);
	    }
	    ReadIndices(fpindex,&index);
	} else 
	if(allspikes){
	    /*
	    ** if all spikes are selected then compute the number of spikes
	    ** from the size of the file
	    */
	    fstat(fileno(fpst),&stbuf);
	    if(tetrode){
		index.n = (stbuf.st_size - stheadersize)/
		(sizeof(short)*TT_REC_SIZE+TIMESTAMP_SIZE);
	    } else 
	    if(singletrode){
		index.n = (stbuf.st_size - stheadersize)/
		(sizeof(unsigned char)+sizeof(short)+TIMESTAMP_SIZE);
	    } else {
		index.n = (stbuf.st_size - stheadersize)/
		(sizeof(short)*ST_REC_SIZE+TIMESTAMP_SIZE);
	    }
	}
    }
    /*
    ** check to see whether spikes were specified by timestamp
    if(select_by_timestamp){
	ScanTimestamps(fptimestamp,fpst,&index)
    }
    */
    if(!allspikes){
	/*
	** sort the indices
	*/
	if(verbose){
	    fprintf(stderr,"sorting %d indices\n",index.n);
	}
	qsort(index.index,index.n,sizeof(int),indexcompare);
    }
  
    if(verbose){
	fprintf(stderr,"processing %d spikes\n",index.n);
    }
    /*
    ** begin processing spikes
    */
    if(tetrode){
	if(templatematch){
	    TetrodeTemplateMatch(&result,fpst,&index,&spike,fpout,template,showspikes,
	    corrcutoff, lcorrcutoff);
	    exit(0);
	}
#if (SB_AUDIO || SUNAUDIO)
	if(playspikes){
	    PlayTetrodeSpikes(fpst,&index,&spike,fpout,playspikes-1);
	}
#endif
	if(showbursts){
	    DetectBursts(&result,fpst,&index,&spike,fpout,binary,
		min_burst_interval,max_burst_interval,
	    showspiketimes,min_ss_interval,max_ss_interval);
	} else
	if(showsinglespikes){
	    DetectSingleSpikes(&result,fpst,&index,&spike,fpout,binary,
	    min_ss_interval,max_ss_interval,showspiketimes);
	} else
	if(showspiketimes){
	    ShowTetrodeSpikeTimes(&result,fpst,&index,&spike,fpout,binary);
	}
	if(showavg){
	    AvgTetrodeSpikes(&result,fpst,&index,&spike);
	    if(verbose){
		fprintf(stderr,"Read %d waveforms\n",spike.n);
	    }
	    WriteTetrodeSpike(fpout,&spike);
	}
	if(showspikes){
	    ShowTetrodeSpikes(&result,fpst,&index,&spike,fpout,names,fpstout,timech);
	}
    } else 
    if(singletrode){
	if(showspiketimes){
	    ShowSingleSpikeTimes(&result,fpst,&index,&spike,fpout,binary,mingap);
	}
    }else {
	if(templatematch){
	    TemplateMatch(&result,fpst,&index,&spike,fpout,template,showspikes,
	    corrcutoff, lcorrcutoff);
	    exit(0);
	}
#ifdef SUNAUDIO
	if(playspikes){
	    PlaySpikes(result,fpst,&index,&spike,fpout,playspikes-1);
	}
#endif
	if(showspikes){
	    ShowSpikes(result,fpst,&index,&spike,fpout,names,fpstout);
	}
	if(showspiketimes){
	    ShowSpikeTimes(result,fpst,&index,&spike,fpout,binary);
	}
	if(showavg){
	    AvgSpikes(result,fpst,&index,&spike);
	    if(verbose){
		fprintf(stderr,"Read %d waveforms\n",spike.n);
	    }
	    WriteSpike(fpout,&spike);
	}
    }
    exit(0);
}
