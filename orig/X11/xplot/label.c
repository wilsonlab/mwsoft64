#include "xplot_ext.h"

int verbose_legend = 1;

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
	if(plot->escale != 1){
	    sprintf(tmp,"[es=%g]",plot->escale);
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
    case NORMAL_GRAPH:
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

    SetColor(graph->foreground);
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
    if(graph->xaxis.type == LOG10_AXIS){
	strcpy(label,"LOG ");
    }
    */
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

    SetColor(graph->foreground);
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
    if(graph->yaxis.type == LOG10_AXIS){
	strcpy(label,"LOG ");
    }
    */
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
