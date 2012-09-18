/*
*******************************************************************
PROGRAM:
    textract

DESCRIPTION:
    Textract generates a list of spike timestamps, in either
    ascii or binary format, from an index file

AUTHOR:
    Written by Matthew Wilson
    Departments of Brain and Cognitive Sciences, and Biology
    Center for Learning and Memory
    Massachussetts Institute of Technology
    Cambridge MA 02139
    wilson@ai.mit.edu

DATES:
    program update 6/96
     9/2012 - Updated code to run on i686 and x86_64 Stuart Layton <slayton@mit.edu>

*******************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

/* MWL headers */
#include <iolib.h>
#include <header.h>


#define VERSION "1.02"

/*
******************************************
**              DEFINES
******************************************
*/
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define TIMESTAMP_SIZE 4

/*
******************************************
**              STRUCTURES
******************************************
*/

typedef struct index_type {
    int32_t		*index;
    int32_t		n;
} Index;

typedef struct timeindex_type {
    int32_t			index;
    uint32_t	timestamp;
} TimeIndex;

/*
******************************************
**              GLOBALS
******************************************
*/
int32_t	start;
int32_t	allspikes;
int32_t	verbose;
int32_t	stheadersize;
uint32_t	tstart;
uint32_t	tend;
float	samplerate;
int32_t	convert;

/*
******************************************
**              BASIC FUNCTIONS
******************************************
*/
int32_t tindexcompare(p1,p2)
TimeIndex	*p1,*p2;
{
    return(p1->timestamp - p2->timestamp);
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
int32_t	count;
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
int32_t	count;
char	line[1000];
double	fval;
int32_t	nargs;;

    count = 0;
    fseek(fp,0L,0L);
    while(!feof(fp)){
	if(fgets(line,1000,fp) == NULL) break;
	if(line[0] == '%') continue;
	nargs = sgetargs(line,1,&fval);
	idx->index[count++] = (int32_t)(fval+0.5);
    }
    idx->n = count;
}

void DetectBursts(fp,idx,fpout,binary,max_burst_interval,showtimes)
FILE	*fp;
Index	*idx;
FILE	*fpout;
int32_t	max_burst_interval;
int32_t	binary;
int32_t	showtimes;
{
int32_t	i;
int32_t	loc;
uint32_t timestamp;
uint32_t ptimestamp;
uint32_t stimestamp;
int32_t	nburst_events;
int16_t	in_burst;
int16_t	end_burst;
int32_t	duration;
int32_t	sid;
int32_t	ploc;

    nburst_events = 0;
    in_burst = 0;
    end_burst = 0;
    ptimestamp = 0;
    ploc = 0;
    for(i=0;i<idx->n;i++){
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	/*
	** seek to the spike location
	*/
	fseek(fp,(start + loc)*TIMESTAMP_SIZE + stheadersize,0L);
	/*
	** read the timestamp
	*/
	if(fread(&timestamp,sizeof(uint32_t),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(uint32_t));
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	end_burst = 0;
	/*
	** now check for maximum interspike interval to satisfy
	** burst event requirements
	*/
	if((ptimestamp > 0) && (timestamp - ptimestamp <= max_burst_interval)){
	    /*
	    ** is this the beginning of a burst event?
	    */
	    if(!in_burst){
		stimestamp = ptimestamp;
		nburst_events = 1;
		sid = ploc;
	    }
	    in_burst = 1;
	    nburst_events++;
	} else {
	    /*
	    ** is this the end of a burst event?
	    */
	    if(in_burst){
		end_burst = 1;
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
		    if(fwrite(&sid,sizeof(int32_t),1,fpout) != 1){
			fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		    }
		}
		if(fwrite(&stimestamp,sizeof(uint32_t),1,fpout) != 1){
		    fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		}
		if(!showtimes){
		    if(fwrite(&nburst_events,sizeof(int32_t),1,fpout) != 1){
			fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		    }
		    if(fwrite(&duration,sizeof(int32_t),1,fpout) != 1){
			fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		    }
		}
	    } else {
		fprintf(fpout,"%d\t%"PRIu32"\t%d\t%d\n",sid,stimestamp,nburst_events,duration);
	    }
	    nburst_events = 0;
	}
	ptimestamp = timestamp;
	ploc = loc;
    }
}

void DetectSingleSpikes(fp,idx,fpout,binary,max_burst_interval,showtimes)
FILE	*fp;
Index	*idx;
FILE	*fpout;
int32_t	max_burst_interval;
int32_t	binary;
int32_t	showtimes;
{
int32_t	i;
int32_t	loc;
uint32_t timestamp;
uint32_t ptimestamp;
int16_t	in_burst;
int32_t	ploc;
int32_t	single_spike;

    in_burst = 0;
    ptimestamp = 0;
    ploc = 0;
    single_spike = 0;
    for(i=0;i<idx->n;i++){
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	/*
	** seek to the spike location
	*/
	fseek(fp,(start + loc)*TIMESTAMP_SIZE + stheadersize,0L);
	/*
	** read the timestamp
	*/
	if(fread(&timestamp,sizeof(uint32_t),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(uint32_t));
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	/*
	** now check for maximum interspike interval to satisfy
	** burst event requirements
	*/
	if((ptimestamp > 0) && (timestamp - ptimestamp > max_burst_interval)){
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
		    if(fwrite(&ploc,sizeof(int32_t),1,fpout) != 1){
			fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		    }
		}
		if(fwrite(&ptimestamp,sizeof(uint32_t),1,fpout) != 1){
		    fprintf(stderr,"ERROR: unable to write record %d\n",loc);
		}
	    } else {
		fprintf(fpout,"%d\t%"PRIu32"\n",ploc,ptimestamp);
	    }
	    single_spike = 0;
	}
	ptimestamp = timestamp;
	ploc = loc;
    }
}

/*
******************************************
**              OUTPUT ROUTINES
******************************************
*/
void ShowSpikeTimes(fp,idx,fpout,binary)
FILE	*fp;
Index	*idx;
FILE	*fpout;
int32_t	binary;
{
int32_t	i;
uint32_t	timestamp;
int32_t	loc;

    for(i=0;i<idx->n;i++){
	if(allspikes){
	    loc = i;
	} else {
	    loc = idx->index[i];
	}
	fseek(fp,(start + loc)*TIMESTAMP_SIZE + stheadersize,0L);
	if(fread(&timestamp,sizeof(uint32_t),1,fp) != 1){
	    fprintf(stderr,"ERROR: illegal read from record %d\n",loc);
	    return;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(uint32_t));
	}
	if(timestamp < tstart) continue;
	if((tend > 0) && (timestamp > tend)) continue;
	if(binary){
	    if(fwrite(&timestamp,sizeof(uint32_t),1,fpout) != 1){
		fprintf(stderr,"ERROR: unable to write record %d\n",loc);
	    }
	} else {
	    fprintf(fpout,"%"PRIu32"\t1\n",timestamp);
	}
    }
}

/*
******************************************
**              MAIN
******************************************
*/
int32_t main(argc,argv)
int32_t argc;
char **argv;
{
int32_t	i;
int32_t	nxtarg;
FILE	*fpindex;
FILE	*fpst;
FILE	*fpout;
int32_t	binary;
int32_t	showspiketimes;
int32_t	names;
struct stat stbuf;
char	**header;
char	*outname;
char	*stname = NULL;
char	*indexname = NULL;
Index	index;
int16_t	showbursts;
int16_t	showsinglespikes;
int32_t	max_burst_interval;
int32_t	noheader;
char	*filetype;


    noheader = 0;
    showbursts = 0;
    showsinglespikes = 0;
    max_burst_interval = 0;
    nxtarg = 0;
    verbose = 0;
    index.n = 0;
    fpout = stdout;
    outname = "STDOUT";
    fpst = NULL;
    start = 0;
    binary = 0;
    showspiketimes = 0;
    fpindex = NULL;
    names = FALSE;
    allspikes = 0;
    tstart = 0;
    tend = 0;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,
	    "usage: %s spikefile [-iall][-i index][-if indexfile][-v]\n",
	    argv[0]);
	    fprintf(stderr, "\t[-o file]\n");
	    fprintf(stderr, "\t[-showtimes]\n");
	    fprintf(stderr, "\t[-binary]\n");
	    fprintf(stderr, "\t[-tstart timestamp][-tend timestamp]\n");
	    fprintf(stderr,"\t[-bursts max_isi(msec)]\n");
	    fprintf(stderr,"\t[-singlespikes min_isi(msec)]\n");
	    fprintf(stderr,"\t[-nh]\t no header in the output\n");
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
	if(strcmp(argv[nxtarg],"-bursts") == 0){
	    max_burst_interval = atof(argv[++nxtarg])*10;
	    showbursts = 1;
	} else
	if(strcmp(argv[nxtarg],"-singlespikes") == 0){
	    max_burst_interval = atof(argv[++nxtarg])*10;
	    showsinglespikes = 1;
	} else
	if(strcmp(argv[nxtarg],"-nh") == 0){
	    noheader = 1;
	} else
	if(strcmp(argv[nxtarg],"-iall") == 0){
	    allspikes = 1;
	} else
	if(strcmp(argv[nxtarg],"-showtimes") == 0){
	    showspiketimes = 1;
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
	if(strcmp(argv[nxtarg],"-binary") == 0){
	    binary = 1;
	} else
	if(strcmp(argv[nxtarg],"-start") == 0){
	    start = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-v") == 0){
	    verbose = 1;
	} else
	if(strcmp(argv[nxtarg],"-i") == 0){
	    if(index.n == 0){
		if((index.index = (int32_t *)malloc(sizeof(int32_t))) == NULL){
		    fprintf(stderr,"MEMORY ERROR: unable to allocate index array\n");
		    exit(-1);
		}
	    } else 
	    if((index.index = (int32_t *)
		realloc(index.index,(index.n +1)*sizeof(int32_t))) == NULL){
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
    if((filetype = GetHeaderParameter(header,"Extraction type:")) != NULL){
	if(strncmp(filetype,"tetrode waveforms",strlen("tetrode waveforms")) ==
	0){
	}
    }
    if(!noheader){
	/*
	** write the output file header
	*/
	BeginStandardHeader(fpout,argc,argv,VERSION);
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
	if(showbursts){
	    fprintf(fpout,"%% Burst detection interval: \t%d\n",max_burst_interval);
	    if(binary){
		if(showspiketimes){
		    fprintf(fpout,"%% Fields: \t%s,%d,%d,%d\n",
		    "timestamp",ULONG,sizeof(uint32_t),1);
		} else
		fprintf(fpout,"%% Fields: \t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\t%s,%d,%d,%d\n",
		"id",INT,sizeof(int32_t),1,
		"timestamp",ULONG,sizeof(uint32_t),1,
		"nevents",INT,sizeof(int32_t),1,
		"duration",INT,sizeof(int32_t),1);
	    } else {
		fprintf(fpout,"%% Fields: \t%s\t%s\t%s\t%s\n",
		"id","timestamp","nevents","duration");
	    }
	}
	if(showsinglespikes){
	    fprintf(fpout,"%% Single spike detection interval: \t%d\n",max_burst_interval);
	    if(binary){
		if(showspiketimes){
		    fprintf(fpout,"%% Fields: \t%s,%d,%d,%d\n",
		    "timestamp",ULONG,sizeof(uint32_t),1);
		} else
		fprintf(fpout,"%% Fields: \t%s,%d,%d,%d\t%s,%d,%d,%d\n",
		"id",INT,sizeof(int32_t),1,
		"timestamp",ULONG,sizeof(uint32_t),1);
	    } else {
		fprintf(fpout,"%% Fields: \t%s\t%s\n",
		"id","timestamp");
	    }
	}
	EndStandardHeader(fpout);
	/*
	** end output file header
	*/
    }

    /*
    ** check to see if any indices were manually specified
    ** If not then try to read them from the index file.
    ** This should be changed to allow combining multiple file and
    ** manual specifications
    */
    if(index.n == 0){
	if(fpindex != NULL){
	    ScanIndices(fpindex,&index);
	    if((index.index = (int32_t *)malloc(index.n*sizeof(int32_t))) == NULL){
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
	    index.n = (stbuf.st_size - stheadersize)/ TIMESTAMP_SIZE;
	}
    }
  
    if(verbose){
	fprintf(stderr,"processing %d spikes\n",index.n);
    }
    /*
    ** begin processing spikes
    */
    if(showbursts){
	DetectBursts(fpst,&index,fpout,binary,max_burst_interval,
	showspiketimes);
    } else
    if(showsinglespikes){
	DetectSingleSpikes(fpst,&index,fpout,binary,
	max_burst_interval,showspiketimes);
    } else
    if(showspiketimes){
	ShowSpikeTimes(fpst,&index,fpout,binary);
    }
    exit(0);
}
