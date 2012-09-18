#include "xclust_ext.h"

InitializeSourceDefaults(graph,source)
DataSource	*source;
Graph		*graph;
{
    if(graph == NULL || source == NULL) return;
    source->defaults = (float *)calloc(MAXDEFAULTS,sizeof(float));
    bcopy(graph->defaults,source->defaults,MAXDEFAULTS*sizeof(float));
    source->defaults[LINESTYLE] = -1;
}


InitializeDefaults(graph)
Graph	*graph;
{
    graph->defaults[PLOTTYPE] = SCATTER_GRAPH;
    graph->defaults[POINTTYPE] = DOT_PT;
    graph->defaults[ZMODE] = NORMAL;
    graph->defaults[POINTFREQ] = 1;
    graph->defaults[SHOWPOINTS] = 0;
    graph->defaults[FOREGROUND] = graph->foreground;
    graph->defaults[BACKGROUND] = graph->background;
    graph->defaults[ESCALE] = 1;
    graph->defaults[LINESTYLE] = MININTERFACECOLOR + ZEROCLUSTER;
}

SetGraphDefault(graph,option,value)
Graph	*graph;
int	option;
float	value;
{
    if(graph == NULL || option < 0 || option >= MAXDEFAULTS){
	return;
    }
    graph->defaults[option] = value;
}

SetSourceDefault(source,option,value)
DataSource	*source;
int	option;
float	value;
{
    if(source == NULL || option < 0 || option >= MAXDEFAULTS){
	return;
    }
    source->defaults[option] = value;
}

SetSourceDefaultsFromPlot(plot,source)
Plot		*plot;
DataSource	*source;
{
    if(source == NULL || plot == NULL) return;
    source->defaults[SHOWPOINTS] = plot->showpoints;
    source->defaults[POINTFREQ] = plot->pointfreq;
    source->defaults[ZMODE] = plot->zmode;
    source->defaults[PLOTTYPE] = plot->plot_type;
    source->defaults[POINTTYPE] = plot->point_symbol;
    source->defaults[ESCALE] = plot->escale;
    source->defaults[LINESTYLE] = plot->linestyle;
    source->defaults[FOREGROUND] = WhitePixelIdx();
}


UseSourceDefaults(source,plot)
DataSource	*source;
Plot		*plot;
{
    if(source == NULL || plot == NULL) return;
    plot->showpoints = source->defaults[SHOWPOINTS];
    plot->pointfreq = source->defaults[POINTFREQ];
    plot->zmode = source->defaults[ZMODE];		/* 3d mode */
    plot->plot_type = source->defaults[PLOTTYPE];
    plot->point_symbol = source->defaults[POINTTYPE];
    plot->escale = source->defaults[ESCALE];
    plot->linestyle = source->defaults[LINESTYLE];
}

UseGraphDefaults(graph,plot)
Graph		*graph;
Plot		*plot;
{
    if(graph == NULL || plot == NULL) return;
    plot->showpoints = graph->defaults[SHOWPOINTS];
    plot->pointfreq = graph->defaults[POINTFREQ];
    plot->zmode = graph->defaults[ZMODE];		/* 3d mode */
    plot->plot_type = graph->defaults[PLOTTYPE];
    plot->point_symbol = graph->defaults[POINTTYPE];
    plot->escale = graph->defaults[ESCALE];
    plot->linestyle = graph->defaults[LINESTYLE];
}

ProcessDefaults(fp)
FILE	*fp;
{
Graph	*graph;
char	line[1001];

    if(fp == NULL) return;
    graph = GetGraph("/graph");
    /*
    ** if a defaults file was located then read it
    */
    while(!feof(fp)){
	if(fgets(line,1000,fp) == NULL) break;
	/*
	** process the command
	*/
	line[1000] = '\0';
	DoCommand(graph,SelectedPlot(graph),line);
    }
    fclose(fp);
}

ReadDefaults(frame)
Frame	*frame;
{
FILE	*fp;
char	*home;
char	dir[100];
char	*getenv();

    /*
    ** try to load in default values from the defaults file
    */
    fp = NULL;
    /*
    ** first try the home directory defaults file
    */
    if((home = getenv("HOME")) != NULL){
	sprintf(dir,"%s/.xclust3rc",home);
	if((fp = fopen(dir,"r")) != NULL){
	    fprintf(stderr,"\nReading global defaults file '%s'\n",dir);
	}
    }
    ProcessDefaults(fp);
    /*
    ** then try to open the local defaults file
    */
    if((fp = fopen(".xclust3rc","r")) == NULL){
    } else {
	fprintf(stderr,"\nReading local defaults file '%s'\n",".xclust3rc");
    }
    ProcessDefaults(fp);

    /* We have to call this here to refresh the bitmapped color
       buttons on the cluster menu after changing cluster colors in
       the .xclust3rc's*/
    RefreshSelectedClusterButton();
}
