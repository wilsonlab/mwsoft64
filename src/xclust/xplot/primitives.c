#include	"xplot_ext.h"

#define MAXBITMAPS	100

struct bitmaplist_type {
    char	*name;
    Pixmap	bitmap;
    short	w,h;
} bitmaplist[MAXBITMAPS];

int	nbitmaps = 0;

AddBitmap(basic,name,bits,w,h)
BasicWindow	*basic;
char	*name;
char	*bits;
int	w,h;
{
Pixmap	bitmap;

    bitmap = XCreatePixmapFromBitmapData(basic->display,basic->window, bits,
    w,h,basic->foreground,basic->background,
    DefaultDepthOfScreen(DefaultScreenOfDisplay(basic->display)));

    if(nbitmaps < MAXBITMAPS){
	bitmaplist[nbitmaps].name = CopyString(name);
	/*
    fprintf(stderr,"adding %dx%d bitmap %s\n",w,h,bitmaplist[nbitmaps].name);
    */
	bitmaplist[nbitmaps].bitmap = bitmap;
	bitmaplist[nbitmaps].w = w;
	bitmaplist[nbitmaps].h = h;
	nbitmaps++;
    }

}

Pixmap GetBitmap(id,w,h)
int	id;
short	*w;
short	*h;
{
    if(id < nbitmaps){
	*w = bitmaplist[id].w;
	*h = bitmaplist[id].h;
	return(bitmaplist[id].bitmap);
    }
    return((Pixmap)NULL);
}

Pixmap GetPixmapByName(name)
char	*name;
{
int	id;
    for(id=0;id<nbitmaps;id++){
	if(strcmp(bitmaplist[id].name,name) == 0){
	    return(bitmaplist[id].bitmap);
	}
    }
    return((Pixmap)NULL);
}

Beep()
{
    fprintf(stderr,"\007");
}

DrawSuperLine(basic,sx1,sy1,sx2,sy2,color)
BasicWindow *basic;
int sx1,sy1,sx2,sy2;
unsigned long	color;
{
	SetColor(color^basic->background);
	/*
	SetColor(basic->foreground^basic->background);
	*/
	DrawMode(basic,GXxor);
	DrawLine(sx1,sy1,sx2,sy2);
	DrawMode(basic,GXcopy);
}

EraseSuperLine(basic,sx1,sy1,sx2,sy2,color)
BasicWindow *basic;
int sx1,sy1,sx2,sy2;
unsigned long	color;
{
	SetColor(color^basic->background);
	/*
	SetColor(basic->foreground^basic->background);
	*/
	DrawMode(basic,GXxor);
	DrawLine(sx1,sy1,sx2,sy2);
	DrawMode(basic,GXcopy);
}

DrawSuperBox(basic,sx1,sy1,sx2,sy2,color)
BasicWindow *basic;
int sx1,sy1,sx2,sy2;
unsigned long	color;
{
    /*
    SetColor(basic->foreground^basic->background);
    */
    SetColor(color^basic->background);
    DrawMode(basic,GXxor);
    Box(sx1,sy1,sx2,sy2);
    DrawMode(basic,GXcopy);
}

EraseSuperBox(basic,sx1,sy1,sx2,sy2,color)
BasicWindow *basic;
int sx1,sy1,sx2,sy2;
unsigned long	color;
{
    /*
    SetColor(basic->foreground^basic->background);
    */
    SetColor(color^basic->background);
    DrawMode(basic,GXxor);
    Box(sx1,sy1,sx2,sy2);
    DrawMode(basic,GXcopy);
}

ZapExpose(display)
Display *display;
{
XEvent	e;
    while(XCheckMaskEvent(display,ExposureMask,&e))
	;
}

DrawBitmap(basic,bitmap,x,y,w,h)
BasicWindow *basic;
Pixmap	bitmap;
int	x,y;
{
    XCopyArea(basic->display,bitmap,basic->window,
    basic->context, 0,0,w,h,x,y);

}

DrawMode(basic,mode)
BasicWindow *basic;
int mode;
{
    XSetFunction(basic->display,basic->context,mode);
}

/*
char *CopyString(s)
char *s;
{
char *tmp;

    if(s == NULL){
	return(NULL);
    }
    tmp = (char *)malloc(strlen(s) + 1);
    strcpy(tmp,s);
    return(tmp);
}
*/

SetLinestyle(basic,style)
BasicWindow *basic;
int	style;
{
XGCValues	values;

    if(style == 0){
	values.line_style = LineSolid;
	XChangeGC(basic->display,basic->context,GCLineStyle,&values);
    } else {
	values.line_style = LineDoubleDash;
	XChangeGC(basic->display,basic->context,GCLineStyle,&values);
    } 
}

_SetColor(basic,pixel)
BasicWindow *basic;
int	pixel;
{
    basic->color = ColorMap(pixel);
    XSetForeground(basic->display,basic->context,(unsigned long)basic->color);
}

Invert(basic)
BasicWindow *basic;
{
    if(basic->display){
	XSetForeground(basic->display,basic->context,ColorMap(basic->background));
	XSetBackground(basic->display,basic->context,ColorMap(basic->foreground));
    }
}

Normal(basic)
BasicWindow *basic;
{
    if(basic->display){
	XSetForeground(basic->display,basic->context,ColorMap(basic->foreground));
	XSetBackground(basic->display,basic->context,ColorMap(basic->background));
    }
}

SetColor(pixel)
int	pixel;
{
    G->color = ColorMap(pixel);
    if(G->display){
	XSetForeground(G->display,G->context,(unsigned long)G->color);
	SetPSDefaultForeground(pixel);
    } else {
	SetPSDefaultForeground(pixel);
    }
}

SetForeground(basic,pixel)
BasicWindow *basic;
int	pixel;
{
    basic->foreground = pixel;
    if(G->display){
	XSetForeground(G->display,G->context,(unsigned long)ColorMap(pixel));
	SetPSDefaultForeground(pixel);
    } else {
	SetPSDefaultForeground(pixel);
    }
}


#ifdef OLD
SetColor(pixel)
int	pixel;
{
    G->color = pixel;
    if(G->display){
	XSetForeground(G->display,G->context,(unsigned long)pixel);
    }
}

SetForeground(basic,pixel)
BasicWindow *basic;
int	pixel;
{
    basic->foreground = pixel;
    if(basic->display){
	XSetForeground(basic->display,basic->context,(unsigned long)pixel);
    }
}
#endif

SetBackground(basic,pixel)
BasicWindow *basic;
int	pixel;
{
    basic->background = pixel;
    if(basic->display){
	XSetBackground(basic->display,basic->context,(unsigned long)ColorMap(pixel));
    }
}

SetPlaneMask(mask)
int	mask;
{
    XSetPlaneMask(G->display,G->context,(unsigned long)mask);
}

SetFont(frame,name)
BasicWindow *frame;
char *name;
{
    frame->font = XLoadFont(frame->display,name);
    if (frame->font == 0){
	fprintf(stderr,"unable to open text font");
	return(0);
    }
    frame->fontinfo = XQueryFont(frame->display,frame->font);
    frame->fontheight = frame->fontinfo->ascent + frame->fontinfo->descent;
    frame->fontwidth = frame->fontinfo->max_bounds.rbearing -
        frame->fontinfo->min_bounds.lbearing;
    XSetFont(frame->display,frame->context,frame->font);
    ManualSetFontInfo(frame->fontheight,frame->fontwidth,frame->foreground);
    return(1);
}

SetLineWidth(width)
int width;
{
XGCValues	values;

    G->linewidth =  width;
    if(G->display){
	values.line_width = width;
	XChangeGC(G->display,G->context,GCLineWidth,&values);
    }
}

GContext ContextID()
{
    /*
    ** get the resource ID for the graphics context
    */
    XGContextFromGC(G->context);
}

TextExtent(s,width,height)
char 	*s;
int	*width;
int	*height;
{
int	ascent;
int	descent;
int	direction;
XCharStruct	overall;

    if(s == NULL){
	*height = 0;
	*width = 0;
	return;
    }
    if(!F->mapped){
	*height = 9;
	*width = 6;
	return;
    }
    XTextExtents(G->fontinfo,s,strlen(s),&direction,&ascent,&descent,&overall);
    *height = overall.ascent + overall.descent;
    *height = G->fontheight;
    *width = overall.width;
}

_TextExtent(basic,s,width,height)
BasicWindow	*basic;
char 	*s;
int	*width;
int	*height;
{
int	ascent;
int	descent;
int	direction;
XCharStruct	overall;

    if(!basic->mapped){
	*height = 9;
	*width = 6;
	return;
    }
    if((basic == NULL) || (s == NULL)){
	*height = 0;
	*width = 0;
	return;
    }
    XTextExtents(basic->fontinfo,s,strlen(s),&direction,&ascent,&descent,&overall);
    *height = overall.ascent + overall.descent;
    *height = basic->fontheight;
    *width = overall.width;
}


Text(basic,x,y,s)
BasicWindow	*basic;
int	x,y;
char	*s;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawText(basic->display,basic->drawable,basic->context,x,y,s);
}

_FilledBox(basic,xl,yb,xr,yt)
BasicWindow	*basic;
int		xl,xr,yb,yt;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    if(yb > yt){
	XPSFillRectangle(basic->display,basic->drawable,basic->context,xl,yt,xr-xl,yb-yt);
    } else {
	XPSFillRectangle(basic->display,basic->drawable,basic->context,xl,yb,xr-xl,yt-yb);
    }
}

FilledBox(xl,yb,xr,yt)
int		xl,xr,yb,yt;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    if(yb > yt){
	XPSFillRectangle(G->display,G->drawable,G->context,xl,yt,xr-xl,yb-yt);
    } else {
	XPSFillRectangle(G->display,G->drawable,G->context,xl,yb,xr-xl,yt-yb);
    }
}

_Box(basic,xl,yb,xr,yt)
BasicWindow	*basic;
int		xl,xr,yb,yt;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawRectangle(basic->display,basic->drawable,basic->context,xl,yb,xr-xl,yt-yb);
}

Box(xl,yb,xr,yt)
int		xl,xr,yb,yt;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawRectangle(G->display,G->drawable,G->context,xl,yb,xr-xl,yt-yb);
}

FilledPoly(coord,ncoords)
Coord	*coord;
int	ncoords;
{
    XPSFillPolygon(G->display,G->drawable,G->context,coord,ncoords,Convex,CoordModeOrigin);
}

_DrawPoint(b,x1,y1)
BasicWindow	*b;
int		x1,y1;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawPoint(b->display,b->drawable,b->context,x1,y1);
}

_DrawLine(b,x1,y1,x2,y2)
BasicWindow	*b;
int		x1,y1,x2,y2;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawLine(b->display,b->drawable,b->context,x1,y1,x2,y2);
}

DrawLine(x1,y1,x2,y2)
int		x1,y1,x2,y2;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawLine(G->display,G->drawable,G->context,x1,y1,x2,y2);
}

MultipleLines(coord,ncoords)
Coord	*coord;
int	ncoords;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawLines(G->display,G->drawable,G->context,coord,ncoords,CoordModeOrigin);
}

FloatMultipleLines(coord,ncoords,fcoord)
Coord	*coord;
int	ncoords;
FCoord	*fcoord;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawLinesFloat(G->display,G->drawable,G->context,coord,ncoords,CoordModeOrigin,fcoord);
}

ClearWindow(basic)
BasicWindow	*basic;
{
    if(basic->display){
	XClearWindow(basic->display,basic->window);
    }
}

Bell(vol)
int	vol;
{
    XBell(G->display,0);
}

DebugX(s)
char *s;
{
XEvent	E;
    fprintf(stderr,"%s\n",s);
    XNextEvent(G->display,&E);
    fprintf(stderr,"OK\n");
}

Circle(cx,cy,d)
int	cx,cy,d;
{
    /*if(PSStatus() == 0 && F->mapped == 0) return;*/
    if(G->display){
	XPSDrawArc(G->display,G->drawable,G->context,cx-d/2,cy-d/2,d,d,0,360*64);
    }
}

FilledCircle(cx,cy,d)
int	cx,cy,d;
{
    /*if(PSStatus() == 0 && F->mapped == 0) return;*/
    if(G->display){
	XPSFillArc(G->display,G->drawable,G->context,cx-d/2,cy-d/2,d,d,0,360*64);
    }
}

Triangle(cx,cy,d)
int	cx,cy,d;
{
Coord	coord[4];
int	ncoords = 4;

    coord[0].x = cx - d/2;
    coord[0].y = cy + d/2;
    coord[1].x = cx;
    coord[1].y = cy - d/2;
    coord[2].x = cx + d/2;
    coord[2].y = cy + d/2;
    coord[3].x = coord[0].x;
    coord[3].y = coord[0].y;

    XPSDrawLines(G->display,G->drawable,G->context,coord,ncoords,CoordModeOrigin);
}

FilledTriangle(cx,cy,d)
int	cx,cy,d;
{
Coord	coord[4];
int	ncoords = 4;

    coord[0].x = cx - d/2;
    coord[0].y = cy + d/2;
    coord[1].x = cx;
    coord[1].y = cy - d/2;
    coord[2].x = cx + d/2;
    coord[2].y = cy + d/2;
    coord[3].x = coord[0].x;
    coord[3].y = coord[0].y;

    XPSFillPolygon(G->display,G->drawable,G->context,coord,ncoords,Convex,CoordModeOrigin);
}

