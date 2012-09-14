#include 	"xview_ext.h"

IncrementViewTime(inc)
float inc;
{
float	time;

    time = V->view_time + inc;
    if(time < V->start_time) time = V->start_time;
    V->view_time =  (int)(time/V->dt +.5)*V->dt;
}

SetViewTime(time)
float time;
{
    if(time < V->start_time) time = V->start_time;
    V->view_time =  (int)(time/V->dt +.5)*V->dt;
}

DisplayView()
{
    if(V->display_mode != COLORBOX && V->display_mode != COLORCONTOUR){
	ClearImage();
    }
    V->valid_frame = GetFrame(V->view_time);
    /*
    ** HACKED in for continous play
    if(!V->valid_frame){
	V->view_time = V->start_time;
	V->valid_frame = GetFrame(V->view_time);
	V->singlestep = FALSE;
    }
    */
    DisplayFrame();
}

EventLoop()
{
XEvent	event; 
int	i,j;
extern int signal_step,got_signal;

    XFlush(G->display);
    /*
    ** display the first frame
    */
    V->valid_frame = GetFrame(V->view_time);
    Rescale();
    /* DisplayFrame(); */
    if(commandfile){
	ReadCommandFile(commandsource,commandfile);
    }
    got_signal = 0;
    for (;;) {
	if(signal_step){
	    while(!got_signal) Usleep(100);
	    got_signal = 0;
	    V->singlestep = 0;
	} 
	if(!V->singlestep){
	    /*
	    ** continuous display
	    */
	    ForwardStep();
	    if(speed > 0){
		XFlush(G->display);
		for(i=0;i<speed;i++) Usleep(1000);
	    }
	}
	/*
	** only go into the event loop if in singlestep mode
	** or there is an event pending
	*/
	if(V->singlestep || (XPending(G->display) != 0)){
	    /*
	    ** remove all the unwanted expose events from the queue
	    */
	    while(XCheckMaskEvent(G->display,ExposureMask,&event))
		;

	    XNextEvent(G->display,&event);
	    if(debug){
		EventString(&event);
	    }
	    event_handler(&event);
	}
    }
}
