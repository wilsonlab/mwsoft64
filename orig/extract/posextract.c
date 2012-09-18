
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
    original xx/xx

**************************************************
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <header.h>
#include <iolib.h>

#define VERSION "1.23"

#define MAXCOORDS	256
#define MAXDIODESPREAD	1000
#define BEHIND		0
#define AHEAD		1
#define MAXBOUNDS	100

#define XSIZE	365
#define YSIZE	256
short	**grid;
short	**grid2;


typedef struct {
    float	x,y;
} FCoord;

typedef struct bound_type {
    int		x;
    int		y;
    int		x2;
    int		y2;
    unsigned long	t1;
    unsigned long	t2;
} Bound;

typedef struct result_type {
    FILE	*fpout;
    FILE	*fpin;
    FILE	*fpcb;
    FILE	*fpstatout;
    short	filter_points;
    Bound	bound[MAXBOUNDS];
    int		nbounds;
    short	px,py;
    float	maxdist;
    short	ignorefb;
    short	switchfb;
    int		ascii;
    FCoord	*fcoord;
    int		ncoords;
    unsigned long		toffset;	
    int		nframes;
    float		smooth;
} Result;

typedef struct coord_type {
    short	xcoord;
    unsigned char	ycoord;
    unsigned char	match_ahead;
    unsigned char	match_behind;
} Coord;

typedef struct posrec_type {
    unsigned long timestamp;
    unsigned char	frame;	
    unsigned char	nitems;
    Coord coord[MAXCOORDS];
    int			accepted;
    int			rejected;
} PositionRecord;


int verbose;
int	dump;
int	singlecolumn;
int	convert;

void ReadClusterBounds(result)
Result	*result;
{
char	line[200];
float	wx1,wy1;
int	p0,p1;
int	clusterid;
FILE	*fp;
int	level;
int	count;

    fp = result->fpcb;
    /*
    ** backup the current bounds
    */
    level = 0;
    count = 0;
    while(!feof(fp)){
	if(fgets(line,1000,fp) == NULL) break;
	/*
	** ignore comments
	*/
	if(line[0] == '%') continue;
	switch(level){
	case 0:
	    /*
	    ** read in the clusterid
	    */
	    if(sscanf(line,"%d",&clusterid) == 1){
		level = 1;
	    }
	    break;
	case 1:			/* projection numbers */
	    if(sscanf(line,"%d%d",&p0,&p1)  == 2){
		level = 2;
	    }
	    break;
	case 2:			/* projection 0 name */
	    if(strlen(line) > 0){
		level = 3;
	    }
	    break;
	case 3:			/* projection 1 name */
	    if(strlen(line) > 0){
		level = 4;
	    }
	    break;
	case 4:			/* number of coordinates */
	    if(sscanf(line,"%d",&result->ncoords)  == 1){
		level = 5;
		/*
		** allocate the coordinate array
		*/
		result->fcoord = (FCoord *)calloc(result->ncoords,sizeof(FCoord));
	    }
	    break;
	case 5:			/* coordinates */
	    if(sscanf(line,"%f%f",&wx1,&wy1)  == 2){
		result->fcoord[count].x = wx1;
		result->fcoord[count].y = wy1;
		if(count == result->ncoords -1){
		    level = 0;
		    count = 0;
		} else {
		    count++;
		}
	    }
	    break;
	}
    }
    fclose(fp);

    if(level != 0){
	fprintf(stderr,"error in reading bounds file\n");
    }
}


int IsInPolygon(result,wx,wy,fcoord,ncoords)
Result	*result;
float	wx,wy;
FCoord	*fcoord;
int	ncoords;
{
int	i;
int	pcross;
double	FY,bx;
    
    if(fcoord == NULL) return(0);
    /*
    ** look for odd number of intersections with poly segments
    */
    pcross = 0;
    /*
    ** extend a horizontal line along the positive x axis and
    ** look at intersections
    */
    for(i=0;i<ncoords-1;i++){
	/*
	** only examine segments whose endpoint y coords
	** bracket those of the test coord
	*/
	if((wy >= fcoord[i].y && wy < fcoord[i+1].y) ||
	(wy < fcoord[i].y && wy >= fcoord[i+1].y)){
	    /*
	    ** count those which are on the positive x side 
	    ** by computing the intercept.
	    ** find the x value of the line between the two fcoords
	    ** which is at wy
	    */
	    FY = (fcoord[i+1].y-wy)/(fcoord[i].y-wy);
	    bx = (fcoord[i].x*FY - fcoord[i+1].x)/(FY-1);

	    if(bx >= wx){
		pcross++;
	    }
	}
    }
    if(i == ncoords-1){
	/*
	** compute the final point which closes the polygon
	*/
	if((wy >= fcoord[i].y && wy < fcoord[0].y) ||
	(wy < fcoord[i].y && wy >= fcoord[0].y)){
	    FY = (fcoord[0].y-wy)/(fcoord[i].y-wy);
	    bx = (fcoord[i].x*FY - fcoord[0].x)/(FY-1);
	    if(bx > wx){
		pcross++;
	    }
	}
    }
    /*
    ** now look for an odd number of crossings
    */
    if(pcross > 0 && (pcross%2 == 1)){
	return(1);
    } else {
	return(0);
    }
}

int GetPosRecord(Result *result,PositionRecord *posrec)
{
int	size;
int	i;

    /*
    ** read in a position record 
    */
    /*
    ** add up the size of the first 3 fields
    */
    size = sizeof(unsigned long) + sizeof(unsigned char) + 
	sizeof(unsigned char);
    /*
    ** nitems
    */
    if(fread(&posrec->nitems,sizeof(unsigned char),1,result->fpin) != 1){
	if(feof(result->fpin)) return(0);
	fprintf(stderr,"\nerror reading nitems at offset %ld\n",
	ftell(result->fpin));
	return(0);
    }
    /*
    ** read in the frame
    */
    if(fread(&posrec->frame,sizeof(unsigned char),1,result->fpin) != 1){
	if(feof(result->fpin)) return(0);
	fprintf(stderr,"\nerror reading frame at offset %ld\n",
	ftell(result->fpin));
	return(0);
    }

/*     Commented out 11/22/04 by Tom D.: This code doesn't do
       anything. Switching to posrec->frame = !posrec->frame seems to
       produce odd results. (probably because the switching is
       interacting with other filtering (like max distance?) */
/* */
/*     if(result->switchfb){ */
/* 	posrec->frame != posrec->frame; */
/*     } */

    /*
    ** read in the timestamp
    */
    if(fread(&posrec->timestamp,sizeof(unsigned long),1,result->fpin) != 1){
	if(feof(result->fpin)) return(0);
	fprintf(stderr,"\nerror reading timestamp at offset %ld\n",
	ftell(result->fpin));
	return(0);
    }
    if(convert){
	ConvertData(&posrec->timestamp,sizeof(unsigned long));
    }
    if(result->toffset > 0){
	posrec->timestamp += result->toffset;
    }
    if(dump && !singlecolumn)
	fprintf(result->fpout,"%lu:\t%2d\t%2d\t",posrec->timestamp,posrec->frame,posrec->nitems);
    /*
    ** read in all of the coords
    if(verbose){
	fprintf(stderr,"%d items frame %d at time %lu offset %d\n",
	(int)posrec->nitems,(int)posrec->frame,
	posrec->timestamp,ftell(result->fpin));
    }
    */
    for(i=0;i<posrec->nitems;i++){
	posrec->coord[i].match_ahead = 0;
	posrec->coord[i].match_behind = 0;
	if(fread(&posrec->coord[i].xcoord,sizeof(short),1,result->fpin) != 1){
	    if(feof(result->fpin)) return(0);
	    fprintf(stderr,"\nerror reading xcoord at offset %ld\n",
	    ftell(result->fpin));
	    return(0);
	}
	if(convert){
	    ConvertData(&posrec->coord[i].xcoord,sizeof(short));
	}
	if(dump && singlecolumn)
	    fprintf(result->fpout,"%d\t",posrec->coord[i].xcoord);
	if(dump && !singlecolumn)
	    fprintf(result->fpout,"%d,",posrec->coord[i].xcoord);
	if(fread(&posrec->coord[i].ycoord,sizeof(unsigned char),1,result->fpin) != 1){
	    if(feof(result->fpin)) return(0);
	    fprintf(stderr,"\nerror reading ycoord at offset %ld\n",
	    ftell(result->fpin));
	    return(0);
	}
	if(dump)
	    fprintf(result->fpout,"%d\t",posrec->coord[i].ycoord);
	if(singlecolumn)
	    fprintf(result->fpout,"\n");
    }
    if(dump)
	fprintf(result->fpout,"\n");
    return(1);
}

void ComparePixels(PositionRecord *posrec1, PositionRecord *posrec2,int
match_direction)
{
short	xcoord1;
unsigned char ycoord1;
short	xcoord2;
unsigned char ycoord2;
int	i,j;

    /*
    ** reject any pixels in rec2 which are in common with rec1
    */
    for(i=0;i<posrec1->nitems;i++){
	xcoord1 = posrec1->coord[i].xcoord;
	ycoord1 = posrec1->coord[i].ycoord;
	for(j=0;j<posrec2->nitems;j++){
	    xcoord2 = posrec2->coord[j].xcoord;
	    ycoord2 = posrec2->coord[j].ycoord;
	    if((xcoord1 == xcoord2) && (ycoord1 == ycoord2)){
		if(match_direction == AHEAD){
		    posrec2->coord[j].match_ahead = 1;
		} else {
		    posrec2->coord[j].match_behind = 1;
		}
		break;
	    }
	}
    }
}

int PixelDiff(PositionRecord *posrec1, PositionRecord *posrec2)
{
short	xcoord1;
unsigned char ycoord1;
short	xcoord2;
unsigned char ycoord2;
int	i,j;
int	diffpixels;

    /*
    ** count any pixels in rec2 which are not in common with rec1
    */
    diffpixels = 0;
    /*
    ** count the number of pixels in posrec1 that are not in posrec2
    */
    for(i=0;i<posrec1->nitems;i++){
	xcoord1 = posrec1->coord[i].xcoord;
	ycoord1 = posrec1->coord[i].ycoord;
	for(j=0;j<posrec2->nitems;j++){
	    xcoord2 = posrec2->coord[j].xcoord;
	    ycoord2 = posrec2->coord[j].ycoord;
	    if((xcoord1 == xcoord2) && (ycoord1 == ycoord2)){
		break;
	    }
	}
	/*
	** went through the list and didnt find a match
	*/
	if(j==posrec2->nitems){
		diffpixels++;
	}
    }
    /*
    ** count the number of pixels in posrec2 that are not in posrec1
    */
    for(i=0;i<posrec2->nitems;i++){
	xcoord1 = posrec2->coord[i].xcoord;
	ycoord1 = posrec2->coord[i].ycoord;
	for(j=0;j<posrec1->nitems;j++){
	    xcoord2 = posrec1->coord[j].xcoord;
	    ycoord2 = posrec1->coord[j].ycoord;
	    if((xcoord1 == xcoord2) && (ycoord1 == ycoord2)){
		break;
	    }
	}
	/*
	** went through the list and didnt find a match
	*/
	if(j==posrec1->nitems){
		diffpixels++;
	}
    }
    return(diffpixels);
}

void Smooth(Result *result, short **grid,short xcoord,unsigned char ycoord)
{
int x,y;
int i,j;

    /*
    ** fill in pixels within the specified radius 
    */
    for(i=-result->smooth;i<=result->smooth;i++){
	x = xcoord+i;
	if(x <0 || x>=XSIZE) continue; 
	for(j=-result->smooth;j<=result->smooth;j++){
	    y = (int)ycoord+j;
	    if(y <0 || y>=YSIZE) continue; 
	    /*
	    ** could impose a real radial constraint here
	    ** x^2 + y^2 < r^2
	    */
	    grid[x][y] = 1;
	}
    }
}

int EvaluateQueuedDifference(Result *result, PositionRecord *posrec,
    int recnum,float *distance, int *pixeldiff)
{
short	xcoord1;
unsigned char ycoord1;
short	xcoord2;
unsigned char ycoord2;
int	i,j;
PositionRecord *posrec1;
int	tmprecnum;
int	totalpixels;
int	framecount;
int	pointcount;
double	xmean1,xmean2;
double	ymean1,ymean2;
int	done;
short	*gridptr;
short	*gridptr2;

    if(verbose){
	fprintf(stderr,".");
    }
    /*
    ** clear the grid
    */
    for(i=0;i<XSIZE;i++){
	bzero(grid[i],YSIZE*sizeof(short));
	bzero(grid2[i],YSIZE*sizeof(short));
    }
    /*
    ** now compute the first half vs second half mean distance 
    */
    tmprecnum = recnum;
    /*
    ** do the second half
    */
    xmean2 = 0;
    ymean2 = 0;
    framecount=0;
    pointcount=0;
    done = 0;
    while(!done){
	/*
	** select the next frame
	*/
	if((tmprecnum = (++tmprecnum)%result->nframes) == recnum){
	    /*
	    ** we have gone through the entire list 
	    ** this is an error
	    */
	    break;
	}
	posrec1 = posrec + tmprecnum;
	for(i=0;i<posrec1->nitems;i++){
	    xcoord2 = posrec1->coord[i].xcoord;
	    ycoord2 = posrec1->coord[i].ycoord;
	    if(result->smooth > 0){
		Smooth(result,grid,xcoord2,ycoord2);
	    } else {
		grid[xcoord2][ycoord2] = 1;
	    }

	    xmean2 += xcoord2;
	    ymean2 += ycoord2;
	    pointcount++;

	}
	framecount++;
	if(framecount >= result->nframes/2){
	    done = 1;
	}
    }
    if(!done){
	fprintf(stderr,"frame error in distance calculation\n");
	return(0);
    }
    if(pointcount > 0){
	xmean2 /= pointcount;
	ymean2 /= pointcount;
    }
    /*
    ** do the first half starting from where we left off
    */
    xmean1 = 0;
    ymean1 = 0;
    framecount=0;
    pointcount=0;
    done=0;
    while(!done){
	/*
	** select the next frame
	*/
        tmprecnum++;
        tmprecnum = tmprecnum%result->nframes;
	posrec1 = posrec + tmprecnum;
	if(tmprecnum == recnum){
	    /*
	    ** current record 
	    */
	    for(i=0;i<posrec1->nitems;i++){
		xcoord1 = posrec1->coord[i].xcoord;
		ycoord1 = posrec1->coord[i].ycoord;
		if(result->smooth > 0){
		    Smooth(result,grid2,xcoord1,ycoord1);
		} else {
		    grid[xcoord1][ycoord1]++;
		}
		/*
		** points of overlap will have the value of 2
		** points in either previous or corrent posrecs but not both
		** will have the value 1 and all others will be zero
		*/
		xmean1 += xcoord1;
		ymean1 += ycoord1;
		pointcount++;
	    }
	} else {
	    for(i=0;i<posrec1->nitems;i++){
		xcoord1 = posrec1->coord[i].xcoord;
		ycoord1 = posrec1->coord[i].ycoord;
		if(result->smooth > 0){
		    Smooth(result,grid2,xcoord1,ycoord1);
		} else {
		    grid[xcoord1][ycoord1]++;
		}

		xmean1 += xcoord1;
		ymean1 += ycoord1;
		pointcount++;
	    }
	}
	if(tmprecnum == recnum){
	    /*
	    ** we have gone through the entire list 
	    */
	    done = 1;
	    break;
	}
	framecount++;
    }
    if(pointcount > 0){
	xmean1 /= pointcount;
	ymean1 /= pointcount;
    }
    /*
    ** the difference is computed by counting the number of pixels with
    ** value 1.  The ratio is the difference over the number of pixels
    ** with value greater than zero
    */
    *pixeldiff = 0;
    totalpixels = 0;
    /*
    ** ignore the zero coordinate
    */
    if(result->smooth > 0){
	grid[0][0] = 0;
	grid2[0][0] = 0;
	for(i=0;i<XSIZE;i++){
	    gridptr = grid[i];
	    gridptr2 = grid2[i];
	    for(j=0;j<YSIZE;j++,gridptr++,gridptr2++){
		if((*gridptr == 0) && (*gridptr2 == 0)){
		    continue;
		} else
		if((*gridptr > 0) && (*gridptr2 > 0)){
		    (*pixeldiff)++;
		    totalpixels++;
		} else 
		if((*gridptr == 0) || (*gridptr2 == 0)){
		    totalpixels++;
		}
	    }
	}
    } else {
	grid[0][0] = 0;
	for(i=0;i<XSIZE;i++){
	    gridptr = grid[i];
	    for(j=0;j<YSIZE;j++,gridptr++){
		if(*gridptr == 0){
		    continue;
		} else
		if(*gridptr == 1){
		    (*pixeldiff)++;
		    totalpixels++;
		} else 
		if(*gridptr > 0){
		    totalpixels++;
		}
	    }
	}
    }

    *distance = sqrt((xmean1 - xmean2)*(xmean1 - xmean2) +
	(ymean1 - ymean2)*(ymean1 - ymean2));
    return(totalpixels);
}

void GetDiodeCoord(Result *result,PositionRecord *posrec,Coord *returncoord)
{
int	i,j;
float	xcoord;
float	ycoord;
int	count;

    count = 0;
    xcoord = 0;
    ycoord = 0;
    posrec->accepted = 0;
    posrec->rejected = 0;
    for(i=0;i<posrec->nitems;i++){
	/*
	** check bounds
	*/
	for(j=0;j<result->nbounds;j++){
	    if(((posrec->timestamp < result->bound[j].t1) || 
	    (posrec->timestamp > result->bound[j].t2)) &&
	    !((result->bound[j].t1 == 0) && (result->bound[j].t2 == 0))){
		continue;
	    }
	    /*
	    ** valid bound so check it
	    */
	    if((posrec->coord[i].xcoord < result->bound[j].x) ||
	    (posrec->coord[i].xcoord > result->bound[j].x2) ||
	    (posrec->coord[i].ycoord < result->bound[j].y) ||
	    (posrec->coord[i].ycoord > result->bound[j].y2)){
		break;
	    }
	}
	if(j<result->nbounds){
	    posrec->rejected++;
	    continue;
	}
	/*
	** check against cluster bounds
	*/
	if(result->fcoord){
	    if(!IsInPolygon(result,(float)posrec->coord[i].xcoord,
		(float)posrec->coord[i].ycoord,result->fcoord,
		result->ncoords)){
		posrec->rejected++;
		continue;
	    }
	}
	if( (result->filter_points == 0) 
	||  ((result->filter_points == 1) && 
	    (posrec->coord[i].match_ahead == 0 ||
	    posrec->coord[i].match_behind == 0))
	||  ((result->filter_points == 2) && 
	    (posrec->coord[i].match_ahead == 0 &&
	    posrec->coord[i].match_behind == 0))){
	    /*
	    ** check to see whether the coord is too far
	    if((count ==  0) || 
	    (abs((int)(posrec->coord[i].xcoord - xcoord/count))<MAXDIODESPREAD
	    && abs((int)(posrec->coord[i].ycoord - ycoord/count))<MAXDIODESPREAD)
	    ){
	    */
            if(result->maxdist < 0 || result->px <= 0 || result->py <= 0 ||
            (sqrt((double)((posrec->coord[i].xcoord - result->px)*
            (posrec->coord[i].xcoord - result->px) +
            (posrec->coord[i].ycoord - result->py)*
            (posrec->coord[i].ycoord - result->py))) < result->maxdist)
            ){

		xcoord += posrec->coord[i].xcoord;
		ycoord += posrec->coord[i].ycoord;
		count++;
	    }
	    posrec->accepted++;
	} else {
	    posrec->rejected++;
	}
    }
    if(count == 0){
	returncoord->xcoord = 0;
	returncoord->ycoord = 0;
    } else {
	returncoord->xcoord = xcoord/count;
	returncoord->ycoord = ycoord/count;
    }
}

int main(argc,argv)
int	argc;
char 	**argv;
{
int	nxtarg;
Result	result;
int	recnum;
int	nrecnum;
int	precnum;
int	count;
short	sval;
PositionRecord *posrec;
short	got_front;
short	got_rear;
Coord	front_diode;
Coord	rear_diode;
unsigned long	front_time;
unsigned long	rear_time;
int	headersize;
char	**header;
int	accepted;
int	rejected;
int	i;
int	frontfill;
int	backfill;
int	nitems;
int	diff;
float	dist;
int	jump;
char	*ratestr;
int	rate;
unsigned long mininterval;

    verbose = 0;
    nxtarg = 0;
    frontfill = 0;
    backfill = 0;
    result.nframes = 3;
    result.fpin = NULL;
    result.fpcb = NULL;
    result.fpout = stdout;
    result.fpstatout = NULL;
    result.filter_points = 1;
    result.ignorefb = 0;
    result.maxdist = -1;
    result.ascii = 0;
    result.toffset = 0;
    result.smooth = 0;
    result.switchfb = 0;
    result.fcoord = NULL;
    jump = 0;
    grid = (short **)malloc(sizeof(short *)*XSIZE);
    grid2 = (short **)malloc(sizeof(short *)*XSIZE);
    for(i=0;i<XSIZE;i++){
	grid[i] = (short *)malloc(sizeof(short)*YSIZE);
	grid2[i] = (short *)malloc(sizeof(short)*YSIZE);
    }

    accepted = 0;
    rejected = 0;
    dump = 0;
    singlecolumn=0;
    result.nbounds = 0;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,"usage: %s posfile [options]\n",argv[0]);
	    fprintf(stderr,"\tOptions:\n");
	    fprintf(stderr,"\t[-all]		extract all points\n");
	    fprintf(stderr,"\t[-filter2]	stronger filter criterion\n");
	    fprintf(stderr,"\t[-bound x y x2 y2]	ignore pixels outside of bounds\n");
	    fprintf(stderr,"\t[-tbound x y x2 y2 t1 t2]	ignore pixels outside of bounds from t1 to t2\n");
	    fprintf(stderr,"\t[-dump]		show all points\n");
	    fprintf(stderr,"\t[-singlecolumn]	dump points in a single column\n");
	    fprintf(stderr,"\t[-smooth radius]	expand pixels in xy by radius\n");
            fprintf(stderr,"\t[-ignorefb]       ignore front/back distinction\n");
            fprintf(stderr,"\t[-switchfb]       (BROKEN) switch front and back diodes\n");
	   
            fprintf(stderr,"\t[-backfill]       fill missing back diode frames\n");
            fprintf(stderr,"\t[-frontfill]       fill missing front diode frames\n");
            fprintf(stderr,"\t[-maxdist pixels] maximum distance between frames\n");
            fprintf(stderr,"\t[-cbfile file]	cluster bounds file\n");
            fprintf(stderr,"\t[-nframes n]	window size to use in stats calculation\n");
            fprintf(stderr,"\t[-jump]	stats output every nframes\n");
            fprintf(stderr,"\t[-toffset time]	add 'time' to all posfile times\n");
            fprintf(stderr,"\t\t\t	Note that 'time' MUST be positive\n");
            fprintf(stderr,"\t[-ascii]          extract points in ascii\n");
	    fprintf(stderr,"\t[-v]		verbose on\n");
	    fprintf(stderr,"\t[-version]	show version\n");
	    fprintf(stderr,"\t[-o file]		output to file\n");
	    fprintf(stderr,"\t[-stato file]		pixel statistics output file\n");
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
	if(strcmp(argv[nxtarg],"-dump") == 0){
	    dump = 1;
	} else
	if(strcmp(argv[nxtarg],"-singlecolumn") == 0){
	    singlecolumn = 1;
	} else
	if(strcmp(argv[nxtarg],"-all") == 0){
	    result.filter_points = 0;
	} else
	if(strcmp(argv[nxtarg],"-ascii") == 0){
	    result.ascii = 1;
	} else
	if(strcmp(argv[nxtarg],"-jump") == 0){
	    jump = 1;
	} else
        if(strcmp(argv[nxtarg],"-nframes") == 0){
            result.nframes = atoi(argv[++nxtarg]);
	    if(result.nframes < 3){
		fprintf(stderr,"nframes must be greater than 3. Using default (3)\n");
		result.nframes = 3;
	    }
        } else
        if(strcmp(argv[nxtarg],"-ignorefb") == 0){
            result.ignorefb = 1;
        } else
        if(strcmp(argv[nxtarg],"-switchfb") == 0){
            result.switchfb = 1;
	    fprintf(stderr,"Posextract error: switchfb option not currently supported\n");
	    exit(-1);
        } else
        if(strcmp(argv[nxtarg],"-frontfill") == 0){
            frontfill = 1;
            result.ignorefb = 1;
        } else
        if(strcmp(argv[nxtarg],"-backfill") == 0){
            backfill = 1;
            result.ignorefb = 1;
        } else
        if(strcmp(argv[nxtarg],"-smooth") == 0){
            result.smooth = atof(argv[++nxtarg]);
        } else
        if(strcmp(argv[nxtarg],"-toffset") == 0){
            result.toffset = ParseTimestamp(argv[++nxtarg]);
        } else
        if(strcmp(argv[nxtarg],"-maxdist") == 0){
            result.maxdist = atof(argv[++nxtarg]);
        } else
	if(strcmp(argv[nxtarg],"-filter2") == 0){
	    result.filter_points = 2;
	} else
	if(strcmp(argv[nxtarg],"-cbfile") == 0){
	    if((result.fpcb = fopen(argv[++nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else
	if(strcmp(argv[nxtarg],"-bound") == 0){
	    if(result.nbounds >= MAXBOUNDS){
		fprintf(stderr,"exceeded maximum number of bounds %d\n",
		MAXBOUNDS);
		exit(-1);
	    }
	    result.bound[result.nbounds].x = atoi(argv[++nxtarg]);
	    result.bound[result.nbounds].y = atoi(argv[++nxtarg]);
	    result.bound[result.nbounds].x2 = atoi(argv[++nxtarg]);
	    result.bound[result.nbounds].y2 = atoi(argv[++nxtarg]);
	    result.bound[result.nbounds].t1 = 0;
	    result.bound[result.nbounds].t2 = 0;
	    result.nbounds++;
	} else
	if(strcmp(argv[nxtarg],"-tbound") == 0){
	    if(result.nbounds >= MAXBOUNDS){
		fprintf(stderr,"exceeded maximum number of bounds %d\n",
		MAXBOUNDS);
		exit(-1);
	    }
	    result.bound[result.nbounds].x = atoi(argv[++nxtarg]);
	    result.bound[result.nbounds].y = atoi(argv[++nxtarg]);
	    result.bound[result.nbounds].x2 = atoi(argv[++nxtarg]);
	    result.bound[result.nbounds].y2 = atoi(argv[++nxtarg]);
	    result.bound[result.nbounds].t1 = ParseTimestamp(argv[++nxtarg]);
	    result.bound[result.nbounds].t2 = ParseTimestamp(argv[++nxtarg]);
	    result.nbounds++;
	} else
	if(strcmp(argv[nxtarg],"-v") == 0){
	    verbose = 1;
	} else
	if(strcmp(argv[nxtarg],"-stato") == 0){
	    if((result.fpstatout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-o") == 0){
	    if((result.fpout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(argv[nxtarg][0] != '-'){
	    if((result.fpin = fopen(argv[nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else {
	    fprintf(stderr,"invalid option '%s'\n",argv[nxtarg]);
	    exit(-1);
	}
    }
    posrec = (PositionRecord *)calloc(result.nframes,sizeof(PositionRecord));
    header = ReadHeader(result.fpin,&headersize);
    /*
    ** compare architectures
    */
    if((GetLocalArchitecture() == GetFileArchitecture(header)) ||
	(GetFileArchitecture(header) == ARCH_UNKNOWN)) {
	convert = 0;
	fprintf(stderr,"No data conversion file from %s architecture.\n",
	GetFileArchitectureStr(header));
    } else {
	convert = 1;
	fprintf(stderr,"Converting data from %s to %s architectures.\n",
	     GetFileArchitectureStr(header),
	     GetLocalArchitectureStr());
    }
    /*
    ** determine the tracker sampling rate
    */
    if((ratestr = GetHeaderParameter(header,"tracker_rate:")) == NULL){
	fprintf(stderr,"Warning: unable to determine tracker sampling rate from header.\nUsing default 60 Hz.\n");
	rate = 1;
    } else {
	rate = atoi(ratestr);
	if(rate <= 0){
	    fprintf(stderr,"Warning: invalid tracker sampling rate from header.\nUsing default 60 Hz.\n");
	    rate = 1;
	}
    }
    BeginStandardHeader(result.fpout,argc,argv,VERSION);
    if(dump){
	fprintf(result.fpout,"%% File type:\tAscii\n");
	fprintf(result.fpout,
	"%% Fields:\t%s,%d,%d,1\t%s,%d,%d,1\n",
	    "x",SHORT,sizeof(short),
	    "y",SHORT,sizeof(short));
    } else {
	fprintf(result.fpout,"%% File type:\tBinary\n");
	fprintf(result.fpout,"%% Record type:\tT\n");
	fprintf(result.fpout,"%% Extraction type:\textended dual diode position\n");
	fprintf(result.fpout,
	"%% Fields:\t%s,%d,%d,1\t%s,%d,%d,1\t%s,%d,%d,1\t%s,%d,%d,1\t%s,%d,%d,1\n",
	    "timestamp",ULONG,sizeof(unsigned long),
	    "xfront",SHORT,sizeof(short),
	    "yfront",SHORT,sizeof(short),
	    "xback",SHORT,sizeof(short),
	    "yback",SHORT,sizeof(short));
	fprintf(result.fpout,"%%\n%% Pos file header:\n");
	DisplayHeader(result.fpout,header,headersize);
    }
    EndStandardHeader(result.fpout);
    if(result.fpstatout){
	    BeginStandardHeader(result.fpstatout,argc,argv,VERSION);
	    fprintf(result.fpstatout,"%% File type:\tAscii\n");
	    fprintf(result.fpstatout,"%% Extraction type:\tdual diode statistics\n");
	    fprintf(result.fpstatout,
	    "%% Fields:\t%s\t%s\t%s\t%s\t%s\n",
		"timestamp",
		"ndiff",
		"npix",
		"percent",
		"distance");
	    fprintf(result.fpstatout,"%%\n%% Pos file header:\n");
	    DisplayHeader(result.fpstatout,header,headersize);
	    EndStandardHeader(result.fpstatout);
    }
    /*
    ** read in cluster bounds if any
    */
    if(result.fpcb){
	ReadClusterBounds(&result);
    }
    /*
    ** read in position records
    */
    recnum = 0;
    count = 0;
    got_front = 0;
    got_rear = 0;
    mininterval = 1.5*(1e4*rate/60);
if(result.nbounds > 0){
   for(i=0;i<result.nbounds;i++){
      fprintf(stderr,"%lu %lu\n",
      result.bound[i].t1,
      result.bound[i].t2);
   }
}
    /*
    ** preload the posrec data structure, which will typically hold 3 frames
    ** of position data, with two frames worth of data.
    */
    /*
    ** get first position record
    */
    GetPosRecord(&result,posrec + recnum);
    precnum = recnum;
    recnum = (recnum + 1)%result.nframes;
    /*
    ** get second position record
    */
    GetPosRecord(&result,posrec + recnum);
    while(!feof(result.fpin)){
	nrecnum = (recnum + 1)%result.nframes;
	/*
	** get the next position record
	*/
	if(GetPosRecord(&result,posrec + nrecnum) == 0) break;
	count++;
	if(dump){
	    continue;
	}
	/*
	** compare pixels in adjacent records and reject as noise
	** in the current rec those that are active in both
	**
	** this doesnt work too well in practice.  Too many instances of
	** positive rejection
	*/
	if(result.filter_points > 0){
	    ComparePixels(posrec + precnum,posrec + recnum,BEHIND);
	    ComparePixels(posrec + nrecnum,posrec + recnum,AHEAD);
	}
	/*
	** process those pixels which pass the test
	*/
	/*
	** front diode
	*/
	if(posrec[recnum].frame == 0){
            if(count > 1){
                result.px = rear_diode.xcoord;
                result.py = rear_diode.ycoord;
            } else {
                result.px = front_diode.xcoord;
                result.py = front_diode.ycoord;
            }
   
	    GetDiodeCoord(&result,posrec+recnum,&front_diode);
	    front_time = posrec[recnum].timestamp;
	    got_front = 1;
	    /*
	    if(verbose){
		fprintf(stderr,"front\n");
	    }
	    */
	    accepted += posrec->accepted;
	    rejected += posrec->rejected;
	    /*
	    ** ignore any back diode that might have been previously
	    ** detected.  This enforces the rule that the coordinates
	    ** will always be based upon the front diode, and the back
	    ** diode that follows it in the next frame.
	    */
	    if(got_rear && verbose){
		fprintf(stderr,"calibrate front/back diode order at timestamp %ld\n",front_time);
	    }
	    got_rear = 0;
	} 
	/*
	** rear diode
	*/
	if(posrec[recnum].frame == 1){
            if(count > 1){
                result.px = rear_diode.xcoord;
                result.py = rear_diode.ycoord;
            } else {
                result.px = front_diode.xcoord;
                result.py = front_diode.ycoord;
            }
   
	    GetDiodeCoord(&result,posrec+recnum,&rear_diode);
	    rear_time = posrec[recnum].timestamp;
	    got_rear = 1;
	    /*
	    if(verbose){
		fprintf(stderr,"rear\n");
	    }
	    */
	    accepted += posrec->accepted;
	    rejected += posrec->rejected;
	} 
        if(result.ignorefb){
            if(got_rear || (frontfill && got_front && count > 1)){
                /*
                ** write out the position
                */
                fwrite(&rear_time,sizeof(unsigned long),1,result.fpout);
                sval = rear_diode.xcoord;
                fwrite(&sval,sizeof(short),1,result.fpout);
                sval = rear_diode.ycoord;
                fwrite(&sval,sizeof(short),1,result.fpout);
                sval = rear_diode.xcoord;
                fwrite(&sval,sizeof(short),1,result.fpout);
                sval = rear_diode.ycoord;
                fwrite(&sval,sizeof(short),1,result.fpout);
		if(result.fpstatout){
#ifdef OLD
		    if((nitems=posrec[precnum].nitems + posrec[recnum].nitems) > 0){
			diff = PixelDiff(posrec + precnum,posrec + recnum);
			fprintf(result.fpstatout,"%lu %d %d %-7.3g %-7.3g\n",
			front_time,
			diff,
			nitems,
			100.0*diff/nitems,
			sqrt(
			    (double)((rear_diode.ycoord - front_diode.ycoord)*
			    (rear_diode.ycoord - front_diode.ycoord) +
			    (rear_diode.xcoord - front_diode.xcoord)*
			    (rear_diode.xcoord - front_diode.xcoord)
			    ))
			);
		    }
#else
		    /*
		    ** compute the distance between the mean position in the 
		    ** earlier and latter half of the queued posrec frames.
		    ** also compute the absolute and fractional pixel 
		    ** difference between the current
		    ** frame and the cumulative pixels in the queued frames
		    */
		    /*
		    ** only do it in blocks of nframes
		    */
		    if(recnum == 0 || !jump){
			nitems = EvaluateQueuedDifference(&result,posrec,recnum,
			&dist,&diff);
			if(nitems > 0){
			    fprintf(result.fpstatout,"%lu %d %d %-7.3g %-7.3g\n",
			    rear_time,
			    diff,
			    nitems,
			    100.0*diff/nitems,
			    dist
			    );
			}
		    }
#endif
		}
            } 
            if(got_front || (backfill && got_rear && count > 1)){
                /*
                ** write out the position
                */
                fwrite(&front_time,sizeof(unsigned long),1,result.fpout);
                sval = front_diode.xcoord;
                fwrite(&sval,sizeof(short),1,result.fpout);
                sval = front_diode.ycoord;
                fwrite(&sval,sizeof(short),1,result.fpout);
                sval = front_diode.xcoord;
                fwrite(&sval,sizeof(short),1,result.fpout);
                sval = front_diode.ycoord;
                fwrite(&sval,sizeof(short),1,result.fpout);
		if(result.fpstatout){
#ifdef OLD
		    if((nitems=posrec[precnum].nitems + posrec[recnum].nitems) > 0){
			diff = PixelDiff(posrec + precnum,posrec + recnum);
			fprintf(result.fpstatout,"%lu %d %d %-7.3g %-7.3g\n",
			front_time,
			diff,
			nitems,
			100.0*diff/nitems,
			sqrt(
			    (double)((rear_diode.ycoord - front_diode.ycoord)*
			    (rear_diode.ycoord - front_diode.ycoord) +
			    (rear_diode.xcoord - front_diode.xcoord)*
			    (rear_diode.xcoord - front_diode.xcoord)
			    ))
			);
		    }
#else
		    /*
		    ** compute the distance between the mean position in the 
		    ** earlier and latter half of the queued posrec frames.
		    ** also compute the absolute and fractional pixel 
		    ** difference between the current
		    ** frame and the cumulative pixels in the queued frames
		    */
		    /*
		    ** only do it in blocks of nframes
		    */
		    if(recnum == 0 || !jump){
			nitems = EvaluateQueuedDifference(&result,posrec,recnum,
			&dist,&diff);
			if(nitems > 0){
			    fprintf(result.fpstatout,"%lu %d %d %-7.3g %-7.3g\n",
			    front_time,
			    diff,
			    nitems,
			    100.0*diff/nitems,
			    dist
			    );
			}
		    }
#endif
		}
            }
	    got_rear = 0;
	    got_front = 0;
        } else
	if(got_front && got_rear){
	    got_front = 0;
	    got_rear = 0;
	    /*
	    ** reject any pair in which the time between front and rear diode
	    ** samples is greater that one sampling interval 
	    */
	    if((rear_time - front_time > mininterval) || 
	    (rear_time - front_time < 0)){
		fprintf(stderr,"skip diode gap at timestamp %ld\n",front_time);
	    } else {
		/*
		** write out the position
		*/
		if(result.ascii){
		      fprintf(result.fpout,
		      "%lu %3hd,%3hd %3hd,%3hd\n",
		      front_time,
		      front_diode.xcoord,
		      front_diode.ycoord,
		      rear_diode.xcoord,
		      rear_diode.ycoord);
		} else {
		    fwrite(&front_time,sizeof(unsigned long),1,result.fpout);
		    sval = front_diode.xcoord;
		    fwrite(&sval,sizeof(short),1,result.fpout);
		    sval = front_diode.ycoord;
		    fwrite(&sval,sizeof(short),1,result.fpout);
		    sval = rear_diode.xcoord;
		    fwrite(&sval,sizeof(short),1,result.fpout);
		    sval = rear_diode.ycoord;
		    fwrite(&sval,sizeof(short),1,result.fpout);
		}
		/*
		if(verbose){
		    fprintf(stderr,
		    "diode %lu %3hd,%3hd %3hd,%3hd\n",
		    front_time,
		    front_diode.xcoord,
		    front_diode.ycoord,
		    rear_diode.xcoord,
		    rear_diode.ycoord);
		}
		*/
	    }
	}
	precnum = recnum;
	recnum = (recnum + 1)%result.nframes;
    }
    if(verbose){
	fprintf(stderr,"Processed %d position records\n",count);
	fprintf(stderr,"Accepted %d points ; rejected %d(%g %%) \n",
	accepted,rejected,100.0*rejected/(accepted + rejected));
    }
    if(result.fpout) fclose(result.fpout);
    if(result.fpstatout) fclose(result.fpstatout);
    exit(0);
}
