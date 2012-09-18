#include "behav_ext.h"

void ScaleClusterBounds(result)
Result	*result;
{
int	i;
int	x,y;
    
    if(result->fcoord == NULL) return;
    for(i=0;i<result->ncoords;i++){
	ComputeCoordinates( (int)result->fcoord[i].x,
	(int)result->fcoord[i].y,
	result, &x,&y);
	result->fcoord[i].x = x;
	result->fcoord[i].y = y;
    }
}


int IsInPolygon(wx,wy,fcoord,ncoords)
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
	if((wy > fcoord[i].y && wy <= fcoord[i+1].y) ||
	(wy < fcoord[i].y && wy >= fcoord[i+1].y)){
	    /*
	    ** count those which are on the positive x side 
	    ** by computing the intercept.
	    ** find the x value of the line between the two fcoords
	    ** which is at wy
	    */
	    FY = (fcoord[i+1].y-wy)/(fcoord[i].y-wy);
	    if(FY == 1){
		    pcross++;
	    } else {
		bx = (fcoord[i].x*FY - fcoord[i+1].x)/(FY-1);

		if(bx >= wx){
		    pcross++;
		}
	    }
	}
    }
    if(i == ncoords-1){
	/*
	** compute the final point which closes the polygon
	*/
	if((wy > fcoord[i].y && wy <= fcoord[0].y) ||
	(wy < fcoord[i].y && wy >= fcoord[0].y)){
	    FY = (fcoord[0].y-wy)/(fcoord[i].y-wy);
	    if(FY == 1){
		    pcross++;
	    } else {
		bx = (fcoord[i].x*FY - fcoord[0].x)/(FY-1);
		if(bx > wx){
		    pcross++;
		}
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

/*
** test the x y coordinates to see whether they fall within the bounds
** specified in the results structure
** return 0 = out of bounds
** return 1 = in bounds
*/
int BoundsTest(result,x,y)
Result	*result;
int	x;
int	y;
{

    if(result == NULL) return(0);
    /*
    ** check for cluster bounds
    */
    if(result->ncoords > 0){
	return(IsInPolygon((float)x,(float)y,result->fcoord,
	    result->ncoords));
    }
    /*
    ** if no bounds are defined then return success always
    */
    if(result->hasbounds == 0) return(1);
    /*
    ** check the x bounds
    */
    if(result->bound_x1 > result->bound_x2){
	if(x > result->bound_x1 ||
	x < result->bound_x2){
	    /*
	    ** failed
	    */
	    return(0);
	}
    } else {
	if(x > result->bound_x2 ||
	x < result->bound_x1){
	    /*
	    ** failed
	    */
	    return(0);
	}
    }
    /*
    ** check the y bounds
    */
    if(result->bound_y1 > result->bound_y2){
	if(y > result->bound_y1 ||
	y < result->bound_y2){
	    /*
	    ** failed
	    */
	    return(0);
	}
    } else {
	if(y > result->bound_y2 ||
	y < result->bound_y1){
	    /*
	    ** failed
	    */
	    return(0);
	}
    }
    /*
    ** in bounds
    */
    return(1);
}

int RangeMapValue(result,x,y,mintheta,maxtheta)
Result	*result;
int	x;
int	y;
int	*mintheta;
int	*maxtheta;
{
int	xbin,ybin;

    if(result == NULL || mintheta == NULL || maxtheta == NULL) {
	return(0);
    }
    /*
    ** compute the angular range bins
    */
    xbin = x*result->angular_range_map->xsize/result->xsize;
    ybin = y*result->angular_range_map->ysize/result->ysize;
    /*
    ** check the coordinates for valid xy range
    */
    if((xbin >= 0) && (xbin < result->angular_range_map->xsize) &&
    (ybin >= 0) && (ybin < result->angular_range_map->ysize)){
	*mintheta = result->angular_range_map->grid[xbin][ybin].mintheta;
	*maxtheta = result->angular_range_map->grid[xbin][ybin].maxtheta;
	return(1);
    } else {
	/*
	** bogus
	*/
	return(0);
    }
}

int CheckTimestampRange(timestamp,result)
unsigned long	timestamp;
Result		*result;
{
TRange	*trange;

    if(result->trange == NULL) return(1);
    /*
    ** go through the time ranges and see whether it falls within one
    */
    for(trange=result->trange;trange;trange=trange->next){
	if(timestamp >= trange->tstart && timestamp <= trange->tend){
	    return(1);
	}
    }
    /*
    ** didnt fall within any of the time ranges so fail
    */
    return(0);
}

void ReadClusterBounds(result,fp)
Result	*result;
FILE	*fp;
{
char	line[200];
float	wx1,wy1;
int	p0,p1;
int	clusterid;
int	level;
int	count;

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


