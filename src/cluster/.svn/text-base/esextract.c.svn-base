/*
*******************************************************************
PROGRAM:
    esextract

DESCRIPTION:
    extracts timestamp and string information for particular event string
    from event string files extracted from ad files.

AUTHOR:
    Written by Matthew Wilson
    Massachusetts Institute of Technology
    Center for Learning and Memory
    Cambridge MA 02139
    wilson@ai.mit.edu

DATES:
*******************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* MWL headers */
#include <header.h>
#include <iolib.h>

#define VERSION "1.08"

#define MAXES	500
#define MAXESLIST	100000
#define MAXESLEN	80

typedef struct pair_type {
    char	*espairlist[2];
    char	esmatch1[80];
    char	esmatch2[80];
    unsigned long		timestamp[2];
    short	selectpair;
    short	verified;
} PairList;

typedef struct es_type {
    char	es[MAXESLEN];
    unsigned long		timestamp;
} EString;

typedef struct result_type {
    FILE	*fpout;
    FILE	*fpin;
    int		count;
    int		binary;
    int		nspikes;
    char	*fname_in;
    char	*fname_out;
    char	**eslist;
    PairList	*pair;
    PairList	*esrange;
    EString	*es;
    int		neslist;
    int		nes;
    int		npaires;
    int		nflags;
    int		nesrange;
    int		alles;
    int		timestamp_out;
    unsigned long	toffset;
} Result;

int	verbose;
int	eventstrlen;

void ReadES(result)
Result	*result;
{
unsigned long 	timestamp;
char	es[eventstrlen];
long	headersize;
char	**header;
int	binaryformat;
char	tstr[30];
char	line[201];
char	*filetype;
int	convert;

    /*
    ** rewind the ef file
    */
    fseek(result->fpin,0L,0L);
    header = ReadHeader(result->fpin,&headersize);
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
    ** allocate the es structure
    */
    result->es = (EString *)calloc(MAXESLIST,sizeof(EString));
    result->neslist = 0;
    while(!feof(result->fpin)){
	if(binaryformat){
	    /*
	    ** read in data from the event string file
	    */
	    if(fread(&timestamp,sizeof(unsigned long),1,result->fpin) != 1) break;
	    if(convert){
		ConvertData(&timestamp,sizeof(unsigned long));
	    }
	    if(result->toffset > 0){
		timestamp += result->toffset;
	    }
	    if(fread(es,sizeof(char),eventstrlen,result->fpin) != 
		eventstrlen) break;
	} else {
	    if(fgets(line,200,result->fpin) == NULL){
		break;
	    }
	    /*
	    ** ignore comments and commands
	    */
	    if(line[0] == '%') continue;
	    if(line[0] == '/') continue;
	    if(sscanf(line,"%s %s",es,tstr) != 2) continue;
	    timestamp = ParseTimestamp(tstr);
	}
	if(result->neslist >= MAXESLIST){
	    fprintf(stderr,
	    "ERROR: exceeded maximum allowable number of event strings (%d)\n",
	    MAXESLIST);
	    break;
	}
	/*
	** add the event string to the list
	*/
	result->es[result->neslist].timestamp = timestamp;
	strcpy(result->es[result->neslist].es,es);
	result->neslist++;
    }
}

/*
** return 1 if the index is between the specified ranges
** 0 otherwise
*/
int CheckRange(result,index)
Result	*result;
int	index;
{
int	j;
int	i;
int	k;
char	*es;

    /*
    ** start searching from the index until reaching the
    ** second pair of a range specification
    */
    /*
    ** first check to see that we are past the first flag of all
    ** ranges, otherwise there is no point in proceeding with
    ** the next step
    */
    for(i=0;i<result->nesrange;i++){
	/*
	** did we hit a range in which the first flag has not
	** been reached yet?
	*/
	if(result->esrange[i].selectpair == 0){
	    /*
	    ** still looking for the first in the pair 
	    ** cause for failure
	    */
	    return(0);
	}
	/*
	** flag the range as not yet verified
	*/
	result->esrange[i].verified = 0;
    }
    /*
    ** search the event flags for the ending pair for the range
    */
    for(j=index;j<result->neslist;j++){
	es = result->es[j].es;
	/*
	** go through each range pair
	*/
	for(i=0;i<result->nesrange;i++){
	    /*
	    ** did we come across the first range flag before
	    ** hitting the second in an unverified range pair
	    */
	    if((!result->esrange[i].verified) &&
	    (strcmp(es,result->esrange[i].espairlist[0]) == 0)){
		/*
		** that is cause for failure to meet within-range 
		** requirements
		*/
		return(0);
	    } 
	    /*
	    ** did we hit the second flag of an unverified range?
	    */
	    if((!result->esrange[i].verified) &&
	    (strcmp(es,result->esrange[i].espairlist[1]) == 0)){
		/*
		** now make sure that the ranges are nested properly
		** ie they must verify in order. lower numbered
		** ranges are most deeply nested.
		*/
		for(k=0;k<i;k++){
		    if(!result->esrange[k].verified){
			/*
			** improperly nested ranges.
			** failed to meet all range requirements
			*/
			return(0);
		    }
		}
		/*
		** this range is verified
		*/
		result->esrange[i].verified = 1;
	    } 
	}
    }
    /*
    ** check that all ranges were verified
    */
    for(i=0;i<result->nesrange;i++){
	if(!result->esrange[i].verified){
	    /*
	    ** failed to meet all range requirements
	    */
	    return(0);
	}
    }
    /*
    ** satisfied all within range requirements
    */
    return(1);
}

int ScanES(result)
Result	*result;
{
unsigned long 	timestamp;
int	nmatch;
int	i;
int	j;
char	*es;
EString	*eslist;

    nmatch = 0;
    for(j=0;j<result->neslist;j++){
	eslist = &(result->es[j]);
	timestamp = eslist->timestamp;
	es = eslist->es;
	/*
	** check for flag indicating output of all event strings in the file
	*/
	if(result->alles){
	    nmatch++;
	    if(result->binary){
		fwrite(&timestamp,sizeof(unsigned long),1,result->fpout);
	    } else {
		if(result->timestamp_out){
		    fprintf(result->fpout,"%s\t%lu\n",
		    es,timestamp);
		} else {
		    fprintf(result->fpout,"%s\t%s\n",
		    es,TimestampToString(timestamp));
		}
	    }
	} else {
	    /*
	    ** are we within all the specified event string ranges?
	    */
	    for(i=0;i<result->nesrange;i++){
		/*
		** look for first string
		*/
		if(result->esrange[i].selectpair == 0 && 
		    (strcmp(es,result->esrange[i].espairlist[0]) == 0)){
		    /*
		    ** found the first string
		    ** so set search to second
		    */
		    result->esrange[i].selectpair = 1;
		    /*
		    ** keep track of the timestamp 
		    */
		    result->esrange[i].timestamp[0] = timestamp;
		}
	    }
	    /*
	    ** search for pairs within the last (most deeply nested) range
	    */
	    /*
	    ** first look for pairs
	    */
	    for(i=0;i<result->npaires;i++){
		/*
		** look for first string
		*/
		if(strncmp(es,result->pair[i].espairlist[0],
		strlen(result->pair[i].espairlist[0])) == 0){
		    /*
		    ** found the first flag
		    ** so set search to second
		    */
		    result->pair[i].selectpair = 1;
		    /*
		    ** keep track of the timestamp 
		    */
		    result->pair[i].timestamp[0] = timestamp;
		    strcpy(result->pair[i].esmatch1,es);
		} else
		/*
		** look second flag if first flag has already been found
		*/
		if((result->pair[i].selectpair == 1) && 
		    (strncmp(es,result->pair[i].espairlist[1],
		    strlen(result->pair[i].espairlist[1])) == 0)){
		    /*
		    ** found the second flag
		    ** so set search back to first
		    */
		    result->pair[i].selectpair = 0; 
		    /*
		    ** keep track of the timestamp 
		    */
		    result->pair[i].timestamp[1] = timestamp;
		    strcpy(result->pair[i].esmatch2,es);
		    /*
		    ** scan the range list to insure that the
		    ** timestamps are within the desired range
		    */
		    if(CheckRange(result,j)){
			nmatch++;
			/*
			** output the results
			*/
			if(result->binary){
			    /*
			    ** binary timestamp output
			    */
			    fwrite(&(result->pair[i].timestamp[0]),
			    sizeof(long),1,result->fpout);
			    fwrite(&(result->pair[i].timestamp[1]),
			    sizeof(long),1,result->fpout);
			} else {
			    /*
			    ** ascii string/timestamp output
			    */
			    fprintf(result->fpout,"%s\t%s",
				result->pair[i].esmatch1,
				result->pair[i].esmatch2);
			    if(result->timestamp_out){
				fprintf(result->fpout,"\t%lu",
				    result->pair[i].timestamp[0]);
				fprintf(result->fpout,"\t%lu\n",
				    result->pair[i].timestamp[1]);
			    } else {
				fprintf(result->fpout,"\t%s",
				    TimestampToString(result->pair[i].timestamp[0]));
				fprintf(result->fpout,"\t%s\n",
				    TimestampToString(result->pair[i].timestamp[1]));
			    }
			}
		    }
		} 
	    }
	    /*
	    ** next do the single strings
	    */
	    for(i=0;i<result->nes;i++){
		/*
		** compare the current string with the requested string
		*/
		if(strncmp(es,result->eslist[i],strlen(result->eslist[i])) == 0){
		    /*
		    ** matched
		    */
		    /*
		    ** scan the range list to insure that the
		    ** timestamps are within the desired range
		    */
		    if(CheckRange(result,j)){
			nmatch++;
			if(result->binary){
			    fwrite(&timestamp,sizeof(unsigned long),1,result->fpout);
			} else {
			    if(result->timestamp_out){
				fprintf(result->fpout,"%s\t%lu\n",
				es,timestamp);
			    } else {
				fprintf(result->fpout,"%s\t%s\n",
				es,TimestampToString(timestamp));
			    }
			}
		    }
		}
	    }
	    for(i=0;i<result->nesrange;i++){
		/*
		** look second string
		*/
		if((result->esrange[i].selectpair == 1) && 
		    strcmp(es,result->esrange[i].espairlist[1])){
		    /*
		    ** found the second string
		    ** so set search back to first
		    */
		    result->esrange[i].selectpair = 0; 
		    /*
		    ** keep track of the timestamp 
		    */
		    result->esrange[i].timestamp[1] = timestamp;
		} 
	    }
	}
    }
    return(nmatch);
}

void WriteOutputHeader(result,fp,fname,formatstr,argc,argv)
Result	*result;
FILE	*fp;
char	*fname;
char	*formatstr;
int	argc;
char	**argv;
{
int 	i;

    if(fp == NULL) return;
    BeginStandardHeader(fp,argc,argv,VERSION);
    fprintf(fp,"%% File type:\t%s\n",formatstr);
    if(result->binary){
	fprintf(fp,"%% Timestamp output file:\t");
	if(fname){
	    fprintf(fp,"%s\n",fname);
	} else {
	    fprintf(fp,"%s\n","none");
	}
    } else {
	fprintf(fp,"%% ES output file:\t%s\n",fname);
    }
    fprintf(fp,"%% Event flags requested: ");
    if(result->alles){
	fprintf(fp,"\tALL\n");
    } else {
	if(result->nes > 0 || result->npaires > 0){
	    fprintf(fp,"\n");
	}
	if(result->nes > 0){
	    fprintf(fp,"%%\tSINGLE:");
	    for(i=0;i<result->nes;i++){
		fprintf(fp,"\t%s",result->eslist[i]);
	    }
	    fprintf(fp,"\n");
	}
	if(result->npaires > 0){
	    fprintf(fp,"%%\tPAIRS:");
	    for(i=0;i<result->npaires;i++){
		fprintf(fp,"\t'%s' '%s'",result->pair[i].espairlist[0],
		result->pair[i].espairlist[1]);
	    }
	    fprintf(fp,"\n");
	}
    }
    if(result->nesrange > 0){
	fprintf(fp,"%% Event string ranges: ");
	fprintf(fp,"\n");
	for(i=0;i<result->nesrange;i++){
	    fprintf(fp,"\t%s %s",result->esrange[i].espairlist[0],
	    result->esrange[i].espairlist[1]);
	}
	fprintf(fp,"\n");
    }
    EndStandardHeader(fp);
}

int main(argc,argv)
int	argc;
char 	**argv;
{
int	nxtarg;
Result	result;
int	count;
int	nmatches;

    nxtarg = 0;
    verbose = 0;
    result.fpout = stdout;
    result.fname_out = "STDOUT";
    result.eslist = (char **)calloc(MAXES,sizeof(char *));
    result.pair = (PairList *)calloc(MAXES,sizeof(PairList));
    result.esrange = (PairList *)calloc(MAXES,sizeof(PairList));
    result.nes = 0;
    result.npaires = 0;
    result.nesrange = 0;
    count = 0;
    result.binary = 0;
    result.alles = 0;
    eventstrlen = MAXESLEN;
    result.timestamp_out = 0;
    result.toffset = 0;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,"usage: %s esfile [options]\n", argv[0]);
	    fprintf(stderr,"Available options:\n");
	    fprintf(stderr,"\t-es str...\n");
	    fprintf(stderr,"\t[-espair str1 str2]...\n");
	    fprintf(stderr,"\t[-esrange str1 str2]...\n");
	    fprintf(stderr,"\t-o file      \toutput file\n");
	    fprintf(stderr,"\t-binary      \tbinary output file\n");
	    fprintf(stderr,"\t-v           \tverbose\n");
	    fprintf(stderr,"\t-all         \tall event strings\n");
	    fprintf(stderr,"\t-timestamp   \toutput times as timestamps\n");
	    fprintf(stderr,"\t-toffset time\tadd 'time' to all esfile timestamps\n");
	    fprintf(stderr,"\t               \t'time' MUST be positive\n");
	    fprintf(stderr,"\t-eslen40     \tolder 40 char eventstring format\n");
	    fprintf(stderr,"\t-eslen80     \tnewer 80 char eventstring format\n");
	    fprintf(stderr,"\t               \t(default eslen is 80)\n");
	    fprintf(stderr,"\t** note that esranges must be specified with\n");
	    fprintf(stderr,"\tmost deeply nested ranges listed earliest.\n");
	    exit(-1);
	} else
	if(strcmp(argv[nxtarg],"-espair") == 0){
	    if(result.npaires >= MAXES){
		fprintf(stderr,"exceeded maximum allowable event string pairs (%d)\n",
		MAXES);
		exit(-1);
	    }
	    result.pair[result.npaires].espairlist[0] = argv[++nxtarg];
	    result.pair[result.npaires].espairlist[1] = argv[++nxtarg];
	    result.npaires++;
	} else
	if(strcmp(argv[nxtarg],"-esrange") == 0){
	    if(result.nesrange >= MAXES){
		fprintf(stderr,"exceeded maximum allowable event string pairs (%d)\n",
		MAXES);
		exit(-1);
	    }
	    result.esrange[result.nesrange].espairlist[0] = argv[++nxtarg];
	    result.esrange[result.nesrange].espairlist[1] = argv[++nxtarg];
	    result.nesrange++;
	} else
	if(strcmp(argv[nxtarg],"-es") == 0){
	    if(result.nes >= MAXES){
		fprintf(stderr,"exceeded maximum allowable event strings (%d)\n",
		MAXES);
		exit(-1);
	    }
	    result.eslist[result.nes] = argv[++nxtarg];
	    result.nes++;
	} else
	if(strcmp(argv[nxtarg],"-toffset") == 0){
	    result.toffset = ParseTimestamp(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-timestamp") == 0){
	    result.timestamp_out= 1;
	} else
	if(strcmp(argv[nxtarg],"-all") == 0){
	    result.alles= 1;
	} else
	if(strcmp(argv[nxtarg],"-eslen40") == 0){
	    eventstrlen = 40;
	} else
	if(strcmp(argv[nxtarg],"-eslen80") == 0){
	    eventstrlen = 80;
	} else
	if(strcmp(argv[nxtarg],"-binary") == 0){
	    result.binary= 1;
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
		fprintf(stderr,"unable to read event string file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else {
	    fprintf(stderr,"invalid option '%s'\n",argv[nxtarg]);
	    exit(-1);
	}
    }

    if(result.nes <= 0 && result.npaires <= 0 && !result.alles) {
	fprintf(stderr,"must specify an event string\n");
	exit(-1);
    }
    if(result.fpin == NULL){
	fprintf(stderr,"must specify event string file\n");
	exit(-1);
    }
    /*
    ** write out the output file header
    */
    if(result.binary){
	WriteOutputHeader(&result,result.fpout,result.fname_out,"Binary",
	argc,argv);
    } else {
	WriteOutputHeader(&result,result.fpout,result.fname_out,"Ascii",
	argc,argv);
    }


    /*
    ** read in the event strings
    */
    ReadES(&result);
    /*
    ** evaluate the event strings
    */
    nmatches = ScanES(&result);
    if(verbose){
	fprintf(stderr,"Number of event strings processed:\t%d\n",result.neslist);
	fprintf(stderr,"Number of event/pair matches:\t%d\n",nmatches);
    }
    exit(0);
}
