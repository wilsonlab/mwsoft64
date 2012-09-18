#include "xclust_ext.h"
#include <math.h>
#include <sys/time.h>

/* button bitmaps */
#include "ibutton.bitmap"
#include "eye.bitmap"
#include "eye2.bitmap"
#include "rarrow.bitmap"
#include "larrow.bitmap"
#include "zap.bitmap"
#include "toggle2.bitmap"
#include "ibutton2.bitmap"
#include "circle.bitmap"
#include "fcircle.bitmap"
#include "uparrow.bitmap"
#include "iuparrow.bitmap"
#include "dnarrow.bitmap"
#include "idnarrow.bitmap"
#include "zoomin.bitmap"
#include "zoomout.bitmap"
#include "forward.bitmap"
#include "fforward.bitmap"
#include "ifforward.bitmap"
#include "fbackward.bitmap"
#include "backward.bitmap"
#include "ifbackward.bitmap"

extern int MenuEvalOverlap();
extern int MenuSetShowClusters();
extern int MenuContinuousForward();
extern int MenuContinuousBackward();
extern int MenuZoomIn();
extern int MenuZoomOut();
extern int MenuAutoscale();
extern int MenuFlashCluster();
extern int MenuClusterBlockUp();
extern int MenuClusterBlockDown();
extern int MenuProjectionBlockUp();
extern int MenuProjectionBlockDown();
extern int MenuUndoClusterChange();
extern int MenuForwardXhilo();
extern int MenuBackwardXhilo();
extern int MenuAutoprocess();
extern int MenuSaveDefaults();
extern int MenuEpochName();
extern int MenuEpochKill();
extern int MenuLabelClusters();
extern int MenuButtonShowOnlyCluster();
extern int MenuButtonHideCluster();
extern int MenuShowClustStat();
extern int MenuEnterStart();
extern int MenuEnterEnd();
extern int MenuEnterTstart();
extern int MenuEnterTend();
/* extern int MenuSpikeTimestamp(); */
/* extern int MenuSpikeId(); */
extern int MenuLoadPoints();
extern int MenuLoadAllPoints();
extern int MenuLoadEpochFile();
extern int MenuSaveEpochFile();
extern int MenuDefineNewEpoch();
extern int MenuChangeUnits();
extern int MenuMovieFrame();
extern int MenuSetOverlay();
extern int MenuGrab();
extern int MenuViewAvg();
extern int MenuViewDir();
extern int MenuViewRate();
extern int MenuViewBehav();
extern int MenuViewSpot();
extern int MenuViewAcorr();
extern int MenuViewXcorr();
extern int MenuViewISI();
extern int MenuViewSpikes();
extern int MenuHideCluster();
extern int MenuHideAllBounds();
extern int MenuEnableCluster();
extern int MenuWriteClusterIndices();
extern int MenuCopyCluster();
extern int MenuWorldTextEntry();
extern int MenuPartialPlotLoad();
extern int MenuPartialPlotRead();
extern int MenuPartialPlotReset();
extern int MenuDeletePlot();
extern int MenuWriteClusterBounds();
extern int MenuReadClusterBounds();
extern int MenuDeleteCluster();
extern int MenuDeleteAllClusters();
extern int MenuDeleteClusterBounds();
extern int MenuWriteClusters();
extern int MenuSetCluster();
extern int MenuSetDrawmode();
extern int MenuSetZapmode();
extern int MenuSetPinchMode();
extern int MenuSetPolyLineMode();
extern int MenuSetClusterBoxMode();
extern int MenuSetClusterPolyLineMode();
extern int MenuSetClusterMode();
extern int MenuSetProjection0();
extern int MenuSetProjection1();
extern int MenuSetProjection2();
extern int MenuSetClusterScore();
extern int MenuSetClusterColor();
extern int MenuToggleXAutoscale();
extern int MenuUnsetClusterScore();
extern int MenuRandomizeProjection();
extern int MenuSetProjectionNext();
extern int MenuBlockMovie();
extern int Quit();
extern int RightShift();
extern int LeftShift();

static MenuItem	*debugitem;

/* #ifdef OLD */
/* Usleep(usec) */
/* int usec; */
/* { */
/* struct timeval  timeout; */

/*     timeout.tv_sec = 0; */
/*     timeout.tv_usec = usec; */
/*     select(32,NULL,NULL,NULL,&timeout); */
/* } */
/* #endif */

/* #define BM_BUTTON	0 */
/* #define BM_RARROW	1 */
/* #define BM_LARROW	2 */
/* #define BM_ZAP		3 */
/* #define BM_TOGGLE2	4 */
/* #define BM_BUTTON2	5 */

/* Height in pixels of buttons */
#define BUTTONH		20
#define SHORTBUTTONH	18

CollapseMenu(menuframe)
     MenuFrame *menuframe;
{
  MenuContainer *menucontainer;
  MenuFrame *nextframe;
  XWindowAttributes window_attr;
  int hdiff = 0, height = 0, reqheight = 0;


  menucontainer = menuframe->container;

  if (menuframe->collapsible) {

    height = menuframe->wheight;
    reqheight = GetMenuFrameHeight(menuframe);

    if (height==reqheight)
      return;

    hdiff = height - reqheight;
    menuframe->wheight = reqheight;
    XResizeWindow(menuframe->display,menuframe->window,
		  menuframe->wwidth,menuframe->wheight);

    /* move up following menus */
    for (nextframe = menuframe->next; nextframe; nextframe = nextframe->next) {
      XGetWindowAttributes(nextframe->display, nextframe->window, &window_attr);
      XMoveWindow(nextframe->display, nextframe->window, window_attr.x, window_attr.y - hdiff);
    }
  }
}

MenuFrame *AddMenuFrame(menucontainer, collapsible, expanded, showtitle, title)
     MenuContainer *menucontainer;
     int collapsible;
     int expanded;
     int showtitle;
     char *title;
{

  MenuFrame *menuframe;

  menuframe = (MenuFrame *)calloc(1, sizeof(MenuFrame));
  menuframe->collapsible = collapsible;
  menuframe->expanded = expanded;
  menuframe->showtitle = showtitle;
  menuframe->title = title;
  menuframe->mapped = 1;
  menuframe->fontname = DEFAULTFONT;

  menuframe->container = menucontainer;
  menuframe->next = menucontainer->menuframe;
  menucontainer->menuframe = menuframe;

  return menuframe;

}

PopulateMenuContainer(menucontainer)
     MenuContainer *menucontainer;
{
  MenuFrame *menuframe;
  MenuWindow *menu;

  /* Add Menus in reverse order */

  /* add epoch menu */
  menuframe = (MenuFrame*) AddMenuFrame(menucontainer, 1, 1, 1, "epoch menu");

  menu = (MenuWindow*)calloc(1, sizeof(MenuWindow));  
  menu->windowname = "/epochmenu";
  strcpy(menu->line, "EPOCHMENU");
  menu->frame = menuframe->container->frame;
  menu->mapped = 1;
  menu->fontname = DEFAULTFONT;
  menu->background = MININTERFACECOLOR + MENUBACKGROUND;
  menu->foreground = MININTERFACECOLOR + MENUFOREGROUND;
  menu->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
  menu->itemforeground = MININTERFACECOLOR + MENUITEMFOREGROUND;
  menu->parent = menuframe;
  menuframe->menu = menu;

  /* add analysis menu */
  menuframe = (MenuFrame*) AddMenuFrame(menucontainer, 1, 1, 1, "analysis menu");

  menu = (MenuWindow*)calloc(1, sizeof(MenuWindow));  
  menu->windowname = "/analysismenu";
  strcpy(menu->line, "ANALYSISMENU");
  menu->frame = menuframe->container->frame;
  menu->mapped = 1;
  menu->fontname = DEFAULTFONT;
  menu->background = MININTERFACECOLOR + MENUBACKGROUND;
  menu->foreground = MININTERFACECOLOR + MENUFOREGROUND;
  menu->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
  menu->itemforeground = MININTERFACECOLOR + MENUITEMFOREGROUND;
  menu->parent = menuframe;
  menuframe->menu = menu;

  /* add cluster info menu */
  menuframe = (MenuFrame*) AddMenuFrame(menucontainer, 1, 0, 1, "cluster info");

  menu = (MenuWindow*)calloc(1, sizeof(MenuWindow));
  menu->windowname = "/cluststat";
  strcpy(menu->line, "CLUSTSTAT");
  menu->frame = menuframe->container->frame;
  menu->mapped = 1;
  menu->fontname = DEFAULTFONT;
  menu->background = BLACK;
  menu->foreground = WHITE;
  menu->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
  menu->itemforeground = MININTERFACECOLOR + MENUITEMFOREGROUND;
  menu->parent = menuframe;
  menuframe->menu = menu;

  /* add cluster menu */
  menuframe = (MenuFrame*) AddMenuFrame(menucontainer, 1, 1, 1, "cluster menu");

  menu = (MenuWindow*)calloc(1, sizeof(MenuWindow));  
  menu->windowname = "/clustermenu";
  strcpy(menu->line, "CLUSTERMENU");
  menu->frame = menuframe->container->frame;
  menu->mapped = 1;
  menu->fontname = DEFAULTFONT;
  menu->background = MININTERFACECOLOR + MENUBACKGROUND;
  menu->foreground = MININTERFACECOLOR + MENUFOREGROUND;
  menu->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
  menu->itemforeground = MININTERFACECOLOR + MENUITEMFOREGROUND;
  menu->parent = menuframe;
  menuframe->menu = menu;

  /* add color menu */
  menuframe = (MenuFrame*) AddMenuFrame(menucontainer, 1, 0, 1, "color menu");

  menu = (MenuWindow*)calloc(1, sizeof(MenuWindow));  
  menu->windowname = "/colormenu";
  strcpy(menu->line, "COLORMENU");
  menu->frame = menuframe->container->frame;
  menu->mapped = 1;
  menu->fontname = DEFAULTFONT;
  menu->background = MININTERFACECOLOR + MENUBACKGROUND;
  menu->foreground = MININTERFACECOLOR + MENUFOREGROUND;
  menu->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
  menu->itemforeground = MININTERFACECOLOR + MENUITEMFOREGROUND;
  menu->parent = menuframe;
  menuframe->menu = menu;

  /* add control menu */
  menuframe = (MenuFrame*) AddMenuFrame(menucontainer, 0, 1, 0, "control menu");

  menu = (MenuWindow*)calloc(1, sizeof(MenuWindow));  
  menu->windowname = "/controlmenu";
  strcpy(menu->line, "CONTROLMENU");
  menu->frame = menuframe->container->frame;
  menu->mapped = 1;
  menu->fontname = DEFAULTFONT;
  menu->background = MININTERFACECOLOR + MENUBACKGROUND;
  menu->foreground = MININTERFACECOLOR + MENUFOREGROUND;
  menu->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
  menu->itemforeground = MININTERFACECOLOR + MENUITEMFOREGROUND;
  menu->parent = menuframe;
  menuframe->menu = menu;

}



MenuWindow	*GetMenu(name)
     char	*name;
{
  MenuWindow	*menu;
  MenuFrame *menuframe;

  for (menuframe=F->menucontainer->menuframe; menuframe; menuframe = menuframe->next) {
    if(strcmp(menuframe->menu->windowname, name) == 0)
      return menuframe->menu;
  }

  /*
   ** lookup the menu 
   */
/*   for(menu=F->menu;menu;menu=menu->next){ */
/*     if(strcmp(menu->windowname,name) == 0){ */
/*       return(menu); */
/*     } */
/*   } */
  for(menu=F->text;menu;menu=menu->next){
    if(strcmp(menu->windowname,name) == 0){
      return(menu);
    }
  }
  return(NULL);
}


MoveMenuWindow(y)
int	y;
{
/* MenuWindow	*menu; */
/* char	*menuname; */
  MenuContainer *menucontainer;
  int	height;

  menucontainer = F->menucontainer;  

/*     menuname = "/menu"; */
/*     if((menu = GetMenu(menuname)) == NULL){ */
/* 	/\* */
/* 	** unable to locate menu given in itemname */
/* 	*\/ */
/* 	fprintf(stderr,"ERROR: invalid menu name '%s'\n", */
/* 	    menuname); */
/* 	return(0); */
/*     } */

    if((height = menucontainer->wheight-y+1) < 1){
	height = 1;
    }
/*     XMoveResizeWindow(menu->display,menu->window,menu->frame->graph->wwidth,y, */
/* 		      menu->wwidth,height); */
    XMoveResizeWindow(menucontainer->display,menucontainer->window,menucontainer->frame->graph->wwidth,y,
		      menucontainer->wwidth,height);
		      /*
    DrawMenu(menu);
    */
}

MenuItem *LookupMenuItem(itemname)
char	*itemname;
{
MenuItem	*item;
MenuWindow	*menu;
char	menuname[100];
char	*ptr;
char	*strchr();

    if(itemname == NULL) return(NULL);
    /*
    ** locate the menu from the root of the itemname
    */
    strcpy(menuname,itemname);
    if((ptr = strchr(menuname+1,'/')) == NULL){
	/*
	** unable to extract the menu name from the itemname
	*/
	fprintf(stderr,"ERROR: invalid item name '%s'\n",
	    itemname);
	return(0);
    }
    *ptr = '\0';
    if((menu = GetMenu(menuname)) == NULL){
	/*
	** unable to locate menu given in itemname
	*/
	fprintf(stderr,"ERROR: invalid menu name '%s' for item '%s'\n",
	    menuname,itemname);
	return(0);
    }
    for(item = menu->item;item;item=item->next){
	if(item->name && (strcmp(item->name,itemname) == 0)){
	    return(item);
	}
    }
    return(NULL);
}

LoadBitmaps(basic)
BasicWindow	*basic;
{
    AddBitmap(basic,"button",button_bits,16,16,-1,-1);
    AddBitmap(basic,"button2",button2_bits,16,16,-1,-1);
    AddBitmap(basic,"zap",zap_bits,16,16,-1,-1);
    AddBitmap(basic,"circle",circle_bits,5,5,-1,-1);
    AddBitmap(basic,"fcircle",fcircle_bits,5,5,-1,-1);
    AddBitmap(basic,"uparrow",uparrow_bits,16,16,-1,-1);
    AddBitmap(basic,"iuparrow",iuparrow_bits,16,16,-1,-1);
    AddBitmap(basic,"dnarrow",dnarrow_bits,16,16,-1,-1);
    AddBitmap(basic,"idnarrow",idnarrow_bits,16,16,-1,-1);
    AddBitmap(basic,"zoomin",zoomin_bits,16,16,-1,-1);
    AddBitmap(basic,"zoomout",zoomout_bits,16,16,-1,-1);
    AddBitmap(basic,"fbackward",fbackward_bits,16,16,-1,-1);
    AddBitmap(basic,"ifbackward",ifbackward_bits,16,16,-1,-1);
    AddBitmap(basic,"ifforward",ifforward_bits,16,16,-1,-1);
    AddBitmap(basic,"fforward",fforward_bits,16,16,-1,-1);
    AddBitmap(basic,"backward",backward_bits,16,16,-1,-1);
    AddBitmap(basic,"forward",forward_bits,16,16,-1,-1);
    AddBitmap(basic,"eye",eye_bits,16,16,-1,-1);
    AddBitmap(basic,"eye2",eye2_bits,16,16,-1,-1);

/*     /\* black (k) background buttons for clusters *\/ */
/*     AddBitmap(basic,"kbutton",button_bits,16,16,BLACK,WHITE); */
/*     AddBitmap(basic,"kbutton2",button2_bits,16,16,BLACK,WHITE); */
/*     AddBitmap(basic,"keye",eye_bits,16,16,BLACK,WHITE); */
/*     AddBitmap(basic,"keye2",eye2_bits,16,16,BLACK,WHITE); */

    fprintf(stderr,"done\n");
}

RefreshMenu(basic)
BasicWindow	*basic;
{
    DisplayLabels(basic);
}

RefreshMenuItems(menu)
MenuWindow	*menu;
{
    ClearWindow(menu);
    DrawMenu(menu); 
}

RefreshItem(itemname)
char	*itemname;
{
MenuItem	*item;

    if((item = LookupMenuItem(itemname)) == NULL) {
	return(0);
    }
    if(item->mapped){
	if(item->redrawfunc){
	    item->redrawfunc(item);
	}
    }
}


/* Pixmap CreateItemPixmap(item,bits) */
/* MenuItem	*item; */
/* char		*bits; */
/* { */
/* int	depth = 1; */

/*     if(bits){ */
/* 	return( */
/* 	XCreatePixmapFromBitmapData(item->display,item->window, bits, */
/* 	16,16,GetPixel(item->foreground),GetPixel(item->background), */
/* 	DefaultDepthOfScreen(DefaultScreenOfDisplay(item->display))) */
/* 	); */
/*     } else { */
/* 	return((Pixmap)NULL); */
/*     } */
/* } */

ChangeItemBitmap(item,bitname,bitname2)
MenuItem	*item;
char		*bitname;
char		*bitname2;
{
    if(bitname){
	item->bitmap1 = GetPixmapByName(bitname);
	/*
	item->bitmap1 = CreateItemPixmap(item,bits);
	** is the second bitmap specified
	*/
	if(bitname2){
	/*
	    item->bitmap2 = CreateItemPixmap(item,bits2);
	    */
	    item->bitmap2 = GetPixmapByName(bitname2);
	} else {
	    /*
	    ** use the default bitmap
	    item->bitmap2 = CreateItemPixmap(item,button2_bits);
	    */
	    item->bitmap2 = GetPixmapByName("button2");
	}
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
char	line[200];
char	*valuestr;
int	maxlen;
int	trim;
int	th;
int	strsize;
int	valuesize;
int	linesize;
int	i;
int     stroffset = 0;

    if(item == NULL) return;
    if(!item->mapped) return;
    line[0] = '\0';
    valuesize = 0;
    strsize = 0;
    linesize = 0;
    SetForeground(item,item->itemforeground);
    SetBackground(item,item->itembackground);
    /*
    ** prepare the text to be displayed in the item
    */
    if(item->string){
	TextExtent(item->string,&strsize,&th);
    } 
    if(item->bitmap1 || item->bitmap2){
      stroffset = 16;
    }
    if(item->value){
	/*
	** check for value strings which exceed the width of the
	** item
	*/
	TextExtent(item->value,&valuesize,&th);
	/*
	** note add space to take into account the space between the str and 
	** the value and add a bit more to give a space at the end
	*/
	if(valuesize + stroffset + strsize + 10> item->w){
	    /*
	    ** trim off letters until it fits
	    */
	    for(i=0;i<strlen(item->value);i++){
		valuestr = item->value + i;
		TextExtent(valuestr,&valuesize,&th);
		if(valuesize + stroffset + strsize + 10 < item->w) break;
	    }
	} else {
	    /*
	    ** show the whole string
	    */
	    valuestr = item->value;
	}
	/*
	** add in the item string
	*/
	if(item->string){
	    if(item->state == 0){
		sprintf(line,"%s %s",item->string,valuestr);
	    } else {
		sprintf(line,"%s %s",item->string2,valuestr);
	    }
	} else {
	    sprintf(line,"%s",item->value);
	}
    } else {
	/*
	** the item has no value string so just display the item string
	*/
	if(item->string){
	    if(item->state == 0){
		sprintf(line,"%s",item->string);
	    } else {
		sprintf(line,"%s",item->string2);
	    }
	}
    }
    /*
    ** prepare the colors for normal or inverted display dependent
    ** on the item state
    */
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
    /*
    ** display the background
    */
    _FilledBox(item,0,0,item->w,item->h);
    /*
    ** if there is a bitmap then put it up over the background
    */

    /*
    ** select the bitmap based on the item state
    */
    if(item->state == 0){
      if (item->bitmap1){
	DrawItemBitmap(item,item->bitmap1);
      }
    } else{
      if (item->bitmap2){
	DrawItemBitmap(item,item->bitmap2);
      }
    }
    
    Normal(item);
    if(!item->enabled){
      /*
      ** show the item in a disabled state
      */
      _SetColor(item,MININTERFACECOLOR + MENUITEMDISABLE);
    }
    /*
    ** display the item text
    */
    if(item->state == 0){
      if(item->invert){
	Invert(item);
      }
      ItemText(item,line, stroffset);
    } else {
      if(item->invert2){
	Invert(item);
      }
      ItemText(item,line, stroffset);
    }
    /*
    ** display the text input area if enabled
    */
    if(item->type & INPUTAREA){
      /*
	SetBackground(item,MINCOLOR + MENUITEMINPUT);
	_FilledBox(item,strsize+3,3,item->w-3,item->h-3);
	** 3d edges
	*/
      _SetColor(item,MININTERFACECOLOR + MENUITEMDARKEDGE);
      _DrawLine(item,strsize+3,2,strsize+3,item->h-3);
      _DrawLine(item,strsize+3,2,item->w-3,2);
      _SetColor(item,MININTERFACECOLOR + MENUITEMLIGHTEDGE);
      _DrawLine(item,item->w-3,item->h-3,item->w-3,3);
      _DrawLine(item,item->w-3,item->h-3,strsize+3,item->h-3);
      /*
      ** if so then draw the cursor at the current text entry
      ** point
      */
      TextExtent(line,&linesize,&th);
      if(valuesize > 0){
	_SetColor(item,item->foreground);
	_DrawLine(item,linesize+1,item->h-4,linesize+3,item->h-6);
	_DrawLine(item,linesize+3,item->h-6,linesize+5,item->h-4);
      }
    }
    
    /*
    ** draw the 3d edges
    */
    if(item->state == 0){
	if(item->invert){
	    _SetColor(item,MININTERFACECOLOR + MENUITEMDARKEDGE);
	} else {
	    _SetColor(item,MININTERFACECOLOR + MENUITEMLIGHTEDGE);
	}
    } else {
	if(item->invert2){
	    _SetColor(item,MININTERFACECOLOR + MENUITEMDARKEDGE);
	} else {
	    _SetColor(item,MININTERFACECOLOR + MENUITEMLIGHTEDGE);
	}
    }
    _DrawLine(item,1,1,1,item->h-2);
    _DrawLine(item,1,1,item->w-2,1);

    if(item->state == 0){
	if(item->invert){
	    _SetColor(item,MININTERFACECOLOR + MENUITEMLIGHTEDGE);
	} else {
	    _SetColor(item,MININTERFACECOLOR + MENUITEMDARKEDGE);
	}
    } else {
	if(item->invert2){
	    _SetColor(item,MININTERFACECOLOR + MENUITEMLIGHTEDGE);
	} else {
	    _SetColor(item,MININTERFACECOLOR + MENUITEMDARKEDGE);
	}
    }
    _DrawLine(item,item->w-2,item->h-2,item->w-2,1);
    _DrawLine(item,item->w-2,item->h-2,1,item->h-2);
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
    DisplayLabels(menu);
    for(item=menu->item;item;item=item->next){
	if(item->mapped){
	    if(item->redrawfunc){
		item->redrawfunc(item);
	    }
	}
    }
}

MenuItem *CreateMenuItem(menu,x,y,w,h,name,string,invert,string2,invert2,fontname,func,type,bitname,bitname2,value,class,data,redrawfunc)
MenuWindow	*menu;
int x,y,w,h;
char	*name;
char	*string;
int	invert;
char	*string2;
int	invert2;
char		*fontname;
PFI		func;
int		type;
char		*bitname;
char		*bitname2;
char		*value;
char		*class;
char            *data;
PFI		redrawfunc;
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
    border = menu->itemforeground;
    background = menu->itembackground;
    item = (MenuItem *)calloc(1,sizeof(MenuItem));
    CopyWindowStruct(menu,item);
    item->redrawfunc = redrawfunc;
    item->type = type;
    item->windowname = string;
    item->menu = menu;
    item->mapped = 1;
    item->enabled = 1;
    item->itemforeground = menu->itemforeground;
    item->itembackground = menu->itembackground;
    SetForeground(item,item->itemforeground);
    SetBackground(item,item->itembackground);
    /* item->itembackground = GRAY1; */
    item->name = name;
    item->x = x;
    item->y = y;
    item->w = w-1;
    item->h = h-1;
    item->func = func;
    item->invert = invert;
    item->invert2 = invert2;
    item->string = string;
    item->string2 = string2;
    item->data = data;
    item->entered = 0;
    item->window = (Window)XCreateSimpleWindow(item->display,menu->window,
	x,y,w-1,h-1,borderwidth,GetPixel(border),GetPixel(background));
    item->drawable = item->window;
    if(value){
	item->value = malloc(strlen(value)+1);
	strcpy(item->value,value);
    } else {
	item->value = NULL;
    }
    item->class = class;
    XSelectInput(item->display,item->window, 
	KeyPressMask | ExposureMask | ButtonPressMask | ButtonReleaseMask
	| EnterWindowMask | LeaveWindowMask );
    item->context = XCreateGC(item->display,item->window,0,0);
    XCopyGC(menu->display,menu->context,~0,item->context);
    SetFont(item,fontname);
    if(bitname){
	item->bitmap1 = GetPixmapByName(bitname);
	/*
	item->bitmap1 = CreateItemPixmap(item,bits);
	** is the second bitmap specified
	*/
	if(bitname2){
	/*
	    item->bitmap2 = CreateItemPixmap(item,bits2);
	    */
	    item->bitmap2 = GetPixmapByName(bitname2);
	} else {
	    /*
	    ** use the default bitmap
	    item->bitmap2 = CreatetIemPixmap(item,button2_bits);
	    */
	    item->bitmap2 = GetPixmapByName("button2");
	}
    }
    windowdata = (WindowData *)calloc(1,sizeof(WindowData));
    windowdata->id = MENU_WINDOW;
    windowdata->parent = (BasicWindow *)item;
    windowdata->func = item_event;
    XSaveContext(item->display,item->window,datacontext,(XPointer)windowdata);
    XMapWindow(item->display,item->window);
    return(item);
}

MenuItem *AddMenuItem(menu,x,y,w,h,name,string,invert,string2,invert2,fontname,func,type,bits,bits2,value,class,data)
MenuWindow	*menu;
int 		x,y,w,h;
char		*name;
char		*string;
char		*string2;
char		*fontname;
PFI		func;
int		type;
char		*bits;
char		*bits2;
char		*value;
char		*class;
char            *data;
{
MenuItem	*item;

    item = CreateMenuItem(menu,x,y,w,h,name,string,invert,string2,invert2,menu->fontname,func,type,bits,bits2,value,class,data,DrawItem);
    /*
    ** add the item to the list
    */
    item->next = menu->item;
    menu->item = item;
    return(item);
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

ItemText(item,s, offset)
MenuItem	*item;
char		*s;
int              offset;
{
    if(s == NULL) return;
    XPSDrawText(item->display,item->window,item->context,
    offset+3,item->fontheight,s);
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
    /*
    ** draw the highlight box around the item
    */
    _SetColor(item,item->foreground);
    _Box(item,0,0,item->w-1,item->h-1);
}

Highlight2(item)
MenuItem	*item;
{
    _SetColor(item,item->background);
    _Box(item,0,0,item->w-1,item->h-1);
}

MenuCallItem(item,event)
MenuItem	*item;
XEvent		*event;
{
    if(item->func){
	item->func(item,event);
    }
    DrawItem(item);
}

MenuClassSetState(protecteditem,menu,class,state)
MenuItem		*protecteditem;
MenuWindow	*menu;
char	*class;
int	state;
{
MenuItem	*item;

    for(item=menu->item;item;item=item->next){
	if((item != protecteditem) && (item->class) 
	&& (strcmp(item->class,class) == 0)){
	    item->state = state;
	}
    }
}

MenuClassSetFunc(protecteditem,menu,class,func)
MenuItem		*protecteditem;
MenuWindow	*menu;
char	*class;
PFI	func;
{
MenuItem	*item;

    for(item=menu->item;item;item=item->next){
	if((item != protecteditem) && (item->class) 
	&& (strcmp(item->class,class) == 0)){
	  item->func = func;
	}
    }
}

MenuClassCallItems(protecteditem,menu,class)
MenuItem	*protecteditem;
MenuWindow	*menu;
char	*class;
{
MenuItem	*item;

    for(item=menu->item;item;item=item->next){
	if((item != protecteditem) && (item->class) 
	&& (strcmp(item->class,class) == 0)){
	    MenuCallItem(item,NULL);
	}
    }
}

MenuClassDrawItems(protecteditem,menu,class)
MenuItem	*protecteditem;
MenuWindow	*menu;
char	*class;
{
MenuItem	*item;

    for(item=menu->item;item;item=item->next){
	if((item != protecteditem) && (item->class) 
	&& (strcmp(item->class,class) == 0)){
	  DrawItem(item,NULL);
	}
    }
}

ItemEnterValue(item,event)
MenuItem	*item;
XKeyEvent	*event;
{
char	buffer[100];
char	c;
KeySym	key;
char	tmpstr[500];
int	strend;

    buffer[0] = '\0';
    /* 
    ** do key mapping to determine the actual key pressed
    */
    XLookupString(event,buffer,100, &key, NULL);
    if((c = *buffer) == '\0'){
	/*
	** check the key code for special keys
	*/
	if(debug){
	    printf("%s\n",XKeysymToString(key));
	}
	return;
    }
    if(c == '' || c == ''){		/* delete char */
	if(item->value && strlen(item->value) > 0){
	    strend = strlen(item->value)-1;
	    item->value[strend] = '\0';
	}
    } else 
#ifdef OLD
    if(c == ''){		/* new cluster color */
	if(strcmp(item->class,"cluster") == 0){
	    ChangeColor(MINCLUSTER + atoi(item->value));
	    RefreshGraph(G);
	}
    } else 
#endif
/*     if(c == ''){		/\* rotate cluster color *\/ */
/* 	if(strcmp(item->class,"cluster") == 0){ */
/* 	    rotatecolor = !rotatecolor; */
/* 	    if(rotatecolor == 1){ */
/* 		/\* */
/* 		** save the current value of the pixel */
/* 		*\/ */
/* 		SavePixel(); */
/* 	    } else { */
/* 		/\* */
/* 		** restore the previous value of the pixel */
/* 		*\/ */
/* 		RestorePixel(); */
/* 	    } */
/* 	} */
/*     } else  */
#ifdef OLD
    if(c == ''){		/* increment cluster color */
	if(strcmp(item->class,"cluster") == 0){
	    item->itemforeground++;
	    fprintf(stderr,"%s : %d\n",item->name,item->itemforeground);
	    SetClusterColor(item->menu->frame->graph,atoi(item->value),
	    item->itemforeground);
	}
    } else 
    if(c == ''){		/* decrement cluster color */
	if(strcmp(item->class,"cluster") == 0){
	    item->itemforeground--;
	    fprintf(stderr,"%s : %d\n",item->name,item->itemforeground);
	    SetClusterColor(item->menu->frame->graph,atoi(item->value),
	    item->itemforeground);
	}
    } else 
#endif
    if(c == ''){		/* delete line */
	if(item->value && strlen(item->value) > 0){
	    item->value[0] = '\0';
	}
    } else 
    if(c == '\r'){
	MenuCallItem(item,event);
    } else {
	if(item->value){
	    /*
	    ** append the character
	    */
	    sprintf(tmpstr,"%s%c",item->value,c);
	    free(item->value);
	} else {
	    sprintf(tmpstr,"%c",c);
	}
	/*
	** place the new string in the value
	*/
	item->value = (char *)malloc((strlen(tmpstr) + 1)*sizeof(char));
	strcpy(item->value,tmpstr);
    }
    DrawItem(item);
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
	if(item->enabled){
	    Highlight1(item);
	    item->entered = 1;
	}
	break;
    case LeaveNotify:
	if(item->enabled){
	    Highlight2(item);
	    item->entered = 0;
	}
	break;
    case ButtonPress:
      if(item->enabled){ 
	if (((XButtonPressedEvent*)event)->button == 1){
	  
	  if(item->type & TOGGLE){
	    ToggleItemState(item);
	  } else if (item->type & RADIO && item->state==1) {
	    /* abort if item state is already 1 */
	    break;
	  } else {
	    item->state = 1;
	  }
	  
	  /* if it's a radiobutton, unset all other members of the class */
	  /* MenuClassDrawItems is used to redraw other class buttons */
	  if (item->type & RADIO &&
	      item->class){
	    MenuClassSetState(item,item->menu,item->class,0);
	    MenuClassDrawItems(NULL,item->menu,item->class);
	  }

	  MenuCallItem(item,event);
	}
	else { /* button != 1 */

	  /* HACK: send other buttons only to functions that we know
	  ** can handle it. better solution is to make all menu
	  ** functions test for button 1, or to add a second function
	  ** to each item for 'higher' buttons*/
	  if (item->name &&
	      (item->func == MenuCopyCluster ||
	       item->func == MenuSetClusterScore))
	    MenuCallItem(item,event);
	}
      } /* end if(item->enabled) */
      break;
    case ButtonRelease:
	if(item->enabled && ( ((XButtonReleasedEvent*)event)->button == 1)){
	    if(item->type & BUTTON){
		item->state = 0;
	    }

	    /* HACK to avoid redrawing cluster buttons using messed up
	       colors while we are flashing cluster */
	    if(rotatecolor &&
	       item->class &&
	       strcmp(item->class, "cluster") == 0)
	      break;
	    else

	    DrawItem(item);
	}
	break;
    case KeyPress:
	if(item->enabled){
	    if(item->type & DIALOG){
		ItemEnterValue(item,event);
	    }
	}
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

MenuCSI()
{
  MenuWindow* menu;
    ComputeClusterIntervals(G,G->selectedcluster,0);
    UpdateProjectionMenu();
    menu = GetMenu("/cluststat");
    if(menu->mapped){
	DrawClustStat(menu);
    }
    RescaleGraph(G);
}

MenuAllCSI()
{
ClusterList	*clist;
 MenuWindow* menu;
    for(clist=G->clusterlist;clist;clist=clist->next){
	ComputeClusterIntervals(G,clist->clusterid,1);
    }
    UpdateProjectionMenu();
    menu = GetMenu("/cluststat");
    if(menu->mapped){
	DrawClustStat(menu);
    }
    RescaleGraph(G);
}

MenuShowClustStat(item)
MenuItem	*item;
{
MenuWindow	*cluststat;

 cluststat = GetMenu("/cluststat");
    if(item->state == 0){
	cluststat->mapped = 0;
	XUnmapWindow(cluststat->display,cluststat->window);
    } else {
	cluststat->mapped = 1;
	XMapWindow(cluststat->display,cluststat->window);
	DrawClustStat(cluststat);
    }
}

MenuShowColorStat(item)
MenuItem	*item;
{
MenuWindow	*colorstat;

 colorstat = GetMenu("/colorstat");
    if(item->state == 0){
	colorstat->mapped = 0;
	XUnmapWindow(colorstat->display,colorstat->window);
    } else {
	colorstat->mapped = 1;
	XMapWindow(colorstat->display,colorstat->window);
	DrawColorStat(colorstat);
    }
}



UpdateItemMapping(item)
MenuItem	*item;
{
    if(item->mapped){
	XMapWindow(item->display,item->window);
    } else {
	XUnmapWindow(item->display,item->window);
    }
}

SampleClustStat(cluststat)
MenuWindow	*cluststat;
{
/*
    AddMenuItem(cluststat,0,20,80,BUTTONH,"/cluststat/cluster",
	"Cluster: ",0,"Cluster: ",0,DEFAULTFONT,
	NULL,DIALOG,NULL,NULL,NULL,NULL);
*/
}

CreateColorStat(menu)
MenuWindow	*menu;
{
int	i,j;
MenuItem	*item;
 int	count;
 int itemsize_x = 20;
 int itemsize_y = 16;
 int ncol = 16;
 int nrow; /* = 16;*/
 nrow = ceil(MAXCLUSTER / ncol);

    count=0;
    for(j=0;j<nrow;j++){
	for(i=0;i<ncol;i++){
	  if (count>=MAXCOLORS)
	    break;
	  item = AddMenuItem(menu,i*itemsize_x,j*itemsize_y,itemsize_x,itemsize_y,NULL,
			     "",0,"",0,DEFAULTFONT,
			     MenuSetClusterColor,BUTTON,NULL,NULL,NULL,NULL);
	  item->itembackground = MAXINTERFACECOLORS + count;
	  item->itemforeground = MAXINTERFACECOLORS + count;
	  count++;
	}
    }
}

CreateMenus(menucontainer)
     MenuContainer *menucontainer;
{
  MenuWindow *menu;

  menu = (MenuWindow*) GetMenu("/controlmenu");

  if (menu) {
    MainMenuControlBlock(menu,0);
  }

  menu = (MenuWindow*) GetMenu("/colormenu");

  if (menu) {
    CreateColorStat(menu);
  }

  menu = (MenuWindow*) GetMenu("/clustermenu");

  if (menu) {
    MainMenuClusterBlock(menu,0);
  }

  menu = (MenuWindow*) GetMenu("/analysismenu");

  if (menu) {
    MainMenuAnalysisBlock(menu,0);
  }

  menu = (MenuWindow*) GetMenu("/epochmenu");

  if (menu) {
    MainMenuEpochBlock(menu,0);
  }
}

CreateMainMenu(menu)
MenuWindow	*menu;
{
int voffset;

/* start buttons at top of menu frame */
 voffset = 0;

 voffset = MainMenuControlBlock(menu,voffset);
 voffset += 5;
 voffset = MainMenuClusterBlock(menu,voffset);
 voffset += 5;

 /* indicate the height at which we should draw the cluster info
    window (see init.c) */
 menu->cluststatoffset = voffset; 

 voffset = MainMenuAnalysisBlock(menu,voffset);
 voffset += 5;
 voffset = MainMenuEpochBlock(menu,voffset);

 /* disable unused menu items */
    SetItemEnable("/menu/randx",0);
    SetItemEnable("/menu/randy",0);
    SetItemEnable("/menu/resetfile",0);
    SetItemEnable("/menu/text",0);
    SetItemEnable("/menu/poly",0);
    SetItemEnable("/menu/zap",0);
    SetItemEnable("/menu/pinch",0);
    SetItemEnable("/menu/deleteplot",0);
    SetItemEnable("/menu/draw",0);
    SetItemEnable("/menu/resetfile",1);

    fprintf(stderr,"done\n");
}

MainMenuControlBlock(menu,voffset)
MenuWindow	*menu;
int		voffset;
{
  int i;
  char* istr;
  int hoffset;
  char *newstr;
  int  width;
  char tempstr[256];

    AddMenuItem(menu,0,voffset,40,BUTTONH,NULL,
	"QUIT",0,"QUIT",0,DEFAULTFONT,
	Quit,BUTTON,NULL,NULL,NULL,NULL, NULL);
    AddMenuItem(menu,40,voffset,40,BUTTONH,NULL,
	"PRINT",0,"PRINT",0,DEFAULTFONT,
	DoPrintOut,BUTTON,NULL,NULL,NULL,NULL, NULL);

    AddLabelString(menu, "Save .xclust3rc:",
	87, voffset+BUTTONH-6,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,menu->fontname);

    AddMenuItem(menu,170,voffset,40,BUTTONH, "/controlmenu/savedefaultsglobal",
	"Global",0,"Global",1,DEFAULTFONT,
	MenuSaveDefaults,BUTTON,NULL,NULL,NULL,NULL, NULL);
    AddMenuItem(menu,210,voffset,40,BUTTONH, "/controlmenu/savedefaultslocal",
	"Local",0,"Local",1,DEFAULTFONT,
	MenuSaveDefaults,BUTTON,NULL,NULL,NULL,NULL, NULL);

    AddMenuItem(menu,260,voffset,60,BUTTONH, "/controlmenu/autoscale",
	"Autoscale",0, "Autoscale",0,DEFAULTFONT,
	MenuAutoscale,BUTTON,NULL,NULL,NULL,NULL, NULL);

    /* next row of buttons */
    voffset+=BUTTONH;

/*     AddMenuItem(menu,80,voffset,80,BUTTONH,"/controlmenu/deleteplot", */
/* 	"DELETE PLOT",0,"DELETE PLOT",0, */
/* 	DEFAULTFONT, MenuDeletePlot,BUTTON,NULL,NULL,NULL,NULL, NULL); */

/*     AddMenuItem(menu,160,voffset,40,BUTTONH,"/controlmenu/randx", */
/* 	"RandX",0,"RandX",1,DEFAULTFONT, */
/* 	NULL,TOGGLE,NULL,NULL,NULL,NULL, NULL); */
/*     AddMenuItem(menu,200,voffset,40,BUTTONH,"/controlmenu/randy", */
/* 	"RandY",0,"RandY",1,DEFAULTFONT, */
/* 	NULL,TOGGLE,NULL,NULL,NULL,NULL, NULL); */

/*     AddMenuItem(menu,160,voffset,40,BUTTONH,"/controlmenu/randx", */
/* 	"RandX",0,"RandX",1,DEFAULTFONT, */
/* 	MenuRandomizeX,TOGGLE,NULL,NULL,NULL,NULL, NULL); */
/*     AddMenuItem(menu,200,voffset,40,BUTTONH,"/controlmenu/randy", */
/* 	"RandY",0,"RandY",1,DEFAULTFONT, */
/* 	MenuRandomizeY,TOGGLE,NULL,NULL,NULL,NULL, NULL); */



    /*
    ** FORWARD/BACKWARD plot stepping
    */

    AddMenuItem(menu,0,voffset,45,BUTTONH, "/controlmenu/continuousbackwardstep",
	NULL,0,NULL,1,DEFAULTFONT,
	MenuContinuousBackward,(RADIO | TOGGLE | DIALOG),
	"fbackward","ifbackward","-10","continuous",NULL);
    AddMenuItem(menu,45,voffset,45,BUTTONH, "/controlmenu/backwardstep",
	NULL,0,NULL,0,DEFAULTFONT,
	MenuPartialPlotLoad,(TOGGLE | DIALOG),
	"backward","backward","-1",NULL, NULL);
    AddMenuItem(menu,90,voffset,45,BUTTONH, "/controlmenu/forwardstep",
	NULL,0,NULL,0,DEFAULTFONT,
	MenuPartialPlotLoad,(TOGGLE | DIALOG),
	"forward","forward","1",NULL, NULL);
    AddMenuItem(menu,135,voffset,45,BUTTONH, "/controlmenu/continuousforwardstep",
	NULL,0,NULL,1,DEFAULTFONT,
	MenuContinuousForward,(RADIO | TOGGLE | DIALOG),
	"fforward","ifforward","10","continuous", NULL);
    /*
    ** OVERLAY
    */
    AddMenuItem(menu,190,voffset,50,BUTTONH, "/controlmenu/overlay",
	"Overlay",0,"Overlay",1,DEFAULTFONT,
	MenuSetOverlay,TOGGLE,NULL,NULL,NULL,NULL, NULL);
    AddMenuItem(menu,240,voffset,40,BUTTONH, "/controlmenu/grab",
	"Grab",0,"Grab",0,DEFAULTFONT,
	MenuGrab,BUTTON,NULL,NULL,NULL,NULL, NULL);
/*     AddMenuItem(menu,240,voffset,40,BUTTONH, "/controlmenu/resetfile", */
/* 	"Reset",0, "Reset",0,DEFAULTFONT, */
/* 	MenuPartialPlotReset,BUTTON,NULL,NULL,NULL,NULL, NULL); */
    /*
    ** FLASH
    */
    AddMenuItem(menu,280,voffset,40,BUTTONH, "/controlmenu/flashcluster",
	"Flash",0, "Flash",1,DEFAULTFONT,
	MenuFlashCluster,TOGGLE,NULL,NULL,NULL,NULL, NULL);

/*     voffset += floor(1.5*BUTTONH); */

/*     AddLabelLine(menu, 0, voffset-2, voffset-2, 320, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, */
/* 		 SCREEN_LBL, PERMANENT_LBL); */
/*     AddLabelLine(menu, 0, voffset, voffset, 320, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, */
/* 		 SCREEN_LBL, PERMANENT_LBL); */
    /*
    ** CLUSTER BLOCK UP/DOWN MODE
    */
    voffset+=floor(1.75*BUTTONH);
    AddMenuItem(menu,0,voffset,20,SHORTBUTTONH, "/controlmenu/cluster/up",
	NULL,0,NULL,1,DEFAULTFONT,
	MenuClusterBlockDown,BUTTON,"uparrow","iuparrow","0",NULL, NULL);
    AddMenuItem(menu,0,voffset+SHORTBUTTONH,20,SHORTBUTTONH, "/controlmenu/cluster/down",
	NULL,0,NULL,1,DEFAULTFONT,
	MenuClusterBlockUp,BUTTON,"dnarrow","idnarrow","0",NULL, NULL);

    /*
    ** CLUSTER CHOICES
    */
    AddLabelString(menu, "Clusters",
	50,voffset-2,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,menu->fontname);

    for (i=0; i<CLUSTER_BLOCK_SIZE; i++) {

      AddMenuCluster(menu,i+1,20,voffset+i*SHORTBUTTONH);

    }

    /*
    ** x axis projection selections
    */
    AddLabelString(menu, "X",
	215,voffset-2,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,menu->fontname);
    AddLabelString(menu, "Y",
	242,voffset-2,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,menu->fontname);
    AddLabelString(menu, "Z",
	269,voffset-2,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,menu->fontname);
    AddLabelString(menu, "rand",
	287,voffset-2,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,menu->fontname);

    AddMenuItem(menu,127,voffset,20,SHORTBUTTONH, "/controlmenu/projection/x/up",
	NULL,0,NULL,1,DEFAULTFONT,
	MenuProjectionBlockDown,BUTTON,"uparrow","iuparrow","0",NULL, NULL);
    AddMenuItem(menu,127,voffset+SHORTBUTTONH,20,SHORTBUTTONH, "/controlmenu/projection/x/down",
	NULL,0,NULL,1,DEFAULTFONT,
	MenuProjectionBlockUp,BUTTON,"dnarrow","idnarrow","0",NULL, NULL);

    for (i=0; i<PROJECTION_BLOCK_SIZE; i++) {
      hoffset = 147; 
      width = 60;

      istr = (char *)malloc((ceil(log10((double)MAXPROJECTION))+1)*sizeof(char));
      sprintf(istr,"%d",i);

      sprintf(tempstr, "/controlmenu/projection/name/p%d", i);
      newstr = (char*)malloc(strlen(tempstr)+1);
      strcpy(newstr, tempstr);
      AddMenuItem(menu,hoffset,voffset+i*SHORTBUTTONH,width,SHORTBUTTONH, newstr,
		  "P",0,"P",1,DEFAULTFONT,
		  MenuSetProjection0,RADIO,NULL,NULL,"0","pname", istr);
      
      hoffset += width;

      width = 27;

      sprintf(tempstr, "/controlmenu/projection/x/p%d", i);
      newstr = (char*)malloc(strlen(tempstr)+1);
      strcpy(newstr, tempstr);
      AddMenuItem(menu,hoffset,voffset+i*SHORTBUTTONH,width,SHORTBUTTONH,newstr,
		  "",0,"",1,DEFAULTFONT,
		  MenuSetProjection0,BUTTON,NULL, NULL, NULL,"p0", istr);
      hoffset += width;

      sprintf(tempstr, "/controlmenu/projection/y/p%d", i);
      newstr = (char*)malloc(strlen(tempstr)+1);
      strcpy(newstr, tempstr);
      AddMenuItem(menu,hoffset,voffset+i*SHORTBUTTONH,width,SHORTBUTTONH,newstr,
		  "",0,"",1,DEFAULTFONT,
		  MenuSetProjection1,BUTTON,NULL,NULL,NULL,"p1", istr);
      hoffset += width;

      sprintf(tempstr, "/controlmenu/projection/z/p%d", i);
      newstr = (char*)malloc(strlen(tempstr)+1);
      strcpy(newstr, tempstr);
      AddMenuItem(menu,hoffset,voffset+i*SHORTBUTTONH,width,SHORTBUTTONH,newstr,
		  "",0,"",1,DEFAULTFONT,
		  MenuSetProjection2,BUTTON,NULL,NULL,NULL,"p2", istr);
      hoffset += (width+3);

      width = 15;

      sprintf(tempstr, "/controlmenu/projection/rand/p%d", i);
      newstr = (char*)malloc(strlen(tempstr)+1);
      strcpy(newstr, tempstr);
      AddMenuItem(menu,hoffset,voffset+i*SHORTBUTTONH,width,SHORTBUTTONH,newstr,
		  "",0,"r",1,DEFAULTFONT,
		  MenuRandomizeProjection,TOGGLE,NULL,NULL,NULL,"rand", istr);
    }

    voffset +=BUTTONH;
    return (voffset);
}

MainMenuClusterBlock(menu,voffset)
MenuWindow	*menu;
int		voffset;
{
  int i;
  int hoffset;
  char* gradestr;
  char *newstr;
  char tempstr[256];
  MenuItem *item;

    /*
    ** CLUSTER BOUND MODE
    */

    AddLabelString(menu, "Drawmode:",
	2, voffset+BUTTONH-6,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,menu->fontname);

    AddMenuItem(menu,60,voffset,65,BUTTONH,"/clustermenu/clustermode",
	"",0,"",1,DEFAULTFONT,
	MenuSetClusterMode,BUTTON,NULL,NULL,POLYGONSTRING,"mode", NULL);

    AddLabelString(menu, "Cluster scores:",
	132, voffset+BUTTONH-6,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,menu->fontname);

    /* buttons for cluster 'scores' */

    for(i = 1, hoffset = 212;
	i <= MAX_CLUST_SCORE; /* 5 + 1 for the 'X' button */
	i++, hoffset = hoffset + 18){

      gradestr = (char*)malloc(2);
      sprintf(gradestr,"%d",i);

      sprintf(tempstr,"/clustermenu/clusterscore%d",i);
      newstr = (char*)malloc((strlen(tempstr)+1)*sizeof(char));
      strcpy (newstr,tempstr);

      AddMenuItem(menu,hoffset,voffset,18,BUTTONH,newstr,
		  "",0,"",1,DEFAULTFONT,
		  MenuSetClusterScore,RADIO,NULL,NULL,gradestr,"score", NULL);
    }
    
    AddMenuItem(menu,hoffset,voffset,18,BUTTONH,"/clustermenu/unsetclusterscore",
		"X",0,"X",1,DEFAULTFONT,
		MenuUnsetClusterScore,BUTTON,NULL,NULL,NULL,NULL, NULL);

    voffset+=BUTTONH+5;

  /* show / hide / label clusters */

    AddMenuItem(menu,0,voffset,110,BUTTONH, "/clustermenu/copycluster",
	"Copy to cluster ",0,"Copy to cluster ",0,DEFAULTFONT,
	MenuCopyCluster,(DIALOG | INPUTAREA),NULL,NULL,NULL,NULL, NULL);

    AddMenuItem(menu,110,voffset,100,BUTTONH,"/clustermenu/showbounds",
        "Hide bounds",0,"Show bounds",1,DEFAULTFONT,
	MenuHideAllBounds,TOGGLE,NULL,NULL,NULL,NULL, NULL);

    AddMenuItem(menu,210,voffset,110,BUTTONH, "/clustermenu/deleteclusterbox",
	"Delete this bound",0,"Delete this bound",0,DEFAULTFONT,
	MenuDeleteClusterBounds,BUTTON,NULL,NULL,NULL,NULL, NULL);

   voffset+=BUTTONH;

    AddMenuItem(menu,0,voffset,110,BUTTONH,NULL,
        "Label clusters",0,"Label clusters",1,DEFAULTFONT,
	MenuLabelClusters,TOGGLE,NULL,NULL,NULL,NULL, NULL);

    AddMenuItem(menu,110,voffset,100,BUTTONH, "/clustermenu/hidecluster",
	"Hide cluster ",0,"Hide cluster ",1,DEFAULTFONT,
	MenuHideCluster,(DIALOG | TOGGLE),NULL,NULL,"0",NULL, NULL);

    AddMenuItem(menu,210,voffset,110,BUTTONH, "/clustermenu/deletecluster",
	"Delete Cluster",0,"Delete cluster?",0,DEFAULTFONT,
	MenuDeleteCluster,BUTTON,NULL,NULL,NULL,NULL, NULL);

    voffset+=BUTTONH;

    item = AddMenuItem(menu,0,voffset,110,BUTTONH, "/clustermenu/undoclusterchange",
	"  UNDO CHANGE",0,"  UNDO CHANGE",0,DEFAULTFONT,
	MenuUndoClusterChange,BUTTON,NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR + HIGHLIGHT1;

    AddMenuItem(menu,110,voffset,100,BUTTONH, "/clustermenu/showclusters",
	"Hide ALL clusters",0, "Show ALL clusters",1,DEFAULTFONT,
	MenuSetShowClusters,TOGGLE,NULL,NULL,NULL,NULL, NULL);

    AddMenuItem(menu,210,voffset,110,BUTTONH, "/clustermenu/deleteallclusters",
	"Delete ALL clusters",0, "Delete ALL clusters",0,DEFAULTFONT,
	MenuDeleteAllClusters,BUTTON,NULL,NULL,NULL,NULL, NULL);

 

/*     /\* */
/*     ** COLOR STATUS WINDOW */
/*     *\/ */
/*     AddMenuItem(menu,0,voffset+2*SHORTBUTTONH,20,SHORTBUTTONH,"/clustermenu/colorstat", */
/* 	NULL,0,NULL,1,DEFAULTFONT, */
/* 	MenuShowColorStat,TOGGLE,NULL,NULL,"M",NULL, NULL); */




/*     voffset+=(MAX(SHORTBUTTONH*PROJECTION_BLOCK_SIZE, */
/* 		  SHORTBUTTONH*CLUSTER_BLOCK_SIZE) */
/* 	      +7); */


/* cluster/bound deletion */










/*     AddMenuItem(menu,230,voffset,90,BUTTONH,"/clustermenu/clusterinfo", */
/* 	"Cluster Info",0,"Cluster Info",1,DEFAULTFONT, */
/* 	MenuShowClustStat,TOGGLE,NULL,NULL,NULL,NULL, NULL); */


    voffset+=(BUTTONH+5);

    AddMenuItem(menu,0,voffset,320,BUTTONH, "/clustermenu/readbounds",
	"Read bounds from: ",0,"Read bounds from: ",0,DEFAULTFONT,
	MenuReadClusterBounds,(DIALOG | INPUTAREA),NULL,NULL,"cbfile",NULL, NULL);
    voffset+=BUTTONH;
    AddMenuItem(menu,0,voffset,320,BUTTONH, "/clustermenu/writebounds",
	"Write bounds to: ",0,"Write bounds to: ",0,DEFAULTFONT,
	MenuWriteClusterBounds,(DIALOG | INPUTAREA),NULL,NULL,"cbfile",NULL, NULL);
    voffset+=BUTTONH;
    AddMenuItem(menu,0,voffset,320,BUTTONH, "/clustermenu/writeclusters",
	"Write Clusters to: ",0,"Write Clusters to: ",0,DEFAULTFONT,
	MenuWriteClusters,(DIALOG | INPUTAREA),NULL,NULL,"cl-",NULL, NULL);
/* #ifdef OLD */
/*     AddMenuItem(menu,0,voffset,320,BUTTONH, "/clustermenu/writetransfer", */
/* 	"Write Transfer file: ",0,"Write Transfer file: ",0,DEFAULTFONT, */
/* 	MenuWriteClusterIndices,DIALOG,NULL,NULL,"trans",NULL, NULL); */
/* #endif */
/*     voffset+=BUTTONH; */
/*     AddMenuItem(menu,0,voffset,160,BUTTONH, "/clustermenu/applytopoints", */
/* 	"Apply to loaded points",0,"Apply between start/end",1,DEFAULTFONT, */
/* 	NULL,TOGGLE,NULL,NULL,NULL,NULL, NULL); */


    voffset+=BUTTONH;
    return(voffset);
}

MainMenuAnalysisBlock(menu,voffset)
MenuWindow	*menu;
int		voffset;
{

  MenuItem *item;

    AddMenuItem(menu,0,voffset,35,BUTTONH,NULL,
	"CSI",0,"CSI",1,DEFAULTFONT,
	MenuCSI,BUTTON,NULL,NULL,NULL,NULL, NULL);
    AddMenuItem(menu,35,voffset,50,BUTTONH,NULL,
	"CSIALL",0,"CSI",1,DEFAULTFONT,
	MenuAllCSI,BUTTON,NULL,NULL,NULL,NULL, NULL);
    AddMenuItem(menu,85,voffset,70,BUTTONH, "/analysismenu/csimin",
	"Min: ",0, "Min: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"2",NULL, NULL);
    AddMenuItem(menu,155,voffset,70,BUTTONH, "/analysismenu/csimax",
	"Max: ",0, "Max: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"15",NULL, NULL);
    AddMenuItem(menu,225,voffset,70,BUTTONH, "/analysismenu/csbwin",
	"Bwin: ",0, "Bwin: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"100",NULL, NULL);
    AddMenuItem(menu,295,voffset,25,BUTTONH,"/analysismenu/csusedh",
	"dH",0,"dH",1,DEFAULTFONT,
	NULL,TOGGLE,NULL,NULL,NULL,NULL, NULL);

    voffset += BUTTONH + 5;

    item = AddMenuItem(menu,0,voffset,80,BUTTONH, "/analysismenu/evaloverlap",
	"EvalOverlap",0, "EvalOverlap",0,DEFAULTFONT,
	MenuEvalOverlap,BUTTON,NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR +  HIGHLIGHT1;

    item = AddMenuItem(menu,80,voffset, 240,BUTTONH, "/analysismenu/viewxcorr",
	"Crosscorr with: ",0, "Crosscorr with: ",0,DEFAULTFONT,
	MenuViewXcorr,(DIALOG | INPUTAREA),NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR + HIGHLIGHT1;

    voffset += BUTTONH;

    item = AddMenuItem(menu,0,voffset,80,BUTTONH, "/analysismenu/viewisi",
	"View LogISI",0, "View LogISI",0,DEFAULTFONT,
	MenuViewISI,BUTTON,NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR + HIGHLIGHT1;

    item = AddMenuItem(menu,80,voffset,80,BUTTONH, "/analysismenu/viewacorr",
	"View acorr",0, "View acorr",0,DEFAULTFONT,
	MenuViewAcorr,BUTTON,NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR + HIGHLIGHT1;

    item = AddMenuItem(menu,160,voffset,80,BUTTONH, "/analysismenu/viewrate",
	"View rate",0, "View rate",0,DEFAULTFONT,
	MenuViewRate,BUTTON,NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR + HIGHLIGHT1;

    item = AddMenuItem(menu,240,voffset,80,BUTTONH, "/analysismenu/viewavg",
	"View avg",0, "View avg",0,DEFAULTFONT,
	MenuViewAvg,BUTTON,NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR + HIGHLIGHT1;

    voffset+=BUTTONH;

    item = AddMenuItem(menu,0,voffset,80,BUTTONH, "/analysismenu/viewspikes",
	"View spikes",0, "View spikes",0,DEFAULTFONT,
	MenuViewSpikes,BUTTON,NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR + HIGHLIGHT1;

    item = AddMenuItem(menu,80,voffset,80,BUTTONH, "/analysismenu/viewbehav",
	"View behav",0, "View behav",0,DEFAULTFONT,
	MenuViewBehav,BUTTON,NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR + HIGHLIGHT1;

    item = AddMenuItem(menu,160,voffset,80,BUTTONH, "/analysismenu/viewspot",
	"View spot",0, "View spot",0,DEFAULTFONT,
	MenuViewSpot,BUTTON,NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR + HIGHLIGHT1;

    item = AddMenuItem(menu,240,voffset,80,BUTTONH, "/analysismenu/viewdir",
	"View dir",0, "View dir",0,DEFAULTFONT,
	MenuViewDir,BUTTON,NULL,NULL,NULL,NULL, NULL);
    item->itemforeground = MININTERFACECOLOR + HIGHLIGHT1;

    voffset += BUTTONH + 5;

    AddMenuItem(menu,0,voffset,120,BUTTONH, "/analysismenu/binsize",
	"Bin (msec): ",0, "Bin (msec): ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"10",NULL, NULL);
    AddMenuItem(menu,120,voffset,200,BUTTONH, "/analysismenu/tmax",
	"Tmax (msec): ",0, "Tmax (msec): ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"1000",NULL, NULL);

    voffset+=BUTTONH;

    AddMenuItem(menu,0,voffset,120,BUTTONH, "/analysismenu/tstart",
	"tstart: ",0, "tstart: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"0:0",NULL, NULL);
    AddMenuItem(menu,120,voffset,120,BUTTONH, "/analysismenu/tend",
	"tend: ",0, "tend: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"0:0",NULL, NULL);
    AddMenuItem(menu,240,voffset,80,BUTTONH, "/analysismenu/gridsize",
	"grid : ",0, "grid : ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"256",NULL, NULL);

    voffset+=BUTTONH + 5;

    AddMenuItem(menu,0,voffset,320,BUTTONH, "/analysismenu/spikefile",
	"Spike file: ",0, "Spike file: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"*.tt",NULL, NULL);

    voffset+=BUTTONH;

    AddMenuItem(menu,0,voffset,320,BUTTONH, "/analysismenu/positionfile",
	"Position file: ",0, "Position file: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"../*.p",NULL, NULL);

    voffset+=BUTTONH;

    AddMenuItem(menu,0,voffset,320,BUTTONH, "/analysismenu/trangefile",
	"Trange file: ",0, "Trange file: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"",NULL, NULL);

    voffset+=BUTTONH;

    AddMenuItem(menu,0,voffset,160,BUTTONH, "/analysismenu/autoprocess",
	"Autoprocess: ",0, "Autoprocess: ",0,DEFAULTFONT,
	MenuAutoprocess,(DIALOG | INPUTAREA),NULL,NULL,".",NULL, NULL);
    AddMenuItem(menu,160,voffset,160,BUTTONH, "/analysismenu/autocommand",
	"Command: ",0, "Command: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"tprocessall",NULL, NULL);


    voffset+=BUTTONH;

    return(voffset);
}

UpdateStatusTime()
{
  Plot *plot;
  DataSource *source;
  MenuWindow *menu;
  Label *label;
  char str_start[20], str_end[20];
  int s,e;

  plot = SelectedPlot(F->graph);
  if (plot == NULL)
    return;

  source = plot->source;
  if (source==NULL)
    return;

  menu = GetMenu("/epochmenu");

  label =  GetNamedLabel(menu, "epochstatus2");
  sprintf(label->u.string.line, "Time %11.4f-  %11.4f", source->starttimestamp/1e4, source->endtimestamp/1e4);


  if (source->startline<0)
    s = 0;
  else
    s = source->startline;

  if (source->endline<0)
    e = s + plot->npoints-1;
  else
    e = source->endline;
  label = GetNamedLabel(menu, "epochstatus3");
  strcpy(str_start, TimestampToString(source->starttimestamp));
  strcpy(str_end,TimestampToString(source->endtimestamp));
  sprintf(label->u.string.line, "Points %7d-%7d   %13.13s-%13.13s", s, e, str_start, str_end );

  RefreshMenu(menu);


}

UpdateStatusEpoch(epoch)
     char *epoch;
{
  MenuWindow *menu;
  Label *label;

  menu = GetMenu("/epochmenu");
  label = GetNamedLabel(menu, "epochstatus1");
  if (epoch == NULL || 
      strlen(epoch) == 0)
    sprintf(label->u.string.line, "Epoch %-10.10s", "" );
  else
    sprintf(label->u.string.line, "Epoch %-10.10s", epoch );

  RefreshMenu(menu);

}

/* SetEpochStatus(menu, epochname, pointstart, pointend, timestampstart, timestampend) */
/*      MenuWindow *menu; */
/*      char *epochname; */
/*      int pointstart; */
/*      int pointend; */
/*      int timestampstart; */
/*      int timestampend; */
/* { */
/*   Label *label; */
/*   char str_start[20], str_end[20]; */

/*   label = GetNamedLabel(menu, "epochstatus1"); */

/*   sprintf(label->u.string.line, "Epoch %-10.10s      Time %11.4f-  %11.4f", epochname,timestampstart/1e4, timestampend/1e4); */

/*   label = GetNamedLabel(menu, "epochstatus2"); */
/*   strcpy(str_start, TimestampToString(timestampstart)); */
/*   strcpy(str_end,TimestampToString(timestampend)); */
/*   sprintf(label->u.string.line, "Points %7d-%7d   %13.13s-%13.13s", pointstart, pointend, str_start, str_end ); */

/*   RefreshMenu(menu); */

/* } */

MainMenuEpochBlock(menu,voffset)
MenuWindow	*menu;
int		voffset;
{
  int i;
  int hoffset;
  Label *label;

    label = AddLabelString(menu, "",
	2, voffset+BUTTONH-6,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,FIXEDWIDTHFONT2);
    label->name = (char*)calloc(20,sizeof(char));
    sprintf(label->name, "epochstatus1");
    label = AddLabelString(menu, "",
	134, voffset+BUTTONH-6,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,FIXEDWIDTHFONT2);
    label->name = (char*)calloc(20,sizeof(char));
    sprintf(label->name, "epochstatus2");

    voffset += BUTTONH - 4;

    label = AddLabelString(menu, "",
	2, voffset+BUTTONH-6,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,FIXEDWIDTHFONT2);
    label->name = (char*)calloc(20,sizeof(char));
    sprintf(label->name, "epochstatus3");

    voffset += BUTTONH;

    AddLabelLine(menu, 0, voffset-2, voffset-2, 320, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
		 SCREEN_LBL, PERMANENT_LBL);
    AddLabelLine(menu, 0, voffset, voffset, 320, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
		 SCREEN_LBL, PERMANENT_LBL);


    voffset += 4;

    label = AddLabelString(menu, "Units:",
	2, voffset+BUTTONH-6,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,DEFAULTFONT);

    AddMenuItem(menu,40,voffset,40,BUTTONH,"/epochmenu/units_id",
	"id",0,"id",1,DEFAULTFONT,
	MenuChangeUnits,RADIO,NULL,NULL,NULL,"units", NULL);
    AddMenuItem(menu,80,voffset,40,BUTTONH,"/epochmenu/units_time",
	"time",0,"time",1,DEFAULTFONT,
	MenuChangeUnits,RADIO,NULL,NULL,NULL,"units", NULL);

    AddMenuItem(menu,200,voffset,120,BUTTONH,"/epochmenu/xautoscaletoggle",
	"Autoscale X on load",0,"Autoscale X on load",1,DEFAULTFONT,
	MenuToggleXAutoscale,TOGGLE,NULL,NULL,NULL,NULL, NULL);

    voffset += BUTTONH;

    AddMenuItem(menu,0,voffset,50,BUTTONH, "/epochmenu/loadallpoints",
	"Load All",0, "Load All",0,DEFAULTFONT,
	MenuLoadAllPoints,BUTTON,NULL,NULL,NULL,NULL, NULL);

    AddMenuItem(menu,50,voffset,50,BUTTONH, "/epochmenu/loadpoints",
	"Load pts",0, "Load pts",0,DEFAULTFONT,
	MenuLoadPoints,BUTTON,NULL,NULL,NULL,NULL, NULL);

    AddMenuItem(menu,100,voffset,110,BUTTONH, "/epochmenu/start",
	"start",0, "start",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"",NULL, NULL);
    AddMenuItem(menu,210,voffset,110,BUTTONH, "/epochmenu/end",
	"end",0, "end",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"",NULL, NULL);

    voffset += BUTTONH;

    AddMenuItem(menu,0,voffset,100,BUTTONH, "/epochmenu/rangefield",
	"Field: ",0, "Field: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,NULL,NULL, NULL);
    AddMenuItem(menu,100,voffset,110,BUTTONH, "/epochmenu/startrange",
	"RStart: ",0, "RStart: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,NULL,NULL, NULL);
    AddMenuItem(menu,210,voffset,110,BUTTONH, "/epochmenu/endrange",
	"REnd: ",0, "REnd: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,NULL,NULL, NULL);    

    voffset+=BUTTONH + 5;

    hoffset = 0;

    AddMenuItem(menu,hoffset,voffset,20,BUTTONH, "/epochmenu/blockmoviebackward",
	NULL,0,NULL,0,DEFAULTFONT,
	MenuBlockMovie,(RADIO | TOGGLE),
	"fbackward","ifbackward","-1","blockmovie", NULL);
    hoffset+=20;
    AddMenuItem(menu,hoffset,voffset,18,BUTTONH, "/epochmenu/prevblock",
	NULL,0,NULL,0,DEFAULTFONT,
	MenuMovieFrame,BUTTON,"backward","backward","-1",NULL, NULL);
    hoffset+=18;
    AddMenuItem(menu,hoffset,voffset,18,BUTTONH, "/epochmenu/nextblock",
	NULL,0,NULL,0,DEFAULTFONT,
	MenuMovieFrame,BUTTON,"forward","forward","1",NULL, NULL);
    hoffset+=18;
    AddMenuItem(menu,hoffset,voffset,20,BUTTONH, "/epochmenu/blockmovieforward",
	NULL,0,NULL,0,DEFAULTFONT,
	MenuBlockMovie,(RADIO | TOGGLE),
	"fforward","ifforward","1","blockmovie", NULL);
    hoffset+=20;

    AddMenuItem(menu,hoffset,voffset,122,BUTTONH, "/epochmenu/blocksize",
	"blocksize",0, "blocksize",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"",NULL, NULL);
    hoffset+=122;
    AddMenuItem(menu,hoffset,voffset,122,BUTTONH, "/epochmenu/stepsize",
	"step (%)",0, "step (%)",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"",NULL, NULL);

    voffset += BUTTONH;

    AddMenuItem(menu,0,voffset,35,BUTTONH,"/epochmenu/looptoggle",
	"Loop",0,"Loop",1,DEFAULTFONT,
	NULL,TOGGLE,NULL,NULL,NULL,NULL, NULL);
    AddMenuItem(menu,35,voffset,65,BUTTONH, "/epochmenu/delay",
	"delay",0, "delay",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"1",NULL, NULL);
    AddMenuItem(menu,100,voffset,110,BUTTONH, "/epochmenu/moviestart",
	"start",0, "start",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"",NULL, NULL);
    AddMenuItem(menu,210,voffset,110,BUTTONH, "/epochmenu/movieend",
	"end",0, "end",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,"",NULL, NULL);

    voffset += BUTTONH + 5;


    AddMenuItem(menu,0,voffset,280,BUTTONH, "/epochmenu/epochfile",
	"Load epoch file: ",0, "Load epoch file: ",0,DEFAULTFONT,
	MenuLoadEpochFile,(DIALOG | INPUTAREA),NULL,NULL,"../*.epoch",NULL, NULL);
    AddMenuItem(menu,280,voffset,40 ,BUTTONH*2, "/epochmenu/newepoch",
	" +E ",0, " +E ",0,DEFAULTFONT,
	MenuDefineNewEpoch,BUTTON,NULL,NULL,NULL,NULL, NULL);
    voffset+=BUTTONH;

    AddMenuItem(menu,0,voffset,280 ,BUTTONH, "/epochmenu/saveepochs",
	"Save epoch file: ",0, "Save epoch file: ",0,DEFAULTFONT,
	MenuSaveEpochFile,(DIALOG | INPUTAREA),NULL,NULL,"../",NULL, NULL);
    voffset+=BUTTONH;

    /* add epoch buttons */
    /* (for backwards compatibility with people's .xclustrc files,
    ** these are 1-indexed) */
    for(i = 1;
	i <= NUM_EPOCHS; /* number of epochs */
	i++, voffset = voffset + BUTTONH){
      AddMenuEpoch(menu,i,voffset);
    }

/*     voffset += 2*BUTTONH; */

/*     AddMenuItem(menu,0,voffset,100,BUTTONH, "/epochmenu/epochname", */
/* 	"Epoch: ",0, "Epoch: ",0,DEFAULTFONT, */
/* 	NULL,(DIALOG | INPUTAREA),NULL,NULL,NULL,NULL, NULL); */
/*     AddMenuItem(menu,100,voffset,110,BUTTONH, "/epochmenu/tstartspike", */
/* 	"T |< ",0, "T |< ",0,DEFAULTFONT, */
/* 	MenuEnterTstart,(DIALOG | INPUTAREA),NULL,NULL,"",NULL, NULL); */
/*     AddMenuItem(menu,210,voffset,110,BUTTONH, "/epochmenu/tendspike", */
/* 	"T >| ",0, "T >| ",0,DEFAULTFONT, */
/* 	MenuEnterTend,(DIALOG | INPUTAREA),NULL,NULL,"",NULL, NULL); */
/*     voffset+=BUTTONH; */


/*     hoffset = 0; */
/*     AddMenuItem(menu,hoffset,voffset,51,BUTTONH, "/epochmenu/loadpoints", */
/* 	"Load pts",0, "Load pts",0,DEFAULTFONT, */
/* 	MenuLoadPoints,BUTTON,NULL,NULL,NULL,NULL, NULL); */
/*     hoffset+=51; */
/*     AddMenuItem(menu,hoffset,voffset,89,BUTTONH, "/epochmenu/startspike", */
/* 	"Start: ",0, "Start: ",0,DEFAULTFONT, */
/* 	MenuEnterStart,(DIALOG | INPUTAREA),NULL,NULL,NULL,NULL, NULL); */
/*     hoffset+=89; */
/*     AddMenuItem(menu,hoffset,voffset,89,BUTTONH, "/epochmenu/endspike", */
/* 	"End: ",0, "End: ",0,DEFAULTFONT, */
/* 	MenuEnterEnd,(DIALOG | INPUTAREA),NULL,NULL,NULL,NULL, NULL); */
/*     hoffset+=89; */
/* #ifdef OLD */
/*     AddMenuItem(menu,100,voffset,80,BUTTONH, "/epochmenu/nextblock", */
/* 	"Next block",0, "Next block",0,DEFAULTFONT, */
/* 	MenuNextBlock,BUTTON,NULL,NULL,NULL,NULL, NULL); */
/*     AddMenuItem(menu,180,voffset,80,BUTTONH, "/epochmenu/prevblock", */
/* 	"Prev block",0, "Prev block",0,DEFAULTFONT, */
/* 	MenuPrevBlock,BUTTON,NULL,NULL,NULL,NULL, NULL); */
/* #endif */



    voffset+=BUTTONH;

    return(voffset);
}




static int menupos = -1;

MenuScrollUp(item)
MenuItem	*item;
{
int	scrollsize;

    if(item->value){
	scrollsize = atoi(item->value);
    }
    menupos -= atoi(item->value);
    MoveMenuWindow(menupos);
}

MenuScrollDown(item)
MenuItem	*item;
{
int	scrollsize;

    if(item->value){
	scrollsize = atoi(item->value);
    }
    menupos += scrollsize;
    if(menupos > 0) menupos = 0;
    MoveMenuWindow(menupos);
}

CreateTextMenu(menu)
MenuWindow	*menu;
{

    /*
    ** CLUSTER BLOCK UP/DOWN MODE
    */
    AddMenuItem(menu,menu->wwidth-20,0,20,20, "/text/scrollup",
	NULL,0,NULL,1,DEFAULTFONT,
	MenuScrollDown,BUTTON,"uparrow",NULL,"100",NULL, NULL);
    AddMenuItem(menu,menu->wwidth-40,0,20,20, "/text/scrolldown",
	NULL,0,NULL,1,DEFAULTFONT,
	MenuScrollUp,BUTTON,"dnarrow",NULL,"100",NULL, NULL);
}

SetItemEnable(itemname,enable)
char	*itemname;
int	enable;
{
MenuItem	*item;

    if((item = LookupMenuItem(itemname)) == NULL) {
	return(0);
    }
    item->enabled = enable;
    return(1);
}

AddMenuCluster(menu,clusternum,x,y)
MenuWindow	*menu;
int	clusternum;
int	x;
int	y;
{
char	line[100];
char	*name;
char	*cnum;
MenuItem	*item;

extern int MenuSetCluster();

    sprintf(line,"%d",clusternum);
    cnum = (char *)malloc(strlen(line) + 1);
    strcpy(cnum,line);

    sprintf(line,"/controlmenu/cl%d",clusternum);
    name = (char *)malloc(strlen(line) + 1);
    strcpy(name,line);
    item = AddMenuItem(menu,x,y,40,SHORTBUTTONH,name,
	"C",0,"C",1,DEFAULTFONT,
	MenuSetCluster,RADIO,NULL,NULL,cnum,"cluster", NULL);

    item->itembackground = BLACK;

    sprintf(line,"/controlmenu/clenable%d",clusternum);
    name = (char *)malloc(strlen(line) + 1);
    strcpy(name,line);

    /* note that the bitmaps are set in clust_menu for these 3 buttons */

    item = AddMenuItem(menu,x+40,y,20,SHORTBUTTONH,name,
	NULL,0,NULL,1,DEFAULTFONT,
	MenuEnableCluster,TOGGLE,NULL,NULL,cnum,NULL, NULL);

    item->itembackground = BLACK;


    sprintf(line,"/controlmenu/clhide%d",clusternum);
    name = (char *)malloc(strlen(line) + 1);
    strcpy(name,line);
    item = AddMenuItem(menu,x+60,y,20,SHORTBUTTONH,name,
	NULL,0,NULL,1,DEFAULTFONT,
	MenuButtonHideCluster,TOGGLE,NULL,NULL,cnum,NULL, NULL);

    item->itembackground = BLACK;


    sprintf(line,"/controlmenu/clshowonly%d",clusternum);
    name = (char *)malloc(strlen(line) + 1);
    strcpy(name,line);
    item = AddMenuItem(menu,x+80,y,20,SHORTBUTTONH,name,
	NULL,0,NULL,1,DEFAULTFONT,
	MenuButtonShowOnlyCluster,TOGGLE,NULL,NULL,cnum,NULL, NULL);

    item->itembackground = BLACK;


    sprintf(line,"/controlmenu/clmask%d",clusternum);
    name = (char *)malloc(strlen(line) + 1);
    strcpy(name,line);
    item = AddMenuItem(menu,x+40,y,60,SHORTBUTTONH,name,
	NULL,0,NULL,0,DEFAULTFONT,
	NULL,BUTTON,NULL,NULL,NULL,NULL, NULL);

    item->itembackground = BLACK;

    SetItemEnable(line,0);
}


AddMenuEpoch(menu,epochnum,y)
MenuWindow	*menu;
int	epochnum;
int	y;
{
char	*epochname;

    epochname = (char *)malloc(40);
    sprintf(epochname,"/epochmenu/epochkill/%d",epochnum);
    AddMenuItem(menu,0,y,15,BUTTONH,epochname,
	"X",0, "X",0,DEFAULTFONT,
	MenuEpochKill,BUTTON,NULL,NULL,NULL,"ekill", NULL);

    epochname = (char *)malloc(40);
    sprintf(epochname,"/epochmenu/epoch/%d",epochnum);
    AddMenuItem(menu,15,y,85,BUTTONH,epochname,
	"Epoch: ",0, "Epoch: ",0,DEFAULTFONT,
	MenuEpochName,(DIALOG | INPUTAREA),NULL,NULL,NULL,"ename", NULL);

    epochname = (char *)malloc(40);
    sprintf(epochname,"/epochmenu/epochstart/%d",epochnum);
    AddMenuItem(menu,100,y,110,BUTTONH, epochname,
	"Tstart: ",0, "Tstart: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,NULL,"etime", NULL);

    epochname = (char *)malloc(40);
    sprintf(epochname,"/epochmenu/epochend/%d",epochnum);
    AddMenuItem(menu,210,y,110,BUTTONH, epochname,
	"Tend: ",0, "Tend: ",0,DEFAULTFONT,
	NULL,(DIALOG | INPUTAREA),NULL,NULL,NULL,"etime", NULL);
}

int CallMenuItem(itemname)
char	*itemname;
{
MenuItem	*item;
XButtonPressedEvent	event;

    if(itemname == NULL){
	fprintf(stderr,"ERROR: invalid itemname\n");
	return(0);
    }
    /*
    ** lookup the menu item
    */
    if((item = LookupMenuItem(itemname)) == NULL){
	fprintf(stderr,"ERROR: unable to find item '%s'\n",itemname);
	return(0);
    }
    event.type = ButtonPress;
    event.button = 1;
    item_event(item,&event);
    return(1);
}


int PutItemValue(itemname,value)
char	*itemname;
char	*value;
{
MenuItem	*item;

    /*
    ** lookup the item
    */
    item = LookupMenuItem(itemname);
    if(item == NULL){
	fprintf(stderr,"unable to find menu item '%s'\n",itemname);
	return(0);
    }
    item->value = CopyString(value);
    return(1);
}

char *GetItemValue(itemname)
char	*itemname;
{
MenuItem	*item;

    item = LookupMenuItem(itemname);
    if(item == NULL){
	fprintf(stderr,"unable to find menu item '%s'\n",itemname);
	return(NULL);
    }
    return(item->value);
}

int GetItemState(itemname)
char	*itemname;
{
MenuItem	*item;

    item = LookupMenuItem(itemname);
    if(item == NULL){
	fprintf(stderr,"unable to find menu item '%s'\n",itemname);
	return(0);
    }
    return(item->state);
}


InitMenu(menucontainer)
MenuContainer	*menucontainer;
{
MenuItem	*item;
ProjectionInfo	*pinfo;
char		itemname[100];
int		i;
 MenuWindow *menu;
 char startid[100], endid[100];
 Plot *p;
 int s, e;
 int start, end;

    /*
    ** attempt to initialize the projection button names
    */
    for(i=0;i<15;i++){
	sprintf(itemname,"/controlmenu/projection/name/p%d",i);
	if(item = LookupMenuItem(itemname)){
	    if(pinfo = GetProjectionInfo(menucontainer->frame->graph,i)){
		item->string = pinfo->name;
		item->string2 = pinfo->name;
		item->value = NULL;
	    } else {
		item->string = "";
		item->string2 = "";
	    }
	}
    }

    /*
    ** select the default projection buttons
    */

    F->graph->selectedprojection[0] = -1;
    F->graph->currentprojection[0] = 0;

    F->graph->selectedprojection[1] = -1;
    F->graph->currentprojection[1] = 1;

    F->graph->selectedprojection[2] = -1;
    F->graph->currentprojection[2] = 2;

    AssignProjectionButtons();

/*     CallMenuItem("/controlmenu/projection/x/p0"); */
/*     CallMenuItem("/controlmenu/projection/y/p1"); */
/*     CallMenuItem("/controlmenu/projection/z/p2"); */


    /*
    ** Select the default cluster 1 
    */
    CallMenuItem("/controlmenu/cl1");

    /*
    ** toggle the dH button
    */
    CallMenuItem("/analysismenu/csusedh");
    CallMenuItem("/epochmenu/units_id");

 /* disable unused menu items */
/*     SetItemEnable("/controlmenu/randx",0); */
/*     SetItemEnable("/controlmenu/randy",0); */
/*     SetItemEnable("/controlmenu/resetfile",0); */
/*     SetItemEnable("/controlmenu/deleteplot",0); */
/*     SetItemEnable("/clustermenu/clusterinfo",0); */
/*     SetItemEnable("/clustermenu/colorstat",0); */

/*     menu = GetMenu("/epochmenu"); */
/*     p = SelectedPlot(F->graph); */
/*     start = p->source->startline; */
/*     end = p->source->endline; */
/*     if (start<0) */
/* 	  start = 0; */
/*     if (end <0) */
/*       end = start + p->npoints-1; */
/*     sprintf(startid, "%d", start); */
/*     sprintf(endid, "%d", end); */
/*     s = Atoi(LookupSpikeTime(startid)); */
/*     e = Atoi(LookupSpikeTime(endid)); */

    UpdateStatusTime();
    UpdateStatusEpoch("");
}

