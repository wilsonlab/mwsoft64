/*
**		PSdriver
** 		X11 -> postscript library routines
**		Copyright 1989,1990  Matt Wilson 
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
#include <stdlib.h>
#include <time.h>
#include <X11/Xlib.h>
#include <signal.h>

/*
typedef struct {
    int16_t x,y;
} Coord;
*/
typedef XPoint Coord;

typedef struct {
    float x,y;
} FCoord;

struct defaultinfo_type {
    int32_t		whitepixel;
    int32_t		blackpixel;
    int32_t		foreground;
    int32_t		linewidth;
    int32_t		font_width;
    int32_t		font_height;
    int32_t		font_color;
};

typedef struct color_info {
    float	r;
    float	g;
    float	b;
} ColorInfo;



#define XOUT	0
#define PSOUT	1

/* max points in a single multiple line vector call in X windows */
#define MAXCHUNK 10000

/* max strokes in a single postscript path */
#define MAXSTROKES 200

#define DPI	300
#define PAGEHEIGHT	11.0
#define PAGEWIDTH	8.5
/* default user space coords are 1/72 of an inch each */
#define DEFAULTRES	72

#define DEFAULT_BLACK_PIXEL	1
#define DEFAULT_WHITE_PIXEL	0
#define DEFAULT_FONT_HEIGHT	5
#define DEFAULT_FONT_WIDTH	3
#define DEFAULTFONT "/Helvetica"

struct defaultinfo_type defaultinfo = {
	DEFAULT_WHITE_PIXEL,
	DEFAULT_BLACK_PIXEL,
	DEFAULT_WHITE_PIXEL,
	0,
	DEFAULT_FONT_WIDTH,
	DEFAULT_FONT_HEIGHT,
	DEFAULT_WHITE_PIXEL};

static int32_t	output_flag = XOUT;
static FILE 	*PSfp;
static float 	pix_scale;
static float 	page_scale;
static int16_t 	file_output = 0;
static char 	*ps_filename;
static char 	*ps_filemode = "w";
static int32_t 	current_gray = -1;
static int32_t 	current_line = -1;
static int32_t 	max_gray = 256;
static int32_t	inverse = 1;
static int32_t 	global_window_width;
static int32_t 	global_window_height;
static int32_t 	color_mode = 0;
static ColorInfo	current_color;
static Colormap		window_colormap;
static int32_t		fontselected = 0;
static char		currentfont[200];

PSStatus()
{
    return(output_flag);
}

SetPSColor(state)
int32_t state;
{
    color_mode = state;
}


SetPSInverse(state)
int32_t state;
{
    inverse = state;
}

SetPSFilename(name)
char *name;
{
    ps_filename = name;
}

SetPSDefaultForeground(val)
int32_t val;
{
    defaultinfo.foreground = val;
}

SetPSFilemode(mode)
char *mode;
{
    ps_filemode = mode;
}

SetPSFileOutput(state)
int32_t state;
{
    file_output = state;
}

SetMaxGray(val)
int32_t	val;
{
    max_gray = val;
}

float ComputeGray(pixel)
int32_t pixel;
{
    return(1.0-(float)pixel/max_gray);
}

PSSetPixel(display,pixel)
Display *display;
int32_t pixel;
{
XColor color;
int32_t intensity;
int32_t	black_pixel;
int32_t	white_pixel;

    /*
    ** has the color/grayscale changed from its previous state?
    */
    if(pixel != current_gray){
	current_gray = pixel;
	/*
	** are we doing RGB or grayscale PostScript
	*/
	if(color_mode && display){
	    color.pixel = pixel;
	    /*
	    XQueryColor(display,DefaultColormap(display,
		XDefaultScreen(display)),&color);
	    */
	    /*
	    ** get the RGB value of the pixel from the colormap
	    ** associated with the window
	    */
	    XQueryColor(display,window_colormap,&color);
	    intensity = (color.red + color.green + color.blue)/3;
	    /*
	    ** convert to postscript 0-1 range
	    */
	    current_color.r = (color.red>>8)/255.0;
	    current_color.g = (color.green>>8)/255.0;
	    current_color.b = (color.blue>>8)/255.0;
	    fprintf(PSfp,"%5.3f %5.3f %5.3f R\n",
		current_color.r,
		current_color.g,
		current_color.b);
	} else {
	    /*
	    ** deal with black and white pixels
	    */
	    if(display){
		black_pixel = XBlackPixel(display,XDefaultScreen(display));
		white_pixel = XWhitePixel(display,XDefaultScreen(display));
	    } else {
		black_pixel = defaultinfo.blackpixel;
		white_pixel = defaultinfo.whitepixel;
	    }
	    if(pixel == black_pixel){
		fprintf(PSfp,"%d G\n",inverse);
	    } else 
	    if(pixel == white_pixel){
		fprintf(PSfp,"%d G\n",!inverse);
	    } else {
		fprintf(PSfp,"%5.3f G\n",ComputeGray(pixel));
	    }
	}
    }
}

PSSetLineWidth(width)
int32_t width;
{
    if(width == 0) width = 1;
    if(current_line != width){ 
	current_line = width;
	/*
	** this seems to make the lines too narrow with respect
	** to the displayed lines in X
	fprintf(PSfp,"%f setlinewidth\n",current_line/pix_scale);
	*/
	fprintf(PSfp,"%d setlinewidth\n",current_line);
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

PSCircle(x,y,r,arcstart,arcend)
int32_t x,y;
{
    fprintf(PSfp,"%d %d %d %d %d arc\n", x, global_window_height - y,
    r,arcstart,arcend);
}

PSMoveto(x,y)
int32_t x,y;
{
    fprintf(PSfp,"%d %d M\n", x, global_window_height - y);
}

PSLineto(x,y)
int32_t x,y;
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


PSSetFont(name)
char	*name;
{
    fontselected = 1;
    strcpy(currentfont,name);
}

PSFont(height)
int32_t height;
{
    fprintf(PSfp,"%s findfont %d scalefont setfont\n",currentfont,height);
}

XPSFontSize(display,drawable,context)
Display 	*display;
Drawable	drawable;
GC		context;
{
}

ManualSetFontInfo(width,height,color)
int32_t	width;
int32_t	height;
int32_t	color;
{
    defaultinfo.font_width = width;
    defaultinfo.font_height = height;
    defaultinfo.font_color = color;
}

XPSDrawText(display,drawable,context,x,y,s)
Display 	*display;
Drawable	drawable;
GC		context;
int32_t 		x, y;
char 		*s;
{
XFontStruct *finfo;
int32_t	height;
int32_t	color;

    if(s==NULL) return;
    if(output_flag == XOUT){
	XDrawImageString(display,drawable,context,x,y,s,strlen(s));
    } else{
#ifdef GC
	if(display){
	    finfo  = XQueryFont(display,context->gid);
	    height = finfo->ascent + finfo->descent;
	    color = context->values.foreground;
	} else {
	    height = defaultinfo.font_height;
	    color = defaultinfo.font_color;
	}
#else
	    height = defaultinfo.font_height;
	    color = defaultinfo.font_color;
#endif
	/*
	** added PSSetPixel call : 11/19/90  M.Wilson
	*/
	PSSetPixel(display,color);
	PSFont(height);
	PSMoveto(x,y);
	PSShow(s);
    }
}
int32_t PSGetLineWidth(display,context)
Display	*display;
GC 	context;
{
#ifdef GC
    if(display){
	return((int32_t)context->values.line_width);
    } else {
	return(defaultinfo.linewidth);
    }
#else
	return(defaultinfo.linewidth);
#endif
}

int32_t PSGetForeground(display,context)
Display	*display;
GC 	context;
{
#ifdef GC
    if(display){
	return((int32_t)context->values.foreground);
    } else {
	return(defaultinfo.foreground);
    }
#else
	return(defaultinfo.foreground);
#endif
}

XPSDrawArc (display, drawable, context, x, y, xd,yd,arcstart,arcend)
Display	*display;
Drawable drawable;
GC 	context;
int32_t 	x,y,xd,yd,arcstart,arcend;
{
int32_t	pixel;
int32_t	linewidth;
int32_t	cx,cy,r;

    if(output_flag == XOUT){
	XDrawArc(display,drawable,context,x,y,xd,yd,arcstart,arcend);
    } else {
	cx = x + xd/2;
	cy = y + yd/2;
	r = (xd + yd)/4;	/* average x y radius */
	/*
	** get the current foreground color from the graphics context
	*/
	pixel = PSGetForeground(display,context);
	linewidth = PSGetLineWidth(display,context);
	PSSetPixel(display,pixel);
	PSSetLineWidth(linewidth);
	PSNewpath();
	PSCircle(cx,cy,r,arcstart/64,arcend/64);
	PSStroke();
    }
}

XPSFillArc (display, drawable, context, x, y, xd,yd,arcstart,arcend)
Display	*display;
Drawable drawable;
GC 	context;
int32_t 	x,y,xd,yd,arcstart,arcend;
{
int32_t	pixel;
int32_t	linewidth;
int32_t	cx,cy,r;

    if(output_flag == XOUT){
	XFillArc(display,drawable,context,x,y,xd,yd,arcstart,arcend);
    } else {
	cx = x + xd/2;
	cy = y + yd/2;
	r = (xd + yd)/4;	/* average x y radius */
	/*
	** get the current foreground color from the graphics context
	*/
	pixel = PSGetForeground(display,context);
	linewidth = PSGetLineWidth(display,context);
	PSSetPixel(display,pixel);
	PSSetLineWidth(linewidth);
	PSNewpath();
	PSCircle(cx,cy,r,arcstart/64,arcend/64);
	PSClosefill();
    }
}

XPSDrawPoint (display, drawable, context, x1, y1)
Display	*display;
Drawable drawable;
GC 	context;
int32_t 	x1,y1;
{
int32_t	pixel;
int32_t	linewidth;

    if(output_flag == XOUT){
	XDrawPoint (display, drawable, context, x1,y1);
    } else {
	/*
	** get the current foreground color from the graphics context
	*/
	pixel = PSGetForeground(display,context);
	linewidth = PSGetLineWidth(display,context);


	PSSetPixel(display,pixel);
	PSSetLineWidth(linewidth);
	PSNewpath();
	PSMoveto(x1,y1);
	PSLineto(x1+1,y1);
	PSStroke();
    }
}


XPSDrawLine (display, drawable, context, x1, y1, x2, y2)
Display	*display;
Drawable drawable;
GC 	context;
int32_t 	x1,y1,x2,y2;
{
int32_t	pixel;
int32_t	linewidth;

    if(output_flag == XOUT){
	XDrawLine (display, drawable, context, x1,y1,x2,y2);
    } else {
	/*
	** get the current foreground color from the graphics context
	*/
	pixel = PSGetForeground(display,context);
	linewidth = PSGetLineWidth(display,context);


	PSSetPixel(display,pixel);
	PSSetLineWidth(linewidth);
	PSNewpath();
	PSMoveto(x1,y1);
	PSLineto(x2,y2);
	PSStroke();
    }
}


XPSDrawLines(display,drawable,context,coord,ncoords,mode)
Display *display;
Drawable drawable;
GC context;
Coord   *coord;
int32_t     ncoords;
int32_t     mode;
{
int32_t	i;
int32_t	pcount;
int32_t	nchunks;
int32_t	chunksize;
int32_t	foreground;
int32_t	linewidth;

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
	foreground = PSGetForeground(display,context);
	linewidth = PSGetLineWidth(display,context);
	PSSetPixel(display,foreground);
	PSSetLineWidth(linewidth);
	PSMoveto(coord[0].x,coord[0].y);
	pcount = 0;
	for(i=1;i<ncoords;i++){
	    if((coord[i].x == coord[i-1].x) && (coord[i].y == coord[i-1].y) &&
	    i < ncoords-1){
		continue;
	    }
	    PSLineto(coord[i].x,coord[i].y);
	    /*
	    ** break it up into managable cuhnks
	    */
	    if(pcount > MAXSTROKES){
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
int32_t     ncoords;
int32_t     mode;
FCoord   *fcoord;
{
int32_t	i;
int32_t	pcount;
int32_t	nchunks;
int32_t	chunksize;
int32_t	foreground;
int32_t	linewidth;

    if(output_flag == XOUT){
	/*
	** avoid the limit on the length of a multiple line vector
	** by doing it in multiple calls
	*/
	nchunks = (ncoords-1)/MAXCHUNK + 1;
	/*
	fprintf(stderr,"FML %d %d\n",ncoords,nchunks);
	*/
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
	    /*
	    fprintf(stderr,"%d %d\n",i*MAXCHUNK,chunksize);
	    */
	}
    } else {
	if(ncoords <= 0 ){
	    return;
	}
	PSNewpath();
	foreground = PSGetForeground(display,context);
	linewidth = PSGetLineWidth(display,context);
	PSSetPixel(display,foreground);
	PSSetLineWidth(linewidth);
	PSMovetoFloat(fcoord[0].x,fcoord[0].y);
	pcount = 0;
	for(i=1;i<ncoords;i++){
	    if((fcoord[i].x == fcoord[i-1].x) && 
	    (fcoord[i].y == fcoord[i-1].y) &&
	    i < ncoords-1){
		continue;
	    }
	    PSLinetoFloat(fcoord[i].x,fcoord[i].y);
	    /*
	    * break it up into managable cuhnks
	    */
	    if(pcount > MAXSTROKES){
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
int32_t     x,y,w,h;
{
int32_t	foreground;
int32_t	linewidth;
    if(output_flag == XOUT){
	XDrawRectangle(display,drawable,context,x,y,w,h);
    } else {
	foreground = PSGetForeground(display,context);
	linewidth = PSGetLineWidth(display,context);
	PSSetPixel(display,foreground);
	PSSetLineWidth(linewidth);
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
int32_t     x,y,w,h;
{
int32_t	foreground;
    if(output_flag == XOUT){
	XFillRectangle(display,drawable,context,x,y,w,h);
    } else {
	foreground = PSGetForeground(display,context);
	PSSetPixel(display,foreground);
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
int32_t     ncoords;
int32_t     shape;
int32_t     mode;
{
int32_t	i;
int32_t	pcount;
int32_t	foreground;

    if(output_flag == XOUT){
	XFillPolygon(display,drawable,context,coord,ncoords,shape,mode);
    } else{
	if(ncoords <= 0){
	    return;
	}
	PSNewpath();
	foreground = PSGetForeground(display,context);
	PSSetPixel(display,foreground);
	PSMoveto(coord[0].x,coord[0].y);
	pcount = 0;
	for(i=1;i<ncoords;i++){
	    PSLineto(coord[i].x,coord[i].y);
	    /*
	    ** break it up into managable cuhnks
	    */
	    if(pcount > MAXSTROKES){
		PSClosefill();
		PSMoveto(coord[i].x,coord[i].y);
		pcount = 0;
	    }
	    pcount++;
	}
	PSClosefill();
    }
}

PSHeader(requested_scale,box,header,window_width,window_height)
float		requested_scale;
int32_t		box;
int32_t		header;
int32_t 	window_width;
int32_t 	window_height;
{
float	scale,scalex,scaley;
int32_t	clock;
char	headerstr[200];
char	*namestr;
char	*getenv();//,*strchr();
char	*ptr;
char	clockstr[100];
int32_t    tx,ty;

    /*
    ** calculate the scale factors in inches/screenpixel
    */
    scalex = PAGEWIDTH/window_width;
    scaley = PAGEHEIGHT/window_height;
    /*
    ** use the smaller scale factor so that everything will fit on the page
    */
    scale = requested_scale*((scalex < scaley) ? scalex : scaley);
    /*
    ** calculate the dots/screenpixel scale factor
    */
    pix_scale = scale*DPI;
    /*
    ** convert to postscriptpoints/screenpixel
    */
    page_scale = scale*DEFAULTRES;

    /*
    ** center it on the page
    */
    tx = DEFAULTRES*(PAGEWIDTH - scale*window_width)/2.0;
    ty = DEFAULTRES*(PAGEHEIGHT - scale*window_height)/2.0;

    /*
    ** add postscript comments
    */
    fprintf(PSfp,"%%!\n");
/*
    fprintf(PSfp,"initgraphics\n");
*/
    fprintf(PSfp,"/M { moveto } def\n");
    fprintf(PSfp,"/L { lineto } def\n");
    fprintf(PSfp,"/S { stroke } def\n");
    fprintf(PSfp,"/N { newpath } def\n");
    fprintf(PSfp,"/C { closepath fill } def\n");
    fprintf(PSfp,"/G { setgray } def\n");
    fprintf(PSfp,"/R { setrgbcolor } def\n");

    /*
    ** declare the bounding box comment for all to read
    */
    fprintf(PSfp,"%%%%BoundingBox: %d %d %d %d\n",
	tx,ty,
	tx + (int32_t)(page_scale*window_width),
	ty + (int32_t)(page_scale*window_height));
    fprintf(PSfp,"%%%%EndComments\n");

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
	clockstr, getcwd(NULL,200),namestr);
	PSNewpath();
	PSFont(12);
	fprintf(PSfp,"75 20 M\n");
	fprintf(PSfp,"%%HEADER\n");
	PSShow(headerstr);
	PSStroke();
    }

    fprintf(PSfp,"%d %d translate\n",tx,ty);
    fprintf(PSfp,"%f %f scale\n",page_scale,page_scale);

    if(box){
	/*
	** added current_gray assignment : 11/19/90  M.Wilson
	*/
	/*
	** draw the bounding box in half intensity gray
	*/
	current_gray = max_gray/2;
	fprintf(PSfp,"%f G\n",ComputeGray(current_gray));
	PSNewpath();
	PSMoveto(0,0);
	PSLineto(0,window_height);
	PSLineto(window_width,window_height);
	PSLineto(window_width,0);
	PSLineto(0,0);
	PSStroke();
    }
    
    PSNewpath();
    PSMoveto(0,0);
    PSLineto(0,window_height);
    PSLineto(window_width,window_height);
    PSLineto(window_width,0);
    PSLineto(0,0);
    fprintf(PSfp,"eoclip\n");
}

PSTrailer()
{
    fprintf(PSfp,"showpage\n");
}


PreparePS(display,window,scale,box,header,window_width,window_height)
Display	*display;
Window 	window;
float	scale;
int32_t	box;
int32_t	header;
int32_t	window_width;
int32_t	window_height;
{
char command[80];
XWindowAttributes	info;
char *printer;
char *pcom;

    current_gray = -1;
    current_line = -1;
    if(display){
	XGetWindowAttributes(display,window,&info);
	/*
	** get the color map used in the window
	*/
	if((window_colormap = info.colormap) == 0){
	    fprintf(stderr,"Using default colormap\n");
	    window_colormap = DefaultColormap(display,XDefaultScreen(display));
	}
    }
    if(window_width <= 0){
	window_width = 1;
    }
    if(window_height <= 0){
	window_height = 1;
    }
    if(!fontselected){
	PSSetFont(DEFAULTFONT);
    }

    /*
    window_width = info.width;
    window_height = info.height;
    */
    global_window_width = window_width;
    global_window_height = window_height;
    if(file_output){
	if((PSfp = fopen(ps_filename,ps_filemode)) == NULL){
	    fprintf(stderr,"unable to open '%s'\n",ps_filename);
	    return;
	}
    } else {
	if((printer = (char *)getenv("PRINTER")) == NULL){
	    printer = "lw";
	}
	if((pcom = (char *)getenv("PRINTCOMMAND")) == NULL){
	    sprintf(command,"lpr -h -P%s",printer);
	    pcom = command;
	}
	if((PSfp = popen(pcom,"w")) == NULL){
	    fprintf(stderr,"unable to execute '%s'\n",command);
	    return;
	}
    }
    PSHeader(scale,box,header,window_width,window_height);
    output_flag = PSOUT;
}

FinishPS()
{
    PSTrailer();
    output_flag = XOUT;
    if(file_output){
	if(PSfp)
	    fclose(PSfp);
    } else {
	if(PSfp)
	    pclose(PSfp);
    }
}

