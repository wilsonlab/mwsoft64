#include "pop_ext.h"

int32_t InRange(result,timestamp)
Result	*result;
uint32_t	timestamp;
{
int32_t	inrange;
int32_t	j;

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

int32_t ReadRange(fp,result)
FILE	*fp;
Result  *result;
{
int32_t     size;
char    **header;
uint32_t   timestamp;
char	*filetype;
int32_t	binaryformat;
char	line[201];
char	ef0[10];
char	ef1[10];
char	startstr[30];
char	endstr[30];
uint32_t tstart;
uint32_t tend;
int32_t	convert;

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
	    if(fread(&tstart,sizeof(uint32_t),1,fp) != 1){
		break;
	    }
	    /*
	    ** read the ending timestamp
	    */
	    if(fread(&tend,sizeof(uint32_t),1,fp) != 1){
		break;
	    }
	    if(convert){
		ConvertData(&tstart,sizeof(uint32_t));
		ConvertData(&tend,sizeof(uint32_t));
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

int32_t ReconInRange(result,timestamp)
Result	*result;
uint32_t	timestamp;
{
int32_t	inrange;
int32_t	j;

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

int32_t ReconReadRange(fp,result)
FILE	*fp;
Result  *result;
{
int32_t     size;
char    **header;
uint32_t   timestamp;
char	*filetype;
int32_t	binaryformat;
char	line[201];
char	ef0[10];
char	ef1[10];
char	startstr[30];
char	endstr[30];
uint32_t tstart;
uint32_t tend;
int32_t	convert;

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
	    if(fread(&tstart,sizeof(uint32_t),1,fp) != 1){
		break;
	    }
	    /*
	    ** read the ending timestamp
	    */
	    if(fread(&tend,sizeof(uint32_t),1,fp) != 1){
		break;
	    }
	    if(convert){
		ConvertData(&tstart,sizeof(uint32_t));
		ConvertData(&tend,sizeof(uint32_t));
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
