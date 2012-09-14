#include "pop_ext.h"

int InRange(result,timestamp)
Result	*result;
unsigned long	timestamp;
{
int	inrange;
int	j;

    if(result->nranges == 0) return(1);
    inrange = 0;
    for(j=0;j<result->nranges;j++){
	/*
	** is the spike within the specified range?
	*/
	if((timestamp >= result->range[j].stime) &&
	((result->range[j].etime == 0) ||
	(timestamp <= result->range[j].etime))){
	    inrange = 1;
	    break;
	}
    }
    return(inrange);
}

int ReadRange(fp,result)
FILE	*fp;
Result  *result;
{
long     size;
char    **header;
unsigned long   timestamp;
char	*filetype;
int	binaryformat;
char	line[201];
char	ef0[10];
char	ef1[10];
char	startstr[30];
char	endstr[30];
unsigned long tstart;
unsigned long tend;
int	convert;

    if(fp == NULL) return(0);
    fseek(fp,0L,0L);            /* rewind range file */
    /*
    ** read in the header
    */
    header = ReadHeader(fp,&size);
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
    while(!feof(fp)){           /* scan the entire file */
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
	    if(convert){
		ConvertData(&tstart,sizeof(unsigned long));
		ConvertData(&tend,sizeof(unsigned long));
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
	result->range[result->nranges].stime = tstart;
	result->range[result->nranges].etime = tend;
        /*
        ** increment range count
        */
        result->nranges++;
	if(result->nranges >= result->maxranges){
	    fprintf(stderr,"WARNING: exceeded maximum number of ranges\n",
	    MAXRANGES);
	    break;
	}
    }
    return(result->nranges);
}

int ReconInRange(result,timestamp)
Result	*result;
unsigned long	timestamp;
{
int	inrange;
int	j;

    if(result->nreconranges == 0) return(1);
    inrange = 0;
    for(j=0;j<result->nreconranges;j++){
	/*
	** is the spike within the specified range?
	*/
	if((timestamp >= result->reconrange[j].stime) &&
	((result->reconrange[j].etime == 0) ||
	(timestamp <= result->reconrange[j].etime))){
	    inrange = 1;
	    break;
	}
    }
    return(inrange);
}

int ReconReadRange(fp,result)
FILE	*fp;
Result  *result;
{
long     size;
char    **header;
unsigned long   timestamp;
char	*filetype;
int	binaryformat;
char	line[201];
char	ef0[10];
char	ef1[10];
char	startstr[30];
char	endstr[30];
unsigned long tstart;
unsigned long tend;
int	convert;

    if(fp == NULL) return(0);
    fseek(fp,0L,0L);            /* rewind range file */
    /*
    ** read in the header
    */
    header = ReadHeader(fp,&size);
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
    while(!feof(fp)){           /* scan the entire file */
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
	    if(convert){
		ConvertData(&tstart,sizeof(unsigned long));
		ConvertData(&tend,sizeof(unsigned long));
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
	result->reconrange[result->nreconranges].stime = tstart;
	result->reconrange[result->nreconranges].etime = tend;
        /*
        ** increment range count
        */
        result->nreconranges++;
	if(result->nreconranges >= result->maxranges){
	    fprintf(stderr,"WARNING: exceeded maximum number of ranges\n",
	    MAXRANGES);
	    break;
	}
    }
    return(result->nreconranges);
}
