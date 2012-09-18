#include	"xclust_ext.h"
#include 	<X11/cursorfont.h>
#include 	<X11/Xutil.h>

XContext	datacontext;

Graph	*GetGraph(name)
     char	*name;
{
  Graph	*graph;
  
  /*
   ** lookup the graph 
   */
  for(graph=F->graph;graph;graph=graph->next){
    if(strcmp(graph->windowname,name) == 0){
      return(graph);
    }
  }
  return(NULL);
}

MapMenu(val)
     int val;
{
  int menu_width;
  
  menu_width = 10*F->fontwidth;
  F->menucontainer->mapped = val;
  if(val == 0){
    XUnmapWindow(F->menucontainer->display,F->menucontainer->window);
    /*
     ** adjust the frame window 
     */
    XResizeWindow(F->display,F->window, F->wwidth - menu_width,F->wheight);
  } else {
    /*
     ** adjust the frame window 
     */
    XResizeWindow(F->display,F->window, F->wwidth + menu_width,F->wheight);
    /*
     ** add the menu window 
     */
    F->menucontainer->wwidth = menu_width;
    F->menucontainer->wheight = F->wheight;
    XMoveResizeWindow(F->display,F->menucontainer->window,F->graph->wwidth,-1,
		      F->menucontainer->wwidth,F->menucontainer->wheight);
  }
  MapWindows(F);
}

NormalSelectInputs(graph)
     Graph *graph;
{
  XSelectInput(graph->display,graph->window, 
	       KeyPressMask | KeyReleaseMask | ExposureMask | ButtonPressMask | ButtonReleaseMask
	       | Button1MotionMask | Button2MotionMask | Button3MotionMask | PointerMotionMask );
}

TextSelectInputs(graph)
     Graph *graph;
{
  XSelectInput(graph->display,graph->window, 
	       KeyPressMask | ExposureMask | PointerMotionMask );
}

ClickSelectInputs(graph)
     Graph *graph;
{
  XSelectInput(graph->display,graph->window, 
	       KeyPressMask | ExposureMask | PointerMotionMask |
	       ButtonPressMask | ButtonReleaseMask );
}

InteractiveSelectInputs(graph)
     Graph *graph;
{
  XSelectInput(graph->display,graph->window, 
	       KeyPressMask |  ExposureMask | ButtonMotionMask |
	       ButtonPressMask | ButtonReleaseMask |PointerMotionHintMask  );
}

WaitForKeyReleaseInputs(graph)
     Graph *graph;
{
  XSelectInput(graph->display,graph->window, 
	       KeyReleaseMask | LeaveWindowMask | ExposureMask | ButtonPressMask |
	       ButtonReleaseMask );
}


ChangeGeometry(geometry,frame)
     char    *geometry;
     Frame	*frame;
{
  int	status,x,y,width,height;
  XSizeHints hint;
  
  hint.flags = 0;
  status = XParseGeometry(geometry,&x,&y,&width,&height);
  if(status & XValue){
    frame->x = x;
    hint.flags |= PPosition;
    hint.x = x;
  }
  if(status & YValue){
    frame->y = y;
    hint.flags |= PPosition;
    hint.y = y;
  }
  if(status & WidthValue){
    frame->width = width;
    /*
      frame->wwidth = width;
      */
    hint.flags |= PSize;
    hint.width = width;
  }
  if(status & HeightValue){
    frame->height = height;
    /*
      frame->wheight = height;
      */
    hint.flags |= PSize;
    hint.height = height;
  }
  if(frame->display){
    XMoveResizeWindow(frame->display,frame->window,frame->x,frame->y,
		      frame->width,frame->height);
  }
}

SetInverse(basic,fore,back)
     BasicWindow *basic;
     int	fore,back;
{
  if(basic->inverse){
    SetPSInverse(0);
	if(back != -1){
	  SetBackground(basic,back);
	} else {
	  SetBackground(basic,WhitePixelIdx());
	}
    if(fore != -1){
      SetForeground(basic,fore);
    } else {
      SetForeground(basic,BlackPixelIdx());
    }
  } else {
    SetPSInverse(1);
    if(back != -1){
      SetForeground(basic,back);
    } else {
      SetForeground(basic,WhitePixelIdx());
    }
    if(fore != -1){
      SetBackground(basic,fore);
    } else {
      SetBackground(basic,BlackPixelIdx());
    }
  }
}

InitGraph(graph)
     Graph *graph;
{
  Plot	*plot;
  DataSource	*source;
  extern  char *restorefile;
  
  /*
   ** make sure there are plots. If not then exit
   */
  if(!graph->plot){
    /*
     ** are there any commands to execute?
     */
    if(CommandStr()){
      /*
       ** tack on a plot
       */
      source = AddSource(graph);
      graph->plot = AddPlot(graph,source);
      graph->plot->selected = 1;
      ExecuteCommands(graph);
      ClearCommands();
    } else {
      Quit();
    }
  }

  /* set default rotpointfreq (should be set by user in .xclust3rc with
  ** /rotpointfreq or /rotpointsmax)*/

  InterpretCommand(graph,SelectedPlot(G), "/rotpointsmax 100000");

  /*   graph->rotpointfreq = 25; */

  /*
   ** setup the default linestyle of the plots
   */
  for(plot=graph->plot;plot;plot=plot->next){
    if(plot->linestyle == -1){
      plot->linestyle = MININTERFACECOLOR + ZEROCLUSTER; /*graph->foreground;*/
    }
  }
  if(graph->plot){
    /*
     ** select the first plot
     */
    graph->plot->selected = 1;
  }
  if(!restorefile){
    /*
     ** determine the world coordinate boundaries of the window and
     ** autoscaling the data if necessary
     */
    AutoScale(graph,graph->xaxis.autoscale,graph->yaxis.autoscale);
  }
}

CopyWindowStruct(src,dst)
     BasicWindow *src,*dst;
{
  bcopy(src,dst,sizeof(SharedWindow));
  /*
  */
  dst->context = XCreateGC(dst->display,dst->window,0,0);
  XCopyGC(src->display,src->context,~0,dst->context); 
}

MapWindows(frame)
Frame *frame;
{
  MenuFrame *menuframe;

    if(frame->mapped){
	XMapWindow(frame->display,frame->window);
    }
    if(frame->graph->mapped){
	XMapWindow(frame->graph->display,frame->graph->window);
    }
    if (frame->menucontainer->mapped){
      XMapWindow(frame->menucontainer->display, frame->menucontainer->window);
      /* loop through menu frames */
      for (menuframe = frame->menucontainer->menuframe; menuframe; menuframe=menuframe->next) {
	if (menuframe->mapped) {
	  XMapWindow(menuframe->display, menuframe->window);
	}
	if (menuframe->menu->mapped) {
	  XMapWindow(menuframe->menu->display, menuframe->menu->window);
	}
      }
    }
/*     if(frame->menu->mapped){ */
/* 	XMapWindow(frame->menu->display,frame->menu->window); */
/*     } */
    if(frame->text->mapped){
	XMapWindow(frame->text->display,frame->text->window);
    }
/*     if(frame->cluststat->mapped){ */
/* 	XMapWindow(frame->cluststat->display,frame->cluststat->window); */
/*     } */
/*     if(frame->colorstat->mapped){ */
/* 	XMapWindow(frame->colorstat->display,frame->colorstat->window); */
/*     } */
}

InitX(frame)
     Frame *frame;
{
  char *str;
  char *XGetDefault();
  char *CopyString();
  XVisualInfo	visual,*visuallist;
  int	visualsmatched;
  
  /*
   ** open the display
   */
  frame->display = XOpenDisplay(frame->display_name);
  if (frame->display == NULL) {
    fprintf(stderr,"Unable to open X display [%s]\n", frame->display_name);
    exit(1);
  }
#ifdef DEBUG
  /*
   ** force all calls to complete before returning
   ** this synchronizes errors with the routines that invoked them
   */
  XSynchronize(frame->display,1);
#endif
  if(frame->fontname == NULL){
    str = XGetDefault(frame->display,"xclust","GraphFont");
    if(str == NULL){
      frame->fontname = DEFAULTFONT;
    } else {
      frame->fontname = CopyString(str);
    }
  }
  if(frame->inverse == -1){
    str = XGetDefault(frame->display,"xclust","Inverse");
    if(str == NULL){
      frame->inverse = 0;
    } else {
      if(strcmp(str,"yes") == 0){
	frame->inverse = 1;
      } else {
	frame->inverse = 0;
      }
    }
  }
  
  /*
   ** use the default screen
   */
  frame->screen_number = XDefaultScreen(frame->display);
  
  /*
   ** use the default visual
   */
   /*
   visual.depth = 8;
   visual.screen = frame->screen_number;
   visuallist = XGetVisualInfo(frame->display,VisualScreenMask|VisualDepthMask,
   &visual,&visualsmatched);
   fprintf(stderr,"found %d matching visuals\n",visualsmatched);
   frame->visual = visuallist[0].visual;
  */
  frame->visual = XDefaultVisual(frame->display,frame->screen_number);
  
  /*
   ** use the default context
  frame->context = XDefaultGC(frame->display,frame->screen_number);
   */
  
  /*
   ** determine whether or not color can be displayed
   */
  if (XDisplayPlanes(frame->display,frame->screen_number) < 2){
    frame->color_mode = 0;
  }
  /*
  SetFont(frame,frame->fontname);
  */
  /*
    ** set the foreground and backgrounds for the windows
    */
  frame->foreground = MININTERFACECOLOR + FRAMEFOREGROUND;
  frame->background = MININTERFACECOLOR + FRAMEBACKGROUND;
  frame->graph->background = MININTERFACECOLOR + GRAPHBACKGROUND;
  frame->graph->foreground = MININTERFACECOLOR + GRAPHFOREGROUND;

  frame->menucontainer->background = MININTERFACECOLOR + MENUBACKGROUND;
  frame->menucontainer->foreground = MININTERFACECOLOR + MENUFOREGROUND;

/*   frame->menu->background = MINCOLOR + MENUBACKGROUND; */
/*   frame->menu->foreground = MINCOLOR + MENUFOREGROUND; */
/*   frame->menu->itembackground = MINCOLOR + MENUITEMBACKGROUND; */
/*   frame->menu->itemforeground = MINCOLOR + MENUITEMFOREGROUND; */

/*   frame->cluststat->background = MINCOLOR + CLUSTSTATBACKGROUND; */
/*   frame->cluststat->foreground = MINCOLOR + CLUSTSTATFOREGROUND; */
/*   frame->colorstat->background = MINCOLOR + CLUSTSTATBACKGROUND; */
/*   frame->colorstat->foreground = MINCOLOR + CLUSTSTATFOREGROUND; */

  frame->text->background = MININTERFACECOLOR + TEXTBACKGROUND;
  frame->text->foreground = MININTERFACECOLOR + TEXTFOREGROUND;
  frame->text->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
  frame->text->itemforeground = MININTERFACECOLOR + MENUITEMFOREGROUND;

/*
  frame->foreground = WhitePixelIdx();
  frame->background = BlackPixelIdx();
    */
  /*
   ** create the windows
   */
  CreateTopFrame(frame);
  frame->context = XCreateGC(frame->display,frame->window,0,0);
  SetFont(frame,frame->fontname);
  /*
   ** create the colormap
   */
  if(frame->color_mode){
    MakeColormap(frame);
    SetFrameColormap();
  }
  CreateWindows(frame);
  

  NormalCursor(frame);
  
  /*
   ** make sure it all got to the server
   */
  XFlush(frame->display);
}

TargetCursor(frame)
Frame *frame;
{
Cursor cursor;
    /*
    ** select the cursor
    */
    cursor = XCreateFontCursor(frame->display,XC_target);
    XDefineCursor(frame->display,frame->window,cursor);
}

NormalCursor(frame)
Frame *frame;
{
Cursor cursor;
    /*
    ** select the cursor
    */
    cursor = XCreateFontCursor(frame->display,XC_left_ptr);
    XDefineCursor(frame->display,frame->window,cursor);
}

TextCursor(frame)
Frame *frame;
{
Cursor cursor;
    /*
    ** select the cursor
    */
    cursor = XCreateFontCursor(frame->display,XC_pencil);
    XDefineCursor(frame->display,frame->window,cursor);
}

HandCursor(frame)
Frame *frame;
{
Cursor cursor;
    /*
    ** select the cursor
    */
    cursor = XCreateFontCursor(frame->display,XC_hand2);
    XDefineCursor(frame->display,frame->window,cursor);
}

CrosshairCursor(frame)
Frame *frame;
{
Cursor cursor;
    /*
    ** select the cursor
    */
    cursor = XCreateFontCursor(frame->display,XC_crosshair);
    XDefineCursor(frame->display,frame->window,cursor);
}

FleurCursor(frame)
Frame *frame;
{
Cursor cursor;
    /*
    ** select the cursor
    */
    cursor = XCreateFontCursor(frame->display,XC_fleur);
    XDefineCursor(frame->display,frame->window,cursor);
}

RenameWindow(s)
char *s;
{
XSizeHints		hints;

    if(s == NULL) return;
    if(!F->mapped) return;
    hints.flags = 0;
    XSetStandardProperties(F->display,F->window,
	s,		/* window name */
	s,		/* icon name */
	None,			/* icon pixmap */
	NULL,			/* command argv */
	0,			/* command argc */
	&hints			/* window sizing hints */
	);
}

CreateTopFrame(frame)
     Frame		*frame;
{
  int	width;
  int	height;
  int	x;
  int	y;
  unsigned long 		border;
  unsigned long 		background;
  XWindowAttributes	winfo;
  XSetWindowAttributes	attrib;
  Window			root_window;
  int			borderwidth;
  XSizeHints		hints;
  WindowData		*windowdata;
  Graph			*graph;
  TextWindow		*text;
  MenuWindow		*menu;
  MenuWindow		*cluststat;
  MenuWindow		*colorstat;
  int			status = 0;
  extern			int frame_event();
  extern			int graph_event();
  extern			int text_event();
  extern			int menu_event();
  extern			int cluststat_event();
  extern			int colorstat_event();
  char			*windowtitle;
  char			*str;
 
  /*
   ** get the root window and its attributes
   */
  root_window = XRootWindow(frame->display,frame->screen_number);
  XGetWindowAttributes(frame->display,root_window,&winfo);
  if(frame->x == -1){
    frame->x = winfo.width / 20;
    status = 1;
  }
  if(frame->y == -1){
    frame->y = winfo.height / 20;
    status = 1;
  }
  if(frame->width == -1){
    frame->width = winfo.height / 4;
    frame->width = 800;
    if(frame->menucontainer->mapped){
      frame->width += MENU_WIDTH;
    }
    status = 1;
  }
  if(frame->height == -1){
    frame->height = winfo.height / 4;
    frame->height = 870;
    status = 1;
  }
  
  height = frame->height;
  width = frame->width;
  x = frame->x;
  y = frame->y;
  borderwidth = 1;
  border = frame->foreground;
  background = frame->background;

  border = WhitePixel(frame->display,frame->screen_number);
  background = BlackPixel(frame->display,frame->screen_number);
  
  /*
   ** 			TOP-LEVEL FRAME WINDOW
   */
  frame->inverse = 0;
  windowdata = (WindowData *)calloc(1,sizeof(WindowData));
  windowdata->id = FRAME_WINDOW;
  windowdata->parent = (BasicWindow *)frame;
  windowdata->func = frame_event;
  frame->windowname = "/";
/*
  frame->window = (Window)XCreateWindow(frame->display,root_window,
      x,y,width,height,1,
      8,InputOutput,
      frame->visual,0,NULL);
					      */
  frame->window = (Window)XCreateSimpleWindow(frame->display,root_window,
					      x,y,width,height,1,border,background);
  XSaveContext(frame->display,frame->window,datacontext,(XPointer) windowdata);
  frame->drawable = frame->window;
  XSelectInput(frame->display,frame->window, 
	       StructureNotifyMask);
  /*
   ** set the standard properties of the window
   */
  if(!status){
    hints.flags = PSize | PPosition | USSize | USPosition;
  } else {
    hints.flags = 0L;
  }
  hints.x = x;
  hints.y = y;
  hints.height = height;
  hints.width = width;
  if(frame->graph->title){
    windowtitle = frame->graph->title;
  } else {
    windowtitle = "XCLUST3";
  }
  XSetStandardProperties(frame->display,frame->window,
			 windowtitle,		/* window name */
			 windowtitle,		/* icon name */
			 None,			/* icon pixmap */
			 NULL,			/* command argv */
			 0,			/* command argc */
			 &hints			/* window sizing hints */
			 );
} 

CreateWindows(frame)
     Frame		*frame;
{
  int	width;
  int	height;
  int	x;
  int	y;
  unsigned long 		border;
  unsigned long 		background;
  XWindowAttributes	winfo;
  XSetWindowAttributes	attrib;
  Window			root_window;
  int			borderwidth;
  XSizeHints		hints;
  WindowData		*windowdata;
  Graph			*graph;
  TextWindow		*text;
  MenuContainer *menucontainer;
  MenuWindow		*menu;
  MenuWindow		*cluststat;
  MenuWindow		*colorstat;
  int			status = 0;
  extern			int frame_event();
  extern			int graph_event();
  extern			int text_event();
  extern			int menucontainer_event();
  extern			int menu_event();
  extern			int cluststat_event();
  extern			int colorstat_event();
  char			*windowtitle;
  char			*str;
  MenuItem		*item;
 
  graph = frame->graph;
/*   menu = frame->menu; */
  menucontainer = frame->menucontainer;
/*   cluststat = frame->cluststat; */
/*   colorstat = frame->colorstat; */
  text = frame->text;
  /*
   ** get the root window and its attributes
   */
  root_window = XRootWindow(frame->display,frame->screen_number);
  XGetWindowAttributes(frame->display,root_window,&winfo);
  if(frame->x == -1){
    frame->x = winfo.width / 20;
    status = 1;
  }
  if(frame->y == -1){
    frame->y = winfo.height / 20;
    status = 1;
  }
  if(frame->width == -1){
    frame->width = winfo.height / 4;
    frame->width = 800;
    if(frame->menucontainer->mapped){
      frame->width += MENU_WIDTH;
    }
    status = 1;
  }
  if(frame->height == -1){
    frame->height = winfo.height / 4;
    frame->height = 870;
    status = 1;
  }
  
  height = frame->height;
  width = frame->width;
  x = frame->x;
  y = frame->y;
  borderwidth = 1;
  border = frame->foreground;
  background = frame->background;
  
  /*
   ** set the standard properties of the window
   */
  if(!status){
    hints.flags = PSize | PPosition | USSize | USPosition;
  } else {
    hints.flags = 0L;
  }
  hints.x = x;
  hints.y = y;
  hints.height = height;
  hints.width = width;
  if(graph->title){
    windowtitle = graph->title;
  } else {
    windowtitle = "XCLUST3";
  }
  XSetStandardProperties(frame->display,frame->window,
			 windowtitle,		/* window name */
			 windowtitle,		/* icon name */
			 None,			/* icon pixmap */
			 NULL,			/* command argv */
			 0,			/* command argc */
			 &hints			/* window sizing hints */
			 );
  

  /*
   ** note that the order in which the subwindows are created
   ** determines the stacking order. This has an effect on the
   ** way exposure events are generated during resizing
   */
  /*
   ** 			GRAPH WINDOW
   */
  CopyWindowStruct(frame,graph);
  graph->inverse = 1;
  graph->background = MININTERFACECOLOR + GRAPHBACKGROUND;
  graph->foreground = MININTERFACECOLOR + GRAPHFOREGROUND;
  /*
  graph->foreground = WhitePixelIdx();
  graph->background = BlackPixelIdx();
  */
  graph->windowname = "/graph";
  windowdata = (WindowData *)calloc(1,sizeof(WindowData));
  windowdata->id = GRAPH_WINDOW;
  windowdata->parent = (BasicWindow *)graph;
  windowdata->func = graph_event;
  graph->supercolor = graph->foreground;
  graph->window =(Window)XCreateSimpleWindow(graph->display,frame->window,
	 0,0,width,height,borderwidth,border,GetPixel(graph->background));
    Normal(graph);
  XSaveContext(graph->display,graph->window,datacontext,(XPointer)windowdata);
  graph->drawable = graph->window;
  attrib.backing_store = Always;
  /*
    attrib.backing_store = NotUseful;
    */
  XChangeWindowAttributes(graph->display,graph->window,CWBackingStore,&attrib);
  /*
   ** request that the window be notified of certain events
   */
  XSelectInput(graph->display,graph->window, 
	       KeyPressMask |  ExposureMask | ButtonPressMask | ButtonReleaseMask
	       | Button1MotionMask | Button2MotionMask | Button3MotionMask | PointerMotionMask );
  /*
   ** 			MENU CONTAINER WINDOW
   */

  CreateMenuWindows(menucontainer);


  /*
   ** 			MENU WINDOW
   */
 /*  CopyWindowStruct(frame,menu); */
/*   menu->inverse = 1; */
/*   menu->background = MINCOLOR + MENUBACKGROUND; */
/*   menu->foreground = MINCOLOR + MENUFOREGROUND; */
/*   menu->itembackground = MINCOLOR + MENUITEMBACKGROUND; */
/*   menu->itemforeground = MINCOLOR + MENUITEMFOREGROUND; */
  
/*   str = XGetDefault(frame->display,"xclust","MenuFont"); */
/*   if(str != NULL){ */
/*     frame->menu->fontname = CopyString(str); */
/*   } */
/*   SetFont(menu,menu->fontname); */
/*   menu->windowname = "/menu"; */
/*   windowdata = (WindowData *)calloc(1,sizeof(WindowData)); */
/*   windowdata->id = MENU_WINDOW; */
/*   windowdata->parent = (BasicWindow *)menu; */
/*   windowdata->func = menu_event; */
/*   /\* */
/*     menu->window = (Window)XCreateSimpleWindow(menu->display,frame->window, */
/*     0,0,width,height,borderwidth,border,background); */
/*     *\/ */
/*   menu->window = (Window)XCreateSimpleWindow(menu->display,frame->window, */
/* 					     0,0,width,height,borderwidth, */
/* 					     border,GetPixel(menu->background)); */
/*     Normal(menu); */
/*     XSaveContext(menu->display,menu->window,datacontext,(XPointer)windowdata); */
/*     menu->drawable = menu->window; */
/*     XSelectInput(menu->display,menu->window,ExposureMask); */
/*   /\* */
/*     XSelectInput(menu->display,menu->window,  */
/*     KeyPressMask | ExposureMask | ButtonPressMask | ButtonReleaseMask */
/*     | Button1MotionMask ); */
/*     *\/ */
/*     /\*  */
/*     ** do a little color shuffle to get the bitmap backgrounds right */
/*     *\/ */
/*   menu->background = menu->itembackground; */
/*   menu->foreground = menu->itemforeground; */
/*   LoadBitmaps(menu); */
/*   menu->background = MINCOLOR + MENUBACKGROUND; */
/*   menu->foreground = MINCOLOR + MENUFOREGROUND; */

  /*CreateMainMenu(menu);*/


  /*
   ** 			CLUSTER STATUS WINDOW
   */
/*   CopyWindowStruct(frame,cluststat); */
/*   cluststat->inverse = 1; */
/*   cluststat->background = MINCOLOR + CLUSTSTATBACKGROUND; */
/*   cluststat->foreground = MINCOLOR + CLUSTSTATFOREGROUND; */
/*   str = XGetDefault(frame->display,"xclust","MenuFont"); */
/*   if(str != NULL){ */
/*     cluststat->fontname = CopyString(str); */
/*   } */
/*   SetFont(cluststat,cluststat->fontname); */
/*   cluststat->windowname = "/cluststat"; */
/*   windowdata = (WindowData *)calloc(1,sizeof(WindowData)); */
/*   windowdata->id = CLUSTSTAT_WINDOW; */
/*   windowdata->parent = (BasicWindow *)cluststat; */
/*   windowdata->func = cluststat_event; */
/*   cluststat->wheight = 600; */
/*   cluststat->wwidth = MENU_WIDTH; */

/*   cluststat->window = (Window)XCreateSimpleWindow(menu->display,frame->menu->window, */
/*       0,menu->cluststatoffset,cluststat->wwidth,cluststat->wheight,borderwidth,border, */
/*       GetPixel(cluststat->background)); */
/*     Normal(cluststat); */
/*     XSaveContext(cluststat->display,cluststat->window,datacontext,(XPointer)windowdata); */
/*     cluststat->drawable = cluststat->window; */
/*     XSelectInput(cluststat->display,cluststat->window,  */
/* 	       ExposureMask); */
/*   SampleClustStat(cluststat); */
  /*
   ** 			COLOR STATUS WINDOW
   */
/*   CopyWindowStruct(frame,colorstat); */
/*   colorstat->inverse = 1; */
/*   colorstat->background = MINCOLOR + CLUSTSTATBACKGROUND; */
/*   colorstat->foreground = MINCOLOR + CLUSTSTATFOREGROUND; */
/*   str = XGetDefault(frame->display,"xclust","MenuFont"); */
/*   if(str != NULL){ */
/*     colorstat->fontname = CopyString(str); */
/*   } */
/*   SetFont(colorstat,colorstat->fontname); */
/*   colorstat->windowname = "/colorstat"; */
/*   windowdata = (WindowData *)calloc(1,sizeof(WindowData)); */
/*   windowdata->id = COLORSTAT_WINDOW; */
/*   windowdata->parent = (BasicWindow *)colorstat; */
/*   windowdata->func = colorstat_event; */
/*   colorstat->wheight = 256; */
/*   colorstat->wwidth = 256; /\*MENU_WIDTH;*\/ */

/*   colorstat->window = (Window)XCreateSimpleWindow(menu->display,frame->menu->window, */
/* 						  2,menu->cluststatoffset, */
/* 						  colorstat->wwidth,colorstat->wheight, */
/* 						  borderwidth,border, */
/* 						  GetPixel(colorstat->background)); */
/*   Normal(colorstat); */
/*   XSaveContext(colorstat->display,colorstat->window,datacontext,(XPointer)windowdata); */
/*   colorstat->drawable = colorstat->window; */
/*   XSelectInput(colorstat->display,colorstat->window,  */
/* 	       ExposureMask); */
/*   CreateColorStat(colorstat); */
  /*
   ** 			TEXT WINDOW
   */
  CopyWindowStruct(frame,text);
  text->inverse = 0;
  text->background = MININTERFACECOLOR + TEXTBACKGROUND;
  text->foreground = MININTERFACECOLOR + TEXTFOREGROUND;
  text->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
  text->itemforeground = MININTERFACECOLOR + MENUITEMFOREGROUND;
  
  text->windowname = "/text";
  windowdata = (WindowData *)calloc(1,sizeof(WindowData));
  windowdata->id = TEXT_WINDOW;
  windowdata->parent = (BasicWindow *)text;
  windowdata->func = text_event;
  text->window = (Window)XCreateSimpleWindow(text->display,frame->window,
		 0,0,width,height,borderwidth,border,GetPixel(text->background));
    Normal(text);
  XSaveContext(text->display,text->window,datacontext,(XPointer)windowdata);
  text->drawable = text->window;
  XSelectInput(text->display,text->window, 
	       ExposureMask | KeyPressMask);
  CreateTextMenu(text);
}


CreateMenuWindows(menucontainer)
     MenuContainer *menucontainer;
{
  MenuFrame *menuframe;
  MenuWindow *menu;
  Frame *frame;
  char *str, *tmpstr;
  int borderwidth;
  unsigned long border, fg, bg;
  extern int menucontainer_event();
  extern int menu_event();
  extern int menuframe_event();

  WindowData		*windowdata;
  int index=0;
  int voffset;

  frame = menucontainer->frame;

  str = XGetDefault(frame->display,"xclust","MenuFont");

  borderwidth = 1;
  border = frame->foreground;

  /* create menu container window */

  CopyWindowStruct(frame,menucontainer);
  menucontainer->inverse = 1;
  LoadBitmaps(menucontainer);
  menucontainer->background = MININTERFACECOLOR + MENUBACKGROUND;
  menucontainer->foreground = MININTERFACECOLOR + MENUFOREGROUND;
  if(str != NULL){
    menucontainer->fontname = CopyString(str);
  }
  SetFont(menucontainer,menucontainer->fontname);
  menucontainer->windowname = "/menucontainer";
  windowdata = (WindowData *)calloc(1,sizeof(WindowData));
  windowdata->id = MENU_CONTAINER;
  windowdata->parent = (BasicWindow *)menucontainer;
  windowdata->func = menucontainer_event;

  menucontainer->window = (Window)XCreateSimpleWindow(menucontainer->display,frame->window,
					     0,0,frame->width,frame->height,borderwidth,
					     border,GetPixel(menucontainer->background));
  Normal(menucontainer);
  XSaveContext(menucontainer->display,menucontainer->window,datacontext,(XPointer)windowdata);
  menucontainer->drawable = menucontainer->window;
  XSelectInput(menucontainer->display,menucontainer->window,ExposureMask);

  /* loop through all menu frames */

  for (menuframe = menucontainer->menuframe; menuframe; menuframe = menuframe->next) {
    
    /* create menu frame window */
    tmpstr = menuframe->windowname;
    CopyWindowStruct(frame,menuframe);
    menuframe->windowname = tmpstr;

    menuframe->inverse = 1;
    menuframe->background = MININTERFACECOLOR + MENUFRAMEBACKGROUND;
    menuframe->foreground = MININTERFACECOLOR + MENUFRAMEFOREGROUND;
    if(str != NULL){
      menuframe->fontname = CopyString(str);
    }
    SetFont(menuframe,menuframe->fontname);

    windowdata = (WindowData *)calloc(1,sizeof(WindowData));
    windowdata->id = MENU_FRAME_OFFSET + index;
    windowdata->parent = (BasicWindow *)menuframe;
    windowdata->func = menuframe_event;

    menuframe->window = (Window)XCreateSimpleWindow(menuframe->display,menucontainer->window,
						    0,0,frame->width,frame->height,borderwidth,
						    border,GetPixel(menuframe->background));
    Normal(menuframe);
    XSaveContext(menuframe->display,menuframe->window,datacontext,(XPointer)windowdata);
    menuframe->drawable = menuframe->window;
    XSelectInput(menuframe->display,menuframe->window,ExposureMask | ButtonPressMask | ButtonReleaseMask);

    if (menuframe->showtitle) {
      AddLabelString(menuframe, menuframe->title,
		     20,12,0.0,0.0,SCREEN_LBL,PERMANENT_LBL,menuframe->fontname);
    }

    /* create menu window */
    menu = menuframe->menu;
    
    tmpstr = menu->windowname;
    fg =  menu->foreground;
    bg =  menu->background;
    CopyWindowStruct(frame,menu);
    menu->windowname = tmpstr;
    menu->foreground = fg;
    menu->background = bg;

    menu->inverse = 1;
/*     menu->background = MINCOLOR + MENUBACKGROUND; */
/*     menu->foreground = MINCOLOR + MENUFOREGROUND; */
/*     menu->itembackground = MINCOLOR + MENUITEMBACKGROUND; */
/*     menu->itemforeground = MINCOLOR + MENUITEMFOREGROUND; */
  
    if(str != NULL){
      menu->fontname = CopyString(str);
    }
    SetFont(menu,menu->fontname);

    windowdata = (WindowData *)calloc(1,sizeof(WindowData));
    windowdata->id = MENU_OFFSET + index;
    windowdata->parent = (BasicWindow *)menu;
    windowdata->func = menu_event;

    voffset = 0;
    if (menuframe->showtitle)
      voffset += 16;

    menu->window = (Window)XCreateSimpleWindow(menu->display,menuframe->window,
					     0,voffset,frame->width,frame->height,borderwidth,
					     border,GetPixel(menu->background));
    Normal(menu);
    XSaveContext(menu->display,menu->window,datacontext,(XPointer)windowdata);
    menu->drawable = menu->window;
    XSelectInput(menu->display,menu->window,ExposureMask | ButtonPressMask | ButtonReleaseMask);

    /* 
    ** do a little color shuffle to get the bitmap backgrounds right
    */
/*     menu->background = menu->itembackground; */
/*     menu->foreground = menu->itemforeground; */
/*    /\*  LoadBitmaps(menu); *\/ */
/*     menu->background = MINCOLOR + MENUBACKGROUND; */
/*     menu->foreground = MINCOLOR + MENUFOREGROUND; */

    index++;

  }

  CreateMenus(menucontainer);

}


Quit()
{
    if(F->display){
	XDestroyWindow(F->display,F->window);
	XCloseDisplay(F->display);
    }
    /*
    ** check to see whether cluster bounds have been defined
    ** If so then backup the bounds before leaving
    */
    if(G->clusterbounds){
	WriteAllClusterBounds(G,"cbfile.tmp");
	fprintf(stderr,"saving cluster bounds to '%s'\n", 
	"cbfile.tmp");
    }
    fprintf(stderr,"Exiting Xclust...\n");
    exit(0);
}
