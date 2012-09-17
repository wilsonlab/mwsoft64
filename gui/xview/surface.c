#include "xview_ext.h"

Coord	*coord;
Coord	*coord2;

CoordAlloc(xmax,ymax)
{
    V->cellnum = (V->xmax +1)*(V->ymax+1);
    coord = (Coord *) malloc (V->cellnum*5 * sizeof (Coord));
    coord2 = (Coord *) malloc (V->cellnum*5 * sizeof (Coord));
}

CoordFree()
{
    free(coord);
    free(coord2);
}

DrawSurface(image,xsize,ysize,xmax,ymax)
Image	*image;
float	xsize;
float	ysize;
int	xmax;
int	ymax;
{
Image	*imageptr;
int	i,j;
float	x,y;
Coord	*coord_ptr;
int	coord_count;

    imageptr = image + (xmax +1)*(ymax+1) -1;

    CoordAlloc(xmax,ymax);
    coord_ptr = coord;
    coord_count = 0;
    SetColor(G->foreground);
    for (j = 0; j <=ymax; j++) {
	coord_ptr->x = xsize * xmax *0.7 + (ymax-j) * ysize / 3; 
	coord_ptr->y =  yb +ysize*SHIFT + 
	    j * ysize *0.7 - imageptr->value*zsize;

	/*
	ScreenTransform(0.0,(float)j,imageptr->value,coord_ptr);
	*/
	/* dont connect */
	if(j > 0){
	    DrawLine(coord_ptr->x,coord_ptr->y,
	    (coord_ptr - xmax - 1)->x,
	    (coord_ptr - xmax - 1)->y);
	}
	coord_ptr++;
	coord_count++;
	imageptr--;
	for (i =xmax-1; i >= 0; i--) {
	    coord_ptr->x = xsize * i *0.7 + (ymax-j) * ysize / 3; 
	    coord_ptr->y =  yb +ysize*SHIFT + 
		j * ysize *0.7 - imageptr->value*zsize;

	    if(j > 0){
		DrawLine(coord_ptr->x,coord_ptr->y,
		(coord_ptr - xmax - 1)->x,
		(coord_ptr - xmax - 1)->y);
	    }
	    imageptr--;
	    coord_ptr++;
	    coord_count++;
	}
    }
    for (j = 0; j <=ymax; j++) {
	MultipleLines (coord+j*(xmax+1), (xmax+1)); 
    }
    /* MultipleLines (coord, coord_count); */
    CoordFree();
}

DrawColorSurface(image,xsize,ysize,xmax,ymax)
Image	*image;
float	xsize;
float	ysize;
int	xmax;
int	ymax;
{
Image	*imageptr;
int	i,j;
float	x,y;
Coord	*coord_ptr;

    CoordAlloc(xmax,ymax);
    imageptr = image + (xmax +1)*(ymax+1) -1;
    SetColor (G->background);
    coord_ptr = coord;
    for (j = 0; j <=ymax; j++) {
	coord_ptr->x = xsize * xmax *0.7 + (ymax-j) * ysize / 3; 
	coord_ptr->y =  yb +ysize*SHIFT + 
	    j * ysize *0.7 - imageptr->value*zsize;
	    /* dont connect */
	if(j > 0){
	    SetColor ((int) (color_scale * (imageptr->value) + color_min));
	    DrawLine(coord_ptr->x,coord_ptr->y,
	    (coord_ptr - xmax - 1)->x,
	    (coord_ptr - xmax - 1)->y);
	}
	coord_ptr++;
	imageptr--;
	for (i =xmax-1; i >= 0; i--) {
	    SetColor ((int) (color_scale * (imageptr->value) + color_min));
	    coord_ptr->x = xsize * i *0.7 + (ymax-j) * ysize / 3; 
	    coord_ptr->y =  yb +ysize*SHIFT + 
		j * ysize *0.7 - imageptr->value*zsize;

	    if(j > 0){
		DrawLine(coord_ptr->x,coord_ptr->y,
		(coord_ptr - xmax - 1)->x,
		(coord_ptr - xmax - 1)->y);
	    }
	    DrawLine(coord_ptr->x,coord_ptr->y,
	    (coord_ptr - 1)->x,
	    (coord_ptr - 1)->y);
	    imageptr--;
	    coord_ptr++;
	}
    }
    CoordFree();
}

DrawFilledSurface(image,xsize,ysize,xmax,ymax)
Image	*image;
float	xsize;
float	ysize;
int	xmax;
int	ymax;
{
Image	*imageptr;
int	i,j;
float	x,y;
Coord	*coord_ptr;
Coord	*start_ptr;

    CoordAlloc(xmax,ymax);
    imageptr = image + (xmax +1)*(ymax+1) -1;
    /*
    ** calculate the coords
    */
    coord_ptr = coord;
    for (j = 0; j <=ymax; j++) {
	coord_ptr->x = xsize * xmax *0.7 + (ymax-j) * ysize / 3; 
	coord_ptr->y =  yb +ysize*SHIFT + 
	    j * ysize *0.7 - imageptr->value*zsize;
	    /* dont connect */
	coord_ptr++;
	imageptr--;
	for (i =xmax-1; i >= 0; i--) {
	    coord_ptr->x = xsize * i *0.7 + (ymax-j) * ysize / 3; 
	    coord_ptr->y =  yb +ysize*SHIFT + 
		j * ysize *0.7 - imageptr->value*zsize;

	    imageptr--;
	    coord_ptr++;
	}
    }
    /*
    ** draw the lines and polygons
    */
    coord_ptr = coord;
    for (j = 0; j <=ymax; j++) {
	if(j > 0){
	    DrawLine(coord_ptr->x,coord_ptr->y,
	    (coord_ptr - xmax - 1)->x,
	    (coord_ptr - xmax - 1)->y);
	}
	coord_ptr++;
	start_ptr = coord_ptr;
	/*
	** draw a row of polygons
	*/
	for (i =xmax-1; i >= 0; i--) {
	    /*
	    ** draw the box
	    */
	    if(j > 0){
		coord2[0].x = coord_ptr->x;
		coord2[0].y = coord_ptr->y;
		coord2[1].x = (coord_ptr - 1)->x;
		coord2[1].y = (coord_ptr - 1)->y;
		coord2[2].x = (coord_ptr -xmax - 2)->x;
		coord2[2].y = (coord_ptr -xmax - 2)->y;
		coord2[3].x = (coord_ptr -xmax - 1)->x;
		coord2[3].y = (coord_ptr -xmax - 1)->y;
		coord2[4].x = coord_ptr->x;
		coord2[4].y = coord_ptr->y;

		SetColor (G->background);
		FilledPoly (coord2, 5);

	    } 
	    coord_ptr++;
	}
	/*
	** draw the surface lines for the row
	*/
	coord_ptr = start_ptr;
	for (i =xmax-1; i >= 0; i--) {
	    /*
	    ** draw the box
	    */
	    if(j > 0){
		SetColor(G->foreground);
		/*
		** draw the cross line
		*/
		DrawLine(coord_ptr->x,coord_ptr->y,
		(coord_ptr - xmax - 1)->x,
		(coord_ptr - xmax - 1)->y);
		/*
		** draw the raster line
		*/
		DrawLine(
		(coord_ptr - xmax - 2)->x,
		(coord_ptr - xmax - 2)->y,
		(coord_ptr - xmax - 1)->x,
		(coord_ptr - xmax - 1)->y);
	    } 
	    if(j == ymax ){
		SetColor(G->foreground);
		DrawLine(coord_ptr->x,coord_ptr->y,
		(coord_ptr - 1)->x,
		(coord_ptr - 1)->y);
	    }
	    coord_ptr++;
	}
    }
    CoordFree();
}

#ifdef OLD
DrawColorFilledSurface(image,xsize,ysize,xmax,ymax)
Image	*image;
float	xsize;
float	ysize;
int	xmax;
int	ymax;
{
Image	*imageptr;
int	i,j;
float	x,y;
Coord	*coord_ptr;

    CoordAlloc(xmax,ymax);
    imageptr = image + (xmax +1)*(ymax+1) -1;
    SetColor (G->background);
    coord_ptr = coord;
    for (j = 0; j <=ymax; j++) {
	coord_ptr->x = xsize * xmax *0.7 + (ymax-j) * ysize / 3; 
	coord_ptr->y =  yb +ysize*SHIFT + 
	    j * ysize *0.7 - imageptr->value*zsize;
	    /* dont connect */
	if(j > 0){
	    DrawLine(coord_ptr->x,coord_ptr->y,
	    (coord_ptr - xmax - 1)->x,
	    (coord_ptr - xmax - 1)->y);
	}
	coord_ptr++;
	imageptr--;
	for (i =xmax-1; i >= 0; i--) {
	    coord_ptr->x = xsize * i *0.7 + (ymax-j) * ysize / 3; 
	    coord_ptr->y =  yb +ysize*SHIFT + 
		j * ysize *0.7 - imageptr->value*zsize;

	    /*
	    ** draw the box
	    */
	    if(j > 0){
		coord2[0].x = coord_ptr->x;
		coord2[0].y = coord_ptr->y;
		coord2[1].x = (coord_ptr - 1)->x;
		coord2[1].y = (coord_ptr - 1)->y;
		coord2[2].x = (coord_ptr -xmax - 2)->x;
		coord2[2].y = (coord_ptr -xmax - 2)->y;
		coord2[3].x = (coord_ptr -xmax - 1)->x;
		coord2[3].y = (coord_ptr -xmax - 1)->y;
		coord2[4].x = coord_ptr->x;
		coord2[4].y = coord_ptr->y;

		SetColor ((int) (color_scale * 
		    ((imageptr + xmax + 1)->value) + color_min));
		FilledPoly (coord2, 5);

		SetColor(G->background);
		/*
		** draw the cross line
		*/
		DrawLine(coord_ptr->x,coord_ptr->y,
		(coord_ptr - xmax - 1)->x,
		(coord_ptr - xmax - 1)->y);
		/*
		** draw the raster line
		*/
		DrawLine( (coord_ptr - xmax - 2)->x,(coord_ptr - xmax - 2)->y,
		(coord_ptr - xmax - 1)->x,(coord_ptr - xmax - 1)->y);
	    } 
	    if(j == ymax ){
		SetColor(G->background);
		DrawLine(coord_ptr->x,coord_ptr->y,
		(coord_ptr - 1)->x,
		(coord_ptr - 1)->y);
	    }
	    imageptr--;
	    coord_ptr++;
	}
    }
    CoordFree();
}
#endif

DrawColorFilledSurface(image,xsize,ysize,xmax,ymax)
Image	*image;
float	xsize;
float	ysize;
int	xmax;
int	ymax;
{
Image	*imageptr;
int	i,j;
float	x,y;
Coord	*coord_ptr;
Coord	*start_ptr;

    CoordAlloc(xmax,ymax);
    imageptr = image + (xmax +1)*(ymax+1) -1;
    /*
    ** calculate the coords
    */
    coord_ptr = coord;
    for (j = 0; j <=ymax; j++) {
	coord_ptr->x = xsize * xmax *0.7 + (ymax-j) * ysize / 3; 
	coord_ptr->y =  yb +ysize*SHIFT + 
	    j * ysize *0.7 - imageptr->value*zsize;
	    /* dont connect */
	coord_ptr++;
	imageptr--;
	for (i =xmax-1; i >= 0; i--) {
	    coord_ptr->x = xsize * i *0.7 + (ymax-j) * ysize / 3; 
	    coord_ptr->y =  yb +ysize*SHIFT + 
		j * ysize *0.7 - imageptr->value*zsize;

	    imageptr--;
	    coord_ptr++;
	}
    }
    /*
    ** draw the lines and polygons
    */
    coord_ptr = coord;
    imageptr = image + (xmax +1)*(ymax+1) -1;
    for (j = 0; j <=ymax; j++) {
	if(j > 0){
	    DrawLine(coord_ptr->x,coord_ptr->y,
	    (coord_ptr - xmax - 1)->x,
	    (coord_ptr - xmax - 1)->y);
	}
	coord_ptr++;
	start_ptr = coord_ptr;
	imageptr--;
	/*
	** draw a row of polygons
	*/
	for (i =xmax-1; i >= 0; i--) {
	    /*
	    ** draw the box
	    */
	    if(j > 0){
		coord2[0].x = coord_ptr->x;
		coord2[0].y = coord_ptr->y;
		coord2[1].x = (coord_ptr - 1)->x;
		coord2[1].y = (coord_ptr - 1)->y;
		coord2[2].x = (coord_ptr -xmax - 2)->x;
		coord2[2].y = (coord_ptr -xmax - 2)->y;
		coord2[3].x = (coord_ptr -xmax - 1)->x;
		coord2[3].y = (coord_ptr -xmax - 1)->y;
		coord2[4].x = coord_ptr->x;
		coord2[4].y = coord_ptr->y;

		SetColor ((int) (color_scale * 
		    ((imageptr + xmax + 1)->value) + color_min));
		FilledPoly (coord2, 5);

	    } 
	    coord_ptr++;
	    imageptr--;
	}
	/*
	** draw the surface lines for the row
	*/
	coord_ptr = start_ptr;
	for (i =xmax-1; i >= 0; i--) {
	    /*
	    ** draw the box
	    */
	    if(j > 0){
		SetColor(G->foreground);
		/*
		** draw the cross line
		*/
		DrawLine(coord_ptr->x,coord_ptr->y,
		(coord_ptr - xmax - 1)->x,
		(coord_ptr - xmax - 1)->y);
		/*
		** draw the raster line
		*/
		DrawLine(
		(coord_ptr - xmax - 2)->x,
		(coord_ptr - xmax - 2)->y,
		(coord_ptr - xmax - 1)->x,
		(coord_ptr - xmax - 1)->y);
	    } 
	    if(j == ymax ){
		SetColor(G->foreground);
		DrawLine(coord_ptr->x,coord_ptr->y,
		(coord_ptr - 1)->x,
		(coord_ptr - 1)->y);
	    }
	    coord_ptr++;
	}
    }
    CoordFree();
}
