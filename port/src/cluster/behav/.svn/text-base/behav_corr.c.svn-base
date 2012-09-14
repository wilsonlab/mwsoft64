#include "behav_ext.h"

/*
*************************************
**           CORE ROUTINE          **
*************************************
*/

void SkipSpikes(spike,nclusters,newptimestamp)
SpikeList	*spike;
int		nclusters;
unsigned long	newptimestamp;
{
int	i;
int	n;
unsigned long	stime;

    /*
    ** skip all of the spikes in this positional interval
    */
    for(n=0;n<nclusters;n++){
	for(i=spike[n].currentspike;i<spike[n].nspikes;i++){
	    /*
	    ** get the spike time and do any time shifting
	    ** requested
	    */
	    stime = spike[n].data[i].timestamp;
	    /*
	    ** test against the positional interval
	    ** i.e. did this spike occur between the last
	    ** measured position and the present position
	    */
	    if(stime >= newptimestamp) break;
	}
	/*
	** keep track of where we left off
	*/
	spike[n].currentspike = i;
    }
}



void ComputeCorrelate(plist,result,spike,nclusters)
PositionList	*plist;
Result		*result;
SpikeList	*spike;
int		nclusters;
{
unsigned long	ptimestamp;
unsigned long	stimestamp;
int 		interval;
unsigned long	stime;
unsigned long	ltime;
short	*xy;
int		x = INT_MIN,y = INT_MIN;			/* current position */
int		x1,y1;			/* current position of diode 1 */
int		x2,y2;			/* current position of diode 2 */
int		px = INT_MIN,py = INT_MIN;			/* prev position of diode 1 */
int		px2 = INT_MIN,py2 = INT_MIN;		/* prev position of diode 2 */
int		i,j;
unsigned long	startstamp;
int		prevdirection;		/* previous direction */
int		newdirection = INT_MIN;		/* current direction */
unsigned long	newptimestamp;		/* current timestamp */
int		d = INT_MIN;
int		n;
int		intervalcount;
double		corr = INT_MIN;
int		hasdirection;
int		deldir;
int		velocityang;		/* velocity angle of diode 1 */
int		velocityang2;		/* velocity angle of diode 2 */
double		velocitymag;		/* velocity magnitude of diode 1 */
double		velocitymag2;		/* velocity magnitude of diode 1 */
double		angvelocity;		/* angular velocity */
unsigned long 	gapstart;
unsigned long 	gaplen;
int		vdeldir;
int		vdeldir2;
unsigned long	dt;
int		flipped;
int		gapflipped;
int		vdiff;
int		vmagdiff;
int		firstvalid;
int		changedirection;
int		changeposition;
int		occupancydirection;
int		skip_position;
int		min,sec;
int		mintheta,maxtheta;

    startstamp = 0;
    stimestamp = 0;
    ptimestamp = 0;
    result->positionmissing = 0;
    result->directionmissing = 0;
    result->exceededradius = 0;
    result->gapflip = 0;
    result->corrected_flips = 0;
    result->corrected_gapflips = 0;
    result->consecutivemissing = 0;
    result->maxconsecutive = 0;
    result->validposition = 0;
    result->validdirection = 0;
    result->ndirectiongaps = 0;
    result->nofrontdiode = 0;
    result->nobackdiode = 0;
    prevdirection = 0;
    firstvalid = 1;
    bzero(result->vector,sizeof(float)*MAXTHETA);
    if(verbose){
	if(starttime == 0 && endtime == 0) {
	    fprintf(stderr,"using all positions\n");
	} else {
	    FormatTime(starttime,&min,&sec);
	    fprintf(stderr,"using positions between timestamp %lu (%d:%d)",
	    starttime,min,sec);
	    FormatTime(endtime,&min,&sec);
	    fprintf(stderr,"and %lu (%d:%d)\n",
	    endtime,min,sec);
	}
    }
    EvaluateBehavior(result,plist);

    /*
    ** go through each position entry
    */
    gapstart = plist->positionarray[0].timestamp;
    gaplen = 0;
    for(j=0;j<plist->npositions;j++){
	flipped = 0;
	gapflipped = 0;
	vdiff = 0;
	/*
	** get the timestamp for the positional information
	*/
	newptimestamp = plist->positionarray[j].timestamp;
	if(newptimestamp < starttime) continue;
	if((endtime > starttime) && (newptimestamp > endtime)) break;
	/*
	** check to see that the timestamp is within requested time
	** ranges
	*/
	if(CheckTimestampRange(newptimestamp,result) == 0){
	    /*
	    ** it isnt so dont process
	    */
	    continue;
	}
	/*
	** initialize the starting time stamp. This is used when 
	** temporal binning is selected to determine when a
	** specified interval has passed
	*/
	if(startstamp == 0){
	    startstamp = newptimestamp;
	}

	/*
	** test for a valid direction
	*/
	if(plist->positionarray[j].direction == INVALIDDIR){
	    hasdirection = 0;
	    result->directionmissing++;
	    result->consecutivemissing++;
	    /*
	    ** this terminates an occupancy bin but since the
	    ** current algoritm will produce an invalid position
	    ** condition if there is an invalid direction we will
	    ** leave the occupancy handling to the valid position test
	    */
	} else {
	    hasdirection = 1;
	    newdirection = plist->positionarray[j].direction;
	    result->validdirection++;
	}
	/*
	** keep track of the max gap length
	*/
	gaplen = newptimestamp - gapstart;
	if(result->consecutivemissing > result->maxconsecutive){
	    result->maxconsecutive = result->consecutivemissing;
	}
	/*
	** test for a valid position
	*/
	if(!plist->positionarray[j].valid){
	    result->positionmissing++;
	    /*
	    ** skip over spikes which occurred in this positional 
	    ** interval
	    */
	    SkipSpikes(spike,nclusters,newptimestamp);
	    /*
	    ** terminate an occupancy bin on the first missing position
	    */
	    if(!firstvalid && (result->consecutivemissing == 1)){
		ComputeOccupancy(result,px,py,
		prevdirection,newptimestamp,1);

	    }
	    /*
	    ** dont process this position
	    */
	    /*
	    ** if a rate file is being generated then
	    ** write out a null record
	    */
	    WriteRateFile(result,0,0,0,0,0.0);
	    /*
	    ** if a position file is being generated then
	    ** write out a null record
	    */
	    WritePositionFile(result,newptimestamp,0,0,0,0);
	    continue;
	} else {
	    /*
	    ** ok we have a valid position
	    */
	    result->validposition++;
	}
	if(firstvalid){
	    /*
	    ** begin the first occupancy with the first valid position
	    */
	    result->occupancystart = newptimestamp;
	}
	/*
	** reset the occupancy variables following a gap of any length
	*/
	if(result->consecutivemissing > 0){
	    /*
	    ** reset the occupancy variables
	    */
	    result->occupancystart = newptimestamp;
	    result->occupancyspikes = 0;
	    result->occx = px;
	    result->occy = py;
	    result->occdir = prevdirection;
	}

	/*
	** get the xy positional information
	*/
	xy = plist->positionarray[j].xy;
	/*
	** For display purposes, assume that xy(0,0) is at the lower
	** left corner
	** xy[0] = diode 1 x
	** xy[1] = diode 1 y
	** xy[2] = diode 2 x
	** xy[3] = diode 2 y
	*/

	/*
	** compute diode 1 coordinates in the grid coordinate system
	*/
	ComputeCoordinates((int)xy[0],(int)xy[1],result,&x1,&y1);
	/*
	** compute diode 2 coordinates in the grid coordinate system
	*/
	ComputeCoordinates((int)xy[2],(int)xy[3],result,&x2,&y2);
	switch(result->positioneval){
	case CENTROID:
	    /*
	    ** use center of diode 1 and 2 for positional information
	    */
	    /*
	    ** under the condition of a missing front or back diode
	    ** use the remaining diode
	    */
	    if((x1==0) && (y1==0)){
		x = x2;
		y = y2;
	    } else
	    if((x2==0) && (y2==0)){
		x = x1;
		y = y1;
	    } else {
		x = (x1 + x2)/2;
		y = (y1 + y2)/2;
	    }
	    break;
	case FRONT:
	    /*
	    ** use diode 1 for positional information
	    */
	    x = x1;
	    y = y1;
	    break;
	}
	/*
	** NOTE that x and y are now in the grid coordinate system
	*/
	/*
	** do some bounds testing (just to be safe)
	*/
	if((x < 0) || (x >= result->xsize)){
	    fprintf(stderr,"ERROR: x coord %d out of range\n",x);
	    WriteRateFile(result,0,0,0,0,0.0);
	    WritePositionFile(result,newptimestamp,0,0,0,0);
	    continue;
	}
	if((y < 0) || (y >= result->ysize)){
	    fprintf(stderr,"ERROR: y coord %d out of range\n",y);
	    WriteRateFile(result,0,0,0,0,0.0);
	    WritePositionFile(result,newptimestamp,0,0,0,0);
	    continue;
	}

	if(firstvalid){
	    /*
	    ** begin the first occupancy with the first valid position
	    */
	    result->occx = x;
	    result->occy = y;
	}

	/*
	** evaluate the size of the gap in valid positions/directions 
	** if the gap is too large then dont compute velocity since
	** the interval between the current and previous position
	** will be too large to make a valid velocity estimate 
	** The first position will also have no velocity estimate
	** due to lack of prior positional information
	*/
	if(firstvalid || (gaplen > maxgaplen)){
	    velocitymag = INVALIDMAG;
	    velocitymag2 = INVALIDMAG;
	    velocityang = INVALIDDIR;
	    velocityang2 = INVALIDDIR;
	    angvelocity = INVALIDANGV;
	} else {
	    dt = newptimestamp - ptimestamp;
	    if(dt <= TINYVAL){
		fprintf(stderr,"ERROR: dt=%lu %lu %lu\n",dt,newptimestamp,
		ptimestamp);
	    }
	    /*
	    ** compute the velocity vector using the previous valid
	    ** position
	    */
	    velocitymag = 1e4*sqrt((double)(x-px)*(x-px) + 
		(double)(y-py)*(y-py))/dt;
	    velocitymag2 = 1e4*sqrt((double)(x2-px2)*(x2-px2) + 
		(double)(y2-py2)*(y2-py2))/dt;
	    velocityang = ComputeAngle(x,y,px,py);
	    velocityang2 = ComputeAngle(x2,y2,px2,py2);
	    angvelocity = 1e4*SignedDeltaDirection(newdirection,prevdirection)/dt;
	}

	/*
	** note that the current evaluation of valid positions demands
	** that the direction also be valid so that this condition
	** will always be met...for now.
	*/
	if(hasdirection){
	    /*
	    ** look for flipped directions. i.e. directions which change
	    ** by more than FLIPANG degrees over a single positional sample
	    ** of 50 msec
	    */
	    deldir = DeltaDirection(newdirection,prevdirection);

	    /*
	    ** if there has been a large gap then test for
	    ** a flip through maxlookahead
	    */
	    if(firstvalid || (gaplen > maxgaplen)){
		result->gapflip++;
		if(!result->noflip && TestFlip(plist,newdirection,j+1,maxlookahead)){
		    /*
		    ** flip it
		    */
		    newdirection = Flip(newdirection);
		    flipped = 180;
		    gapflipped = 180;
		    result->corrected_flips++;
		    result->corrected_gapflips++;
		    /*
		    ** flip diodes positions
		    */
		    x = x2; y = y2;
		    x2 = x1; y2 = y1;
		    x1 = x; y2 = y;
		    /*
		    ** dont need to recompute velocity since it is
		    ** not valid after a gap
		    */
		}
	    } else 
	    /*
	    ** test to see whether this point should be flipped
	    ** Since the first point has no prior information with which
	    ** to compare, a flag designates whether it should be 
	    ** flipped or not
	    */
	    if(!result->noflip && (result->validdirection > 0) && (deldir > FLIPANG)){
		/*
		** flip it
		*/
		newdirection = Flip(newdirection);
		result->flipdirectionsum += 
		    DeltaDirection(newdirection,prevdirection);
		result->directionsum += 
		    DeltaDirection(newdirection,prevdirection);
		result->corrected_flips++;
		flipped = 180;
		/*
		** swap diode coordinates 
		*/
		x = x2; y = y2;
		x2 = x1; y2 = y1;
		x1 = x; y2 = y;
		/*
		** recompute velocity
		*/
		dt = newptimestamp - ptimestamp;
		if(dt <= TINYVAL){
		    fprintf(stderr,"ERROR: dt=%lu %lu %lu\n",dt,newptimestamp,
		    ptimestamp);
		}
		/*
		** compute the velocity vector using the previous valid
		** position
		*/
		velocitymag = 1e4*sqrt((double)(x-px)*(x-px) + 
		    (double)(y-py)*(y-py))/dt;
		velocitymag2 = 1e4*sqrt((double)(x2-px2)*(x2-px2) + 
		    (double)(y2-py2)*(y2-py2))/dt;
		velocityang = ComputeAngle(x,y,px,py);
		velocityang2 = ComputeAngle(x2,y2,px2,py2);
		angvelocity = 1e4*SignedDeltaDirection(newdirection,prevdirection)/dt;
	    } else {
		result->directionsum += deldir;
	    }
	    /*
	    ** compute discrepancy between velocity angle and directional
	    ** angle
	    */
	    if(!firstvalid && (velocityang != INVALIDDIR) && 
	    (velocityang2 != INVALIDDIR)){
		vdeldir = DeltaDirection(newdirection,velocityang);
		vdeldir2 = DeltaDirection(velocityang2,velocityang);
		result->sumvelocitydiff += vdeldir;
		result->nvelocityang++;
		/*
		** look for excessive discrepancies between velocity
		** angle and direction angle, but only use velocities
		** which indicate linear travel i.e. diode 1 and 2 moving
		** in the same direction
		*/
		if((vdeldir2 < LINEARANG) && (vdeldir > FLIPANG)){
		    vdiff = FLIPANG;
		    result->nvangdiff++;
		}
		/*
		** test for relative angular movement of diode 1 and 2
		** only look at cases showing angular movement
		*/
		if(vdeldir2 > LINEARANG){
		    if(velocitymag2 > DIODE_ANGFACTOR*velocitymag){
			result->nvmagok++;
		    } else {
			vmagdiff = FLIPANG;
			result->nvmagdiff++;
		    }
		}
	    }

	    /*
	    ** just being safe
	    */
	    if(newdirection < 0 || newdirection > 360){
		fprintf(stderr,"ERROR: out of bounds direction encountered\n");
		WriteRateFile(result,0,0,0,0,0.0);
		WritePositionFile(result,newptimestamp,0,0,0,0);
		continue;
	    }
	    /*
	    ** check for min velocity criterion
	    fprintf(stderr,"%ld %d %d %g\t%g\t",newptimestamp,x,y,velocitymag,result->minvelocity);
	    */
	    if((velocitymag >= 0) && ((velocitymag <= result->minvelocity) ||
	    (result->maxvelocity >= 0 && velocitymag > result->maxvelocity))){
		WriteRateFile(result,0,0,0,0,0.0);
		WritePositionFile(result,newptimestamp,0,0,0,0);
		/*
		fprintf(stderr,"DROP\n");
		*/
		continue;
	    }
	    /*
	    fprintf(stderr,"\n");
	    ** Compute which directional bin to use if saving as a vector.
	    */
#ifdef PREVDIR
	    /*
	    ** Note that we are using the previous direction and
	    ** not the newly computed newdirection for binning purposes.
	    ** This means that spikes between the previous direction and
	    ** the current direction are assigned to the previous direction.
	    */
	    d = ((float)prevdirection)*((float)result->dsize/MAXTHETA);
#else
	    /*
	    ** Note that we are using current direction for binning purposes.
	    ** This means that spikes between the previous direction and
	    ** the current direction are assigned to the current direction.
	    */
	    d = ((float)newdirection)*((float)result->dsize/MAXTHETA);
#endif
	    /*
	    ** count gaps
	    */
	    if(result->consecutivemissing){
		result->ndirectiongaps++;
	    }
	    result->consecutivemissing = 0;
	    /*
	    ** a valid direction resets the gap 
	    */
	    gaplen = 0;
	    gapstart = newptimestamp;
	}
	/*
	** get the direction at the beginning of the occupancy
	** It isnt used for anything yet but it will probably
	** come in useful
	*/
	if(result->occupancystart == newptimestamp){
	    occupancydirection = newdirection;
	}
	if(firstvalid){
	    /*
	    ** begin the first occupancy with the first valid position
	    */
	    result->occdir = newdirection;
	}
	/*
	** test for occupancy change
	*/
	if(newptimestamp != result->occupancystart){
	    if(prevdirection - result->occdir > result->occdirangle){
		/*
		** has it moved out of a directional bin yet?
		*/
		changedirection = 1;
	    } else {
		changedirection = 0;
	    }
	    if(sqrt((double)(x - result->occx)*(x -  result->occx) +
	    (double)(y - result->occy)*(y - result->occy)) > result->occradius){
		/*
		** has it moved out of the positional radius yet?
		*/
		changeposition = 1;
	    } else {
		changeposition = 0;
	    }
	    /*
	    ** occupancy changes with position or (optionally) directional
	    ** changes
	    */
	    if(changeposition || 
	    (changedirection && result->enable_occdirection_change)){
		/*
		** compute the occupancy statistics for the previous
		** occupancy location
		*/
		ComputeOccupancy(result,px,py,
		prevdirection,newptimestamp,0);
		/*
		** reset the occupancy variables
		*/
		result->occx = x;
		result->occy = y;
		result->occdir = prevdirection;
		result->occupancystart = newptimestamp;
		result->occupancyspikes = 0;
	    }
	}
	/*
	** 
	** output the direction
	*/
	if(result->fpdir){
	    fprintf(result->fpdir,
	    "%lu \t%d \t%d \t%d \t%d \t%d \t%g \t%g \t%d \t%d \t%d\n",
	    newptimestamp,
	    x,y,x2,y2,
	    newdirection,
	    angvelocity,
	    velocitymag,
	    velocityang,
	    flipped,
	    gapflipped
	    );
	}
	/*
	** if binning has been selected then test to see whether the
	** bin interval has passed
	*/
	if((result->binsize > 0) && 
	(newptimestamp -startstamp >= result->binsize)){
	    /*
	    **  save this bin interval
	    */
	    switch(result->savemode){
	    case VECTOR:
		WriteVector(result);
		break;
	    case GRID:
		WriteGrid(result);
		break;
	    }
	    /*
	    ** and reset things for the next interval
	    */
	    startstamp = newptimestamp;
	    ResetVectors(result);
	}
#ifdef NEW
	/*
	** check for multiple behavioral contingencies
	*/
	if(behav_contingencies){
	    for(i=0;i<ncontingencies;i++){
		intervalcount = 0;
		for(k=0;k<plist->npositions;k++){
		    /*
		    ** test current behavioral information against
		    ** time shifted behavioral information
		    */
		    stime = plist->positionarray[k].timestamp +
		    contingency->timeshift;
		    if(stime >= newptimestamp) break;
		    if(stime < newptimestamp && 
		    stime >= ptimestamp){
			if(
			plist->positionarray[j].xy[0];
			intervalcount++;
		    }
		}
	    }
	}
#endif
	/*
	** test for bounds
	*/
	skip_position = 0;
	if(result->hasbounds){
	    /*
	    ** check the bounds. 
	    */
	    if(!BoundsTest(result,x,y)){
		/*
		** if not within the bounds then dont analyze the position
		*/
		skip_position = 1;
	    }
	}
	if(!skip_position){
	    /*
	    ** If nclusters == 0 then no spike information is available so
	    ** just monitor position and direction independent of correlated
	    ** unit activity.
	    */
	    if(nclusters>0){
		/*
		** if it is the beginning of a bin interval then there wont
		** be any spikes (0 width and all) so dont bother
		*/
		if(startstamp != newptimestamp){
		    /*
		    ** count the number of position samples at the
		    ** current location and the number of direction
		    ** samples in the current direction
		    */
		    if(hasdirection){
			result->vectortotaln[d]++;
		    }
		    result->gridtotaln[x][y]++;
		    /*
		    ** count the total number of position samples
		    */
		    result->totaltime++;
		    /*
		    ** count the spikes which occurred between the last positional
		    ** timestamp and the current. This can be done with multiple
		    ** clusters in which correlated firing of multiple units
		    ** (with specifiable time delay) is monitored. This condition
		    ** is indicated by nclusters taking on a value > 1.
		    */
		    for(n=0;n<nclusters;n++){
			intervalcount = 0;
			ltime = 0;
			for(i=spike[n].currentspike;i<spike[n].nspikes;i++){
			    /*
			    ** get the spike time and do any time shifting
			    ** requested
			    */
			    stime = spike[n].data[i].timestamp;
			    /*
			    sindex = spike[n].data[i].index;
			    */
			    /*
			    ** test against the positional interval
			    ** i.e. did this spike occur between the last
			    ** measured position and the present position
			    */
			    if(stime >= newptimestamp) break;
			    /*
			    ** check to see that the timestamp is within 
			    ** the appropriate time range
			    */
			    if(stime < newptimestamp && 
			    stime >= ptimestamp &&
			    CheckTimestampRange(stime,result) != 0
			    ){
				/*
				** count spikes which fall within the positional
				** interval 
				*/
				if(result->spatial_measure == EVENTS){
				    intervalcount++;
				} else
				/*
				** compute interval. Note that more than a single
				** event per positional bin is required
				** for an interval to be evaluated
				*/
				if((result->spatial_measure == INTERVALS) && 
				(ltime > 0)){
				    /*
				    ** this gives integer value to all intervals
				    ** less than 1000 msec which should be
				    ** more than adequate given the 50 msec
				    ** positional sampling
				    */
				    interval = stime - ltime;
				    intervalcount += 10000/interval;
				} else
				/*
				** either accumulate the actual interval
				** measure or count events based on 
				** an interval threshold
				*/
				if((result->spatial_measure == INTERVALTHRESH) && 
				(ltime > 0)){
				    /*
				    ** this gives integer value to all intervals
				    ** less than 1000 msec which should be
				    ** more than adequate given the 50 msec
				    ** positional sampling
				    */
				    interval = stime - ltime; 
				    if(((result->lowerthresh == -1) || 
				    (interval > result->lowerthresh*10)) &&
				    ((result->upperthresh == -1) || 
				    (interval < result->upperthresh*10))){
					intervalcount++;
				    }
				}
				/*
				** if a bounding region was specified and
				** the current position falls within that
				** bound then output the index of the spike
				** event that occurred within the region
				*/
				if(result->fpbound){
				    /*
				    ** check the bounds. 
				    */
				    if(BoundsTest(result,x,y)){
					/*
					** if within the bounds then
					** output the spike time
					*/
					fwrite(&stime,sizeof(unsigned long),
					1,result->fpbound);
				    }
				}
				spike[n].cumspikes++;
			    }
			    ltime = stime;
			}
			if(n==0){
			    /*
			    ** if this is the first (and possibly only) cluster
			    ** then just add it in
			    */
			    corr = intervalcount;
			} else {
			    /*
			    ** otherwise compute the running correlation with
			    ** other clusters
			    */
			    corr *= intervalcount;
			}
			/*
			** keep track of where we left off
			*/
			spike[n].currentspike = i;
		    }
		    /*
		    ** count the number of non-zero correlational events
		    */
		    if(corr != 0){
			result->gridn[x][y]++;
			if(hasdirection){
			    result->vectorn[d]++;
			}
		    }
		    /*
		    ** if there is a limit on the occupancy time the
		    ** terminate the occupancy and count the spikes 
		    ** up to this point
		    */
		    if(result->limited_occupancy){
			if(newptimestamp - result->occupancystart <=
			result->limited_occupancy){
			    result->occupancyspikes += corr;
			}
		    } else {
			    result->occupancyspikes += corr;
		    }
		    /*
		    ** computing directional correlate?
		    */
		    if(hasdirection && (result->behavmode == DIRECTION)){
			/*
			** are we saving the directional information as a 
			** function of position over the specified 
			** angular region?
			*/
			if(result->savemode == GRID){
			    /*
			    ** does an angular range map exist?
			    ** if so then get the angular range for the
			    ** current position
			    */
			    if(result->angular_range_map){
				if(RangeMapValue(result,x,y,&mintheta,&maxtheta) == 0){
				    fprintf(stderr,
				    "ERROR: invalid angular range for position %d,%d\n",x,y);
				}
			    } else {
				/*
				** otherwise use the general
				** angular range limits
				*/
				mintheta = result->mintheta;
				maxtheta = result->maxtheta;
			    }
			    /*
			    ** check the direction to see whether it is
			    ** within the desired angular range 
			    */
			    if(mintheta > maxtheta){
				if((newdirection >= mintheta) ||
				(newdirection <= maxtheta)){
				    result->grid[x][y] += corr;
				    result->gridsumsqr[x][y] += corr*corr;
				    /*
				    ** write out the rate record
				    */
				    WriteRateFile(result,newptimestamp,x,y,newdirection,corr);
				} else {
				    WriteRateFile(result,0,0,0,0,0.0);
				}
			    } else
			    if((mintheta < maxtheta) &&
			    (newdirection >= mintheta) && 
			    (newdirection <= maxtheta)){
				result->grid[x][y] += corr;
				result->gridsumsqr[x][y] += corr*corr;
				/*
				** write out the rate record
				*/
				WriteRateFile(result,newptimestamp,x,y,newdirection,corr);
			    } else {
				WriteRateFile(result,0,0,0,0,0.0);
			    }
			} else 
			/*
			** or just the cumulative directional histogram
			*/
			if(result->savemode == VECTOR){
			    result->vector[d] += corr;
			    result->vectorsumsqr[d] += corr*corr;
			}
		    } else {
			/*
			** positional correlate
			*/
			if((result->behavmode == ANGVELOCITY) && 
			(result->savemode == RAW)){
			    if(angvelocity != INVALIDANGV){
				fprintf(result->fp,"%lu %d %d %g %g\n",
				newptimestamp,x,y,angvelocity,corr);
			    }
			} else
			if((result->behavmode == VELOCITY) && 
			(result->savemode == RAW)){
			    if(velocitymag != INVALIDMAG){
				fprintf(result->fp,"%lu %d %d %g %g\n",
				newptimestamp,x,y,velocitymag,corr);
			    }
			} else {
			    result->grid[x][y] += corr;
			    result->gridsumsqr[x][y] += corr*corr;
			}
			/*
			** write out the rate record
			*/
			WriteRateFile(result,newptimestamp,x,y,newdirection,corr);
		    }
		    WritePositionFile(result,newptimestamp,x,y,x2,y2);
		}
	    } else {
		/*
		** no spike information to correlate with, therefore just
		** accumulate behavioral information
		*/
		if(result->behavmode == VELOCITY){
		    if(velocitymag > TINYVAL){
			if(result->savemode == RAW){
			    fprintf(result->fp,"%lu %d %d %d %d %d %g\n",
			    newptimestamp,x,y,x2,y2,newdirection,velocitymag);
			} 
			result->gridtotaln[x][y]++;
			result->totaltime++;
			if(hasdirection){
			    result->vectortotaln[d]++;
			}
			corr = velocitymag;
			if(velocitymag > MAXVELOCITY){
			    fprintf(stderr,"v=%g\n",velocitymag);
			}
		    } else {
			corr = 0;
		    }
		} else 
		if(result->behavmode == ANGVELOCITY){
		    if(angvelocity != INVALIDANGV){
			if(result->savemode == RAW){
			    fprintf(result->fp,"%lu %d %d %d %d %d %g\n",
			    newptimestamp,x,y,x2,y2,newdirection,angvelocity);
			} 
			result->gridtotaln[x][y]++;
			result->totaltime++;
			if(hasdirection){
			    result->vectortotaln[d]++;
			}
			corr = angvelocity;
		    } else {
			corr = 0;
		    }
		} else {
		    corr = 1;
		    result->gridtotaln[x][y] = 1;
		    result->totaltime++;
		    if(hasdirection){
			result->vectortotaln[d] = 1;
		    }
		}
		if(hasdirection && (result->behavmode == DIRECTION)){
		    /*
		    ** directional information
		    */
		    if(result->savemode == GRID){
			/*
			** as a function of position over an angular region
			*/
			/*
			** does an angular range map exist?
			** if so then get the angular range for the
			** current position
			*/
			if(result->angular_range_map){
			    if(RangeMapValue(result,x,y,&mintheta,&maxtheta) == 0){
				fprintf(stderr,
				"ERROR: invalid angular range for position %d,%d\n",x,y);
			    }
			} else {
			    /*
			    ** otherwise use the general
			    ** angular range limits
			    */
			    mintheta = result->mintheta;
			    maxtheta = result->maxtheta;
			}
			if(mintheta > maxtheta){
			    if((newdirection >= mintheta) ||
			    (newdirection <= maxtheta)){
				result->grid[x][y] += corr;
				result->gridsumsqr[x][y] += corr*corr;
			    }
			} else
			if((mintheta < maxtheta) &&
			(newdirection >= mintheta) && 
			(newdirection <= maxtheta)){
			    result->grid[x][y] += corr;
			    result->gridsumsqr[x][y] += corr*corr;
			}
		    } else 
		    if(result->savemode == VECTOR){
			/*
			** cumulative direction
			*/
			result->vector[d] += corr;
		    }
		} else {
		    /*
		    ** positional information
		    */
		    result->grid[x][y] += corr;
		    result->gridsumsqr[x][y] += corr*corr;
		}
		WritePositionFile(result,newptimestamp,x,y,x2,y2);
	    }
	} else {
	    /*
	    ** write out a null rate record
	    */
	    WriteRateFile(result,0,0,0,0,0.0);
	    WritePositionFile(result,newptimestamp,0,0,0,0);
	}
	if(hasdirection){
	    prevdirection = newdirection;
	}
	ptimestamp = newptimestamp;
	px = x; py = y;
	px2 = x2; py2 = y2;
	firstvalid = 0;
    }
    if(verbose){
	PrintResults(result,plist);
	/*
	** if there were spikes then perform statistical analysis
	** of spatial bias
	*/
	if(nclusters > 0){
/*	    SpatialBiasStatistics(result); */
	}
    }
    switch(result->savemode){
    case VECTOR:
	WriteVector(result);
	break;
    case GRID:
	WriteGrid(result);
	break;
    }
}
