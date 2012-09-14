/*
**************************************************
PROGRAM: y

DESCRIPTION: extract columns of data from standard data files

AUTHOR:
    Written by Matthew Wilson
    Massachusetts Institute of Techonology
    Cambridge MA 02139
    wilson@ladyday.mit.edu

DATES:
    original xx/xx

**************************************************
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <header.h>
#include <iolib.h>

#define VERSION "1.23"
#define MAXCOL	10000
#define MAXLINE 10000
#define MAXRANGES	1000

typedef struct colrange_type {
    int		col;
    float	min;
    float	max;
    char	*name;
    int		include;
} ColRange;


int verbose;


int sgetstrargs(line,nmax,vector)
char	*line;
int	nmax;
char	**vector;
{
int	n;
char	*ptr;
char	*eptr;
int	done;

    if(line == NULL) return(0);
    ptr = line;
    done = 0;
    n = 0;
    while(!done){
	/*
	** skip any white space
	*/
	while((*ptr==' ') || (*ptr=='\t') || (*ptr=='\n')) 
	    ptr++;
	if(*ptr == '\0'){
	    /*
	    ** end of the line
	    */
	    break;
	}
	/*
	** find the end of the parameter string
	*/
	for(eptr = ptr;
	    (*eptr != '\0') && (*eptr != ' ') 
	    && (*eptr != '\t') && (*eptr != '\n'); 
	    eptr++);
	if(*eptr == '\0'){
	    /*
	    ** end of the line
	    */
	    done = 1;
	}
	/*
	** terminate it
	*/
	*eptr = '\0';
	/*
	** read the value
	*/
	vector[n] = ptr;
	n++;
	if(n >= nmax) break;
	/*
	** advance to the next parameter
	*/
	ptr = eptr+1;
    }
    return(n);
}

int main(argc,argv)
int argc;
char **argv;
{
int	col;
FILE	*fp;
FILE	*fpout;
static char	*v[MAXCOL];
static int	colnum[MAXCOL];
static char	*colname[MAXCOL];
char	line[MAXLINE];
int	nxtarg;
int	ncol;
int	nnamecol;
int	nactualcol;
int	i,j;
int	headersize;
char	**header;
FieldInfo	fieldinfo;
char	*fieldstr;
int	addheader;
int	startcol;
int	endcol;
ColRange	colrange[MAXRANGES];
int	nranges;
double	fval;
int	includecomments;

    fpout = stdout;
    fp = stdin;
    ncol=0;
    nranges=0;
    nnamecol=0;
    nxtarg = 0;
    verbose = 0;
    addheader = 1;
    includecomments = 0;
    while(++nxtarg < argc){
	if(strcmp("-usage",argv[nxtarg]) == 0){
	    fprintf(stderr,
	    "usage: %s [file][-o file][-ncol name][-col n [-col n] ..]\n",argv[0]);
	    fprintf(stderr,"\t[-nh]\tno output file header\n");
	    fprintf(stderr,"\t[-cols start end]\toutput columns between start and end\n");
	    fprintf(stderr,
	    "\t[-colrange col# min max]\t\tonly process data points within the\n");
	    fprintf(stderr,
	    "\t[-ncolrange colname min max]\t\tonly process data points within the\n");
	    fprintf(stderr,"\t\t\t\t\tspecified range for the given column #\n");
	    fprintf(stderr,
	    "\t[-xcolrange col# min max]\t\texclude data points within the\n");
	    fprintf(stderr,
	    "\t[-nxcolrange colname min max]\t\texclude data points within the\n");
	    fprintf(stderr,"\t\t\t\t\tspecified range for the given column #\n");
	    fprintf(stderr,
	    "\t[-includecomments]\t\tprocess comments as well\n");
	    fprintf(stderr,"\t* note col index is one-based\n");
	    exit(1);
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
	if(strcmp(argv[nxtarg],"-includecomments") == 0){
	    includecomments = 1;
	} else
	if(strcmp(argv[nxtarg],"-nh") == 0){
	    addheader = 0;
	} else
	if(strcmp("-o",argv[nxtarg]) == 0){
	    if((fpout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to write file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else
	if (strcmp(argv[nxtarg],"-colrange") == 0){
	    colrange[nranges].col = atoi(argv[++nxtarg]);
	    colrange[nranges].min = atof(argv[++nxtarg]);
	    colrange[nranges].max = atof(argv[++nxtarg]);
	    colrange[nranges].name = NULL;
	    colrange[nranges].include = 1;
	    nranges++;
	} else 
	if (strcmp(argv[nxtarg],"-ncolrange") == 0){
	    colrange[nranges].col = -1;
	    colrange[nranges].name = argv[++nxtarg];
	    colrange[nranges].min = atof(argv[++nxtarg]);
	    colrange[nranges].max = atof(argv[++nxtarg]);
	    colrange[nranges].include = 1;
	    nranges++;
	    nnamecol++;
	} else 
	if (strcmp(argv[nxtarg],"-xcolrange") == 0){
	    colrange[nranges].col = atoi(argv[++nxtarg]);
	    colrange[nranges].min = atof(argv[++nxtarg]);
	    colrange[nranges].max = atof(argv[++nxtarg]);
	    colrange[nranges].name = NULL;
	    colrange[nranges].include = 0;
	    nranges++;
	} else 
	if (strcmp(argv[nxtarg],"-xncolrange") == 0){
	    colrange[nranges].col = -1;
	    colrange[nranges].name = argv[++nxtarg];
	    colrange[nranges].min = atof(argv[++nxtarg]);
	    colrange[nranges].max = atof(argv[++nxtarg]);
	    colrange[nranges].include = 0;
	    nranges++;
	    nnamecol++;
	} else 
	if(strcmp("-cols",argv[nxtarg]) == 0){
	    startcol = atoi(argv[++nxtarg]);
	    endcol = atoi(argv[++nxtarg]);
	    if(ncol >= MAXCOL){
		fprintf(stderr,
		"exceeded maximum number of allowable columns (%d)\n",MAXCOL);
		exit(-1);
	    }
	    /*
	    ** activate the columns between the specified range
	    */
	    for(i=startcol;i<=endcol;i++){
		if(ncol >= MAXCOL){
		    fprintf(stderr,
		    "exceeded maximum number of allowable columns (%d)\n",MAXCOL);
		    exit(-1);
		}
		colnum[ncol] = i;
		colname[ncol] = NULL;
		ncol++;
	    }
	} else
	if(strcmp("-col",argv[nxtarg]) == 0){
	    if(ncol >= MAXCOL){
		fprintf(stderr,
		"exceeded maximum number of allowable columns (%d)\n",MAXCOL);
		exit(-1);
	    }
	    /*
	    ** assign the column number 
	    */
	    colnum[ncol] = atoi(argv[++nxtarg]);
	    colname[ncol] = NULL;
	    ncol++;
	} else
	if(strcmp("-ncol",argv[nxtarg]) == 0){
	    if(ncol >= MAXCOL){
		fprintf(stderr,
		"exceeded maximum number of allowable columns (%d)\n",MAXCOL);
		exit(-1);
	    }
	    colnum[ncol] = -1;
	    colname[ncol] = argv[++nxtarg];
	    ncol++;
	    nnamecol++;
	} else
	if(argv[nxtarg][0] != '-'){
	    if((fp = fopen(argv[nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to open file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else {
	    fprintf(stderr,"invalid option '%s'\n",argv[nxtarg]);
	    exit(-1);
	}
    }
    /*
    ** test for the default column specification
    */
    if(ncol == 0){
	colnum[ncol] = 2;		/* use column 2 by default */
	colname[ncol] = NULL;
	ncol++;
    }
    if(nnamecol > 0){
	/*
	** look for any named columns that need to be looked up
	*/
	if((header = ReadHeader(fp,&headersize)) == NULL){
	    fieldstr = NULL;
	} else {
	    fieldstr = GetFieldString(header);
	}
	if(fieldstr == NULL){
	    fprintf(stderr,
		"ERROR: unable to find named field list in file header\n");
	    exit(-1);
	}
	for(i=0;i<ncol;i++){
	    if(colname[i] != NULL){
		/*
		** look it up
		*/
		if(GetFieldInfoByName(fieldstr,colname[i],&fieldinfo)){
		    colnum[i] = fieldinfo.column + 1;
		} else {
		    fprintf(stderr,"unable to find field '%s' in data file header\n",
		    colname[i]);
		    exit(-1);
		}
	    }
	}
    }
    if(nranges > 0){
	if(nnamecol > 0){
	    /*
	    ** look for any named columns that need to be looked up
	    */
	    if(header == NULL){
		header = ReadHeader(fp,&headersize);
	    }
	    if(header == NULL){
		fieldstr = NULL;
	    } else {
		fieldstr = GetFieldString(header);
	    }
	    if(fieldstr == NULL){
		fprintf(stderr,
		    "ERROR: unable to find named field list in file header\n");
		exit(-1);
	    }
	}
	for(i=0;i<nranges;i++){
	    if(colrange[i].name != NULL){
		/*
		** look it up
		*/
		if(GetFieldInfoByName(fieldstr,colrange[i].name,&fieldinfo)){
		    colrange[i].col = fieldinfo.column + 1;
		} else {
		    fprintf(stderr,"unable to find field '%s' in data file header\n",
		    colrange[i].name);
		    exit(-1);
		}
	    }
	    /*
	    ** check for a valid column in the colrange
	    */
	    if(colrange[i].col < 0){
		fprintf(stderr,"invalid column reference\n");
		exit(-1);
	    }
	}
    }
    if(addheader){
	BeginStandardHeader(fpout,argc,argv,VERSION);
	if(nnamecol){
	    fprintf(fpout,"%% Fields:");
	    for(i=0;i<ncol;i++){
		if(colname[i] != NULL){
		    fprintf(fpout,"\t%s",colname[i]);
		} else {
		    fprintf(fpout,"\tcolumn%d",colnum[i]);
		}
	    }
	    fprintf(fpout,"\n");
	}
	EndStandardHeader(fpout);
    }
    while(!feof(fp)){
	if(fgets(line,MAXLINE,fp)== NULL) break;
	/*
	** ignore comment lines
	*/
	if(!includecomments){
	    if(line[0] == '%') continue;
	}
	/*
	** and xplot commands
	*/
	if(line[0] == '/') continue;
	/*
	** parse the line into the argument array
	*/
	if((nactualcol = sgetstrargs(line,MAXCOL,v)) > 0){
	    /*
	    ** check for inclusive column range limitations
	    */
	    for(j=0;j<nranges;j++){
		if(!colrange[j].include) continue;
		fval = atof(v[colrange[j].col-1]);
		if(fval < colrange[j].min || fval > colrange[j].max){
		    break;
		}
	    }
	    if(j!=nranges){
		/*
		** data point failed one of the column range limitations
		** so dont process it
		*/
		continue;
	    }
	    /*
	    ** check for exclusive column range limitations
	    */
	    for(j=0;j<nranges;j++){
		if(colrange[j].include) continue;
		fval = atof(v[colrange[j].col-1]);
		if(fval >= colrange[j].min && fval <= colrange[j].max){
		    break;
		}
	    }
	    if(j!=nranges){
		/*
		** data point failed one of the column range limitations
		** so dont process it
		*/
		continue;
	    }
	    /*
	    ** print out the columns specified in the column list
	    */
	    for(i=0;i<ncol;i++){
		/*
		** test for a valid column
		*/
		col = colnum[i] -1;
		if(col >= 0 && col < nactualcol){
		    if(i!=0){		/* tab separate */
			fprintf(fpout,"\t");
		    }
		    fprintf(fpout,"%s",v[col]);
		}
	    }
	    fprintf(fpout,"\n");
	}
    }
    return(0);
}
