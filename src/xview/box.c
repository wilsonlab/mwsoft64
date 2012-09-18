#include "xview_ext.h"

DrawColorBox(image,xsize,ysize,xmax,ymax)
Image	*image;
float	xsize;
float	ysize;
int32_t	xmax;
int32_t	ymax;
{
Image	*imageptr;
int32_t	i,j;
int32_t	x,y;

    imageptr = image + (xmax+1)*(ymax+1) - 1;
    for (j = 0; j <= ymax; j++){
	for (i = xmax; i >= 0; i--){
	    x = xsize*i;
	    y = ysize*j + yb;
	    /*
	    ** hack to skip background boxes
	    */
	    if((imageptr->value == 0) && (PSStatus() == 1)){
		imageptr--;
		continue;
	    }
	    if(
	    (int32_t) (color_scale * (imageptr->value) + color_min)
	    < color_min){
		fprintf(stderr,"error in color %d\n",
		(int32_t) (color_scale * (imageptr->value) + color_min));
	    }
	    SetColor ((int32_t) (color_scale * (imageptr->value) + color_min));
#ifdef GAP
	    FilledBox((int32_t)x, (int32_t)y, 
	    (int32_t)(x + xsize - 2 + 0.5),
	    (int32_t)(y + ysize - 2 + 0.5));
#endif
	    FilledBox((int32_t)x, (int32_t)y,
	    (int32_t)(x + xsize + 1),
	    (int32_t)(y + ysize + 1));
	    imageptr--;
	}
    }
}

DrawFilledBox(image,xsize,ysize,xmax,ymax)
Image	*image;
float	xsize;
float	ysize;
int32_t	xmax;
int32_t	ymax;
{
Image	*imageptr;
int32_t	i,j;
int32_t	x,y;

    imageptr = image + (xmax+1)*(ymax +1) -1;
    SetColor(G->foreground);
    switch(representation){
    case AREA:
	for (j = 0; j <= ymax; j++){
	    for (i = xmax; i >=0; i--){
		val = sqrt(imageptr->value);
		x = xsize*i+(xsize*(1-val))/2;
		y = yb + ysize*j+(ysize*(1-val))/2;
		FilledBox (
		x, 
		y, 
#ifdef OLD
		x+(int32_t)(val*xsize)+1,
		y+(int32_t)(val*ysize)+1
#else
		x+(int32_t)(val*xsize),
		y+(int32_t)(val*ysize)
#endif
		);
		imageptr--;
	    }
	}
	break;
    case LENGTH:
	for (j = 0; j <= ymax; j++){
	    for (i = xmax; i >=0; i--){
		x = xsize*i+(xsize*(1-imageptr->value))/2;
		y = yb + ysize*j+(ysize*(1-imageptr->value))/2;
#ifdef ORIENT
		if(orient == 0){
		    FilledBox (
		    (int32_t)(xsize*(i+0.5)), 
		    y, 
		    1+(int32_t)(xsize*(i+0.5)), 
		    y+(int32_t)((imageptr->value)*ysize)
		    );
		} else{
		    FilledBox (
		    x, 
		    (int32_t)(ysize*(j+0.5))+yb, 
		    x+(int32_t)((imageptr->value)*xsize),
		    1+(int32_t)(ysize*(j+0.5))+yb
		    );
		}
#else
		FilledBox (
		x, 
		y, 
		x+(int32_t)((imageptr->value)*xsize),
		y+(int32_t)((imageptr->value)*ysize)
		);
#endif
		imageptr--;
	    }
	}
	break;
    }
}

DrawBox(image,xsize,ysize,xmax,ymax)
Image	*image;
float	xsize;
float	ysize;
int32_t	xmax;
int32_t	ymax;
{
Image	*imageptr;
int32_t	i,j;
int32_t	x,y;

    imageptr = image + (xmax+1)*(ymax +1) -1;
    SetColor(G->foreground);
    switch(representation){
    case AREA:
	for (j = 0; j <= ymax; j++){
	    for (i = xmax; i >=0; i--){
		val = sqrt(imageptr->value);
		if(val > 0){
		    x= xsize*i+(xsize*(1-val))/2;
		    y= yb + ysize*j+(ysize*(1-val))/2;
		    if(posneg && (imageptr->sign < 0)){
			FilledBox (
			x, 
			y, 
			x+(int32_t)(val*xsize),
			y+(int32_t)(val*ysize)
			);
		    } else
		    Box (
		    x, 
		    y, 
		    x+(int32_t)(val*xsize),
		    y+(int32_t)(val*ysize)
		    );
		}
		imageptr--;
	    }
	}
	break;
    case LENGTH:
	for (j = 0; j <= ymax; j++){
	    for (i = xmax; i >=0; i--){
		val = imageptr->value;
		if(val > 0){
		    x= xsize*i+(xsize*(1-val))/2;
		    y= yb + ysize*j+(ysize*(1-val))/2;
		    if(posneg && (imageptr->sign < 0)){
			FilledBox (
			x, 
			y, 
			x+(int32_t)(val*xsize),
			y+(int32_t)(val*ysize)
			    );
		    } else {
			Box (
			x, 
			y, 
			x+(int32_t)(val*xsize),
			y+(int32_t)(val*ysize)
			);
		    }
		}
		imageptr--;
	    }
	}
	break;
    }
}

DrawNumberBox(image,xsize,ysize,xmax,ymax)
Image	*image;
float	xsize;
float	ysize;
int32_t	xmax;
int32_t	ymax;
{
Image	*imageptr;
int32_t	i,j;
int32_t	x,y;
char	str[100];
char	comstr[10];
int32_t	n;
int32_t	w;

    SetColor(G->foreground);
    if((w = (xsize-4)/G->fontwidth) < 4) w = 4;
    sprintf(comstr,"%%%d.%dg",w,(w-2)/2);
    for (j = 0; j <= ymax; j++){
	for (i = xmax; i >=0; i--){
	    x= xsize*i;
	    y= yb + ysize*j;
	    n = (ymax - j)*(xmax+1)+i;
	    Box ( x, y, x+(int32_t)xsize, y+(int32_t)ysize);
	    sprintf(str,comstr,data[n]);
	    Text(x+4,y+(int32_t)(ysize/2),str);
	}
    }
}

DrawPoint(image,xsize,ysize,xmax,ymax)
Image	*image;
float	xsize;
float	ysize;
int32_t	xmax;
int32_t	ymax;
{
Image	*imageptr;
int32_t	i,j;
int32_t	x,y;

    imageptr = image + (xmax+1)*(ymax +1) -1;
    SetColor(G->foreground);
    for (j = 0; j <= ymax; j++){
	for (i = xmax; i >=0; i--){
	    val = imageptr->value;
	    if(val > 0){
		x= xsize*i+(xsize*(1-val))/2;
		y= yb + ysize*j+(ysize*(1-val))/2;
		DrawLine (
		x, 
		y, 
		x,
		y
		);
	    }
	    imageptr--;
	}
    }
}
