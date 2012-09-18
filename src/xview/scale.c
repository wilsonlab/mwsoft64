#include "xview_ext.h"

#ifdef OLD
Rescale()
{
XWindowAttributes	winfo;
Window			*root;

    if(header){
	yb = G->fontheight*3;
    } else {
	yb = 0;
    }
    if(G->display){
	XGetWindowAttributes(G->display,G->imagewindow,&winfo);

	G->wwidth = width = winfo.width;
	G->wheight = winfo.height;
    } else {
	width = G->wwidth;
    }
    if(header){
	height = G->wheight - (yb + 2*G->fontheight);
    } else {
	height = winfo.height;
    }
    if(manual_xsize)
	xsize = xs;
    else
	xsize = (float)width/(V->xmax +1);
    if(manual_ysize)
	ysize = ys;
    else 
	ysize = (float)(height)/(V->ymax +1);
    if(zsize == 0)
    zsize = ysize*30;

    if(header){
	legend_base = yb + height + G->fontheight;
    } else {
	legend_base = 0;
    }
}
#endif
Rescale()
{
XWindowAttributes	winfo;
Window			*root;

    yb = G->fontheight*3;
    if(G->display){
	XGetWindowAttributes(G->display,G->imagewindow,&winfo);

	G->wwidth = width = winfo.width;
	G->wheight = winfo.height;
    } else {
	width = G->wwidth;
    }
    height = G->wheight - (yb + 2*G->fontheight);
    if(manual_xsize)
	xsize = xs;
    else
	xsize = (float)width/(V->xmax +1);
    if(manual_ysize)
	ysize = ys;
    else 
	ysize = (float)(height)/(V->ymax +1);
    if(zsize == 0)
    zsize = ysize*30;

    legend_base = yb + height + G->fontheight;
}

do_autoscale(fp)
FILE *fp;
{
float *dataptr;
int	i;
int	firsttime;

    firsttime = 1;
    /* 
    * calculate autoscale parameters 
    */
    fseek (fp, (long) (V->headersize), 0);
    while(!feof(fp)){
	GetaRecord();
	dataptr = data;
	for (i = 0; i < V->cellnum; i++) {
	    if(firsttime){
		minval = *dataptr;
		maxval = *dataptr;
		firsttime = 0;
	    }
	    if (*dataptr > maxval)
		maxval = *dataptr;
	    if (*dataptr < minval)
		minval = *dataptr;
	    dataptr++;
	}
    }
    /* 
    * go back to the beginning of the file 
    */
    fseek (fp, (long) (V->headersize), 0);
    /* 
    * clear the data 
    */
    for(i=0;i<V->cellnum;i++)
	    data[i] = 0;
    if((scale = maxval - minval) == 0){
	scale = 1;
    }
}

