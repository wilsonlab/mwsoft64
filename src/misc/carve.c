
/*
**************************************************
DESCRIPTION:

AUTHOR:
    Written by Matthew Wilson
    Departments of Brain and Cognitive Sciences and Biology
    Massachussetts Institute of Technology
    wilson@ladyday.mit.edu

DATES:
    original xx/xx

**************************************************
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* MWL headers */
#include <iolib.h>
#include <header.h>

#define VERSION "1.01"

int verbose;

int main(argc,argv)
int	argc;
char 	**argv;
{
int	nxtarg;
FILE	*fpout;
FILE	*fpin;
int	count;
int	startbyte;
int	endbyte;
char	byte;

    verbose = 0;
    nxtarg = 0;
    fpin = NULL;
    fpout = NULL;
    startbyte = 0;
    endbyte = -1;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,
	    "usage: %s file [-start byte][-end byte][-o file][-v]\n",
	    argv[0]);
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
	if(strcmp(argv[nxtarg],"-start") == 0){
	    startbyte = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-end") == 0){
	    endbyte = atoi(argv[++nxtarg]);
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
    ** seek to the beginning of the carving point
    */
    fseek(fpin,startbyte,0L);
    count = 0;
    while(!feof(fpin)){
	if(count > endbyte - startbyte) break;
	if(fread(&byte,sizeof(char),1,fpin) != 1){
	    fprintf(stderr,"ERROR: unable to read byte %d\n",
	    startbyte+count);
	    break;
	}
	if(fwrite(&byte,sizeof(char),1,fpout) != 1){
	    fprintf(stderr,"ERROR: unable to write byte %d\n",
	    startbyte+count);
	    break;
	}
	count++;
    }
    fprintf(stderr,"wrote %d bytes\n",count);
    exit(0);
}
