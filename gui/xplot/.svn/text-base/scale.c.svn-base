#include "xplot_ext.h"

static calcwxmin = 1;
static calcwymin = 1;
static calcwxmax = 1;
static calcwymax = 1;

SuppressWxmin()
{
    calcwxmin = 0;
}

SuppressWymin()
{
    calcwymin = 0;
}

SuppressWxmax()
{
    calcwxmax = 0;
}

SuppressWymax()
{
    calcwymax = 0;
}

RescaleGraph(graph)
Graph *graph;
{
Plot	*plot;
double val;

    /*
    ** move the current world view to slot one
    ** save the new world view in slot zero
    */
    graph->savedview[1] = graph->savedview[0];
    ProcessSavedView(graph,1,0);
    if((val = graph->wxmax - graph->wxmin) == 0){
	val = 1;
    }
    graph->wxscale = graph->wwidth/val;
    if((val = graph->wymax - graph->wymin) == 0){
	val = 1;
    }
    graph->wyscale = graph->wheight/val;
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->visible){
	    RescalePlot(plot);
	}
    }
}

SetPlotVisibility(plot,val)
Plot	*plot;
int	val;
{
    if(plot == NULL) return;
    plot->visible = val;
    if(plot->visible){
	RescalePlot(plot);
    }
}

RescaleFrame(frame)
Frame	*frame;
{
XWindowAttributes	winfo;
Plot			*plot;
int			w,h;

    if(!frame->mapped){
	return;
    }
    /*
    ** get the size of the frame
    */
    XGetWindowAttributes(frame->display,frame->window,&winfo);
    /*
    ** check to see whether the window size has changed
    */
    w = winfo.width;
    h = winfo.height;
    /*
    ** if it has then update the window parameters
    */
    if(w != frame->wwidth || h != frame->wheight){
	frame->wheight = winfo.height;
	frame->wwidth = winfo.width;

	/*
	** position the subwindows
	*/
	PositionSubwindows(frame);
	RescaleGraph(frame->graph);
	return(1);
    } 
    return(0);
}

PositionSubwindows(frame)
Frame	*frame;
{
TextWindow		*text;
MenuWindow		*menu;
Graph			*graph;
int			text_height;
int			menu_width;

    graph = frame->graph;
    text = frame->text;
    menu = frame->menu;

    if(text->mapped){
	text_height = 2*frame->fontheight;
    } else {
	text_height = 0;
    }
    if(menu->mapped){
	/* menu_width = 15*menu->fontwidth; */
	menu_width = 80;
    } else {
	menu_width = 0;
    }
    if(graph->mapped){
	/*
	** adjust the graph window accordingly
	*/
	graph->wwidth = frame->wwidth - menu_width;
	graph->wheight = frame->wheight - text_height;
	if(graph->display){
	    XMoveResizeWindow(graph->display,graph->window,-1,-1,
	    graph->wwidth,graph->wheight);
	}
    }
    if(menu->mapped){
	/*
	** adjust the menu window 
	*/
	menu->wwidth = menu_width;
	menu->wheight = frame->wheight;
	if(menu->display){
	    XMoveResizeWindow(menu->display,menu->window,graph->wwidth,-1,
	    menu->wwidth,menu->wheight);
	}
    }
    if(text->mapped){
	/*
	** adjust the text window 
	*/
	text->wwidth = frame->wwidth - menu_width;
	text->wheight = text_height;
	if(text->display){
	    XMoveResizeWindow(text->display,text->window,-1,graph->wheight,
	    graph->wwidth,text_height);
	}
    }

}

UpdateRotationMatrix(graph)
Graph	*graph;
{
double	Sx,Sy,Sz;
double	Cx,Cy,Cz;
int	i;

    /*
    ** check for the matrix
    */
    if(!graph->matrix){
	/*
	** allocate the matrix
	*/
	graph->matrix = (double **)malloc(3*sizeof(double *));
	for(i=0;i<3;i++){
	    graph->matrix[i] = (double *)malloc(3*sizeof(double));
	}
    }
    /*
    ** fill the matrix according to the current rotational parameters
    */
    Sx = sin(graph->thetax);
    Sy = sin(graph->thetay);
    Sz = sin(graph->thetaz);
    Cx = cos(graph->thetax);
    Cy = cos(graph->thetay);
    Cz = cos(graph->thetaz);

    graph->matrix[0][0] = Cy*Cz;
    graph->matrix[1][0] = Sx*Sy*Cz - Cx*Sz;
    graph->matrix[2][0] = Sy*Cx*Cz + Sx*Sz;

    graph->matrix[0][1] = Cy*Sz;
    graph->matrix[1][1] = Sx*Sy*Sz + Cx*Cz;
    graph->matrix[2][1] = Sy*Cx*Sz - Sx*Cz;

    graph->matrix[0][2] = -Sy;
    graph->matrix[1][2] = Sx*Cy;
    graph->matrix[2][2] = Cx*Cy;
}

MatrixTransform(basic,x,y,z,nx,ny,nz,disparityscale)
BasicWindow	*basic;
double	x,y,z;
double	*nx,*ny,*nz;
float	disparityscale;
{
double	**m;

    if((basic == NULL) || (m = basic->matrix) == NULL){
	*nx = x;
	*ny = y;
	*nz = z;
    } else {
	*nx = m[0][0]*x + m[1][0]*y + m[2][0]*z;
	*ny = m[0][1]*x + m[1][1]*y + m[2][1]*z;
	*nz = m[0][2]*x + m[1][2]*y + m[2][2]*z;
    }
    /*
    ** apply disparity based on z
    */
    *nx += *nz*disparityscale;
}

GetDataPoint(plot,wx,wy,wz,index)
Plot	*plot;
float	*wx,*wy,*wz;
int	index;
{
FCoord	*data;
double	x,y,z;

    /*
    ** get original data points
    */
    data = plot->data + index;
    x = data->x*plot->xscale + plot->xoffset;
    y = data->y*plot->scale + plot->yoffset;
    if(plot->zmode && plot->edata){
	z = plot->edata[index].value*plot->escale;
    } else {
	z = 0;
    }
    /*
    ** adjust for axis scaling and offsets
    */
    switch(plot->graph->xaxis.type){
    case LINEAR_AXIS:
	*wx = x + plot->xoffset_axis;
	break;
    case LOG10_AXIS:
	*wx = log10(x) + plot->xoffset_axis;
	break;
    }
    switch(plot->graph->yaxis.type){
    case LINEAR_AXIS:
	*wy = y + plot->yoffset_axis;
	break;
    case LOG10_AXIS:
	*wy = log10(y) + plot->yoffset_axis;
	break;
    }
    switch(plot->graph->zaxis.type){
    case LINEAR_AXIS:
	*wz = z + plot->zoffset_axis;
	break;
    case LOG10_AXIS:
	*wz = log10(z) + plot->zoffset_axis;
	break;
    }
}

GetTransformedDataPoint(plot,wx,wy,wz,index)
Plot	*plot;
float	*wx,*wy,*wz;
int	index;
{
double	nx,ny,nz;

    /*
    ** get the original data points
    */
    GetDataPoint(plot,wx,wy,wz,index);
    /*
    ** apply transformations
    */
    MatrixTransform(plot->graph,(double)*wx,(double)*wy,(double)*wz,&nx,&ny,&nz,
    plot->graph->disparityscale);
    /*
    ** and assign the new values
    */
    *wx = nx;
    *wy = ny;
    *wz = nz;
}

RescalePlot(plot)
Plot	*plot;
{
int 	i;
float	sx,sy;
float	wx,wy,wz;
FCoord	*data;
Coord	*coord;
FCoord	*fcoord;
int	randomizex;
int	randomizey;

    /*
    ** transform each data coordinate into a screen coordinate
    */
    data = plot->data;
    coord = plot->coord;
    fcoord = plot->fcoord;
    if(data == NULL || coord == NULL || fcoord == NULL) return;
    data += plot->xlo;
    coord += plot->xlo;
    fcoord += plot->xlo;
    if(plot->graph){
	randomizex = plot->randomizex;
	randomizey = plot->randomizey;
    } else {
	randomizex = 0;
	randomizey = 0;
    }
    for(i=plot->xlo;i<=plot->xhi;i++){
	GetTransformedDataPoint(plot,&wx,&wy,&wz,i);
	/*
	** if randomization is selected then add a value
	*/
	if(randomizex){
	    wx += frandom(-0.5,0.5);
	}
	if(randomizey){
	    wy += frandom(-0.5,0.5);
	}
	if(i==plot->xlo){
	    plot->xmin = plot->xmax = wx;
	    plot->ymin = plot->ymax = wy;
	} else {
	    if(wx < plot->xmin) plot->xmin = wx;
	    if(wx > plot->xmax) plot->xmax = wx;
	    if(wy < plot->ymin) plot->ymin = wy;
	    if(wy > plot->ymax) plot->ymax = wy;
	}
	FloatScreenTransform(plot->graph, wx,wy, &sx,&sy);
	coord->x = sx;
	coord->y = sy;
	fcoord->x = sx;
	fcoord->y = sy;
	data++;
	coord++;
	fcoord++;
    }
}

RandomizeX(plot)
Plot	*plot;
{
int 	i;
float	sx,sy;
float	wx,wy,wz;
Coord	*coord;
FCoord	*fcoord;

    /*
    ** add random noise to each data point
    */
    /*
    ** transform each data coordinate into a screen coordinate
    */
    coord = plot->coord;
    fcoord = plot->fcoord;
    if(coord == NULL || fcoord == NULL) return;
    coord += plot->xlo;
    fcoord += plot->xlo;
    for(i=plot->xlo;i<=plot->xhi;i++){
	GetTransformedDataPoint(plot,&wx,&wy,&wz,i);
	wx += frandom(0,1);
	if(i==plot->xlo){
	    plot->xmin = plot->xmax = wx;
	    plot->ymin = plot->ymax = wy;
	} else {
	    if(wx < plot->xmin) plot->xmin = wx;
	    if(wx > plot->xmax) plot->xmax = wx;
	    if(wy < plot->ymin) plot->ymin = wy;
	    if(wy > plot->ymax) plot->ymax = wy;
	}
	FloatScreenTransform(plot->graph, wx,wy, &sx,&sy);
	coord->x = sx;
	coord->y = sy;
	fcoord->x = sx;
	fcoord->y = sy;
	coord++;
	fcoord++;
    }
}

RandomizeY(plot)
Plot	*plot;
{
int 	i;
float	sx,sy;
float	wx,wy,wz;
Coord	*coord;
FCoord	*fcoord;

    /*
    ** add random noise to each data point
    */
    /*
    ** transform each data coordinate into a screen coordinate
    */
    coord = plot->coord;
    fcoord = plot->fcoord;
    if(coord == NULL || fcoord == NULL) return;
    coord += plot->xlo;
    fcoord += plot->xlo;
    for(i=plot->xlo;i<=plot->xhi;i++){
	GetTransformedDataPoint(plot,&wx,&wy,&wz,i);
	wy += frandom(0,1);
	if(i==plot->xlo){
	    plot->xmin = plot->xmax = wx;
	    plot->ymin = plot->ymax = wy;
	} else {
	    if(wx < plot->xmin) plot->xmin = wx;
	    if(wx > plot->xmax) plot->xmax = wx;
	    if(wy < plot->ymin) plot->ymin = wy;
	    if(wy > plot->ymax) plot->ymax = wy;
	}
	FloatScreenTransform(plot->graph, wx,wy, &sx,&sy);
	coord->x = sx;
	coord->y = sy;
	fcoord->x = sx;
	fcoord->y = sy;
	coord++;
	fcoord++;
    }
}


/*
** transform world coordinates x,y into screen coordinate sx,sy
*/
ScreenTransform(graph,wx,wy,sx,sy)
Graph	*graph;
float	wx,wy;
int	*sx,*sy;
{


    *sx = (wx - graph->wxmin)*graph->wxscale;
    if(*sx > 10000) *sx = 10000;
    if(*sx < -10000) *sx = -10000;
    /*
    ** invert the screen coordinates so that the top of the window is up
    */
    *sy = graph->wheight - (wy - graph->wymin)*graph->wyscale;
    if(*sy > 10000) *sy = 10000;
    if(*sy < -10000) *sy = -10000;
}

/*
** transform world coordinates x,y into screen coordinate sx,sy
*/
FloatScreenTransform(graph,wx,wy,sx,sy)
Graph	*graph;
float	wx,wy;
float	*sx,*sy;
{

    *sx = (wx - graph->wxmin)*graph->wxscale;
#ifdef OLD
    /*
    ** try to keep X from losing its shorts over large integers
    */
    if(*sx > 10000) *sx = 10000;
    if(*sx < -10000) *sx = -10000;
#endif
    /*
    ** invert the screen coordinates so that the top of the window is up
    */
    *sy = graph->wheight - (wy - graph->wymin)*graph->wyscale;
#ifdef OLD
    if(*sy > 10000) *sy = 10000;
    if(*sy < -10000) *sy = -10000;
#endif
}

/*
** transform screen coordinates sx,sy into world coordinate wx,wy
*/
WorldTransform(graph,sx,sy,wx,wy)
Graph *graph;
int sx,sy;
float *wx,*wy;
{
    if((graph->wxscale == 0) || (graph->wyscale == 0)){
	*wx = 0;
	*wy = 0;
    }
    /*
    ** do the inverse transform from screen coords to world coords
    */
    *wx = sx/graph->wxscale +graph->wxmin;
    /*
    ** invert the y screen coord before transforming
    */
    *wy = (graph->wheight - sy)/graph->wyscale +graph->wymin;
}

AutoScale(graph,autoscale_x,autoscale_y)
Graph *graph;
int autoscale_x,autoscale_y;
{
int	cnt;
float	val;
Plot	*plot;

    if(autoscale_x){
	cnt = 0;
	for(plot=graph->plot;plot;plot=plot->next){
	    if(plot->visible){
		if(cnt++ < 1){
		    graph->xmax = plot->xmax;
		    graph->xmin = plot->xmin;
		} else {
		    if(plot->xmin < graph->xmin) graph->xmin = plot->xmin;
		    if(plot->xmax > graph->xmax) graph->xmax = plot->xmax;
		}
	    }
	}
    }
    if(autoscale_y){
	cnt = 0;
	for(plot=graph->plot;plot;plot=plot->next){
	    if(plot->visible){
		if(cnt++ < 1){
		    graph->ymax = plot->ymax;
		    graph->ymin = plot->ymin;
		} else {
		    if(plot->ymin < graph->ymin) graph->ymin = plot->ymin;
		    if(plot->ymax > graph->ymax) graph->ymax = plot->ymax;
		}
	    }
	}
    }
    /*
    ** determine the world coordinate bounds of the window
    ** assume that the data area will take up a fraction of the window
    */
    val = graph->xmax - graph->xmin;
    if(val ==0) val=1;
    if(calcwxmin){
	graph->wxmin = graph->xmin - 0.15*val;
    }
    if(calcwxmax){
	graph->wxmax = graph->xmax + 0.1*val;
    }

    val = graph->ymax - graph->ymin;
    if(val ==0) val=1;
    if(calcwymin){
	graph->wymin = graph->ymin - 0.15*val;
    }
    if(calcwymax){
	graph->wymax = graph->ymax + 0.1*val;
    }

    calcwxmin = 1;
    calcwymin = 1;
    calcwxmax = 1;
    calcwymax = 1;
}

AutoOrigin(graph)
Graph *graph;
{
int sw,sh;
int tw,th;
float ww,wh;
char label[80];
int i;

    /*
    ** autolocate the origin within the current window
    ** make enough room to fit the labels
    */
    label[0] = '\0';
    for(i=0;i<graph->yaxis.rightdp + graph->yaxis.leftdp +2;i++){
	strcat(label,"0");
    }
    TextExtent(label,&tw,&th);
    sh = graph->wheight - th*3.5;
    sw = tw;

    WorldTransform(graph,sw,sh,&ww,&wh);
    graph->yaxis.xintcpt = ww;
    graph->xaxis.yintcpt = wh;
}

AutoXOrigin(graph)
Graph *graph;
{
int sw,sh;
int tw,th;
float ww,wh;
char label[80];
int i;

    /*
    ** autolocate the origin within the current window
    ** make enough room to fit the labels
    */
    label[0] = '\0';
    for(i=0;i<graph->yaxis.rightdp + graph->yaxis.leftdp +2;i++){
	strcat(label,"0");
    }
    TextExtent(label,&tw,&th);
    sh = graph->wheight - th*3.5;
    sw = 2*tw;

    WorldTransform(graph,sw,sh,&ww,&wh);
    graph->yaxis.xintcpt = ww;
}

AutoYOrigin(graph)
Graph *graph;
{
int sw,sh;
int tw,th;
float ww,wh;
char label[80];
int i;

    /*
    ** autolocate the origin within the current window
    ** make enough room to fit the labels
    */
    label[0] = '\0';
    for(i=0;i<graph->yaxis.rightdp + graph->yaxis.leftdp +2;i++){
	strcat(label,"0");
    }
    TextExtent(label,&tw,&th);
    sh = graph->wheight - th*3.5;
    sw = 2*tw;

    WorldTransform(graph,sw,sh,&ww,&wh);
    graph->yaxis.xintcpt = ww;
}

