#include "xclust_ext.h"
#define BLOCKSIZE 1000
#define OLDBINARY 100

typedef struct field_range_type {
    char	*name;
    int		index;
    int		hasstart;
    double	start;
    int		hasend;
    double	end;
} FieldRange;

static blocksize = BLOCKSIZE;
char *restorefile = NULL;
static FieldRange rangedata;

CheckFieldRange(graph,rangedata)
Graph		*graph;
FieldRange	*rangedata;
{
char	*sptr;
ProjectionInfo	*pinfo;

    /*
    ** check for range information
    */
    rangedata->hasstart = 0;
    rangedata->hasend = 0;
    if(sptr = GetItemValue("/epochmenu/rangefield")){
	/*
	** any field specified?
	*/
	if(strlen(sptr) > 0){
	    fprintf(stderr,"Field range limiting active\n");
	    /*
	    ** look up the projection by name
	    */
	    if((pinfo = GetProjectionInfoByName(graph,sptr)) == NULL){
		/*
		** if that fails then look  up by number
		*/
		pinfo = GetProjectionInfo(graph,Atoi(sptr));
	    }
	    if(pinfo){
		rangedata->index = pinfo->projectionid;
		fprintf(stderr,"\tField: '%s' (%d)\n",sptr,rangedata->index);
		/*
		** check the ranges
		*/
		if(sptr = GetItemValue("/epochmenu/startrange")){
		    if(strlen(sptr) > 0){
			rangedata->hasstart = 1;
			/*
			** look for special timestamp format
			*/
			if(strchr(sptr,':')){
			    rangedata->start = ParseTimestamp(sptr)/1e4;
			} else {
			    rangedata->start = atof(sptr);
			}
			fprintf(stderr,"\tStart: %g\n",rangedata->start);
		    }
		}
		if(sptr = GetItemValue("/epochmenu/endrange")){
		    if(strlen(sptr) > 0){
			rangedata->hasend = 1;
			if(strchr(sptr,':')){
			    rangedata->end = ParseTimestamp(sptr)/1e4;
			} else {
			    rangedata->end = atof(sptr);
			}
			fprintf(stderr,"\tEnd: %g\n",rangedata->end);
		    }
		}
	    } else {
		fprintf(stderr,
		"Could not find field '%s' for range limiting\n",sptr);
	    }
	}
    }
}

SetRestoreFile(name)
char *name;
{
    restorefile = name;
}

DataSource *AddSource(graph)
Graph *graph;
{
DataSource	*source;

    /*
    ** allocate it
    */
    if((source = (DataSource *)calloc(1,sizeof(DataSource))) == NULL){
	fprintf(stderr,"MEMORY ERROR: unable to allocate data source\n");
	return(NULL);
    }
    /*
    ** insert it into the global list
    */
    source->next = graph->source;
    graph->source = source;
    source->parent = (BasicWindow *)graph;
    InitializeSourceDefaults(graph,source);
    return(source);
}

Plot *ClosePlot(plot)
Plot	*plot;
{
    if(plot == NULL) return(NULL);
    if(plot->npoints > 0){
	/*
	** assign the source defaults to the plot
	UseSourceDefaults(plot->source,plot);
	*/

	/*
	** allocate space for the screen coordinates
	*/
	if(plot->coord == NULL){
	    if((plot->coord = (Coord *)calloc(plot->npoints,sizeof(Coord))) ==
	    NULL){
		fprintf(stderr,"MEMORY ERROR: unable to allocate plot coordinates.\n");
		fprintf(stderr,"\tToo many points (%d). Try a partial load.\n",
		plot->npoints);
		return(NULL);
	    }
	}
	if(plot->fcoord == NULL){
	    if((plot->fcoord = (FCoord *)calloc(plot->npoints,sizeof(FCoord)))
	    == NULL){

		fprintf(stderr,"MEMORY ERROR: unable to allocate plot coordinates.\n");
		fprintf(stderr,"\tToo many points (%d). Try a partial load.\n",
		plot->npoints);
		return(NULL);
	    }
	}
	if(plot->xhi < 0){
	    plot->xhi = plot->npoints-1;
	}
    } else {
#ifdef OLD
	DeletePlot(plot);
#endif
	return(NULL);
    }
    return(plot);
}

Plot *AddPlot(graph,source)
Graph 		*graph;
DataSource	*source;
{
Plot	*plot;

    /*
    ** allocate it
    */
    if((plot = (Plot *)calloc(1,sizeof(Plot))) == NULL){
	fprintf(stderr,"MEMORY ERROR: unable to allocate plot\n");
	return(NULL);
    }
    /*
    ** insert it into the global list
    */
    plot->next = graph->plot;
    graph->plot = plot;
    plot->graph = graph;
    plot->visible = 1;
    plot->linestyle = -1;	/* indicate that no linestyle was specified */
    plot->scale = 1;		
    plot->xscale = 1;		
    plot->zscale = 1;		
    plot->escale = 1;		
    plot->cscale = 1;		
    if((plot->rawdata = (DataCoord *)calloc(blocksize,sizeof(DataCoord))) == NULL){
	fprintf(stderr,"MEMORY ERROR: unable to allocate coordinate block\n");
	return(NULL);
    }
    plot->arraysize = blocksize;
    plot->npoints = 0;
    plot->xhi = -1;	
    plot->xlo = 0;	
    plot->refreshtime = 0;		
    plot->source = source;
    /*
    UseGraphDefaults(plot->graph,plot);
    ** assign the source defaults to the plot
    */
    UseSourceDefaults(plot->source,plot);
    plot->ndatafields = source->nsourcefields;
    return(plot);
}

DeleteSource(graph,source)
Graph *graph;
DataSource *source;
{
DataSource *s;

    if(graph->source == source){
	graph->source = source->next;
	free(source);
	return;
    }
    for(s=graph->source;s;s=s->next){
	if(s->next == source){
	    s->next = source->next;
	    free(source);
	    return;
	}
    }
}

int AddPoint(plot,line,datavector,ndatafields)
Plot *plot;
char *line;
char	*datavector;
int	ndatafields;
{
float	x,y,err;
int	nitems;
int	have_error;
char	*ptr;
char	*strchr();
double	vector[100];
int	i;
int	shift;
double	dval;
ProjectionInfo	*pinfo;
char	namestr[80];

    shift = 0;
    /*
    ** is the data coming from the line or the data vector
    */
    if(datavector){
	/*
	** from the data vector
	*/
	nitems = ndatafields;
    } else 
	/*
	** from the line
	*/
    if(line != NULL){
	/*
	** deal with comments
	*/
	if(ptr = strchr(line,'%')){
	    *ptr = '\0';
	}
	/*
	** get the coordinates
	*/
	nitems = sgetargs(line,100,vector);
	if(nitems < 1){
	    return(0);
	}
    } else {
	return(0);
    }
    /*
    ** deal with the case of no header info and fill in the missing info
    ** based upon what was found
    */
    if((plot->graph->source->nsourcefields == 0) && (nitems > 0)){
	plot->graph->source->nsourcefields = nitems;
	for(i=0;i<nitems;i++){
	    sprintf(namestr,"P%d",i);
	    AddProjectionInfo(plot->graph,i,namestr,FLOAT,sizeof(float));
	}
	plot->graph->source->recordsize = nitems*sizeof(float);
    }
    /*
    ** fill the data vector
    */
    if(plot->ndatafields != 0 && plot->ndatafields != nitems && suppresswarnings == 0){
	fprintf(stderr,"warning: mismatch in data vector size (%d fields) at point %d\n",
	nitems,plot->npoints);
    } else {
    /*
	plot->ndatafields = nitems;
	*/
    }
#ifdef OLD
    /*
    ** check for some special cases of implied x coordinates
    */
    if((nitems == 1) && (plot->auto_x == 0)){
	plot->auto_x = 1;
	/*
	** check to see if the projections have been established
	*/
	if(plot->graph->nprojections < 2){
	    /*
	    ** get the current projection 0 info
	    */
	    if(pinfo = GetProjectionInfo(plot->graph,0)){
		/*
		** and shift it over
		*/
		AddProjectionInfo(plot->graph,1,pinfo->name,pinfo->type,pinfo->size);
	    }
	    /*
	    ** add the autox projection
	    */
	    AddProjectionInfo(plot->graph,0,"autox",FLOAT,sizeof(float));
	} 
    }
    if(plot->auto_x == 1){
	shift = 1;
    } else {
	shift = 0;
    }
#endif
    /*
    ** if range restrictions are in force then check them
    */
    if(rangedata.hasstart){
	if((pinfo=GetProjectionInfo(plot->graph,rangedata.index)) == NULL){
	    fprintf(stderr,"ERROR: unable to find projection %d for range\n",rangedata.index);
	}
	dval = GetDoubleFromRawData(datavector,pinfo);
	if(datavector){
	    if(dval  < rangedata.start){
		return(0);
	    } 
	} else
	if(vector[rangedata.index] < rangedata.start){
	    return(0);
	}
    }
    if(rangedata.hasend){
	if((pinfo=GetProjectionInfo(plot->graph,rangedata.index)) == NULL){
	    fprintf(stderr,"ERROR: unable to find projection %d for range\n",rangedata.index);
	}
	dval = GetDoubleFromRawData(datavector,pinfo);
	if(datavector){
	    if(dval > rangedata.end){
		return(0);
	    }
	} else
	if(vector[rangedata.index] > rangedata.end){
	    return(0);
	}
    }
    /*
    ** allocate the data vector
    */
    if((plot->rawdata[plot->npoints].data = 
	(char *)calloc(1,plot->source->recordsize + shift*sizeof(float))) == NULL){
	fprintf(stderr,"MEMORY ERROR: unable to allocate data vector of size %d\n", plot->source->recordsize);
	return(-1);
    }
    /*
    ** assign the implied x coord if defined
    if(plot->auto_x == 1){
	*((float *)(plot->rawdata[plot->npoints].data)) = plot->auto_xstart +
		plot->npoints*plot->auto_x;
    }
    */
    plot->rawdata[plot->npoints].datasize = plot->source->recordsize +shift*sizeof(float);
    if(datavector){
	    bcopy(datavector,
	    plot->rawdata[plot->npoints].data+shift*sizeof(float),plot->source->recordsize);
    } else {
	/*
	** copy the parsed data into the coordinate vector
	*/
	for(i=0;i<nitems;i++){

	    if((pinfo=GetProjectionInfo(plot->graph,i+shift)) == NULL){
		fprintf(stderr,"ERROR: unable to find projection %d for range\n",rangedata.index);
		break;
	    }
	    AssignRawDataFromDouble(plot->rawdata[plot->npoints].data,pinfo,vector[i]);
	}
    }
    /*
    ** assign the point to the default cluster
    */
    plot->rawdata[plot->npoints].clusterid = plot->graph->defaultcluster;

    /*
    ** assume that more than 3 coordinate values implies error
    ** bars are to be displayed
    if(nitems + shift > 2){
	plot->zmode = ERRORBAR;
    }
    */
    /*
    ** keep track of the min max values
    */
    if((pinfo=GetProjectionInfo(plot->graph, plot->graph->currentprojection[0])) != NULL){
	x = GetDoubleFromRawData(plot->rawdata[plot->npoints].data,pinfo);
    }
    if((pinfo=GetProjectionInfo(plot->graph, plot->graph->currentprojection[1])) != NULL){
	y = GetDoubleFromRawData(plot->rawdata[plot->npoints].data,pinfo);
    }
    if(plot->npoints < 1){
	plot->xmin = plot->xmax = x;
	plot->ymin = plot->ymax = y;
    } else {
	if(x < plot->xmin) plot->xmin = x;
	if(x > plot->xmax) plot->xmax = x;
	if(y < plot->ymin) plot->ymin = y;
	if(y > plot->ymax) plot->ymax = y;
    }
    /*
    ** increment the points counter
    */
    plot->xhi = plot->npoints;
    ++(plot->npoints);
    /*
    ** check the array size and allocate more space if
    ** necessary
    */
    if(plot->npoints%blocksize == 0){
	plot->arraysize += blocksize;
	if(plot->rawdata){
	    if((plot->rawdata = (DataCoord *)
	    realloc(plot->rawdata,plot->arraysize*sizeof(DataCoord))) == NULL){
		fprintf(stderr,
		"MEMORY ERROR: unable to reallocate data vector while adding point %d\n",plot->npoints);
		return(-1);
	    }
	}
    }
    return(1);
}

LoadGraphData(graph)
Graph	*graph;
{
DataSource	*source;

    /*
    ** set the default blocksize
    */
    blocksize = BLOCKSIZE;
    /*
    ** if restore file is set then load that 
    */
    /*
    ** go through the data sources
    */
    for(source=graph->source;source;source=source->next){
	LoadPlotFromSource(graph,source);
    }
    /*
    ** initialize cluster assignments
    */
    TouchClusters(graph);
    AssignClusterPoints(graph);
}

int SavePlotOffset(source,index,byteoffset,lineoffset)
DataSource	*source;
int		index;
long		byteoffset;
int		lineoffset;
{
PlotOffset	*plotoffset;
PlotOffset	*newpoffset;

    /*
    ** add the file position information for the specified plot
    ** to the source plot offset list if not already present
    */
    for(plotoffset=source->plotoffset;plotoffset;plotoffset=plotoffset->next){
	/*
	** found it
	*/
	if(plotoffset->index == index){
	    /*
	    ** check for consistency
	    */
	    if(plotoffset->byteoffset != byteoffset){
		fprintf(stderr,
		"ERROR: inconsistency in byte offsets (%d:%d) for plot %d\n",
		plotoffset->byteoffset,byteoffset,index);
	    } 
	    if(plotoffset->lineoffset != lineoffset){
		fprintf(stderr,
		"ERROR: inconsistency in line offsets (%d:%d) for plot %d\n",
		plotoffset->lineoffset,lineoffset,index);
	    }
	    /*
	    ** just go back
	    */
	    return(1);
	}
    }
    /*
    ** otherwise add it to the list
    */
    newpoffset = (PlotOffset *)calloc(1,sizeof(PlotOffset));
    newpoffset->next = source->plotoffset;
    newpoffset->index = index;
    newpoffset->byteoffset = byteoffset;
    newpoffset->lineoffset = lineoffset;
    source->plotoffset = newpoffset;
    return(1);
}

PlotOffset *FindOffset(source,index)
DataSource	*source;
int		index;
{
PlotOffset	*poffset;

    for(poffset=source->plotoffset;poffset;poffset=poffset->next){
	if(poffset->index == index){
	    return(poffset);
	}
    }
    return(NULL);
}

FILE *OpenSource(source)
     DataSource *source;
{
  FILE *fp;

  /* is the file closed? */
  if(source->fp == NULL) {
    
    /* use stdin only if in FULL_LOAD mode */
    if(strcmp(source->filename,"STDIN") == 0){

      if(source->loadmode != FULL_LOAD){
	printf("can't open 'STDIN' in partial load mode \n");
	return(NULL);
      } else {
	fp = stdin;
      }

    } else {
      if((fp = fopen(source->filename,"r")) == NULL){
	printf("can't open file '%s'\n",source->filename);
	return(NULL);
      }
    }
    
  } else {
    fp = source->fp;
  }

  return fp;
}




Plot *LoadPlotFromSource(graph,source)
Graph	*graph;
DataSource	*source;
{
  /* local variables */
static char timestampstr[50];
Plot	*plot;
Plot	*oldplot;
/* FILE	*fp; */
char	line[1001];
char	lastline[1001];
int	headersize;
char	**header;
char	*filetype;
int	*vectorformat;
char	vector[1000];
double  dvector[100]; 
unsigned char diodeval[4];
short diodeval2[4];
char	*parmnames;
char	*sptr;
int	fieldcount;
long	bytefileoffset;
int	linefileoffset;
int	xsize,ysize;
PlotOffset	*poffset;
unsigned long timestamp;
char		str[30];
int	i,j;
FieldInfo	fieldinfo;
int	status;
int	got_header;
short	type;
short	size;
char	channelstr[80];
char	*nchannelstr;
char	*ratestr;
double	time;
int	startline_temp;

 int	id_column = 0;
 int	id_type = INVALID_TYPE;
 int	id_offset = 0;
 float	flastid = 0;
 int	ilastid = 0;
long	lastrec = 0;
long	lastid = 0;
 int	firstpass = 0; /* is this our first time through the source file*/
 char *tmpstr;


    oldplot = NULL;
    plot = NULL;
    got_header = 0;

  /*only load from file */
  if(source->type == FROM_FILE) {

    /* open the source file */
    if ((source->fp = OpenSource(source)) == NULL)
      return(NULL);

    /* is it the beginning of the file? */
    if(source->currentline == 0){ 

      firstpass = 1;
      
      /* read the file header */
      header = ReadHeader(source->fp,&headersize);
      
      /* compare architectures */
	   
      if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	 (GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	source->convert = 0;

      } else {
	source->convert = 1;
	fprintf(stderr,"Data conversion no longer supported");
	exit(0);
      }
      
      got_header = 1;
      source->headersize = headersize;

      /* track the position of the first plot */
      SavePlotOffset(source,source->currentplot,ftell(source->fp),source->currentline);


      /* get fileformat */
      /* can be ASCII, BINARY, OLD_BINARY, INVALID or EEG */

/*       if((filetype = GetHeaderParameter(header,"Extraction type:")) != NULL) { */

/* 	if(strncmp(filetype,"continuous data", strlen("continuous data")) == 0){ */
/* 	  fprintf(stderr,"file type = '%s'\n",filetype); */
/* 	  source->fileformat = EEG; */
/* 	} else { */
/* 	  source->fileformat = GetFileType(header); */
/* 	} */

/*       } else { */
	source->fileformat = GetFileType(header);
/*       } */


/*       if(source->fileformat == EEG){ */
/* 	if(SetProjectionName(graph,0,"timestamp") == 0){ */
/* 	  AddProjectionInfo(graph,0,"timestamp",ULONG,sizeof(unsigned long)); */
/* 	} */
/* 	/\* */
/* 	** determine the number of channels */
/* 	*\/ */
/* 	if((nchannelstr = GetHeaderParameter(header,"nchannels:")) != NULL){ */
/* 	  source->nchannels = atoi(nchannelstr); */
/* 	  fprintf(stderr,"%d eeg channels\n",source->nchannels); */
/* 	  for(i=0;i<source->nchannels;i++){ */
/* 	    sprintf(channelstr,"CH %d",i); */
/* 	    if(SetProjectionName(graph,i+1,channelstr) == 0){ */
/* 	      AddProjectionInfo(graph,i+1,channelstr,SHORT,sizeof(short)); */
/* 	    } */
/* 	  } */
/* 	  source->nsourcefields = source->nchannels +1; */
/* 	  source->recordsize = sizeof(unsigned long) + source->nchannels*sizeof(short); */
/* 	} */
/*       } */

/*       if(source->fileformat == EEG){ */
/* 	/\* */
/* 	** determine the data block size */
/* 	*\/ */
/* 	GetFieldInfoByNumber(parmnames,1,&fieldinfo); */
/* 	source->datablocksize = fieldinfo.count; */
/* 	fprintf(stderr,"data blocksize = %d\n",source->datablocksize); */
/* 	/\* */
/* 	** determine the sampling rate */
/* 	*\/ */
/* 	if((ratestr = GetHeaderParameter(header,"rate:")) != NULL){ */
/* 	  source->eegdt = (10000*source->nchannels)/atof(ratestr); */
/* 	  fprintf(stderr,"dt per channel = %g\n",source->eegdt); */
/* 	} else { */
/* 	  fprintf(stderr,"ERROR: cannot determine sampling rate from header\n"); */
/* 	  source->eegdt = 1; */
/* 	} */
/*       } */

      /* get field info */
      parmnames = GetFieldString(header);
      fieldcount = GetFieldCount(parmnames);
      if(debug) fprintf(stderr,"count = %d ; fieldstr = '%s'\n",fieldcount,parmnames);

      /*
      ** if the file format is empty assume ASCII
      */
      if(source->fileformat == INVALID_TYPE){
	fprintf(stderr,"No filetype found in header.  Assuming ASCII.\n");
	source->fileformat = ASCII;
      }

      if(fieldcount == 0){
	if(source->fileformat == ASCII){
	  fprintf(stderr,"No Fields found in header.  Assuming default floating point fields.\n");
	} else {
	  fprintf(stderr,"No Fields found in header. Unable to read datafile.\n");
	  exit(-1);
	}
      }

      if((source->fileformat == ASCII) || (source->fileformat == BINARY)){

	/*
	** check to see if projections have already been allocated
	*/
	fprintf(stderr,"\nProjection:\ttype\tsize\tcount\n");
	source->recordsize = 0;
	for(i=0;i<fieldcount;i++){
	  GetFieldInfoByNumber(parmnames,i,&fieldinfo);
	  type = fieldinfo.type;
	  size = fieldinfo.size;
	  /*
	  ** echo the field name
	  */
	  fprintf(stderr,"P%d :'%s'\t%d\t%d\t%d\t(%s)\n",
		  i,
		  fieldinfo.name,
		  fieldinfo.type,
		  fieldinfo.size,
		  fieldinfo.count,
		  FieldTypeToString(fieldinfo.type)
		  );
	  if(fieldinfo.count > 1){
	    fprintf(stderr,"ERROR: multiple count (%d) fields (%s) not supported\n",fieldinfo.count,fieldinfo.name);
	    exit(0);
	  }
	  if(fieldinfo.type == -1){
	    fprintf(stderr,"No valid type information found for field %s.  Assuming default floating point fields.\n", fieldinfo.name);
	    type = FLOAT;
	    size = sizeof(float);
	  }
	  
	  /*
	  ** if the projections have not been previously allocated then
	  ** do it now
	  */
	  if(graph->nprojections < fieldcount){
	    /*
	    ** and set the projection name to the field name
	    */
	    if(SetProjectionName(graph,i,fieldinfo.name) == 0){
	      AddProjectionInfo(graph,i,fieldinfo.name,type,size);
	    }
	  }
	  source->recordsize += size;
	}
	source->nsourcefields = fieldcount;
	fprintf(stderr,"total recordsize = %d bytes\n",source->recordsize);
	
	/* Check to see if we have a time or timestamp field
	** (used below to test whether we can use a
	** 'degenerate parm-file 
	*/
	if (!GetFieldInfoByName(parmnames,"timestamp",&fieldinfo) &&
	    !GetFieldInfoByName(parmnames,"time",&fieldinfo))
	  source->notimefield = 1;
	
      }

      /*
      ** deal with old and new binary formats
      */
      if(source->fileformat == BINARY){
	/*
	** is this an old binary format file?
	*/
	if(fieldinfo.type == -1){
	  source->fileformat = OLDBINARY;
	}
      }
    }

    /*
    ** look for any field range restrictions
    ** (ranges will be checked in 'AddPoint')
    */
    CheckFieldRange(graph,&rangedata);
    
    /*
    ** automatically add a plot for the data file
    */
    plot = AddPlot(graph,source);
    plot->title = plot->filename = source->filename;
    
    /* PREPARE THE LOADING */
    if(source->loadmode == PARTIAL_LOAD){
      /*
      ** check the current line and determine whether
      ** rewinding is necessary
      */
      if(source->startline < source->currentline){
	fseek(source->fp,source->headersize,0L);
	source->currentline = 0;
      }
      if(source->currentline == 0){
	switch(source->fileformat){
	case BINARY:
	  if (debug) fprintf(stderr,"BINARY format\n");
	  if(got_header)
	    source->nsourcefields = fieldcount;
	  break;
/* 	case EEG: */
/* 	  if (debug) fprintf(stderr,"EEG format\n"); */
/* 	  source->nsourcefields = source->nchannels +1; */
/* 	  source->recordsize = sizeof(unsigned long) + source->nchannels*sizeof(short); */
/* 	  break; */
	case ASCII:
	  if (debug) fprintf(stderr,"ASCII format\n");
	  if(got_header)
	    source->nsourcefields = fieldcount;
	  break;
	default:
	  break;
	}
      }
    } else if(source->loadmode == PARTIAL_PLOT_LOAD){
      /*
      ** prepare to partially load the file by plot
      */
      
      /*
	fprintf(stderr,"partially loading plot %d to %d : ",
	source->startplot,source->endplot);
      */
      /*
      ** check the current plot and determine whether
      ** rewinding is necessary
	*/
      if(source->startplot < source->currentplot){
	/*
	** try to determine the file offset for the
	** beginning of the plot
	*/
	if((poffset = FindOffset(source,source->startplot)) == NULL){
	  fprintf(stderr,"unable to locate offset for plot %d\n",
		  source->startplot);
	  fseek(source->fp,source->headersize,0L);
	  source->currentline = 0;
	    source->currentplot = 0;
	} else {
	  /*
	  ** position the file at the beginning of the plot
	  */
	  fseek(source->fp,poffset->byteoffset,0L);
	  source->currentline = poffset->lineoffset;
	  source->currentplot = source->startplot;
	}
      }
      /*
      ** if its the beginning of the file then skip past the
      ** vector info header
      */
      if(source->currentplot == 0){
	switch(source->fileformat){
	case BINARY:
	  fprintf(stderr,"BINARY format\n");
	  if(got_header)
	    source->nsourcefields = fieldcount;
	  break;
/* 	case EEG: */
/* 	  fprintf(stderr,"EEG format\n"); */
/* 	  source->nsourcefields = source->nchannels +1; */
/* 	  source->recordsize = sizeof(unsigned long) + source->nchannels*sizeof(short); */
/* 	  break; */
	case ASCII:
	  fprintf(stderr,"ASCII format\n");
	  if(got_header)
	    source->nsourcefields = fieldcount;
	  break;
	default:
	  break;
	}
      }
    } else {
      switch(source->fileformat){
      case BINARY:
	fprintf(stderr,"BINARY format\n");
	if(got_header)
	    source->nsourcefields = fieldcount;
	break;
/*       case EEG: */
/* 	fprintf(stderr,"EEG format\n"); */
/* 	source->nsourcefields = source->nchannels +1; */
/* 	source->recordsize = sizeof(unsigned long) + source->nchannels*sizeof(short); */
/* 	break; */
      case ASCII:
	fprintf(stderr,"ASCII format\n");
	if(got_header)
	  source->nsourcefields = fieldcount;
	break;
      default:
	break;
      }
    }
    
    status = 0;
    
    /* DO THE ACTUAL LOADING */
    while(!feof(source->fp) && (status != -1)){
      oldplot = plot;
      /*
      ** keep track of the file position
      */
      bytefileoffset = ftell(source->fp);
      linefileoffset = source->currentline;
      /*
      ** is this an ascii or a binary file
      */
      switch(source->fileformat){
      case ASCII:
	/*
	** read a line from the data file
	*/
	if(fgets(line,1000,source->fp) == NULL) {
	  break;
	}
	if(line[0] == '%') continue;
	line[1000] = '\0';
	source->currentline++;
	
	/* DoCommand modifies 'line' string (via sgetargs), we
	** need a copy of the last line 
	** later */
	strcpy(lastline, line);
	
	if(source->loadmode == PARTIAL_LOAD){
	  if((source->startline >= 0) && 
	       (source->currentline <= source->startline)){
	    continue;
	  }
	  if((source->endline >= 0) && 
	     (source->currentline > source->endline+1)) { 
	    /* don't break, but continue, so we can use
	    ** currentline to determine the number of
	    ** records below */
	    continue;
	  }
	}
	DoCommand(graph,&plot,line);
	break;
/*       case EEG: */
/* 	/\* */
/* 	** read the timestamp */
/* 	*\/ */
/* 	if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){ */
/* 	  fprintf(stderr,"ERROR: unable to read timestamp\n"); */
/* 	  break; */
/* 	} */
/* 	time = timestamp; */
/* 	sprintf(timestampstr,"%u",timestamp); */
/* 	for(i=0;i<source->datablocksize;i+=source->nchannels){ */
/* 	  /\* */
/* 	  ** read a multichannel eeg vector and add the points */
/* 	  *\/ */
/* 	  bcopy(&timestamp,vector,sizeof(unsigned long)); */
/* 	  if(fread(vector+sizeof(unsigned long),sizeof(short),source->nchannels,fp) != source->nchannels){ */
/* 	    fprintf(stderr,"ERROR: unable to read eeg info\n"); */
/* 	    break; */
/* 	  } */
/* 	  source->currentline++; */
/* 	  if(source->loadmode == PARTIAL_LOAD){ */
/* 	    if((source->startline >= 0) && */
/* 	       (source->currentline <= source->startline)){ */
/* 	      continue; */
/* 	    } */
/* 	    /\* test for special '-1' endline, which indicates end of tile *\/ */
/* 	    if((source->endline >= 0) &&  */
/* 	       (source->currentline > source->endline+1)) {  */
/* 	      /\* don't break, but continue, so we can use */
/* 	      ** currentline to determine the number of */
/* 	      ** records below *\/ */
/* 	      continue; */
/* 	    } */
/* 	  } */
	  

/* 	  /\* */
/* 	  ** add it to the plot */
/* 	  *\/ */
/* 	  if((status = AddPoint(plot,NULL,vector,source->nsourcefields)) == -1) break; */
/* 	  time += source->eegdt; */
/* 	  timestamp = time; */
/* 	} */
/* 	plot->title = timestampstr; */
/* 	break; */
	case BINARY:
	  /*
	  ** read the data vector
	  */
	  if(fread(vector,sizeof(char),source->recordsize,source->fp) != 
	     source->recordsize) break;
	  /*
	    if(source->convert){
	    for(i=0;i<source->nsourcefields;i++){
	    ConvertData(vector+i,sizeof(float));
	    }
	    }
	  */
	  source->currentline++;
	  if(source->loadmode == PARTIAL_LOAD){
	    /* test for special '-1' startline, which indicates start of file */
	    if((source->startline >= 0) && 
	       (source->currentline <= source->startline)){
	      continue;
	    }
	    
	    /* test for special '-1' endline, which indicates end of tile */
	    if((source->endline >= 0) && 
	       (source->currentline > source->endline+1)) { 
	      /* don't break, but continue, so we can use
	      ** currentline to determine the number of
	      ** records below */
	      continue;
	    }
	  }
	  /*
	  ** add it to the plot
	  */
	  status = AddPoint(plot,NULL,vector,source->nsourcefields);
	  break;
      }
      
      if(source->loadmode == PARTIAL_PLOT_LOAD){
	if((source->endplot >= 0) && 
	   (source->currentplot > source->endplot)){
	  break;
	}
      }
    }
    
    plot = ClosePlot(plot);
    if(source->loadmode == PARTIAL_PLOT_LOAD){
      plot = oldplot;
    }
    
    if(source->loadmode == FULL_LOAD){
      fclose(source->fp);
      source->fp = NULL;
    }
    source->plot = plot;
    
  }
  
  
  if(plot != NULL){
    SelectPlot(plot);
    
    /* Determine whether this file is suitable for lookups of
    ** time<-->spike index. Flag the source object so we can test
    ** this flag, e.g. when we want to look up epoch start/end
    ** times.
    **
    ** Only test this the first time through the source (firstpass
    ** == 1), but do it down here to take advantage of looping
    ** already done above.
    **
    ** - The relevant features are whether the parm file is ASCII
    ** or BINARY, whether it has a field representing spike time,
    ** and whether it is 'degenerate' (i.e. whether it is a subset
    ** of spikes from a spikefile). These are represented in the
    ** new source struct members: 
    **       -source.subsetparmfile (0/1)
    **       -source.notimefield (0/1) 
    **       -source.timelookupfile (INVALIDFILE/PARMFILE/TTFILE)
    **
    **
    **   -the matrix of dependencies is as follows:
    **
    **	     | BINARY  |  ASCII  |
    **     -------------------------------
    **     full +ts   |1 PARM   |5   TT   |
    **     full -ts   |2  TT    |6   TT   |
    **     subset +ts |3 PARM   |7   -    |
    **     subset -ts |4   -    |8   -    |
    **
    ** Conceivably we could eventually work with all these
    ** files. For now, the limitations are:
    **
    **     - (7) findspike doesn't work on ASCII files
    **
    **     - (4) if we get a spike id range from a time range
    **     using findspike on a .TT file, we don't have a way of
    **     turning this into a range of indices into the
    **     degenerate parm file. This would be simple to fix, but
    **     even easier is to require people who want to work with
    **     degenerate parm files to have binary files with a time
    **     or timestamp field.
    **
    **     - (8) problem 4 and 7
    */
	
    if(source->loadmode == PARTIAL_LOAD && firstpass){

      source->timelookupfile = INVALIDFILE;

      /*
      ** First test whether it's degenerate by comparing # of
      ** records in the file (lastrec) with the last spike id
      ** (lastid).
      **
      */
	  
      /* get 'id' field column and type */
      GetFieldInfoByName(parmnames,"id",&fieldinfo);
      id_column = fieldinfo.column;
      id_type = fieldinfo.type;
	  
      switch(source->fileformat){
      case BINARY:
	    
	/*
	** get # of spikes in file
	*/
  
	lastrec = source->currentline;
	  
	/*
	** get last spike record's 'id' field
	*/

	/* get byte offset of id field */
	for (i = 0; 
	     GetFieldInfoByNumber(parmnames,i,&fieldinfo);
	     i++,
	       id_offset += fieldinfo.size * fieldinfo.count){
	  if (i >= id_column)
	    break;
	}
    
	/*          fprintf(stderr,"*** id_offset: %d",id_offset); */

	/* seek to last record + offset of id field */
	fseek(source->fp,-source->recordsize+id_offset,SEEK_END);

	/* read id field */
	switch(id_type){
	case FLOAT:
	  if(fread(&flastid,sizeof(float),1,source->fp) != 1){
	    fprintf(stderr,"ERROR: reading last index (float)\n");      
	    return(NULL);
	  }
	  lastid = (long)flastid;
	  break;
	    
	case INT:
	  if(fread(&ilastid,sizeof(int),1,source->fp) != 1){
	    fprintf(stderr,"ERROR: reading last index (int)\n");      
	    return(NULL);
	  }
	  lastid = (long)ilastid;
	  break;

	default:
	  fprintf(stderr,"ERROR: getting last index: only int and float type index fields supported");
	  return(NULL);
	  break;
	}
	break;
	    
      case ASCII:

	/* get last record in file */
	lastrec = source->currentline;

	/* get lastid */
	if (sgetargs(lastline,100,dvector) < id_column) {
	  fprintf(stderr,"ERROR: not enough data columns to find id field in column (%d)",id_column);
	  return(NULL);
	}
	lastid = dvector[id_column];
	break;
	    
      default:
	fprintf(stderr,"ERROR: can't do partial loads of EEG files?");
	break;
      }

      /*
      ** set subset flag 
      */
      source->subsetparmfile = (lastid >= lastrec);
	  
      /* display subset calculations */
      fprintf(stderr,
	      "\n"
	      "spikes in file:     %ld\n"
	      "last spike id:      %ld (0-indexed)\n"
	      "subset parm file?:  %s\n\n",
	      lastrec,
	      lastid,
	      source->subsetparmfile ? "Yes" : "No");

      /* 
      ** set nlines (number of spikes in file)
      */
      
      source->nlines = lastrec;
      
      /*
      ** determine if we are going to be able to use this source
      ** (parm) file for time lookups
      */

      switch (source->fileformat){  
      case ASCII: 
	if (source->subsetparmfile){
	  fprintf(stderr,"WARNING: epochs not supported (degenerate ASCII parm file)\n");   
	  break;
	} 
	source->timelookupfile = TTFILE;
	break;
	    
      case BINARY:
	if(!source->notimefield){ /* use parm file if it has time info */
	  source->timelookupfile = PARMFILE;
	  break;
	}
	else /* use spike file unless parm file is degenerate */
	  if(!source->subsetparmfile){
	    source->timelookupfile = TTFILE;
	    break;
	  } 
	/* notimefield & subsetparmfile */
	fprintf(stderr,"WARNING: epochs not supported (degenerate parm files with no time/timestamp field)\n");   
	break;
	  	    
      default:
	fprintf(stderr,"WARNING: epochs not supported (non-ASCII, non-BINARY (EEG?) parm file)\n");   
	break;
      }
    }
	
    /* epochs not meaningful in partialplotload (aka view-spikes) mode */
    if (source->loadmode == PARTIAL_PLOT_LOAD)
      source->timelookupfile = INVALIDFILE;

  }

  /* now we want to find the time corresponding to the start and end point */
  if (source->timelookupfile != INVALIDFILE) {
    if (source->startline < 0)
      startline_temp = 0;
    else
      startline_temp = source->startline;

    sprintf(str, "%d", startline_temp);

    tmpstr = (char*)LookupSpikeTime(source, str);
    if (tmpstr!=NULL)
      source->starttimestamp = Atoi(tmpstr);

    if(source->plot)
      sprintf(str,"%d",startline_temp+source->plot->npoints-1);
    else
      sprintf(str,"%d",-1);

    tmpstr = (char*)LookupSpikeTime(source, str);
    if (tmpstr!=NULL)
      source->endtimestamp = Atoi(tmpstr);

    /* determine timestamp of first last spike in file */
    if(firstpass){
      source->mintimestamp = source->starttimestamp;
      source->maxtimestamp = source->endtimestamp;
    }

  }

  return(plot);

}

/* Plot *LoadPlotFromSource(graph,source) */
/* Graph	*graph; */
/* DataSource	*source; */
/* { */
/* static char timestampstr[50]; */
/* Plot	*plot; */
/* Plot	*oldplot; */
/* FILE	*fp; */
/* char	line[1001]; */
/* char	lastline[1001]; */
/* int	headersize; */
/* char	**header; */
/* char	*filetype; */
/* int	*vectorformat; */
/* char	vector[1000]; */
/* double  dvector[100];  */
/* unsigned char diodeval[4]; */
/* short diodeval2[4]; */
/* char	*parmnames; */
/* char	*sptr; */
/* int	fieldcount; */
/* long	bytefileoffset; */
/* int	linefileoffset; */
/* int	xsize,ysize; */
/* PlotOffset	*poffset; */
/* unsigned long timestamp; */
/* char		str[30]; */
/* int	i,j; */
/* FieldInfo	fieldinfo; */
/* int	status; */
/* int	got_header; */
/* short	type; */
/* short	size; */
/* char	channelstr[80]; */
/* char	*nchannelstr; */
/* char	*ratestr; */
/* double	time; */
/* int	startline_temp; */

/*  int	id_column = 0; */
/*  int	id_type = INVALID_TYPE; */
/*  int	id_offset = 0; */
/*  float	flastid = 0; */
/*  int	ilastid = 0; */
/* long	lastrec = 0; */
/* long	lastid = 0; */
/*  int	firstpass = 0; /\* is this our first time through the source file*\/ */

/*     oldplot = NULL; */
/*     plot = NULL; */
/*     got_header = 0; */
/*     if(source->type == FROM_FILE){ */
/* 	if(source->fp == NULL){ */
/* 	    /\* */
/* 	    ** open the data file associated with the plot */
/* 	    *\/ */
/* 	    if(strcmp(source->filename,"STDIN") == 0){ */
/* 		if(source->loadmode != FULL_LOAD){ */
/* 		    printf("can't open 'STDIN' in partial load mode \n"); */
/* 		    /\* */
/* 		    DeleteSource(graph,source); */
/* 		    *\/ */
/* 		    return(NULL); */
/* 		} else { */
/* 		    fp = stdin; */
/* 		} */
/* 	    } else */
/* 	    if((fp = fopen(source->filename,"r")) == NULL){ */
/* 		printf("can't open file '%s'\n",source->filename); */
/* 		/\* */
/* 		DeleteSource(graph,source); */
/* 		*\/ */
/* 		return(NULL); */
/* 	    } */
/* 	    source->fp = fp; */
/* 	} else { */
/* 	    fp = source->fp; */
/* 	} */
/* 	/\* */
/* 	** is it the beginning of the file? */
/* 	*\/ */
/*  	if(source->currentline == 0){  */

/* 	  firstpass = 1; */
/* 	    /\* */
/* 	    ** read the file header */
/* 	    *\/ */
/* 	    header = ReadHeader(fp,&headersize); */
/* 	    /\* */
/* 	    ** compare architectures */
/* 	    *\/ */
/* 	    if((GetLocalArchitecture() == GetFileArchitecture(header)) || */
/* 		(GetFileArchitecture(header) == ARCH_UNKNOWN)) { */
/* 		source->convert = 0; */

/* 	    } else { */
/* 		source->convert = 1; */
/* 		fprintf(stderr,"Data conversion no longer supported"); */
/* 		exit(0); */
/* 	    } */
/* 	    got_header = 1; */

/* 	    /\* */
/* 	    DisplayHeader(stderr,header,headersize); */
/* 	    *\/ */

/* 	    source->headersize = headersize; */

/* 	    /\* */
/* 	    ** track the position of the first plot */
/* 	    *\/ */
/* 	    SavePlotOffset(source,source->currentplot,ftell(source->fp), */
/* 	    source->currentline); */
/* 	    /\* */
/* 	    ** look for the file format in the header */
/* 	    *\/ */

/* 	    if((filetype = GetHeaderParameter(header,"Extraction type:")) != NULL){ */
/*                 /\* */
/*                 ** look for the dual diode string */
/*                 *\/ */
/*                 if(strncmp(filetype,"continuous data", */
/*                 strlen("continuous data")) == 0){ */
/*                     fprintf(stderr,"file type = '%s'\n",filetype); */
/*                     source->fileformat = EEG; */
/*                 } */
/*                 if(source->fileformat == EEG){ */
/* 		  if(SetProjectionName(graph,0,"timestamp") == 0){ */
/* 		    AddProjectionInfo(graph,0,"timestamp",ULONG,sizeof(unsigned long)); */
/* 		  } */
/* 		  /\* */
/* 		  ** determine the number of channels */
/* 		  *\/ */
/* 		  if((nchannelstr = GetHeaderParameter(header,"nchannels:")) != NULL){ */
/* 		    source->nchannels = atoi(nchannelstr); */
/* 		    fprintf(stderr,"%d eeg channels\n",source->nchannels); */
/* 		    for(i=0;i<source->nchannels;i++){ */
/* 		      sprintf(channelstr,"CH %d",i); */
/* 		      if(SetProjectionName(graph,i+1,channelstr) == 0){ */
/* 			AddProjectionInfo(graph,i+1,channelstr,SHORT,sizeof(short)); */
/* 			    } */
/* 		    } */
/* 		    source->nsourcefields = source->nchannels +1; */
/* 		    source->recordsize = sizeof(unsigned long) + source->nchannels*sizeof(short); */
/* 		  } */
/*                 } else { */
/* 		  source->fileformat = GetFileType(header); */
/* 		} */
/*             } else { */
/* 		source->fileformat = GetFileType(header); */
/* 	    } */
/* 	    parmnames = GetFieldString(header); */
/* 	    fieldcount = GetFieldCount(parmnames); */
/* 	    if(debug) fprintf(stderr,"count = %d ; fieldstr = '%s'\n",fieldcount,parmnames); */
/* 	    if(source->fileformat == EEG){ */
/* 		/\* */
/* 		** determine the data block size */
/* 		*\/ */
/* 		GetFieldInfoByNumber(parmnames,1,&fieldinfo); */
/* 		source->datablocksize = fieldinfo.count; */
/* 		fprintf(stderr,"data blocksize = %d\n",source->datablocksize); */
/* 		/\* */
/* 		** determine the sampling rate */
/* 		*\/ */
/* 		if((ratestr = GetHeaderParameter(header,"rate:")) != NULL){ */
/* 		    source->eegdt = (10000*source->nchannels)/atof(ratestr); */
/* 		    fprintf(stderr,"dt per channel = %g\n",source->eegdt); */
/* 		} else { */
/* 		    fprintf(stderr,"ERROR: cannot determine sampling rate from header\n"); */
/* 		    source->eegdt = 1; */
/* 		} */
/* 	    } */
/* 	    /\* */
/* 	    ** if the file format is empty assume ASCII */
/* 	    *\/ */
/* 	    if(source->fileformat == INVALID_TYPE){ */
/* 		    fprintf(stderr,"No filetype found in header.  Assuming ASCII.\n"); */
/* 		source->fileformat = ASCII; */
/* 	    } */
/* 	    if(fieldcount == 0){ */
/* 		if(source->fileformat == ASCII){ */
/* 		    fprintf(stderr,"No Fields found in header.  Assuming default floating point fields.\n"); */
/* 		} else { */
/* 		    fprintf(stderr,"No Fields found in header. Unable to read datafile.\n"); */
/* 		    exit(-1); */
/* 		} */
/*  	    }  */

/* 	    if((source->fileformat == ASCII) || (source->fileformat == BINARY)){ */

/* 		/\* */
/* 		** check to see if projections have already been allocated */
/* 		*\/ */
/* 		fprintf(stderr,"\nProjection:\ttype\tsize\tcount\n"); */
/* 		source->recordsize = 0; */
/* 		for(i=0;i<fieldcount;i++){ */
/* 		    GetFieldInfoByNumber(parmnames,i,&fieldinfo); */
/* 		    type = fieldinfo.type; */
/* 		    size = fieldinfo.size; */
/* 		    /\* */
/* 		    ** echo the field name */
/* 		    *\/ */
/* 		    fprintf(stderr,"P%d :'%s'\t%d\t%d\t%d\t(%s)\n", */
/* 			    i, */
/* 			    fieldinfo.name, */
/* 			    fieldinfo.type, */
/* 			    fieldinfo.size, */
/* 			    fieldinfo.count, */
/* 			    FieldTypeToString(fieldinfo.type) */
/* 		    ); */
/* 		    if(fieldinfo.count > 1){ */
/* 			fprintf(stderr,"ERROR: multiple count (%d) fields (%s) not supported\n",fieldinfo.count,fieldinfo.name); */
/* 			exit(0); */
/* 		    } */
/* 		    if(fieldinfo.type == -1){ */
/* 			fprintf(stderr,"No valid type information found for field %s.  Assuming default floating point fields.\n", fieldinfo.name); */
/* 			type = FLOAT; */
/* 			size = sizeof(float); */
/* 		    } */

/* 		    /\* */
/* 		    ** if the projections have not been previously allocated then */
/* 		    ** do it now */
/* 		    *\/ */
/* 		    if(graph->nprojections < fieldcount){ */
/* 			/\* */
/* 			** and set the projection name to the field name */
/* 			*\/ */
/* 			if(SetProjectionName(graph,i,fieldinfo.name) == 0){ */
/* 			    AddProjectionInfo(graph,i,fieldinfo.name,type,size); */
/* 			} */
/* 		    } */
/* 		    source->recordsize += size; */
/* 		} */
/* 		source->nsourcefields = fieldcount; */
/* 		fprintf(stderr,"total recordsize = %d bytes\n",source->recordsize); */
		
/* 		/\* Check to see if we have a time or timestamp field */
/* 		** (used below to test whether we can use a */
/* 		** 'degenerate parm-file  */
/* 		*\/ */
/* 		if (!GetFieldInfoByName(parmnames,"timestamp",&fieldinfo) && */
/* 		    !GetFieldInfoByName(parmnames,"time",&fieldinfo)) */
/* 		  source->notimefield = 1; */

/* 	    } */
/* 	    /\* */
/* 	    ** deal with old and new binary formats */
/* 	    *\/ */
/* 	    if(source->fileformat == BINARY){ */
/* 		/\* */
/* 		** is this an old binary format file? */
/* 		*\/ */
/* 		if(fieldinfo.type == -1){ */
/* 		    source->fileformat = OLDBINARY; */
/* 		} */
/* 	    } */
/* 	} */
/* 	/\* */
/* 	** look for any field range restrictions */
/* 	** (ranges will be checked in 'AddPoint') */
/* 	*\/ */
/* 	CheckFieldRange(graph,&rangedata); */

/* 	/\* */
/* 	** read the data source file */
/* 	fprintf(stderr,"Loading from %s",source->filename); */
/* 	*\/ */
/* 	/\* */
/* 	** automatically add a plot for the data file */
/* 	*\/ */
/* 	plot = AddPlot(graph,source); */
/* 	/\* */
/* 	** give the plot the default title which is the */
/* 	** filename the data is coming from */
/* 	*\/ */
/* 	plot->title = plot->filename = source->filename; */
/* 	/\* */
/* 	** prepare to partially load the file by line */
/* 	*\/ */
/* 	if(source->loadmode == PARTIAL_LOAD){ */
/* 	    /\* */
/* 	    fprintf(stderr,"points %d to %d : ", */
/* 	    source->startline,source->endline); */
/* 	    *\/ */
/* 	    /\* */
/* 	    ** check the current line and determine whether */
/* 	    ** rewinding is necessary */
/* 	    *\/ */
/* 	    if(source->startline < source->currentline){ */
/* 		fseek(fp,source->headersize,0L); */
/* 		source->currentline = 0; */
/* 	    } */
/* 	    if(source->currentline == 0){ */
/* 		switch(source->fileformat){ */
/* 		case BINARY: */
/* 		    if (debug) fprintf(stderr,"BINARY format\n"); */
/* 		    if(got_header) */
/* 			source->nsourcefields = fieldcount; */
/* 		    break; */
/*                 case EEG: */
/*                     if (debug) fprintf(stderr,"EEG format\n"); */
/*                     source->nsourcefields = source->nchannels +1; */
/* 		    source->recordsize = sizeof(unsigned long) + source->nchannels*sizeof(short); */
/*                     break; */
/* 		case ASCII: */
/* 		    if (debug) fprintf(stderr,"ASCII format\n"); */
/* 		    if(got_header) */
/* 			source->nsourcefields = fieldcount; */
/* 		    break; */
/* 		default: */
/* 		    break; */
/* 		} */
/* 	    } */
/* 	} else */
/* 	/\* */
/* 	** prepare to partially load the file by plot */
/* 	*\/ */
/* 	if(source->loadmode == PARTIAL_PLOT_LOAD){ */
/* 	    /\* */
/* 	    fprintf(stderr,"partially loading plot %d to %d : ", */
/* 	    source->startplot,source->endplot); */
/* 	    *\/ */
/* 	    /\* */
/* 	    ** check the current plot and determine whether */
/* 	    ** rewinding is necessary */
/* 	    *\/ */
/* 	    if(source->startplot < source->currentplot){ */
/* 		/\* */
/* 		** try to determine the file offset for the */
/* 		** beginning of the plot */
/* 		*\/ */
/* 		if((poffset = FindOffset(source,source->startplot)) == NULL){ */
/* 		    fprintf(stderr,"unable to locate offset for plot %d\n", */
/* 		    source->startplot); */
/* 		    fseek(fp,source->headersize,0L); */
/* 		    source->currentline = 0; */
/* 		    source->currentplot = 0; */
/* 		} else { */
/* 		    /\* */
/* 		    ** position the file at the beginning of the plot */
/* 		    *\/ */
/* 		    fseek(fp,poffset->byteoffset,0L); */
/* 		    source->currentline = poffset->lineoffset; */
/* 		    source->currentplot = source->startplot; */
/* 		} */
/* 	    } */
/* 	    /\* */
/* 	    ** if its the beginning of the file then skip past the */
/* 	    ** vector info header */
/* 	    *\/ */
/* 	    if(source->currentplot == 0){ */
/* 		switch(source->fileformat){ */
/* 		case BINARY: */
/* 		    fprintf(stderr,"BINARY format\n"); */
/* 		    if(got_header) */
/* 			source->nsourcefields = fieldcount; */
/* 		    break; */
/*                 case EEG: */
/*                     fprintf(stderr,"EEG format\n"); */
/*                     source->nsourcefields = source->nchannels +1; */
/* 		    source->recordsize = sizeof(unsigned long) + source->nchannels*sizeof(short); */
/*                     break; */
/* 		case ASCII: */
/* 		    fprintf(stderr,"ASCII format\n"); */
/* 		    if(got_header) */
/* 			source->nsourcefields = fieldcount; */
/* 		    break; */
/* 		default: */
/* 		    break; */
/* 		} */
/* 	    } */
/* 	} else { */
/* 	    switch(source->fileformat){ */
/* 	    case BINARY: */
/* 		fprintf(stderr,"BINARY format\n"); */
/* 		if(got_header) */
/* 		    source->nsourcefields = fieldcount; */
/* 		break; */
/* 	    case EEG: */
/* 		fprintf(stderr,"EEG format\n"); */
/* 		source->nsourcefields = source->nchannels +1; */
/* 		source->recordsize = sizeof(unsigned long) + source->nchannels*sizeof(short); */
/* 		break; */
/* 	    case ASCII: */
/* 		fprintf(stderr,"ASCII format\n"); */
/* 		if(got_header) */
/* 		    source->nsourcefields = fieldcount; */
/* 		break; */
/* 	    default: */
/* 		break; */
/* 	    } */
/* 	} */
/* 	status = 0; */
/* 	while(!feof(fp) && (status != -1)){ */
/* 	    oldplot = plot; */
/* 	    /\* */
/* 	    ** keep track of the file position */
/* 	    *\/ */
/* 	    bytefileoffset = ftell(fp); */
/* 	    linefileoffset = source->currentline; */
/* 	    /\* */
/* 	    ** is this an ascii or a binary file */
/* 	    *\/ */
/* 	    switch(source->fileformat){ */
/* 	    case ASCII: */
/* 		/\* */
/* 		** read a line from the data file */
/* 		*\/ */
/* 		if(fgets(line,1000,fp) == NULL) { */
/* 		    break; */
/* 		} */
/* 		if(line[0] == '%') continue; */
/* 		line[1000] = '\0'; */
/* 		source->currentline++; */

/* 		/\* DoCommand modifies 'line' string (via sgetargs), we */
/* 		** need a copy of the last line  */
/* 		** later *\/ */
/* 		strcpy(lastline, line); */

/* 		if(source->loadmode == PARTIAL_LOAD){ */
/* 		    if((source->startline >= 0) &&  */
/* 			(source->currentline <= source->startline)){ */
/* 			continue; */
/* 		    } */
/* 		    if((source->endline >= 0) &&  */
/* 		       (source->currentline > source->endline+1)) {  */
/* 		      /\* don't break, but continue, so we can use */
/* 		      ** currentline to determine the number of */
/* 		      ** records below *\/ */
/* 		      continue; */
/* 		    } */
/* 		} */
/* 		DoCommand(graph,&plot,line); */
/* 		break; */
/*             case EEG: */
/*                 /\* */
/*                 ** read the timestamp */
/*                 *\/ */
/*                 if(fread(&timestamp,sizeof(unsigned long),1,fp) != 1){ */
/*                     fprintf(stderr,"ERROR: unable to read timestamp\n"); */
/*                     break; */
/*                 } */
/* 		time = timestamp; */
/*                 sprintf(timestampstr,"%u",timestamp); */
/* 		for(i=0;i<source->datablocksize;i+=source->nchannels){ */
/* 		    /\* */
/* 		    ** read a multichannel eeg vector and add the points */
/* 		    *\/ */
/* 		    bcopy(&timestamp,vector,sizeof(unsigned long)); */
/* 		    if(fread(vector+sizeof(unsigned long),sizeof(short),source->nchannels,fp) != source->nchannels){ */
/* 			fprintf(stderr,"ERROR: unable to read eeg info\n"); */
/* 			break; */
/* 		    } */
/* 		    source->currentline++; */
/* 		    if(source->loadmode == PARTIAL_LOAD){ */
/* 			if((source->startline >= 0) && */
/* 			    (source->currentline <= source->startline)){ */
/* 			    continue; */
/* 			} */
/* 			/\* test for special '-1' endline, which indicates end of tile *\/ */
/* 			if((source->endline >= 0) &&  */
/* 			   (source->currentline > source->endline+1)) {  */
/* 			  /\* don't break, but continue, so we can use */
/* 			  ** currentline to determine the number of */
/* 			  ** records below *\/ */
/* 			  continue; */
/* 			} */
/* 		    } */

/* 		    /\* */
/* 		    fprintf(stderr,"%u: ",timestamp); */
/* 		    for(j=0;j<source->nchannels;j++){ */
/* 			fprintf(stderr,"\t%d",*((short *)(vector+sizeof(long)+sizeof(short)*j))); */
/* 		    } */
/* 		    fprintf(stderr,"\n"); */
/* 		    *\/ */
/* 		    /\* */
/* 		    ** add it to the plot */
/* 		    *\/ */
/* 		    if((status = AddPoint(plot,NULL,vector,source->nsourcefields)) == -1) break; */
/* 		    time += source->eegdt; */
/* 		    timestamp = time; */
/* 		} */
/*                 plot->title = timestampstr; */
/*                 break; */
/* 	    case BINARY: */
/* 		/\* */
/* 		** read the data vector */
/* 		*\/ */
/* 		if(fread(vector,sizeof(char),source->recordsize,fp) !=  */
/* 		    source->recordsize) break; */
/* 		    /\* */
/* 		if(source->convert){ */
/* 		    for(i=0;i<source->nsourcefields;i++){ */
/* 			ConvertData(vector+i,sizeof(float)); */
/* 		    } */
/* 		} */
/* 		*\/ */
/* 		source->currentline++; */
/* 		if(source->loadmode == PARTIAL_LOAD){ */
/* 		  /\* test for special '-1' startline, which indicates start of file *\/ */
/* 		    if((source->startline >= 0) &&  */
/* 			(source->currentline <= source->startline)){ */
/* 		      continue; */
/* 		    } */

/* 		    /\* test for special '-1' endline, which indicates end of tile *\/ */
/* 		    if((source->endline >= 0) &&  */
/* 		       (source->currentline > source->endline+1)) {  */
/* 		      /\* don't break, but continue, so we can use */
/* 		      ** currentline to determine the number of */
/* 		      ** records below *\/ */
/* 		      continue; */
/* 		    } */
/* 		} */
/* 		/\* */
/* 		** add it to the plot */
/* 		*\/ */
/* 		status = AddPoint(plot,NULL,vector,source->nsourcefields); */
/* 		break; */
/* 	    } */
/* 	    /\* */
/* 	    ** has a new plot been created as a result of the  */
/* 	    ** line processed */
/* 	    if(oldplot != plot){ */
/* 		source->currentplot++; */
/* 	    } */
/* 	    *\/ */
/* 	    /\* */
/* 	    ** check the number of plots */
/* 	    *\/ */
/* 	    /\* */
/* 	    if(source->loadmode == PARTIAL_LOAD){ */
/* 		if((source->startplot >= 0) &&  */
/* 		    (source->currentplot < source->startplot)){ */
/* 		    DeletePlot(plot); */
/* 		    plot = AddPlot(graph,source); */
/* 		    continue; */
/* 		} */
/* 	    } */
/* 	    *\/ */
/* 	    /\* */
/* 	    ** if partially loading by plot then see whether the */
/* 	    ** requested plots have been loaded */
/* 	    *\/ */
/* 	    if(source->loadmode == PARTIAL_PLOT_LOAD){ */
/* 		if((source->endplot >= 0) &&  */
/* 		(source->currentplot > source->endplot)){ */
/* 		    break; */
/* 		} */
/* 	    } */
/* 	} */

/* 	plot = ClosePlot(plot); */
/* 	if(source->loadmode == PARTIAL_PLOT_LOAD){ */
/* 	    plot = oldplot; */
/* 	} */
/* 	/\* */
/* 	if(plot){ */
/* 	    fprintf(stderr,"%d points\n", plot->npoints); */
/* 	} else { */
/* 	    fprintf(stderr,"no points\n"); */
/* 	} */
/* 	*\/ */
/* 	if(source->loadmode == FULL_LOAD){ */
/* 	    fclose(fp); */
/* 	    source->fp = NULL; */
/* 	} */
/* 	source->plot = plot; */
/* 	/\* */
/* 	** update the relevant menu dialogs */
/* 	*\/ */
/* 	sprintf(str,"%d",source->startline); */
/* 	PutItemValue("/epochmenu/startspike",str); */

/* 	/\* after a successful -1 / -1 load, put the actual spike */
/* 	   record numbers in the menu *\/ */
/* 	if (source->startline < 0){ */
/* 	  startline_temp = 0; */
/* 	  PutItemValue("/epochmenu/startspike","0"); */
/* 	} */
/* 	else */
/* 	  startline_temp = source->startline; */
/* 	if(source->plot){ */
/* 	    sprintf(str,"%d",startline_temp+source->plot->npoints-1); */
/* 	} else { */
/* 	    sprintf(str,"%d",-1); */
/* 	} */
/* 	PutItemValue("/epochmenu/endspike",str); */
/* 	/\* */
/* 	DrawMenu(graph->frame->menu); */
/* 	*\/ */
/* 	RefreshItem("/epochmenu/startspike"); */
/* 	RefreshItem("/epochmenu/endspike"); */

/*     } */
/*     if(plot != NULL){ */
/* 	SelectPlot(plot); */
	
/* 	/\* Determine whether this file is suitable for lookups of */
/* 	** time<-->spike index. Flag the source object so we can test */
/* 	** this flag, e.g. when we want to look up epoch start/end */
/* 	** times. */
/* 	** */
/* 	** Only test this the first time through the source (firstpass */
/* 	** == 1), but do it down here to take advantage of looping */
/* 	** already done above. */
/* 	** */
/* 	** - The relevant features are whether the parm file is ASCII */
/* 	** or BINARY, whether it has a field representing spike time, */
/* 	** and whether it is 'degenerate' (i.e. whether it is a subset */
/* 	** of spikes from a spikefile). These are represented in the */
/* 	** new source struct members:  */
/* 	**       -source.subsetparmfile (0/1) */
/* 	**       -source.notimefield (0/1)  */
/* 	**       -source.timelookupfile (INVALIDFILE/PARMFILE/TTFILE) */
/* 	** */
/* 	** */
/* 	**   -the matrix of dependencies is as follows: */
/* 	** */
/* 	**	     | BINARY  |  ASCII  | */
/* 	**     ------------------------------- */
/* 	**     full +ts   |1 PARM   |5   TT   | */
/* 	**     full -ts   |2  TT    |6   TT   | */
/* 	**     subset +ts |3 PARM   |7   -    | */
/* 	**     subset -ts |4   -    |8   -    | */
/* 	** */
/* 	** Conceivably we could eventually work with all these */
/* 	** files. For now, the limitations are: */
/* 	** */
/* 	**     - (7) findspike doesn't work on ASCII files */
/* 	** */
/* 	**     - (4) if we get a spike id range from a time range */
/* 	**     using findspike on a .TT file, we don't have a way of */
/* 	**     turning this into a range of indices into the */
/* 	**     degenerate parm file. This would be simple to fix, but */
/* 	**     even easier is to require people who want to work with */
/* 	**     degenerate parm files to have binary files with a time */
/* 	**     or timestamp field. */
/* 	** */
/* 	**     - (8) problem 4 and 7 */
/* 	*\/ */
	
/* 	if(source->loadmode == PARTIAL_LOAD && firstpass){ */

/* 	  source->timelookupfile = INVALIDFILE; */

/* 	  /\* */
/* 	  ** First test whether it's degenerate by comparing # of */
/* 	  ** records in the file (lastrec) with the last spike id */
/* 	  ** (lastid). */
/* 	  ** */
/* 	  *\/ */
	  
/* 	  /\* get 'id' field column and type *\/ */
/* 	  GetFieldInfoByName(parmnames,"id",&fieldinfo); */
/* 	  id_column = fieldinfo.column; */
/* 	  id_type = fieldinfo.type; */
	  
/* 	  switch(source->fileformat){ */
/*      	  case BINARY: */
	    
/* 	    /\* */
/* 	    ** get # of spikes in file */
/* 	    *\/ */
  
/* 	    lastrec = source->currentline; */
	  
/* 	    /\* */
/* 	    ** get last spike record's 'id' field */
/* 	    *\/ */

/* 	    /\* get byte offset of id field *\/ */
/* 	    for (i = 0;  */
/* 		 GetFieldInfoByNumber(parmnames,i,&fieldinfo); */
/* 		 i++, */
/* 		   id_offset += fieldinfo.size * fieldinfo.count){ */
/* 	      if (i >= id_column) */
/* 		break; */
/* 	    } */
    
/* /\*          fprintf(stderr,"*** id_offset: %d",id_offset); *\/ */

/* 	    /\* seek to last record + offset of id field *\/ */
/* 	    fseek(fp,-source->recordsize+id_offset,SEEK_END); */

/* 	    /\* read id field *\/ */
/* 	    switch(id_type){ */
/* 	    case FLOAT: */
/* 	      if(fread(&flastid,sizeof(float),1,fp) != 1){ */
/* 		fprintf(stderr,"ERROR: reading last index (float)\n");       */
/* 		return(NULL); */
/* 	      } */
/* 	      lastid = (long)flastid; */
/* 	      break; */
	    
/* 	    case INT: */
/* 	      if(fread(&ilastid,sizeof(int),1,fp) != 1){ */
/* 		fprintf(stderr,"ERROR: reading last index (int)\n");       */
/* 		return(NULL); */
/* 	      } */
/* 	      lastid = (long)ilastid; */
/* 	      break; */

/* 	    default: */
/* 	      fprintf(stderr,"ERROR: getting last index: only int and float type index fields supported"); */
/* 	      return(NULL); */
/* 	      break; */
/* 	    } */
/* 	    break; */
	    
/* 	  case ASCII: */

/* 	    /\* get last record in file *\/ */
/* 	    lastrec = source->currentline; */

/* 	    /\* get lastid *\/ */
/* 	    if (sgetargs(lastline,100,dvector) < id_column) { */
/* 	      fprintf(stderr,"ERROR: not enough data columns to find id field in column (%d)",id_column); */
/* 	      return(NULL); */
/* 	    } */
/* 	    lastid = dvector[id_column]; */
/* 	    break; */
	    
/* 	  default: */
/* 	    fprintf(stderr,"ERROR: can't do partial loads of EEG files?"); */
/* 	    break; */
/* 	  } */

/* 	  /\* */
/* 	  ** set subset flag  */
/* 	  *\/ */
/* 	  source->subsetparmfile = (lastid > lastrec); */
	  
/* 	  /\* display subset calculations *\/ */
/* 	  fprintf(stderr, */
/* 		  "\n" */
/* 		  "spikes in file:     %ld\n" */
/*                   "last spike id:      %ld (0-indexed)\n" */
/* 		  "subset parm file?:  %s\n\n", */
/* 		  lastrec, */
/* 		  lastid, */
/* 		  source->subsetparmfile ? "Yes" : "No"); */

/* 	  /\* */
/* 	  ** determine if we are going to be able to use this source */
/* 	  ** (parm) file for time lookups */
/* 	  *\/ */

/* 	  switch (source->fileformat){   */
/* 	  case ASCII:  */
/* 	    if (source->subsetparmfile){ */
/* 	      fprintf(stderr,"WARNING: epochs not supported (degenerate ASCII parm file)\n");    */
/* 	      break; */
/* 	    }  */
/* 	    source->timelookupfile = TTFILE; */
/* 	    break; */
	    
/* 	  case BINARY: */
/* 	    if(!source->notimefield){ /\* use parm file if it has time info *\/ */
/* 	      source->timelookupfile = PARMFILE; */
/* 	      break; */
/* 	    } */
/* 	    else /\* use spike file unless parm file is degenerate *\/ */
/* 	      if(!source->subsetparmfile){ */
/* 		source->timelookupfile = TTFILE; */
/* 		break; */
/* 	      }  */
/* 	    /\* notimefield & subsetparmfile *\/ */
/* 	    fprintf(stderr,"WARNING: epochs not supported (degenerate parm files with no time/timestamp field)\n");    */
/* 	    break; */
	  	    
/* 	  default: */
/* 	    fprintf(stderr,"WARNING: epochs not supported (non-ASCII, non-BINARY (EEG?) parm file)\n");    */
/* 	    break; */
/* 	  } */
/* 	} */
	
/* 	/\* epochs not meaningful in partialplotload (aka view-spikes) mode *\/ */
/* 	if (source->loadmode == PARTIAL_PLOT_LOAD) */
/* 	  source->timelookupfile = INVALIDFILE; */

/* /\* 	if (source->timelookupfile != INVALIDFILE){ *\/ */
/* /\* 	  /\\* refresh times corresponding to points loaded *\\/ *\/ */
/* /\* 	  CallMenuItem("/menu/startspike"); *\/ */
/* /\* 	  CallMenuItem("/menu/endspike"); *\/ */
/* /\* 	} *\/ */
/*     } */
/*     return(plot); */
/* } */

SetPlotDefaults(graph)
Graph	*graph;
{
Plot		*plot;
int		selected=0;

    /*
    ** make sure there are plots. If not then exit
    */
    if(!graph->plot){
	return;
    }
    /*
    ** setup the default linestyle of the plots
    */
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->linestyle == -1){
	    plot->linestyle = graph->foreground;
	}
	if(plot->selected){
	    selected = 1;
	}
    }
    if(!selected){
	/*
	** select the first plot
	*/
	graph->plot->selected = 1;
    }
}


LoadPlot(graph,filename,loadmode,startline,endline,startplot,endplot)
Graph *graph;
char	*filename;
int	loadmode;
int	startline;
int	endline;
int	startplot;
int	endplot;
{
DataSource *source;
DataSource *AddSource();
Plot		*plot;

    if(graph == NULL || filename == NULL) return;
    source = AddSource(graph);
    source->type = FROM_FILE;
    source->loadmode = loadmode;
    source->startline = startline;
    source->endline = endline;
    source->startplot = startplot;
    source->endplot = endplot;
    source->filename = CopyString(filename);
    LoadPlotFromSource(graph,source);
    SetPlotDefaults(graph);
}

DoCommand(graph,plot,lineptr)
Graph	*graph;
Plot	**plot;
char 	*lineptr;
{
char *strchr();
Plot	*p;
int	status;

    do {
	/* skip white space */
	while(lineptr && 
	(*lineptr == ' ' || *lineptr == '\t' || *lineptr == '\n')){
	    if(*lineptr == '\0') break;
	    lineptr++;
	}
	if(lineptr[0] == ';'){
	    lineptr++;
	    /* skip white space */
	    while(lineptr && 
	    (*lineptr == ' ' || *lineptr == '\t' || *lineptr == '\n')){
		if(*lineptr == '\0') break;
		lineptr++;
	    }
	}
	/*
	** check to see if it is a plot command
	*/
	if(lineptr[0] == '/'){
	    if(GlobalOperation()){
		for(p=graph->plot;p;p=p->next){
		    InterpretCommand(graph,&p,lineptr);
		}
		ResetGlobalOperation();
	    } else {
		InterpretCommand(graph,plot,lineptr);
	    }
	} else 
	if(lineptr[0] == '%'){
	    /*
	    ** skip the rest of the line for a comment
	    */
	    return;
	} else 
	if(*lineptr == '\0'){
	    break;
	} else {
	    /*
	    ** otherwise assume that the line contains coordinates
	    ** to be added to the plot
	    */
	    /*
	    ** is there a plot to add it too?
	    */
	    if(*plot == NULL){
		/*
		** if not then add one ( this should never happen)
		*/
		fprintf(stderr,"NULL plot\n");
		*plot = AddPlot(graph,NULL);
	    }
	    status = AddPoint(*plot,lineptr,NULL,0);
	}
    } while(lineptr = strchr(lineptr+1,';'));
}

DoFileCommands(graph,plot,filename)
Graph	*graph;
Plot	**plot;
char	*filename;
{
FILE	*fp;
char	line[1001];

    /*
    ** open the data file associated with the plot
    */
    if((fp = fopen(filename,"r")) == NULL){
	printf("cant open file '%s'\n",filename);
	return;
    }
    while(!feof(fp)){
	/*
	** read a line from the data file
	*/
	if(fgets(line,1000,fp) == NULL) {
	    break;
	}
	line[1000] = '\0';
	DoCommand(graph,plot,line);
    }
    ClosePlot(*plot);
    fclose(fp);
}

Plot *PartialLoadPlot(graph,plot,startline,endline,startplot,endplot)
Graph	*graph;
Plot	*plot;
int	startline;
int	endline;
int	startplot;
int	endplot;
{
DataSource	*source;

 if (startline > 0 &&
     endline > 0 &&
     startline > endline){
   fprintf(stderr,"ERROR: Start spike (%d) is greater than End spike(%d) (PartialLoadPlot)\n",
	   startline, endline);
   return;
 }
    /*
    ** go through all of the data sources and perform the partial
    ** load on the matching one
    */
    for(source=graph->source;source;source=source->next){
	/*
	** is this the plot to be reloaded?
	*/
	if(source->plot == plot || source->plot==NULL){
	    /*
	    ** was it partially loaded?
	    */
	    if(source->loadmode != PARTIAL_LOAD){
		fprintf(stderr,"Plot is not in partial load mode\n");
		return(NULL);
	    } else {
		DeletePlot(plot);
		/*
		** set the desired source load parameters
		*/
		source->startline = startline;
		source->endline = endline;
		source->startplot = startplot;
		source->endplot = endplot;
		/*
		** set the suggested block size
		*/
		if((blocksize = endline - startline + 1) <= 0){
		    blocksize = BLOCKSIZE;
		}
		/*
		** and load the plot
		*/
		LoadPlotFromSource(graph,source);
		SetPlotDefaults(graph);
		/*
		** recompute cluster assignments
		*/
		TouchClusters(graph);
		AssignClusterPoints(graph);

		return(source->plot);
	    }
	}
    }
    return(NULL);
}

MenuPartialPlotLoad(item)
MenuItem	*item;
{
Plot	*p;
Plot	*newplot;
DataSource	*source;
int	stepsize;
int	i;
int	currentcolor;    

    p = SelectedPlot(G);
    if(p == NULL){
	return;
    }
    currentcolor = p->linestyle;
    if((source = p->source) == NULL){
	fprintf(stderr,"No data source found for this plot\n");
	return;
    }
    /*
    ** propagate the
    ** current state to the loaded plot
    */
    SetSourceDefaultsFromPlot(p,source);
    if(item->value){
	stepsize = Atoi(item->value);
    } else {
	stepsize = 1;
    }
    source->startplot = source->currentplot+stepsize-1;
    source->endplot = source->currentplot+stepsize-1;
    if(source->startplot <= 0) {
	/*
	** dont go past the beginning of the file
	*/
	return;
    }
    if(G->overlay){
	/*
	** redraw in the current color
	*/
	RefreshGraph(G);
    }
    /*
    ** load the new plot
    ** note that this updates the menu which is somewhat annoying
    */
    newplot = LoadPlotFromSource(G,source);
    if(newplot != NULL && newplot->npoints > 0){
/* 	/\* */
/* 	** note that this will override any color specification in */
/* 	** the plot file itself */
/* 	*\/ */
/* 	newplot->linestyle = source->defaults[FOREGROUND]; */
	DeletePlot(p);
	/*
	for(i=0;i<stepsize;i++){
	    DeletePlot(p);
	}
	*/
	/*
	** if overlay is set then redraw in the foreground color
	*/
	if(G->overlay){
	    newplot->linestyle = 0;
	}
	ScaleAndRefreshGraph(G);
	/*
	** and set the color to the previous value
	*/
	newplot->linestyle = currentcolor;
    } else {
	ScaleAndRefreshGraph(G);
    }
    /*
    ** this takes advantage of the way in which the plots are
    ** being loaded.  Each real plot starts off by creating a new
    ** plot.  This means that the plot automatically created when
    ** anything is loaded is empty and will just keep accumulating.
    ** So delete any empty plots after loading.
    */
    for(p=G->plot;p;p=p->next){
	if(p->npoints == 0){
	    DeletePlot(p);
	    break;
	}
    }
}

MenuPartialPlotReset(item)
MenuItem	*item;
{
Plot	*p;
Plot	*newplot;
DataSource	*source;

    p = SelectedPlot(G);
    if(p == NULL){
	return;
    }
    if((source = p->source) == NULL){
	fprintf(stderr,"No data source found for this plot\n");
	return;
    }
    source->startplot = 0;
    source->endplot = 1;
    newplot = LoadPlotFromSource(G,source);
    if(newplot != NULL && newplot->npoints > 0){
	DeletePlot(p);
    }
    ScaleAndRefreshGraph(G);
}

MenuPartialPlotRead(item)
MenuItem	*item;
{
    LoadPlot(G,item->value,PARTIAL_PLOT_LOAD,-1,-1,0,0);
    ScaleAndRefreshGraph(G);
}

/* GlobExpand:
**
** Perform shell 'globbing' (filename expansion) on string 'namein',
** returning expanded filename in 'nameout'.
**
** Returns 0 on success: Only one match, and it's a file, not a directory
** Returns 1 otherwise: nameout is not changed
**
** The same pointer can be passed in as namein and nameout. In this
** case, the original string will be replaced with the expanded
** filename.
*/

int GlobExpand(char *namein, char *nameout)
{
  glob_t	retglob;
  
  /* get the filename to load, using glob to perform shell expansion */
  /* GLOB_TILDE = use tilde expansion */
  /* GLOB_MARK = append slash to end of each path which is a directory */
  if (glob(namein,GLOB_TILDE|GLOB_MARK,NULL,&retglob) != 0){
    fprintf(stderr,"ERROR: No match for file '%s' (GlobExpand)\n",namein);
    return(1);
  }
  if (retglob.gl_pathc > 1){
    fprintf(stderr,"ERROR: multiple files match '%s' (GlobExpand)\n",namein);
    globfree(&retglob);
    return(1);
  }

  /* print match message if we had shell expansion */
  if(strcmp(retglob.gl_pathv[0], namein)){
    fprintf(stderr,"File '%s' matches '%s' (GlobExpand)\n",retglob.gl_pathv[0], namein);
  }

  /* test whether the returned path is a directory (is last char '/' ? */
  if (retglob.gl_pathv[0][strlen(retglob.gl_pathv[0])-1] == '/'){
    fprintf(stderr,"ERROR: '%s' matches a directory, not a file (GlobExpand)\n",namein);
    globfree(&retglob);
    return(1);
  }

  strcpy(nameout,retglob.gl_pathv[0]);

  globfree(&retglob);
  return(0);
}

LoadEpochFile(char *filename)
{
  char tmp[200];
  FILE	*fp;
  glob_t	retglob;
  
  int	i;
  
  char	line[1001];
  char	**header;
  char	*fieldstring;
  int	headersize;
  FieldInfo fieldinfo;

  /* epoch strings and corresponding columns in the epoch file */
  char  *estr[3];
  int	ecol[3];
  
  /* epoch string and column array indexes */
#define ENAME	0
#define ESTART	1
#define EEND	2

  /* line parsing vars */
  char	*ptr;
  char	*eptr;
  int done = 0;
  int n = 0;
  int nmax = 0;
  
  /* Hard coded because I don't know how to dynamically allocate memory yet (tjd) */
#define MAXPARAMS 50
#define MAXVALUELEN 50
  char params[MAXPARAMS][MAXVALUELEN];

  int	epoch;

  fprintf(stderr,"Trying to load epoch file '%s' ... (LoadEpochFile)\n",filename); /* to be continued with a 'done'*/
  
  /* if we can't expand the filename, return.*/
  if(GlobExpand(filename,tmp)) {
    return(1);
  }

  if((fp = fopen(tmp,"r")) == NULL){
    fprintf(stderr,"ERROR: Can't open file '%s' (LoadEpochFile)\n",filename);
    return(1);
  }

  header = ReadHeader(fp,&headersize);
  
  if(header){
    fieldstring = GetFieldString(header);
    
    switch(GetFileType(header)){
    case BINARY:
      fprintf(stderr,"ERROR: Binary epoch files not supported (LoadEpochFile)\n");
      return;
      
    case INVALID_TYPE:
      fprintf(stderr,"No filetype found in header.  Assuming ASCII.(LoadEpochFile)\n");
      /* note code continues to next case since there is no 'break' or 'return' */

    case ASCII:
      if (GetFieldInfoByName(fieldstring,"ep_name",&fieldinfo)){
	ecol[ENAME] = fieldinfo.column;
	if (GetFieldInfoByName(fieldstring,"ep_start",&fieldinfo)){
	  ecol[ESTART] = fieldinfo.column;
	  if (GetFieldInfoByName(fieldstring,"ep_end",&fieldinfo)){
	    ecol[EEND] = fieldinfo.column;
	    break; /* found all three fields */
	  }
	}
      }
      /* missing one or more fields */
      fprintf(stderr,"ERROR: Required fields (ep_end, ep_start, ep_end) not found: '%s' (LoadEpochFile)\n",fieldstring);
      return(1);
    } /* end of filetype switch */
  } else { /* No header in file: assume ASCII and default columns */
    fprintf(stderr,"No header found: Assuming ASCII format and using default columns for epoch file (name/start/end) (LoadEpochFile)\n");
    ecol[ENAME] = 0;
    ecol[ESTART] = 1;
    ecol[EEND] = 2;
  } 

  /* calculate nmax (highest field column we will need to read) */
  /* iterate over ecol array */
  for (i = 0; i < ((sizeof ecol)/sizeof(int)); i++) 
    if (ecol[i] > nmax)
      nmax = ecol[i];
  
  /* loop over each epoch slot available */
  /* epochs are 1-indexed for historical reasons */
  for(epoch=1; epoch <= NUM_EPOCHS; epoch++){
    if(feof(fp)){
      SetEpochValues(epoch,NULL,NULL,NULL);
      continue;
    }
    if(fgets(line,1000,fp) == NULL){
      SetEpochValues(epoch,NULL,NULL,NULL);
      continue;
    }
    
    /* null terminate the string, if it's not already */
    line[1000] = '\0';
    
    /* ignore comment lines (shouldn't see them outside of header?) */
    /* and empty lines */
    
    if(line[0] == '%' ||
       IsStringEmpty(line)) {
      
      /* don't advance epoch counter */
      epoch--;
      continue;
    }
    
    /* parse fields (adapted from Matt's sgetargs) */
    
    /* we allow white-space within fields (but issue a warning?),
       which must be tab separated */
    
    ptr = line;
    
    /* read one line of params */

    n = 0;
    while (!done){      
      /* end of the line ?*/
      if(*ptr == '\0') break;
      
      /* find end of parameter (allow spaces) */
      for(eptr = ptr;
	  (*eptr != '\0') && (*eptr != '\t') && (*eptr != '\n');
	  eptr++);
	
      /* is end of parameter == end of line */
      if(*eptr == '\0'){
	done = 1;
      }
	
      /* copy parameter to values array */

      if (n < MAXPARAMS-1 && 
	  eptr-ptr < MAXVALUELEN-1){ /* hack to avoid dynamic memory
					allocation (see #defines
					above) */
	strncpy(params[n], ptr, eptr-ptr);
	params[n][eptr-ptr] = '\0';
      } else{
	fprintf(stderr,"ERROR: Epoch string too long: field %d of '%s'\n", n, line );
	fclose(fp);
	return(1);
      }

      /* have we read all parameters we need? */
      n++;
      if(n > nmax) break;
	
      /* advance to the next parameter */
      ptr = eptr+1;
    } /* finished reading the params from this line */
    
    /* did we read enough params? */
    if (n <= nmax){
      fprintf(stderr,"ERROR: could only read %d of %d needed fields (LoadEpochFile)\n",n,nmax+1);
      return(1);
    }
    /* assign correct params to estr array (by looking up param column in ecol) */
    for (i = 0; i < (sizeof estr/sizeof(char*)); i++){
      estr[i] = params[ecol[i]];
    }
    
    /* test whether estart and eend are parseabe time strings (don't change format, though) */
    if (!ParseTimestamp(estr[ESTART])){
      fprintf(stderr,
	      "ERROR: epoch %d start time not valid: '%s' (LoadEpochFile)\n",
	      epoch,
	      estr[ESTART]);
      return(1);
    }
    if (!ParseTimestamp(estr[EEND])){
      fprintf(stderr,
	      "ERROR: epoch %d end time not valid: '%s' (LoadEpochFile)\n",
	      epoch,
	      estr[EEND]);
      return(1);
    }

    /* Add values to slot 'epoch' in list */
    SetEpochValues(epoch,estr[ENAME], estr[ESTART], estr[EEND]);
    
  }/* finished iterating over epochs */
  
  /* are there more non-blank, non-comment lines in the file than we
     could load? */
  
  while (!feof(fp)){
    if(fgets(line,1000,fp) == NULL)
      break;
    
    /* null terminate the string, if it's not already */
    line[1000] = '\0';
    
    if(!(line[0] == '%') &&
       !(IsStringEmpty(line))){
      
      /* not EOF, not a comment, not a blank line */
      fprintf(stderr,
	      "Warning: Possibly more epochs specified than epoch "
	      "slots available in xclust (LoadEpochFile)\n");
      break;
    }
  } /* done checking for extra lines at end of epoch file */

  /* finish up */

  fprintf(stderr," ...done. (LoadEpochFile)\n");
  fclose(fp);

} /* End LoadEpochFile */



SaveEpochFile(char *file)
{

  FILE *fp;
  char *epoch_name;
  char *epoch_tstart;
  char *epoch_tend;
  char itemstr[100];
  char comstr[100];
  int i;
    /*
    ** backup the previous epoch file
    */
    sprintf(comstr,"/bin/cp %s %s~",file,file);
    system(comstr);
    /*
    ** open the file
    */
    fprintf(stderr,"Writing epoch file '%s' (backup saved to %s~)\n",
	file,file);
    if((fp = fopen(file,"w")) == NULL){
	fprintf(stderr,"ERROR: unable to write epochs file '%s'\n",file);
	return;
    }
    BeginStandardHeader(fp,standardargc,standardargv,VERSION);
    fprintf(fp,"%% File type:\tAscii\n");
    fprintf(fp,"%% File contents:\tepoch definitions\n");
    fprintf(fp,"%% Fields:\tep_name\tep_start\tep_end\n");
    EndStandardHeader(fp);

    for(i = 1; i <= NUM_EPOCHS;	i++){
      sprintf(itemstr,"/epochmenu/epoch/%d",i);
      epoch_name = GetItemValue(itemstr);
      sprintf(itemstr,"/epochmenu/epochstart/%d",i);
      epoch_tstart = GetItemValue(itemstr);
      sprintf(itemstr,"/epochmenu/epochend/%d",i);
      epoch_tend = GetItemValue(itemstr);
      if (epoch_name==NULL)
	epoch_name = "";;
      if (epoch_tstart==NULL)
	epoch_tstart = "";
      if (epoch_tend==NULL)
	epoch_tend="";

      if (strlen(epoch_name)>0 || strlen(epoch_tstart)>0 || strlen(epoch_tend)>0)
	fprintf(fp, "%s\t%s\t%s\n", epoch_name, epoch_tstart, epoch_tend);
      else /* empty epoch definition */
	continue;
    }

    fclose(fp);
    fprintf(stderr,"done\n");

}

