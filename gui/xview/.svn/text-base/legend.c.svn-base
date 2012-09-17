#include "xview_ext.h"

DrawBorder()
{
int ymin;
int ymax;
int space;

    space = G->fontheight*0.5;
    ymin = G->fontheight*2 + space;
    ymax = legend_base - space;
    DrawLine (-space, ymin, width+space, ymin);
    DrawLine ( width+space, ymin, width+space, ymax);
    DrawLine ( width+space, ymax, -space, ymax);
    DrawLine ( -space, ymax, -space, ymin);
}

DrawColorScale()
{
int 	i,x;
int	xbase,ybase;
int	maxbase;
    SetColor (G->foreground);
    xbase = G->fontwidth;
    ybase = 2*G->fontheight;
    /*
    ** scale labels
    */
    /*
    ** minval
    */
    if(fabs(minval) < 0.1)
	sprintf (valstr, "%-7.1e", minval);
    else
	sprintf (valstr, "%-7.2f", minval);
    Text (G->fontwidth, ybase, valstr);
    /*
    ** maxval
    */
    if(fabs(maxval) < 0.1)
	sprintf (valstr, "%-7.1e", maxval);
    else
	sprintf (valstr, "%-7.2f", maxval);
    maxbase = width - G->fontwidth*10;
    if(maxbase < G->fontwidth*10) {
	maxbase = G->fontwidth*10;
    }
    Text (maxbase, ybase, valstr);
}

DrawColorBar()
{
float	color_width;
int 	i,x;
int	xbase,ybase;
    xbase = G->fontwidth;
    ybase = G->fontheight;
    /*
    ** color bar
    */
    color_width = width/color_scale;
    for(i=0;i<(int)(color_scale);i++){
	SetColor((int)(i+color_min));
	/*
	x = xbase + (int)(i*G->fontwidth/5);
	*/
	x = (i*color_width);
	FilledBox (
	    x, 0,
	    (int)(x + color_width + 1),
	    G->fontheight);
    }
}

ClearTitle()
{
    SetColor(G->background);
    FilledBox(0, 0, width,G->fontheight);
}

legend_background()
{
int	i;
/*
    SetMaxGray(color_scale);
*/

    SetColor (G->foreground);
    if(header || (PSStatus() == 0)){
	/*
	** time label
	*/
	Text (G->fontwidth*9, legend_base, "msec");
	/*
	** title
	ClearTitle();
	*/
	Text (G->fontwidth*18, legend_base, title[title_mode]);
    } else {
	SetColor (G->background);
	FilledBox (0,legend_base,width,legend_base + G->fontheight);
    }
    DrawColorScale();
}

legend_foreground()
{
    /* 
    ** display the time 
    */
    SetColor (G->foreground);
    if(V->valid_frame){
	sprintf(timestr,"%7.2f",V->view_time);
    } else {
	sprintf(timestr,"<<END>>");
    }
    Text (G->fontwidth, legend_base, timestr);
}

