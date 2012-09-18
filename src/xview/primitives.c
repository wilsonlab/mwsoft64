#include	"xview_ext.h"

SetColor(pixel)
int	pixel;
{
int	color;
    G->color = ColorMap(pixel);
    color = G->color;
    if(G->display){
	XSetForeground(G->display,G->context,(unsigned long)color);
	SetPSDefaultForeground(pixel);
    } else {
	SetPSDefaultForeground(pixel);
    }
}

SetForeground(pixel)
int	pixel;
{
int	color;
    G->foreground = pixel;
    color = ColorMap(pixel);
    if(G->display){
	XSetForeground(G->display,G->context,(unsigned long)color);
	SetPSDefaultForeground(pixel);
    } else {
	SetPSDefaultForeground(pixel);
    }
}

SetBackground(pixel)
int	pixel;
{
    G->background = pixel;
    if(G->display){
	XSetBackground(G->display,G->context,(unsigned long)ColorMap(pixel));
    }
}

SetPlaneMask(mask)
int	mask;
{
    XSetPlaneMask(G->display,G->context,(unsigned long)mask);
}

SetFont(name)
char *name;
{
    G->font = XLoadFont(G->display,name);
    if (G->font == BadName){
	fprintf(stderr,"unable to open text font");
	return(0);
    }
    G->fontinfo = XQueryFont(G->display,G->font);
    G->fontheight = G->fontinfo->ascent - G->fontinfo->descent;
    G->fontwidth = G->fontinfo->max_bounds.rbearing -
        G->fontinfo->min_bounds.lbearing;
    XSetFont(G->display,G->context,G->font);
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

TextExtent(s,height,width)
char 	*s;
int	*height;
int	*width;
{
int	ascent;
int	descent;
int	direction;
XCharStruct	overall;

    XTextExtents(G->fontinfo,s,strlen(s),&direction,&ascent,&descent,&overall);
    *height = overall.ascent - overall.descent;
    *width = overall.width;
}

Text(x,y,s)
int	x,y;
char	*s;
{
    if((PSStatus() == 0) && (G->mapped == 0)) { return;}
    XPSDrawText(G->display,G->drawable,G->context,x,y,s);
}


FilledBox(xl,yb,xr,yt)
int		xl,xr,yb,yt;
{
    if((PSStatus() == 0) && (G->mapped == 0)) { return;}
    XPSFillRectangle(G->display,G->drawable,G->context,xl,yb,xr-xl,yt-yb);
}

Box(xl,yb,xr,yt)
int		xl,xr,yb,yt;
{
    if((PSStatus() == 0) && (G->mapped == 0)) { return;}
    XPSDrawRectangle(G->display,G->drawable,G->context,xl,yb,xr-xl,yt-yb);
}

FilledPoly(coord,ncoords)
Coord	*coord;
int	ncoords;
{
    if((PSStatus() == 0) && (G->mapped == 0)) { return;}
    XPSFillPolygon(G->display,G->drawable,G->context,coord,ncoords,Convex,CoordModeOrigin);
}

DrawLine(x1,y1,x2,y2)
int		x1,y1,x2,y2;
{
    if((PSStatus() == 0) && (G->mapped == 0)) { return;}
    XPSDrawLine(G->display,G->drawable,G->context,x1,y1,x2,y2);
}

MultipleLines(coord,ncoords)
Coord	*coord;
int	ncoords;
{
    if((PSStatus() == 0) && (G->mapped == 0)) { return;}
    XPSDrawLines(G->display,G->drawable,G->context,coord,ncoords,CoordModeOrigin);
}

ClearWindow()
{
    if(G->display){
	SetColor(G->background);
	XClearWindow(G->display,G->imagewindow);
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

