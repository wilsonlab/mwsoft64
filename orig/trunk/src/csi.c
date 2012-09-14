/*
**************************************************
PROGRAM:

DESCRIPTION:

AUTHOR:
    Written by Matthew Wilson
    Massachusetts Institute of Technology
    Center for Learning and Memory
    Department of Brain and Cognitive Sciences and Biology
    Cambridge MA 02134
    wilson@ai.mit.edu

DATES:
    original xx/xx

**************************************************
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <header.h>
#include <iolib.h>

#define VERSION "1.04"
#define MAXPOINTS	100000
#define MAXCOL	10000
#define MAXLINE 10000
#define CSI 0
#define SINGLE 1

typedef struct point_type {
    double	t;
    float	h;
} Point;

int verbose;
Point	point[MAXPOINTS];
double	v[MAXCOL];

int main(argc,argv)
int	argc;
char 	**argv;
{
int	nxtarg;
FILE	*fpout;
FILE	*fpin;
int	tcol,hcol;
int	nactualcol;
char	line[MAXLINE];
float	ht;
float	csi;
double	interval1;
double	interval2;
double	minint;
int	ncsi;
int	npts;
int	i;
double	max_interval;
double	min_interval;
int	no_header;
int	eval_mode;

    eval_mode = 0;
    no_header = 0;
    verbose = 0;
    nxtarg = 0;
    fpin = stdin;
    fpout = stdout;
    tcol = 0;
    hcol = 1;
    min_interval = 0.003;
    max_interval = 0.015;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,"usage: %s [file][options]\n",argv[0]);
	    fprintf(stderr,"\tOptions:\n");
	    fprintf(stderr,"\t[-tcol col]	time column\n");
	    fprintf(stderr,"\t[-hcol col]	height column\n");
	    fprintf(stderr,"\t[-maxinterval sec]	maximum interval window\n");
	    fprintf(stderr,"\t[-mininterval sec] 	minimum interval window\n");
	    fprintf(stderr,"\t[-nh]		dont output a header\n");
	    fprintf(stderr,"\t[-singleeval]	simple single spike evaluation\n");
	    fprintf(stderr,"\t[-v]		verbose on\n");
	    fprintf(stderr,"\t[-version]	show version\n");
	    fprintf(stderr,"\t[-o file]		output to file\n");
	    fprintf(stderr,"\t*note that column numbers are 0 based\n");
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
	if(strcmp(argv[nxtarg],"-nh") == 0){
	    no_header = 1;
	} else
	if(strcmp(argv[nxtarg],"-tcol") == 0){
	    tcol = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-mininterval") == 0){
	    min_interval = strtod(argv[++nxtarg], NULL);
	} else
	if(strcmp(argv[nxtarg],"-maxinterval") == 0){
	    max_interval = strtod(argv[++nxtarg], NULL);
	} else
	if(strcmp(argv[nxtarg],"-singleeval") == 0){
	    eval_mode = 1;
	} else
	if(strcmp(argv[nxtarg],"-hcol") == 0){
	    hcol = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-v") == 0){
	    verbose = 1;
	} else
	if(strcmp(argv[nxtarg],"-o") == 0){
	    if((fpout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(argv[nxtarg][0] != '-'){
	    if((fpin = fopen(argv[nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else {
	    fprintf(stderr,"invalid option '%s'\n",argv[nxtarg]);
	    exit(-1);
	}
    }
    /*
    ** write the output file header
    */
    if(!no_header){
	BeginStandardHeader(fpout,argc,argv,VERSION);
	if(eval_mode == CSI){
	    fprintf(fpout,"%% Evaluation mode:\t Complex Spike\n");
	} else {
	    fprintf(fpout,"%% Evaluation mode:\t Simple\n");
	}
	fprintf(fpout,"%% Min interval:\t %g\n",min_interval);
	fprintf(fpout,"%% Max interval:\t %g\n",max_interval);
	EndStandardHeader(fpout);
    }

    npts = 0;
    while(!feof(fpin)){
	if(fgets(line,MAXLINE,fpin)== NULL) break;
	/*
	** ignore comment lines
	*/
	if(line[0] == '%') continue;
	/*
	** and xplot commands
	*/
	if(line[0] == '/') continue;
	/*
	** parse the line into the argument array
	*/
	if((nactualcol = sgetargs(line,MAXCOL,v)) > 0){
	    /*
	    ** get the time column
	    */
	    point[npts].t = v[tcol];
	    /*
	    ** get the ht column
	    */
	    point[npts].h = v[hcol];
	    npts++;
	}
    }
    /*
    ** now compute the csi by examining the min interval for each
    ** point
    */
    csi = 0;
    ncsi = 0;
    for(i=1;i<npts-1;i++){
	/*
	** look at adjacent times
	*/
	interval1 = point[i].t - point[i-1].t;
	interval2 = point[i].t - point[i+1].t;
	/*
	** take the min interval
	*/
	if(interval1 < -interval2){
	    ht = point[i].h - point[i-1].h;
	    minint = interval1;
	} else {
	    ht = point[i].h - point[i+1].h;
	    minint = interval2;
	}
	/*
	** compute the csi
	*/
	if(fabs(minint) <= max_interval){
	    switch(eval_mode){
	    case CSI:
		if(fabs(minint) < min_interval){
		    csi--;
		} else
		if(ht <= 0 && minint > 0){
		    csi++;
		} else
		if(ht > 0 && minint < 0){
		    csi++;
		} else
		if(ht <= 0 && minint < 0){
		    csi--;
		} else
		if(ht > 0 && minint > 0){
		    csi--;
		}
		break;
	    case SINGLE:
		/*
		** only look for spikes which fall within the minimum
		** refractory interval
		*/
		if(fabs(minint) < min_interval){
		    csi--;
		} 
		break;
	    }
	    /*
	    ** keep track of the number of points going into the csi
	    */
	    ncsi++;
	}
    }
    if(ncsi > 0){
	fprintf(fpout,"%.16g\n",100*csi/npts);
    } else {
	fprintf(fpout,"0\n");
    }
    exit(0);
}
