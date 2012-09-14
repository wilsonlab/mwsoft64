#include "xplot_ext.h"

RefreshText(text)
TextWindow	*text;
{
    ClearWindow(text);
    DisplayLabels(text);
}

DisplayLabels(basic)
BasicWindow	*basic;
{
int sx,sy,sr;
int sx2,sy2;
double	wx,wy,wz;
Label	*label;

    if(basic->hide_labels) return;
    for(label=basic->label;label;label=label->next){
	/*
	** set the color
	*/
	SetColor(label->color);
	/*
	** put up the label data
	*/
	switch(label->type){
	case STRING_LBL:
	    /*
	    ** get the coords
	    */
	    switch(label->coord_mode){
	    case WORLD_LBL:		/* world coord */
		MatrixTransform(basic,(double)label->u.string.wx,
		    (double)label->u.string.wy,(double)label->u.string.wz,
		    &wx,&wy,&wz,0.0);
		ScreenTransform(basic,wx,wy,&sx,&sy);
		break;
	    case SCREEN_LBL:		/* screen coord */
		sx = label->u.string.sx;
		sy = label->u.string.sy;
		break;
	    case WINDOW_LBL:		/* window coord */
		sx = label->u.string.wx * basic->wwidth;
		sy = label->u.string.wy * basic->wheight;
		break;
	    }
	    /*
	    ** draw the text
	    */
#ifdef TEMP
	    if(basic->display){
		XSetFont(basic->display,basic->context,label->u.string.font);
	    }
#endif
	    Text(basic,sx,sy,label->u.string.line);
	    break;
	case BOX_LBL:
	    /*
	    ** get the coords
	    */
	    switch(label->coord_mode){
	    case WORLD_LBL:		/* world coord */
		MatrixTransform(basic,(double)label->u.box.wx,
		    (double)label->u.box.wy,(double)label->u.box.wz,
		    &wx,&wy,&wz,0.0);
		ScreenTransform(basic,wx,wy,&sx,&sy);
		sr = label->u.box.wr;
		break;
	    case SCREEN_LBL:		/* screen coord */
		sx = label->u.box.sx;
		sy = label->u.box.sy;
		sr = label->u.box.sr;
		break;
	    case WINDOW_LBL:		/* window coord */
		sx = label->u.box.wx*basic->wwidth;
		sy = label->u.box.wy*basic->wheight;
		sr = label->u.box.wr*basic->wwidth;
		break;
	    }
	    /*
	    ** draw the box
	    */
	    Box(sx-sr,sy-sr,sx+sr,sy+sr);
	    break;
	case LINE_LBL:
	    /*
	    ** get the coords
	    */
	    switch(label->coord_mode){
	    case WORLD_LBL:		/* world coord */
		MatrixTransform(basic,(double)label->u.line.wx,
		    (double)label->u.line.wy,(double)label->u.line.wz,
		    &wx,&wy,&wz,0.0);
		ScreenTransform(basic,wx,wy, &sx,&sy);
		MatrixTransform(basic,label->u.line.wx2,
		    label->u.line.wy2,label->u.line.wz2,
		    &wx,&wy,&wz,0.0);
		ScreenTransform(basic,wx,wy,&sx2,&sy2);
		break;
	    case SCREEN_LBL:		/* screen coord */
		sx = label->u.line.sx;
		sy = label->u.line.sy;
		sx2 = label->u.line.sx2;
		sy2 = label->u.line.sy2;
		break;
	    case WINDOW_LBL:		/* window coord */
		sx = label->u.line.wx*basic->wwidth;
		sy = label->u.line.wy*basic->wheight;
		sx2 = label->u.line.wx2*basic->wwidth;
		sy2 = label->u.line.wy2*basic->wheight;
		break;
	    }
	    /*
	    ** draw the line
	    */
	    DrawLine(sx,sy,sx2,sy2);
	    break;
	case BITMAP_LBL:
	    /*
	    ** get the coords
	    */
	    switch(label->coord_mode){
	    case WORLD_LBL:		/* world coord */
		MatrixTransform(basic,(double)label->u.box.wx,
		    (double)label->u.box.wy,(double)label->u.box.wz,
		    &wx,&wy,&wz,0.0);
		ScreenTransform(basic,wx,wy,&sx,&sy);
		break;
	    case SCREEN_LBL:		/* screen coord */
		sx = label->u.box.sx;
		sy = label->u.box.sy;
		break;
	    case WINDOW_LBL:		/* window coord */
		sx = label->u.box.wx*basic->wwidth;
		sy = label->u.box.wy*basic->wheight;
		break;
	    }
	    /*
	    ** draw the bitmap
	    */
	    DrawBitmap(basic,
	    label->u.bitmap.bitmap,
	    sx,sy,label->u.bitmap.w,label->u.bitmap.h);
	    break;
	}
    }
}

/*
** append a label to the window
*/
AddLabelString(basic,s,sx,sy,wx,wy,coord_mode,priority,fontname)
BasicWindow	*basic;
char		*s;
short		sx,sy;
float		wx,wy;
short		coord_mode;
short		priority;
char		*fontname;
{
Label	*newlabel;

    newlabel = (Label *)calloc(1,sizeof(Label));
    newlabel->type = STRING_LBL;
    newlabel->priority = priority;
    /*
    ** assign the color
    */
    newlabel->color = basic->foreground;
    /*
    ** get the label coordinates
    */
    newlabel->coord_mode = coord_mode;
    /*
    ** assign the font
    */
    if(fontname == NULL){
	fprintf(stderr,"NULL fontname\n");
	fontname = "fixed";
    }
    if(basic->display){
	newlabel->u.string.font = XLoadFont(basic->display,fontname);
	if (newlabel->u.string.font == BadName){
	    fprintf(stderr,"unable to open text font %s",fontname);
	    newlabel->u.string.font = XLoadFont(basic->display,"fixed");
	}
    }
    switch(coord_mode){
    case WORLD_LBL:	/* world coords */
	newlabel->u.string.wx = wx;
	newlabel->u.string.wy = wy;
	break;
    case SCREEN_LBL:	/* screen coords */
	newlabel->u.string.sx = sx;
	newlabel->u.string.sy = sy;
	break;
    case WINDOW_LBL:	/* screen coords */
	newlabel->u.string.wx = (float)sx/basic->wwidth;
	newlabel->u.string.wy = (float)sy/basic->wheight;
	break;
    }
    /*
    ** get the string
    */
    strncpy(newlabel->u.string.line,s,MAX_TEXT_LINE);
    /* just to make sure it is NULL terminated */
    newlabel->u.string.line[MAX_TEXT_LINE-1] = '\0';		
    /* 
    ** insert it into the list 
    */
    newlabel->next = basic->label;
    basic->label = newlabel;
}

/*
** append a label to plot
*/
AddPlotLabelString(plot,s,sx,sy,wx,wy,coord_mode,priority,fontname)
Plot		*plot;
char		*s;
short		sx,sy;
float		wx,wy;
short		coord_mode;
short		priority;
char		*fontname;
{
BasicWindow	*basic;
Label	*newlabel;

    basic = (BasicWindow *)(plot->graph);
    newlabel = (Label *)calloc(1,sizeof(Label));
    newlabel->type = STRING_LBL;
    newlabel->priority = priority;
    /*
    ** assign the color
    */
    newlabel->color = basic->foreground;
    /*
    ** get the label coordinates
    */
    newlabel->coord_mode = coord_mode;
    /*
    ** assign the font
    */
    if(fontname == NULL){
	fprintf(stderr,"NULL fontname\n");
	fontname = "fixed";
    }
    if(basic->display){
	newlabel->u.string.font = XLoadFont(basic->display,fontname);
	if (newlabel->u.string.font == BadName){
	    fprintf(stderr,"unable to open text font %s",fontname);
	    newlabel->u.string.font = XLoadFont(basic->display,"fixed");
	}
    }
    switch(coord_mode){
    case WORLD_LBL:	/* world coords */
	newlabel->u.string.wx = wx;
	newlabel->u.string.wy = wy;
	break;
    case SCREEN_LBL:	/* screen coords */
	newlabel->u.string.sx = sx;
	newlabel->u.string.sy = sy;
	break;
    case WINDOW_LBL:	/* screen coords */
	newlabel->u.string.wx = (float)sx/basic->wwidth;
	newlabel->u.string.wy = (float)sy/basic->wheight;
	break;
    }
    /*
    ** get the string
    */
    strncpy(newlabel->u.string.line,s,MAX_TEXT_LINE);
    /* just to make sure it is NULL terminated */
    newlabel->u.string.line[MAX_TEXT_LINE-1] = '\0';		
    /* 
    ** insert it into the list 
    */
    newlabel->next = plot->label;
    plot->label = newlabel;
}

/*
** frees all labels
*/
FreeAllLabels(basic)
BasicWindow	*basic;
{
Label	*next;

    while(basic->label){
	next = basic->label->next;
	free(basic->label);
	basic->label = next;
    }
}

/*
** frees temporary labels
*/
FreeLabels(basic)
BasicWindow	*basic;
{
Label	*next;
Label	*current;
Label	*prev;

    prev = NULL;
    current = basic->label;
    while(current){
	next = current->next;
	if(current->priority == TEMPORARY_LBL){
	    free(current);
	    if(prev == NULL){
		basic->label = next;
	    } else {
		prev->next = next;
	    }
	} else {
	    prev = current;
	}
	current = next;
    }
}

/*
** append a line label to the window
*/
AddLabelLine(basic,sx,sy,sy2,sx2,wx,wy,wz,wx2,wy2,wz2,coord_mode,priority)
BasicWindow	*basic;
short		sx,sy;
short		sx2,sy2;
float		wx,wy,wz;
float		wx2,wy2,wz2;
short		coord_mode;
short		priority;
{
Label	*newlabel;

    newlabel = (Label *)calloc(1,sizeof(Label));
    newlabel->type = LINE_LBL;
    newlabel->priority = priority;
    /*
    ** assign the color
    */
    newlabel->color = basic->foreground;
    /*
    ** get the label coordinates
    */
    newlabel->coord_mode = coord_mode;
    switch(coord_mode){
    case WORLD_LBL:	/* world coords */
	newlabel->u.line.wx = wx;
	newlabel->u.line.wy = wy;
	newlabel->u.line.wz = wz;
	newlabel->u.line.wx2 = wx2;
	newlabel->u.line.wy2 = wy2;
	newlabel->u.line.wz2 = wz2;
	break;
    case SCREEN_LBL:	/* screen coords */
	newlabel->u.line.sx = sx;
	newlabel->u.line.sy = sy;
	newlabel->u.line.sx2 = sx2;
	newlabel->u.line.sy2 = sy2;
	break;
    case WINDOW_LBL:	/* window coords */
	newlabel->u.line.wx = (float)sx/basic->wwidth;
	newlabel->u.line.wy = (float)sy/basic->wheight;
	newlabel->u.line.wz = 0.0;
	newlabel->u.line.wx2 = (float)sx2/basic->wwidth;
	newlabel->u.line.wy2 = (float)sy2/basic->wheight;
	newlabel->u.line.wz2 = 0.0;
	break;
    }
    /* 
    ** insert it into the list 
    */
    newlabel->next = basic->label;
    basic->label = newlabel;
}

/*
** append a label to the window
*/
AddLabelBox(basic,sx,sy,sr,wx,wy,wr,coord_mode,priority)
BasicWindow	*basic;
short		sx,sy,sr;
float		wx,wy,wr;
short		coord_mode;
short		priority;
{
Label	*newlabel;

    newlabel = (Label *)calloc(1,sizeof(Label));
    newlabel->type = BOX_LBL;
    newlabel->priority = priority;
    /*
    ** assign the color
    */
    newlabel->color = basic->foreground;
    /*
    ** get the label coordinates
    */
    newlabel->coord_mode = coord_mode;
    switch(coord_mode){
    case WORLD_LBL:	/* world coords */
	newlabel->u.box.wx = wx;
	newlabel->u.box.wy = wy;
	newlabel->u.box.wr = wr;
	break;
    case SCREEN_LBL:	/* screen coords */
	newlabel->u.box.sx = sx;
	newlabel->u.box.sy = sy;
	newlabel->u.box.sr = sr;
	break;
    case WINDOW_LBL:	/* window coords */
	newlabel->u.box.wx = (float)sx/basic->wwidth;
	newlabel->u.box.wy = (float)sy/basic->wheight;
	newlabel->u.box.wr = (float)sr/basic->wwidth;
	break;
    }
    /* 
    ** insert it into the list 
    */
    newlabel->next = basic->label;
    basic->label = newlabel;
}

/*
** append a label to the window
*/
AddLabelBitmap(basic,bitmap,sx,sy,wx,wy,w,h,coord_mode,priority)
BasicWindow	*basic;
Pixmap		bitmap;
short		sx,sy;
float		wx,wy;
int		w,h;
short		coord_mode;
short		priority;
{
Label	*newlabel;

    newlabel = (Label *)calloc(1,sizeof(Label));
    newlabel->type = BITMAP_LBL;
    newlabel->priority = priority;
    /*
    ** assign the color
    */
    newlabel->color = basic->foreground;
    /*
    ** get the label coordinates
    */
    newlabel->coord_mode = coord_mode;
    newlabel->u.bitmap.bitmap = bitmap;
    newlabel->u.bitmap.w = w;
    newlabel->u.bitmap.h = h;
    switch(coord_mode){
    case WORLD_LBL:	/* world coords */
	newlabel->u.bitmap.wx = wx;
	newlabel->u.bitmap.wy = wy;
	break;
    case SCREEN_LBL:	/* screen coords */
	newlabel->u.bitmap.sx = sx;
	newlabel->u.bitmap.sy = sy;
	break;
    case WINDOW_LBL:	/* window coords */
	newlabel->u.bitmap.wx = (float)sx/basic->wwidth;
	newlabel->u.bitmap.wy = (float)sy/basic->wheight;
	break;
    }
    /* 
    ** insert it into the list 
    */
    newlabel->next = basic->label;
    basic->label = newlabel;
}

Label *FindLabel(basic,cx,cy)
BasicWindow	*basic;
short	cx,cy;
{
Label	*label;
Label	*current;
int	distance = -1;
int	ds;
int	sx,sy,sr;

    current = NULL;
    for(label=basic->label;label;label=label->next){
	switch(label->type){
	case STRING_LBL:
	    /*
	    ** get the coords
	    */
	    switch(label->coord_mode){
	    case WORLD_LBL:		/* world coord */
		ScreenTransform(basic,label->u.string.wx,label->u.string.wy,&sx,&sy);
		break;
	    case SCREEN_LBL:		/* screen coord */
		sx = label->u.string.sx;
		sy = label->u.string.sy;
		break;
	    case WINDOW_LBL:		/* window coord */
		sx = label->u.string.wx * basic->wwidth;
		sy = label->u.string.wy * basic->wheight;
		break;
	    }
	    break;
	case BOX_LBL:
	    /*
	    ** get the coords
	    */
	    switch(label->coord_mode){
	    case WORLD_LBL:		/* world coord */
		ScreenTransform(basic,label->u.box.wx,label->u.box.wy,&sx,&sy);
		sr = label->u.box.wr;
		break;
	    case SCREEN_LBL:		/* screen coord */
		sx = label->u.box.sx;
		sy = label->u.box.sy;
		sr = label->u.box.sr;
		break;
	    case WINDOW_LBL:		/* window coord */
		sx = label->u.box.wx*basic->wwidth;
		sy = label->u.box.wy*basic->wheight;
		sr = label->u.box.wr*basic->wwidth;
		break;
	    }
	    break;
	case LINE_LBL:
	    /*
	    ** get the coords
	    */
	    switch(label->coord_mode){
	    case WORLD_LBL:		/* world coord */
		ScreenTransform(basic,label->u.line.wx,label->u.line.wy,&sx,&sy);
		break;
	    case SCREEN_LBL:		/* screen coord */
		sx = label->u.line.sx;
		sy = label->u.line.sy;
		break;
	    case WINDOW_LBL:		/* window coord */
		sx = label->u.line.wx*basic->wwidth;
		sy = label->u.line.wy*basic->wheight;
		break;
	    }
	    break;
	}
	ds = sqrt((double)((sx-cx)*(sx-cx) + (sy-cy)*(sy-cy)));
	if(ds < distance || distance == -1){
	    distance = ds;
	    current = label;
	}
    }
    return(current);
}

DeleteLabel(basic,label)
BasicWindow *basic;
Label *label;
{
Label *l;

    if(label == NULL) return;
    if(label == basic->label){
	basic->label = basic->label->next;
	return;
    }
    for(l=basic->label;l;l=l->next){
	if(label == l->next){
	    l->next = l->next->next;
	}
    }
}
