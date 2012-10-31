#include	"xclust_ext.h"
extern ColorTable idx[];

#define MAXBITMAPS	100

static Font *theFont = NULL;
static XFontStruct *theFontStruct = NULL;

struct bitmaplist_type {
    char	*name;
    Pixmap	bitmap;
    int16_t	w,h;
} bitmaplist[MAXBITMAPS];

int32_t	nbitmaps = 0;

int32_t WhitePixelIdx()
{
    return(WHITE);
}

int32_t BlackPixelIdx()
{
    return(BLACK);
}

Pixmap CreateBitmap(basic,bits,w,h,fg,bg)
BasicWindow	*basic;
char	*bits;
int32_t	w,h;
int32_t	fg,bg;
{
Pixmap	bitmap;

    /* a foreground or background argument of -1 means use the default */
    if(fg < 0)
      fg = GetPixel(basic->foreground);
    else
      fg = GetPixel(fg);
    
    if(bg < 0)
      bg = GetPixel(basic->background);
    else
      bg = GetPixel(bg);
    
    bitmap = XCreatePixmapFromBitmapData(basic->display,basic->window, bits,
    w,h,fg,bg,
    DefaultDepthOfScreen(DefaultScreenOfDisplay(basic->display)));

    return bitmap;
}


AddBitmap(basic,name,bits,w,h,fg,bg)
BasicWindow	*basic;
char	*name;
char	*bits;
int32_t	w,h;
int32_t	fg,bg;
{
Pixmap	bitmap;

 bitmap = CreateBitmap(basic,bits,w,h,fg,bg);
 
 if(nbitmaps < MAXBITMAPS){
   bitmaplist[nbitmaps].name = CopyString(name);
   bitmaplist[nbitmaps].bitmap = bitmap;
   bitmaplist[nbitmaps].w = w;
   bitmaplist[nbitmaps].h = h;
   nbitmaps++;
 }
}


Pixmap GetBitmap(id,w,h)
int32_t	id;
int16_t	*w;
int16_t	*h;
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
int32_t	id;
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
int32_t sx1,sy1,sx2,sy2;
uint32_t	color;
{
	/*
	SetColor(color^basic->background);
	SetColor(basic->foreground^basic->background);
	*/
    XSetForeground(basic->display,basic->context,(GetPixel(color)^GetPixel(basic->background)));
	DrawMode(basic,GXxor);
	DrawLine(sx1,sy1,sx2,sy2);
	DrawMode(basic,GXcopy);
}

EraseSuperLine(basic,sx1,sy1,sx2,sy2,color)
BasicWindow *basic;
int32_t sx1,sy1,sx2,sy2;
uint32_t	color;
{
	SetColor(color^basic->background);
	/*
	SetColor(basic->foreground^basic->background);
	*/
    XSetForeground(basic->display,basic->context,(GetPixel(color)^GetPixel(basic->background)));
	DrawMode(basic,GXxor);
	DrawLine(sx1,sy1,sx2,sy2);
	DrawMode(basic,GXcopy);
}

DrawSuperBox(basic,sx1,sy1,sx2,sy2,color)
BasicWindow *basic;
int32_t sx1,sy1,sx2,sy2;
uint32_t	color;
{
    /*
    SetColor(basic->foreground^basic->background);
    SetColor(color^basic->background);
    */
    XSetForeground(basic->display,basic->context,(GetPixel(color)^GetPixel(basic->background)));
    DrawMode(basic,GXxor);
    Box(sx1,sy1,sx2,sy2);
    DrawMode(basic,GXcopy);
}

EraseSuperBox(basic,sx1,sy1,sx2,sy2,color)
BasicWindow *basic;
int32_t sx1,sy1,sx2,sy2;
uint32_t	color;
{
    /*
    SetColor(basic->foreground^basic->background);
    SetColor(color^basic->background);
    */
    XSetForeground(basic->display,basic->context,(GetPixel(color)^GetPixel(basic->background)));
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
int32_t	x,y;
{
    XCopyArea(basic->display,bitmap,basic->window,
    basic->context, 0,0,w,h,x,y);

}

DrawMode(basic,mode)
BasicWindow *basic;
int32_t mode;
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
int32_t	style;
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

_SetColor(basic,arrayid)
BasicWindow *basic;
int32_t	arrayid;
{
int32_t	pixel;
    basic->color = arrayid;
    pixel = idx[arrayid].pixel;
    XSetForeground(basic->display,basic->context,(uint32_t)pixel);
}

Invert(basic)
BasicWindow *basic;
{
    if(basic->display){
	XSetForeground(basic->display,basic->context,idx[basic->background].pixel);
	XSetBackground(basic->display,basic->context,idx[basic->foreground].pixel);
    }
}

Normal(basic)
BasicWindow *basic;
{
    if(basic->display){
	XSetForeground(basic->display,basic->context,idx[basic->foreground].pixel);
	XSetBackground(basic->display,basic->context,idx[basic->background].pixel);
    }
}

SetColor(arrayid)
int32_t	arrayid;
{
int32_t	pixel;
    G->color = arrayid;
    pixel = idx[arrayid].pixel;
    if(G->display){
	XSetForeground(G->display,G->context,(uint32_t)pixel);
	SetPSDefaultForeground(pixel);
    } else {
	SetPSDefaultForeground(pixel);
    }
}

SetForeground(basic,arrayid)
BasicWindow *basic;
int32_t	arrayid;
{
int32_t	pixel;
    basic->foreground = arrayid;
    pixel = idx[arrayid].pixel;
    if(G->display){
	XSetForeground(G->display,G->context,(uint32_t)pixel);
	SetPSDefaultForeground(pixel);
    } else {
	SetPSDefaultForeground(pixel);
    }
}


#ifdef OLD
SetColor(pixel)
int32_t	pixel;
{
    G->color = pixel;
    if(G->display){
	XSetForeground(G->display,G->context,(uint32_t)pixel);
    }
}

SetForeground(basic,arrayid)
BasicWindow *basic;
int32_t	arrayid;
{
int32_t	pixel;

    basic->foreground = arrayid;
    pixel = idx[arrayid].pixel;
    if(basic->display){
	XSetForeground(basic->display,basic->context,(uint32_t)pixel);
    }
}
#endif

SetBackground(basic,arrayid)
BasicWindow *basic;
int32_t	arrayid;
{
int32_t	pixel;
    basic->background = arrayid;
    pixel = idx[arrayid].pixel;
    if(basic->display){
	XSetBackground(basic->display,basic->context,(uint32_t)pixel);
    }
}

SetPlaneMask(mask)
int32_t	mask;
{
    XSetPlaneMask(G->display,G->context,(uint32_t)mask);
}

SetFont(basic,name)
BasicWindow *basic;
char *name;
{
    fprintf(stderr, "Setting font to:%s\n", name);

    if (theFontStruct == NULL)
    {
        fprintf(stderr, "Font is NULL! Loading it now... ");

        theFontStruct = XLoadQueryFont(basic->display, name);

        theFont = theFontStruct->fid;
        
        if (theFont == BadName){
    	   fprintf(stderr,"unable to open text font:BadName\n");
    	   return(0);
        }
        if (theFont == BadAlloc){
           fprintf(stderr,"unable to open text font:BadAlloc\n");
           return(0);
        }
        if (theFont == BadFont){
            fprintf(stderr,"unable to open text font:BadFont\n");
            return (0);
        }

        fprintf(stderr," DONE!\n");
    }
    
    basic->font = theFont;
    basic->fontinfo = theFontStruct;
    
    basic->fontheight = basic->fontinfo->ascent + basic->fontinfo->descent;
    basic->fontwidth = basic->fontinfo->max_bounds.rbearing -
    basic->fontinfo->min_bounds.lbearing;

    XSetFont(basic->display,basic->context,basic->font);
    ManualSetFontInfo(basic->fontheight,basic->fontwidth,GetPixel(basic->foreground));
    
    return(1);
}

SetLineWidth(width)
int32_t width;
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
int32_t	*width;
int32_t	*height;
{
int32_t	ascent;
int32_t	descent;
int32_t	direction;
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
int32_t	*width;
int32_t	*height;
{
int32_t	ascent;
int32_t	descent;
int32_t	direction;
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
int32_t	x,y;
char	*s;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawText(basic->display,basic->drawable,basic->context,x,y,s);
}

_FilledBox(basic,xl,yb,xr,yt)
BasicWindow	*basic;
int32_t		xl,xr,yb,yt;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    if(yb > yt){
	XPSFillRectangle(basic->display,basic->drawable,basic->context,xl,yt,xr-xl,yb-yt);
    } else {
	XPSFillRectangle(basic->display,basic->drawable,basic->context,xl,yb,xr-xl,yt-yb);
    }
}

FilledBox(xl,yb,xr,yt)
int32_t		xl,xr,yb,yt;
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
int32_t		xl,xr,yb,yt;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawRectangle(basic->display,basic->drawable,basic->context,xl,yb,xr-xl,yt-yb);
}

Box(xl,yb,xr,yt)
int32_t		xl,xr,yb,yt;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawRectangle(G->display,G->drawable,G->context,xl,yb,xr-xl,yt-yb);
}

FilledPoly(coord,ncoords)
Coord	*coord;
int32_t	ncoords;
{
    XPSFillPolygon(G->display,G->drawable,G->context,coord,ncoords,Convex,CoordModeOrigin);
}

_DrawPoint(b,x1,y1)
BasicWindow	*b;
int32_t		x1,y1;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawPoint(b->display,b->drawable,b->context,x1,y1);
}

_DrawLine(b,x1,y1,x2,y2)
BasicWindow	*b;
int32_t		x1,y1,x2,y2;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawLine(b->display,b->drawable,b->context,x1,y1,x2,y2);
}

DrawLine(x1,y1,x2,y2)
int32_t		x1,y1,x2,y2;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawLine(G->display,G->drawable,G->context,x1,y1,x2,y2);
}

MultipleLines(coord,ncoords)
Coord	*coord;
int32_t	ncoords;
{
    if((PSStatus() == 0) && (F->mapped == 0)) { return;}
    XPSDrawLines(G->display,G->drawable,G->context,coord,ncoords,CoordModeOrigin);
}

FloatMultipleLines(coord,ncoords,fcoord)
Coord	*coord;
int32_t	ncoords;
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
int32_t	vol;
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
int32_t	cx,cy,d;
{
    /*if(PSStatus() == 0 && F->mapped == 0) return;*/
    if(G->display){
	XPSDrawArc(G->display,G->drawable,G->context,cx-d/2,cy-d/2,d,d,0,360*64);
    }
}

FilledCircle(cx,cy,d)
int32_t	cx,cy,d;
{
    /*if(PSStatus() == 0 && F->mapped == 0) return;*/
    if(G->display){
	XPSFillArc(G->display,G->drawable,G->context,cx-d/2,cy-d/2,d,d,0,360*64);
    }
}

Triangle(cx,cy,d)
int32_t	cx,cy,d;
{
Coord	coord[4];
int32_t	ncoords = 4;

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
int32_t	cx,cy,d;
{
Coord	coord[4];
int32_t	ncoords = 4;

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

