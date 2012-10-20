#include	"xplot_ext.h"
static int16_t file_output;
static char *ps_filename;
static int32_t	max_gray;

XPSDrawText(display,drawable,context,x,y,s)
Display *display;
Drawable drawable;
GC context;
int32_t x,y;
char *s;
{
    XDrawImageString(display,drawable,context,x,y,s,strlen(s));
}

XPSFillRectangle(display,drawable,context,x,y,w,h)
Display *display;
Drawable drawable;
GC context;
int32_t	x,y,w,h;
{
    XFillRectangle(display,drawable,context,(int32_t)x,(int32_t)y,(uint32_t)w,(uint32_t)h);
}

XPSDrawRectangle(display,drawable,context,x,y,w,h)
Display *display;
Drawable drawable;
GC context;
int32_t	x,y,w,h;
{
    XDrawRectangle(display,drawable,context,(int32_t)x,(int32_t)y,(uint32_t)w,(uint32_t)h);
}

XPSDrawLine(display,drawable,context,x,y,x2,y2)
Display *display;
Drawable drawable;
GC context;
int32_t	x,y,x2,y2;
{
    XDrawLine(display,drawable,context,(int32_t)x,(int32_t)y,(int32_t)x2,(int32_t)y2);
}

XPSDrawLines(display,drawable,context,coord,ncoords,mode)
Display *display;
Drawable drawable;
GC context;
Coord	*coord;
int32_t	ncoords;
int32_t	mode;
{
    XDrawLines(display,drawable,context,coord,ncoords,mode);
}

XPSFillPolygon(display,drawable,context,coord,ncoords,shape,mode)
Display *display;
Drawable drawable;
GC context;
Coord	*coord;
int32_t	ncoords;
int32_t	shape;
int32_t	mode;
{
    XFillPolygon(display,drawable,context,coord,ncoords,shape,mode);
}

PreparePS()
{
}

FinishPS()
{
}

SetPSFilename(name)
char *name;
{
    ps_filename = name;
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
