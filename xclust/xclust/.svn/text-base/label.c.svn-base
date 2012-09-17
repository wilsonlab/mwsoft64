#include "xclust_ext.h"

int verbose_legend = 1;

Label *GetNamedLabel(basic,name)
BasicWindow	*basic;
char	*name;
{
Label	*label;

    /*
    ** go through all of the labels attached to the window
    */
    for(label=basic->label;label;label=label->next){
	/*
	** find the label with the matching name
	*/
	if(label->name && (strcmp(label->name,name) == 0)){
	    /*
	    ** and return it
	    */
	    return(label);
	}
    }
    return(NULL);
}

TextWindowLabel(label)
char *label;
{
    SetColor(F->text->foreground);
    FreeLabels(F->text);
    AddLabelString(F->text, label,
    0,F->text->fontheight,0.0,0.0,SCREEN_LBL,TEMPORARY_LBL,F->fontname);
    Text(F->text,0,F->text->fontheight,label);
}

DrawGraphTitle(graph)
Graph	*graph;
{
int	center;
int	tw,th;

    if(graph->title){
	SetColor (graph->foreground);
	TextExtent(graph->title,&tw,&th);
	center = (graph->wwidth - tw)/2;
	Text (graph,center , th*2, graph->title);
    }
}

DrawPlotLegend(plot,count)
Plot *plot;
int count;
{
int sx,sy;
char legend[200];
char tmp[80];
int	tw,th;


    if(plot->visible){
	SetColor (plot->linestyle);
    } else {
	SetColor (plot->graph->background);
    }
    legend[0] = '\0';
    if(plot->title){
	strcpy(legend,plot->title);
    }
    if(verbose_legend){
	if(plot->scale != 1){
	    sprintf(tmp,"[ys=%g]",plot->scale);
	    strcat(legend,tmp);
	}
	if(plot->xscale != 1){
	    sprintf(tmp,"[xs=%g]",plot->xscale);
	    strcat(legend,tmp);
	}
	if(plot->zscale != 1){
	    sprintf(tmp,"[zs=%g]",plot->zscale);
	    strcat(legend,tmp);
	}
	if(plot->escale != 1){
	    sprintf(tmp,"[es=%g]",plot->escale);
	    strcat(legend,tmp);
	}
	if(plot->cscale != 1){
	    sprintf(tmp,"[cs=%g]",plot->cscale);
	    strcat(legend,tmp);
	}
	if(plot->xoffset != 0){
	    sprintf(tmp,"[x=%g]",plot->xoffset);
	    strcat(legend,tmp);
	}
	if(plot->yoffset != 0){
	    sprintf(tmp,"[y=%g]",plot->yoffset);
	    strcat(legend,tmp);
	}
	if(plot->zoffset != 0){
	    sprintf(tmp,"[z=%g]",plot->zoffset);
	    strcat(legend,tmp);
	}
	if(plot->xoffset_axis != 0){
	    sprintf(tmp,"[xa=%g]",plot->xoffset_axis);
	    strcat(legend,tmp);
	}
	if(plot->yoffset_axis != 0){
	    sprintf(tmp,"[ya=%g]",plot->yoffset_axis);
	    strcat(legend,tmp);
	}
    }
    TextExtent(legend,&tw,&th);
    sx = plot->graph->wwidth - tw - 25;
    sy = th*(count+3);
    /*
    ** show the linestyle of the plot
    */
    switch(plot->plot_type){
    case LINE_GRAPH:
	DrawLine(plot->graph->wwidth - 20,sy,plot->graph->wwidth-5,sy);
	if(plot->showpoints){
	    DrawPointIcon(plot,plot->graph->wwidth - 13,sy);
	}
	break;
    case BAR_GRAPH:
	DrawLine(plot->graph->wwidth - 20,sy,plot->graph->wwidth-5,sy);
	break;
    case SCATTER_GRAPH:
	DrawPointIcon(plot,plot->graph->wwidth - 13,sy);
	break;
    }
    SetColor (plot->graph->foreground);
    /*
    ** show the name of the plot
    */
    Text (plot->graph,sx,sy,legend);
    /*
    ** indicate whether it is selected
    */
    if(plot->selected){
	Box(sx-8,sy-th/2-2,sx-4,sy-th/2+2);
    } else {
	/*
	** erase any previous selection
	*/
	SetColor (plot->graph->background);
	Box(sx-8,sy-th/2-2,sx-4,sy-th/2+2);
    }
}

DrawXAxisLabel(graph)
Graph	*graph;
{
char	label[80];
char	tmp[80];
int	tw,th;
int	sx,sy;

    SetColor(MININTERFACECOLOR + AXESLABELS);
    ScreenTransform(graph,graph->wxmax,graph->xaxis.yintcpt,&sx,&sy);
    /*
    ** calculate the axis scale factor
    CalculateAxisScale(graph->wxmin,graph->wxmax,
	graph->xaxis.leftdp,graph->xaxis.rightdp,&(graph->xaxis.exponent),
	graph->xaxis.nticks);
    */
    /*
    ** print the axis label
    */
    TextExtent(graph->xaxis.title,&tw,&th);
    /*
    ** center the axis title
    */
    if(graph->xtitlejustify == CENTERJUSTIFY){
	Text(graph,(graph->wwidth-tw)/2, sy + th*3 + graph->xaxis.title_offset,
	graph->xaxis.title);
    } else 
    if(graph->xtitlejustify == RIGHTJUSTIFY){
	Text(graph,(graph->wwidth-tw-1), sy + th*3 + graph->xaxis.title_offset,
	graph->xaxis.title);
    } else
    if(graph->xtitlejustify == LEFTJUSTIFY){
	Text(graph,1, sy + th*3 + graph->xaxis.title_offset,
	graph->xaxis.title);
    }
    label[0] = '\0';
    /*
    ** identify log axis
    */
    if(graph->xaxis.type == LOG10_AXIS){
	strcpy(label,"LOG ");
    }
    /*
    ** label the scale factor if any
    */
    if(graph->xaxis.exponent != 0){
	sprintf(tmp,"x10^%d",graph->xaxis.exponent);
	strcat(label,tmp);
    }
    if(strlen(label) > 0){
	TextExtent(label,&tw,&th);
	if(graph->xtitlejustify == RIGHTJUSTIFY){
	    Text(graph,sx - tw -10, sy + th*4,label);
	} else {
	    Text(graph,sx - tw -10, sy + th*3,label);
	}
    }
}

DrawYAxisLabel(graph)
Graph *graph;
{
char	label[80];
char	tmp[80];
int	tw,th;
int sx,sy;

    SetColor(MININTERFACECOLOR + AXESLABELS);
    ScreenTransform(graph,graph->yaxis.xintcpt,graph->wymin,&sx,&sy);
    /*
    ** calculate the axis scale factor
    CalculateAxisScale(graph->wymin,graph->wymax,
	graph->yaxis.leftdp,graph->yaxis.rightdp,&(graph->yaxis.exponent),
	graph->yaxis.nticks);
    */
    /*
    ** print the axis label
    */
    TextExtent(graph->yaxis.title,&tw,&th);
    Text(graph,sx +5 +graph->yaxis.title_offset, th, graph->yaxis.title);
    label[0] = '\0';
    /*
    ** identify log axis
    */
    if(graph->yaxis.type == LOG10_AXIS){
	strcpy(label,"LOG ");
    }
    /*
    ** label the scale factor if any
    */
    if(graph->yaxis.exponent != 0){
	sprintf(tmp,"x10^%d",graph->yaxis.exponent);
	strcat(label,tmp);
    }
    if(strlen(label) > 0){
	TextExtent(label,&tw,&th);
	Text(graph,sx + 5, th*2,label);
    }
}

ApplyPlotTransforms(plot,wx,wy,wz)
Plot	*plot;
double	*wx,*wy,*wz;
{
    if(plot->xscale > 0){
	*wx *= plot->xscale;
    }
    if(plot->scale > 0){
	*wy *= plot->scale;
    }
    if(plot->zscale > 0){
	*wz *= plot->zscale;
    }
    *wx += plot->xoffset;
    *wy += plot->yoffset;
    *wz += plot->zoffset;
}

DisplayPlotLabels(plot)
Plot		*plot;
{
BasicWindow	*basic;
int sx,sy,sr;
int sx2,sy2;
double	wx,wy,wz;
Label	*label;

    if(plot == NULL || (basic = (BasicWindow *)(plot->graph)) == NULL){
	return;
    }
    if(basic->hide_labels) return;
    for(label=plot->label;label;label=label->next){
	if(!label->visible) continue;
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
		    &wx,&wy,&wz);
		ApplyPlotTransforms(plot,&wx,&wy,&wz);
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
	    if(basic->display){
		XSetFont(basic->display,basic->context,label->u.string.font);
	    }
	    */
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
		    &wx,&wy,&wz);
		ApplyPlotTransforms(plot,&wx,&wy,&wz);
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
		    &wx,&wy,&wz);
		ApplyPlotTransforms(plot,&wx,&wy,&wz);
		ScreenTransform(basic,wx,wy, &sx,&sy);
		MatrixTransform(basic,label->u.line.wx2,
		    label->u.line.wy2,label->u.line.wz2,
		    &wx,&wy,&wz);
		ApplyPlotTransforms(plot,&wx,&wy,&wz);
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
		    &wx,&wy,&wz);
		ApplyPlotTransforms(plot,&wx,&wy,&wz);
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

DisplayLabels(basic)
BasicWindow	*basic;
{
int sx,sy,sr;
int sx2,sy2;
double	wx,wy,wz;
Label	*label;

    if(basic->hide_labels) return;
    for(label=basic->label;label;label=label->next){
	if(!label->visible) continue;
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
		    &wx,&wy,&wz);
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
	    
	    /* draw the text*/
	    if(basic->display){
		XSetFont(basic->display,basic->context,label->u.string.font);
	    }
	    
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
		    &wx,&wy,&wz);
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
		    &wx,&wy,&wz);
		ScreenTransform(basic,wx,wy, &sx,&sy);
		MatrixTransform(basic,label->u.line.wx2,
		    label->u.line.wy2,label->u.line.wz2,
		    &wx,&wy,&wz);
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
	    _DrawLine(basic,sx,sy,sx2,sy2);
	    break;
	case BITMAP_LBL:
	    /*
	    ** get the coords
	    */
	    switch(label->coord_mode){
	    case WORLD_LBL:		/* world coord */
		MatrixTransform(basic,(double)label->u.box.wx,
		    (double)label->u.box.wy,(double)label->u.box.wz,
		    &wx,&wy,&wz);
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
Label *AddLabelString(basic,s,sx,sy,wx,wy,coord_mode,priority,fontname)
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
    newlabel->visible = 1;
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
	fontname = DEFAULTFONT;
    }
    if(basic->display){
	newlabel->u.string.font = XLoadFont(basic->display,fontname);
	if (newlabel->u.string.font == BadName){
	    fprintf(stderr,"unable to open text font %s",fontname);
	    newlabel->u.string.font = XLoadFont(basic->display,DEFAULTFONT);
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
    return(newlabel);
}

/*
** append a label to plot
*/
Label *AddPlotLabelString(plot,s,sx,sy,wx,wy,coord_mode,priority,fontname)
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

    if(plot == NULL || (basic = (BasicWindow *)(plot->graph)) == NULL){
	return;
    }
    newlabel = (Label *)calloc(1,sizeof(Label));
    newlabel->type = STRING_LBL;
    newlabel->priority = priority;
    newlabel->visible = 1;
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
	fontname = DEFAULTFONT;
    }
    if(basic->display){
	newlabel->u.string.font = XLoadFont(basic->display,fontname);
	if (newlabel->u.string.font == BadName){
	    fprintf(stderr,"unable to open text font %s",fontname);
	    newlabel->u.string.font = XLoadFont(basic->display,DEFAULTFONT);
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
    return(newlabel);
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
Label *AddPlotLabelLine(plot,sx,sy,sy2,sx2,wx,wy,wz,wx2,wy2,wz2,coord_mode,priority)
Plot		*plot;
short		sx,sy;
short		sx2,sy2;
float		wx,wy,wz;
float		wx2,wy2,wz2;
short		coord_mode;
short		priority;
{
BasicWindow	*basic;
Label	*newlabel;

    if(plot == NULL || (basic = (BasicWindow *)(plot->graph)) == NULL){
	return;
    }
    newlabel = (Label *)calloc(1,sizeof(Label));
    newlabel->type = LINE_LBL;
    newlabel->priority = priority;
    newlabel->visible = 1;
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
    newlabel->next = plot->label;
    plot->label = newlabel;
    return(newlabel);
}

/*
** append a line label to the window
*/
Label *AddLabelLine(basic,sx,sy,sy2,sx2,wx,wy,wz,wx2,wy2,wz2,coord_mode,priority)
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
    newlabel->visible = 1;
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
    return(newlabel);
}

/*
** append a label to the plot
*/
Label *AddPlotLabelBox(plot,sx,sy,sr,wx,wy,wr,coord_mode,priority)
Plot		*plot;
short		sx,sy,sr;
float		wx,wy,wr;
short		coord_mode;
short		priority;
{
BasicWindow	*basic;
Label	*newlabel;

    if(plot == NULL || (basic = (BasicWindow *)(plot->graph)) == NULL){
	return;
    }
    newlabel = (Label *)calloc(1,sizeof(Label));
    newlabel->type = BOX_LBL;
    newlabel->priority = priority;
    newlabel->visible = 1;
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
    newlabel->next = plot->label;
    plot->label = newlabel;
    return(newlabel);
}

/*
** append a label to the window
*/
Label *AddLabelBox(basic,sx,sy,sr,wx,wy,wr,coord_mode,priority)
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
    newlabel->visible = 1;
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
    return(newlabel);
}

/*
** append a label to the window
*/
Label *AddLabelBitmap(basic,bitmap,sx,sy,wx,wy,w,h,coord_mode,priority)
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
    newlabel->visible = 1;
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
    return(newlabel);
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
