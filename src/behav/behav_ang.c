#include "behav_ext.h"

int32_t DeltaDirection(d1,d2)
int32_t	d1,d2;
{
int32_t	deldir;

    deldir = abs(d1 - d2);
    /*
    ** correct for modulo 180 
    */
    if(deldir > 180){
	deldir = 360 - deldir;
    }
    return(deldir);
}

int32_t SignedDeltaDirection(d1,d2)
int32_t	d1,d2;
{
int32_t	deldir;

    deldir = d1 - d2;
    /*
    ** correct for modulo 180 
    */
    if(deldir < -180){
	deldir = 360 + deldir;
    } else 
    if(deldir > 180){
	deldir = deldir - 360;
    }
    return(deldir);
}

int32_t ComputeAngle(x1,y1,x2,y2)
int32_t	x1,y1;
int32_t	x2,y2;
{
int32_t	newdirection;

    if((x1 == x2) && (y1 == y2)){
	return(INVALIDDIR);
    }
    /*
    ** compute direction based on angle between xy position pairs
    **
    ** compute the angle assuming diode 1 is in front (vector head)
    */
    if((x1 == x2) && (y1 - y2 > 0)){
	/*
	** lets not upset atan 
	*/
	newdirection = 90;
    } else 
    if((x1 == x2) && (y1 - y2 < 0)){
	newdirection = -90;
    } else {
	/*
	** compute the angle in degrees
	*/
	newdirection = MAXTHETA*atan(((double)y1 - (double)y2)/
	((double)x1 - (double)x2))/(2*M_PI);
    }
    /*
    ** correct the angle to reflect the true 360 degree value
    */
    if((x1 - x2) < 0){ 		/* quadrants 2 and 3 */
	newdirection += 180;
    } else
    if((y1 - y2) <= 0){		/* quadrant 4 */
	newdirection += 360;
    }
    /*
    ** make sure the direction is modulo 360
    */
    if(newdirection > 360){
	newdirection -= 360;
    }
    /*
    ** just being safe
    */
    if(newdirection < 0 || newdirection > 360){
	fprintf(stderr,
	"ERROR: ComputeAngle: out of bounds direction encountered: %d (%d,%d)(%d,%d)\n",
	newdirection,x1,y1,x2,y2);
    }
    return(newdirection);
}

