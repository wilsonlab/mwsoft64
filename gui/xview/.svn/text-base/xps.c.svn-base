#include	"xview_ext.h"
static short file_output;
static char *ps_filename;
static int	max_gray;

char *CopyString(s)
char *s;
{
    if(s == NULL) return(NULL);
    return((char *)strcpy((char *)malloc(strlen(s)+1),s));
}

XPSDrawText(display,drawable,context,x,y,s)
Display *display;
Drawable drawable;
GC context;
int x,y;
char *s;
{
/*
XTextItem	text;

    text.chars = CopyString(s);
    text.nchars = strlen(s);
    text.font = G->font;
    XDrawText(display,drawable,context,x,y,&text,1); 
    */

    XDrawImageString(display,drawable,context,x,y,s,strlen(s)); 
}

XPSFillRectangle(display,drawable,context,x,y,w,h)
Display *display;
Drawable drawable;
GC context;
int	x,y,w,h;
{
    XFillRectangle(display,drawable,context,(int)x,(int)y,(unsigned int)w,(unsigned int)h);
}

XPSDrawRectangle(display,drawable,context,x,y,w,h)
Display *display;
Drawable drawable;
GC context;
int	x,y,w,h;
{
    XDrawRectangle(display,drawable,context,(int)x,(int)y,(unsigned int)w,(unsigned int)h);
}

XPSDrawLine(display,drawable,context,x,y,x2,y2)
Display *display;
Drawable drawable;
GC context;
int	x,y,x2,y2;
{
    XDrawLine(display,drawable,context,(int)x,(int)y,(int)x2,(int)y2);
}

XPSDrawLines(display,drawable,context,coord,ncoords,mode)
Display *display;
Drawable drawable;
GC context;
Coord	*coord;
int	ncoords;
int	mode;
{
    XDrawLines(display,drawable,context,coord,ncoords,mode);
}

XPSFillPolygon(display,drawable,context,coord,ncoords,shape,mode)
Display *display;
Drawable drawable;
GC context;
Coord	*coord;
int	ncoords;
int	shape;
int	mode;
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
int state;
{
    file_output = state;
}

SetMaxGray(val)
int	val;
{
    max_gray = val;
}
