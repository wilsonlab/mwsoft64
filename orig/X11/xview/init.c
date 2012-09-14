#include	"xview_ext.h"
#include 	<X11/cursorfont.h>
#include 	<X11/Xutil.h>

static int typeadjust = 0;
extern Colormap	cmap;

adjusttype(x)
int x;
{
typeadjust = x;
}

FileInit()
{
int ival;
int	headersize;
char	*tstring;
int	haveformat;
FieldInfo	fieldinfo;
char	**header;

    zbase_size = ybase_size *8;

    header = ReadHeader(fp,&headersize);
    /*
    ** check for formatting information in the header
    */
    haveformat = 1;
    if((tstring = GetHeaderParameter(header,"File type:")) != NULL){
        /*
        ** get the file type
        */
	if(strncmp("Xview",tstring,5) != 0){
	    haveformat = 0;
	} 
    } else {
	haveformat = 0;
    }
    if((tstring = GetHeaderParameter(header,"Xsize:")) != NULL){
        /*
        ** get the bounds
        */
        sscanf(tstring,"%d",&ival);
	if(V->xmax < 0){
	    V->xmax = ival-1;
	}
    } else {
	haveformat = 0;
    }
    if((tstring = GetHeaderParameter(header,"Ysize:")) != NULL){
        /*
        ** get the bounds
        */
        sscanf(tstring,"%d",&ival);
	if(V->ymax < 0){
	    V->ymax = ival-1;
	}
    } else {
	haveformat = 0;
    }
    if((tstring = GetFieldString(header)) != NULL){
        /*
        ** get the field info
        */
	GetFieldInfoByNumber(tstring,0,&fieldinfo);
	V->datatype = fieldinfo.type;
    } else {
	haveformat = 0;
    }


    /*
    ** if so then ignore the imbedded stuff
    */
    if(haveformat) {
	V->dt = 1;
	V->headersize = 0;
	V->plain = 1;
    }
    if(V->posdata) {
	V->dt = 1;
	V->headersize = 0;
	V->plain = 1;
	V->xmax = 364;
	V->ymax = 255;
    }
    if(!V->plain){
	fread (&ival, sizeof (int),1,fp);
	if(V->xmax < 0) V->xmax = ival;
	fread (&ival, sizeof (int),1,fp);
	if(V->ymax < 0) V->ymax = ival;
	fread (&V->dt, sizeof (float),1,fp);
	fread (&V->datatype, sizeof (int),1,fp);
	V->headersize = 3*sizeof(int) + sizeof(float);
    }
    V->headersize += headersize;

    V->view_time = V->start_time = 0;
    V->view_step = V->dt;

    V->datatype += typeadjust;
    V->cellnum = (V->xmax+1)*(V->ymax+1);

    switch(V->datatype){
	case CHAR : V->datasize = sizeof(char); break;
	case SHORT : V->datasize = sizeof(short); break;
	case INT : V->datasize = sizeof(int); break;
	case FLOAT : V->datasize = sizeof(float); break;
	case DOUBLE : V->datasize = sizeof(double); break;
    }

    data = (float *) calloc (V->cellnum , sizeof(float));
    curdata = (float *) calloc (V->cellnum , sizeof(float));
    tmpdata = (float *) calloc (V->cellnum , sizeof(float));
    image = (struct image_type *) calloc (V->cellnum , sizeof (struct image_type));
	if(!no_lines){
	}

    if (autoscale) do_autoscale(fp);

}

InitX(display_name)
char	*display_name;
{
    /*
    ** open the display
    */
    G->display = XOpenDisplay(display_name);
    if (G->display == NULL) {
	fprintf(stderr,"Unable to open X display [%s]\n", display_name);
	exit(1);
    }
    if(debug){
	/*
	** force all calls to complete before returning
	** this synchronizes errors with the routines that invoked them
	*/
	XSynchronize(G->display,1);
    }

    /*
    ** use the default screen
    */
    G->screen_number = XDefaultScreen(G->display);

    /*
    ** use the default visual
    */
    G->visual = XDefaultVisual(G->display,G->screen_number);

    /*
    ** use the default context
    */
    G->context = XDefaultGC(G->display,G->screen_number);

    /*
    ** determine whether or not color can be displayed
    */
    if (XDisplayPlanes(G->display,G->screen_number) < 2){
	color_mode = 0;
    }
    /*
    ** create the colormap
    */
    if(color_mode){
	MakeColormap();
    }

    /*
    ** select the font 
    */
    SetFont("6x13");

    /*
    ** select the cursor
    */
    XCreateFontCursor(G->display,XC_left_ptr);

    /*
    ** create the colormap
    if(color_mode){
	MakeColormap();
    }
    */

    /*
    ** get the foreground and background
    */
    if(G->inverse){
    /*
	SetBackground(XBlackPixel(G->display,G->screen_number));
	SetForeground(XWhitePixel(G->display,G->screen_number));
	*/
	SetBackground(BLACK);
	SetForeground(WHITE);
    } else {
    /*
	SetBackground(XWhitePixel(G->display,G->screen_number));
	SetForeground(XBlackPixel(G->display,G->screen_number));
	*/
	SetBackground(WHITE);
	SetForeground(BLACK);
    }
    SetPSInverse(1);

    /*
    ** create the windows
    */
    CreateWindows();

    /*
    ** use the window as the drawable
    */
    G->drawable = G->imagewindow;


    XSetWindowColormap(G->display,G->drawable,cmap);

    /*
    ** make the window visible
    */
    if(G->mapped){
	XMapWindow(G->display,G->imagewindow);
    }
    /*
    ** make sure it all got to the server
    */
    XFlush(G->display);
}

CreateWindows()
{
int	width;
int	height;
int	x;
int	y;
unsigned long 		border;
unsigned long 		background;
XWindowAttributes	winfo;
Window			root_window;
int			borderwidth;
XSizeHints		hints;
XSetWindowAttributes	attrib;
int			status;

    /*
    ** get the root window and its attributes
    */
    root_window = XRootWindow(G->display,G->screen_number);
    XGetWindowAttributes(G->display,root_window,&winfo);
    status = 0;
    if(geometry){
	status = XParseGeometry(geometry,&x,&y,&width,&height);
    } else {
	x = winfo.width / 10;
	y = winfo.height / 10;
	if(V->ymax > V->xmax){
	    width = ((float)(V->xmax + 1)/(V->ymax + 1))*winfo.height /3.0;
	    height = winfo.height/3.0 + 5*G->fontheight;
	} else {
	    height = ((float)(V->ymax + 1)/(V->xmax + 1))*winfo.height /3.0
		+ 5*G->fontheight;
	    width = winfo.height/3.0;
	}
    }

    borderwidth = 1;
    border = G->foreground;
    background = G->background;

    /*
    ** create the image window
    */
    G->imagewindow = (Window)XCreateSimpleWindow(
	G->display,root_window,x,y,width,height,borderwidth,border,ColorMap(background));

    attrib.backing_store = WhenMapped;
    XChangeWindowAttributes(G->display,G->imagewindow,CWBackingStore,&attrib);

    /*
    ** request that the window be notified of certain events
    */
    XSelectInput(G->display,G->imagewindow, 
	KeyPressMask | ExposureMask | ButtonPressMask) ;

    /*
    ** set the standard properties of the window
    */
    hints.flags = 0L;
    if(status & XValue){
	hints.flags |= PPosition;
	hints.flags |= USPosition;
	hints.x = x;
    }
    if(status & YValue){
        hints.flags |= PPosition;
	hints.flags |= USPosition;
        hints.y = y;
    }
    if(status & WidthValue){
        hints.flags |= PSize;
	hints.flags |= USSize;
        hints.width = width;
    }
    if(status & HeightValue){
        hints.flags |= PSize;
	hints.flags |= USSize;
        hints.height = height;
    }


    XSetStandardProperties(G->display,G->imagewindow,
    V->filename,	/* window name */
    V->filename,	/* icon name */
    None,		/* icon pixmap */
    NULL,		/* command argv */
    0,			/* command argc */
    &hints		/* window sizing hints */
    );


}

Quit()
{
    if(G->display){
	XDestroyWindow(G->display,G->imagewindow);
	XCloseDisplay(G->display);
    }
    exit(0);
}
