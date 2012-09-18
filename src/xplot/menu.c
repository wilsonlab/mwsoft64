#include "xplot_ext.h"
#include "button.bitmap"
#include "rarrow.bitmap"
#include "larrow.bitmap"
#include "zap.bitmap"
#include "toggle2.bitmap"
#include "button2.bitmap"
#include "circle.bitmap"
#include "fcircle.bitmap"

#include <sys/time.h>


#define BM_BUTTON	0
#define BM_RARROW	1
#define BM_LARROW	2
#define BM_ZAP		3
#define BM_TOGGLE2	4
#define BM_BUTTON2	5

LoadBitmaps(basic)
BasicWindow	*basic;
{
    AddBitmap(basic,"button",button_bits,16,16);
    AddBitmap(basic,"button",button2_bits,16,16);
    AddBitmap(basic,"zap",zap_bits,16,16);
    AddBitmap(basic,"circle",circle_bits,5,5);
    AddBitmap(basic,"fcircle",fcircle_bits,5,5);
}

RefreshMenu(menu)
MenuWindow	*menu;
{
    ClearWindow(menu);
    DrawMenu(menu); 
}

Pixmap CreateItemPixmap(item,bits)
MenuItem	*item;
char		*bits;
{
int	depth = 1;

    if(bits){
	return(
	XCreatePixmapFromBitmapData(item->display,item->window, bits,
	16,16,item->foreground,item->background,
	DefaultDepthOfScreen(DefaultScreenOfDisplay(item->display)))
	);
    } else {
	return(0);
    }
}


DrawItemBitmap(item,bitmap)
MenuItem	*item;
Pixmap		bitmap;
{
/*
    XSetWindowBackgroundPixmap(item->display,item->window,bitmap);
*/
    XCopyArea(item->display,bitmap,item->window,
    item->context, 0,0,16,16,0,0);
}


DrawItem(item)
MenuItem	*item;
{
    Invert(item);
    if(item->state == 0){
	if(item->invert){
	    Normal(item);
	}
    } else {
	if(item->invert2){
	    Normal(item);
	}
    }
    if(item->string){
	_FilledBox(item,0,0,item->w-1,item->h-1);
    } else {
	if(item->state == 0){
	    DrawItemBitmap(item,item->bitmap1);
	} else {
	    DrawItemBitmap(item,item->bitmap2);
	}
    }
    Normal(item);
    if(item->state == 0){
	if(item->invert){
	    Invert(item);
	}
	ItemText(item,item->string);
    } else {
	if(item->invert2){
	    Invert(item);
	}
	ItemText(item,item->string2);
    }
    Normal(item);
}

DrawMenu(menu)
MenuWindow	*menu;
{
MenuItem	*item;

    /*
    _SetColor(menu,menu->foreground);
    Text(menu,menu->fontwidth,menu->fontheight,menu->line);
    ** display menu items
    */
    for(item=menu->item;item;item=item->next){
	if(item->mapped){
	    DrawItem(item);
	}
    }
}

MenuItem *CreateMenuItem(menu,x,y,w,h,string,invert,string2,invert2,fontname,func,type,bits)
MenuWindow	*menu;
int x,y,w,h;
char	*string;
int	invert;
char	*string2;
int	invert2;
char		*fontname;
PFI		func;
int		type;
char		*bits;
{
MenuItem	*item;
int	borderwidth = 1;
int	border;
int	background;
WindowData		*windowdata;
extern	int	item_event();

    /*
    ** 			ITEM WINDOW
    */
    border = menu->foreground;
    background = menu->background;
    item = (MenuItem *)calloc(1,sizeof(MenuItem));
    CopyWindowStruct(menu,item);
    item->type = type;
    item->windowname = string;
    item->menu = menu;
    item->mapped = 1;
    item->x = x;
    item->y = y;
    item->w = w-1;
    item->h = h-1;
    item->func = func;
    item->invert = invert;
    item->invert2 = invert2;
    item->string = string;
    item->string2 = string2;
    item->window = (Window)XCreateSimpleWindow(item->display,menu->window,
	x,y,w-1,h-1,borderwidth,border,background);
    item->drawable = item->window;
    XSelectInput(item->display,item->window, 
	KeyPressMask | ExposureMask | ButtonPressMask | ButtonReleaseMask
	| EnterWindowMask | LeaveWindowMask );
    item->context = XCreateGC(item->display,item->window,0,0);
    XCopyGC(menu->display,menu->context,~0,item->context);
    SetFont(item,fontname);
    item->bitmap1 = CreateItemPixmap(item,bits);
    item->bitmap2 = CreateItemPixmap(item,button2_bits);
    windowdata = (WindowData *)calloc(1,sizeof(WindowData));
    windowdata->id = MENU_WINDOW;
    windowdata->parent = (BasicWindow *)item;
    windowdata->func = item_event;
    XSaveContext(item->display,item->window,datacontext,(XPointer)windowdata);
    XMapWindow(item->display,item->window);
    return(item);
}

AddMenuItem(menu,x,y,w,h,string,invert,string2,invert2,fontname,func,type,bits)
MenuWindow	*menu;
int 		x,y,w,h;
char		*string;
char		*string2;
char		*fontname;
PFI		func;
int		type;
char		*bits;
{
MenuItem	*item;

    item = CreateMenuItem(menu,x,y,w,h,string,invert,string2,invert2,fontname,func,type,bits);
    /*
    ** add the item to the list
    */
    item->next = menu->item;
    menu->item = item;
}

SetItemFont(item,fontname)
MenuItem	*item;
char		*fontname;
{
    item->font = XLoadFont(item->display,fontname);
    if (item->font == BadName){
	fprintf(stderr,"unable to open text font");
	return(0);
    }
    XSetFont(item->display,item->context,item->font);
}

ItemText(item,s)
MenuItem	*item;
char		*s;
{
    if(s == NULL) return;
    XPSDrawText(item->display,item->window,item->context,
    3,item->fontheight,s);
}

PrintItem(item,event)
MenuItem	*item;
XEvent		*event;
{
    fprintf(stderr,"%s\n",item->string);
}

Highlight1(item)
MenuItem	*item;
{
    _SetColor(item,item->foreground);
    _Box(item,0,0,item->w-1,item->h-1);
}

Highlight2(item)
MenuItem	*item;
{
    _SetColor(item,item->background);
    _Box(item,0,0,item->w-1,item->h-1);
}

item_event(item,event)
MenuItem	*item;
XEvent		*event;
{
XExposeEvent	*xev;

    switch (event->type) {
    case Expose:
	xev = (XExposeEvent *)event;
	if(xev->count == 0){
	    DrawItem(item);
	}
	break;
    case EnterNotify:
	Highlight1(item);
	break;
    case LeaveNotify:
	Highlight2(item);
	break;
    case ButtonPress:
	if(item->type == TOGGLE){
	    ToggleItemState(item);
	} else {
	    item->state = 1;
	}
	if(item->func){
	    item->func(item,event);
	}
	DrawItem(item);
	break;
    case ButtonRelease:
	if(item->type == BUTTON){
	    item->state = 0;
	}
	DrawItem(item);
	break;
    case KeyPress:
	break;		
    case MotionNotify :
	break;		
    default:
	break;
    }
}

ToggleItemState(item)
MenuItem	*item;
{
    item->state = !item->state;
}

ToggleAxis(item)
MenuItem	*item;
{
    if(item->state == 0){
	SetAxisMode(XMODE);
    } else {
	SetAxisMode(YMODE);
    }
}

DoPrintOut()
{
    PrintOut(G);
    RefreshGraph(G);
}

#define VEL 5

RandomShift()
{
int j,i;
Plot	*plot;

    for(j=0;j<50;j++){
	plot = SelectedPlot(G);
	if(plot->edata == NULL) return;
	for(i=0;i<plot->npoints;i++){
	    /*
	    ** use the error bars for direction
	    */
	    plot->data[i].x += VEL*cos(plot->edata[i].value);
	    plot->data[i].y += VEL*sin(plot->edata[i].value);
	    plot->edata[i].value += 5;
	}
	ScaleAndRefreshGraph(G);
	XFlush(G->display);
	Usleep(100000);
    }
}


SampleMenu(menu)
{
extern int ToggleDrawmode();
extern int ToggleZapmode();
extern int TogglePinchmode();
extern int TogglePolyLineMode();
extern int Quit();
extern int RightShift();
extern int LeftShift();

    AddMenuItem(menu,0,0,40,20,"XAXIS",0,"YAXIS",0,"fixed",
	ToggleAxis,TOGGLE,NULL);
    AddMenuItem(menu,40,0,80,20,"PINCH",0,"PINCH",1,"fixed",
	TogglePinchmode,TOGGLE,NULL);
    AddMenuItem(menu,0,20,40,20,"DRAW",0,"DRAW",1,"fixed",
	ToggleDrawmode,TOGGLE,NULL);
    AddMenuItem(menu,40,20,80,20,"ZAP",0,"ZAP",1,"fixed",
	ToggleZapmode,TOGGLE,NULL);
    AddMenuItem(menu,0,40,20,20,NULL,1,NULL,1,"fixed",
	TogglePolyLineMode,TOGGLE,zap_bits);
    AddMenuItem(menu,60,40,20,20,NULL,1,NULL,1,"fixed",
	NULL,TOGGLE,toggle2_bits);
    AddMenuItem(menu,20,40,20,20,NULL,1,NULL,1,"fixed",
	LeftShift,BUTTON,larrow_bits);
    AddMenuItem(menu,40,40,20,20,NULL,0,NULL,1,"fixed",
	RightShift,BUTTON,rarrow_bits);
    AddMenuItem(menu,0,60,40,20,"QUIT",0,"QUIT",0,"fixed",
	Quit,BUTTON,NULL);
    AddMenuItem(menu,40,60,40,20,"PRINT",0,"PRINT",0,"fixed",
	DoPrintOut,BUTTON,NULL);
    AddMenuItem(menu,0,80,20,20,NULL,0,NULL,1,"fixed",
	RandomShift,BUTTON,button_bits);
    AddMenuItem(menu,20,80,20,20,NULL,0,NULL,1,"fixed",
	NULL,TOGGLE,button2_bits);
    AddMenuItem(menu,40,80,20,20,NULL,0,NULL,1,"fixed",
	NULL,TOGGLE,button_bits);
    AddMenuItem(menu,60,80,20,20,NULL,0,NULL,1,"fixed",
	NULL,BUTTON,button_bits);
    /*
    AddMenuItem(menu,0,60,80,20,"Item4","times_roman8",PrintItem);
    AddMenuItem(menu,0,80,80,20,"Item5","courier10",PrintItem);
    AddMenuItem(menu,0,100,80,20,"Item6","helvetica10",PrintItem);
    */
}

