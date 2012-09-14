/*
*******************************************************************
PROGRAM:
    crextract

DESCRIPTION:
    extracts continuous record information from cr files 

AUTHOR:
    Written by Matthew Wilson
    Center for Learning and Memory
    Massachusetts Institute of Technology
    Cambridge MA 02139
    wilson@ai.mit.edu

DATES:
    original program 9/92
*******************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* MWL headers */
#include <iolib.h>
#include <header.h>

#define VERSION "1.17"
#define MAXCRDATA	100000

#define ASCII	0
#define BINARY	1


typedef struct result_type {
    FILE	*fpout;
    FILE	*fpin;
    char	*fname_in;
    char	*fname_out;
    unsigned long	starttime;
    unsigned long	endtime;
    int		fileformat;
    int		nox;
    int		uff_format;
    int		nchannels;
    int		bufsize;
    int		channel;
    long	freq;
    unsigned long	toffset;
} Result;

int verbose;
short	crdata[MAXCRDATA];
int	convert;

void ExtractCR(result)
Result	*result;
{
double 	timestamp;
unsigned long 	startstamp;
double 	dt = 0;
unsigned long 	freq;
long	i;
unsigned short	nsamples;
int	first;
unsigned short	sval;
unsigned long	lval;

    first = 1;
    timestamp = 0;
    if(!result->uff_format){
	nsamples = result->bufsize;
	dt = 1e4/result->freq;
    }
    while(!feof(result->fpin)){
	if(result->uff_format){
	    if(fread(&startstamp,sizeof(unsigned long),1,result->fpin) != 1) break;
	    if(convert){
		ConvertData(&startstamp,sizeof(unsigned long));
	    }
	    if(result->toffset > 0){
		startstamp += result->toffset;
	    }
	    if(fread(&nsamples,sizeof(unsigned short),1,result->fpin) != 1) break;
	    if(convert){
		ConvertData(&nsamples,sizeof(unsigned short));
	    }
	    if(fread(&freq,sizeof(unsigned long),1,result->fpin) != 1) break;
	    if(convert){
		ConvertData(&freq,sizeof(unsigned long));
	    }
	    if(fread(crdata,sizeof(short),nsamples,result->fpin) != nsamples) break;
	    if(convert){
		for(i=0;i<nsamples;i++){
		    ConvertData(&crdata[i],sizeof(short));
		}
	    }
	    dt = 1e4/freq;
	} else {
	    if(fread(&startstamp,sizeof(unsigned long),1,result->fpin) != 1) break;
	    if(convert){
		ConvertData(&startstamp,sizeof(unsigned long));
	    }
	    if(result->toffset > 0){
		startstamp += result->toffset;
	    }
	    if(fread(crdata,sizeof(short),nsamples,result->fpin) != nsamples) break;
	    if(convert){
		for(i=0;i<nsamples;i++){
		    ConvertData(&crdata[i],sizeof(short));
		}
	    }
	}
	/*
	** check the timestamp
	*/
	if((startstamp <result->starttime)){
	    continue;
	}
	if((result->endtime > 0) && (timestamp > result->endtime)){
	    break;
	}

	/*
	** to be compatible with the uff cr format we will write out
	** blocks which contain the starting timestamp, freq, and nsamples
	** followed by a chunk of crdata
	*/
	if(result->fileformat == BINARY){
	    /*
	    ** note that each channel is sampled at successive
	    ** sample periods due to the nature of the multiplexing
	    ** unless burst mode has been used.  Thus the starting
	    ** timestamp must be corrected to take this into account.
	    ** Note that this is automatically taken care of for
	    ** ascii mode output since the timestamp is calculated
	    ** from the multiplexed data directly.
	    */
	    startstamp += (result->channel%result->nchannels)*dt;

	    fwrite(&startstamp,sizeof(unsigned long),1,result->fpout);
	    sval = nsamples/result->nchannels;
	    fwrite(&sval,sizeof(unsigned short),1,result->fpout);
	    lval = result->freq/result->nchannels;
	    fwrite(&lval,sizeof(unsigned long),1,result->fpout);
	}
	/*
	** write out the data
	*/
	for(i=0;i<nsamples;i++){
	    timestamp = startstamp + i*dt;
	    if(first && verbose){
		fprintf(stderr,"Starting timestamp: %s (%g)\n",
		TimestampToString((unsigned long)timestamp),
		timestamp);
		first = 0;
	    }
	    if(result->fileformat == ASCII){
		if(result->uff_format || 
		(i%result->nchannels == result->channel)){
		    if(!result->nox){
			fprintf(result->fpout,"%30.15lg \t",timestamp);
		    }
		    fprintf(result->fpout,"%hd\n",crdata[i]);
		}
	    } else
	    if(result->fileformat == BINARY){
		if(result->uff_format || 
		(i%result->nchannels == result->channel)){
		/*
		    if(!result->nox){
			tstamp = timestamp;
			fwrite(&tstamp,sizeof(unsigned long),1,
			result->fpout);
		    }
		    */
		    fwrite(&crdata[i],sizeof(short),1,result->fpout);
		}
	    }
	    /*
	    ** break if at the end of the requested range
	    if((result->endtime > 0) && (timestamp > result->endtime)){
		break;
	    }
	    */
	}
	/*
	** break if at the end of the requested range
	*/
	if((result->endtime > 0) && (timestamp > result->endtime)){
	    break;
	}
    }
    if(verbose){
	fprintf(stderr,"Ending timestamp: %s (%lg)\n",
	    TimestampToString((unsigned long)timestamp),
	    timestamp);
    }
}

void WriteOutputHeader(result,argc,argv)
Result	*result;
int	argc;
char	**argv;
{
    if(result->fpout == NULL) return;
    BeginStandardHeader(result->fpout,argc,argv,VERSION);
    fprintf(result->fpout,"%% Input data file:\t%s\n",result->fname_in);
    fprintf(result->fpout,"%% File contents:\tCR waveform\n");
    fprintf(result->fpout,"%% File type:\t");
    if(result->fileformat == ASCII){
	fprintf(result->fpout,"Ascii\n");
    } else
    if(result->fileformat == BINARY){
	fprintf(result->fpout,"Binary\n");
    }
    fprintf(result->fpout,"%% CR channel:\t%d\n",
	result->channel);
    fprintf(result->fpout,"%% Start time:\t%s (%ld)\n",
	TimestampToString(result->starttime),result->starttime);
    fprintf(result->fpout,"%% End time:\t%s (%ld)\n",
	TimestampToString(result->endtime),result->endtime);
    fprintf(result->fpout,"%% rate:\t%ld\n",result->freq);
    if(result->fileformat == BINARY){
	fprintf(result->fpout,"%% Fields:\t");
	if(!result->nox){
	    fprintf(result->fpout,"timestamp,%d,%d,1\t",
	    DOUBLE,sizeof(unsigned long));
	}
	fprintf(result->fpout,"data,%d,%d,1\n",
	SHORT,sizeof(short));
    } else {
	fprintf(result->fpout,"%% Fields:\t");
	if(!result->nox){
	    fprintf(result->fpout,"timestamp\t");
	}
	fprintf(result->fpout,"data\n");
    }
    EndStandardHeader(result->fpout);
}

int main(argc,argv)
int	argc;
char 	**argv;
{
int	nxtarg;
Result	result;
long	headersize;
char	**header;
char	*parmstr;

    nxtarg = 0;
    verbose = 0;
    result.fpout = stdout;
    result.fname_out = "STDOUT";
    result.fileformat = ASCII;
    result.starttime = 0;
    result.endtime = 0;
    result.nox = 0;
    result.uff_format = 0;
    result.channel = -1;
    result.nchannels = -1;
    result.bufsize = -1;
    result.toffset = 0;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,
	    "usage: %s crfile [-tstart timestamp][-tend timestamp][-binary]\n",
	    argv[0]);
	    fprintf(stderr,"\t[-o file][-v][-nox][-uff][-channel #]\n");
	    fprintf(stderr,"\t[-toffset time]\tadd 'time' to all crfile timestamps\n");
	    fprintf(stderr,"\t\t\t\t'time' MUST be positive\n");
	    exit(-1);
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
	if(strcmp(argv[nxtarg],"-v") == 0){
	    verbose = 1;
	} else
	if(strcmp(argv[nxtarg],"-toffset") == 0){
	    result.toffset = ParseTimestamp(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-channel") == 0){
	    result.channel = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-binary") == 0){
	    result.fileformat = BINARY;
	} else
	if(strcmp(argv[nxtarg],"-uff") == 0){
	    result.uff_format = 1;
	} else
	if(strcmp(argv[nxtarg],"-nox") == 0){
	    result.nox = 1;
	} else
	if(strcmp(argv[nxtarg],"-tstart") == 0){
	    result.starttime = ParseTimestamp(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-tend") == 0){
	    result.endtime = ParseTimestamp(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-o") == 0){
	    result.fname_out = argv[++nxtarg];
	    if((result.fpout = fopen(result.fname_out,"w")) == NULL){
		fprintf(stderr,"unable to create file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(argv[nxtarg][0] != '-'){
	    result.fname_in = argv[nxtarg];
	    if((result.fpin = fopen(result.fname_in,"r")) == NULL){
		fprintf(stderr,"unable to read cr data file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else {
	    fprintf(stderr,"invalid option '%s'\n",argv[nxtarg]);
	    exit(-1);
	}
    }

    if(result.fpin == NULL){
	fprintf(stderr,"must specify cr data file\n");
	exit(-1);
    }
    header = ReadHeader(result.fpin,&headersize);
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

    if(!result.uff_format){
	if((parmstr = GetHeaderParameter(header,"nchannels:")) != NULL){
	    result.nchannels = atoi(parmstr);
	}
	if((parmstr = GetHeaderParameter(header,"dma_bufsize:")) != NULL){
	    result.bufsize = atoi(parmstr);
	}
	if((parmstr = GetHeaderParameter(header,"rate:")) != NULL){
	    result.freq = atol(parmstr);
	}
	if(result.channel < 0){
	    fprintf(stderr,"must specify channel to extract\n");
	    exit(-1);
	}
	if(result.nchannels < 0){
	    fprintf(stderr,"unable to determine nchannels from header\n");
	    exit(-1);
	}
	if(result.bufsize < 0){
	    fprintf(stderr,"unable to determine bufsize from header\n");
	    exit(-1);
	}
    }
    if(verbose){
	fprintf(stderr,"Input data file:\t%s\n",result.fname_in);
	fprintf(stderr,"Output file:\t%s\n",result.fname_out);
	fprintf(stderr,"Nchannels:\t%d\n",result.nchannels);
	fprintf(stderr,"Bufsize:\t%d\n",result.bufsize);
	fprintf(stderr,"Sample Rate:\t%ld\n",result.freq);
	fprintf(stderr,"\n\n");
    }
    /*
    ** write out the output file header
    */
    WriteOutputHeader(&result,argc,argv);

    /*
    ** extract the cr data
    */
    ExtractCR(&result);
    exit(0);
}
