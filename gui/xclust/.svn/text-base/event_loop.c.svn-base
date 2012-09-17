#include 	"xclust_ext.h"
#include	<sys/time.h>
#include        <math.h>

extern Label *commandlabel;
extern Label *coordlabel;
extern Label *rotlabel;
extern int skipevent;
extern int rotatecolor;
long	clockval;
long	lastexposed=0;
int	needsrefresh=0;
int	nexpose=0;


EventLoop(frame)
Frame	*frame;
{
XEvent		event; 
WindowData	*data;
Graph 		*graph;
extern char	*restorefile;
char		versionstr[100];
char		coordstring[50];
char		rotstring[50];
int		r,g,b;
int		already_processed;
int		ncycles;
Job		*job;

XTimeCoord	*xtc;
int		nevents;

XEvent		motion_event, last_motion_event;
XEvent		*pme = NULL;

    XFlush(frame->display);
    graph = frame->graph;
    RescaleFrame(frame);
    /*
    ** display the graph
    */
    sprintf(versionstr,"XCLUST3 v%s %s",VERSION,COPYRIGHT);
    commandlabel = AddLabelString(frame->text,versionstr,
				  frame->text->fontwidth,frame->text->fontheight,0.0,0.0,
				  SCREEN_LBL,TEMPORARY_LBL,frame->fontname);

    sprintf(coordstring, "sx: %3d  sy: %3d  wx: %9.2f  wy: %9.2f", 0,0,0.0,0.0);
    coordlabel = AddLabelString(frame->text,coordstring,
				frame->text->fontwidth + 400,frame->text->fontheight,0.0,0.0,
				SCREEN_LBL,TEMPORARY_LBL,frame->fontname);

    sprintf(rotstring, "tx: %5d  ty: %5d  tz: %5d", 0,0,0);
    rotlabel = AddLabelString(frame->text,rotstring,
			      frame->text->fontwidth + 675,frame->text->fontheight,0.0,0.0,
			      SCREEN_LBL,TEMPORARY_LBL,frame->fontname);

    /*
    ** start with the first saved view, if available
    */

    InterpretCommand(G,SelectedPlot(G),"/nextsavedview");

    
    /*
    ** execute command file or line if any
    */
    ExecuteCommands(graph);

    /*
    SmoothRange();
    */

    /*
    ** process any pending events
    */
    while(XPending(frame->display)){
	ProcessEvent(frame);
    }
    RescaleFrame(frame);

    MapWindows(frame);

    if(restorefile){
	if(F->display && F->width > 0 && F->height > 0){
	    XResizeWindow(F->display,F->window,F->width,F->height);
	    XFlush(frame->display);
	    RescaleFrame(frame);
	}
    }

    ActivateIntermittentSave(30000);
    ChangeIntermittentSaveDelay(30000);

    ncycles = 1;
    while(1){
	already_processed = 0;
	/*
	** handle any unfinished refreshes
	*/

/* 	if (skipevent) { */
/* 	pme = NULL; */
/* 	XFlush(G->display); */
/* 	while (XCheckMaskEvent(G->display, ButtonMotionMask, &motion_event)) { */
/* 	  fprintf(stderr, "skipping motion event"); */
/* 	  pme = &motion_event; */
/* 	} */
/* 	/\*graph_event(frame, &last_motion_event);*\/ */
/* 	if (pme!=NULL) */
/* 	  XPutBackEvent(G->display, pme); */
/* 	} */

/* 	if (skipevent) { */
/* 	  xtc = NULL; */
/* 	  xtc = XGetMotionEvents(G->display, G->window, 0, CurrentTime, &nevents ); */
/* 	  if (nevents>0) */
/* 	    fprintf(stderr, "nevents: %d\n", nevents); */
/* 	  XFree(xtc); */
/* 	  xtc = NULL; */
/* 	} */

	time(&clockval);
	if((clockval - lastexposed >= 1) && needsrefresh){
	/*
		fprintf(stderr,"refreshed %d after expose at %s\n",nexpose,ctime(&clockval));
		*/
		RefreshGraph(graph);
		needsrefresh = 0;
		nexpose++;
	}
	/*
	** scan through the job list for stuff to do on this cycle
	*/
	for(job=frame->joblist;job;job=job->next){
	    /*
	    ** process any X pending events - but dont block
	    */
	    while(XPending(frame->display)){
		ProcessEvent(frame);
	    }
	    Usleep(10000);
	    /*
	    ** if it is active then fire it up
	    */
	    if (job->priority ==0 ||
		ncycles%job->priority == 0){
		if(job->func){
		    /*
		    ** call the function
		    */
		    job->func(job);
		} 
	    }
	    already_processed = 1;
	}
	if(rotatecolor){
	    /*
	    ** process X events - dont block
	    */
	    while(XPending(frame->display)){
		ProcessEvent(frame);
	    }
	    Usleep(50000);
	    if(rotatecolor){
	    /*
		CyclePixel(graph->selectedcluster+MINCLUSTER,1,1,1,40,40,40);
		*/
		ChangeColor(LookupClusterColor(graph,graph->selectedcluster));
		RefreshGraph(G);
	    }
	    already_processed = 1;
	} 
	if(!already_processed){
	    /*
	    ** just process X events - block
	    */
	    ProcessEvent(frame);
	}
	ncycles++;
#ifdef DEBUG
	if(data->id == FRAME_WINDOW){
	    if(debug){
		printf("FRAME ");
		EventString(&event);
	    }
	    data->func(data->parent,&event);
	    frame_event(data->parent,&event);
	} else
	if(data->id == GRAPH_WINDOW){
	    if(debug){
		printf("GRAPH ");
		EventString(&event);
	    }
	    graph_event(data->parent,&event);
	} else
	if(data->id == TEXT_WINDOW){
	    if(debug){
		printf("TEXT ");
		EventString(&event);
	    }
	    text_event(data->parent,&event);
	} else
	if(data->id == MENU_WINDOW){
	    if(debug){
		printf("MENU ");
		EventString(&event);
	    }
	    menu_event(data->parent,&event);
	}
#endif
    }
}

ProcessEvent(frame)
Frame	*frame;
{
WindowData	*data;
XEvent		event; 
XEvent          motionevent;
XEvent          *pme;

    XNextEvent(frame->display,&event);
    /*
    ** get data from the window the event occured in
    */

    pme = NULL;
    if (event.type == MotionNotify) {
      while (XCheckTypedEvent(frame->display, MotionNotify, &motionevent)) {
	/*fprintf(stderr, "skipping motion event");*/
	pme = &motionevent;
      }
      if (pme)
	event = *pme;
    }



    XFindContext(frame->display,((XKeyEvent *)(&event))->window,
	datacontext, (XPointer *) &data );
    if(debug){
	fprintf(stderr,"window %s\n",data->parent->windowname);
    }
    data->func(data->parent,&event);
}

frame_event(frame,event)
Frame		*frame;
XEvent		*event;
{
int stat;
 MenuFrame *menuframe;

 if(debug){
   fprintf(stderr,"frame action %d\n",event->type);
 }

    switch (event->type) {
    case ConfigureNotify:
	XSynchronize(frame->display,1);
	stat = RescaleFrame(frame);

	XSynchronize(frame->display,0);
	if(stat){
	    /*
	    ** get rid of the unpredictable refreshes generated by the 
	    ** reconfigure
	    */
	    ZapExpose(frame->display);
	    /*
	    ** refresh the subwindows myself
	    */
	    RefreshGraph(frame->graph);
	    RefreshText(frame->text);
/* 	    RefreshMenuItems(frame->menu); */
/* 	    RefreshMenu(frame->menu); */
	    RefreshMenuItems(frame->text);
	    RefreshMenu(frame->text);
	    for(menuframe = frame->menucontainer->menuframe; menuframe; menuframe = menuframe->next) {
	      RefreshMenuItems(menuframe->menu);
	      if (strcmp(menuframe->menu->windowname, "/cluststat")==0) {
		DrawClustStat(menuframe->menu);
	      } else {
		RefreshMenu(menuframe->menu);
	      }
	      RefreshMenu(menuframe);
	    }
	    RefreshMenu(frame->menucontainer);
	}
	break;
    default:
	break;
    }
}
graph_event(graph,event)
Graph		*graph;
XEvent		*event;
{
XExposeEvent	*xev;

    switch (event->type) {
    case Expose:
	xev = (XExposeEvent *)event;
	time(&clockval);
	if(xev->count == 0){
	    /*
	    ** try to filter unnecessary multiple refresh
	    ** events by keeping them to a most 1 per sec
	    */
	    /*
	    ** flag that a refresh event happened but might not be 
	    ** serviced due to filtering.  
	    ** handle the first one here and then wait to handle others
	    */
	    needsrefresh = 1;
	    /*
	    if(clockval - lastexposed >= 2){
		fprintf(stderr,"refreshed %d with expose at %s\n",nexpose,ctime(&clockval));
		RefreshGraph(graph);
		needsrefresh = 0;
		nexpose++;
	    }
		*/
	    lastexposed = clockval;
	}
	break;
    case ButtonPress:
	ButtonPressAction(graph,event);
	break;
    case ButtonRelease:
	ButtonReleaseAction(graph,event);
	break;
    case KeyPress:
	KeyAction(graph,event);
	break;		
    case KeyRelease:
        KeyReleaseAction(graph,event);
	break;
    case MotionNotify :
	PointerMotionAction(graph,event);
	break;		
    case LeaveNotify:
    case FocusOut:
        LeaveNotifyAction(graph,event);
        break;
    default:
	break;
    }
}

text_event(text,event)
TextWindow	*text;
XEvent		*event;
{
XExposeEvent	*xev;

    switch (event->type) {
    case Expose:
	xev = (XExposeEvent *)event;
	if(xev->count == 0){
	    RefreshText(text);
	}
	break;
    case KeyPress:
	KeyAction(text->frame->graph,event);
	break;		
    default:
	break;
    }
}

menucontainer_event(menucontainer, event)
     MenuContainer *menucontainer;
     XEvent *event;
{
XExposeEvent	*xev;

    if(debug){
	fprintf(stderr,"menu containeraction %d\n",event->type);
    }
    switch (event->type) {
    case Expose:
	xev = (XExposeEvent *)event;
	if(xev->count == 0){
	  RefreshMenu(menucontainer);
	}
	break;
    default:
	break;
    }
}

menuframe_event(menuframe, event)
     MenuFrame *menuframe;
     XEvent *event;
{
XExposeEvent	*xev;

 if(debug){
   fprintf(stderr,"menu frame action %d\n",event->type);
 }
    switch (event->type) {
    case Expose:
	xev = (XExposeEvent *)event;
	if(xev->count == 0){
	  RefreshMenu(menuframe);
	}
	break;
    case ButtonPress:
      menuframe->expanded = !menuframe->expanded;
      CollapseMenu(menuframe);
      break;
    default:
	break;
    }
}

menu_event(menu,event)
MenuWindow	*menu;
XEvent		*event;
{
XExposeEvent	*xev;
XButtonEvent    *bev;
 int pid;
 ClusterBounds *cb;
 int clusterid=0, nbound=0;
 char tmpstr[30];

    if(debug){
	fprintf(stderr,"menu action %d\n",event->type);
    }
    switch (event->type) {
    case Expose:
	xev = (XExposeEvent *)event;
	if(xev->count == 0){
	  if (strcmp(menu->windowname, "/cluststat")==0) {
	    DrawClustStat(menu);
	  } else {
	    RefreshMenu(menu);
	  }

	}
	break;
    case ButtonPress:
      bev = (XButtonEvent*)event;
      clusterid = F->graph->selectedcluster;

      if (bev->x > 5 && bev->x < 110 && bev->y>=55) {
	pid = floor( (double) ( (bev->y - 55) / 15) ) + 1;

	for(cb=F->graph->clusterbounds;cb;cb=cb->next){
	  if(cb->clusterid == clusterid){
	    nbound++;
	    if (nbound == pid) {
	      sprintf(tmpstr, "/projection %d %d", cb->projection[0], cb->projection[1]);
	      InterpretCommand(F->graph, SelectedPlot(F->graph), tmpstr);
	      RefreshGraph(F->graph);
	      break;
	    }
	  }
	}
      }
      break;
    default:
	break;
    }
}

cluststat_event(menu,event)
MenuWindow	*menu;
XEvent		*event;
{
XExposeEvent	*xev;

    if(debug){
	fprintf(stderr,"menu action %d\n",event->type);
    }
    switch (event->type) {
    case Expose:
	xev = (XExposeEvent *)event;
	if(xev->count == 0){
	    DrawClustStat(menu);
	}
	break;
    default:
	break;
    }
}

colorstat_event(menu,event)
MenuWindow	*menu;
XEvent		*event;
{
XExposeEvent	*xev;

    if(debug){
	fprintf(stderr,"menu action %d\n",event->type);
    }
    switch (event->type) {
    case Expose:
	xev = (XExposeEvent *)event;
	if(xev->count == 0){
	    DrawColorStat(menu);
	}
	break;
    default:
	break;
    }
}


EventString(E)
XEvent	*E;
{
XButtonPressedEvent	*B;
static int count=0;

    printf("%d ",count);
    switch(E->type){
    case ButtonPress :
	B = (XButtonPressedEvent *)E;
	printf("ButtonPress ");
	if(B->button == 1){
	   printf("1 "); 
	} 
	if(B->button == 2){
	   printf("2 "); 
	} 
	if(B->button == 3){
	   printf("3"); 
	} 
	printf("\n");
	break;
    case ButtonRelease :
	B = (XButtonPressedEvent *)E;
	printf("ButtonRelease ");
	if(B->button == 1){
	   printf("1 "); 
	} 
	if(B->button == 2){
	   printf("2 "); 
	} 
	if(B->button == 3){
	   printf("3"); 
	} 
	printf("\n");
	break;
    case MotionNotify :
	printf("PointerMoved %d %d\n",
	((XPointerMovedEvent *)E)->state,
	((XPointerMovedEvent *)E)->is_hint);
	break;
    case Expose :
	printf("Expose %d\n",((XExposeEvent *)E)->count);
	break;
    case KeyPress :
	printf("KeyPress\n");
	break;
    case ConfigureNotify:
	printf("Configure\n");
	break;
    default:
	printf("unknown event type %d\n",E->type);
	break;
    }
    count++;
}

