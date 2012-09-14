
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
    original 9/95

**************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <header.h>
#include <iolib.h>

#define VERSION "1.18"

#define MAXRECTYPES	20

#define MAXSUBFIELDS	100
#ifndef SEEK_CUR
#define SEEK_CUR	1
#endif

#define EVENTSTRLEN	80

#define EVENTREC	'e'
#define POSREC	'T'
#define SPKREC	's'
#define CONTREC	'c'

typedef struct subfield_type {
    int		field;
    char	*name;
} Subfield;

typedef struct rec_type {
    unsigned char	id;
    int			size;
    int			nfields;
    FieldInfo		*field;
} RecordInfo;

typedef struct rec_data {
    char	type;
    char	*data;
} RecordData;

typedef struct result_type {
    FILE	*fpout;
    FILE	*fpin;
    int		nrectypes;
    RecordInfo	rectype[MAXRECTYPES];
    int		file_offset;
    char	*recdata;
    int		recoffset;
    int		nsubfields;
    Subfield	subfield[MAXSUBFIELDS];
    char	requested_recid;
    int		probenumber;
    int		probefield;
    int		nitemsfield;
    int		buggyad;
    int		bufsize;
    int		nchannels;
} Result;

int verbose;
int	convert;
int	eventstrlen;

/* Swap the contents of two byte-locations */
static void swapbyte(char *x,char *y)
{
char temp;
    
    temp = *x;
    *x = *y;
    *y = temp;
}

/*
** convert two byte quantities by byte swapping
** Pass the address of the 2 byte value to be converted
** it is converted in place
*/
static void convert2byte(char *x)
{
  swapbyte(x, x+1);
}

/*
** reverse the order of all bytes
** this works on floats and ints
*/
static void convert4byte(char *x)
{
  swapbyte(x, x+3);
  swapbyte(x+1, x+2);
}

/*
** reverse the order of all bytes
** this works on floats and ints
*/
static void convert8byte(char *x)
{
  swapbyte(x, x+7);
  swapbyte(x+1, x+6);
  swapbyte(x+2, x+5);
  swapbyte(x+3, x+4);
}

/*
** convert each field in a record
*/	
static void ConvertField(char *field,FieldInfo *fieldinfo)
{

    switch(fieldinfo->size){
    case 2:
	convert2byte(field);
	break;
    case 4:
	convert4byte(field);
	break;
    case 8:
	convert8byte(field);
	break;
    default:
	break;
    }
}

RecordInfo *FindRecordType(Result *result,char rectype)
{
int	i;
RecordInfo	*currenttype;

    /*
    ** go through all of the defined record types
    ** and match the id
    */
    currenttype = NULL;
    for(i=0;i<result->nrectypes;i++){
	if(result->rectype[i].id == rectype){
	    /*
	    ** dont stop in case of multiple definitions, take the last
	    */
	    currenttype = &result->rectype[i];
	}
    }
    return(currenttype);
}

unsigned char GetUCharField(Result *result,RecordInfo *recinfo,int field)
{
int	j,k;
int	offset;
int	alignment;

    offset = 0;
    for(j=0;j<field;j++){
	/*
	** calculate the necessary alignment
	** In linux, the alignment is to the size of the
	** data type. e.g. 2 byte short -> 2 byte aligned.
	** Under DOS it may be zero.
	** This will be different dependent on the source
	** architecture. This info should really be placed in
	** the file header 
	*/
	for(k=0;k<recinfo->field[j].count;k++){
	    alignment = offset%recinfo->field[j].size;
	    alignment = 0;
	    offset += recinfo->field[j].size + alignment;
	}
    }
    return(*((unsigned char *)(result->recdata + offset)));
}

short GetShortField(Result *result,RecordInfo *recinfo,int field)
{
int	j,k;
int	offset;
int	alignment;
short	tmpval;

    offset = 0;
    for(j=0;j<field;j++){
	/*
	** calculate the necessary alignment
	** In linux, the alignment is to the size of the
	** data type. e.g. 2 byte short -> 2 byte aligned.
	** Under DOS it may be zero.
	** This will be different dependent on the source
	** architecture. This info should really be placed in
	** the file header 
	*/
	for(k=0;k<recinfo->field[j].count;k++){
	    alignment = offset%recinfo->field[j].size;
	    alignment = 0;
	    offset += recinfo->field[j].size + alignment;
	}
    }
    bcopy(result->recdata + offset,&tmpval,sizeof(short));
    if(convert)
    ConvertField((char *)(&tmpval), &recinfo->field[j]);
    return(tmpval);
}

RecordInfo *GetRecord(Result *result)
{
char	tmprecid;
char	recid;
char	recid2;
RecordInfo	*recinfo;
int	nitems;
int	i;

    /*
    ** note that the structure of every record is 
    ** recid - data - recid
    ** where recid is a single character
    ** Because of a bug in older ad code, extra recids were placed at
    ** the beginning and end of the buffers containing multiple spikes.
    ** Hence, some spike records will have an extra recid at either the
    ** beginning or end. 
    */
    if((result->requested_recid == POSREC) && result->buggyad){
	/*
	** HACK ALERT
	**__________________________
	** old ad code neglected to put in the recids for pos records
	*/
	recid = POSREC;
    } else {
    /*
    ** read in the field type indicator
    */
    if(fread(&recid,sizeof(char),1,result->fpin) != 1){
	/*
	fprintf(stderr,"\nerror reading record id\n");
	*/
	return(NULL);
    }
    }
    if(recid == SPKREC  && result->buggyad){
	/*
	** HACK ALERT
	**__________________________
	** ok, now we have to do this painful hack to check for the
	** extra recids at the beginning of the record
	*/
	if(fread(&tmprecid,sizeof(char),1,result->fpin) != 1){
	    fprintf(stderr,"\nerror reading record id\n");
	    return(NULL);
	}
	/*
	** check it. If it is not a recid then go back
	*/
	if(tmprecid != recid){
	    fseek(result->fpin,-1,SEEK_CUR);
	} else {
	    if(verbose){
		fprintf(stderr,"found extra start recid '%c' at offset %ld\n",
		tmprecid,
		ftell(result->fpin)-2);
	    }
	}
	/* 
	** END OF HACK
	**__________________________
	*/
    }

    /*
    ** find the record information based on the type 
    */
    if((recinfo = FindRecordType(result,recid)) == NULL){
	fprintf(stderr,
	"\nunable to find information on record type '%c'(%d) at offset %ld\n",
	recid,(int)recid,ftell(result->fpin));
	return(NULL);
    }
    /*
    ** have to process position records differently due to the 
    ** variable record length. The record size must be computed
    ** for each record
    */
    if(recid == POSREC){
	/*
	** add up the size of the first 3 fields
	*/
	recinfo->size = 0;
	for(i=0;i<3;i++){
	    recinfo->size += recinfo->field[i].size;
	}
	/*
	** read in the nitems field
	*/
	result->recdata = (char *)malloc(recinfo->size);
	result->recoffset = ftell(result->fpin);
	if(fread(result->recdata,recinfo->size,1,result->fpin) != 1){
	    fprintf(stderr,"\nerror reading nitems at offset %d\n",
	    result->recoffset);
	    return(NULL);
	}
	/*
	** go back
	*/
	fseek(result->fpin,-(long)recinfo->size,SEEK_CUR);
	nitems  = GetUCharField(result,recinfo,result->nitemsfield);
	/*
	** then add the size of the xy coords
	*/
	for(i=0;i<nitems;i++){
	    recinfo->size += recinfo->field[3].size;
	    recinfo->size += recinfo->field[4].size;
	}
	free(result->recdata);
    } 
    /*
    ** read in the record 
    */
    result->recdata = (char *)malloc(recinfo->size);
    result->recoffset = ftell(result->fpin);
    if(fread(result->recdata,recinfo->size,1,result->fpin) != 1){
	fprintf(stderr,"\nerror reading record data\n");
	return(NULL);
    }
    /*
    ** check the recid at the end to make sure the record is ok
    */
    if((result->requested_recid == POSREC) && result->buggyad){
	/*
	** HACK ALERT
	**__________________________
	** old ad code neglected to put in the recids for pos records
	*/
	recid2 = POSREC;
    } else {
    if(fread(&recid2,sizeof(char),1,result->fpin) != 1){
	fprintf(stderr,"\nerror reading record terminal id\n");
	return(NULL);
    }
    }
    /*
    ** older ad cold neglected to place a valid recid at the end
    ** of the record, so dont check it for now.
    */
    if(recid != recid2){
    /*
	fprintf(stderr,"\ncorrupted record of type '%c' at offset %d\n",
	recid, result->file_offset);
	return(NULL);
    */
    }
    if(recid == SPKREC && result->buggyad){
	/*
	** HACK ALERT
	**__________________________
	** ok, now we have to do this painful hack to check for the
	** extra recids
	*/
	/*
	** first get what could be a valid beginning of record recid
	*/
	if(fread(&tmprecid,sizeof(char),1,result->fpin) != 1){
	    fprintf(stderr,"\nerror reading record id\n");
	    return(NULL);
	}
	/*
	** now get what could be a bogus end of record recid
	*/
	if(fread(&tmprecid,sizeof(char),1,result->fpin) != 1){
	    fprintf(stderr,"\nerror reading record id\n");
	    return(NULL);
	}
	/*
	** check it. If it is not a recid then go back
	*/
	if(tmprecid != recid){
	    fseek(result->fpin,-2,SEEK_CUR);
	} else {
	    fseek(result->fpin,-1,SEEK_CUR);
	    if(verbose){
		fprintf(stderr,"found extra end recid '%c' at offset %ld\n",
		tmprecid,
		ftell(result->fpin)-1);
	    }
	}
	/* 
	** END OF HACK
	**__________________________
	*/
    }
    /*
    ** update the file offset 
    */
    result->file_offset = ftell(result->fpin);
    return(recinfo);
}

void WriteRequestedFields(Result *result,RecordInfo *recinfo)
{
int	i,j,k;
int	type;
int	size;
int	count;
int	field;
int	offset;
int	foffset;
int	alignment;

    /*
    ** go through each of the subfields and locate the corresponding
    ** field in the record
    */
    for(i=0;i<result->nsubfields;i++){
	field = result->subfield[i].field;
	type = recinfo->field[field].type;
	size = recinfo->field[field].size;
	count = recinfo->field[field].count;
	offset = 0;
	for(j=0;j<field;j++){
	    /*
	    ** calculate the necessary alignment
	    ** In linux, the alignment is to the size of the
	    ** data type. e.g. 2 byte short -> 2 byte aligned.
	    ** Under DOS it may be zero.
	    ** This will be different dependent on the source
	    ** architecture. This info should really be placed in
	    ** the file header 
	    */
	    for(k=0;k<recinfo->field[j].count;k++){
		alignment = offset%recinfo->field[j].size;
		alignment = 0;
		offset += recinfo->field[j].size + alignment;
	    }
	}
	/*
	** calculate the offset of the field in the record
	*/
	/*
	** and convert all data elements in the field
	*/
	foffset = offset;
	for(k=0;k<count;k++){
	    if(convert)
	    ConvertField(result->recdata + foffset,
		&recinfo->field[field]);
	    foffset += recinfo->field[j].size;
	}
	/*
	** and write out the field
	*/
	if(fwrite(result->recdata + offset, size,count,result->fpout) != count){
	    fprintf(stderr,"unable to write output file\n");
	    exit(-1);
	}
    }
}


void FreeRecord(Result *result)
{
    if(result->recdata)
    free(result->recdata);
}

void LoadDefaultRecordTypes(Result *result)
{
RecordInfo	*rectype;
    /*
    ** preload several record types because they may not be defined
    ** in older ad headers. note that the 2 'id' fields chars is a hack to deal
    ** with extra recids which were placed in older data files.
    ** newer versions of ad correct this and the header definition of
    ** the 's' record should override this definition
    */
    rectype = &result->rectype[result->nrectypes];
    rectype->id = SPKREC;
    rectype->size = sizeof(short) + sizeof(unsigned long) + 128*sizeof(short);
    rectype->nfields = 3;
    rectype->field = 
	(FieldInfo *)calloc(rectype->nfields,
	sizeof(FieldInfo));
	/*
    rectype->field[0].name = "id";
    rectype->field[0].type = CHAR;
    rectype->field[0].size = sizeof(char);
    rectype->field[0].count = 1;
    */
    rectype->field[0].name = "electrode";
    rectype->field[0].type = SHORT;
    rectype->field[0].size = sizeof(short);
    rectype->field[0].count = 1;
    rectype->field[1].name = "timestamp";
    rectype->field[1].type = ULONG;
    rectype->field[1].size = sizeof(unsigned long);
    rectype->field[1].count = 1;
    rectype->field[2].name = "data";
    rectype->field[2].type = SHORT;
    rectype->field[2].size = sizeof(short);
    rectype->field[2].count = 128;
    /*
    rectype->field[4].name = "id";
    rectype->field[4].type = CHAR;
    rectype->field[4].size = sizeof(char);
    rectype->field[4].count = 1;
    */

    result->nrectypes++;

    rectype = &result->rectype[result->nrectypes];
    rectype->id = POSREC;
    rectype->size = sizeof(unsigned long) + sizeof(short) + 
	3*sizeof(unsigned char);
    rectype->nfields = 5;
    rectype->field = 
	(FieldInfo *)calloc(rectype->nfields,
	sizeof(FieldInfo));
    rectype->field[2].name = "timestamp";
    rectype->field[2].type = ULONG;
    rectype->field[2].size = sizeof(unsigned long);
    rectype->field[2].count = 1;
    rectype->field[1].name = "frame";
    rectype->field[1].type = CHAR;
    rectype->field[1].size = sizeof(unsigned char);
    rectype->field[1].count = 1;
    rectype->field[0].name = "nitems";
    rectype->field[0].type = CHAR;
    rectype->field[0].size = sizeof(unsigned char);
    rectype->field[0].count = 1;
    rectype->field[3].name = "x coord";
    rectype->field[3].type = SHORT;
    rectype->field[3].size = sizeof(short);
    rectype->field[3].count = 1;
    rectype->field[4].name = "y coord";
    rectype->field[4].type = CHAR;
    rectype->field[4].size = sizeof(unsigned char);
    rectype->field[4].count = 1;

    result->nrectypes++;
    rectype = &result->rectype[result->nrectypes];
    rectype->id = EVENTREC;
    rectype->size = sizeof(unsigned long) + eventstrlen*sizeof(char);
    rectype->nfields = 2;
    rectype->field = 
	(FieldInfo *)calloc(rectype->nfields,
	sizeof(FieldInfo));
    rectype->field[0].name = "timestamp";
    rectype->field[0].type = ULONG;
    rectype->field[0].size = sizeof(unsigned long);
    rectype->field[0].count = 1;
    rectype->field[1].name = "string";
    rectype->field[1].type = CHAR;
    rectype->field[1].size = sizeof(char);
    rectype->field[1].count = eventstrlen;
    result->nrectypes++;

    rectype = &result->rectype[result->nrectypes];
    rectype->id = CONTREC;
    rectype->size = sizeof(unsigned long) + result->bufsize*sizeof(short);
    rectype->nfields = 2;
    rectype->field = 
	(FieldInfo *)calloc(rectype->nfields,
	sizeof(FieldInfo));
    rectype->field[0].name = "timestamp";
    rectype->field[0].type = ULONG;
    rectype->field[0].size = sizeof(unsigned long);
    rectype->field[0].count = 1;
    rectype->field[1].name = "cont data";
    rectype->field[1].type = SHORT;
    rectype->field[1].size = sizeof(short);
    rectype->field[1].count = result->bufsize;
    result->nrectypes++;
}

void ProcessPositionRecord(Result *result,RecordInfo *recinfo)
{
int	i;
int	size;
int	offset;
int	nitems;

    /*
    ** go through each of the subfields and locate the corresponding
    ** field in the record
    */
    nitems  = GetUCharField(result,recinfo,result->nitemsfield);
    offset = 0;
    /*
    ** convert the nitems
    */
    if(convert)
    ConvertField(result->recdata + offset,
	&recinfo->field[0]);
    offset += recinfo->field[0].size;
    /*
    ** convert the frame
    */
    if(convert)
    ConvertField(result->recdata + offset,
	&recinfo->field[1]);
    offset += recinfo->field[1].size;

    /*
    ** convert the timestamp
    */
    if(convert)
    ConvertField(result->recdata + offset,
	&recinfo->field[2]);
    offset += recinfo->field[2].size;
    /*
    ** convert each of the xy coords
    */
    for(i=0;i<nitems;i++){
	/*
	** x coord
	*/
	if(convert)
	ConvertField(result->recdata + offset,
	    &recinfo->field[3]);
	offset += recinfo->field[3].size;
	/*
	** y coord
	*/
	if(convert)
	ConvertField(result->recdata + offset,
	    &recinfo->field[4]);
	offset += recinfo->field[4].size;
    }
    size = offset;
    /*
    ** and write out the whole record
    */
    if(fwrite(result->recdata,size,1,result->fpout) != 1){
	fprintf(stderr,"unable to write output file\n");
	exit(-1);
    }
}

int main(argc,argv)
int	argc;
char 	**argv;
{
int	nxtarg;
char	**header;
int	headersize;
RecordInfo	*recinfo;
char	tmpstr[80];
int	i,j;
Result	result;
char	*extracttype = NULL;
char	*recstr;
char	*fieldstr;
FieldInfo	fieldinfo;
int	reccount;
int	totcount;
short	probeval;
int	checkprobe;
char	*fnamein = NULL;

    verbose = 0;
    nxtarg = 0;
    result.fpin = NULL;
    result.fpout = NULL;
    result.nrectypes = 0;
    result.probefield = -1;
    result.nitemsfield = -1;
    result.probenumber = -1;
    result.buggyad = 0;
    result.bufsize = -1;
    result.nchannels = -1;
    reccount = 0;
    totcount = 0;
    convert = 0;
    checkprobe = 0;
    eventstrlen = EVENTSTRLEN;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,"usage: %s file [options]\n",argv[0]);
	    fprintf(stderr,"Available options:\n");
	    fprintf(stderr,"\t-v	verbose\n");
	    fprintf(stderr,"\t-p	extract position records\n");
	    fprintf(stderr,"\t-t	extract tetrode spike records\n");
	    fprintf(stderr,"\t-e	extract event string records\n");
	    fprintf(stderr,"\t-c	extract continuous data records\n");
	    fprintf(stderr,"\t-eslen40	older 40 char eventstring format\n");
	    fprintf(stderr,"\t-eslen80	newer 80 char eventstring format\n");
	    fprintf(stderr,"\t-probe #	electrode #\n");
	    fprintf(stderr,"\t-o file	output file\n");
	    fprintf(stderr,"\tfile	input data file\n");
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
	if(strcmp(argv[nxtarg],"-eslen40") == 0){
	    eventstrlen = 40;
	} else
	if(strcmp(argv[nxtarg],"-eslen80") == 0){
	    eventstrlen = 80;
	} else
	if(strcmp(argv[nxtarg],"-probe") == 0){	/* probe number */
	    result.probenumber = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-bug") == 0){	/* tetrode waveforms */
	    result.buggyad = 1;
	} else
	if(strcmp(argv[nxtarg],"-c") == 0){	/* continuous data */
	    extracttype = "continuous data";
	    checkprobe = 0;
	    result.requested_recid = CONTREC;
	    result.nsubfields = 2;
	    result.subfield[0].field = 0;
	    result.subfield[0].name = "timestamp";
	    result.subfield[1].field = 1;
	    result.subfield[1].name = "data";
	} else 
	if(strcmp(argv[nxtarg],"-e") == 0){	/* event string */
	    extracttype = "event strings";
	    checkprobe = 0;
	    result.requested_recid = EVENTREC;
	    result.nsubfields = 2;
	    result.subfield[0].field = 0;
	    result.subfield[0].name = "timestamp";
	    result.subfield[1].field = 1;
	    result.subfield[1].name = "string";
	} else 
	if(strcmp(argv[nxtarg],"-t") == 0){	/* tetrode waveforms */
	    extracttype = "tetrode waveforms";
	    checkprobe = 1;
	    result.requested_recid = SPKREC;
	    result.probefield = 0;
	    result.nsubfields = 2;
	    result.subfield[0].field = 1;
	    result.subfield[0].name = "timestamp";
	    result.subfield[1].field = 2;
	    result.subfield[1].name = "waveform";
	} else 
	if(strcmp(argv[nxtarg],"-p") == 0){	/* dual diode positions */
	    extracttype = "extended dual diode position";
	    checkprobe = 0;
	    result.requested_recid = POSREC;
	    result.nsubfields = 5;
	    result.subfield[2].field = 2;
	    result.subfield[2].name = "timestamp";
	    result.subfield[1].field = 1;
	    result.subfield[1].name = "frame";
	    result.subfield[0].field = 0;
	    result.subfield[0].name = "nitems";
	    result.subfield[3].field = 3;
	    result.subfield[3].name = "target x";
	    result.subfield[4].field = 4;
	    result.subfield[4].name = "target y";
	    result.nitemsfield = 0;
	} else 
	if(strcmp(argv[nxtarg],"-o") == 0){
	    if((result.fpout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(argv[nxtarg][0] != '-'){
	    fnamein = argv[nxtarg];
	    if((result.fpin = fopen(argv[nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else {
	    fprintf(stderr,"invalid option '%s'\n",argv[nxtarg]);
	    exit(-1);
	}
    }
    if(result.fpin == NULL){
	fprintf(stderr,"%s: input data file must be specified\n",argv[0]);
	exit(-1);
    }
    if(result.probenumber < 0 && checkprobe){
	fprintf(stderr,"%s: probe number must be specified\n",argv[0]);
	exit(-1);
    }
    if(verbose){
	fprintf(stderr,"Welcome to %s\n",argv[0]);
    }
    header = ReadHeader(result.fpin,&headersize);
    /*
    ** compare architectures
    */
    if(GetLocalArchitecture() == ARCH_SPARC){
	fprintf(stderr,"Converting '%s' from DOS to Sparc format\n",
	extracttype);
	convert = 1;
    } else {
	fprintf(stderr,"Extracting '%s' from DOS format: No conversion\n",
	extracttype);
	convert = 0;
    }

    /*
    ** get info on the number of channels
    */
    if((recstr = GetHeaderParameter(header,"nchannels:")) != NULL){
	result.nchannels = atoi(recstr);
	if(verbose){
	    fprintf(stderr,"Nchannels %d\n",result.nchannels);
	}
    } 
    /*
    ** get info on the buffer size
    */
    if((recstr = GetHeaderParameter(header,"dma_bufsize:")) != NULL){
	result.bufsize = atoi(recstr);
	if(verbose){
	    fprintf(stderr,"Bufsize %d\n",result.bufsize);
	}
    } 
    if((result.bufsize < 0) && (result.requested_recid == CONTREC)){
	fprintf(stderr,"invalid continuous record bufsize\n");
	exit(-1);
    }
    LoadDefaultRecordTypes(&result);
    /*
    ** get the list of record types contained in the file
    ** This is not essential, but if it isnt there, only the default
    ** types (s,p,e) will be searched
    */
    if((recstr = GetHeaderParameter(header,"Record Types:")) != NULL){
	/*
	** get the record types defined in the file by scanning the string
	** contained in the Record Types: field
	*/
	for(i=0;i<strlen(recstr);i++){
	    /* skip white space */
	    if((recstr[i] == ' ') || (recstr[i] == ',') || (recstr[i] == '\t') ||
		(recstr[i] == '\n') ) continue;
	    result.rectype[result.nrectypes].id = recstr[i];
	    result.nrectypes++;
	    if(result.nrectypes >= MAXRECTYPES){
		fprintf(stderr,"reached maximum number of record types: %d\n",
		MAXRECTYPES);
		break;
	    }
	}
    } else {
	if(verbose){
	    fprintf(stderr,"No record types found in header\n");
	}
    }
    for(i=0;i<result.nrectypes;i++){
	if(verbose){
	    fprintf(stderr,"Getting info on record type '%c'\n",
	    result.rectype[i].id);
	}
	sprintf(tmpstr,"Record Info[%c]:",result.rectype[i].id);
	if((fieldstr = GetHeaderParameter(header,tmpstr)) != NULL){
	    /*
	    ** the first arg is the record type and size
	    */
	    GetFieldInfoByNumber(fieldstr,0,&fieldinfo);
	    result.rectype[i].id = fieldinfo.name[0];
	    result.rectype[i].size = fieldinfo.size;
	    /*
	    ** get the field information
	    */
	    result.rectype[i].nfields = GetFieldCount(fieldstr)-1;
		 if(verbose){
			fprintf(stderr,"recid=%c size=%d nfields=%d\n",
				result.rectype[i].id,
				result.rectype[i].size,
				result.rectype[i].nfields);
		 }
	    result.rectype[i].field = 
		(FieldInfo *)calloc(result.rectype[i].nfields,sizeof(FieldInfo));
	    for(j=1;j<result.rectype[j].nfields+1;j++){
		GetFieldInfoByNumber(fieldstr,j,&fieldinfo);
		result.rectype[i].field[j-1] = fieldinfo;
		result.rectype[i].field[j-1].name = 
		    (char *)malloc(sizeof(char)*(strlen(fieldinfo.name)+1));
		strcpy(result.rectype[i].field[j-1].name,
		    fieldinfo.name);
		 if(verbose){
			fprintf(stderr,"\tfield[%d]=%s type=%d size=%d count=%d\n",j,
				result.rectype[i].field[j-1].name,
				result.rectype[i].field[j-1].type,
				result.rectype[i].field[j-1].size,
				result.rectype[i].field[j-1].count);
		 }
	    }
	    if(verbose){
		fprintf(stderr,"got it\n");
	    }
	} else {
	    if(verbose){
		fprintf(stderr,"Info on record type %c not found in header\n",
		result.rectype[i].id);
	    }
	}
    }
    /*
    ** prepare the output file
    */
    BeginStandardHeader(result.fpout,argc,argv,VERSION);
    fprintf(result.fpout,"%% File type: \tBinary\n");
    fprintf(result.fpout,"%% Extraction type: \t%s\n",extracttype);
    fprintf(result.fpout,"%% Probe: \t%d\n",result.probenumber);
    fprintf(result.fpout,"%% Fields: \t");
    recinfo = FindRecordType(&result,result.requested_recid);
    for(i=0;i<result.nsubfields;i++){
	fprintf(result.fpout,"%s,%d,%d,%d\t",
	    result.subfield[i].name,
	    recinfo->field[result.subfield[i].field].type,
	    recinfo->field[result.subfield[i].field].size,
	    recinfo->field[result.subfield[i].field].count);
    }
    fprintf(result.fpout,"\n");
    /*
    ** store the original data file header info
    */
    fprintf(result.fpout,
	"%%\n%% Beginning of header from input file '%s'\n",fnamein);
    DisplayHeader(result.fpout,header,headersize);
    fprintf(result.fpout,
	"%% End of header from input file '%s'\n%%\n",fnamein);
    EndStandardHeader(result.fpout);
    if(verbose){
	fprintf(stderr,"Beginning processing...\n");
    }
    if(verbose){
	fprintf(stderr, "%7s/%7s\t%6s\t%7s\t%10s\t%10s\n",
	"record","tot","recid",
	"recsize","recoffset","octal");
    }
    /*
    ** now with all the record info, perform the requested extraction
    */
    while(!feof(result.fpin)){
	/*
	** read in a record
	*/

	if((recinfo = GetRecord(&result)) == NULL) break;
	totcount++;
	if(verbose){
	    fprintf(stderr, "%7d/%7d\t%c(%3d)\t%7d\t%10d\t%10o\n",
	    reccount,totcount,recinfo->id,(int)recinfo->id,
	    recinfo->size,result.recoffset,result.recoffset);
	}
	/*
	** check the type
	*/
	if(recinfo->id == result.requested_recid){
	    /*
	    ** check the probe number
	    */
	    reccount++;
	    if(recinfo->id == POSREC){
		/*
		** need to handle the position records differently
		** due to the variable record length 
		*/
		ProcessPositionRecord(&result,recinfo);
	    } else {
		if(result.probefield >= 0){
		    probeval  = GetShortField(&result,recinfo,result.probefield);
		    if(verbose){
			fprintf(stderr,"\t%d\n",probeval);
		    }
		    if(probeval  != result.probenumber){
			FreeRecord(&result);
			continue;
		    }
		} else {
		    if(verbose){
			fprintf(stderr,"\n");
		    }
		}
		/*
		** if it matches the requested type then extract the
		** requested fields
		*/
		WriteRequestedFields(&result,recinfo);
	    }
	}
	FreeRecord(&result);
    }
    exit(0);
}
