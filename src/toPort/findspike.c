/*
*******************************************************************
PROGRAM:
    findspike

DESCRIPTION:
    Locates a spike in a spike- or parm-file based on index or
    timestamp and returns the corresponding timestamp or index.

AUTHOR:
    Written by Matthew Wilson
    ARL Division of Neural Systems, Memory, and Aging
    University of Arizona
    Tucson, AZ 85724
    wilson@nsma.arizona.edu

DATES:
    original program 7/92
    
    now accepts any data file with a 'time' or 'timestamp' field,
    e.g. pxyabw or other parm files Tom Davidson (tjd@mit.edu) 12/04

*******************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* MWL Headers */
#include <header.h>
#include <iolib.h>

#define VERSION "1.07"

/*
*********************************************************
**              DEFINES
*********************************************************
*/

/*    Return codes:    */

#define SUCCESS			0
#define ERROR_GENERIC		1 
#define ERROR_FILE		2 /* file read error */
#define ERROR_OUTOFRANGE	3 /* out of range index or timestamp input */
#define ERROR_NOTIMEFIELD	4 /* no 'time' or 'timestamp' field found in input file */


#define INVALIDMODE	-1
#define GETTIMESTAMP	0
#define GETIDX		1
#define GETIDXFROMFILE	2
#define GETSPIKECOUNT	3

/*
*********************************************************
**              STRUCTURES
*********************************************************
*/
typedef struct result_type {
    int		spike_rec_size;
    int		headersize;
    int		ts_offset;
    int		ts_type;
    int		ts_convertsecs;
    int		id_offset;
    int		id_type;
    int		mode;
    FILE	*fpin;
    FILE	*fpout;
    FILE	*fptin;
    int		format;
    int		findeof;
    int		convert;
} Result;

/*
*********************************************************
**              GLOBALS
*********************************************************
*/
int verbose;

/* ReadTS */
/* Read a single time value (float, double, or unsigned long), convert */
/* from seconds to timestamps (if necessary) and return a timestamp */
/* (unsigned long). Returns 1 if read was successful, -1 otherwise.  */

int ReadTS(unsigned long *ptimestamp,
	      FILE *fpin,
	      int ts_type,
	      int ts_offset,
	      int ts_convertsecs)
{
  double dtimestamp = 0; 
  float ftimestamp = 0; 

/* Seek to timestamp */
  if(fseek(fpin,ts_offset,SEEK_CUR) == -1){
    fprintf(stderr,"ERROR: unable to seek to timestamp offset\n");
    exit(ERROR_FILE);
  }

/* Read timestamp */
  switch (ts_type){
  case ULONG:
    if(fread(ptimestamp,sizeof(unsigned long),1,fpin) !=1)
      return(-1);
    return(1);

  case DOUBLE:
    if(fread(&dtimestamp,sizeof(double),1,fpin) != 1)
      return(-1);      
    if (ts_convertsecs)
      dtimestamp *= 10000; /* convert from seconds to timestamps */
    *ptimestamp = (unsigned long)dtimestamp;
    return(1);
    
  case FLOAT:
    if(fread(&ftimestamp,sizeof(float),1,fpin) != 1)
      return(-1);
    if (ts_convertsecs)
      ftimestamp *= 10000; /* convert from seconds to timestamps */
    *ptimestamp = (unsigned long)ftimestamp;
    return(1);

  }
  return (-1);
}

long SpikeSearch(result,startid,endid,starttime,endtime,targettime)
Result	*result;
long	startid;
long	endid;
unsigned long	starttime;
unsigned long	endtime;
unsigned long	targettime;
{
long	centerid;
unsigned long	centertime;
long	targetid;

    if(endid < startid){
	fprintf(stderr,"findspike: invalid spike id range\n");
	exit(ERROR_GENERIC);
    }
    /*
    ** get the spike between the start and end
    */
    centerid = startid + (endid-startid)/2;
    /*
    ** are we finished?
    */
    if(centerid == startid){
      if ( (targettime-starttime) <= (endtime-targettime) ) {
	return(centerid);
      } else {
	return(endid);
      }
    }
    if(fseek(result->fpin,result->headersize+centerid*(result->spike_rec_size),SEEK_SET) == -1){
	fprintf(stderr,"findspike: unable to seek to spikeid '%ld'\n",centerid);
	exit(ERROR_FILE);
    }
    if(ReadTS(&centertime,result->fpin,result->ts_type,result->ts_offset,result->ts_convertsecs) != 1){
	fprintf(stderr,"findspike: unable to read timestamp for spike '%ld'\n",centerid);
	exit(ERROR_GENERIC);
    }
    if(result->convert){
	ConvertData(&centertime,sizeof(unsigned long));
    }
    /*
    ** see which half the target falls in
    */
    if((targettime >= starttime) && (targettime < centertime)){
	if(centertime == 0){
	    fprintf(stderr,"findspike: center timestamp %ld out of search range (%ld - %ld)\n",
	    centertime,starttime,endtime);
	    exit(ERROR_GENERIC);
	}
	/*
	** search lower half
	*/
	targetid = SpikeSearch(result,startid,centerid,starttime,
	centertime,targettime);
    } else 
    if((targettime <= endtime) && (targettime >= centertime)){
	if(endtime == 0){
	    fprintf(stderr,"findspike: end timestamp %ld out of search range (%ld - %ld)\n",
	    centertime,starttime,endtime);
	    exit(ERROR_GENERIC);
	}
	/*
	** search upper half
	*/
	targetid = SpikeSearch(result,centerid,endid,centertime,
	endtime,targettime);
    } else {
	fprintf(stderr,"findspike: spike timestamp %ld out of search range (%ld - %ld)\n",
	targettime,starttime,endtime);
	exit(ERROR_OUTOFRANGE);
    }
    return(targetid);
}

long FindSpikeByTimestamp(result,timestamp)
Result	*result;
unsigned long timestamp;
{

long index;
unsigned long starttime;
unsigned long endtime;
long 	startid;
long 	endid;
long	filesize;

    /*
    ** get the timestamps of the beginning and end of the spike file
    */

    /*
    ** first spike
    */
 
    if(fseek(result->fpin,result->headersize,SEEK_SET) == -1){
	fprintf(stderr,"ERROR: unable to seek to beginning of spike file\n");
	exit(ERROR_FILE);
      }
    startid = 0;

    if(ReadTS(&starttime,result->fpin,result->ts_type,result->ts_offset,result->ts_convertsecs) != 1){
	fprintf(stderr,"ERROR: unable to read first timestamp\n");
	exit(ERROR_GENERIC);
    }
    if(result->convert){
	ConvertData(&starttime,sizeof(unsigned long));
    }
    /*
    ** last spike
    */
    if(fseek(result->fpin,0L,SEEK_END) == -1){
	fprintf(stderr,"ERROR: unable to seek to end of spike file\n");
	exit(ERROR_FILE);
    }

    filesize = ftell(result->fpin);
    endid = (filesize - result->headersize)/(result->spike_rec_size) -1;
    /*
    ** handle the special end of spike file request
    */
    if(result->findeof){
	return(endid);
    }
    if(fseek(result->fpin,filesize-result->spike_rec_size,SEEK_SET) == -1){
	fprintf(stderr,"ERROR: unable to seek to last spike\n");
	exit(ERROR_FILE);
    }
    if(ReadTS(&endtime,result->fpin,result->ts_type,result->ts_offset,result->ts_convertsecs) != 1){
	fprintf(stderr,"ERROR: unable to read last timestamp\n");
	exit(ERROR_GENERIC);
    }
    if(result->convert){
	ConvertData(&endtime,sizeof(unsigned long));
    }

    /*
    ** use a binary search to locate the spike
    */
    index = SpikeSearch(result,startid,endid,starttime,endtime,timestamp);
    return(index);
}

unsigned long FindSpikeByIndex(result,index)
Result	*result;
long	index;
{
unsigned long	timestamp;
int	nspikes;
    if(index < 0){
	fprintf(stderr,"findspike: ERROR: invalid index '%ld'\n",index);
	exit(ERROR_OUTOFRANGE);
    }
    /*
    ** handle the special end of spike file request
    */
    if(result->findeof){
	/*
	** last spike
	*/
	if(fseek(result->fpin,0,SEEK_END) == -1){
	    fprintf(stderr,"ERROR: unable to seek to end of spike file\n");
	    exit(ERROR_FILE);
	}
	index = (ftell(result->fpin) - result->headersize)/
	(result->spike_rec_size) -1;
    }
    /*
    ** then seek to the index
    */

    /* test whether index is out of bounds */
    fseek(result->fpin,0L,SEEK_END); 
    nspikes = (ftell(result->fpin) - result->headersize)/result->spike_rec_size;    
    if (index > nspikes - 1){
      fprintf(stderr,"findspike: invalid index. unable to seek to id %ld.\n",index);
      exit(ERROR_OUTOFRANGE);
    }

    /* set file pointer to correct index before calling ReadTS */
    fseek(result->fpin,result->headersize + (index * (result->spike_rec_size)),SEEK_SET);

    if(ReadTS(&timestamp,result->fpin,result->ts_type,result->ts_offset,result->ts_convertsecs) != 1){    
	fprintf(stderr,"findspike: read error at %ld\n",index);
	exit(ERROR_FILE);
    }
    if(result->convert){
	ConvertData(&timestamp,sizeof(unsigned long));
    }
    return(timestamp);
}

long GetIdByIndex(Result *result,long index)
{  
  float findex;
  int iindex;
  
  if (verbose)
          fprintf(stderr,"findspike: Getting index field for record index %ld (GetIdByIndex)\n",index);

  fseek(result->fpin,
	result->headersize + index*(result->spike_rec_size) + result->id_offset,
	SEEK_SET);

/* Read index */
  switch (result->id_type){

  case FLOAT:
    if(fread(&findex,sizeof(float),1,result->fpin) != 1){
      fprintf(stderr,"findspike: ERROR: reading index (float) %ld (GetIdByIndex)\n",index);      
      exit(ERROR_FILE);
    }
    return((long)findex);

  case INT:
    if(fread(&iindex,sizeof(int),1,result->fpin) != 1){
      fprintf(stderr,"findspike: ERROR: reading index (int) at index %ld (GetIdByIndex)\n",index);
      exit(ERROR_FILE);
    }
    return((long)iindex);
  }
  fprintf(stderr,"findspike: ERROR: only int and float type index fields supported");
  exit (ERROR_GENERIC);
}


int main(argc,argv)
int	argc;
char 	**argv;
{
 int	nxtarg;
 unsigned long timestamp;
 long	index = 0;
 char	**header;
 Result	result;
 FieldInfo	fieldinfo;
 char	*fieldstr;
 int	ts_column;
 int	ts_size;
 int	id_field = 0;
 int	id_column  = -1;
 int	i;
 int	fileopen = 0;
 long	fsize;
 long	nrecords;
 int argcheck = 0;
 int recid = 0;
 int minusOne = 0;

    verbose = 0;
    nxtarg = 0;
    result.fpout = stdout;
    result.fpin = NULL;
    result.fptin = NULL;
    result.mode = INVALIDMODE;
    result.format = 0;
    result.findeof = 0;
    result.convert = 0;
    result.ts_offset = 0;
    result.spike_rec_size = 0;

    while(++nxtarg < argc){
      /* -usage option*/
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,
	    "usage: %s spikefile [-o file][-v][-i index][-t timestamp][-format]\n",
	    argv[0]);
	    fprintf(stderr,"\t[-tf timestampfile]\n");
	    fprintf(stderr,"\t[-countspikes] count the spikes, dummy.\n");
	    fprintf(stderr,"\t[-recid] return record id, even if there is a spike id field.\n");
	    exit(SUCCESS);
	} else /* -version option */
	if(strcmp(argv[nxtarg],"-version") == 0){
	    fprintf(stderr,"%s : version %s : updated %s\n",
		argv[0],VERSION,DATE);

/* passed in from makefile when building an RPM*/
#ifdef MWSOFT_RPM_STRING 
	    fprintf(stderr,"From RPM: %s\n",
		    MWSOFT_RPM_STRING);
#endif

	    fprintf(stderr,"%s\n",COPYRIGHT);
	    exit(SUCCESS);
	} else /* -v option: verbose */
	if(strcmp(argv[nxtarg],"-v") == 0){
	    verbose = 1;
	} else /* -recid option: return record id */
	if(strcmp(argv[nxtarg],"-recid") == 0){
	    recid = 1;
	} else /* -format option */
	if(strcmp(argv[nxtarg],"-format") == 0){
	    result.format = 1;
	} else /* -countspikes option: count number of spikes in file */
	if(strcmp(argv[nxtarg],"-countspikes") == 0){
	    result.mode = GETSPIKECOUNT;
	} else /* -tf option: timestamp file */ 
	if(strcmp(argv[nxtarg],"-tf") == 0){
	  argcheck = checkArgs(&argv[nxtarg], nxtarg, argc, ARGFILE, 1);
	  if (argcheck == ARGERROR_NUMARG) {
	    fprintf(stderr,"findspike: ERROR: -tf option: expecting file\n");
	    exit(ERROR_FILE);
	  } else if (argcheck == ARGERROR_INVALIDFILE) {
	    fprintf(stderr,"findspike: ERROR: -tf option: No such file\n");
	    exit(ERROR_FILE);
	  }
	  if((result.fptin = fopen(argv[++nxtarg],"r")) == NULL){
	    fprintf(stderr,"findspike: ERROR: unable to read timestamp file '%s'\n",
		    argv[nxtarg]);
	    exit(ERROR_FILE);
	  }
	  result.mode = GETIDXFROMFILE;
	} else /* -t option: return spike index, based on time value */
	if(strcmp(argv[nxtarg],"-t") == 0){
	    if(strcmp(argv[nxtarg+1],"end") == 0){
		nxtarg++;
		result.findeof = 1;
		timestamp = 0;
	    } else {
	      argcheck = checkArgs(&argv[nxtarg], nxtarg, argc, ARGNUMBER, 1);
	      if (argcheck == ARGERROR_NUMARG) {
		fprintf(stderr,"findspike: ERROR: -t option: expecting time value\n");
		exit(ERROR_GENERIC);
	      } else if (argcheck != ARGERROR_PARTIALNUMBER && argcheck != ARGERROR_NONE) {
		fprintf(stderr,"findspike: ERROR: -t option: invalid time value\n");
		exit(ERROR_GENERIC);
	      }
	      /* Test for special case: if argument is -1, we'll output -1 later */
	      if (strcmp(argv[++nxtarg],"-1") == 0)
		minusOne = 1;
	      else
		timestamp = ParseTimestamp(argv[nxtarg]);
	    }
	    result.mode = GETIDX;
	} else /* -i option: return timestamp based on index */
	  if(strcmp(argv[nxtarg],"-i") == 0){
	    argcheck = checkArgs(&argv[nxtarg], nxtarg, argc, ARGNUMBER, 1);
	    if (argcheck == ARGERROR_NUMARG) {
	      fprintf(stderr,"findspike: ERROR: -i option: expecting index\n");
	      exit(ERROR_GENERIC);
	    } else if (argcheck == ARGERROR_ARGTYPE) {
	      if(strcmp(argv[nxtarg+1],"end") == 0){
		nxtarg++;
		result.findeof = 1;
		index = 0;
	      } else {
		fprintf(stderr,"findspike: ERROR: -i option: invalid index\n");
		exit(ERROR_GENERIC);
	      }
	    } else if (argcheck != ARGERROR_NONE) {
	      fprintf(stderr,"findspike: ERROR: -i option: invalid index\n");
	      exit(ERROR_GENERIC);
	    } else {
	      /* Test for special case: if argument is -1, output -1 */
	      if (strcmp(argv[++nxtarg],"-1") == 0)
		minusOne = 1;
	      else
		index = atoi(argv[nxtarg]);
	    }
	    result.mode = GETTIMESTAMP;
	  } else /* -o option: output file */
	if(strcmp(argv[nxtarg],"-o") == 0){
	  argcheck = checkArgs(&argv[nxtarg], nxtarg, argc, ARGFILENAME, 1);
	  if (argcheck == ARGERROR_NUMARG) {
	    fprintf(stderr,"findspike: ERROR: -o option: expecting file name\n");
	    exit(ERROR_FILE);
	  } else if (argcheck == ARGERROR_INVALIDNAME) {
	    fprintf(stderr,"findspike: ERROR: -o option: invalid file name\n");
	    exit(ERROR_FILE);
	  }
	  if((result.fpout = fopen(argv[++nxtarg],"w")) == NULL){
	    fprintf(stderr,"findspike: ERROR: unable to create file '%s'\n",argv[nxtarg]);
	    exit(ERROR_FILE);
	  }
	} else /* input file */
	if(argv[nxtarg][0] != '-'){
	  if (fileopen) {
		fprintf(stderr,"findspike: ERROR: findspike handles only one file at a time!\n");
		fclose(result.fpin);
		exit(ERROR_GENERIC);	    
	  } else {
	    if((result.fpin = fopen(argv[nxtarg],"r")) == NULL){
		fprintf(stderr,"findspike: ERROR: unable to read file '%s'\n",argv[nxtarg]);
		exit(ERROR_FILE);
	    }
	    fileopen = 1;
	  }
	} else {
	    fprintf(stderr,"findspike: invalid option '%s'\n",argv[nxtarg]);
	    exit(ERROR_GENERIC);
	}
    }

    if(result.fpin == NULL){
	fprintf(stderr,"findspike: ERROR: a spike or parm file must be specified\n");
	exit(ERROR_FILE);
    }

    if (result.mode == INVALIDMODE) {
      fprintf(stderr,"findspike: ERROR: incorrect mode, use either -i, -t, -tf, or -countspikes\n");
      exit(ERROR_GENERIC);
    }

    /* Test for special case: if argument is -1, output -1 */
    if (minusOne){
      fprintf(result.fpout,"-1\n");      
      exit(SUCCESS);
    }

    header = ReadHeader(result.fpin,&result.headersize);

    if (GetFileType(header) != BINARY){
	fprintf(stderr,"findspike: ERROR: only binary file types supported\n");
	exit(ERROR_GENERIC);
    }

    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	result.convert = 0;
	/*
	fprintf(stderr,"findspike: No data conversion file from %s architecture.\n"
,
		GetFileArchitectureStr(header));
		*/
    } else {
	result.convert = 1;
	fprintf(stderr,"findspike: Converting data from %s to %s architectures.\n",
	    GetFileArchitectureStr(header),
	    GetLocalArchitectureStr());
    }

    /* Get field string from header */

    fieldstr = GetFieldString(header); 
        
    /* look for timestamp field, or, if not available, for time field */

    if (GetFieldInfoByName(fieldstr,"timestamp",&fieldinfo)){
      ts_column = fieldinfo.column;
      result.ts_convertsecs = 0;
    } else
      if (GetFieldInfoByName(fieldstr,"time",&fieldinfo)){
	ts_column = fieldinfo.column;
	result.ts_convertsecs = 1;
      } else {
	fprintf(stderr,"findspike: ERROR: No time or timestamp field in input file\n");
	exit(ERROR_NOTIMEFIELD); 
      }
    result.ts_type = fieldinfo.type;
    ts_size = fieldinfo.size;


    /* get id field column, too, if that exists */
    
    if (GetFieldInfoByName(fieldstr,"id",&fieldinfo)){
      id_field = 1;
      id_column = fieldinfo.column;
      result.id_type = fieldinfo.type;
    }

    
    /* verify size of timestamp field */

    switch (result.ts_type){
    case DOUBLE:
      if (sizeof(double) != ts_size){
	fprintf(stderr,"findspike: ERROR: data type/field size mismatch\n");
	exit(ERROR_GENERIC);
      }
      break;
    case FLOAT:
      if (sizeof(float) != ts_size){
	fprintf(stderr,"findspike: ERROR: data type/field size mismatch\n");
	exit(ERROR_GENERIC);
      }
      break;
    case ULONG:
      if (sizeof(unsigned long) != ts_size){
	fprintf(stderr,"findspike: ERROR: data type/field size mismatch\n");
	exit(ERROR_GENERIC);
      }
      break;
    }

    
    /* process all fields (including repeating fields) to get record
       size in bytes, and byte offset of time/timestamp and id fields */
    
    for (i = 0; 
	 GetFieldInfoByNumber(fieldstr,i,&fieldinfo); 
	 i++,result.spike_rec_size += fieldinfo.size * fieldinfo.count){

      if (i == ts_column)
	result.ts_offset = result.spike_rec_size; 
      if (i == id_column)
	result.id_offset = result.spike_rec_size;
    }
    
    /*
    ** get the spike time/index information
    */
    if(result.mode == GETIDXFROMFILE){
	while(!feof(result.fptin)){
	    if(fread(&timestamp,sizeof(unsigned long),1,result.fptin) != 1){
		break;
	    }
	    if(result.convert){
		ConvertData(&timestamp,sizeof(unsigned long));
	    }
	    index = FindSpikeByTimestamp(&result,timestamp);
	    if (id_field && !recid)
	      index = GetIdByIndex(&result,index);
	    fprintf(result.fpout,"%ld\n",index);   
	}
    } else
    if(result.mode == GETIDX){
	if(verbose){
	    fprintf(stderr,"findspike: locating spike at timestamp %s\n",
	    TimestampToString(timestamp));
	}
	index = FindSpikeByTimestamp(&result,timestamp);
	if (id_field && !recid)
	  index = GetIdByIndex(&result,index);
	fprintf(result.fpout,"%ld\n",index);
	
    } else
    if(result.mode == GETTIMESTAMP){
	if(verbose){
	    fprintf(stderr,"findspike: locating spike at index %ld\n",index);
	}
	timestamp  = FindSpikeByIndex(&result,index);
	if(result.format){
	    fprintf(result.fpout,"%s\n",TimestampToString(timestamp));
	} else {
	    fprintf(result.fpout,"%lu\n",timestamp);
	}
    } else
    if(result.mode == GETSPIKECOUNT){
	fseek(result.fpin,0L,SEEK_END);
	fsize = ftell(result.fpin);
	nrecords = (fsize - result.headersize)/result.spike_rec_size;
	fprintf(result.fpout,"%ld\n",nrecords);
    }
    exit(SUCCESS);
}

