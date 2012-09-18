/*
*******************************************************************
PROGRAM:
    behav

DESCRIPTION:
    computes positional and directional correlates to unit activity
    from one or more unit clusters. Can look for time-shifted 
    correlations between unit activity and behavior.

AUTHOR:
    Written by Matthew Wilson
    ARL Division of Neural Systems, Memory, and Aging
    University of Arizona
    Tucson, AZ 85724
    wilson@nsma.arizona.edu

DATES:
    original program 5/91
    program update 12/91
*******************************************************************
*/
#include "behav_ext.h"

/*
*************************************
**            GLOBALS              **
*************************************
*/

int		verbose;
unsigned long	starttime;
unsigned long	endtime;
int		startspike;
int		endspike;
int		inf;
int		min_rad;
int		max_rad;
int		max_consecutive_miss;
int		maxlookahead;
int		maxgaplen;

/*
*************************************
**              MAIN               **
*************************************
*/

int main(argc,argv)
int argc;
char **argv;
{
int		nxtarg;
int		i;
SpikeList	spikearray[MAXCLUSTERS];
PositionList	plist;
Result		result;
int		count;
FILE		*fptmp;
int             xsizeset = 0;
int             ysizeset = 0;
 

    nxtarg = 0;
    plist.fp = NULL;
    bzero(&result,sizeof(Result));
    result.fp = NULL;
    result.fpreport = NULL;
    result.fpdir = NULL;
    result.fpvar = NULL;
    result.fpbound = NULL;
    result.binsize = 0;
    result.dsize = MAXTHETA;
    result.xres = XPOSITION_RESOLUTION;
    result.yres = YPOSITION_RESOLUTION;
    result.xsize = XPOSITION_RESOLUTION;
    result.ysize = YPOSITION_RESOLUTION;
    result.mintheta = 0;
    result.maxtheta = MAXTHETA;
    result.savemode = GRID;
    result.behavmode = POSITION;
    result.pinterval = POSITION_INTERVAL;
    result.format = BINARY;
    result.spatial_measure = EVENTS;
    result.occradius = 12;
    result.xviewformat = 1;
    result.occdirangle = DIRBINSIZE;
    result.upperthresh = -1;
    result.lowerthresh = -1;
    result.normalize = 1;
    result.resulttype = CORR_RESULT;
    result.rotation = GRID_ROTATION;
    result.nclusters = 0;
    result.positioneval = FRONT;
    result.plotmode = LINEAR;
    result.smooth = 0;
    result.enable_occdirection_change = 0;
    result.directionline = 0;
    result.minrate = MINRATE;
    result.minpsamples = 0;
    result.hasbounds = 0;
    result.ignore_missing_front_diode = 0;
    result.ignore_missing_back_diode = 1;
    result.timeshift = 0;
    result.minvelocity = -1;
    result.maxvelocity = -1;
    result.fieldthresh = 0.5;
    result.velspan = 20;
    result.noflip = 1;
    result.ascii = 0;
    starttime = 0;
    endtime = 0;
    startspike = 0;
    endspike = 0;
    inf = INF;
    verbose = VERBOSE;
    maxlookahead = LOOKAHEAD;
    min_rad = DEF_MIN_RADIUS;
    max_rad = DEF_MAX_RADIUS;
    result.useheaderrange  = 0;
    max_consecutive_miss = DEF_MAX_CONSEC_MISS;
    bzero(spikearray,MAXCLUSTERS*sizeof(SpikeList));
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,
	    "usage: %s pfile [options]\n",
	    argv[0]);
	    fprintf(stderr,"\tpfile\t\t position file (required)\n");
	    fprintf(stderr,"Available Options:\n");
	    fprintf(stderr,
	    "\t[-v]\t\t verbose operation mode (recommended)\n");
	    fprintf(stderr,
	    "\t[-bin msec]\t output time bin size\n");
	    fprintf(stderr,
	    "\t[-tshift time]\t shift all spike times\n");
	    fprintf(stderr,
	    "\t[-centroid]\t compute position by averaging front and back diodes \n");
	    fprintf(stderr,
	    "\t[-ignorefront]\t process positions with missing front diodes \n");
	    fprintf(stderr,
	    "\t[-ignoreback]\t process positions with missing back diodes \n");
	    fprintf(stderr,
	    "\t[-requireback]\t dont process positions with missing back diodes \n");
	    fprintf(stderr,
	    "\t[-t file]\t timestamp file\n");
	    fprintf(stderr,
	    "\t[-ts file time]\t timestamp file with a time shift\n");
	    fprintf(stderr,
	    "\t[-o file]\t output file\n");
	    fprintf(stderr,
	    "\t[-grid]\t\t specify output as a grid\n");
	    fprintf(stderr,
	    "\t[-xysize num]\t give the grid xy dimension (e.g. 64 -> 64x64 grid)\n");
	    fprintf(stderr,
	    "\t[-xsize num]\t give the grid x dimension alone\n");
	    fprintf(stderr,
	    "\t[-ysize num]\t give the grid y dimension alone\n");
	    fprintf(stderr,
	    "\t[-dsize #][-dvector][-dpolar][-vgrid]\n");
	    fprintf(stderr,
	    "\t[-dgrid mintheta maxtheta][events][-ithresh low hi][-intervals]\n");
	    fprintf(stderr,
	    "\t[-vo variancefile][-inf infinity (default 0)][-zerofiring][-nonzerofiring]\n");
	    fprintf(stderr,
	    "\t[-do dirfile][-oo occupancyfile][-normalize 0/1]\n");
	    fprintf(stderr,
	    "\t[-xoffset pixels][-yoffset pixels][-occlimit msec][-occradius pixels]\n");
	    fprintf(stderr,
	    "\t[-occgrid][-occrategrid][-correlations][-samplebias]\n");
	    fprintf(stderr,
	    "\t[-maxgap n][-radtest min max][-look maxlookahead][-occdirchange]\n");
	    fprintf(stderr,
	    "\t[-flip][-minvel v][-maxvel v][-tstart timestamp][-tend timestamp]\n");
	    fprintf(stderr,
	    "\t[-velspan npsamples][-sstart spike#][-send spike#]\n");
	    fprintf(stderr,
	    "\t[-bound x1 y1 x2 y2]\t bounding region in xysize coordinate\n");
	    fprintf(stderr,
	    "\t[-posint interval]\t position frame sample interval\n");
	    fprintf(stderr,
	    "\t[-xres x][-yres y]\t x and y resolution of tracker image\n");
	    fprintf(stderr,
	    "\t[-cbfile cbfile]\t bounding region from a cluster bounds file\n");
	    fprintf(stderr,
	    "\t[-boundfile toutfile]\n");
	    fprintf(stderr,
	    "\t[-angularrangemap file][-xview][-noxview][-ro ratefile]\n");
	    fprintf(stderr,
	    "\t[-fo fixedpositionfile][-smooth distance][-minrate rate]\n");
	    fprintf(stderr,
	    "\t[-trangefile timestamp_range_file]\n");
	    fprintf(stderr,
	    "\t[-ascii]\t output firing rate maps in ascii format\n");
	    fprintf(stderr,
	    "\t[-directionline]\n");
	    fprintf(stderr,
		"\t[-psamples]\t output total number of position samples\n");
	    fprintf(stderr,
		"\t\t\t as a function of xy location\n");
	    fprintf(stderr,
		"\t[-minpsamples n]\t only process data for a location when\n");
	    fprintf(stderr,
		"\t\t\t number of position samples > n\n");
	    fprintf(stderr,"\t[-reporto]\t output behavioral spike stats to file\n");
	    fprintf(stderr,"\t[-fieldthresh value]\t firing rate thresh for field size calculation\n");
	    exit(0);
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
	if(strcmp(argv[nxtarg],"-flip") == 0){
	    result.noflip = 0;
	} else
	if(strcmp(argv[nxtarg],"-ascii") == 0){
	    result.ascii = 1;
	} else
	if(strcmp(argv[nxtarg],"-xview") == 0){
	    result.xviewformat = 1;
	} else
	if(strcmp(argv[nxtarg],"-noxview") == 0){
	    result.xviewformat = 0;
	} else
	if(strcmp(argv[nxtarg],"-events") == 0){
	    result.spatial_measure = EVENTS;
	} else
	if(strcmp(argv[nxtarg],"-intervals") == 0){
	    result.spatial_measure = INTERVALS;
	} else
	if(strcmp(argv[nxtarg],"-velspan") == 0){
	    result.velspan = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-ithresh") == 0){
	    result.spatial_measure = INTERVALTHRESH;
	    result.lowerthresh = atoi(argv[++nxtarg]);
	    result.upperthresh = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-ignorefront") == 0){
	    result.ignore_missing_front_diode = 1;
	} else
	if(strcmp(argv[nxtarg],"-ignoreback") == 0){
	    result.ignore_missing_back_diode = 1;
	} else
	if(strcmp(argv[nxtarg],"-requireback") == 0){
	    result.ignore_missing_back_diode = 0;
	} else
	if(strcmp(argv[nxtarg],"-dvector") == 0){
	    result.savemode = VECTOR;
	    result.behavmode = DIRECTION;
	    result.format = ASCII;
	} else
	if(strcmp(argv[nxtarg],"-dpolar") == 0){
	    result.savemode = VECTOR;
	    result.behavmode = DIRECTION;
	    result.plotmode = POLAR;
	    result.format = ASCII;
	} else
	if(strcmp(argv[nxtarg],"-grid") == 0){
	    result.savemode = GRID;
	    result.behavmode = POSITION;
	    result.format = BINARY;
	    result.mintheta = 0;
	    result.maxtheta = MAXTHETA;
	} else
	if(strcmp(argv[nxtarg],"-dgrid") == 0){
	    result.savemode = GRID;
	    result.behavmode = DIRECTION;
	    result.format = BINARY;
	    result.mintheta = atof(argv[++nxtarg]);
	    result.maxtheta = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-vgrid") == 0){
	    result.savemode = GRID;
	    result.behavmode = VELOCITY;
	    result.format = BINARY;
	    result.resulttype = CUM_RESULT;
	} else
	if(strcmp(argv[nxtarg],"-angvgrid") == 0){
	    result.savemode = GRID;
	    result.behavmode = ANGVELOCITY;
	    result.format = BINARY;
	    result.resulttype = CUM_RESULT;
	} else
	if(strcmp(argv[nxtarg],"-vout") == 0){
	    result.savemode = RAW;
	    result.behavmode = VELOCITY;
	    result.format = ASCII;
	} else
	if(strcmp(argv[nxtarg],"-centroid") == 0){
	    result.positioneval = CENTROID;
	} else
	if(strcmp(argv[nxtarg],"-angvout") == 0){
	    result.savemode = RAW;
	    result.behavmode = ANGVELOCITY;
	    result.format = ASCII;
	} else
	if(strcmp(argv[nxtarg],"-v") == 0){
	    verbose = 1;
	} else
	if(strcmp(argv[nxtarg],"-directionline") == 0){
	    result.directionline = 1;
	} else
	if(strcmp(argv[nxtarg],"-psamples") == 0){
	    result.resulttype = POSITION_SAMPLES;
	} else
	if(strcmp(argv[nxtarg],"-occdirchange") == 0){
	    result.enable_occdirection_change = 1;
	} else
	if(strcmp(argv[nxtarg],"-occgrid") == 0){
	    result.resulttype = OCC_RESULT;
	} else
	if(strcmp(argv[nxtarg],"-occrategrid") == 0){
	    result.resulttype = OCCRATE_RESULT;
	} else
	if(strcmp(argv[nxtarg],"-samplebias") == 0){
	    result.resulttype = SAMPLEBIAS_RESULT;
	} else
	if(strcmp(argv[nxtarg],"-correlations") == 0){
	    result.resulttype = CORR_RESULT;
	} else
	if(strcmp(argv[nxtarg],"-reliability") == 0){
	    result.resulttype = RELIABILITY_RESULT;
	} else
	if(strcmp(argv[nxtarg],"-nonzerofiring") == 0){
	    result.resulttype = NONZFIRING_RESULT;
	} else
	if(strcmp(argv[nxtarg],"-zerofiring") == 0){
	    result.resulttype = ZFIRING_RESULT;
	} else
	if(strcmp(argv[nxtarg],"-occradius") == 0){
	    result.occradius  = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-minrate") == 0){
	    result.minrate = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-fieldthresh") == 0){
	    result.fieldthresh = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-xysize") == 0){
	    result.ysize  = 
	    result.xsize  = atoi(argv[++nxtarg]);
	    xsizeset = 1;
	    ysizeset = 1;
	} else
	if(strcmp(argv[nxtarg],"-inf") == 0){
	    inf  = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-useheaderrange") == 0){
	    result.useheaderrange  = 1;
	} else
	if(strcmp(argv[nxtarg],"-radtest") == 0){
	    min_rad  = atoi(argv[++nxtarg]);
	    max_rad  = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-sstart") == 0){
	    startspike  = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-send") == 0){
	    endspike  = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-tstart") == 0){
	    starttime = ParseTimestamp(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-tend") == 0){
	    endtime = ParseTimestamp(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-xoffset") == 0){
	    result.xoffset  = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-yoffset") == 0){
	    result.yoffset  = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-xsize") == 0){
	    result.xsize  = atoi(argv[++nxtarg]);
	    xsizeset = 1;
	} else
	if(strcmp(argv[nxtarg],"-ysize") == 0){
	    result.ysize  = atoi(argv[++nxtarg]);
	    ysizeset = 1;
	} else
	if(strcmp(argv[nxtarg],"-dsize") == 0){
	    result.dsize  = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-minvel") == 0){
	    result.minvelocity  = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-maxvel") == 0){
	    result.maxvelocity  = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-bin") == 0){
	    result.binsize  = 10*atof(argv[++nxtarg]);/* convert to 100 usec */
	} else
	if(strcmp(argv[nxtarg],"-look") == 0){
	    maxlookahead  = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-smooth") == 0){
	    result.smooth  = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-occlimit") == 0){
	    /* convert to 100 usec */
	    result.limited_occupancy = 10*atoi(argv[++nxtarg]); 
	} else
	if(strcmp(argv[nxtarg],"-normalize") == 0){
	    result.normalize = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-maxgap") == 0){
	    max_consecutive_miss = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-minpsamples") == 0){
	    result.minpsamples = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-tshift") == 0){
	    ++nxtarg;
	    if(argv[nxtarg][0] == '-'){
		result.timeshift = 
		-ParseTimestamp(&(argv[nxtarg][1]));
	    } else {
		result.timeshift = 
		ParseTimestamp(argv[nxtarg]);
	    }
	} else
	if(strcmp(argv[nxtarg],"-t") == 0){
	    if(result.nclusters >= MAXCLUSTERS){
		fprintf(stderr,
		"ERROR: Exceeded maximum number of spike clusters (%d)\n",
		MAXCLUSTERS);
		exit(0);
	    }
	    if((spikearray[result.nclusters].fp = 
	    fopen(argv[++nxtarg],"r")) == NULL){
		fprintf(stderr,"ERROR: unable to open spiketime file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	    spikearray[result.nclusters].timeshift  = 0;
	    spikearray[result.nclusters].fname= argv[nxtarg];
	    result.nclusters++;
	} else
	if(strcmp(argv[nxtarg],"-ts") == 0){
	    if(result.nclusters >= MAXCLUSTERS){
		fprintf(stderr,
		"ERROR: Exceeded maximum number of spike clusters (%d)\n",
		MAXCLUSTERS);
		exit(0);
	    }
	    if((spikearray[result.nclusters].fp = 
	    fopen(argv[++nxtarg],"r")) == NULL){
		fprintf(stderr,"ERROR: unable to open spiketime file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	    spikearray[result.nclusters].fname= argv[nxtarg];
	    ++nxtarg;
	    if(argv[nxtarg][0] == '-'){
		spikearray[result.nclusters].timeshift  = 
		-ParseTimestamp(&(argv[nxtarg][1]));
	    } else {
		spikearray[result.nclusters].timeshift  = 
		ParseTimestamp(argv[nxtarg]);
	    }
	    result.nclusters++;
	} else
	if(strcmp(argv[nxtarg],"-trangefile") == 0){
	    result.fptrangename = argv[++nxtarg];
	    if((result.fptrange = fopen(result.fptrangename,"r")) == NULL){
		fprintf(stderr,"ERROR: unable to open timestamp range file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	} else
	if(strcmp(argv[nxtarg],"-angularrangemap") == 0){
	    result.fpangularrangename = argv[++nxtarg];
	    result.savemode = GRID;
	    result.behavmode = DIRECTION;
	    result.format = BINARY;
	    if((result.fpangularrange = fopen(result.fpangularrangename,"r")) == NULL){
		fprintf(stderr,"ERROR: unable to open angular range file '%s' \n",argv[nxtarg]);
		exit(0);
	    }
	} else
	if(strcmp(argv[nxtarg],"-reporto") == 0){
	    if((result.fpreport = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open report file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	} else
	if(strcmp(argv[nxtarg],"-fo") == 0){
	    result.fpfixname = argv[++nxtarg];
	    if((result.fpfix = fopen(result.fpfixname,"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open fixed position output file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	} else
	if(strcmp(argv[nxtarg],"-ro") == 0){
	    result.fpratename = argv[++nxtarg];
	    if((result.fprate = fopen(result.fpratename,"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open rate output file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	} else
	if(strcmp(argv[nxtarg],"-vo") == 0){
	    result.fpvarname = argv[++nxtarg];
	    if((result.fpvar = fopen(result.fpvarname,"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open variance output file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	} else
	if(strcmp(argv[nxtarg],"-boundfile") == 0){
	    result.fpboundname = argv[++nxtarg];
	    if((result.fpbound = fopen(result.fpboundname,"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open bounds output file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	} else
	if(strcmp(argv[nxtarg],"-cbfile") == 0){
	    result.hasbounds = 1;
	    if((fptmp = fopen(argv[++nxtarg],"r")) == NULL){
		fprintf(stderr,"ERROR: unable to open bounding region file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	    ReadClusterBounds(&result,fptmp);
	} else
	if(strcmp(argv[nxtarg],"-bound") == 0){
	    result.bound_x1  = atoi(argv[++nxtarg]);
	    result.bound_y1  = atoi(argv[++nxtarg]);
	    result.bound_x2  = atoi(argv[++nxtarg]);
	    result.bound_y2  = atoi(argv[++nxtarg]);
	    result.hasbounds = 1;
	} else
	if(strcmp(argv[nxtarg],"-do") == 0){
	    result.fpdirname = argv[++nxtarg];
	    if((result.fpdir = fopen(result.fpdirname,"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open direction output file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	} else
	if(strcmp(argv[nxtarg],"-oo") == 0){
	    result.fpoccname = argv[++nxtarg];
	    if((result.fpocc = fopen(result.fpoccname,"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open occupancy output file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	} else
	if(strcmp(argv[nxtarg],"-o") == 0){
	    result.fpoutname = argv[++nxtarg];
	    if((result.fp = fopen(result.fpoutname,"w")) == NULL){
		fprintf(stderr,"ERROR: unable to open output file '%s' \n",
		argv[nxtarg]);
		exit(0);
	    }
	} else
	if (strcmp(argv[nxtarg], "-xres")==0) {
	  result.xres = (float) atof(argv[++nxtarg]);
	  if (xsizeset==0)
	    result.xsize = result.xres;
	} else
	if (strcmp(argv[nxtarg], "-yres")==0) {
	  result.yres = (float) atof(argv[++nxtarg]);
	  if (ysizeset==0)
	    result.ysize = result.yres;
	} else
	if (strcmp(argv[nxtarg], "-posint")==0) {
	  result.pinterval = (float) atof(argv[++nxtarg]);
        } else
	if(argv[nxtarg][0] != '-'){
	    result.pfname = argv[nxtarg];
	    if((plist.fp = fopen(result.pfname,"r")) == NULL){
		fprintf(stderr,"%s: ERROR: unable to open position file '%s' \n",
		argv[0],
		argv[nxtarg]);
		exit(0);
	    }
	} else {
	    fprintf(stderr,"ERROR: invalid option '%s'\n",argv[nxtarg]);
	    exit(0);
	}
    }

    if(!plist.fp){
	fprintf(stderr,"ERROR: Must specify a position file\n");
	exit(0);
    }

    /*
    ** override intrinsic binary format with the ascii flag
    */
    if(result.ascii){
	result.format = ASCII;
    }

    maxgaplen = max_consecutive_miss*result.pinterval*1e4;
    /*
    ** scale the bounding region specification according to the
    ** desired xy gridsize
    */
    result.bound_x1  *= result.xsize/result.xres; /*XPOSITION_RESOLUTION;*/
    result.bound_y1  *= result.ysize/result.yres; /*YPOSITION_RESOLUTION;*/
    result.bound_x2  *= result.xsize/result.xres; /*XPOSITION_RESOLUTION;*/
    result.bound_y2  *= result.ysize/result.yres; /*YPOSITION_RESOLUTION;*/
    ScaleClusterBounds(&result);

    AllocateArrays(&result);

    /*
    ** read in the time range file
    */
    if(result.fptrange){
	count = ReadRange(&result);
	if(verbose){
	    fprintf(stderr,"loaded %d time ranges from '%s'\n",
	    count,result.fptrangename);
	}
    }
    /*
    ** read in the angular range map
    */
    if(result.fpangularrange){
	if(ReadAngularRangeMap(&result) == 0){
	    fprintf(stderr,"ERROR: unable to read angular range map '%s'\n",
	    result.fpangularrangename);
	} else 
	if(verbose){
	    fprintf(stderr,"Using %dx%d angular range map '%s'\n",
	    result.angular_range_map->xsize,
	    result.angular_range_map->ysize,
	    result.fpangularrangename);
	}
    }

    /*
    ** prepare the output file headers
    */
    PrepareOutputFiles(&result,spikearray,argc,argv);

    /*
    ** read in positional information
    */
    if(verbose){
	fprintf(stderr,"Reading positions from '%s'...",result.pfname);
    }
    LoadAndSortPositions(&result,&plist);

    if(verbose){
	fprintf(stderr,"Read %d positions\n",plist.npositions);
    }

    LoadAndSortSpikes(&result,spikearray);
    /*
    ** compute correlates
    */
    ComputeCorrelate(&plist,&result,spikearray,result.nclusters);
    /*
    ** done
    */
    if(result.fpreport){
	for(i=0;i<result.nclusters;i++){
	    fprintf(result.fpreport,"%% Cluster %s : binned %d of %d spikes (%.2g%%)\n",
	    spikearray[i].fname,
	    spikearray[i].cumspikes,
	    spikearray[i].nspikes,
	    100.0*spikearray[i].cumspikes/spikearray[i].nspikes
	    );
	    EvaluateSpatialFiring(&result,spikearray,i);
	}
    }
    exit(0);
}

