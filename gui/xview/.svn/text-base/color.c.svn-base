
/*
** carc color algorithm swiped from Walter Yamada 3/1/88
** (Thanks Walter!)
*/

#include "xview_ext.h"

#define WI 63.0
#define SHARED	0
#define DEFAULT	1
#define PRIVATE	2

#define	arc(x)	(255.0*sqrt(1.0-(((x)-WI)/WI)*(((x)-WI)/WI)))
#define	arc2(W,x)	(sqrt(1.0-(((x)-W)/W)*(((x)-W)/W)))
#define	carc(x)	(((0<=(x))&&((x)<= 2*WI ))?arc(x):0)
#define	carc2(W,x)	(((0<=(x))&&((x)<= 2*W ))?arc2(W,x):0)
#define MAX_COLORS 256
#define MIN_COLOR 25

XColor	color[MAX_COLORS];
int	pixel[MAX_COLORS];
Colormap	cmap;
ColorTable	idx[MAX_COLORS];

void alloc_color(int i, int r, int g, int b)
{
    XColor col;

    col.red =	r; col.green = g; col.blue = b;
    col.flags = DoRed | DoGreen | DoBlue;
    if (XAllocColor(G->display, cmap, &col))
    {
	idx[i].pixel = col.pixel;
    }else
    {
	if (cmap == DefaultColormap(G->display, G->screen_number))
	{
	    cmap = XCopyColormapAndFree(G->display, cmap);
	    XSetWindowColormap(G->display, G->imagewindow, cmap);
	    col.red =	r; col.green = g; col.blue = b;
	    col.flags = DoRed | DoGreen | DoBlue;
	    if (XAllocColor(G->display, cmap, &col))
	    {
		idx[i].pixel = col.pixel;
		fprintf(stderr,"allocating pixel %d: %d %d %d\n",i,r,g,b);
	    } else {
		fprintf(stderr,"unable to allocate color %d: %d %d %d\n",i,r,g,b);
	    }
	}
    }
}

Colormap GetColormap()
{
    return(cmap);
}


ReadColorMap(file)
char	*file;
{
FILE	*fp;
int	i;
int	r,g,b;
char	line[1000];
int	color_max;

    color_max = MAX_COLORS -1;
    if((fp = fopen(file,"r")) == NULL){
	fprintf(stderr,"unable to read cmap file '%s'\n",file);
	return(0);
    }
    i = 0;
    fprintf(stderr,"reading colormap\n");
    while(!feof(fp)){
	if(fgets(line,1000,fp) == NULL){
	    break;
	}
	if(line[0] == '%') continue;
	if(line[0] == '#') continue;
	if(line[0] == '\n') continue;
	sscanf(line,"%d%d%d",&r,&g,&b);
	color[i].red = r << 8;
	color[i].green = g << 8;
	color[i].blue = b << 8;
	if(++i > color_max) break;
	fprintf(stderr,"%d : %d %d %d\n",i,
	r,g,b);
    }
    fprintf(stderr,"finished reading colormap\n");
    fclose(fp);
    if(i<= color_max){
	fprintf(stderr,"incomplete color map specification\n");
	return(0);
    }
    return(1);
}

WriteColorMap(file)
char	*file;
{
FILE	*fp;
int	i,j;
int	done;
long	seed;

    if((fp = fopen(file,"w")) == NULL){
	fprintf(stderr,"unable to write cmap file '%s'\n",file);
	return(0);
    }
    for (i = 0; i < MAX_COLORS; i++) {
	fprintf(fp,"%d %d %d\n",
	color[i].red >> 8,
	color[i].green >> 8,
	color[i].blue >> 8);;
	fflush(fp);
    }
    fclose(fp);
}


int MakeColormap()
{
int	i;
float	scale;
int	powr;
int	ncells;
int	ncolors;
int	colormaptype = PRIVATE;
/*
int	colormaptype = SHARED;
*/
int	status;
int	val;
int	depth;

    color_min = MIN_COLOR;
    ncolors = 0;

    cmap = DefaultColormap(G->display,G->screen_number);
    for (i = 0; i < MAX_COLORS; i++) {
	idx[i].pixel = i;
    }
    ncolors = MAX_COLORS;
    /*
    ** try to allocate as many colors as the depth of the screen
    ** will allow
    */
    color_scale = ncolors - color_min + 1;
    scale = color_scale;

    for (i = 0; i < ncolors; i++) {
	/*
	** skip the first section of the color map
	*/
	if(i < color_min) continue;
	/*
	**define rgb values:
	*/
	switch(G->scale_type){
	case SQUEEZESCALE:
	if(i == -1){
		idx[i].r = 0;
		idx[i].g = 0;
		idx[i].b = 0;
	} else {
		idx[i].r = (int)((255 - color_min)*((float)i/ncolors) + 
		    color_min) << 8;
		idx[i].g = (int)(0.3*((255 - color_min)*(1 -(float)i/ncolors) + 
		    color_min)) << 8;
		idx[i].b = (int)((255 - color_min)*(1 -(float)i/ncolors) + 
		    color_min) << 8;
	    }
	    break;
	case SPECTRALSCALE:
	    if((val = (int)(carc(i  -color_min- 2*scale/3.5)) << 8) >=
	    1<<16){
		val = 1<<16 -1;
	    }
	    idx[i].r = val;
	    if((val = (int)(carc(i  -color_min- scale/3.5)) << 8) >=
	    1<<16){
		val = 1<<16 -1;
	    }
	    idx[i].g =  val;
	    if((val = (int)(carc((float)i -color_min)) << 8) >= 
	    1<<16){
		val = 1<<16 -1;
	    }
	    idx[i].b =  val;
	    break;
	case GRAYSCALE:
	    idx[i].r = (int)((i-color_min)*(255-50)/scale +50) << 8;
	    idx[i].g = (int)((i-color_min)*(255-50)/scale +50) << 8;
	    idx[i].b = (int)((i-color_min)*(255-50)/scale +50) << 8;
	    break;
	case RGRAYSCALE:
	    idx[i].r = (int)((ncolors-i)*(255-50)/scale +50) << 8;
	    idx[i].g = (int)((ncolors-i)*(255-50)/scale +50) << 8;
	    idx[i].b = (int)((ncolors-i)*(255-50)/scale +50) << 8;
	    break;
	case FILESCALE:
	    /*
	    ** already loaded
	    */
	    break;
	}
	alloc_color(i,idx[i].r,idx[i].g,idx[i].b);
    }
    alloc_color(WHITE,65535,65535,65535);
    alloc_color(BLACK,0,0,0);
    return(ncolors);
}

int ColorMap(i)
int i;
{
extern int PSStatus();
int val;

    if(i >= MAX_COLORS){
	i = MAX_COLORS -1;
    }
    if(PSStatus() == 1) {
	if(color_mode){
	    val = idx[i].pixel; 
	    if(!G->mapped){
		val = i;
	    }
	    /*
	    if(val < 0) val = 0;
	    if(val > 255) val = 255;
	    */
	} else {
	    val = (225*(i-color_min+2))/(color_scale);
	    if(val < 0) val = 0;
	    if(val > 255) val = 255;
	}
	return(val);
    } else
    if(color_mode){
	return(idx[i].pixel); 
    } else {
	return(i);
    }
}

