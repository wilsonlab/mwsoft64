
/*
** carc color algorithm swiped from Walter Yamada 3/1/88
** (Thanks Walter!)
*/

#include "xplot_ext.h"

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
int32_t	pixel[MAX_COLORS];
Colormap	cmap;
ColorTable	idx[MAX_COLORS];
int32_t	color_min;
float	color_scale;

void alloc_color(int32_t i, int32_t r, int32_t g, int32_t b)
{
    XColor col;

    col.red =	r; col.green = g; col.blue = b;
    col.flags = DoRed | DoGreen | DoBlue;
    if (XAllocColor(F->display, cmap, &col))
    {
	idx[i].pixel = col.pixel;
    }else
    {
	if (cmap == DefaultColormap(F->display, F->screen_number))
	{
	    cmap = XCopyColormapAndFree(F->display, cmap);
	    XSetWindowColormap(F->display, F->window, cmap);
	    col.red =	r; col.green = g; col.blue = b;
	    col.flags = DoRed | DoGreen | DoBlue;
	    if (XAllocColor(F->display, cmap, &col))
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
int32_t	i;
int32_t	r,g,b;
char	line[1000];
int32_t	color_max;

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
int32_t	i,j;
int32_t	done;
int32_t	seed;

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


int32_t MakeColormap()
{
int32_t	i;
float	scale;
int32_t	powr;
int32_t	ncells;
int32_t	ncolors;
int32_t	colormaptype = PRIVATE;
/*
int32_t	colormaptype = SHARED;
*/
int32_t	status;
int32_t	val;
int32_t	depth;

    color_min = MIN_COLOR;
    ncolors = 0;

    cmap = DefaultColormap(F->display,F->screen_number);
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
	switch(F->scale_type){
	case SPECTRALSCALE:
	    if((val = (int32_t)(carc(i  -color_min- 2*scale/3.5)) << 8) >=
	    1<<16){
		val = 1<<16 -1;
	    }
	    idx[i].r = val;
	    if((val = (int32_t)(carc(i  -color_min- scale/3.5)) << 8) >=
	    1<<16){
		val = 1<<16 -1;
	    }
	    idx[i].g =  val;
	    if((val = (int32_t)(carc((float)i -color_min)) << 8) >= 
	    1<<16){
		val = 1<<16 -1;
	    }
	    idx[i].b =  val;
	    break;
	case GRAYSCALE:
	    idx[i].r = (int32_t)((i-color_min)*(255-50)/scale +50) << 8;
	    idx[i].g = (int32_t)((i-color_min)*(255-50)/scale +50) << 8;
	    idx[i].b = (int32_t)((i-color_min)*(255-50)/scale +50) << 8;
	    break;
	case RGRAYSCALE:
	    idx[i].r = (int32_t)((ncolors-i)*(255-50)/scale +50) << 8;
	    idx[i].g = (int32_t)((ncolors-i)*(255-50)/scale +50) << 8;
	    idx[i].b = (int32_t)((ncolors-i)*(255-50)/scale +50) << 8;
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

int32_t ColorMap(i)
int32_t i;
{
extern int32_t PSStatus();
int32_t val;

    if(i >= MAX_COLORS){
	i = MAX_COLORS -1;
    }
    if(PSStatus() == 1) {
	if(F->color_mode){
	    val = idx[i].pixel; 
	    if(!F->mapped){
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
    if(F->color_mode){
	return(idx[i].pixel); 
    } else {
	return(i);
    }
}

