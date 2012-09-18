#include	"xview_ext.h"

event_handler(event)
XEvent		*event;
{

    switch (event->type) {
    case Expose:
	RefreshXview();
	break;
    case ButtonPress:
	ButtonAction(event);
	break;
    case KeyPress:
	KeyAction(event);
	break;		
    default:
	break;
    }
}

RefreshXview()
{
    if(V->display_mode != COLORBOX && V->display_mode != COLORCONTOUR){
	ClearWindow(G->imagewindow);
    }
    Rescale();
    if(header || (PSStatus() == 0)){
	legend_background();
    }
    DisplayFrame();
    if(header || (PSStatus() == 0)){
	if(V->display_mode == COLORBOX || V->display_mode == COLORCONTOUR ||
	V->display_mode==COLORSURFACE || V->display_mode==COLORFILLEDSURFACE){
	    DrawColorBar();
	}
    }
}

ButtonAction(buttonevent)
XButtonPressedEvent		*buttonevent;
{
    if (buttonevent->button == 1) {
	BackStep();
    } else
    if (buttonevent->button == 2) {
	DisplayLocation(buttonevent->x,buttonevent->y);
    } else
    if (buttonevent->button == 3) {
	ForwardStep();
    }
}

EventString(E)
XEvent	*E;
{
XButtonPressedEvent	*B;

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
    case Expose :
	printf("Expose\n");
	break;
    case KeyPress :
	printf("KeyPress\n");
	break;
    default:
	printf("unknown event type %d\n",E->type);
	break;
    }
}
