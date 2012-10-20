#include "xview_ext.h"

/*
** Matt Wilson 1988 - Xview contour algorithm
** this code draws contours through a 3 dimensional plane
** defined by 3 points
** it requires global variables :
**	minval,maxval
** (just for variable line widths)
*/

/*
** interpolates x and y values from points p1,p2 given a z value
** and returns the result in p
*/
void Interpolate(p1,p2,z,p)
Point	p1,p2;
float	z;
Point	*p;
{
float	scale;

    if(p1.z == p2.z){
	printf("divide by zero\n");
	return;
    }
    scale = (z-p1.z)/(p2.z - p1.z);
    p->x = scale*(p2.x - p1.x) + p1.x;
    p->y = scale*(p2.y - p1.y) + p1.y;
}

/*
** draws a contour line of value through the plane defined by points p1,p2,p3
*/
void ContourLine(p1,p2,p3,value)
Point	p1,p2,p3;
float	value;
{
Point	endpoint[3];
int	index;
float	z1,z2,z3;
extern int	line_width;

    index = 0;
    z1 = p1.z;
    z2 = p2.z;
    z3 = p3.z;
    /*
    ** determine which boundary lines the contour
    ** will intersect
    */
    if((value >= z1 && value <= z2) || (value <= z1 && value >= z2)){
	if(z1 != z2){
	    /*
	    ** compute the endpoints of the line by
	    ** calculating the intersection of the
	    ** line with the plane boundaries
	    */
	    Interpolate(p1,p2,value,&endpoint[index]);
	    index++;
	}
    }
    if((value >= z2 && value <= z3) || (value <= z2 && value >= z3)){
	if(z2 != z3){
	    Interpolate(p2,p3,value,&endpoint[index]);
	    index++;
	}
    }
    if((value >= z1 && value <= z3) || (value <= z1 && value >= z3)){
	if(z1 != z3){
	    Interpolate(p1,p3,value,&endpoint[index]);
	    index++;
	}
    }

    if(index < 1){
    /*
	printf("underspecified contour\n");
	*/
	return;
    }
    if(index > 2){
    /*
	printf("overspecified contour\n");
	*/
	return;
    }
    /*
    line_width = 20*(value - minval)/(maxval - minval);
    if(line_width <= 0) line_width = 1;
    */

    DrawLine((int)endpoint[0].x,(int)endpoint[0].y,
    (int)endpoint[1].x, (int)endpoint[1].y);
}

/*
** draws contour lines through the plane defined by points p1,p2,p3
** the contour values are obtained from the contour list.
*/
DoContours(p1,p2,p3,contour_list)
Point	p1,p2,p3;
ContourList	*contour_list;
{
float	cscale;
int	n;
float	value;
float	zmax,zmin;

    if(p1.z == p2.z && p2.z == p3.z) return;
    zmin = zmax = p1.z;
    if(zmin > p2.z) zmin = p2.z;
    if(zmin > p3.z) zmin = p3.z;
    if(zmax < p2.z) zmax = p2.z;
    if(zmax < p3.z) zmax = p3.z;

    for(n=0;n<contour_list->ncontours;n++){
	value = contour_list->value[n];
	if(value > zmax) break;
	if(value < zmin) continue;
	ContourLine(p1,p2,p3,value);
    }
}

MakeContourList(ncontours,minval,maxval,contour_list)
int	ncontours;
float	minval;
float	maxval;
ContourList	*contour_list;
{
int	n;
float	cscale;

    /*
    ** make the contour list
    */
    if((contour_list->ncontours = ncontours) > MAXCONTOURS){
	contour_list->ncontours = MAXCONTOURS;
    };
    cscale = (maxval - minval)/contour_list->ncontours;
    for(n=0;n<contour_list->ncontours;n++){
	contour_list->value[n] = minval + n*cscale;
    }
}

DrawContours(image,xsize,ysize,xmax,ymax,contour_list)
Image	*image;
float	xsize,ysize;
ContourList	*contour_list;
{
Point	pt[5];
Image	*imageptr;
float	x,y;
int	i,j;

    SetColor (G->foreground);
    imageptr = image + (xmax +1)*(ymax +1) - 1;
    for (j = 0; j < ymax; j++) {
	y = (j + .5)*ysize + yb;
	for (i =xmax; i > 0; i--) {
	    x = (i + .5)*xsize;
	    /*
	    ** compute the surface plane by triangulating the
	    ** surface matrix
	    **            1     0
	    **            o------o xmax,ymax
	    **            |\     |
	    **            | \    |
	    **            |  \   |
	    **            |   \  |
	    **            |    \ |
	    **            o----- o
	    **            2     3
	    */
	    /*
	    ** alternate triangulation involving one interpolated point
	    ** has the advantage of symmetry
	    **            1     0
	    **            o------o xmax,ymax
	    **            |\   / |
	    **            | \ /  |
	    **            |  o 4 |
	    **            | / \  |
	    **            |/   \ |
	    **            o----- o
	    **            2     3
	    */
	    /*
	    ** calculate the points
	    */
	    pt[0].x = x;
	    pt[0].y = y;
	    pt[0].z = minval + scale*imageptr->value;

	    pt[1].x = x - xsize;
	    pt[1].y = y;
	    pt[1].z = minval + scale*(imageptr -1)->value;

	    pt[2].x = x - xsize;
	    pt[2].y = y +ysize;
	    pt[2].z = minval + scale*(imageptr -1 -(xmax+1))->value;

	    pt[3].x = x;
	    pt[3].y = y + ysize;
	    pt[3].z = minval + scale*(imageptr - (xmax +1))->value;

	    if(representation == 0){
		DoContours(pt[0],pt[1],pt[3],contour_list);
		DoContours(pt[1],pt[2],pt[3],contour_list);
	    } else {
		/*
		** this is the alternate surface
		*/
		pt[4].x = x - xsize/2;
		pt[4].y = y + ysize/2;
		pt[4].z = 0.25*(pt[0].z + pt[1].z + pt[2].z + pt[3].z);

		DoContours(pt[0],pt[1],pt[4],contour_list);
		DoContours(pt[1],pt[2],pt[4],contour_list);
		DoContours(pt[2],pt[3],pt[4],contour_list);
		DoContours(pt[0],pt[3],pt[4],contour_list);
	    }

	    imageptr--;
	}
	imageptr--;
    }
}
