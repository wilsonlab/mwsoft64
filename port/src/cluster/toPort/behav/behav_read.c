#include "behav_ext.h"

int ReadAngularRangeMap(result)
Result	*result;
{
int	headersize;
char	**header;
char	*xysize;
int	xsize;
int	ysize;
int	x,y;
int	mintheta,maxtheta;
int	i;
char	line[1000];

    /*
    ** read in the standard header
    */
    header = ReadHeader(result->fpangularrange,&headersize);
    /*
    ** get the xy dimensions for the range grid from the header
    */
    if((xysize = GetHeaderParameter(header,"XYsize:")) == NULL){
	fprintf(stderr,"ERROR: unable to determine angular range grid size from header\n");
	return(0);
    }
    if(sscanf(xysize,"%d%d",&xsize,&ysize) != 2){
	fprintf(stderr,"ERROR: unable to read grid size from XYsize header parameter\n");
	return(0);
    }
    /*
    ** allocate the angular range map
    */
    result->angular_range_map = 
	(AngularRangeMap *)calloc(1,sizeof(AngularRangeMap));
    result->angular_range_map->xsize = xsize;
    result->angular_range_map->ysize = ysize;
    result->angular_range_map->grid = 
	(MinMaxTheta **)calloc(ysize,sizeof(MinMaxTheta *));
    for(i=0;i<ysize;i++){
	result->angular_range_map->grid[i] = 
	(MinMaxTheta *)calloc(xsize,sizeof(MinMaxTheta));
    }
    /*
    ** read in the range values
    */
    while(!feof(result->fpangularrange)){
	if(fgets(line,1000,result->fpangularrange) == NULL) break;
	if(line[0] == '%') continue;
	if(line[0] == '/') continue;
	if(sscanf(line,"%d%d%d%d",&x,&y,&mintheta,&maxtheta) != 4) continue;
	/*
	** fill the grid location
	*/
	if((x >= 0) && (x < xsize) && (y >= 0) && (y < ysize)){
	    result->angular_range_map->grid[x][y].mintheta = mintheta;
	    result->angular_range_map->grid[x][y].maxtheta = maxtheta;
	} else {
	    fprintf(stderr,
	    "ERROR: ignoring invalid xy position (%d,%d) in angular range file\n",x,y);
	}
    }
    return(1);
}

int ReadRange(result)
Result		*result;
{
TRange	*trange;
int	headersize;
FILE	*fp;
char	line[201];
unsigned long	tstart;
unsigned long	tend;
int	count;
int	binaryformat;
char	startstr[30];
char	endstr[30];
char	ef0[10];
char	ef1[10];
char	*filetype;
char	**header;
int	convert;

    if((fp = result->fptrange) == NULL) return(0);
    /*
    ** rewind the file
    */
    fseek(fp,0L,0L);
    /*
    ** read the trange file header
    */
    header = ReadHeader(fp,&headersize);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
	if(verbose)
	fprintf(stderr,"No data conversion file from %s architecture.\n",
	    GetFileArchitectureStr(header));
    } else {
	convert = 1;
	if(verbose)
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
    /*
    ** read the timestamp ranges
    */
    count = 0;
    while(!feof(fp)){
	if(binaryformat){
	    /*
	    ** read the starting timestamp
	    */
	    if(fread(&tstart,sizeof(unsigned long),1,fp) != 1){
		break;
	    }
	    if(convert){
		ConvertData(&tstart,sizeof(unsigned long));
	    }
	    /*
	    ** read the ending timestamp
	    */
	    if(fread(&tend,sizeof(unsigned long),1,fp) != 1){
		break;
	    }
	    if(convert){
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
	/*
	** fill the trange data structure
	*/
	if((trange = (TRange *)calloc(1,sizeof(TRange))) == NULL){
	    fprintf(stderr,"ERROR: memory allocation error\n");
	    exit(-1);
	};
	trange->tstart = tstart;
	trange->tend = tend;
	/*
	** insert it into the trange list
	*/
	trange->next = result->trange;
	result->trange = trange;
	count++;
    }
    fclose(fp);
    return(count);
}

int LoadPositions(plist)
PositionList	*plist;
{
int	count;
char	**headercontents;
int	fsize;
unsigned long timestamp;
unsigned long ptimestamp;
int	out_of_order;
int	dformat;
int	p2format;
char	*typestr;
unsigned char tmpxy[4];
int	convert;

    /*
    ** flag indicating a fixdir output type
    */
    dformat = 0;
    p2format = 0;
    /*
    ** count the number of spikes
    */
    plist->npositions = 0;
    plist->nprocessedpositions = 0;
    /*
    ** rewind the position file
    */
    fseek(plist->fp,0L,0L);
    /*
    ** read the header and determine the header size
    */
    headercontents = ReadHeader(plist->fp,&plist->headersize);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(headercontents)) ||
	(GetFileArchitecture(headercontents) == ARCH_UNKNOWN)) {
	convert = 0;
	if(verbose)
	fprintf(stderr,"No data conversion file from %s architecture.\n",
		GetFileArchitectureStr(headercontents));
    } else {
	convert = 1;
	if(verbose)
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	    GetFileArchitectureStr(headercontents),
	    GetLocalArchitectureStr());
    }

    if((typestr = GetHeaderParameter(headercontents,"Record type:")) != NULL){
	/*
	** look for the D type
	*/
	if(strncmp(typestr,"D",strlen("D")) == 0){
	    dformat = 1;
	} else {
	    dformat = 0;
	}
	/*
	** look for the T type
	*/
	if(strncmp(typestr,"T",strlen("T")) == 0){
	    p2format = 1;
	} else {
	    p2format = 0;
	}
    }
    /*
    if(verbose){
	DisplayHeader(stderr,headercontents,plist->headersize);
    }
    */

    /*
    ** determine the number of positions in the file based on the
    ** file size divided by the size of a position record
    **		timestamp	unsigned long
    **		front diode xy	2 unsigned chars
    **		back diode xy	2 unsigned chars
    */
    fseek(plist->fp,0L,2L);
    fsize = ftell(plist->fp);
    if(p2format){
	plist->npositions = (fsize - plist->headersize)/
	    (sizeof(unsigned long) + 4*sizeof(short));
    } else {
	plist->npositions = (fsize - plist->headersize)/
	    (sizeof(unsigned long) + 4*sizeof(unsigned char));
    }

#ifdef OLD
    while(!feof(plist->fp)){		/* scan the entire file */
	if(fread(&p.timestamp,sizeof(unsigned long),1,plist->fp) != 1){
	    break;
	}
	if(fread(p.xy,sizeof(unsigned char),4,plist->fp) != 4){
	    break;
	}
	plist->npositions++;
    }
#endif
    /*
    ** allocate the position array
    */
    if((plist->positionarray = (Position *)
	malloc(plist->npositions*sizeof(Position))) == NULL){
	    fprintf(stderr,"ERROR: unable to allocate %d bytes of memory for position array\n",
	    plist->npositions*sizeof(Position));
	}
    /*
    ** and read in the positions
    */
    count = 0;
    /*
    ** seek to the first position in the file
    */
    fseek(plist->fp,plist->headersize,0L);
    ptimestamp = 0;
    out_of_order = 0;
    while(!feof(plist->fp)){		
	/*
	** read in the timestamp for the position record
	*/
	if(fread(&timestamp,sizeof(unsigned long),1,plist->fp) != 1){
	    break;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	/*
	** make sure that there are no discrepancies between
	** the computed size and the current size
	*/
	if(count >= plist->npositions){
	    fprintf(stderr,
	    "ERROR: discrepancy between computed and actual position file size\n");
	    break;
	}
	plist->positionarray[count].timestamp = timestamp;
	/*
	** check for proper timestamp order
	*/
	if(ptimestamp > 0 && (ptimestamp > timestamp)){
	    out_of_order = 1;
	}
	if(p2format){
	    /*
	    ** read in the front and back diode positions
	    */
	    if(fread(plist->positionarray[count].xy,
	    sizeof(short),4,plist->fp) != 4){
		break;
	    }
	    if(convert){
		ConvertData(&plist->positionarray[count].xy[0],sizeof(short));
		ConvertData(&plist->positionarray[count].xy[1],sizeof(short));
		ConvertData(&plist->positionarray[count].xy[2],sizeof(short));
		ConvertData(&plist->positionarray[count].xy[3],sizeof(short));
	    }
	} else {
	    /*
	    ** read in the front and back diode positions
	    */
	    if(fread(tmpxy,sizeof(unsigned char),4,plist->fp) != 4){
		break;
	    }
	    plist->positionarray[count].xy[0] = tmpxy[0];
	    plist->positionarray[count].xy[1] = tmpxy[1];
	    plist->positionarray[count].xy[2] = tmpxy[2];
	    plist->positionarray[count].xy[3] = tmpxy[3];
	    if(dformat){
		/*
		** read in the direction
		*/
		if(fread(&plist->positionarray[count].direction,
		sizeof(int),1,plist->fp) != 1){
		    break;
		}
		if(convert){
		    ConvertData(&plist->positionarray[count].direction,
		    sizeof(int));
		}
	    }
	}
	count++;
	ptimestamp = timestamp;
    }
    return(out_of_order);
}

void LoadSpikes(result,spike,timeshift)
Result	*result;
SpikeList *spike;
int	timeshift;
{
unsigned long	timestamp;
int	count;
int	fsize;
char	**header;
int	convert;
char	*tstr;

    /*
    ** count the number of spikes
    */
    spike->nspikes = 0;
    /*
    ** rewind the spike file
    */
    fseek(spike->fp,0L,0L);
    /*
    ** read the header and get the header size
    */
    header = ReadHeader(spike->fp,&spike->headersize);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
	if(verbose)
	fprintf(stderr,"No data conversion file from %s architecture.\n",
		GetFileArchitectureStr(header));
    } else {
	convert = 1;
	if(verbose)
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	    GetFileArchitectureStr(header),
	    GetLocalArchitectureStr());
    }
    spike->convert = convert;
    if(result->useheaderrange){
	/*
	** look for time range information
	*/
	if((tstr = GetHeaderParameter(header,"Start time:")) == NULL){
	    fprintf(stderr,"ERROR: unable to determine start time from header\n");
	} else {
	    spike->tstart = ParseTimestamp(tstr);
	}
	if((tstr = GetHeaderParameter(header,"End time:")) == NULL){
	    fprintf(stderr,"ERROR: unable to determine end time from header\n");
	    return;
	} else {
	    spike->tend = ParseTimestamp(tstr);
	}
	starttime = spike->tstart;
	endtime = spike->tend;
    }

    /*
    ** determine the number of spikes in the file based on the
    ** file size
    */
    fseek(spike->fp,0L,2L);
    fsize = ftell(spike->fp);
    spike->nspikes = (fsize - spike->headersize)/(sizeof(unsigned long));
#ifdef OLD
    while(!feof(spike->fp)){		/* scan the entire file */
	if(fread(&timestamp,sizeof(unsigned long),1,spike->fp) != 1){
	    break;
	}
	spike->nspikes++;
    }
#endif
    /*
    ** allocate the spike array
    */
    spike->data = (SpikeData *)malloc(spike->nspikes*sizeof(SpikeData));
    /*
    ** read them in
    */
    count = 0;
    fseek(spike->fp,spike->headersize,0L);		/* rewind spike file */
    while(!feof(spike->fp)){		/* scan the entire file */
	if(fread(&timestamp,sizeof(unsigned long),1,spike->fp)!=1){
	    break;
	}
	if(convert){
	    ConvertData(&timestamp,sizeof(unsigned long));
	}
	/*
	** make sure that there are no discrepancies between
	** the computed size and the current size
	*/
	if(count >= spike->nspikes){
	    fprintf(stderr,
	    "ERROR: discrepancy between computed and actual spike file size\n");
	    break;
	}
	spike->data[count].timestamp = timestamp + timeshift;
	/*
	** this is not the correct spike index
	spike->data[count].index = count;
	*/
	count++;
    }
}

void ScanSpikes(spike)
SpikeList *spike;
{

    /*
    ** go to the location of the starting spike
    */
    fseek(spike->fp,spike->headersize + startspike*sizeof(unsigned long),0L);
    if(fread(&starttime,sizeof(unsigned long),1,spike->fp) != 1){
	fprintf(stderr,"ERROR: unable to find spike number %d\n",startspike);
    }
    if(spike->convert){
	ConvertData(&starttime,sizeof(unsigned long));
    }
    /*
    ** go to the location of the end spike
    */
    fseek(spike->fp,spike->headersize + endspike*sizeof(unsigned long),0L);
    if(fread(&endtime,sizeof(unsigned long),1,spike->fp) != 1){
	fprintf(stderr,"ERROR: unable to find spike number %d\n",endspike);
    }
    if(spike->convert){
	ConvertData(&endtime,sizeof(unsigned long));
    }
}

void LoadAndSortPositions(result,plist)
Result		*result;
PositionList	*plist;
{
int	sort_status;
unsigned long firsttimestamp;
unsigned long lasttimestamp;

    sort_status = LoadPositions(plist);
    if(sort_status != 0){
	/*
	** sort by timestamp
	*/
	if(verbose){
	    fprintf(stderr,"sorting...");
	}
	qsort((void *)plist->positionarray,plist->npositions,
	sizeof(Position),positioncompare);
    }
    if(verbose) {
	firsttimestamp = plist->positionarray[0].timestamp;
	lasttimestamp = plist->positionarray[plist->npositions-1].timestamp;
	fprintf(stderr,"First timestamp in position file: %ld (%s)\n",
	    firsttimestamp,TimestampToString(firsttimestamp));
	fprintf(stderr,"Last timestamp in position file: %ld (%s)\n",
	    lasttimestamp,TimestampToString(lasttimestamp));
	fprintf(stderr,"Positions offset by %d,%d\n",
	result->xoffset,result->yoffset);
    }
}

void LoadAndSortSpikes(result,spikearray)
Result	*result;
SpikeList	*spikearray;
{
int	i;

    /*
    ** read in spike times for each cluster
    */
    for(i=0;i<result->nclusters;i++){
	if(verbose){
	    fprintf(stderr,"Reading spike cluster %s...",
	    spikearray[i].fname);
	}
	LoadSpikes(result,&spikearray[i],spikearray[i].timeshift + result->timeshift);
	/*
	** sort by timestamp
	*/
	qsort(spikearray[i].data,spikearray[i].nspikes,
	sizeof(SpikeData),datacompare);
	if(verbose){
	    fprintf(stderr,"sorting...");
	}
	if(verbose){
	    fprintf(stderr,"Read %d spikes\n",spikearray[i].nspikes);
	    fprintf(stderr,"from timestamp %lu (%s) ",
		spikearray[i].data[0].timestamp,
		TimestampToString(spikearray[i].data[0].timestamp));
	    fprintf(stderr,"to %lu (%s)\n",
	    spikearray[i].data[spikearray[i].nspikes-1].timestamp,
	    TimestampToString(spikearray[i].data[spikearray[i].nspikes-1].timestamp));

	}
    }
}
