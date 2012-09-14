#include "xview_ext.h"

DisplayFrame()
{
    /* 
    ** display the image according to the display mode
    */
    if(header || (PSStatus() == 0)){
	legend_foreground();
	image_background(); 
    }
    display_image(); 
}

DisplayNextFrame()
{
    /*
    ** get the next frame
    */
    LoadNextFrame();
    /*
    ** and display it
    */
    DisplayFrame();
}

display_image()
{
ContourList	contour_list;

    switch(V->display_mode) {
    case SURFACE :
	DrawSurface(image,xsize,ysize,V->xmax,V->ymax);
	break;
    case COLORSURFACE :
	DrawColorSurface(image,xsize,ysize,V->xmax,V->ymax);
	break;
    case FILLEDSURFACE :
	DrawFilledSurface(image,xsize,ysize,V->xmax,V->ymax);
	break;
    case COLORFILLEDSURFACE :
	DrawColorFilledSurface(image,xsize,ysize,V->xmax,V->ymax);
	break;
    case BOX:
	DrawBox(image,xsize,ysize,V->xmax,V->ymax);
	break;
    case FILLEDBOX:
	DrawFilledBox(image,xsize,ysize,V->xmax,V->ymax);
	break;
    case COLORBOX:
	DrawColorBox(image,xsize,ysize,V->xmax,V->ymax);
	break;
    case COLORCONTOUR:
	DrawColorBox(image,xsize,ysize,V->xmax,V->ymax);
    case CONTOUR :
	MakeContourList(ncontours,minval,maxval,&contour_list);
	DrawContours(image,xsize,ysize,V->xmax,V->ymax, &contour_list);
	break;
    case NUMBERBOX:
	DrawNumberBox(image,xsize,ysize,V->xmax,V->ymax);
	break;
    case POINT:
	DrawPoint(image,xsize,ysize,V->xmax,V->ymax);
	break;
    }
}

ClearImage()
{
    SetColor (G->background);
    /*
    ** G->imagetop = yb;
    ** G->imagebot = height+yb;
    */
    FilledBox (0,yb , width, height +yb);
}

image_background()
{
Coord	coord[5];
Coord	tcoord[5];
Coord	scoord[5];

    if(V->display_mode == SURFACE ||
    V->display_mode == COLORSURFACE ||
    V->display_mode == COLORFILLEDSURFACE ||
    V->display_mode == FILLEDSURFACE){
	SetColor (G->background);
	/*
	** draw the baseline plane
	*/
	/*
	** xmax ymax
	*/
	coord[0].x = xsize * V->xmax *0.7;
	coord[0].y = yb +ysize*SHIFT +V->ymax*ysize*0.7;

	/*
	** 0 ymax
	*/
	coord[1].x = 0;
	coord[1].y = yb +ysize*SHIFT + V->ymax * ysize*0.7;

	/*
	** 0 0
	*/
	coord[2].x = V->ymax * ysize / 3; 
	coord[2].y = yb +ysize*SHIFT;

	/*
	** xmax 0
	*/
	coord[3].x = xsize * V->xmax *0.7 + V->ymax * ysize / 3; 
	coord[3].y = yb +ysize*SHIFT;

	/*
	** xmax ymax
	*/
	coord[4].x = xsize * V->xmax *0.7;
	coord[4].y = yb +ysize*SHIFT +V->ymax*ysize*0.7;

	WorldTransform(coord,tcoord,5);
	ScreenTransform(tcoord,scoord,5);

	MultipleLines(coord,5);
    }
}

