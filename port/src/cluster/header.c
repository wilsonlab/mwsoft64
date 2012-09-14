/*
*******************************************************************
PROGRAM:
    header

DESCRIPTION:
    displays/strips the standard file header from data files

AUTHOR:
    Written by Matthew Wilson
    ARL Division of Neural Systems, Memory, and Aging
    University of Arizona
    Tucson, AZ 85724
    wilson@nsma.arizona.edu

DATES:
    program update 11/91
*******************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iolib.h>

/* MWL Headers */
#include "header.h"

#define VERSION "1.05"

#define BUFFERSIZE 1024

int verbose;

int main(argc,argv)
int	argc;
char 	**argv;
{
int	nxtarg;
FILE	*fpin;
FILE	*fpout;
char	**header;
long 	headersize;
char	*fname;
int	strip;
int	nchars;
char	buffer[BUFFERSIZE];

    verbose = 0;
    strip = 0;
    nxtarg = 0;
    fpin = stdin;
    fname = "none";
    fpout = stdout;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,"usage: %s [file][-o file][-strip]\n",argv[0]);
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
	if(strcmp(argv[nxtarg],"-strip") == 0){
	    strip = 1;
	} else
	if(strcmp(argv[nxtarg],"-o") == 0){
	    if((fpout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(argv[nxtarg][0] != '-'){
	    fname = argv[nxtarg];
	    if((fpin = fopen(argv[nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else {
	    fprintf(stderr,"invalid option '%s'\n",argv[nxtarg]);
	    exit(-1);
	}
    }
    if(fpin == NULL){
	fprintf(stderr,"Input file must be specified\n");
	exit(-1);
    }
    header = ReadHeader(fpin,&headersize);
/*
    if(!strip){
	fprintf(fpout,"File: \t%s\n",fname);
	fprintf(stderr,"Header size: %d bytes\n",headersize);
    }
*/
    if(verbose){
	fprintf(stderr,"File: \t%s\n",fname);
	fprintf(stderr,"Header size: %ld bytes\n",headersize);
    }
    /*
    ** this leaves the file pointer for fpin just after the header
    */
    /*
    ** if the strip option is selected then output everything past the
    ** header, otherwise just output the header
    */
    if(strip){
	while(!feof(fpin)){
	    if((nchars = fread(buffer,sizeof(char),BUFFERSIZE,fpin)) == 0){
		break;
	    }
	    if(fwrite(buffer,sizeof(char),nchars,fpout) != nchars){
		perror("error writing output file");
		exit(-1);
	    }
	}
    } else {
	DisplayHeader(fpout,header,headersize);
    }
    if(headersize > 0){
	exit(0);
    } else {
	exit(1);
    }
}
