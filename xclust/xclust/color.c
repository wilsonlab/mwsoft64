
/*
 ** carc color algorithm swiped from Walter Yamada 3/1/88
 ** (Thanks Walter!)
 */

#include "xclust_ext.h"

#define WI 15.0
#define WI2 63.0
#define CMAPDEPTH	8


#define	arc(x)	(255.0*sqrt(1.0-(((x)-WI)/WI)*(((x)-WI)/WI)))
#define	carc(x)	(((0<=(x))&&((x)<= 2*WI ))?arc(x):0)

#define	arc2(x)	(255.0*sqrt(1.0-(((x)-WI2)/WI2)*(((x)-WI2)/WI2)))
#define	carc2(x)	(((0<=(x))&&((x)<= 2*WI2 ))?arc2(x):0)

ColorTable idx[MAXCOLORS];

/* XColor	color[MAXCOLORS]; */
/* unsigned long	pixel[MAXCOLORS]; */
/* unsigned long	planemask[MAXCOLORS]; */
/* static int	color_scale; */
/* static int	color_max; */

static Display	*display;	
static Colormap	cmap;
static float	Scale;
float	scale;

#define MINDIST 10
#define MINAMP 40

ReadColorMap(file)
     char	*file;
{
/*   FILE	*fp; */
/*   int	i; */
/*   int	r,g,b; */
/*   char	line[1000]; */
  
/*   color_max = MAXCOLORS - MINCOLOR; */
/*   if((fp = fopen(file,"r")) == NULL){ */
/*     fprintf(stderr,"unable to read cmap file '%s'\n",file); */
/*     return(0); */
/*   } */
/*   i = 0; */
/*   while(!feof(fp)){ */
/*     if(fgets(line,1000,fp) == NULL){ */
/*       break; */
/*     } */
/*     if(line[0] == '%') continue; */
/*     if(line[0] == '#') continue; */
/*     if(line[0] == '\n') continue; */
/*     sscanf(line,"%d%d%d",&r,&g,&b); */
/*     color[i].red = r; */
/*     color[i].green = g; */
/*     color[i].blue = b; */
/*     if(++i > color_max) break; */
/*   } */
/*   fclose(fp); */
/*   if(i< color_max){ */
/*     fprintf(stderr,"incomplete color map specification\n"); */
/*     return(0); */
/*   } */
  return(1);
}

WriteColorMap(file)
     char	*file;
{
/*   FILE	*fp; */
/*   int	i,j; */
/*   int	done; */
/*   long	seed; */
  
/*   color_max = MAXCOLORS - MINCOLOR; */
/*   if((fp = fopen(file,"w")) == NULL){ */
/*     fprintf(stderr,"unable to write cmap file '%s'\n",file); */
/*     return(0); */
/*   } */
/*   time(&seed); */
/*   srand(seed); */
/*   for (i = 0; i <= color_max; i++) { */
/*     done = 0; */
/*     while(!done){ */
/*       color[i].red = (int)(frandom(0,65535)); */
/*       color[i].green = (int)(frandom(0,65535)); */
/*       color[i].blue = (int)(frandom(0,65535)); */
/*       if((color[i].red < MINAMP) &&  */
/* 	 (color[i].green < MINAMP) && */
/* 	 (color[i].blue < MINAMP)) */
/* 	continue; */
/*       done = 1; */
/*       for (j = 0; j < i; j++) { */
/* 	if((abs(color[i].red - color[j].red) */
/* 	    + abs(color[i].blue - color[j].blue) */
/* 	    + abs(color[i].green - color[j].green)) < 55){ */
/* 	  done = 0; */
/* 	  break; */
/* 	} */
/*       } */
/*     } */
/*     fprintf(fp,"%d %d %d\n", */
/* 	    color[i].red, */
/* 	    color[i].green, */
/* 	    color[i].blue); */
/*     fflush(fp); */
/*   } */
/*   fclose(fp); */
}
void SetFrameColormap()
{
	XSetWindowColormap(F->display, F->window, cmap);
}

void alloc_color(int i, int r, int g, int b)
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
	    /*
	    XSetWindowColormap(F->display, F->window, cmap);
	    */
	    col.red =	r; col.green = g; col.blue = b;
	    col.flags = DoRed | DoGreen | DoBlue;
	    if (XAllocColor(F->display, cmap, &col))
	    {
		idx[i].pixel = col.pixel;
	    } else {
		fprintf(stderr,"unable to allocate color %d: %d %d %d\n",i,r,g,b);
	    }
	}
    }
}

void alloc_named_color(int i, char *s)
{
    XColor screencol;
    XColor exactcol;

    if (XAllocNamedColor(F->display, cmap, s, &screencol, &exactcol))
    {
	idx[i].pixel = screencol.pixel;
    }else
    {
	if (cmap == DefaultColormap(F->display, F->screen_number))
	{
	    cmap = XCopyColormapAndFree(F->display, cmap);
	    /*
	    XSetWindowColormap(F->display, F->window, cmap);
	    */
	    if (XAllocNamedColor(F->display, cmap, s, &screencol, &exactcol))
	    {
		idx[i].pixel = screencol.pixel;
	    } else {
		fprintf(stderr,"unable to allocate named color %d: %s\n",i,s);
	    }
	}
    }
}

void private_colormap()
{
int h, i, j, k;
/*int r, g, b;*/
int count = 0;
char	*str;
char	optstr[30];
 MenuWindow* menu;
float	spectral_scale;
unsigned short r,g,b;
 int maxint, nr, ng, nb;
 float intensity;

 /* reset map */
 for (i=0; i<256; i++) idx[i].pixel = 0;

 /* define white and black pixels */
 alloc_color(WHITE, 65535,65535,65535);
 alloc_color(BLACK, 0,0,0);

 /* define interface colors */
 if((str = XGetDefault(F->display,"xclust","background")) == NULL){
   str = "gray70";
 }
 AssignColor(str, MININTERFACECOLOR + FRAMEBACKGROUND);
 
 if((str = XGetDefault(F->display,"xclust","foreground")) == NULL){
   str = "gray30";
 }
 AssignColor(str, MININTERFACECOLOR + FRAMEFOREGROUND);
 
/*  if((str = XGetDefault(F->display,"xclust","cluststatbackground")) == NULL){ */
/*    str = "gray70"; */
/*  } */
/*  AssignColor(str, MININTERFACECOLOR + CLUSTSTATBACKGROUND); */

/*  if((str = XGetDefault(F->display,"xclust","cluststatforeground")) == NULL){ */
/*    str = "gray30"; */
/*  } */
/*  AssignColor(str, MININTERFACECOLOR + CLUSTSTATFOREGROUND); */

 if((str = XGetDefault(F->display,"xclust","menubackground")) == NULL){
   str = "ivory";
 }
 AssignColor(str, MININTERFACECOLOR + MENUBACKGROUND);
 
 if((str = XGetDefault(F->display,"xclust","menuforeground")) == NULL){
   str = "black";
 } 
 AssignColor(str, MININTERFACECOLOR + MENUFOREGROUND);
 
 if((str = XGetDefault(F->display,"xclust","menuitemforeground")) == NULL){
   str = "gray30";
 }
 AssignColor(str, MININTERFACECOLOR + MENUITEMFOREGROUND);
 
 if((str=XGetDefault(F->display,"xclust","menuitembackground")) == NULL){
   str = "gray70";
 }
 AssignColor(str, MININTERFACECOLOR + MENUITEMBACKGROUND);

 if((str=XGetDefault(F->display,"xclust","menuitemlightedge")) == NULL){
   str = "gray90";
 }
 AssignColor(str, MININTERFACECOLOR + MENUITEMLIGHTEDGE);
 
 if((str=XGetDefault(F->display,"xclust","menuitemdarkedge")) == NULL){
   str = "gray10";
 }
 AssignColor(str, MININTERFACECOLOR + MENUITEMDARKEDGE);
 
 if((str=XGetDefault(F->display,"xclust","menuitemdisable")) == NULL){
   str = "gray75";
 }
 AssignColor(str, MININTERFACECOLOR + MENUITEMDISABLE);
 
 if((str=XGetDefault(F->display,"xclust","menuiteminput")) == NULL){
   str = "gray60";
 }
 AssignColor(str, MININTERFACECOLOR + MENUITEMINPUT);
 
 if((str = XGetDefault(F->display,"xclust","textbackground")) == NULL){
   str = "gray30";
 }
 AssignColor(str, MININTERFACECOLOR + TEXTBACKGROUND);
 
 if((str = XGetDefault(F->display,"xclust","textforeground")) == NULL){
   str = "red";
 }
 AssignColor(str, MININTERFACECOLOR + TEXTFOREGROUND);
 
 if((str = XGetDefault(F->display,"xclust","graphbackground")) == NULL){
   str = "black";
 }
 AssignColor(str, MININTERFACECOLOR + GRAPHBACKGROUND);
 
 if((str = XGetDefault(F->display,"xclust","graphforeground")) == NULL){
   str = "ivory";
 }
 AssignColor(str, MININTERFACECOLOR + GRAPHFOREGROUND);
 
 if((str = XGetDefault(F->display,"xclust","menuframebackground")) == NULL){
   str = "darkslateblue";
 }
 AssignColor(str, MININTERFACECOLOR + MENUFRAMEBACKGROUND);
 
 if((str = XGetDefault(F->display,"xclust","menuframeforeground")) == NULL){
   str = "ivory";
 }
 AssignColor(str, MININTERFACECOLOR + MENUFRAMEFOREGROUND);
 
 if((str = XGetDefault(F->display,"xclust","highlight1")) == NULL){
   str = "firebrick";
 }
 AssignColor(str, MININTERFACECOLOR + HIGHLIGHT1);

 if((str = XGetDefault(F->display,"xclust","selection1")) == NULL){
   str = "maroon";
 }
 AssignColor(str, MININTERFACECOLOR + SELECTION1);

 if((str = XGetDefault(F->display,"xclust","selection2")) == NULL){
   str = "gray90";
 }
 AssignColor(str, MININTERFACECOLOR + SELECTION2);

 if((str = XGetDefault(F->display,"xclust","zerocluster")) == NULL){
   str = "white";
 }
 AssignColor(str, MININTERFACECOLOR + ZEROCLUSTER);

 if((str = XGetDefault(F->display,"xclust","xaxis")) == NULL){
   str = "blue";
 }
 AssignColor(str, MININTERFACECOLOR + XAXIS);

 if((str = XGetDefault(F->display,"xclust","yaxis")) == NULL){
   str = "green";
 }
 AssignColor(str, MININTERFACECOLOR + YAXIS);

 if((str = XGetDefault(F->display,"xclust","zaxis")) == NULL){
   str = "red";
 }
 AssignColor(str, MININTERFACECOLOR + ZAXIS);

 if((str = XGetDefault(F->display,"xclust","axeslabels")) == NULL){
   str = "beige";
 }
 AssignColor(str, MININTERFACECOLOR + AXESLABELS);

 if((str = XGetDefault(F->display,"xclust","axesticks")) == NULL){
   str = "gray70";
 }
 AssignColor(str, MININTERFACECOLOR + AXESTICKS);

 if((str = XGetDefault(F->display,"xclust","axesticklabels")) == NULL){
   str = "gray70";
 }
 AssignColor(str, MININTERFACECOLOR + AXESTICKLABELS);

 if((str = XGetDefault(F->display,"xclust","gridcolor")) == NULL){
   str = "orange";
 }
 AssignColor(str, MININTERFACECOLOR + GRIDCOLOR);


 /* create cluster color map */

 count = MAXINTERFACECOLORS;
 nr = 3;
 ng = 3;
 nb = 3;
 maxint = ceil((float) MAXCLUSTER/(nr*ng*nb));

 for(h=0; h<maxint; h++) {
   intensity = (float) (maxint - h)/maxint ;
   for(i=1; i<=nb; i++) {
     b = (int) ((float) intensity * (nb - i) * 65535 / (nb-1) );
     for(j=1; j<=ng; j++) {
       g = (int) ((float) intensity * (ng - j) * 65535 / (ng-1) );
       for(k=1; k<=nr; k++) {

	 if (count>=MAXCOLORS)
	   break;
 
	 r = (int) ((float) intensity * (nr - k) * 65535 / (nr-1) );

	 SetPixelRGB(count,r,g,b);
	 count++;

       }
     }
   }
 }


/*  /\* create a smooth spectral scale for the cluster colors *\/  */
/*  spectral_scale = 2*(MAXCLUSTER + 1); */
 
/*  for(i=0; i<MAXCLUSTER; i++){ */
/*    r = (int)((carc(2*i- 2*spectral_scale/16)))<< 8 ; */
/*    g = (int)((carc(2*i- 1*spectral_scale/16)))<< 8 ; */
/*    b = (int)((carc((float)2*i)))<< 8 ; */
/*    SetPixelRGB(MAXINTERFACECOLORS+i,r,g,b); */
/*   } */
 

/*  for (i=0; i<5; i++) */
/*    for (j=0; j<5; j++) */
/*      for (k=0; k<5; k++) */
/*        { */
/* 	 idx[count].r = 65535 - (i*16384); if(idx[count].r<0) idx[count].r=0; */
/* 	 idx[count].g = 65535 - (j*16384); if(idx[count].g<0) idx[count].g=0; */
/* 	 idx[count].b = 65535 - (k*16384); if(idx[count].b<0) idx[count].b=0; */
/* 	 alloc_color(count++, idx[count].r, idx[count].g, idx[count].b); */
/*        } */
 
/*  for (i=0; i<4; i++){ */
/*    for (j=0; j<4; j++){ */
/*      for (k=0; k<4; k++){ */
/*        idx[count].r = 60415 - (i*16384); if(idx[count].r<0) idx[count].r=0; */
/*        idx[count].g = 60415 - (j*16384); if(idx[count].g<0) idx[count].g=0; */
/*        idx[count].b = 60415 - (k*16384); if(idx[count].b<0) idx[count].b=0; */
/*        alloc_color(count++, idx[count].r, idx[count].g, idx[count].b); */
/*        idx[count].r = 55295 - (i*16384); if(idx[count].r<0) idx[count].r=0; */
/*        idx[count].g = 55295 - (j*16384); if(idx[count].g<0) idx[count].g=0; */
/*        idx[count].b = 55295 - (k*16384); if(idx[count].b<0) idx[count].b=0; */
/*        alloc_color(count++, idx[count].r, idx[count].g, idx[count].b); */
/*        if(count > 243) break; */
/*      } */
/*      if(count > 243) break; */
/*    } */
/*    if(count > 243) break; */
/*  } */
/*  count = SmoothRange(count); */

 /*
   count = SmoothRange2(count);
   count = SmoothRange3(count);
   count = SmoothRange4(count);
   count = SmoothRange5(count);
 */
 fprintf(stderr,"allocated %d colors\n",MAXCOLORS);


 /* disable cluster color specification through X resource manager */
 
/*     /\* Get colors for the clusters *\/ */
/*     for(k=0; k<=50; k++) { */
/*         sprintf(optstr, "cluster%d", k); */
/*         if(str = XGetDefault(F->display,"xclust",optstr)){ */
/*   	    AssignColor(str, MINCLUSTER + k); */
/*         } */
/*     } */
}

int get_color(char *col)
{
int i, cindx;
double rd, gd, bd, dist, mindist;
XColor color;
XColor def_colrs[256];

    // create a color from the input string
    XParseColor(display, cmap, col, &color);

    // find closest match
    cindx = -1;
    mindist = 196608.0;             // 256.0 * 256.0 * 3.0
    for (i=0; i<256; i++)
    {
	rd = (idx[i].r - color.red) / 256.0;
	gd = (idx[i].g - color.green) / 256.0;
	bd = (idx[i].b - color.blue) / 256.0;
	dist = (rd * rd) + (gd * gd) + (bd * bd);
	if (dist < mindist)
	{
	    mindist = dist;
	    cindx = idx[i].pixel;
	    if (dist == 0.0) break;
	}
    }
    return cindx;
}


void MakeColormap(frame)
     Frame	*frame;
{
int	i, j, k;

    display = frame->display;
/*     color_max = MAXCOLORS - 1; */
/*     color_scale = color_max - MINCOLOR; */
/*     Scale = color_scale +1; */
    cmap = DefaultColormap(frame->display,frame->screen_number);
    /*
    XSetWindowColormap(F->display,F->window,cmap);
    */
  
    private_colormap();
/*     return(color_scale); */
}

#define RANGE1	100

int SmoothRange(count)
int	count;
{
int	i;
float	spectral_scale;
unsigned short r,g,b;
int	smoothrange;

    spectral_scale = 2*(RANGE1 + 1);
    /*
    ** create a smooth spectral scale at the upper end of the map
    */
  for(i=0; i<RANGE1; i++){
	r = (int)((carc(2*i- 2*spectral_scale/16)))<< 8 ;
	g = (int)((carc(2*i- 1*spectral_scale/16)))<< 8 ;
	b = (int)((carc((float)2*i)))<< 8 ;
	SetPixelRGB(count++,r,g,b);
	if(i> 9) break;
    }
    return(count);
}

int SmoothRange2(count)
int	count;
{
int	i;
float	spectral_scale;
unsigned short r,g,b;
int	smoothrange;

    spectral_scale = 2*(RANGE1 + 1);
    /*
    ** create a smooth spectral scale at the upper end of the map
    */
  for(i=0; i<RANGE1; i++){
	g = (int)((carc(2*i- 2*spectral_scale/16)))<< 8 ;
	b = (int)((carc(2*i- 1*spectral_scale/16)))<< 8 ;
	r = (int)((carc((float)2*i)))<< 8 ;
	SetPixelRGB(count++,r,g,b);
	if(i> 20) break;
    }
    return(count);
}

int SmoothRange3(count)
int	count;
{
int	i;
float	spectral_scale;
unsigned short r,g,b;
int	smoothrange;

    spectral_scale = 2*(RANGE1 + 1);
    /*
    ** create a smooth spectral scale at the upper end of the map
    */
  for(i=0; i<RANGE1; i++){
	b = (int)((carc(2*i- 2*spectral_scale/16)))<< 8 ;
	g = (int)((carc(2*i- 1*spectral_scale/16)))<< 8 ;
	r = (int)((carc((float)2*i)))<< 8 ;
	SetPixelRGB(count++,r,g,b);
	if(i> 20) break;
    }
    return(count);
}

int SmoothRange4(count)
int	count;
{
int	i;
float	spectral_scale;
unsigned short r,g,b;
int	smoothrange;

    spectral_scale = 201;
    /*
    ** create a smooth spectral scale at the upper end of the map
    */
  for(i=0; i<200; i+=5){
	r = (int)((carc2(1*(i- 2*spectral_scale/3.5))))<< 8 ;
	g = (int)((carc2(1*(i- 1*spectral_scale/3.5))))<< 8 ;
	b = (int)((carc2(1*(float)i)))<< 8 ;
	SetPixelRGB(count++,r,g,b);
    }
    return(count);
}

int SmoothRange5(count)
int	count;
{
int	i;
float	spectral_scale;
unsigned short r,g,b;
int	smoothrange;

    spectral_scale = 201;
    /*
    ** create a smooth spectral scale at the upper end of the map
    */
  for(i=0; i<200; i+=5){
	g = (int)((carc2(1*(i- 2*spectral_scale/3.5))))<< 8 ;
	r = (int)((carc2(1*(i- 1*spectral_scale/3.5))))<< 8 ;
	b = (int)((carc2(1*(float)i)))<< 8 ;
	SetPixelRGB(count++,r,g,b);
    }
    return(count);
}


AssignColor(cname, pixval)
     char	*cname;
     int	pixval;
{
    alloc_named_color(pixval, cname);
}


/* ColorMap function not used anymore */

/* int ColorMap(i) */
/* int i; */
/* { */
/* extern int PSStatus(); */
/* int val; */

/*     if(i >= MAXCOLORS){ */
/* 	i = MAXCOLORS -1; */
/*     } */
/*     if(PSStatus() == 1) { */
/* 	if(G->color_mode){ */
/* 	    val = color[i].pixel;  */
/* 	} else { */
/* 	    val = (225*(i-MINCOLOR+2))/(color_scale); */
/* 	    if(val < 0) val = 0; */
/* 	    if(val > 255) val = 255; */
/* 	} */
/* 	return(val); */
/*     } else */
/*     if(G->color_mode){ */
/* 	return(color[i].pixel);  */
/*     } else { */
/* 	return(i); */
/*     } */
/* } */


ChangeColor(pixel)
int	pixel;
{
XColor	color;

    color.red = (int)(frandom(0,65535));
    color.green = (int)(frandom(0,65535));
    color.blue = (int)(frandom(0,65535));
    color.flags = (int)(DoRed|DoGreen|DoBlue);
    color.pixel = pixel;
    /*
    if(F->colormaptype == DEFAULT || F->colormaptype == PRIVATE){
    XStoreColor(F->display,cmap,&color);
    }
    */
    alloc_color(pixel,color.red,color.green,color.blue);
}

SetPixelRGB(pixel,r,g,b)
int	pixel;
unsigned short	r,g,b;
{
XColor	color;
unsigned short red,green,blue;

    color.red = r;
    color.green = g;
    color.blue = b;
    color.flags = (int)(DoRed|DoGreen|DoBlue);
    color.pixel = pixel;
    /*
    if(F->colormaptype == DEFAULT || F->colormaptype == PRIVATE){
    XStoreColor(F->display,cmap,&color);
    }
    */
    alloc_color(pixel,color.red,color.green,color.blue);
    GetPixelRGB(pixel,&red,&green,&blue);
}

unsigned long GetPixel(arrayid)
int	arrayid;
{
	return(idx[arrayid].pixel);
}

GetPixelRGB(pixel,r,g,b)
int	pixel;
unsigned short	*r,*g,*b;
{
XColor	color;

    color.pixel = GetPixel(pixel);
    XQueryColor(F->display,cmap,&color);
    *r = color.red;
    *g = color.green;
    *b = color.blue;
}

/* no longer called, appears to have been replaced by ChangeColor
/* TJD, 12-30-04 */

/* CyclePixel(pixel,rmask,gmask,bmask,rinc,ginc,binc) */
/* int	pixel; */
/* int	rinc,ginc,binc; */
/* { */
/* XColor	color; */
/* XColor	newcolor; */

/*     color.pixel = pixel; */
/*     XQueryColor(F->display,cmap,&color); */
/*     if(rmask){ */
/* 	newcolor.red = color.red + rinc<<8; */
/*     } else { */
/* 	newcolor.red = 0; */
/*     } */
/*     if(gmask){ */
/* 	newcolor.green = color.green + ginc<<8; */
/*     } else { */
/* 	newcolor.green = 0; */
/*     } */
/*     if(gmask){ */
/* 	newcolor.blue = color.blue + binc<<8; */
/*     } else { */
/* 	newcolor.blue = 0; */
/*     } */
/*     newcolor.flags = (int)(DoRed|DoGreen|DoBlue); */
/*     newcolor.pixel = pixel; */
/*     /\* */
/*     if(F->colormaptype == DEFAULT || F->colormaptype == PRIVATE){ */
/*     XStoreColor(F->display,cmap,&newcolor); */
/*     } */
/*     *\/ */
/*     alloc_color(pixel,newcolor.red,newcolor.green,newcolor.blue); */
/* } */
