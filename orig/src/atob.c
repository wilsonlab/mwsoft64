/*
****************************************************
DESCRIPTION:
    convert an ascii file to binary format

AUTHOR:
    Written by Matthew Wilson
    ARL Division of Neural Systems, Memory, and Aging
    University of Arizona
    Tucson, AZ 85724
    wilson@nsma.arizona.edu

DATES:
    update 11/91

****************************************************
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <header.h>
#include <iolib.h>

#define VERSION "1.03"

int main(argc,argv)
int argc;
char **argv;
{
char	*afile,*bfile;
FILE	*afp,*bfp;
int	nxtarg;
float	fval;
short	sval;
int	ival;
double	dval;
unsigned long ulval;
char cval;
int	format;
int	outputformat;
double	scale;
short	differentoutputformat;
char	*outputformatstr;
char	line[1000];
char	**header;
int	headersize;
char	*fieldstr;
FieldInfo	fieldinfo;
char	*fieldname;

    afp = stdin;
    bfp = stdout;
    format = INT;
    nxtarg = 0;
    scale = 1;
    differentoutputformat = 0;
    fieldname = NULL;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,
	    "usage: %s [ascfile][-o binfile][-scale factor]\n",
	    argv[0]);
	    fprintf(stderr,"\t[-ulong][-char][-short][-int][-float][-double][\n");
	    fprintf(stderr,"\t[-output ulong char short int float double]\n");
	    exit(0);
	} else
	if(strcmp(argv[nxtarg],"-o") == 0){
	    bfile = argv[++nxtarg];
	    if((bfp = fopen(bfile,"w")) == NULL){
		fprintf(stderr,"unable to open binary file '%s'\n",bfile);
		exit(0);
	    }
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
	if(strcmp(argv[nxtarg],"-double") == 0){
	    format = DOUBLE;
	} else
	if(strcmp(argv[nxtarg],"-float") == 0){
	    format = FLOAT;
	} else
	if(strcmp(argv[nxtarg],"-int") == 0){
	    format = INT;
	} else
	if(strcmp(argv[nxtarg],"-short") == 0){
	    format = SHORT;
	} else
	if(strcmp(argv[nxtarg],"-ulong") == 0){
	    format = ULONG;
	} else
	if(strcmp(argv[nxtarg],"-char") == 0){
	    format = CHAR;
	} else
	if(strcmp(argv[nxtarg],"-scale") == 0){
	    scale = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-fieldname") == 0){
	    fieldname = argv[++nxtarg];
	} else
	if(strcmp(argv[nxtarg],"-output") == 0){
	    outputformatstr = argv[++nxtarg];
	    if(strcmp(outputformatstr,"char") == 0){
		outputformat = CHAR;
	    } else
	    if(strcmp(outputformatstr,"ulong") == 0){
		outputformat = ULONG;
	    } else
	    if(strcmp(outputformatstr,"short") == 0){
		outputformat = SHORT;
	    } else
	    if(strcmp(outputformatstr,"int") == 0){
		outputformat = INT;
	    } else
	    if(strcmp(outputformatstr,"float") == 0){
		outputformat = FLOAT;
	    } else
	    if(strcmp(outputformatstr,"double") == 0){
		outputformat = DOUBLE;
	    } else
	    {
		fprintf(stderr,"unrecognized output format %s\n",outputformatstr);
		exit(-1);
	    }
	    differentoutputformat = 1;
	} else
	if(argv[nxtarg][0] != '-'){
	    afile = argv[nxtarg];
	    if((afp = fopen(afile,"r")) == NULL){
		fprintf(stderr,"unable to open ascii file '%s'\n",afile);
		exit(0);
	    }
	}
    }
    /*
    ** if an output format has not been specified then assume that it is the
    ** same as the input format
    */
    if(!differentoutputformat){
	outputformat = format;
    }
    /*
    ** try to read the field information from the header
    */
    if(fieldname == NULL){
	if((header = ReadHeader(afp,&headersize)) == NULL){
	    fprintf(stderr,"no header information\n");
	    fieldname = "data";
	} else {
	    if((fieldstr = GetFieldString(header)) != NULL){
		GetFieldInfoByNumber(fieldstr,0,&fieldinfo);
		fieldname = fieldinfo.name;
	    } else {
		fieldname = "data";
	    }
	}
    }
    BeginStandardHeader(bfp,argc,argv,VERSION);
    fprintf(bfp,"%% File type: Binary\n");
    switch(outputformat){
    case CHAR:
    	fprintf(bfp,"%% Fields: %s,%d,%d,%d\n",fieldname,CHAR,sizeof(char),1);
	break;
    case SHORT:
    	fprintf(bfp,"%% Fields: %s,%d,%d,%d\n",fieldname,SHORT,sizeof(short),1);
	break;
    case INT:
    	fprintf(bfp,"%% Fields: %s,%d,%d,%d\n",fieldname,INT,sizeof(int),1);
	break;
    case FLOAT:
    	fprintf(bfp,"%% Fields: %s,%d,%d,%d\n",fieldname,FLOAT,sizeof(float),1);
	break;
    case DOUBLE:
    	fprintf(bfp,"%% Fields: %s,%d,%d,%d\n",fieldname,DOUBLE,sizeof(double),1);
	break;
    case ULONG:
    	fprintf(bfp,"%% Fields: %s,%d,%d,%d\n",fieldname,ULONG,sizeof(unsigned long),1);
	break;
    }
    EndStandardHeader(bfp);
    while(!feof(afp)){
	if(fgets(line,1000,afp) == NULL) break;
	/*
	** ignore comments
	*/
	if(line[0] == '%') continue;
	switch(format){
	case ULONG:
	    if(sscanf(line,"%lu",&ulval) > 0){
		switch(outputformat){
		case CHAR:
		    cval = ulval*scale;
		    fwrite(&cval,sizeof(char),1,bfp);
		    break;
		case SHORT:
		    sval = ulval*scale;
		    fwrite(&sval,sizeof(short),1,bfp);
		    break;
		case INT:
		    ival = ulval*scale;
		    fwrite(&ival,sizeof(int),1,bfp);
		    break;
		case FLOAT:
		    fval = ulval*scale;
		    fwrite(&fval,sizeof(float),1,bfp);
		    break;
		case DOUBLE:
		    dval = ulval*scale;
		    fwrite(&dval,sizeof(double),1,bfp);
		    break;
		case ULONG:
		    ulval = ulval*scale;
		    fwrite(&ulval,sizeof(unsigned long),1,bfp);
		    break;
		}
	    }
	    break;
	case CHAR:
	    if(sscanf(line,"%c",&cval) > 0){
		switch(outputformat){
		case CHAR:
		    cval = cval*scale;
		    fwrite(&cval,sizeof(char),1,bfp);
		    break;
		case SHORT:
		    sval = cval*scale;
		    fwrite(&sval,sizeof(short),1,bfp);
		    break;
		case INT:
		    ival = cval*scale;
		    fwrite(&ival,sizeof(int),1,bfp);
		    break;
		case FLOAT:
		    fval = cval*scale;
		    fwrite(&fval,sizeof(float),1,bfp);
		    break;
		case DOUBLE:
		    dval = cval*scale;
		    fwrite(&dval,sizeof(double),1,bfp);
		    break;
		case ULONG:
		    ulval = cval*scale;
		    fwrite(&ulval,sizeof(unsigned long),1,bfp);
		    break;
		}
	    }
	    break;
	case INT:
	    if(sscanf(line,"%d",&ival) > 0){
		switch(outputformat){
		case CHAR:
		    cval = ival*scale;
		    fwrite(&cval,sizeof(char),1,bfp);
		    break;
		case SHORT:
		    sval = ival*scale;
		    fwrite(&sval,sizeof(short),1,bfp);
		    break;
		case INT:
		    ival = ival*scale;
		    fwrite(&ival,sizeof(int),1,bfp);
		    break;
		case FLOAT:
		    fval = ival*scale;
		    fwrite(&fval,sizeof(float),1,bfp);
		    break;
		case DOUBLE:
		    dval = ival*scale;
		    fwrite(&dval,sizeof(double),1,bfp);
		    break;
		case ULONG:
		    ulval = ival*scale;
		    fwrite(&ulval,sizeof(unsigned long),1,bfp);
		    break;
		}
	    }
	    break;
	case FLOAT:
	    if(sscanf(line,"%f",&fval) > 0){
		switch(outputformat){
		case CHAR:
		    cval = fval*scale + 0.5 ;
		    fwrite(&cval,sizeof(char),1,bfp);
		    break;
		case SHORT:
		    sval = fval*scale + 0.5 ;
		    fwrite(&sval,sizeof(short),1,bfp);
		    break;
		case INT:
		    ival = fval*scale + 0.5 ;
		    fwrite(&ival,sizeof(int),1,bfp);
		    break;
		case FLOAT:
		    fval = fval*scale;
		    fwrite(&fval,sizeof(float),1,bfp);
		    break;
		case DOUBLE:
		    dval = fval*scale;
		    fwrite(&dval,sizeof(double),1,bfp);
		    break;
		case ULONG:
		    ulval = fval*scale + 0.5 ;
		    fwrite(&ulval,sizeof(unsigned long),1,bfp);
		    break;
		}
	    }
	    break;
	case DOUBLE:
	    if(sscanf(line,"%lf",&dval) > 0){
		switch(outputformat){
		case CHAR:
		    cval = dval*scale + 0.5 ;
		    fwrite(&cval,sizeof(char),1,bfp);
		    break;
		case SHORT:
		    sval = dval*scale + 0.5 ;
		    fwrite(&sval,sizeof(short),1,bfp);
		    break;
		case INT:
		    ival = dval*scale + 0.5 ;
		    fwrite(&ival,sizeof(int),1,bfp);
		    break;
		case FLOAT:
		    fval = dval*scale;
		    fwrite(&fval,sizeof(float),1,bfp);
		    break;
		case DOUBLE:
		    dval = dval*scale;
		    fwrite(&dval,sizeof(double),1,bfp);
		    break;
		case ULONG:
		    ulval = dval*scale + 0.5 ;
		    fwrite(&ulval,sizeof(unsigned long),1,bfp);
		    break;
		}
	    }
	    break;
	}
    }
    return(0);
}
