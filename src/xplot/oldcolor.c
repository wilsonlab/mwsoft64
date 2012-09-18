
/*
** carc color algorithm swiped from Walter Yamada 3/1/88
** (Thanks Walter!)
*/

#include "xplot_ext.h"

#define WI 63.0

#define	arc(x)	(255.0*sqrt(1.0-(((x)-WI)/WI)*(((x)-WI)/WI)))
#define	carc(x)	(((0<=(x))&&((x)<= 2*WI ))?arc(x):0)
#define MAX_COLORS 255
#define MIN_COLOR 20

XColor	color[MAX_COLORS];
unsigned long	cells[MAX_COLORS];
static int	color_scale;
static int	color_max;
static int	color_min;

#define MINDIST 10
#define MINAMP 40

ReadColorMap(file)
char	*file;
{
FILE	*fp;
int	i;
int	r,g,b;
char	line[1000];

    color_min = MIN_COLOR;
    color_max = MAXCOLORS -1;
    if((fp = fopen(file,"r")) == NULL){
	fprintf(stderr,"unable to read cmap file '%s'\n",file);
	return(0);
    }
    i = color_min;
    while(!feof(fp)){
	if(fgets(line,1000,fp) == NULL){
	    break;
	}
	if(line[0] == '%') continue;
	if(line[0] == '#') continue;
	if(line[0] == '\n') continue;
	sscanf(line,"%d%d%d",&r,&g,&b);
	color[i].red = r;
	color[i].green = g;
	color[i].blue = b;
	if(++i > color_max) break;
    }
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

    color_min = MIN_COLOR;
    color_max = MAXCOLORS -1;
    if((fp = fopen(file,"w")) == NULL){
	fprintf(stderr,"unable to write cmap file '%s'\n",file);
	return(0);
    }
    time(&seed);
    srand(seed);
    for (i = color_min; i <= color_max; i++) {
	done = 0;
	while(!done){
	    color[i].red = (int)(frandom(0,254));
	    color[i].green = (int)(frandom(0,254));
	    color[i].blue = (int)(frandom(0,254));
	    if((color[i].red < MINAMP) && 
	    (color[i].green < MINAMP) &&
	    (color[i].blue < MINAMP))
		continue;
	    done = 1;
	    for (j = color_min; j < i; j++) {
		if((abs(color[i].red - color[j].red)
		+ abs(color[i].blue - color[j].blue)
		+ abs(color[i].green - color[j].green)) < 55){
		    done = 0;
		    break;
		}
	    }
	}
	fprintf(fp,"%d %d %d\n",
	color[i].red,
	color[i].green,
	color[i].blue);
	fflush(fp);
    }
    fclose(fp);
}


int MakeColormap(G)
Graph	*G;
{
int	planes[30];			/* planes altered by XGCC call */
Colormap	cmap;
int	i;
float	scale;
int	j;
int	done;

    color_min = MIN_COLOR;
    color_max = MAXCOLORS -1;
    color_scale = color_max - color_min;
    scale = color_scale +1;

    cmap = DefaultColormap(G->display,G->screen_number);
    /*
    if((
    cmap = XCreateColormap(G->display,G->window,G->visual,AllocAll)
    ) == NULL){
	fprintf(stderr,"unable to create colormap\n");
	return;
    }
    */
    /*
    **allocate colors:
    if (XAllocColorCells(G->display, cmap, (Bool)1,planes, 
    8,pixel,MAX_COLORS) == 0 ) {
	fprintf(stderr,"could not allocate color cells\n");
	return(0);
    }
    */
    for (i = 0; i < color_min; i++) {
	color[i].pixel = i;
    }
    srand(1234567);
    for (i = color_min; i <= color_max; i++) {
	/*
	**define rgb values:
	*/
	switch(G->scale_type){
	case RANDOMSCALE:
	    /*
	    ** make sure it is a  minimum distance from existing colors
	    */
	    done = 0;
	    while(!done){
		color[i].red = (int)(frandom(0,254)) ;
		color[i].green = (int)(frandom(0,254)) ;
		color[i].blue = (int)(frandom(0,254)) ;
		if((color[i].red < MINAMP) && 
		(color[i].green < MINAMP) &&
		(color[i].blue < MINAMP))
		    continue;
		done = 1;
		for (j = color_min; j < i; j++) {
		    if((abs(color[i].red - color[j].red)
		    + abs(color[i].blue - color[j].blue)
		    + abs(color[i].green - color[j].green)) < MINDIST){
			done = 0;
			break;
		    }
		}
	    }
	    break;
	case SPECTRALSCALE:
	    color[i].red = (int)(carc(i  -color_min- 2*scale/3.5)) ;
	    color[i].green = (int)(carc(i  -color_min- scale/3.5)) ;
	    color[i].blue = (int)(carc((float)i)) ;
	    break;
	case GRAYSCALE:
	    color[i].red = (int)((i-color_min)*(255-50)/scale +50);
	    color[i].green = (int)((i-color_min)*(255-50)/scale +50);
	    color[i].blue = (int)((i-color_min)*(255-50)/scale +50);
	    break;
	case RGRAYSCALE:
	    color[i].red = (int)((color_max-i)*(255-50)/scale +50);
	    color[i].green = (int)((color_max-i)*(255-50)/scale +50);
	    color[i].blue = (int)((color_max-i)*(255-50)/scale +50);
	    break;
	case FILESCALE:
	    /*
	    ** already read in
	    */
	    break;
	}
    }
    for (i = color_min; i <= color_max; i++) {
	color[i].red = color[i].red << 8;
	color[i].green = color[i].green << 8;
	color[i].blue = color[i].blue << 8;
	color[i].flags = DoRed | DoGreen | DoBlue;
	XAllocColor(G->display,cmap,&color[i]);
    }
    /* XSetWindowColormap(G->display,
	XRootWindow(G->display,G->screen_number),cmap); */
    return(color_max);
}

int ColorMap(i)
int i;
{
extern int PSStatus();
int val;

    if(PSStatus() == 1) {
	val = (225*(i-color_min+2))/(color_scale);
	if(val < 0) val = 0;
	if(val > 255) val = 255;
	return(val);
    } else
    if(G->color_mode){
	return(color[i].pixel); 
    } else {
	return(i);
    }
}

