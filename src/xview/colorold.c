
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
Colormap    cmap, orig_cmap;
XVisualInfo   vTemplate, *visualList;
int          visualsMatched;
Visual       *visual;
XSetWindowAttributes atts;

    color_min = MIN_COLOR;
    ncolors = 0;

    /*cmap = DefaultColormap(G->display,G->screen_number);*/
    for (i = 0; i < MAX_COLORS; i++) {
	pixel[i] = i;
    }
    /*
    ** try to allocate as many colors as the depth of the screen
    ** will allow
    */
    depth = 8;
   vTemplate.screen = G->screen_number;
   vTemplate.depth = 8;
   vTemplate.class = PseudoColor;
   visualList = XGetVisualInfo(G->display, 
      VisualScreenMask|VisualClassMask|VisualDepthMask,
      &vTemplate, &visualsMatched);
   if ( visualsMatched == 0 ) {
      puts("visual not found, using default");
      visual = DefaultVisual( G->display, DefaultScreen(G->display));
   } else {
      printf("found %d visuals\n", visualsMatched);
      visual = visualList[0].visual;
   }

   cmap = XCreateColormap(G->display,XDefaultRootWindow(G->display), visual, AllocAll);
   ncolors = 1<<depth;


#ifdef OLD
    if(colormaptype == PRIVATE){
	cmap = XCopyColormapAndFree(G->display,
	    DefaultColormap(G->display,G->screen_number));
	/* XSetWindowColormap(G->display,G->drawable,cmap); */
        if((depth = DefaultDepth(G->display,G->screen_number)) > 8){
            depth = 8;
        }
	/*
        fprintf(stderr,"using screen depth %d (max=%d)\n",depth,
            DefaultDepth(G->display,G->screen_number));
	    */
        for (powr=depth;powr>=1;powr--) {
	    ncells = 1<<powr;
	    if (XAllocColorCells(G->display,cmap,False,NULL,0,pixel,
		    (unsigned int)ncells)) {
		ncolors = ncells;
		break;
	    }
	    fprintf(stderr,"tried %d cells\n",ncells);
	}
        /* 
	** Get colors from bottom of default cmap and store in 
	** bottom of new cmap 
	*/
	for(i=0; i<ncolors; i++){
	    color[i].pixel = i;
	    pixel[i] = i;
	}
	XQueryColors(G->display,
	    DefaultColormap(G->display,G->screen_number),
	    color,ncolors);
    } else 
    if(colormaptype == SHARED){
	cmap = XCopyColormapAndFree(G->display,
	    DefaultColormap(G->display,G->screen_number));
	ncolors = MAX_COLORS;
    } else 
    if(colormaptype == DEFAULT){
	cmap = XCopyColormapAndFree(G->display,
	    DefaultColormap(G->display,G->screen_number));
	for (powr=DefaultDepth(G->display,G->screen_number);powr>=1;powr--) {
	    ncells = 1<<powr;
	    if (XAllocColorCells(G->display,cmap,False,NULL,0,pixel,
		    (unsigned int)ncells)) {
		ncolors = ncells;
		break;
	    }
	    fprintf(stderr,"tried %d cells\n",ncells);
	}
    }
#endif
    color_scale = ncolors - color_min + 1;
    scale = color_scale;

    for (i = 0; i < ncolors; i++) {
	color[i].pixel = pixel[i];
	color[i].flags = DoRed | DoGreen | DoBlue;
	/*
	** skip the first section of the color map
	*/
	if(i < color_min) continue;
	/*
	**define rgb values:
	*/
	switch(G->scale_type){
	case SQUEEZESCALE:
#ifdef OLD
	    color[i].red = (int)(0.5*(int)(carc(i  -color_min- 2*scale/3.5))
		+ 0.5*(int)(carc(i  -color_min- 1.25*scale/3.5))) << 8;
	    color[i].green = (int)(0.5*(int)(carc(i  -color_min- scale/3.5))
		+ 0.5*(int)(carc(i  -color_min- 0.25*scale/3.5))) << 8;
	    color[i].blue = (int)(0.5*(int)(carc((float)i -color_min))
		+ 0.5*(int)(carc((float)i -color_min - 1.75*scale/3.5))) << 8;
#endif
#ifdef OLD
	if(i == color_min){
#else
	if(i == -1){
#endif
		color[i].red = 0;
		color[i].green = 0;
		color[i].blue = 0;
	} else {
		color[i].red = (int)((255 - color_min)*((float)i/ncolors) + 
		    color_min) << 8;
		color[i].green = (int)(0.3*((255 - color_min)*(1 -(float)i/ncolors) + 
		    color_min)) << 8;
		color[i].blue = (int)((255 - color_min)*(1 -(float)i/ncolors) + 
		    color_min) << 8;
	    }
	    break;
	case SPECTRALSCALE:
	    if((val = (int)(carc(i  -color_min- 2*scale/3.5)) << 8) >=
	    1<<16){
		val = 1<<16 -1;
	    }
	    color[i].red = val;
	    if((val = (int)(carc(i  -color_min- scale/3.5)) << 8) >=
	    1<<16){
		val = 1<<16 -1;
	    }
	    color[i].green =  val;
	    if((val = (int)(carc((float)i -color_min)) << 8) >= 
	    1<<16){
		val = 1<<16 -1;
	    }
	    color[i].blue =  val;
#ifdef OLD
	    if(i == color_min){
		    color[i].red = 255 << 8;
		    color[i].green = 255 << 8;
		    color[i].blue = 255 << 8;
	    }
#endif
	    break;
	case GRAYSCALE:
	    color[i].red = (int)((i-color_min)*(255-50)/scale +50) << 8;
	    color[i].green = (int)((i-color_min)*(255-50)/scale +50) << 8;
	    color[i].blue = (int)((i-color_min)*(255-50)/scale +50) << 8;
	    break;
	case RGRAYSCALE:
	    color[i].red = (int)((ncolors-i)*(255-50)/scale +50) << 8;
	    color[i].green = (int)((ncolors-i)*(255-50)/scale +50) << 8;
	    color[i].blue = (int)((ncolors-i)*(255-50)/scale +50) << 8;
	    break;
	case FILESCALE:
	    /*
	    ** already loaded
	    */
	    break;
	}
	if(colormaptype == SHARED){
	    status = XAllocColor(G->display,cmap,&color[i]);
	    /*
	    fprintf(stderr,"pix %i : stat %d\n",i,status);
	    */
	}
    }
    if(colormaptype == DEFAULT || colormaptype == PRIVATE){
	XStoreColors(G->display,cmap,color,ncolors);
	XFlush(G->display);
    }
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
	    val = color[i].pixel; 
	    if(!G->mapped){
		val = i;
	    }
	    if(val < 0) val = 0;
	    if(val > 255) val = 255;
	} else {
	    val = (225*(i-color_min+2))/(color_scale);
	    if(val < 0) val = 0;
	    if(val > 255) val = 255;
	}
	return(val);
    } else
    if(color_mode){
	return(color[i].pixel); 
    } else {
	return(i);
    }
}

