/*
 *******************************************************************
 PROGRAM:
 spikeparms2
 
 DESCRIPTION:
 Spikeparms extracts parameters from spike waveforms. Can extract standard
 Brainwave(tm) parameters as well as alternate parameters based
 on waveform amplitudes taken at fixed offsets from the time of
 the peak amplitude.
 Will currently process single electrode and stereotrode data
 in either binary or ascii format.
 Output is in the form of an ascii file containing the parameters
 for each processed spike waveform on a single line.
 This data can be used as input to the automatic cluster 
 analysis program 'cluster'.
 
 AUTHOR:
 Written by Matthew Wilson
 Department of Brain and Cognitive Sciences, and Biology
 Center for Learning and Memory
 Massachusetts Institute of Technology
 wilson@ai.mit.edu
 
 DATES:
 original program 4/91
 program update 9/95
 *******************************************************************
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

/* MWL Headers */
#include <iolib.h>
#include <header.h>


#define VERSION "1.8"

/*
 *********************************************************
 **              DEFINES
 *********************************************************
 */
#define	MAX_SPIKE_LEN	32
#define TRIGGER_PT	5
#define MAX_VECTORS	200000

#define NPARMBINS	500

#ifndef TRUE
#define	TRUE	1
#define	FALSE	0
#endif

#ifndef M_PI
#define M_PI 3.14159
#endif

#define ST_REC_SIZE 64
#define TT_REC_SIZE 128
#define TIMESTAMP_SIZE 4

#define SINGLE	0
#define STEREO	1
#define TETRODE	2

#define CORRTEMPLATE	0
#define DIFFTEMPLATE	1
#define DOTTEMPLATE	2

#define ASCII	0
#define BINARY	1

#define MAXLINE 1000

/*
 ** this sets the default number of alternate parameter offset times.
 ** This should be consistent with the initialization of
 ** the 'peak_offset' variable
 */
#define N_OFFSET_PTS 7

#define NPARMS 135

/*
 ** --------------------------------------
 ** standard Brainwave(tm) parameters
 ** --------------------------------------
 */
#define PX		0
#define PY		1
#define VX		2
#define VY		3
#define TPX		4
#define TPY		5
#define TVX		6
#define TVY		7
#define MAXHT		8
#define PPHS		9
#define VPHS		10
#define DLAT		11
#define MAXWD		12

/*
 ** --------------------------------------
 ** additional non-Brainwave parameters
 ** --------------------------------------
 */
/*
 ** peak relative amplitude 
 */
#define XM4		13
#define YM4		14
#define XM2		15
#define YM2		16
#define X0		17
#define Y0		18
#define XP2		19
#define YP2		20
#define XP4		21
#define YP4		22
#define XP8		23
#define YP8		24
#define XP12		25
#define YP12		26
/*
 ** peak time
 */
#define T0		27
/*
 ** peak relative phase 
 */
#define PHSM4		28
#define PHSM2		29
#define PHS0		30
#define PHSP2		31
#define PHSP4		32
#define PHSP8		33
#define PHSP12		34

/*
 ** mean phase and phase variance
 */
#define AVGPHS		35
#define SDPHS		36
#define PAMP		37
/* 
 ** zero crossing time relative to peak 
 */
#define ZCROSS		38
/* 
 ** integral ratios relative to zero crossing 
 */
#define INZPM		39

#define INZP2M2		40
#define INZP4M4		41
#define INZP8M8		42

/* 
 ** integral relative to peak 
 */
#define INPM3M6		43
#define INPP12M12	44

/*
 ** template matches
 */
#define MAXTEMPLATES 10
#define MAXTEMPLATESIZE 64
#define TEMPLATE0	45
#define TEMPLATE1	46
#define TEMPLATE2	47
#define TEMPLATE3	48
#define TEMPLATE4	49
#define TEMPLATE5	50
#define TEMPLATE6	51
#define TEMPLATE7	52
#define TEMPLATE8	53
#define TEMPLATE9	54

/*
 ** peak magnitude
 */
#define	PMAG		55
#define	VMAG		56

/*
 ** --------------------------------------
 ** Tetrode parameters	(x,y,,u,v channels)
 ** --------------------------------------
 */
#define T_PX		70
#define T_PY		71
#define T_PA		72
#define T_PB		73

#define T_VX		74
#define T_VY		75
#define T_VA		76
#define T_VB		77

#define T_MAXWD		78
/*
 ** refined tetrode parameters
 */
#define	T_PMAG		79
#define	T_VMAG		80
#define T_PPHSRXY	81
#define T_PPHSRXYA	82
#define T_PPHSRXYAB	83

#define T_VPHSRXY	84
#define T_VPHSRXYA	85
#define T_VPHSRXYAB	86

/*
 ** extra tetrode parameters
 */
#define T_TPX		90
#define T_TPY		91
#define T_TPA		92
#define T_TPB		93

#define T_TVX		94
#define T_TVY		95
#define T_TVA		96
#define T_TVB		97

#define T_PPHSXY	100
#define T_PPHSAB	101
#define T_PPHSAX	102
#define T_PPHSAY	103
#define T_PPHSBX	104
#define T_PPHSBY	105

#define T_VPHSXY	106
#define T_VPHSAB	107
#define T_VPHSAX	108
#define T_VPHSAY	109
#define T_VPHSBX	110
#define T_VPHSBY	111

#define T_MAXHT		112

/* 
 ** tetrode integral relative to peak 
 */
#define T_INPM3M6		43
#define T_INPP12M12		44

/*
 ** spatial location parameters
 */
#define POS_X		113
#define POS_Y		114

/*
 ** time
 */
#define TIME		115
#define TIMELO		116
#define TIMESTAMP	117
#define VEL	118

/* 
 ** tetrode integral relative to peak 
 */
#define T_INPP3M3		119
#define T_INPP3M6		120
#define T_INZPM3		121
#define T_INZPM6		122
#define T_INZ6X			123
#define T_INZ6Y			124
#define T_INZ6A			125
#define T_INZ6B			126
#define T_INPP			127
#define T_INMAX			128
#define T_TPMAX			129
#define T_TVMAX			130

/*  Hadamard transformation axes */
#define T_H1   			131
#define T_H2    		132
#define T_H3    		133
#define T_H4    		134

#define BW_ATAN_FACTOR	3.61

/*
 *********************************************************
 **              STRUCTURES
 *********************************************************
 */
typedef struct {
  unsigned long	timestamp;
  unsigned char	x1;
  unsigned char	y1;
  unsigned char	x2;
  unsigned char	y2;
} OldPosRecord;

typedef struct {
  unsigned long	timestamp;
  short		x1;
  short		y1;
  short		x2;
  short		y2;
} PosRecord;

typedef struct spike_channel_type {
  int		data[MAX_SPIKE_LEN];
  int		len;
  FILE		*fp;
  char		*filename;
} SpikeChannel;

typedef struct trange_type {
  unsigned long	tstart;
  unsigned long	tend;
  struct trange_type	*next;
} TRange;

typedef struct spikeparm_type {
  double	parm;
  char	*name;
  short	type;
  float	sum;
  float	sumsqr;
  int		npts;
  int		*hist;
  int		nbins;
  float	min;
  float	max;
  float	histsum;
  float	histsumsqr;
  float	histvar;
} SpikeParm;

typedef struct single_parameter_type {
  int		xvalues[N_OFFSET_PTS];
} SingleParms;

typedef struct bwsingle_parameter_type {
  int		peak;
  int		valley;
  int		peak_time;
  int		valley_time;
  int		spike_height;
  int		spike_width;
} BWSingleParms;	

typedef struct template_type {
  char			filename[100];
  int				npts;
  double			*value;
  short			*mask;
  float			magnitude;
  int				mode;
  struct template_type 	*next;
} Template;

typedef struct index_type {
  int		*index;
  int		n;
} Index;

typedef struct result_type {
  FILE	*fpout;
  FILE	*fptrange;
  char	*trangefname;
  int		electrode;
  int		inputformat;
  int		outputformat;
  TRange	*trange;
  int		oldpos;
  int		rate;
  float		velspan;
} Result;


/*
 *********************************************************
 **              GLOBALS
 *********************************************************
 */
int		verbose;
int	nparms;
int 	parm_list[NPARMS];
int	trigger_pt;
FILE	*position_file;
int	xoffset;
int	spkvoffset;
int     backdiode;

int		nwaveforms;

Template	*templatelist;

/*
 *********************************************************
 **              BASIC ROUTINES
 *********************************************************
 */

int intcompare(i1,i2)
     int	*i1,*i2;
{
  return(*i1 - *i2);
}

int CheckTimestampRange(timestamp,result)
     unsigned long	timestamp;
     Result		*result;
{
  TRange	*trange;
  
  if(result->trange == NULL) return(1);
  /*
   ** go through the time ranges and see whether it falls within one
   */
  for(trange=result->trange;trange;trange=trange->next){
    if((timestamp >= trange->tstart) && (timestamp <= trange->tend)){
      return(1);
    }
  }
  /*
   ** didnt fall within any of the time ranges so fail
   */
  return(0);
}

void ReadIndices(fp,idx)
     FILE	*fp;
     Index	*idx;
{
  int	count;
  char	line[1000];
  int	ival;
  
  count = 0;
  while(!feof(fp)){
    if(fgets(line,1000,fp) == NULL) break;
    if(line[0] == '%') continue;
    if(line[0] == '/') continue;		/* and xplot commands */
    if(sscanf(line,"%d",&ival) == 1){
      idx->index[count++] = ival;
    }
    if(count >= MAX_VECTORS){
      fprintf(stderr,"exceeded maximum number of spikes (%d)\n", 
	      MAX_VECTORS);
      break;
    }
  }
  idx->n = count;
}


void ShowParmNames(st)
     SpikeParm	*st;
{
  int	i;
  
  for(i=0;i<NPARMS;i++){
    if(st[i].name){
      fprintf(stderr,"%s\n",st[i].name);
    }
  }
}

void SetParmNames(st)
     SpikeParm	*st;
{
  /*
   ** standard Brainwave (tm)
   */
  st[PX].name	 		= "px";
  st[PY].name 		= "py";
  st[VX].name 		= "vx";
  st[VY].name 		= "vy";
  st[TPX].name 		= "tpx";
  st[TPY].name 		= "tpy";
  st[TVX].name 		= "tvx";
  st[TVY].name 		= "tvy";
  st[MAXHT].name 		= "maxht";
  st[PPHS].name 		= "pphs";
  st[VPHS].name 		= "vphs";
  st[DLAT].name 		= "dlat";
  st[MAXWD].name 		= "maxwd";
  
  /*
   ** peak relative amplitude
   */
  st[XM4].name 		= "xm4";
  st[YM4].name 		= "ym4";
  st[XM2].name 		= "xm2";
  st[YM2].name 		= "ym2";
  st[X0].name 		= "x0";
  st[Y0].name 		= "y0";
  st[XP2].name 		= "xp2";
  st[YP2].name 		= "yp2";
  st[XP4].name 		= "xp4";
  st[YP4].name 		= "yp4";
  st[XP8].name 		= "xp8";
  st[YP8].name 		= "yp8";
  st[XP12].name 		= "xp12";
  st[YP12].name 		= "yp12";
  
  /*
   ** peak time
   */
  st[T0].name 		= "t0";
  
  /*
   ** peak relative phase 
   */
  st[PHSM4].name 		= "phsm4";
  st[PHSM2].name 		= "phsm2";
  st[PHS0].name 		= "phs0";
  st[PHSP2].name 		= "phsp2";
  st[PHSP4].name 		= "phsp4";
  st[PHSP8].name 		= "phsp8";
  st[PHSP12].name 		= "phsp12";
  
  /*
   ** mean phase and phase variance
   */
  st[AVGPHS].name 		= "avgphs";
  st[SDPHS].name 		= "sdphs";
  /*
   ** peak amplitude
   */
  st[PAMP].name 		= "pamp";
  st[ZCROSS].name 		= "zcross";
  /* 
   ** integral ratios relative to zero crossing 
   */
  st[INZPM].name 		= "inzpm";
  st[INZP2M2].name 		= "inzp2m2";
  st[INZP4M4].name 		= "inzp4m4";
  st[INZP8M8].name 		= "inzp8m8";
  
  /* 
   ** integral relative to peak 
   */
  st[INPM3M6].name 		= "inpm3m6";
  st[INPP12M12].name		= "inpp12m12";
  
  st[TEMPLATE0].name		= "tmpl0";
  st[TEMPLATE1].name		= "tmpl1";
  st[TEMPLATE2].name		= "tmpl2";
  st[TEMPLATE3].name		= "tmpl3";
  st[TEMPLATE4].name		= "tmpl4";
  st[TEMPLATE5].name		= "tmpl5";
  st[TEMPLATE6].name		= "tmpl6";
  st[TEMPLATE7].name		= "tmpl7";
  st[TEMPLATE8].name		= "tmpl8";
  st[TEMPLATE9].name		= "tmpl9";
  
  st[PMAG].name		= "pmag";
  st[VMAG].name		= "vmag";
  
  /*
   ** tetrode parameters
   */
  st[T_PX].name		= "t_px";
  st[T_PX].type		= SHORT;
  st[T_PY].name		= "t_py";
  st[T_PY].type		= SHORT;
  st[T_PA].name		= "t_pa";
  st[T_PA].type		= SHORT;
  st[T_PB].name		= "t_pb";
  st[T_PB].type		= SHORT;
  
  st[T_VX].name		= "t_vx";
  st[T_VY].name		= "t_vy";
  st[T_VA].name		= "t_va";
  st[T_VB].name		= "t_vb";
  
  st[T_MAXWD].name		= "t_maxwd";
  st[T_MAXWD].type		= SHORT;
  st[T_TPMAX].name		= "t_tpmax";
  st[T_TVMAX].name		= "t_tvmax";
  st[T_TPX].name		= "t_tpx";
  st[T_TPX].type		= SHORT;
  st[T_TPY].name		= "t_tpy";
  st[T_TPY].type		= SHORT;
  st[T_TPA].name		= "t_tpa";
  st[T_TPA].type		= SHORT;
  st[T_TPB].name		= "t_tpb";
  st[T_TPB].type		= SHORT;
  st[T_TVX].name		= "t_tvx";
  st[T_TVY].name		= "t_tvy";
  st[T_TVA].name		= "t_tva";
  st[T_TVB].name		= "t_tvb";
  
  st[T_PMAG].name		= "t_pmag";
  st[T_VMAG].name		= "t_vmag";
  
  st[T_PPHSRXY].name	= 	"t_pphsrxy";
  st[T_PPHSRXYA].name	= 	"t_pphsrxya";
  st[T_PPHSRXYAB].name	= 	"t_pphsrxyab";
  st[T_VPHSRXY].name    = 	"t_vphsrxy";
  st[T_VPHSRXYA].name	= 	"t_vphsrxya";
  st[T_VPHSRXYAB].name	= 	"t_vphsrxyab";
  
  st[T_PPHSXY].name		= "t_pphsxy";
  st[T_PPHSAB].name		= "t_pphsab";
  st[T_PPHSAX].name		= "t_pphsax";
  st[T_PPHSAY].name		= "t_pphsay";
  st[T_PPHSBX].name		= "t_pphsbx";
  st[T_PPHSBY].name		= "t_pphsby";
  
  st[T_VPHSXY].name		= "t_vphsxy";
  st[T_VPHSAB].name		= "t_vphsab";
  st[T_VPHSAX].name		= "t_vphsax";
  st[T_VPHSAY].name		= "t_vphsay";
  st[T_VPHSBX].name		= "t_vphsbx";
  st[T_VPHSBY].name		= "t_vphsby";
  st[T_MAXHT].name		= "t_maxht";
  st[T_MAXHT].type		= SHORT;

  st[T_INPP3M3].name 		= "t_inp3m3";
  st[T_INPP3M6].name 		= "t_inp3m6";
  st[T_INZPM3].name 		= "t_inzm3";
  st[T_INZPM6].name 		= "t_inzm6";
  st[T_INZ6X].name 		= "t_inx";
  st[T_INZ6Y].name 		= "t_iny";
  st[T_INZ6A].name 		= "t_ina";
  st[T_INZ6B].name 		= "t_inb";
  st[T_INPP].name 		= "t_inpp";
  st[T_INMAX].name		= "t_inmax";

  /* Hadamard transformation parameters */
  st[T_H1].name     = "t_h1";
  st[T_H2].name     = "t_h2";
  st[T_H3].name     = "t_h3";
  st[T_H4].name     = "t_h4";

  /*
   ** position parameters
   */
  st[POS_X].name		= "pos_x";
  st[POS_X].type		= SHORT;

  st[POS_Y].name		= "pos_y";
  st[POS_Y].type		= SHORT;

  st[POS_X].name		= "pos_x";
  st[POS_Y].name		= "pos_y";

  /*
   ** time
   */
  st[TIME].name			= "time";
  st[TIME].type			= DOUBLE;
  st[TIMELO].name			= "time_lo";
  st[TIMESTAMP].name			= "timestamp";
  st[TIMESTAMP].type			= ULONG;

  st[VEL].name			= "velocity";
  
}

char *SkipWhiteSpace(s)
     char *s;
{
  if(s == NULL) return(NULL);
  for(;*s != '\0';s++){
    /*
     ** look for a space delimiter
     */
    if((*s != ' ' && *s != '\t' && *s != ',')) break;
  }
  return(s);
}

char *NextDelimiter(s)
char *s;
{
    if(s == NULL) return(NULL);
    for(;*s != '\0';s++){
	/*
	** look for a space delimiter
	** which is the first white space followed by non-white space
	*/
	if((*s == ' ' || *s == '\t' || *s == ',') && 
	    (*(s+1) !=' ' && *(s+1) !='\t' && *(s+1) != '"' && *(s+1) != ',')){
	    break;
	}
    }
    return(s);
}

int GetParm(st,s)
SpikeParm	*st;
char	*s;
{
int	i;

    if(s == NULL) return(-1);
    for(i=0;i<NPARMS;i++){
	if(st[i].name != NULL){
	    if(strcmp(s,st[i].name) == 0){
		return(i);
	    }
	}
    }
    return(-1);
}

void SetParmMask(st,argstr)
SpikeParm	*st;
char	*argstr;
{
char	*eptr;
int	done = 0;
int	parmindex;
char	line[1000];
char	*maskstr;

    /*
    ** copy the string argument to a safe place
    */
    strcpy(line,argstr);
    maskstr = line;
    maskstr = SkipWhiteSpace(maskstr);
    while(!done){
	eptr = NextDelimiter(maskstr);
	if(*eptr == '\0'){
	    done = 1;
	}
	*eptr = '\0';
	parmindex = GetParm(st,maskstr);
	if(parmindex < 0){
	    fprintf(stderr,"invalid parameter '%s'\n",maskstr);
	} else {
	    parm_list[nparms++] = parmindex;
	}
	maskstr = eptr+1;
    }
}


/*
*********************************************************
**              INPUT ROUTINES
*********************************************************
*/
int BinaryDualGetSpike(result,data1,data2,fp,timestamp)
Result		*result;
int		*data1;
int		*data2;
FILE	*fp;
unsigned long	*timestamp;
{
short	rec[ST_REC_SIZE];
int		i;

    /*
    ** read in the data
    */
    if(fread(timestamp,sizeof(unsigned long),1,fp) != 1){
	return(0);
    }
    if(fread(rec,sizeof(short),ST_REC_SIZE,fp) != ST_REC_SIZE){
	return(0);
    }
    /*
    ** is the timestamp within range?
    */
    if(CheckTimestampRange(*timestamp,result) == 0){
	return(0);
    }
    for(i=0;i<ST_REC_SIZE/2;i++){
	data1[i] = rec[2*i] -spkvoffset;		/* x channel */
	data2[i] = rec[2*i +1] -spkvoffset;		/* y channel */
    }
    /* 
    ** return the number of points in the spike 
    */
    return(ST_REC_SIZE/2);
}

int BinaryTetGetSpike(result,data1,data2,data3,data4,fp,timestamp)
Result		*result;
int		*data1;
int		*data2;
int		*data3;
int		*data4;
FILE	*fp;
unsigned long	*timestamp;
{
short	rec[TT_REC_SIZE];
int		i;

    if(fread(timestamp,sizeof(unsigned long),1,fp) != 1){
	return(0);
    }
    if(fread(rec,sizeof(short),TT_REC_SIZE,fp) != TT_REC_SIZE){
	return(0);
    }
    /*
    ** is the timestamp within range?
    */
    if(CheckTimestampRange(*timestamp,result) == 0){
	return(0);
    }
    for(i=0;i<TT_REC_SIZE/4;i++){
	data1[i] = rec[4*i] -spkvoffset;		/* x channel */
	data2[i] = rec[4*i +1] -spkvoffset;		/* y channel */
	data3[i] = rec[4*i +2] -spkvoffset;		/* a channel */
	data4[i] = rec[4*i +3] -spkvoffset;		/* b channel */
    }
    /* 
    ** return the number of points in the spike 
    */
    return(TT_REC_SIZE/4);
}

int DualGetSpike(data1,data2,fp)
int		*data1;
int		*data2;
FILE	*fp;
{
int		npts;
char	line[MAXLINE];
int		nargs;
int		done;
char		*ptr;
char		*eptr;

    /*
    ** read in the data
    */
    done = 0;
    npts = 0;
    while(!done){
	if(fgets(line,MAXLINE,fp) == NULL){	/* read in a line of data */
	    break;
	}
	if(line[0] == '%') continue;		/* ignore comments */
	if(line[0] == '/') continue;		/* and xplot commands */

	/* get the data point */
	nargs = 1;
	ptr = SkipWhiteSpace(line);
	while(nargs > 0){
	    eptr = NextDelimiter(ptr);
	    if(*eptr == '\0'){
		done = 1;
		break;
	    }
	    *eptr = '\0';
	    data1[npts] = atoi(ptr);
	    ptr = eptr+1;
	    eptr = NextDelimiter(ptr);
	    if(*eptr == '\0'){
		fprintf(stderr,"Odd number of data points\n");
		done = 1;
		break;
	    }
	    data2[npts] = atoi(ptr);
	    ptr = eptr+1;
	    /*
	    fprintf(stderr,"(%d %d)",data1[npts],data2[npts]);
	    */
	    npts++;
	    if(npts > MAX_SPIKE_LEN){	/* check for data size */
		fprintf(stderr,
		"number of points in data (%d) exceeded maximum (%d)\n",
		npts,MAX_SPIKE_LEN);
		return(0);
	    }
	}
    }
    /* return the number of points in the spike */
    return(npts);
}

int TetGetSpike(data1,data2,data3,data4,fp)
int		*data1;
int		*data2;
int		*data3;
int		*data4;
FILE	*fp;
{
int		npts;
char	line[MAXLINE];
int		nargs;
int		done;
char		*ptr;
char		*eptr;

    /*
    ** read in the data
    */
    done = 0;
    npts = 0;
    while(!done){
	if(fgets(line,MAXLINE,fp) == NULL){	/* read in a line of data */
	    break;
	}
	if(line[0] == '%') continue;		/* ignore comments */
	if(line[0] == '/') continue;		/* and xplot commands */

	/* get the data point */
	nargs = 1;
	ptr = SkipWhiteSpace(line);
	while(nargs > 0){
	    eptr = NextDelimiter(ptr);
	    if(*eptr == '\0'){
		done = 1;
		break;
	    }
	    *eptr = '\0';
	    data1[npts] = atoi(ptr);
	    ptr = eptr+1;
	    eptr = NextDelimiter(ptr);
	    if(*eptr == '\0'){
		fprintf(stderr,"Odd number of data points\n");
		done = 1;
		break;
	    }
	    data2[npts] = atoi(ptr);
	    ptr = eptr+1;
	    eptr = NextDelimiter(ptr);
	    if(*eptr == '\0'){
		fprintf(stderr,"Odd number of data points\n");
		done = 1;
		break;
	    }
	    data3[npts] = atoi(ptr);
	    ptr = eptr+1;
	    eptr = NextDelimiter(ptr);
	    if(*eptr == '\0'){
		fprintf(stderr,"Odd number of data points\n");
		done = 1;
		break;
	    }
	    data4[npts] = atoi(ptr);
	    ptr = eptr+1;
	    npts++;
	    if(npts > MAX_SPIKE_LEN){	/* check for data size */
		fprintf(stderr,
		"number of points in data (%d) exceeded maximum (%d)\n",
		npts,MAX_SPIKE_LEN);
		return(0);
	    }
	}
    }
    /* return the number of points in the spike */
    return(npts);
}

int AsciiGetSpike(data,fp)
int		*data;
FILE	*fp;
{
int		npts;
char	line[MAXLINE];
int		ival[2];
int		nargs;

    npts = 0;
    /*
    ** read in the data
    */
    while(!feof(fp)){
	if(fgets(line,MAXLINE,fp) == NULL){	/* read in a line of data */
	    break;
	}
	if(strncmp(line,"%END",4) == 0){	/* end of record indicator */
	    break;
	}
	if(line[0] == '%') continue;		/* ignore comments */
	if(line[0] == '/') continue;		/* and xplot commands */

	/* get the data point */
	nargs = sscanf(line,"%d %d",ival+1,ival+2);
	if(nargs == 1) {
	    data[npts++] = ival[1];
	} else 
	if(nargs == 2) {
	    data[npts++] = ival[2];
	}
	if(npts >= MAX_SPIKE_LEN){	/* check for data size */
	    /*
	    fprintf(stderr,
	    "number of points in data exceeded maximum (%d)\n",MAX_SPIKE_LEN);
	    */
	    return(npts);
	}
    }
	/* return the number of points in the spike */
    return(npts);
}

Template *AddTemplate(template,fp,filename,mode)
Template	*template;
FILE		*fp;
char		*filename;
int		mode;
{
Template	*newtemplate;
Template	*t;
double		fval;
char		line[100];
int		nargs;
double		tmpdata[MAXTEMPLATESIZE];
short		tmpmask[MAXTEMPLATESIZE];
int		npts;
int		j;

    /*
    ** allocate and initialize the template
    */
    newtemplate = (Template *)calloc(1,sizeof(Template));
    strcpy(newtemplate->filename,filename);

    /*
    ** read in the template data
    */
    newtemplate->npts = 0;
    newtemplate->mode = mode;
    npts = 0;
    while(!feof(fp)){
	if(fgets(line,100,fp) == NULL){		/* read in a line of data */
	    break;
	}
	if(line[0] == '%') continue;		/* ignore comments */
	if(line[0] == '/') continue;		/* and xplot commands */
	if(line[0] == '@'){	 		/* masked point */
	    tmpmask[npts] = 0;
	    fval = 0;
	    nargs = 1;
	} else {
	    tmpmask[npts] = 1;
	    /* get the data point */
	    nargs = sscanf(line,"%lf",&fval);
	}
	if(nargs > 0) {
	    tmpdata[npts++] = fval;
	}
	if(npts > MAXTEMPLATESIZE){	/* check for template size */
	    fprintf(stderr,
	    "number of points in template exceeded maximum (%d)\n",
	    MAXTEMPLATESIZE);
	    return(NULL);
	}
    }
    newtemplate->npts = npts;
    newtemplate->value = (double *)malloc(npts*sizeof(double));
    newtemplate->mask = (short *)malloc(npts*sizeof(short));
    bcopy(tmpdata,newtemplate->value,npts*sizeof(double));
    bcopy(tmpmask,newtemplate->mask,npts*sizeof(short));
    newtemplate->magnitude = 0;
    for(j=0;j<newtemplate->npts;j++){
	newtemplate->magnitude += newtemplate->value[j]*newtemplate->value[j]
	    *newtemplate->mask[j];
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
*********************************************************
**              OUTPUT ROUTINES
*********************************************************
*/
void WriteSpikeParms(n,st,fp,format)
int		n;
SpikeParm	*st;
FILE		*fp;
int		format;
{
int 	i;
float	fval;
double	dval;
int	ival;
short	sval;
char	cval;
unsigned long	uval;

    if(nparms == 0) return;
    if(format == BINARY){
	/*
	** spike id
	*/
	fwrite(&n,sizeof(int),1,fp);
	/*
	** spike parameters
	*/
	for(i=0;i<nparms;i++){
	    switch(st[parm_list[i]].type){
	    case ULONG:
		uval = st[parm_list[i]].parm;
		fwrite(&uval,sizeof(unsigned long),1,fp);
		break;
	    case DOUBLE:
		dval = st[parm_list[i]].parm;
		fwrite(&dval,sizeof(double),1,fp);
		break;
	    case SHORT:
		sval = st[parm_list[i]].parm;
		fwrite(&sval,sizeof(short),1,fp);
		break;
	    case INT:
		ival = st[parm_list[i]].parm;
		fwrite(&ival,sizeof(int),1,fp);
		break;
	    case CHAR:
		cval = st[parm_list[i]].parm;
		fwrite(&cval,sizeof(char),1,fp);
		break;
	    default:
		fval = st[parm_list[i]].parm;
		fwrite(&fval,sizeof(float),1,fp);
		break;
	    }
	}
    } else {
	fprintf(fp,"%d\t",n);
	for(i=0;i<nparms;i++){
	    fprintf(fp,"%.16g\t",st[parm_list[i]].parm);
	}
	fprintf(fp,"\n");
    }
}

void WriteStereoIdentifiers(result,st,fp)
Result		*result;
SpikeParm	*st;
FILE		*fp;
{
int i;
int	count;
Template	*template;

    for(i=0;i<nparms;i++){
	if((parm_list[i] >= TEMPLATE0) && 
	(parm_list[i] < MAXTEMPLATES + TEMPLATE0)){
	    count = 0;
	    for(template=templatelist;template;template=template->next){
		if(count == (parm_list[i] - TEMPLATE0)) break;
		count++;
	    }
	    if(result->outputformat == BINARY){
		switch(st[parm_list[i]].type){
		case ULONG:
		    fprintf(fp,"%8s:%s,%d,%d,%d\t",
			st[parm_list[i]].name,
			template->filename,
			ULONG,sizeof(unsigned long),1);
		    break;
		case DOUBLE:
		    fprintf(fp,"%8s:%s,%d,%d,%d\t",
			st[parm_list[i]].name,
			template->filename,
			DOUBLE,sizeof(double),1);
		    break;
		case INT:
		    fprintf(fp,"%8s:%s,%d,%d,%d\t",
			st[parm_list[i]].name,
			template->filename,
			INT,sizeof(int),1);
		    break;
		case SHORT:
		    fprintf(fp,"%8s:%s,%d,%d,%d\t",
			st[parm_list[i]].name,
			template->filename,
			SHORT,sizeof(short),1);
		    break;
		case CHAR:
		    fprintf(fp,"%8s:%s,%d,%d,%d\t",
			st[parm_list[i]].name,
			template->filename,
			CHAR,sizeof(char),1);
		    break;
		default:
		    fprintf(fp,"%8s:%s,%d,%d,%d\t",
			st[parm_list[i]].name,
			template->filename,
			FLOAT,sizeof(float),1);
		    break;
		}
	    } else {
		fprintf(fp,"%8s:%s\t",
		    st[parm_list[i]].name,
		    template->filename);
	    }
	} else {
	    if(result->outputformat == BINARY){
		switch(st[parm_list[i]].type){
		case ULONG:
		    fprintf(fp,"%8s,%d,%d,%d\t",
			st[parm_list[i]].name,
			ULONG,sizeof(unsigned long),1);
		    break;
		case DOUBLE:
		    fprintf(fp,"%8s,%d,%d,%d\t",
			st[parm_list[i]].name,
			DOUBLE,sizeof(double),1);
		    break;
		case INT:
		    fprintf(fp,"%8s,%d,%d,%d\t",
			st[parm_list[i]].name,
			INT,sizeof(int),1);
		    break;
		case SHORT:
		    fprintf(fp,"%8s,%d,%d,%d\t",
			st[parm_list[i]].name,
			SHORT,sizeof(short),1);
		    break;
		case CHAR:
		    fprintf(fp,"%8s,%d,%d,%d\t",
			st[parm_list[i]].name,
			CHAR,sizeof(char),1);
		    break;
		default:
		    fprintf(fp,"%8s,%d,%d,%d\t",
			st[parm_list[i]].name,
			FLOAT,sizeof(float),1);
		    break;
		}
	    }else {
		fprintf(fp,"%8s\t",
		st[parm_list[i]].name);
	    }
	}
    }
    fprintf(fp,"\n");
}


void WriteSingleParms(se,fp)
{}

void WriteBWSingleParms(se,fp)
BWSingleParms	*se;
FILE		*fp;
{
    fprintf(fp,"%5d %5d %5d %5d %5d %5d\n",
    se->peak,
    se->valley,
    se->peak_time,
    se->valley_time,
    se->spike_height,
    se->spike_width);
}

int Offset(max,tpeak,offset)
int	max;
int	tpeak;
int	offset;
{
int	parmindex;

    parmindex = tpeak + offset;
    if(parmindex < 0) parmindex = 0;
    if(parmindex >= max) parmindex = max-1;
    return(parmindex);
}

float Phase(y,x)
float	y,x;
{
float	angle;

    if((x == 0) && (y == 0)){
	return(45.0);
    }
    if((x == 0) && (y > 0)){
	return(90.0);
    }
    if((x == 0) && (y < 0)){
	return(270.0);
    }
    angle = 360*atan((double)y/(double)x)/(2*M_PI);
    if(x < 0){		/* quadrant II  & III*/
	angle += 180;
    } 
    return(angle);
}

double SPhase(x,r)
double x,r;
{
    if(r == 0){
	return(45.0);
    } else {
	return(360*asin(x/r)/(2*M_PI));
    }
}

#define MAXRATIO 3

float ZIntegralRatio(channel,offset,zcross)
SpikeChannel	*channel;
int		offset;
float		zcross;
{
int	i;
float	mintegral;
float	pintegral;
float	start;
float	end;
float	ratio;

    mintegral = 0;
    pintegral = 0;
    start = zcross - offset;
    end = zcross + offset + 1;
    if(start < 0) start = 0;
    if(end > channel->len) end = channel->len;
    for(i=start;i<zcross;i++){
	mintegral += channel->data[i];
    }
    for(i=zcross;i<end;i++){
	pintegral += channel->data[i];
    }
    if(mintegral == 0){
	if(pintegral == 0){
	    ratio = 1;
	} else {
	    if(pintegral > 0){
		ratio = MAXRATIO;
	    } else {
		ratio = -MAXRATIO;
	    }
	}
    } else {
	ratio = pintegral/mintegral;
    }
    if(ratio > MAXRATIO) ratio = MAXRATIO;
    if(ratio < -MAXRATIO) ratio = -MAXRATIO;
    return(ratio);
}

float Integral(channel,start,end)
SpikeChannel	*channel;
int		start,end;
{
int	i;
float	integral;

    integral = 0;
    if(start < 0) start = 0;
    if(end > channel->len) end = channel->len;
    for(i=start;i<end;i++){
	integral += channel->data[i];
    }
    return(integral);
}

/*
*********************************************************
**              CORE ROUTINES
*********************************************************
*/
float DiffTemplateMatch(xchannel,ychannel,template)
SpikeChannel	*xchannel;
SpikeChannel	*ychannel;
Template	*template;
{
int	j;
float	diff;
float	diffx,diffy;

    diffx = 0;
    diffy = 0;
    diff = 0;
    /*
    ** template match x component
    */
    for(j=0;j<xchannel->len;j++){
	if(template->mask[j]){
	    diffx += sqrt((double)(xchannel->data[j]-template->value[j])
		    *(xchannel->data[j]-template->value[j]));
	}
    }
    /*
    ** template match y component
    */
    if(template->npts > xchannel->len){
	/*
	** use y template values
	*/
	for(j=0;j<ychannel->len;j++){
	    if(template->mask[j+xchannel->len]){
		diffy += sqrt((double)
		(ychannel->data[j]-template->value[j+xchannel->len])
		*(ychannel->data[j]-template->value[j+xchannel->len]));
	    }
	}
    } else {
	/*
	** reuse x template values
	*/
	for(j=0;j<ychannel->len;j++){
	    if(template->mask[j]){
		diffy += sqrt((double)(ychannel->data[j]-template->value[j])
		*(ychannel->data[j]-template->value[j]));
	    }
	}
    }
    /*
    ** use the total difference
    */
    if(template->npts > xchannel->len){
	diff = diffx + diffy;
    } else {
	/*
	** use the min difference
	*/
	if(diffy < diffx){
	    diff = diffy;
	} else {
	    diff = diffx;
	}
    }
    return(diff);
}

float TemplateMatch(xchannel,ychannel,template)
SpikeChannel	*xchannel;
SpikeChannel	*ychannel;
Template	*template;
{
int	j;
float 	magy;
float 	magx;
float	corr;
float	corrx,corry;

    /*
    ** compute the magnitude of the waveform
    */
    magx = 0;
    magy = 0;
    for(j=0;j<xchannel->len;j++){
	magx += xchannel->data[j]*xchannel->data[j]*template->mask[j];
    }
    for(j=0;j<ychannel->len;j++){
	if(template->npts > xchannel->len){
	    magy += ychannel->data[j]*ychannel->data[j]
		*template->mask[j+xchannel->len];
	} else {
	    magy += ychannel->data[j]*ychannel->data[j]*template->mask[j];
	}
    }
    corrx = 0;
    corry = 0;
    corr = 0;
    /*
    ** template match x component
    */
    for(j=0;j<xchannel->len;j++){
	if(template->mask[j]){
	    corrx += xchannel->data[j]*template->value[j];
	}
    }
    /*
    ** template match y component
    */
    if(template->npts > xchannel->len){
	/*
	** use y template values
	*/
	for(j=0;j<ychannel->len;j++){
	    if(template->mask[j+xchannel->len]){
		corry += ychannel->data[j]*template->value[j+xchannel->len];
	    }
	}
    } else {
	/*
	** reuse x template values
	*/
	for(j=0;j<ychannel->len;j++){
	    if(template->mask[j]){
		corry += ychannel->data[j]*template->value[j];
	    }
	}
    }
    /*
    ** use the total correlation
    */
    if(template->npts > xchannel->len){
	if(template->mode == DOTTEMPLATE){
	    corr = corrx + corry;
	} else {
	    corr = (corrx + corry)/
	    (sqrt(magx + magy)*sqrt(template->magnitude));
	}
    } else {
	/*
	** use the max correlation
	*/
	if(corrx > corry){
	    if(template->mode == DOTTEMPLATE){
		corr = corrx;
	    } else {
		corr = corrx/(sqrt(magx)*sqrt(template->magnitude));
	    }
	} else {
	    if(template->mode == DOTTEMPLATE){
		corr = corry;
	    } else {
		corr = corry/(sqrt(magy)*sqrt(template->magnitude));
	    }
	}
    }
    return(corr);
}

void ExtractStereoParms(xchannel,ychannel,st,analyze)
SpikeChannel	*xchannel;
SpikeChannel	*ychannel;
SpikeParm	*st;
int		analyze;
{
int		*data;
int		peak;
int		tpeak;
int		i;
int		n;
double		ss;
double		avg;
float		phs;
int		valley;
int		tvalley;
SpikeChannel	*channel;
float		weight,sw;
Template	*template;
int		tsearch;

    /* 
    ** find the peak over both channels to determine peak time 
    */ 

    /* 
    ** scan the x channel for peak 
    */
    peak = xchannel->data[0];
    tpeak = 0;
    data = xchannel->data;
    for(i=1;i<xchannel->len;i++){
	if(data[i] > peak){
	    peak = data[i];
	    tpeak = i;
	}
    }
    
    /* 
    ** scan the y channel for peak 
    */
    data = ychannel->data;
    for(i=1;i<ychannel->len;i++){
	if(data[i] > peak){
	    peak = data[i];
	    tpeak = i;
	}
    }
    /*
    ** fill the parms using amplitude values at the specified offsets
    ** from the peak
    */
    st[XM4].parm = xchannel->data[Offset(xchannel->len,tpeak,-4)];
    st[XM2].parm = xchannel->data[Offset(xchannel->len,tpeak,-2)];
    st[X0].parm = xchannel->data[Offset(xchannel->len,tpeak,0)];
    st[XP2].parm = xchannel->data[Offset(xchannel->len,tpeak,2)];
    st[XP4].parm = xchannel->data[Offset(xchannel->len,tpeak,4)];
    st[XP8].parm = xchannel->data[Offset(xchannel->len,tpeak,8)];
    st[XP12].parm = xchannel->data[Offset(xchannel->len,tpeak,12)];

    st[YM4].parm = ychannel->data[Offset(ychannel->len,tpeak,-4)];
    st[YM2].parm = ychannel->data[Offset(ychannel->len,tpeak,-2)];
    st[Y0].parm = ychannel->data[Offset(ychannel->len,tpeak,0)];
    st[YP2].parm = ychannel->data[Offset(ychannel->len,tpeak,2)];
    st[YP4].parm = ychannel->data[Offset(ychannel->len,tpeak,4)];
    st[YP8].parm = ychannel->data[Offset(ychannel->len,tpeak,8)];
    st[YP12].parm = ychannel->data[Offset(ychannel->len,tpeak,12)];

    st[T0].parm = tpeak;

    st[PHSM4].parm = Phase(st[YM4].parm,st[XM4].parm);
    st[PHSM2].parm = Phase(st[YM2].parm,st[XM2].parm);
    st[PHS0].parm = Phase(st[Y0].parm,st[X0].parm);
    st[PHSP2].parm = Phase(st[YP2].parm,st[XP2].parm);
    st[PHSP4].parm = Phase(st[YP4].parm,st[XP4].parm);
    st[PHSP8].parm = Phase(st[YP8].parm,st[XP8].parm);
    st[PHSP12].parm = Phase(st[YP12].parm,st[XP12].parm);

    avg = 0;
    ss = 0;
    sw = 0;
    for(i=0;i<xchannel->len;i++){
	sw += abs(ychannel->data[i] + xchannel->data[i]);
    }
    for(i=0;i<xchannel->len;i++){
	weight = abs(ychannel->data[i] + xchannel->data[i])/sw;
	phs = weight*
	    Phase((float)(ychannel->data[i]),(float)(xchannel->data[i]));
	avg += phs;
	ss += phs*phs;
    }
    st[AVGPHS].parm = avg;
    n = xchannel->len;
    st[SDPHS].parm = sqrt((double)(n*ss - avg*avg)/((double)n*(n-1)));

    if(st[X0].parm > st[Y0].parm){
	st[PAMP].parm = st[X0].parm;
	channel = xchannel;
    } else {
	st[PAMP].parm = st[Y0].parm;
	channel = ychannel;
    }
    st[ZCROSS].parm = channel->len-1;	/* last point by default */
    for(i=tpeak;i<channel->len-1;i++){
	/*
	** find the first negative slope zero crossing relative to peak
	*/
	if((channel->data[i] >= 0) && (channel->data[i+1] <= 0)){
	    /* interpolate zero crossing */
	    st[ZCROSS].parm = i + 
	    (float)channel->data[i]/
	    (float)(channel->data[i] - channel->data[i+1]);
	    break;
	}
    }
    /*
    ** compute zerocrossing relative integrals
    */
    /*
    ** ratio of post zerocrossing integral to pre
    */
    st[INZPM].parm = ZIntegralRatio(channel,channel->len,st[ZCROSS].parm);
    st[INZP2M2].parm = ZIntegralRatio(channel,2,st[ZCROSS].parm);
    st[INZP4M4].parm = ZIntegralRatio(channel,4,st[ZCROSS].parm);
    st[INZP8M8].parm = ZIntegralRatio(channel,8,st[ZCROSS].parm);
    /*
    ** peak relative integral ratio
    */
    st[INPM3M6].parm = ZIntegralRatio(channel,3,(float)(tpeak-3));
    st[INPP12M12].parm = ZIntegralRatio(channel,12,(float)tpeak);

    /*
    ** template matches
    */
    i = 0;
    for(template=templatelist;template;template=template->next){
	if(template->mode == DIFFTEMPLATE){
	    st[TEMPLATE0 + i].parm = DiffTemplateMatch(xchannel,ychannel,template);
	} else {
	    st[TEMPLATE0 + i].parm = TemplateMatch(xchannel,ychannel,template);
	}
	i++;
	if(i >= MAXTEMPLATES) break;
    }

    /*
    ** --------------------------------------
    ** Compute Brainwave (tm) parameters
    ** --------------------------------------
    */
    /* 
    ** scan the x channel for peak and valley 
    */
    tpeak = trigger_pt;
    peak = xchannel->data[tpeak];
    data = xchannel->data;
    for(i=tpeak+1;i<xchannel->len;i++){
	if(data[i] > peak){
	    peak = data[i];
	    tpeak = i;
	}
    }
    /*
    ** find the first peak following the trigger pt
    */
    data = xchannel->data;
    /* 
    ** if no inflection is found then use the last  point
    */
    tsearch = xchannel->len-1;	
    for(i=trigger_pt-1;i<xchannel->len-1;i++){
	if(data[i+1] <= data[i]){
	    tsearch = i;
	    break;
	}
    }
    /*
    ** Brainwave starts the valley search at pt trigger_pt
    */
    /*
    ** use the first peak to start the search
    */
    tvalley = tsearch;
    valley = xchannel->data[tvalley];
    for(i=tvalley+1;i<xchannel->len;i++){
	if(data[i] < valley){
	    valley = data[i];
	    tvalley = i;
	}
    }

    st[PX].parm = peak;
    st[TPX].parm = tpeak;
    st[VX].parm = valley;
    st[TVX].parm = tvalley;

    /*
    ** find the first peak following the trigger pt
    */
    data = ychannel->data;
    /* 
    ** if no inflection is found then use the last  point
    */
    tsearch = ychannel->len-1;	
    for(i=trigger_pt-1;i<ychannel->len-1;i++){
	if(data[i+1] <= data[i]){
	    tsearch = i;
	    break;
	}
    }
    /* 
    ** scan the y channel for peak and valley 
    */
    tpeak = trigger_pt;
    peak = ychannel->data[tpeak];
    data = ychannel->data;
    for(i=tpeak+1;i<ychannel->len;i++){
	if(data[i] > peak){
	    peak = data[i];
	    tpeak = i;
	}
    }
    tvalley = tsearch;
    valley = ychannel->data[tvalley];
    for(i=tvalley+1;i<ychannel->len;i++){
	if(data[i] < valley){
	    valley = data[i];
	    tvalley = i;
	}
    }
    st[PY].parm = peak;
    st[TPY].parm = tpeak;
    st[VY].parm = valley;
    st[TVY].parm = tvalley;

    if(st[PX].parm > st[PY].parm){
	st[MAXHT].parm = st[PX].parm - st[VX].parm;
    } else {
	st[MAXHT].parm = st[PY].parm - st[VY].parm;
    }

    st[PPHS].parm = BW_ATAN_FACTOR * Phase(st[PY].parm,st[PX].parm);
    st[VPHS].parm = Phase(st[VY].parm,st[VX].parm);

    st[DLAT].parm = st[TPY].parm - st[TPX].parm;

    if(st[PX].parm > st[PY].parm){
	st[MAXWD].parm = st[TVX].parm - st[TPX].parm;
    } else {
	st[MAXWD].parm = st[TVY].parm - st[TPY].parm;
    }

    /*
    ** compute magnitude
    */
    st[PMAG].parm = sqrt((double)st[PX].parm*st[PX].parm + 
    (double)st[PY].parm*st[PY].parm);

    st[VMAG].parm = sqrt((double)st[VX].parm*st[VX].parm + 
    (double)st[VY].parm*st[VY].parm);

    if(!analyze){
	/*
	** compute parameter statistics 
	*/
	for(i=0;i<NPARMS;i++){
	    if(st[i].name){
		st[i].npts++;
		st[i].sum += st[i].parm;
		st[i].sumsqr += st[i].parm*st[i].parm;
		if(nwaveforms == 0 || st[i].parm < st[i].min){
		    st[i].min = st[i].parm; 
		}
		if(nwaveforms == 0 || st[i].parm > st[i].max){
		    st[i].max = st[i].parm; 
		}
	    }
	}
	nwaveforms++;
    }
}

void ExtractTetrodeParms(xchannel,ychannel,achannel,bchannel,tt,analyze)
SpikeChannel	*xchannel;
SpikeChannel	*ychannel;
SpikeChannel	*achannel;
SpikeChannel	*bchannel;
SpikeParm	*tt;
int		analyze;
{
int		i;
int		valley;
int		peak;
int		tvalley;
int		tpeak;
double		Rxy;
double		Rxya;
double		Rxyab;
SpikeChannel	*maxchannel;
int		zcross;

    /* 
    ** scan the x channel for peak and valley 
    */
    peak = xchannel->data[0];
    tpeak = 0;
    maxchannel = xchannel;
    for(i=1;i<xchannel->len;i++){
	if(xchannel->data[i] > peak){
	    maxchannel = xchannel;
	    peak = xchannel->data[i];
	    tpeak = i;
	}
    }
    valley = xchannel->data[0];
    tvalley = 0;
    for(i=1;i<xchannel->len;i++){
	if(xchannel->data[i] < valley){
	    valley = xchannel->data[i];
	    tvalley = i;
	}
    }

    tt[T_PX].parm = peak;
    tt[T_VX].parm = valley;
    tt[T_TPX].parm = tpeak;
    tt[T_TVX].parm = tvalley;

    /* 
    ** scan the y channel for peak and valley 
    */
    peak = ychannel->data[0];
    tpeak = 0;
    for(i=1;i<ychannel->len;i++){
	if(ychannel->data[i] > peak){
	    maxchannel = ychannel;
	    peak = ychannel->data[i];
	    tpeak = i;
	}
    }
    valley = ychannel->data[0];
    tvalley = 0;
    for(i=1;i<ychannel->len;i++){
	if(ychannel->data[i] < valley){
	    valley = ychannel->data[i];
	    tvalley = i;
	}
    }

    tt[T_PY].parm = peak;
    tt[T_VY].parm = valley;
    tt[T_TPY].parm = tpeak;
    tt[T_TVY].parm = tvalley;

    /* 
    ** scan the a channel for peak and valley 
    */
    peak = achannel->data[0];
    tpeak = 0;
    for(i=1;i<achannel->len;i++){
	if(achannel->data[i] > peak){
	    peak = achannel->data[i];
	    tpeak = i;
	    maxchannel = achannel;
	}
    }
    valley = achannel->data[0];
    tvalley = 0;
    for(i=1;i<achannel->len;i++){
	if(achannel->data[i] < valley){
	    valley = achannel->data[i];
	    tvalley = i;
	}
    }

    tt[T_PA].parm = peak;
    tt[T_VA].parm = valley;
    tt[T_TPA].parm = tpeak;
    tt[T_TVA].parm = tvalley;

    /* 
    ** scan the b channel for peak and valley 
    */
    peak = bchannel->data[0];
    tpeak = 0;
    for(i=1;i<bchannel->len;i++){
	if(bchannel->data[i] > peak){
	    peak = bchannel->data[i];
	    tpeak = i;
	    maxchannel = bchannel;
	}
    }
    valley = bchannel->data[0];
    tvalley = 0;
    for(i=1;i<bchannel->len;i++){
	if(bchannel->data[i] < valley){
	    valley = bchannel->data[i];
	    tvalley = i;
	}
    }

    tt[T_PB].parm = peak;
    tt[T_VB].parm = valley;
    tt[T_TPB].parm = tpeak;
    tt[T_TVB].parm = tvalley;


    /* Hadamard transformation parameters */
    /* the matrix is
            1  1 -1 -1
            1 -1  1 -1
            1 -1 -1  1
            1  1  1  1 */

    tt[T_H1].parm = tt[T_PX].parm + tt[T_PY].parm - tt[T_PA].parm - tt[T_PB].parm;
    tt[T_H2].parm = tt[T_PX].parm - tt[T_PY].parm + tt[T_PA].parm - tt[T_PB].parm;
    tt[T_H3].parm = tt[T_PX].parm - tt[T_PY].parm - tt[T_PA].parm + tt[T_PB].parm;
    tt[T_H4].parm = tt[T_PX].parm + tt[T_PY].parm + tt[T_PA].parm + tt[T_PB].parm;

    /*
    ** get width from max peak channel
    */
    if((tt[T_PX].parm >= tt[T_PY].parm) && 
    (tt[T_PX].parm >= tt[T_PA].parm) && 
    (tt[T_PX].parm >= tt[T_PB].parm)){
	tt[T_MAXWD].parm = tt[T_TVX].parm - tt[T_TPX].parm;
	tpeak = tt[T_TPX].parm;
	tvalley = tt[T_TVX].parm;
    } else
    if((tt[T_PY].parm >= tt[T_PX].parm) && 
    (tt[T_PY].parm >= tt[T_PA].parm) && 
    (tt[T_PY].parm >= tt[T_PB].parm)){
	tt[T_MAXWD].parm = tt[T_TVY].parm - tt[T_TPY].parm;
	tpeak = tt[T_TPY].parm;
	tvalley = tt[T_TVY].parm;
    } else
    if((tt[T_PA].parm >= tt[T_PX].parm) 
    && (tt[T_PA].parm>= tt[T_PY].parm) && 
    (tt[T_PA].parm >= tt[T_PB].parm)){
	tt[T_MAXWD].parm = tt[T_TVA].parm - tt[T_TPA].parm;
	tpeak = tt[T_TPA].parm;
	tvalley = tt[T_TVA].parm;
    } else
    if((tt[T_PB].parm >= tt[T_PX].parm) && 
    (tt[T_PB].parm >= tt[T_PY].parm) && 
    (tt[T_PB].parm >= tt[T_PA].parm)){
	tt[T_MAXWD].parm = tt[T_TVB].parm - tt[T_TPB].parm;
	tpeak = tt[T_TPB].parm;
	tvalley = tt[T_TVB].parm;
    }
    /*
    ** compute maximum spike height
    */
    tt[T_MAXHT].parm = tt[T_PX].parm - tt[T_VX].parm;
    if(tt[T_PY].parm - tt[T_VY].parm > tt[T_MAXHT].parm){
	tt[T_MAXHT].parm = tt[T_PY].parm - tt[T_VY].parm;
    }
    if(tt[T_PA].parm - tt[T_VA].parm > tt[T_MAXHT].parm){
	tt[T_MAXHT].parm = tt[T_PA].parm - tt[T_VA].parm;
    }
    if(tt[T_PB].parm - tt[T_VB].parm > tt[T_MAXHT].parm){
	tt[T_MAXHT].parm = tt[T_PB].parm - tt[T_VB].parm;
    }

    /* 
    ** get the zero crossing from the max channel
    */
    zcross = achannel->len-1;
    for(i=tpeak;i<achannel->len;i++){
	if(maxchannel->data[i] < 0){
	    zcross = i-1;
	}
    }

    /*
    ** compute integrals
    */
    tt[T_INPP3M3].parm = Integral(maxchannel,tpeak-3,tpeak+3);
    tt[T_INPP3M6].parm = Integral(maxchannel,tpeak-6,tpeak+3);
    tt[T_INZPM3].parm = Integral(maxchannel,tpeak-3,zcross);
    tt[T_INZPM6].parm = Integral(maxchannel,tpeak-6,zcross);
    tt[T_INZ6X].parm = Integral(xchannel,tpeak-3,tpeak+4);
    tt[T_INZ6Y].parm = Integral(ychannel,tpeak-3,tpeak+4);
    tt[T_INZ6A].parm = Integral(achannel,tpeak-3,tpeak+4);
    tt[T_INZ6B].parm = Integral(bchannel,tpeak-3,tpeak+4);
    tt[T_INPP].parm = Integral(maxchannel,tpeak-8,tpeak-4);

    tt[T_INMAX].parm = Integral(maxchannel,tpeak-3,tpeak+4);

    tt[T_TPMAX].parm = tpeak;
    tt[T_TVMAX].parm = tvalley;

    /*
    ** compute magnitude
    */
    tt[T_PMAG].parm = sqrt((double)tt[T_PX].parm*tt[T_PX].parm +
	(double)tt[T_PY].parm*tt[T_PY].parm +
	(double)tt[T_PA].parm*tt[T_PA].parm +
	(double)tt[T_PB].parm*tt[T_PB].parm);

    tt[T_VMAG].parm = sqrt((double)tt[T_VX].parm*tt[T_VX].parm +
	(double)tt[T_VY].parm*tt[T_VY].parm +
	(double)tt[T_VA].parm*tt[T_VA].parm +
	(double)tt[T_VB].parm*tt[T_VB].parm);

    /*
    ** compute peak phase angles
    */
    tt[T_PPHSXY].parm = Phase(tt[T_PY].parm,tt[T_PX].parm);
    tt[T_PPHSAB].parm = Phase(tt[T_PB].parm,tt[T_PA].parm);
    tt[T_PPHSAX].parm = Phase(tt[T_PX].parm,tt[T_PA].parm);
    tt[T_PPHSAY].parm = Phase(tt[T_PY].parm,tt[T_PA].parm);
    tt[T_PPHSBX].parm = Phase(tt[T_PX].parm,tt[T_PB].parm);
    tt[T_PPHSBY].parm = Phase(tt[T_PY].parm,tt[T_PB].parm);

    tt[T_VPHSXY].parm = Phase(tt[T_VY].parm,tt[T_VX].parm);
    tt[T_VPHSAB].parm = Phase(tt[T_VB].parm,tt[T_VA].parm);
    tt[T_VPHSAX].parm = Phase(tt[T_VX].parm,tt[T_VA].parm);
    tt[T_VPHSAY].parm = Phase(tt[T_VY].parm,tt[T_VA].parm);
    tt[T_VPHSBX].parm = Phase(tt[T_VX].parm,tt[T_VB].parm);
    tt[T_VPHSBY].parm = Phase(tt[T_VY].parm,tt[T_VB].parm);

    Rxy = sqrt((double)tt[T_PX].parm*tt[T_PX].parm +
	(double)tt[T_PY].parm*tt[T_PY].parm);
    Rxya = sqrt((double)tt[T_PX].parm*tt[T_PX].parm +
	(double)tt[T_PY].parm*tt[T_PY].parm +
	(double)tt[T_PA].parm*tt[T_PA].parm);
    Rxyab = tt[T_PMAG].parm;

    tt[T_PPHSRXY].parm = SPhase((double)tt[T_PY].parm,Rxy);
    tt[T_PPHSRXYA].parm = SPhase((double)tt[T_PA].parm,Rxya);
    tt[T_PPHSRXYAB].parm = SPhase((double)tt[T_PB].parm,Rxyab);

    /*
    ** compute valley phase angles
    */
    Rxy = sqrt((double)tt[T_VX].parm*tt[T_VX].parm +
	(double)tt[T_VY].parm*tt[T_VY].parm);
    Rxya = sqrt((double)tt[T_VX].parm*tt[T_VX].parm +
	(double)tt[T_VY].parm*tt[T_VY].parm +
	(double)tt[T_VA].parm*tt[T_VA].parm);
    Rxyab = tt[T_VMAG].parm;

    tt[T_VPHSRXY].parm = SPhase((double)tt[T_VY].parm,Rxy);
    tt[T_VPHSRXYA].parm = SPhase((double)tt[T_VA].parm,Rxya);
    tt[T_VPHSRXYAB].parm = SPhase((double)tt[T_VB].parm,Rxyab);

    if(!analyze){
	/*
	** compute parameter statistics 
	*/
	for(i=0;i<NPARMS;i++){
	    if(tt[i].name){
		tt[i].npts++;
		tt[i].sum += tt[i].parm;
		tt[i].sumsqr += tt[i].parm*tt[i].parm;
		if(nwaveforms == 0 || tt[i].parm < tt[i].min){
		    tt[i].min = tt[i].parm; 
		}
		if(nwaveforms == 0 || tt[i].parm > tt[i].max){
		    tt[i].max = tt[i].parm; 
		}
	    }
	}
	nwaveforms++;
    }
}

void ExtractSingleParms(xchannel,separms)
SpikeChannel	*xchannel;
SingleParms		*separms;
{
}

void ExtractBWSingleParms(xchannel,separms)
SpikeChannel	*xchannel;
BWSingleParms		*separms;
{
int		*data;
int		peak;
int		tpeak;
int		valley;
int		tvalley;
int		i;

	/* 
	** scan the x channel for peak and valley 
	*/
	peak = xchannel->data[0];
	valley = xchannel->data[0];
	tpeak = 0;
	tvalley = 0;
	data = xchannel->data;
	for(i=1;i<xchannel->len;i++){
		if(data[i] > peak){
			peak = data[i];
			tpeak = i;
		}
		if(data[i] < valley){
			valley = data[i];
			tvalley = i;
		}
	}
	separms->peak = peak;
	separms->peak_time = tpeak;
	separms->valley = valley;
	separms->valley_time = tvalley;
	
	separms->spike_height = separms->peak - separms->valley;
	separms->spike_width = separms->valley_time - separms->peak_time;
}

void ProcessSpikes(result,sparms,xchannel,ychannel,achannel,bchannel,
	      start,end,analyze,idx)
     Result		*result;
     SpikeParm	*sparms;
     SpikeChannel	*xchannel;
     SpikeChannel	*ychannel;
     SpikeChannel	*achannel;
     SpikeChannel	*bchannel;
     int		start;
     int		end;
     int		analyze;
     Index		*idx;
{
  int	first;
  int	count;
  int	processed_count;
  unsigned long	 timestamp;
  unsigned long	 etimestamp;
  unsigned long	 stimestamp;
  int		processed;
  int		i;
  int		bin;
  OldPosRecord	oldpos;
  PosRecord	pos;
  int		oldx,oldy;
  int           posx, posy;
  unsigned long oldtimestamp;
  double	*vqueue;
  double	*pxqueue;
  double	*pyqueue;
  int	current,prev;
  double	avgvel;
  int	queuesize;
  double	recentvel;
  double	recentx;
  double	recenty;
  int		currentidx;
  
  first = 1;
  processed = 0;
  count = 0;
  processed_count = 0;
  oldpos.timestamp = 0;
  pos.timestamp = 0;
  oldx = oldy = oldtimestamp = 0;
  posx = posy = 0;
  current = 0;
  prev = 0;
  recentvel = 0;
  avgvel = 0;
  currentidx = 0;
  /*
  ** allocate the vqueue, 
  */
  queuesize = result->rate*result->velspan;
  vqueue = (double *)calloc(queuesize,sizeof(double));
  pxqueue = (double *)calloc(queuesize,sizeof(double));
  pyqueue = (double *)calloc(queuesize,sizeof(double));
  while(!feof(xchannel->fp)){
    /* 
     ** is this stereo ? 
     */
    if(result->electrode == STEREO){
      if(result->inputformat == BINARY){
	/*
	 ** binary format 
	 */
	ychannel->len =
	  xchannel->len = BinaryDualGetSpike(result,xchannel->data,
					     ychannel->data, xchannel->fp,&timestamp);
      } else 
	if(result->inputformat == ASCII){
	  /* 
	   ** get an ascii spike record on the x channel 
	   */
	  xchannel->len = AsciiGetSpike(xchannel->data,xchannel->fp);
	  /* 
	   ** get an ascii spike record on the y channel 
	   */
	  ychannel->len = AsciiGetSpike(ychannel->data,ychannel->fp);
	}
      /*
       ** keep track of the spike count
       */
      count++;
      /*
       ** no data?
       */
      if(xchannel->len==0 || ychannel->len==0) continue;
       processed_count++;
      if((count > start) && (end <=0 || count <= end)){
	if(first){
	  /*
	   ** keep track of the timestamp range for
	   ** user info only. No explicit need for it.
	   */
	  stimestamp = timestamp;
	  first = 0;
	}
	etimestamp = timestamp;
	sparms[TIME].parm = timestamp/10000.;
	sparms[TIMELO].parm = timestamp%10000;
	sparms[TIMESTAMP].parm = timestamp;

	/*
	 ** keep track of the number of spikes processed
	 ** for user info only.
	 */
	processed++;
	/* 
	 ** extract the stereo parameters 
	 */
	ExtractStereoParms(xchannel,ychannel,sparms,analyze);

	if(position_file){
	    if(result->oldpos){
		while(oldpos.timestamp < timestamp){
		    if(fread(&oldpos,sizeof(OldPosRecord),1,position_file)!=1){
		      /*
		      fprintf(stderr, "error reading position file\n");
		      */
		      oldpos.x1 = oldpos.x2 = 0;
		      break;
		    }
		}
		if (backdiode) {
		  posx = (int) oldpos.x2;
		  posy = (int) oldpos.y2;
		} else {
		  posx = (int) oldpos.x1;
		  posy = (int) oldpos.y1;
		}
		sparms[POS_X].parm = (float)posx + xoffset;
		sparms[POS_Y].parm = (float)posy;
	    } else  {
		while(pos.timestamp < timestamp){
		    if(fread(&pos,sizeof(PosRecord),1,position_file)!=1){
		      /*
		      fprintf(stderr, "error reading position file\n");
		      */
		      pos.x1 = pos.x2 = 0;
		      break;
		    }
		}
		if (backdiode) {
		  posx = pos.x2;
		  posy = pos.y2;
		} else {
		  posx = pos.x1;
		  posy = pos.y1;
		}
		  sparms[POS_X].parm = (float)posx + xoffset;
		  sparms[POS_Y].parm = (float)posy;

	    }
	}
	
	if(analyze){
	  /*
	   ** bin the parameter values
	   */
	  for(i=0;i<NPARMS;i++){
	    if(sparms[i].name){
	      bin = (int)((sparms[i].nbins - 1)*
			  (sparms[i].parm - sparms[i].min)/
			  (sparms[i].max - sparms[i].min));
	      sparms[i].hist[bin]++;
	    }
	  }
	} else {
	  /* 
	   ** write the parameters to the output file 
	   */
	  WriteSpikeParms(count-1,sparms,result->fpout,result->outputformat);
	}
      }
      if(end > 0 && count > end) break;
    } else 
      if(result->electrode == TETRODE){
	if(result->inputformat == BINARY){
	  /*
	   ** binary format 
	   */
	  achannel->len =
	    bchannel->len =
	      ychannel->len =
		xchannel->len = BinaryTetGetSpike(result,xchannel->data,
						  ychannel->data, achannel->data, bchannel->data,
						  xchannel->fp,&timestamp);
	} else 
	  if(result->inputformat == ASCII){
	    /* 
	     ** get ascii spike records on the individual channels
	     */
	    xchannel->len = AsciiGetSpike(xchannel->data,xchannel->fp);
	    ychannel->len = AsciiGetSpike(ychannel->data,ychannel->fp);
	    achannel->len = AsciiGetSpike(achannel->data,achannel->fp);
	    bchannel->len = AsciiGetSpike(bchannel->data,bchannel->fp);
	  }
	/*
	 ** keep track of the spike count
	 */
	count++;
	/*
	 ** no data?
	 */
	if(xchannel->len==0 || ychannel->len==0 ||
	   achannel->len==0 || bchannel->len==0) continue;
	   processed_count++;
	/*
	** if a range was specified then only process those within it
	*/
	if((count > start) && (end <=0 || count <= end)){
	    /*
	    ** if an index file was specified then check it
	    */
	    if(idx->n > 0){
		if(currentidx >= idx->n) break;
		if(count-1 != idx->index[currentidx]) continue; 
		/*
		** found a match so advance to the next in the list
		*/
		currentidx++;
	    }
	  if(first){
	    /*
	     ** keep track of the timestamp range for
	     ** user info only. No explicit need for it.
	     */
	    stimestamp = timestamp;
	    first = 0;
	  }
	  etimestamp = timestamp;
	  sparms[TIME].parm = timestamp/10000.;
	  sparms[TIMELO].parm = timestamp%10000;
	  sparms[TIMESTAMP].parm = timestamp;

	  /*
	   ** keep track of the number of spikes processed
	   ** for user info only.
	   */
	  processed++;
	  /* 
	   ** extract the tetrode parameters 
	   */
	  ExtractTetrodeParms(xchannel,ychannel,achannel,bchannel,
			      sparms,analyze);
	  
	    if(position_file) {
	 	while(pos.timestamp < timestamp){
		    if(1 != fread(&pos, sizeof(PosRecord), 1, position_file)){
			/*
			fprintf(stderr, "error reading position file\n");
			*/
			pos.x1 = pos.x2 = 0;
			break;
		    }
		    /*
		    ** keep a running average position over 1 sec
		    */
		    
		    if (backdiode) {
		      posx = pos.x2;
		      posy = pos.y2;
		    } else {
		      posx = pos.x1;
		      posy = pos.y1;
		    }

		    if(pos.timestamp != oldtimestamp && posx != 0 && 
		    posy != 0 && oldx != 0 && oldy != 0){
		      vqueue[current] = 
			1e4*sqrt((double)(posy -oldy)*(posy - oldy)+ 
				(double)(posx - oldx)*(posx - oldx))/
					(double)(pos.timestamp - oldtimestamp);
		      pxqueue[current] = posx;
		      pyqueue[current] = posy;
			recentvel = vqueue[current];
			recentx = pxqueue[current];
			recenty = pyqueue[current];
		    } else {
		      vqueue[current] = recentvel;
		      pxqueue[current] = 0;
		      pyqueue[current] = 0;
		    }
		    oldtimestamp = pos.timestamp;
		    oldx = posx;
		    oldy = posy;
		    avgvel += vqueue[current] - vqueue[prev];
		    prev = current;
		    current++;
		    if(current%queuesize == 0){
			current = 0;
		    }
		}
	        sparms[POS_X].parm = (float)posx + xoffset;
	        sparms[POS_Y].parm = (float)posy;

	        /*sparms[VEL].parm = avgvel/queuesize; */
		if(pxqueue[current] == 0 || pxqueue[prev] == 0){
		    sparms[VEL].parm = 0;
		} else {
		    sparms[VEL].parm = 
		    sqrt((pxqueue[current] - pxqueue[prev])*
		    (pxqueue[current] - pxqueue[prev]) +
		    (pyqueue[current] - pyqueue[prev])*
		    (pyqueue[current] - pyqueue[prev]))/queuesize;
		}
	  }
	
	  if(analyze){
	    /*
	     ** bin the parameter values
	     */
	    for(i=0;i<NPARMS;i++){
	      if(sparms[i].name){
		bin = (int)((sparms[i].nbins - 1)*
			    (sparms[i].parm - sparms[i].min)/
			    (sparms[i].max - sparms[i].min));
		sparms[i].hist[bin]++;
	      }
	    }
	  } else {
	    /* 
	     ** write the parameters to the output file 
	     */
	    WriteSpikeParms(count-1,sparms,result->fpout,result->outputformat);
	  }
	}
	if(end > 0 && count > end) break;
      } else { 	
	/* 
	 ** single electrode 
	 */
	/* 
	 ** get a spike record on the x channel 
	 */
	xchannel->len = AsciiGetSpike(xchannel->data,xchannel->fp);
	
	if(xchannel->len==0) continue;
	/* 
	 ** extract the spike parameters 
	 */
	ExtractSingleParms(xchannel,sparms);
	
	if(!analyze){
	  /* 
	   ** write the parameters to the output file 
	   */
	  WriteSingleParms(sparms,result->fpout,result->outputformat);
	}
      }
  }
}

void AnalyzeHistograms(sparms,fp)
     SpikeParm	*sparms;
     FILE		*fp;
{
  int	i;
  int	j;
  
  /*
   ** compute the histogram variance for each parameter
   */
  for(i=0;i<NPARMS;i++){
    if(sparms[i].name){
      for(j=0;j<sparms[i].nbins;j++){
	sparms[i].histsum += sparms[i].hist[j];
	sparms[i].histsumsqr += sparms[i].hist[j]*sparms[i].hist[j];
      }
      sparms[i].histvar =
	((double)sparms[i].nbins*sparms[i].histsumsqr -
	 sparms[i].histsum*sparms[i].histsum)/
	   ((double)sparms[i].nbins*(sparms[i].nbins - 1));
      if(sparms[i].histvar > 0){
	fprintf(fp,"/newplot\n/plotname %s\n",sparms[i].name);
	fprintf(fp,"%% STANDARD DEV %10s\t%g\n",
		sparms[i].name,sqrt((double)sparms[i].histvar));
	for(j=0;j<sparms[i].nbins;j++){
	  fprintf(fp,"%d\n",sparms[i].hist[j]);
	}
      }
    }
  }
}

void AddRange(Result *result,unsigned long tstart,unsigned long tend)
/* Result	*result; */
/* unsigned long tstart; */
/* unsigned long tend; */
{
TRange	*trange;

    /*
    ** fill the trange data structure
    */
    if((trange = (TRange *)calloc(1,sizeof(TRange))) == NULL){
	fprintf(stderr,"ERROR: memory allocation error\n");
	exit(-1);
    };
    trange->tstart = tstart;
    trange->tend = tend;
    if(verbose){
	fprintf(stderr,"Tstart: %s\t ",TimestampToString(tstart));
	fprintf(stderr,"Tend: %s\n",TimestampToString(tend));
    }
    /*
    ** insert it into the trange list
    */
    trange->next = result->trange;
    result->trange = trange;
}

int ReadRange(result)
Result		*result;
{
int	headersize;
FILE	*fp;
char	line[201];
unsigned long	tstart;
unsigned long	tend;
int	count;
int	binaryformat;
char	startstr[30];
char	endstr[30];
char	ef0[10];
char	ef1[10];
char	*filetype;
char	**header;

    if((fp = result->fptrange) == NULL) return(0);
    /*
    ** rewind the file
    */
    fseek(fp,0L,0L);
    /*
    ** read the trange file header
    */
    header = ReadHeader(fp,&headersize);
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
    /*
    ** read the timestamp ranges
    */
    count = 0;
    while(!feof(fp)){
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
	}
	AddRange(result,tstart,tend);
	count++;
    }
    fclose(fp);
    return(count);
}

/*
 *********************************************************
 **              MAIN
 *********************************************************
 */
int main(argc,argv)
     int	argc;
     char	**argv;
{
Index	index;
  SpikeChannel	xchannel;
  SpikeChannel	ychannel;
  SpikeChannel	achannel;
  SpikeChannel	bchannel;
  SpikeParm	*sparms;
  int		nxtarg;
  int		bw;
  FILE		*fpindex;
  int		i;
  int		start;
  int		end;
  int		count;
  int		processed;
  FILE		*fpscaleout;
  FILE		*fptemplate;
  FILE		*fpanalout;
  char		*template_fname;
  int		analyze;
  int		showparms;
  long		headersize;
  char		*fname;
  char		*etype;
  char		**header;
  Result	result;
  unsigned long	tstart;
  unsigned long	tend;
  
  nxtarg = 0;
  bw = 0;
  spkvoffset = 0;
  showparms = 0;
  index.index = (int *)malloc(MAX_VECTORS*sizeof(int));
  index.n = 0;
  fpindex = NULL;
  xchannel.fp = NULL;
  ychannel.fp = NULL;
  achannel.fp = NULL;
  bchannel.fp = NULL;
  fpscaleout = NULL;
  fpanalout = stderr;
  processed = 0;
  count = 0;
  start = 0;
  end = 0;
  verbose = 0;
  nwaveforms = 0;
  analyze = 0;
  nparms = 0;
  templatelist = NULL;
  trigger_pt = TRIGGER_PT;
  sparms = (SpikeParm *)calloc(NPARMS,sizeof(SpikeParm));
  SetParmNames(sparms);
  bzero(&result,sizeof(Result));
  result.trangefname = NULL;
  result.fptrange = NULL;
  result.electrode = STEREO;
  result.inputformat = BINARY;
  result.outputformat = ASCII;
  result.fpout = stdout;
  result.oldpos = 0;
  result.velspan = .5;
  xoffset = 0;
  position_file = NULL;
  backdiode = 0;

  while(++nxtarg < argc){
    if(strcmp(argv[nxtarg],"-usage") == 0){
      fprintf(stderr,
	      "usage: %s spikefile [-single][-tetrode][-cbc][-pset1][-pset2]\n",
	      argv[0]);
      fprintf(stderr,
	      "\t[-parms name,name,..][-start #][-end #][-asciispikes]\n");
      fprintf(stderr,
	      "\t[-o file][-so file][-tset1][-tset2][-i index][-if indexfile]\n");
      fprintf(stderr,
	      "\t[-anal][-ao file][-showparms][-v][-binary][-triggerpt n]\n");
      fprintf(stderr,
	      "\t[-template file][-dtemplate file][-dottemplate file]\n");
      fprintf(stderr,
	      "\t[-trange tstart tend][-trangefile file][-velspan sec]\n");
      fprintf(stderr,
	      "\t[-pos file][-xoffset x][-voffset v][-backdiode]\n");
      exit(-1);
    } 
    else if(strcmp(argv[nxtarg],"-version") == 0){
	fprintf(stderr,"%s : version %s : updated %s\n",
	    argv[0],VERSION,DATE);

/* passed in from makefile when building an RPM*/
#ifdef MWSOFT_RPM_STRING 
	fprintf(stderr,"From RPM: %s\n",
		MWSOFT_RPM_STRING);
#endif

	fprintf(stderr,"%s\n",COPYRIGHT);

      exit(0);
    } 
    else if(strcmp(argv[nxtarg],"-i") == 0){
      index.index[index.n++] = atoi(argv[++nxtarg]);
    } 
    else if(strcmp(argv[nxtarg],"-if") == 0){
      fname = argv[++nxtarg];
      if((fpindex = fopen(fname,"r")) == NULL){
	fprintf(stderr,"unable to open index file '%s'\n",
		fname);
	exit(0);
      }
    } 
    else if(strcmp(argv[nxtarg],"-showparms") == 0){
      ShowParmNames(sparms);
      exit(0);
    } 
    else if(strcmp(argv[nxtarg],"-parms") == 0){
      /* add the listed parms to the parm list */
      SetParmMask(sparms, argv[++nxtarg]);
    } 
    else if(strcmp(argv[nxtarg],"-start") == 0){
      start = atoi(argv[++nxtarg]);
    } 
    else if(strcmp(argv[nxtarg],"-parmlist") == 0){
      for(i=0;i<NPARMS;i++){
	if(sparms[i].name){
	  fprintf(stderr,"%10s",sparms[i].name);
	  if((i+1)%7 == 0) fprintf(stderr,"\n");
	}
      }
      exit(0);
    } 
    else if(strcmp(argv[nxtarg],"-end") == 0){
      end = atoi(argv[++nxtarg]);
    } 
    else if(strcmp(argv[nxtarg],"-asciispikes") == 0){
      result.inputformat = ASCII;
    } 
    else if(strcmp(argv[nxtarg],"-binary") == 0){
      result.outputformat = BINARY;
    } 
    else if(strcmp(argv[nxtarg],"-v") == 0){
      verbose = 1;
    } 
    else if(strcmp(argv[nxtarg],"-anal") == 0){
      analyze = 1;
    } 
    else if(strcmp(argv[nxtarg],"-pset1") == 0){
      parm_list[nparms++] = X0;		/* peak offset 0 x */
      parm_list[nparms++] = Y0;		/* peak offset 0 y */
      parm_list[nparms++] = XP2;		/* peak offset 2 x */
      parm_list[nparms++] = YP2;		/* peak offset 2 y */
      parm_list[nparms++] = XP8;		/* peak offset 8 x */
      parm_list[nparms++] = YP8;		/* peak offset 8 y */
    } 
    else if(strcmp(argv[nxtarg],"-pset2") == 0){
      parm_list[nparms++] = AVGPHS;	/* mean phase */
      parm_list[nparms++] = SDPHS;	/* phase sd */
      parm_list[nparms++] = PAMP;	/* peak amplitude */
      parm_list[nparms++] = ZCROSS;	/* zero crossing time */
      parm_list[nparms++] = INZP4M4; /* integral relative to zcross */
    } 
    else if(strcmp(argv[nxtarg],"-cbc") == 0){
      parm_list[nparms++] = PX;
      parm_list[nparms++] = PY;
      parm_list[nparms++] = PPHS;
      parm_list[nparms++] = VPHS;
      parm_list[nparms++] = MAXHT;
      parm_list[nparms++] = MAXWD;
      parm_list[nparms++] = VX;
      parm_list[nparms++] = VY;
    } 
    else if(strcmp(argv[nxtarg],"-kset") == 0){
      parm_list[nparms++] = PX;
      parm_list[nparms++] = PY;
      parm_list[nparms++] = XM4;
      parm_list[nparms++] = YM4;
      parm_list[nparms++] = XP4;
      parm_list[nparms++] = YP4;
      parm_list[nparms++] = XP8;
      parm_list[nparms++] = YP8;
      parm_list[nparms++] = VX;
      parm_list[nparms++] = VY;
      parm_list[nparms++] = MAXWD;
      parm_list[nparms++] = SDPHS;
      parm_list[nparms++] = PHSP4;
    } 
    else if(strcmp(argv[nxtarg],"-tset1") == 0){
      parm_list[nparms++] = T_PX;
      parm_list[nparms++] = T_PY;
      parm_list[nparms++] = T_PA;
      parm_list[nparms++] = T_PB;
      parm_list[nparms++] = T_MAXWD;
      parm_list[nparms++] = T_MAXHT;
      parm_list[nparms++] = TIME;
      result.electrode = TETRODE;
    } 
    else if(strcmp(argv[nxtarg],"-tset2") == 0){
      parm_list[nparms++] = T_PX;
      parm_list[nparms++] = T_PY;
      parm_list[nparms++] = T_PA;
      parm_list[nparms++] = T_PB;
      result.electrode = TETRODE;
    } 
    else if(strcmp(argv[nxtarg],"-ao") == 0){
      if((fpanalout = fopen(argv[++nxtarg],"w")) == NULL){
	fprintf(stderr,"unable to open hist analysis file '%s'\n",
		argv[nxtarg]);
	exit(-1);
      }
    } 
    else if(strcmp(argv[nxtarg],"-so") == 0){
      if((fpscaleout = fopen(argv[++nxtarg],"w")) == NULL){
	fprintf(stderr,"unable to open scale file '%s'\n",
		argv[nxtarg]);
	exit(-1);
      }
    } 
    else if(strcmp(argv[nxtarg],"-triggerpt") == 0){
      trigger_pt = atoi(argv[++nxtarg]);
    } 
    else if(strcmp(argv[nxtarg],"-velspan") == 0){
      result.velspan = atof(argv[++nxtarg]);
    } 
    else if(strcmp(argv[nxtarg],"-single") == 0){
      result.electrode = SINGLE;
    } 
    else if(strcmp(argv[nxtarg],"-tetrode") == 0){
      result.electrode = TETRODE;
    } 
    else if(strcmp(argv[nxtarg],"-trangefile") == 0){
      result.trangefname = argv[++nxtarg];
      if((result.fptrange = fopen(result.trangefname,"r")) == NULL){
	fprintf(stderr,"unable to open time range file '%s'\n",
		argv[nxtarg]);
	exit(0);
      }
    } 
    else if(strcmp(argv[nxtarg],"-trange") == 0){
      tstart = ParseTimestamp(argv[++nxtarg]);
      tend = ParseTimestamp(argv[++nxtarg]);
      AddRange(&result,tstart,tend);
    } 
    else if(strcmp(argv[nxtarg],"-dtemplate") == 0){
      template_fname = argv[++nxtarg];
      if((fptemplate = fopen(template_fname,"r")) == NULL){
	fprintf(stderr,"unable to open template file '%s'\n",
		argv[nxtarg]);
	exit(0);
      }
      templatelist = 
	AddTemplate(templatelist,fptemplate,template_fname,DIFFTEMPLATE);
      fclose(fptemplate);
    } 
    else if(strcmp(argv[nxtarg],"-template") == 0){
      template_fname = argv[++nxtarg];
      if((fptemplate = fopen(template_fname,"r")) == NULL){
	fprintf(stderr,"unable to open template file '%s'\n",
		argv[nxtarg]);
	exit(0);
      }
      templatelist = 
	AddTemplate(templatelist,fptemplate,template_fname,CORRTEMPLATE);
      fclose(fptemplate);
    }
    else if(strcmp(argv[nxtarg],"-dottemplate") == 0){
      template_fname = argv[++nxtarg];
      if((fptemplate = fopen(template_fname,"r")) == NULL){
	fprintf(stderr,"unable to open template file '%s'\n",
		argv[nxtarg]);
	exit(0);
      }
      templatelist = 
	AddTemplate(templatelist,fptemplate,template_fname,DOTTEMPLATE);
      fclose(fptemplate);
    } 
    else if(strcmp(argv[nxtarg],"-o") == 0){
      if((result.fpout = fopen(argv[++nxtarg],"w")) == NULL){
	fprintf(stderr,"unable to write to parameter file '%s'\n",
		argv[nxtarg]);
	exit(-1);
      }
    } 
    else if(strcmp(argv[nxtarg],"-pos") == 0){
      if((position_file = fopen(argv[++nxtarg],"r")) == NULL){
	fprintf(stderr,"unable to read position file '%s'\n",
		argv[nxtarg]);
	exit(-1);
      }
      header = ReadHeader(position_file,&headersize);
      /*
      ** look for the type of position information
      */
	if((etype = GetHeaderParameter(header,"Extraction type:"))){
	    if(strcmp(etype,"dual diode position") == 0){
	      result.oldpos = 1;
	      fprintf(stderr,"Extracting old dual diode format\n");
	    }
	}
	if((etype = GetHeaderParameter(header,"Rate:"))){
	    result.rate = atoi(etype);
	} else {
	    result.rate = 60;
	}
      parm_list[nparms++] = POS_X;
      parm_list[nparms++] = POS_Y;
      parm_list[nparms++] = VEL;
    }
    else if(strcmp(argv[nxtarg],"-xoffset") == 0){
      xoffset = atoi(argv[++nxtarg]);
    }
    else if(strcmp(argv[nxtarg], "-backdiode") == 0) {
      backdiode = 1;
    }
    else if(strcmp(argv[nxtarg],"-voffset") == 0){
      spkvoffset = atoi(argv[++nxtarg]);
    }
    else if(argv[nxtarg][0] != '-'){
      if((xchannel.fp = fopen(argv[nxtarg],"r")) == NULL){
	fprintf(stderr,"unable to read xy channel file '%s'\n",
		argv[nxtarg]);
	exit(-1);
      }
      xchannel.filename = argv[nxtarg];
    } 
    else 
      {
	fprintf(stderr,"Invalid option '%s'\n",argv[nxtarg]);
	exit(-1);
      }
  }
  if(xchannel.fp == NULL){
    fprintf(stderr,"must enter spike data file\n");
    exit(-1);
  }
  if(trigger_pt < 0 || trigger_pt >= MAX_SPIKE_LEN){
    fprintf(stderr,"ERROR: invalid trigger pt (%d)\n",trigger_pt);
    exit(-1);
  }
  if(result.fptrange){
    count = ReadRange(&result);
    if(verbose){
      fprintf(stderr,"loaded %d time ranges from '%s'\n",
	      count,result.trangefname);
    }
  }
  /*
   ** rewind the data files
   */
  fseek(xchannel.fp,0,0L);
  /*
   ** skip over the header
   */
  header = ReadHeader(xchannel.fp,&headersize);
  /*
   ** try to locate parameters in the header
   */
  if((etype = GetHeaderParameter(header,"Extraction type:"))){
    if(strcmp(etype,"tetrode waveforms") == 0){
      result.electrode = TETRODE;
    }
  }
  if(result.electrode == TETRODE){
    ychannel.fp = 
      achannel.fp =
	bchannel.fp =
	  xchannel.fp;
    ychannel.filename =
      achannel.filename =
	bchannel.filename =
	  xchannel.filename;
  } else 
    if(result.electrode == STEREO){
      ychannel.fp = 
	xchannel.fp;
      ychannel.filename =
	xchannel.filename;
    }
  if(index.n == 0){
    /*
     ** read in the index file if any
     */
    if(fpindex != NULL){
      ReadIndices(fpindex,&index);
      if(index.n == 0){
	fprintf(stderr,"index file empty\n");
	exit(0);
	}
      /*
       ** sort the indices
       */
      qsort(index.index,index.n,sizeof(int),intcompare);
    }
  }
  /*
   ** print out the file header
   */
  BeginStandardHeader(result.fpout,argc,argv,VERSION);
  fprintf(result.fpout,"%%\n");
  if(xchannel.fp == ychannel.fp){
    fprintf(result.fpout,"%%%%XYDATAFILE %s\n",xchannel.filename);
  } else {
    fprintf(result.fpout,"%%%%XDATAFILE %s\n",xchannel.filename);
    fprintf(result.fpout,"%%%%YDATAFILE %s\n",ychannel.filename);
  }
  fprintf(result.fpout,"%% File type: \t");
  if(result.outputformat == BINARY){
    fprintf(result.fpout,"Binary\n");
#ifdef OLD
    fprintf(result.fpout,"%% Format: vectorsize datatype0 ... datatypen data..\n");
#endif
  } else {
    fprintf(result.fpout,"Ascii\n");
  }
  fprintf(result.fpout,"%% Electrode type: \t");
  switch(result.electrode){
  case SINGLE:
    fprintf(result.fpout,"Single electrode\n");
    break;
  case STEREO:
    fprintf(result.fpout,"Stereotrode\n");
    break;
  case TETRODE:
    fprintf(result.fpout,"Tetrode\n");
    break;
  }
  /*
   ** list the parameter identifiers
   */
  if(result.outputformat == BINARY)
    fprintf(result.fpout,"%% Fields:\tid,%d,%d,%d\t",INT,sizeof(int),1);
  else /* ASCII */
    fprintf(result.fpout,"%% Fields:\tid\t");

  WriteStereoIdentifiers(&result,sparms,result.fpout);
  fprintf(result.fpout,"%%\n");
  EndStandardHeader(result.fpout);
#ifdef OLD
  if(result.outputformat == BINARY){
    /*
     ** put in the binary header info
     ** nparms datatypes
     */
    ival = nparms + 1;		/* index + nparms parameters */
    fwrite(&ival,sizeof(int),1,result.fpout);
    ival = FLOAT;		/* index data type */
    fwrite(&ival,sizeof(int),1,result.fpout);
    ival = FLOAT;		/* parameter data types */
    for(i=0;i<nparms;i++){
      fwrite(&ival,sizeof(int),1,result.fpout);
    }
  }
#endif
  /*
   ** process the spikes
   */
  ProcessSpikes(&result,sparms,&xchannel,&ychannel,&achannel,&bchannel,
		start,end,0,&index);
  
  if(fpscaleout){
    fprintf(fpscaleout,"%% RECOMMENDED SCALE VALUES:\n");
    fprintf(fpscaleout,"%% Fields:\n%% ");
    WriteStereoIdentifiers(&result,sparms,fpscaleout);
    fprintf(result.fpout,"%%\n");
    for(i=0;i<nparms;i++){
      fprintf(fpscaleout,"%g\t",
	      1.0/sqrt(((double)nwaveforms*sparms[parm_list[i]].sumsqr -
			sparms[parm_list[i]].sum*sparms[parm_list[i]].sum)/
		       ((double)nwaveforms*(nwaveforms - 1))));
    }
  }
  if(verbose){
    fprintf(stderr,"Processed %d spikes\n", nwaveforms);
  }
  if(analyze){
    /*
     ** compute statistics on the distribution of parameters
     */
    for(i=0;i<NPARMS;i++){
      /*
       ** prepare the distribution histograms
       */
      if(sparms[i].name){
	sparms[i].nbins = NPARMBINS;
	sparms[i].hist = (int *)calloc(sparms[i].nbins,sizeof(int));
      }
    }
    /*
     ** rewind the files
     */
    if(position_file){
      fseek(position_file,0,0L);
      ReadHeader(position_file,&headersize);
    }
    if(xchannel.fp){ 
      fseek(xchannel.fp,0,0L);
      /*
       ** skip over the header
       */
      ReadHeader(xchannel.fp,&headersize);
    }
#ifdef OLD
    if(result.outputformat == BINARY){
      /*
       ** put in the binary header info
       ** nparms datatypes
       */
      ival = nparms + 1;		/* index + nparms parameters */
      fwrite(&ival,sizeof(int),1,result.fpout);
      ival = FLOAT;		/* index data type */
      fwrite(&ival,sizeof(int),1,result.fpout);
      ival = FLOAT;		/* parameter data types */
      for(i=0;i<nparms;i++){
	fwrite(&ival,sizeof(int),1,result.fpout);
      }
    }
#endif
    /*
     ** recompute the parameters maintaining the results in 
     ** histograms
     */
    ProcessSpikes(&result,sparms,&xchannel,&ychannel,&achannel,&bchannel,
		  start,end,1,&index);
    /*
     ** compute histogram statistics
     */
    AnalyzeHistograms(sparms,fpanalout);
  }
  if(nwaveforms == 0){
    exit(1);
  } else {
    exit(0);
  }
}

