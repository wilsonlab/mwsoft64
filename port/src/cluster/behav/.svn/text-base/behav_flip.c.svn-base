#include "behav_ext.h"

void EvaluateBehavior(result,plist)
Result		*result;
PositionList	*plist;
{
int		newdirection;
int		j;
short 		*xy;
int		dx,dy;
int		rad;
int		x1,x2,y1,y2;
short		noback,nofront;

    /*
    ** evaluate valid directions and positions 
    */
    for(j=0;j<plist->npositions;j++){
	if(plist->positionarray[j].timestamp < starttime) continue;
	if((endtime > starttime) && 
	(plist->positionarray[j].timestamp > endtime)) break;
	/*
	** check to see that the timestamp is within requested time
	** ranges
	*/
	if(CheckTimestampRange(plist->positionarray[j].timestamp,result) == 0){
	    /*
	    ** it isnt so dont process
	    */
	    continue;
	}
	/*
	** count the processed positions
	*/
	plist->nprocessedpositions++;
	/*
	** get the xy positional information
	*/
	xy = plist->positionarray[j].xy;
	/*
	** assume a valid position until proven otherwise
	*/
	plist->positionarray[j].valid = 1;
	noback = 0;
	nofront = 0;
	/*
	** update the positional count at the grid point specified
	** by the positional data
	** For display purposes, assume that xy(0,0) is at the lower
	** left corner
	** xy[0] = diode 1 x
	** xy[1] = diode 1 y
	** xy[2] = diode 2 x
	** xy[3] = diode 2 y
	**
	** use diode 1 as the positional indicator
	*/
	x1 = xy[0];
	y1 = xy[1];
	x2 = xy[2];
	y2 = xy[3];
	if((x1 == 0) && (y1 == 0)){
	    /*
	    ** (0,0) on diode 1 indicates an invalid point
	    ** dont process this position or direction
	    */
	    plist->positionarray[j].direction = INVALIDDIR;
	    if(!result->ignore_missing_front_diode){
		plist->positionarray[j].valid = 0;
	    }
	    nofront = 1;
	    result->nofrontdiode++;
	    continue;
	}
	/*
	** check for missing xy positional information on diode 2
	*/
	if((x2 == 0) && (y2 == 0)){
	    /*
	    ** (0,0) on diode 2 means we cant compute direction
	    */
	    plist->positionarray[j].direction = INVALIDDIR;
	    /*
	    ** dont process this position either
	    */
	    if(!result->ignore_missing_back_diode){
		plist->positionarray[j].valid = 0;
	    }
	    noback = 1;
	    result->nobackdiode++;
	    continue;
	} 
	if(noback && nofront){
	    plist->positionarray[j].valid = 0;
	}
	/*
	** compute the separation between the two diodes
	*/
	dx = x1 - x2;
	dy = y1 - y2;
	rad = sqrt((double)(dx*dx + dy*dy)) + .5;
	/*
	** test for radial bounds
	*/
	if((rad > max_rad) || (rad < min_rad)){
	    /*
	    ** dont process this direction
	    */
	    plist->positionarray[j].direction = INVALIDDIR;
	    /*
	    ** dont process this position
	    */
	    plist->positionarray[j].valid = 0;
	    result->exceededradius++;
	    continue;
	} 
	newdirection = ComputeAngle(x1,y1,x2,y2);
	if(newdirection == INVALIDDIR &&
	    !(result->ignore_missing_front_diode ||
	    result->ignore_missing_back_diode)){
	    fprintf(stderr,"ERROR: invalid direction (%d,%d),(%d,%d)\n",x1,y1,x2,y2);
	}
	plist->positionarray[j].direction = newdirection;
    }
}

int Flip(direction)
int	direction;
{
    if(direction < 0){
	fprintf(stderr,"ERROR: Flip: invalid direction: %d\n",direction);
    }
    direction += 180;
    if(direction > 360){
	direction -= 360;
    }
    return(direction);
}

/*
** j is the index of the first direction in plist to test against
** which is usually one past testdirection
** lookahead is the number of positional intervals past j
** to compare against
*/
int CountFlips(j,testdirection,plist,lookahead)
int		j;
int		testdirection;
PositionList	*plist;
int		lookahead;
{
int	flipcount;
unsigned long	consecutive;
unsigned long	gapstart;
unsigned long	gaplen;
int	k;
int	currentdirection;
int	nvalid;
int	deldir;

    flipcount = 0;
    consecutive = 0;
    k = 0;
    nvalid = 0;
    gapstart = plist->positionarray[j].timestamp;
    gaplen = 0;
    while((nvalid < lookahead) && (j+k < plist->npositions)){
	if(plist->positionarray[j+k].direction != INVALIDDIR){
	    gaplen = plist->positionarray[j+k].timestamp - gapstart;
	    currentdirection = plist->positionarray[j+k].direction;
	    /*
	    ** look for gaps
	    */
	    if(gaplen > maxgaplen){
		/*
		** a gap results in an evaluation of flipping
		** independent of the direction prior to the gap
		** Therefore there is no need to look further if
		** we are interested in differences in flip count
		** based on earlier flip decisions
		*/
		break;
#ifdef OLD
		/*
		** decide whether flipping it would result in
		** fewer future flips
		*/
		if(TestFlip(plist, currentdirection, j+k+1,lookahead-nvalid)){
		    /*
		    ** flip it
		    */
		    testdirection = Flip(currentdirection);
		    flipcount++;
		} else {
		    testdirection = currentdirection;
		}
#endif
	    } else { 
		/*
		** look for angles which exceed the maximum acceptable
		*/
		deldir = DeltaDirection(testdirection,currentdirection);
		if(deldir > FLIPANG){
		    flipcount++;
		    testdirection = Flip(currentdirection);
		} else {
		    testdirection = currentdirection;
		}
	    }
	    gapstart = plist->positionarray[j+k].timestamp;
	    gaplen = 0;
	    /*
	    ** count the number of valid positions
	    */
	    nvalid++;
	}
	k++;
    }
    return(flipcount);
}

/*
** j is the index of the first direction in plist to test against
** which is usually one past newdirection
** lookahead is the number of positional intervals past j
** to compare against
*/
int TestFlip(plist,newdirection,j,lookahead)
PositionList	*plist;
int		newdirection;
int		j;
int		lookahead;
{
int	noflipcount;
int	flipcount;
int	testdirection;

    /*
    ** evaluate the number of flips which would occur
    ** lookahead number of point in the future
    ** if the current point were NOT flipped
    */
    testdirection = newdirection;
    noflipcount = CountFlips(j,testdirection,plist,lookahead);

    /*
    ** evaluate the number of flips which would occur
    ** lookahead number of point in the future
    ** if the current point were flipped
    */
    testdirection = Flip(newdirection);
    flipcount = CountFlips(j,testdirection,plist,lookahead);

    if(flipcount < noflipcount){
	/*
	** flip it
	*/
	return(1);
    } else {
	return(0);
    }
}

