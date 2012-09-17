/*
**************************************************
PROGRAM:
showcmd

DESCRIPTION:
extracts the command line arguments from a standard header and writes
the command line to the output

AUTHOR:
    Written by Matthew Wilson
    ARL Division of Neural Systems, Memory, and Aging
    University of Arizona
    Tucson, AZ 85724
    wilson@nsma.arizona.edu

DATES:
    original xx/xx

**************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <header.h>
#include <iolib.h>

#define VERSION "1.02"

int verbose;

int main(argc,argv)
int	argc;
char 	**argv;
{
int	nxtarg;
FILE	*fpout;
FILE	*fpin;
char	**header;
int	headersize;
char	*argstr;
int	nargs;
int	i;
char	valstr[80];
char	*infilename;
char	*outfilename;

    verbose = 0;
    nxtarg = 0;
    infilename = "STDIN";
    outfilename = "STDOUT";
    fpin = stdin;
    fpout = stdout;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,"usage: %s file [-o file][-v]\n",argv[0]);
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
	if(strcmp(argv[nxtarg],"-o") == 0){
	    if((fpout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	    outfilename = argv[nxtarg];
	} else 
	if(argv[nxtarg][0] != '-'){
	    if((fpin = fopen(argv[nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	    infilename = argv[nxtarg];
	} else {
	    fprintf(stderr,"invalid option '%s'\n",argv[nxtarg]);
	    exit(-1);
	}
    }
    if((header = ReadHeader(fpin,&headersize)) == NULL){
	fprintf(stderr,"ERROR: '%s' does not contain a standard header\n",
	infilename);
	exit(-1);
    }
    if((argstr = GetHeaderParameter(header,"Argc:")) == NULL){
	fprintf(stderr,"ERROR: unable to read arglist from header in '%s'\n",
	infilename);
	exit(-1);
    }
    nargs = atoi(argstr);
    if((argstr = GetHeaderParameter(header,"Program:")) == NULL){
	fprintf(stderr,"ERROR: unable to read arglist from header in '%s'\n",
	infilename);
	exit(-1);
    }
    fprintf(fpout,"%s",argstr);
    for(i=1;i<nargs;i++){
	sprintf(valstr,"Argv[%d] :",i);
	if((argstr = GetHeaderParameter(header,valstr)) == NULL){
	    fprintf(stderr,"ERROR: unable to read arglist from header in '%s'\n",
	    infilename);
	    exit(-1);
	}
	fprintf(fpout," %s",argstr);
    }
    fprintf(fpout,"\n");
    exit(0);
}
