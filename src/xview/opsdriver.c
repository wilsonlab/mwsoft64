/*
**		PSdriver
** 		X11 -> postscript library routines
**		Copyright 1989  Matt Wilson 
** 		California Institute of Technology
**		wilson@smaug.cns.caltech.edu
**
** Permission to use, copy, modify, and distribute this
** software and its documentation for any purpose and without
** fee is hereby granted, provided that the above copyright
** notice appear in all copies and that both that copyright
** notice and this permission notice appear in supporting
** documentation, and that the name of the California Institute
** of Technology not be used in advertising or publicity pertaining 
** to distribution of the software without specific, written prior 
** permission.  Neither the author nor California Institute of Technology 
** make any representations about the suitability of this software for 
** any purpose. It is provided "as is" without express or implied warranty.
**
** To use these functions
** replace the X11 calls in your code with calls to the
** comparable driver functions. 
** e.g. instead of 
**	XDrawLine (display, drawable, context, x1, y1, x2, y2)
** use
**	XPSDrawLine (display, drawable, context, x1, y1, x2, y2)
**
** output can be directed to a file or directly to a printer using 
**	SetPSFileOutput(state)		1=file 0=printer
**	SetPSFilename(name)
**
** to produce postscript, output add this sequence of calls to your code
**
** PreparePS(display,window,scale,box,header);
** refresh_window(window);		your routine to refresh the screen
** FinishPS();
**
*/

#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>
#include <signal.h>

typedef struct {
    short x,y;
} Coord;

typedef struct {
    float x,y;
} FCoord;

#define XOUT	0
#define PSOUT	1

#define DPI	300
#define PAGEHEIGHT	11.0
#define PAGEWIDTH	8.5
/* default user space coords are 1/72 of an inch each */
#define DEFAULTRES	72

static int	output_flag = XOUT;
static FILE 	*PSfp;
static int 	global_window_height;
static int 	global_window_width;
static float 	pix_scale;
static float 	page_scale;
static short 	file_output = 0;
static char 	*ps_filename;
static char 	*ps_filemode = "w";
static int 	current_gray = -1;
static int 	current_line = -1;
static int 	max_gray = 256;
static int	inverse = 1;
static int	inverse_gray = 0;

PSStatus()
{
    return(output_flag);
}

SetPSInverseGray(state)
int state;
{
    inverse_gray = state;
}

SetPSInverse(state)
int state;
{
    inverse = state;
}

SetPSFilename(name)
char *name;
{
    ps_filename = name;
}

SetPSFilemode(mode)
char *mode;
{
    ps_filemode = mode;
}

SetPSFileOutput(state)
int state;
{
    file_output = state;
}

SetMaxGray(val)
int	val;
{
    max_gray = val;
}

float ComputeGray(pixel)
int pixel;
{
    if(inverse_gray){
	return((float)pixel/max_gray);
    } else {
	return(1.0-(float)pixel/max_gray);
    }
}

PSSetPixel(display,pixel)
Display *display;
int pixel;
{
XColor color;
int intensity;

    if(pixel != current_gray){
	/*
	color.pixel = pixel;
	XQueryColor(display,DefaultColormap(display,XDefaultScreen(display)),&color);
	intensity = (color.red + color.green + color.blue)/3;
	*/
	current_gray = pixel;
	if(pixel == XBlackPixel(display,XDefaultScreen(display))){
	    fprintf(PSfp,"%d G\n",inverse);
	} else 
	if(pixel == XWhitePixel(display,XDefaultScreen(display))){
	    fprintf(PSfp,"%d G\n",!inverse);
	} else {
	    fprintf(PSfp,"%5.3f G\n",ComputeGray(pixel));
	}
    }
}

PSSetLineWidth(width)
int width;
{
    if(width == 0) width = 1;
    if(current_line != width){ 
	current_line = width;
	fprintf(PSfp,"%f setlinewidth\n",current_line/pix_scale);
    }
}

PSClosefill()
{
    fprintf(PSfp,"C\n");
}

PSNewpath()
{
    fprintf(PSfp,"N\n");
}

PSStroke()
{
    fprintf(PSfp,"S\n");
}

PSMoveto(x,y)
int x,y;
{
    fprintf(PSfp,"%d %d M\n", x, global_window_height - y);
}

PSLineto(x,y)
int x,y;
{
    fprintf(PSfp,"%d %d L\n", x, global_window_height - y);
}

PSMovetoFloat(x,y)
float x,y;
{
    fprintf(PSfp,"%g %g M\n", x, global_window_height - y);
}

PSLinetoFloat(x,y)
float x,y;
{
    fprintf(PSfp,"%g %g L\n", x, global_window_height - y);
}

PSShow(s)
char *s;
{
    fprintf(PSfp,"(%s) show\n",s);
}

PSFont(height)
int height;
{
    fprintf(PSfp,"/Helvetica findfont %d scalefont setfont\n",height);
}

XPSDrawText(display,drawable,context,x,y,s)
Display 	*display;
Drawable	drawable;
GC		context;
int 		x, y;
char 		*s;
{
XFontStruct *finfo;
int	height;

    if(output_flag == XOUT){
	XDrawImageString(display,drawable,context,x,y,s,strlen(s));
    } else{
	finfo  = XQueryFont(display,context->gid);
	height = finfo->ascent + finfo->descent;
	PSFont(height);
	PSMoveto(x,y);
	PSShow(s);
    }
}

XPSDrawLine (display, drawable, context, x1, y1, x2, y2)
Display	*display;
Drawable drawable;
GC 	context;
int 	x1,y1,x2,y2;
{
int	pixel;

    if(output_flag == XOUT){
	XDrawLine (display, drawable, context, x1,y1,x2,y2);
    } else {
	/*
	** get the current foreground color from the graphics context
	*/
	PSSetPixel(display,context->values.foreground);
	PSSetLineWidth(context->values.line_width);
	PSNewpath();
	PSMoveto(x1,y1);
	PSLineto(x2,y2);
	PSStroke();
    }
}

#define MAXCHUNK 10000

XPSDrawLines(display,drawable,context,coord,ncoords,mode)
Display *display;
Drawable drawable;
GC context;
Coord   *coord;
int     ncoords;
int     mode;
{
int	i;
int	pcount;
int	nchunks;
int	chunksize;

    if(output_flag == XOUT){
	/*
	** avoid the limit on the length of a multiple line vector
	** by doing it in multiple calls
	*/
	nchunks = (ncoords-1)/MAXCHUNK + 1;
	for(i=0;i<nchunks;i++){
	    if((chunksize = ncoords - i*MAXCHUNK) > MAXCHUNK){
		chunksize = MAXCHUNK;
	    }
	    /*
	    ** draw one point past to connect this chunk with the 
	    ** next. Dont do it for the last chunk
	    */
	    if(i < nchunks-1) chunksize++;
	    XDrawLines(display,drawable,context,
	    coord + i*MAXCHUNK,chunksize,mode);
	}
    } else {
	if(ncoords <= 0 ){
	    return;
	}
	PSNewpath();
	PSSetPixel(display,context->values.foreground);
	PSSetLineWidth(context->values.line_width);
	PSMoveto(coord[0].x,coord[0].y);
	pcount = 0;
	for(i=1;i<ncoords;i++){
	    if((coord[i].x == coord[i-1].x) && (coord[i].y == coord[i-1].y) &&
	    i < ncoords-1){
		continue;
	    }
	    PSLineto(coord[i].x,coord[i].y);
	    /*
	    * break it up into managable cuhnks
	    */
	    if(pcount > 200){
		PSStroke();
		PSNewpath();
		PSMoveto(coord[i].x,coord[i].y);
		pcount = 0;
	    }
	    pcount++;
	}
	PSStroke();
    }
}

XPSDrawLinesFloat(display,drawable,context,coord,ncoords,mode,fcoord)
Display *display;
Drawable drawable;
GC context;
Coord   *coord;
int     ncoords;
int     mode;
FCoord   *fcoord;
{
int	i;
int	pcount;
int	nchunks;
int	chunksize;

    if(output_flag == XOUT){
	/*
	** avoid the limit on the length of a multiple line vector
	** by doing it in multiple calls
	*/
	nchunks = (ncoords-1)/MAXCHUNK + 1;
	for(i=0;i<nchunks;i++){
	    if((chunksize = ncoords - i*MAXCHUNK) > MAXCHUNK){
		chunksize = MAXCHUNK;
	    }
	    /*
	    ** draw one point past to connect this chunk with the 
	    ** next. Dont do it for the last chunk
	    */
	    if(i < nchunks-1) chunksize++;
	    XDrawLines(display,drawable,context,
	    coord + i*MAXCHUNK,chunksize,mode);
	}
    } else {
	if(ncoords <= 0 ){
	    return;
	}
	PSNewpath();
	PSSetPixel(display,context->values.foreground);
	PSSetLineWidth(context->values.line_width);
	PSMovetoFloat(fcoord[0].x,fcoord[0].y);
	pcount = 0;
	for(i=1;i<ncoords;i++){
	    if((fcoord[i].x == fcoord[i-1].x) && (fcoord[i].y == fcoord[i-1].y) &&
	    i < ncoords-1){
		continue;
	    }
	    PSLinetoFloat(fcoord[i].x,fcoord[i].y);
	    /*
	    * break it up into managable cuhnks
	    */
	    if(pcount > 200){
		PSStroke();
		PSNewpath();
		PSMovetoFloat(fcoord[i].x,fcoord[i].y);
		pcount = 0;
	    }
	    pcount++;
	}
	PSStroke();
    }
}

XPSDrawRectangle(display,drawable,context,x,y,w,h)
Display *display;
Drawable drawable;
GC context;
int     x,y,w,h;
{
    if(output_flag == XOUT){
	XDrawRectangle(display,drawable,context,x,y,w,h);
    } else {
	PSSetPixel(display,context->values.foreground);
	PSSetLineWidth(context->values.line_width);
	PSNewpath();
	PSMoveto(x,y);
	PSLineto(x+w,y);
	PSLineto(x+w,y+h);
	PSLineto(x,y+h);
	PSLineto(x,y);
	PSStroke();
    }
}

XPSFillRectangle(display,drawable,context,x,y,w,h)
Display *display;
Drawable drawable;
GC context;
int     x,y,w,h;
{
    if(output_flag == XOUT){
	XFillRectangle(display,drawable,context,x,y,w,h);
    } else {
	PSSetPixel(display,context->values.foreground);
	PSNewpath();
	PSMoveto(x,y);
	PSLineto(x+w,y);
	PSLineto(x+w,y+h);
	PSLineto(x,y+h);
	PSClosefill();
    }
}

XPSFillPolygon(display,drawable,context,coord,ncoords,shape,mode)
Display *display;
Drawable drawable;
GC context;
Coord   *coord;
int     ncoords;
int     shape;
int     mode;
{
int	i;
int	pcount;

    if(output_flag == XOUT){
	XFillPolygon(display,drawable,context,coord,ncoords,shape,mode);
    } else{
	if(ncoords <= 0){
	    return;
	}
	PSNewpath();
	PSSetPixel(display,context->values.foreground);
	PSMoveto(coord[0].x,coord[0].y);
	pcount = 0;
	for(i=1;i<ncoords;i++){
	    PSLineto(coord[i].x,coord[i].y);
	    /*
	    * break it up into managable cuhnks
	    */
	    if(pcount > 200){
		PSClosefill();
		PSMoveto(coord[i].x,coord[i].y);
		pcount = 0;
	    }
	    pcount++;
	}
	PSClosefill();
    }
}

PSHeader(info,requested_scale,box,header)
XWindowAttributes	*info;
float		requested_scale;
int		box;
int		header;
{
float	scale,scalex,scaley;
long	clock;
char	headerstr[200];
char	*namestr;
char	*getenv(),*strchr();
char	*ptr;
char	clockstr[100];

    fprintf(PSfp,"%%!\n");
    fprintf(PSfp,"initgraphics\n");
    fprintf(PSfp,"/M { moveto } def\n");
    fprintf(PSfp,"/L { lineto } def\n");
    fprintf(PSfp,"/S { stroke } def\n");
    fprintf(PSfp,"/N { newpath } def\n");
    fprintf(PSfp,"/C { closepath fill } def\n");
    fprintf(PSfp,"/G { setgray } def\n");

    if(header){
	if((namestr = getenv("NAME")) == NULL){
	    namestr = "";
	}
	time(&clock);
	strcpy(clockstr,ctime(&clock));
	if(ptr = strchr(clockstr,'\n')){
	    *ptr = '\0';
	}
	sprintf(headerstr,"%s ::  %s ::  %s", 
	clockstr, getcwd(NULL,100),namestr);
	PSNewpath();
	PSFont(12);
	fprintf(PSfp,"75 15 M\n");
	PSShow(headerstr);
	PSStroke();
    }

    global_window_width = info->width;
    global_window_height = info->height;

    /*
    ** calculate the scale factors in inches/screenpixel
    */
    scalex = PAGEWIDTH/global_window_width;
    scaley = PAGEHEIGHT/global_window_height;
    /*
    ** use the smaller scale factor so that everything will fit on the page
    */
    scale = requested_scale*((scalex < scaley) ? scalex : scaley);
    /*
    ** calculate the dots/screenpixel scale factor
    */
    pix_scale = scale*DPI;
    /*
    * convert to postscriptpoints/screenpixel
    */
    page_scale = scale*DEFAULTRES;

    /*
    ** center it on the page
    */
    fprintf(PSfp,"%f %f translate\n",
	DEFAULTRES*(PAGEWIDTH - scale*global_window_width)/2.0,
	DEFAULTRES*(PAGEHEIGHT - scale*global_window_width)/2.0);

    fprintf(PSfp,"%f %f scale\n",page_scale,page_scale);

    if(box){
	fprintf(PSfp,"0.5 G\n");
	PSNewpath();
	PSMoveto(0,0);
	PSLineto(0,global_window_height);
	PSLineto(global_window_width,global_window_height);
	PSLineto(global_window_width,0);
	PSLineto(0,0);
	PSStroke();
    }
    
    PSNewpath();
    PSMoveto(0,0);
    PSLineto(0,global_window_height);
    PSLineto(global_window_width,global_window_height);
    PSLineto(global_window_width,0);
    PSLineto(0,0);
    fprintf(PSfp,"eoclip\n");
}

PSTrailer()
{
    fprintf(PSfp,"showpage ");
}


PreparePS(display,window,scale,box,header)
Display	*display;
Window 	window;
float	scale;
int	box;
int	header;
{
char command[80];
XWindowAttributes	info;
char *printer;

    current_gray = -1;
    current_line = -1;
    XGetWindowAttributes(display,window,&info);
    if(file_output){
	PSfp = fopen(ps_filename,ps_filemode);
    } else {
	if((printer = (char *)getenv("PRINTER")) == NULL){
	    printer = "lw";
	}
	sprintf(command,"lpr -h -P%s",printer);
	PSfp = popen(command,"w");
    }
    PSHeader(&info,scale,box,header);
    output_flag = PSOUT;
}

FinishPS()
{
    PSTrailer();
    output_flag = XOUT;
    if(file_output){
	fclose(PSfp);
    } else {
	pclose(PSfp);
    }
}

