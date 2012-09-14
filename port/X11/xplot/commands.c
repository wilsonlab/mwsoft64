#include "xplot_ext.h"

extern char *strchr();

static argerror = 0;

char *Itoa(i)
int i;
{
char	tmp[100];
char	*copy;
    
    sprintf(tmp,"%d",i);
    copy = (char *)malloc(strlen(tmp)+1);
    strcpy(copy,tmp);
    return(copy);
}

char *Ftoa(f)
double f;
{
char	tmp[100];
char	*copy;
    
    sprintf(tmp,"%g",f);
    copy = (char *)malloc(strlen(tmp)+1);
    strcpy(copy,tmp);
    return(copy);
}

float Atof(s)
char *s;
{
    if(s == NULL){
	return(0);
    }
    return(atof(s));
}

int Atoi(s)
char *s;
{
    if(s == NULL){
	return(0);
    }
    return(atoi(s));
}

char *NextDelimiter(s,quote)
char *s;
int *quote;
{
    if(s == NULL) return(NULL);
    /*
    ** if the last delimiter was a quote find the closing one
    */
    if(*quote){
	while((s != NULL) && (*s != '"') && (*s != '\0')) s++;
	if(s == NULL){
	    return(NULL);
	}
	if(*s == '"') s++;
    }
    *quote = 0;
    for(;*s != '\0';s++){
	/*
	** look for a quote delimiter
	*/
	if(*s == '"'){
	    *quote = 1;
	    break;
	}
	/*
	** look for a space delimiter
	** which is the first white space followed by non-white space
	*/
	if((*s == ' ' || *s == '\t') && 
	    (*(s+1) != ' ' && *(s+1) != '\t' && *(s+1) != '"')){
	    break;
	}
    }
    return(s);
}

void Setenv(var,value)
char	*var,*value;
{
char *newenv;

    newenv = (char *)malloc(strlen(var) + strlen(value) + 4);
    strcpy(newenv,var);
    strcat(newenv," = ");
    strcat(newenv,value);

    if(putenv(newenv) == 0){
	fprintf(stderr,"%s=%s\n",var,value);
    } else {
	fprintf(stderr,"could not change %s\n",var);
    }
}

int CommandArgumentCount(line)
char 	*line;
{
int	i;
int	quote;

    quote = 0;
    /*
    ** advance to the correct argument
    */
    for(i=0;;i++){
	/*
	** locate the open delimiter
	*/
	line = NextDelimiter(line,&quote);
	if(line == NULL || *line == '\0'){
	    break;
	}
	/*
	** advance past the open delimiter
	*/
	line++;
    }
    return(i);
}
/* 
** return an argument string delimited by white space or by quotes
*/
char *CommandArgument(line,argnumber)
char 	*line;
int 	argnumber;
{
int	i;
int	count;
char	*ptr;
char	*ptr2;
char	*copy;
char	tmp[1000];
int	quote;
char	*LookupVariable();

    ptr = line;
    quote = 0;
    /*
    ** advance to the correct argument
    */
    for(i=0;i<argnumber;i++){
	/*
	** locate the open delimiter
	*/
	ptr = NextDelimiter(ptr,&quote);
	if(ptr == NULL){
	    return(NULL);
	}
	/*
	** advance past the delimiter
	*/
	ptr++;
    }
    /*
    ** copy the contents of the line up to the closing delimiter
    */
    for(ptr2=ptr,count=0;*ptr2 != '\0';ptr2++,count++){
	if(quote){
	    if(*ptr2 == '"') break;
	} else {
	    if(*ptr2 == ' ' || *ptr2 == '\t' || *ptr2 == '\n') break;
	}
    }
    if(count > 0){
	strncpy(tmp,ptr,count);
	tmp[count] = '\0';
#ifdef VARIABLES
	/*
	** check for variable notation
	*/
	if(tmp[0] == '$'){
	    ptr = LookupVariable(graph,plot,tmp+1);
	    if(ptr == NULL){
		fprintf(stderr,"invalid variable '%s'\n",tmp);
		return(NULL);
	    }
	} else {
	    ptr = tmp;
	}
#else
	ptr = tmp;
#endif
	copy = (char *)malloc((count+1)*sizeof(char));
	strcpy(copy,ptr);
    } else {
	copy = NULL;
	Beep();
	fprintf(stderr,"missing argument #%d %s",argnumber,line);
	argerror = 1;
    }
    return(copy);
}

#define Match(S) (strncmp(line,S,strlen(S)) == 0)
#define CP if(!p) return
#define CG if(!g) return
#define	ARG(N)	CommandArgument(line,N)
#define	ARGC	CommandArgumentCount(line)

/*
** interpret / commands
*/
InterpretCommand(g,plot,line)
Graph	*g;
Plot **plot;
char *line;
{
Plot	*p;
int	val;
int	fval;
extern int verbose_legend;
short sx,sy;
short sx2,sy2;
float wx,wy,wz;
float wx2,wy2,wz2;
char *s;
char	*fontname;

    if(debug){
	fprintf(stderr,"%s",line);
    }
    argerror = 0;
    p = *plot;
    if(Match("/assigncolors")){
	CG;
	AssignColors(g,Atoi(ARG(1)),Atoi(ARG(2)));
    } else
    if(Match("/showtitle")){
	CG;
	g->show_title = Atoi(ARG(1));
    } else
    if(Match("/graphtitle")){
	CG;
	g->title = ARG(1);
	RenameWindow(g->title);
    } else
    if(Match("/plotname")){
	CP;
	p->title = ARG(1);
    } else
    if(Match("/boldfont")){
	PSSetFont("/Helvetica-Bold");
    } else
    if(Match("/normalfont")){
	PSSetFont("/Helvetica");
    } else
    if(Match("/select")){
	CG;
	CP;
	if(strcmp(ARG(1),"?") == 0){
	    p = SelectedPlot(g);
	    if(p){
		TextWindowLabel(p->title);
		return(1);
	    }
	} else {
	    SelectPlot(g,Atoi(ARG(1)));
	    *plot = SelectedPlot(g);
	}
    } else
    if(Match("/optimizeplots")){
	CG;
	g->optimize = Atoi(ARG(1));
	return(1);
    } else
    if(Match("/optimizepoints")){
	CG;
	g->optimizepoints = Atoi(ARG(1));
	return(1);
    } else
    if(Match("/psres")){
	CG;
	g->resolution = Atoi(ARG(1));
	return(1);
    } else
    if(Match("/pscolor")){
	SetPSColor(Atoi(ARG(1)));
	return(1);
    } else
    if(Match("/psheader")){
	SetHeader(Atoi(ARG(1)));
	return(1);
    } else
    if(Match("/psbox")){
	SetBox(Atoi(ARG(1)));
	return(1);
    } else
    if(Match("/xlog")){
	CG;
	g->xaxis.type = Atoi(ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/ylog")){
	CG;
	g->yaxis.type = Atoi(ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/ticksize")){
	CG;
	g->ticksize = Atoi(ARG(1));
    } else
    if(Match("/xtitle")){
	CG;
	g->xaxis.title = ARG(1);
    } else
    if(Match("/ytitle")){
	CG;
	g->yaxis.title = ARG(1);
    } else
    if(Match("/legends")){
	CG;
	g->show_legend = Atoi(ARG(1));
    } else
    if(Match("/linewidth")){
	CG;
	SetLineWidth(Atoi(ARG(1)));
    } else
    if(Match("/xlabels")){
	CG;
	g->xaxis.show_labels = Atoi(ARG(1));
    } else
    if(Match("/xtickinc")){
	CG;
	g->xaxis.desired_tickinc = Atof(ARG(1));
    } else
    if(Match("/ytickinc")){
	CG;
	g->yaxis.desired_tickinc = Atof(ARG(1));
    } else
    if(Match("/xsubticks")){
	CG;
	g->xaxis.nsubticks = Atoi(ARG(1));
    } else
    if(Match("/ysubticks")){
	CG;
	g->yaxis.nsubticks = Atoi(ARG(1));
    } else
    if(Match("/xprecision")){
	CG;
	g->xaxis.rightdp = Atoi(ARG(1));
    } else
    if(Match("/yprecision")){
	CG;
	g->xaxis.rightdp = Atoi(ARG(1));
    } else
    if(Match("/ptext")){
	CG;
	CP;
	s = ARG(3);
	sx = 0;
	sy = 0;
	wx = Atof(ARG(1));
	wy = Atof(ARG(2));
	if(ARGC > 3){
	    fontname = ARG(4);
	} else {
	    fontname = g->fontname;
	}
	AddPlotLabelString(p,s,sx,sy,wx,wy,WORLD_LBL,PERMANENT_LBL,fontname);
    } else
    if(Match("/stext")){
	CG;
	s = ARG(3);
	wx = 0;
	wy = 0;
	sx = Atoi(ARG(1));
	sy = Atoi(ARG(2));
	if(ARGC > 3){
	    fontname = ARG(4);
	} else {
	    fontname = g->fontname;
	}
	AddLabelString(g,s,sx,sy,wx,wy,SCREEN_LBL,PERMANENT_LBL,fontname);
    } else
    if(Match("/wtext")){
	CG;
	s = ARG(3);
	sx = 0;
	sy = 0;
	wx = Atof(ARG(1));
	wy = Atof(ARG(2));
	if(ARGC > 3){
	    fontname = ARG(4);
	} else {
	    fontname = g->fontname;
	}
	AddLabelString(g,s,sx,sy,wx,wy,WORLD_LBL,PERMANENT_LBL,fontname);
    } else
    if(Match("/ylabels")){
	CG;
	g->yaxis.show_labels = Atoi(ARG(1));
    } else
    if(Match("/autox")){
	CP;
	p->auto_xstart = Atof(ARG(1));
	p->auto_x = Atof(ARG(2));
    } else
    if(Match("/xlo")){
	CP;
	p->xlo = Atoi(ARG(1));
	if(p->xlo < 0) p->xlo = 0;
	if(p->xlo >= p->npoints) p->xlo = p->npoints-1;
    } else
    if(Match("/xhi")){
	CP;
	p->xhi = Atoi(ARG(1));
	if(p->xhi >= p->npoints) p->xhi = p->npoints-1;
	if(p->xhi < 0) p->xhi = 0;
    } else
    if(Match("/xdatarange")){
	CG;
	if(ARGC < 1){
	    Beep();
	    fprintf(stderr,"usage: xdatarange [auto][xmin xmax]\n");
	    return;
	}
	if(strcmp(ARG(1),"auto") == 0){
	    g->xaxis.autoscale = TRUE;
	} else {
	    g->xmin = Atof(ARG(1));
	    g->xmax = Atof(ARG(2));
	    g->xaxis.autoscale = FALSE;
	}
	RescaleGraph(g);
    } else
    if(Match("/ydatarange")){
	CG;
	if(ARGC < 1){
	    Beep();
	    fprintf(stderr,"usage: ydatarange [auto][ymin ymax]\n");
	    return;
	}
	if(strcmp(ARG(1),"auto") == 0){
	    g->yaxis.autoscale = TRUE;
	} else {
	    g->ymin = Atof(ARG(1));
	    g->ymax = Atof(ARG(2));
	    g->yaxis.autoscale = FALSE;
	}
	RescaleGraph(g);
    } else
    if(Match("/wxmin")){
	CG;
	g->wxmin = Atof(ARG(1));
	SuppressWxmin();
	RescaleGraph(g);
    } else
    if(Match("/wxmax")){
	CG;
	g->wxmax = Atof(ARG(1));
	SuppressWxmax();
	RescaleGraph(g);
    } else
    if(Match("/wymin")){
	CG;
	g->wymin = Atof(ARG(1));
	SuppressWymin();
	RescaleGraph(g);
    } else
    if(Match("/wymax")){
	CG;
	g->wymax = Atof(ARG(1));
	SuppressWymax();
	RescaleGraph(g);
    } else
    if(Match("/xrange")){
	CG;
	g->wxmin = Atof(ARG(1));
	g->wxmax = Atof(ARG(2));
	SuppressWxmin();
	SuppressWxmax();
	RescaleGraph(g);
    } else
    if(Match("/yrange")){
	CG;
	g->wymin = Atof(ARG(1));
	g->wymax = Atof(ARG(2));
	SuppressWymin();
	SuppressWymax();
	RescaleGraph(g);
    } else
    if(Match("/xoffset_axis_ns")){
	CP;
	p->xoffset_axis = Atof(ARG(1));
    } else
    if(Match("/yoffset_axis_ns")){
	CP;
	p->yoffset_axis = Atof(ARG(1));
    } else
    if(Match("/xoffset_axis")){
	CP;
	p->xoffset_axis = Atof(ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/yoffset_axis")){
	CP;
	p->yoffset_axis = Atof(ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/xoffset")){
	CP;
	p->xoffset = Atof(ARG(1));
    } else
    if(Match("/yoffset")){
	CP;
	p->yoffset = Atof(ARG(1));
    } else
    if(Match("/yintcpt")){
	CG;
	g->xaxis.yintcpt = Atof(ARG(1));
    } else
    if(Match("/xintcpt")){
	CG;
	g->yaxis.xintcpt = Atof(ARG(1));
    } else
    if(Match("/yscale")){
	CP;
	p->scale = Atof(ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/xscale")){
	CP;
	p->xscale = Atof(ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/etype")){
	CP;
	p->errorbartype = Atoi(ARG(1));
    } else
    if(Match("/escale")){
	CP;
	p->escale = Atof(ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/pointstyle")){
	CP;
	CG;
	p->point_symbol = Atoi(ARG(1));
	p->showpoints = 1;
    } else
    if(Match("/longlegend")){
	CP;
	verbose_legend = Atoi(ARG(1));
    } else
    if(Match("/showerror")){
	CP;
	p->show_error = Atoi(ARG(1));
    } else
    if(Match("/showpoints")){
	CP;
	p->showpoints = Atoi(ARG(1));
    } else
    if(Match("/pointfreq")){
	CP;
	p->pointfreq = Atoi(ARG(1));
	if(p->pointfreq < 1) p->pointfreq = 1;
    } else
    if(Match("/showaxis")){
	CG;
	val = Atoi(ARG(1));
	if(val > 0){
	    g->quadrants |= (1 << (val-1));
	}
	if(val < 0){
	    g->quadrants &= ~(1 << (-val-1));
	}
    } else
    if(Match("/setenv")){
	Setenv(ARG(1),ARG(2));
	return(1);
    } else
    if(Match("/color")){
	CP;
	p->linestyle = Atoi(ARG(1));
    } else
    if(Match("/sline")){
	CG;
	sx = Atoi(ARG(1));
	sy = Atoi(ARG(2));
	sx2 = Atoi(ARG(3));
	sy2 = Atoi(ARG(4));
	AddLabelLine(g,sx,sy,sx2,sy2,wx,wy,wz,wx2,wy2,wz2,SCREEN_LBL,
	PERMANENT_LBL,fontname);
    } else
    if(Match("/wline")){
	CG;
	wx = Atof(ARG(1));
	wy = Atof(ARG(2));
	wz = Atof(ARG(3));
	wx2 = Atof(ARG(4));
	wy2 = Atof(ARG(5));
	wz2 = Atof(ARG(6));
	AddLabelLine(g,sx,sy,sx2,sy2,wx,wy,wz,wx2,wy2,wz2,WORLD_LBL,
	PERMANENT_LBL,fontname);
    } else
    if(Match("/disparityscale")){
	CG;
	g->disparityscale = Atof(ARG(1));
    } else
    if(Match("/xgrid")){
	CG;
	g->xaxis.show_grid = Atoi(ARG(1));
    } else
    if(Match("/ygrid")){
	CG;
	g->yaxis.show_grid = Atoi(ARG(1));
    } else
    if(Match("/gridcolor")){
	CG;
	g->gridcolor = Atoi(ARG(1));
    } else
    if(Match("/xrastaxis")){
	CP;
	p->xaxis = Atoi(ARG(1));
    } else
    if(Match("/yrastaxis")){
	CP;
	p->yaxis = Atoi(ARG(1));
    } else
    if(Match("/randomy")){
	CP;
	p->randomizey = Atoi(ARG(1));
	RescalePlot(p);
    } else
    if(Match("/randomx")){
	CP;
	p->randomizex = Atoi(ARG(1));
	RescalePlot(p);
    } else
    if(Match("/zmode")){
	CP;
	p->zmode = Atoi(ARG(1));
	if(p->zmode) p->show_error = 0;
    } else
    if(Match("/thetay")){
	CG;
	g->thetay = 2*M_PI*Atof(ARG(1))/360;
	UpdateRotationMatrix(g);
	RescaleGraph(g);
    } else
    if(Match("/thetaz")){
	CG;
	g->thetaz = 2*M_PI*Atof(ARG(1))/360;
	UpdateRotationMatrix(g);
	RescaleGraph(g);
    } else
    if(Match("/thetax")){
	CG;
	g->thetax = 2*M_PI*Atof(ARG(1))/360;
	UpdateRotationMatrix(g);
	RescaleGraph(g);
    } else
    if(Match("/shiftpercent")){
	SetShiftPercent(Atof(ARG(1)));
    } else
    if(Match("/readfile")){
	CG;
	LoadPlot(g,ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/deleteplot")){
	CP;
	DeletePlot(p);
    } else
    if(Match("/sort")){
	CG;
	Sort(g);
	RescaleGraph(g);
    } else
    if(Match("/quit")){
	Quit();
    } else
    if(Match("/bargraph")){
	CP;
	p->plot_type = BAR_GRAPH;
    } else
    if(Match("/line")){
	CP;
	p->plot_type = NORMAL_GRAPH;
    } else
    if(Match("/scatter")){
	CP;
	p->plot_type = SCATTER_GRAPH;
    } else
    if(Match("/barwidth")){
	CP;
	if(strcmp(ARG(1),"auto") == 0){
	    AutoBarWidth(p,Atof(ARG(2)));
	} else {
	    p->bar_width = Atof(ARG(1));
	}
    } else
    if(Match("/barfilled")){
	CP;
	p->bar_filled = 1;
    } else
    if(Match("/geometry")){
	CG;
	ChangeGeometry(ARG(1),g->frame);
	RescaleFrame(F);
    } else
    if(Match("/refresh")){
	CG;
	ScaleAndRefreshGraph(g);
    } else
    if(Match("/newplot")){
	CP;
	CG;
	ClosePlot(p);
	*plot = AddPlot(g);
    } else 
    if(Match("/pwd")){
	CG;
	TextWindowLabel(getcwd(NULL,200));
	return(1);
    } else
    if(Match("/closeplot")){
	CP;
	ClosePlot(p);
    } else 
    if(Match("/yraster")){
	fval = Atof(ARG(1));
	val = 0;
	for(p=g->plot;p;p=p->next){
	    p->yoffset_axis = fval*val;
	    val++;
	}
	RescaleGraph(g);
    } else
    if(Match("/xraster")){
	fval = Atof(ARG(1));
	val = 0;
	for(p=g->plot;p;p=p->next){
	    p->xoffset_axis = fval*val;
	    val++;
	}
	RescaleGraph(g);
    } else
    if(Match("/init")){
	InitGraph(G);
	if(F->mapped){
	    RescaleFrame(F);
	} else {
	    F->wwidth = F->width;
	    F->wheight = F->height;
	    PositionSubwindows(F);
	    RescaleGraph(G);
	}
    } else 
    if(Match("/snapmode")){
	SnapMode(Atoi(ARG(1)));
	return(1);
    } else
    if(Match("/xjustify")){
	CG;
	if(strcmp(ARG(1),"c") == 0){
	    g->xtitlejustify = CENTERJUSTIFY;
	} else
	if(strcmp(ARG(1),"r") == 0){
	    g->xtitlejustify = RIGHTJUSTIFY;
	} else
	if(strcmp(ARG(1),"l") == 0){
	    g->xtitlejustify = LEFTJUSTIFY;
	}
	PrintToFile(g,ARG(1),"w");
    } else
    if(Match("/fastsearch")){
	FastSearchMode(Atoi(ARG(1)));
    } else
    if(Match("/stepthrough")){
	CG;
	if(Atoi(ARG(1)) == 1){
	    g->stepthrough = g->plot;
	} else {
	    g->stepthrough = NULL;
	}
	return(1);
    } else
    if(Match("/printtofile")){
	CG;
	PrintToFile(g,ARG(1),"w");
    } else
    if(Match("/global")){
	SetGlobalOperation();
	return(1);
    } else
    if(Match("/savegraph")){
	CG;
	SaveGraph(g,ARG(1));
    } else
    if(Match("/restoregraph")){
	CG;
	RestoreGraph(g,ARG(1));
    } else
    if(Match("/appendtofile")){
	CG;
	PrintToFile(g,ARG(1),"a");
    } else
    if(Match("/print")){
	CG;
	PrintOut(g);
    } else
    if(Match("/wait")){
	if((val = sleep(Atoi(ARG(1)))) > 0){
	    fprintf(stderr,"%d unslept seconds\n",val);
	}
	return(1);
    } else
    if(Match("/autoxaxis")){
	CG;
	g->auto_xaxis = Atoi(ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/autoyaxis")){
	CG;
	g->auto_yaxis = Atoi(ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/autoscale")){
	CG;
	AutoScale(g,g->xaxis.autoscale,g->yaxis.autoscale);
	RescaleGraph(g);
    } else
    if(Match("/defaultcluster")){
	/*
	** no action, just a place holder for compatibility with
	** xclust plotfiles
	*/
    } else
    if(Match("/help")){
	Help();
	return(1);
    } else {
	if(line[0] != '\0'){
	    Beep();
	    fprintf(stderr,"xplot: unrecognized command : %s\n",line);
	}
    }
    return(0);
}

static helpcol;

HP(s)
char *s;
{
    if((helpcol++)%4 == 0){
	fprintf(stderr,"\n");
    }
    fprintf(stderr,"%-20s",s);
}

Help()
{
    fprintf(stderr,"AVAILABLE COMMANDS:\n");
    helpcol = 0;
    HP("appendtofile [file]");
    HP("assigncolors start inc");
    HP("autoorigin");
    HP("barfilled");
    HP("bargraph");
    HP("boldfont");
    HP("barwidth [f][auto percent]");
    HP("closeplot");
    HP("color [0..255]");
    HP("deleteplot");
    HP("disparityscale [f]");
    HP("escale [f]");
    HP("fastsearch [0/1]");
    HP("geometry [str]");
    HP("global");
    HP("gridcolor [0..255]");
    HP("graphtitle [s]");
    HP("init");
    HP("legends [0/1]");
    HP("line");
    HP("linewidth [i]");
    HP("longlegend [0/1]");
    HP("newplot");
    HP("normalfont");
    HP("optimizeplots [0/1]");
    HP("optimizepoints searchlen");
    HP("plotname [s]");
    HP("pointfreq [i]");
    HP("pointstyle [i]");
    HP("print");
    HP("printtofile [file]");
    HP("pscolor [0/1]");
    HP("psres [0/1]");
    HP("psbox [0/1]");
    HP("psheader [0/1]");
    HP("pwd");
    HP("quit");
    HP("readfile [s]");
    HP("refresh");
    HP("restoregraph [file]");
    HP("savegraph [file]");
    HP("scatter");
    HP("setenv [var value]");
    HP("select [plotnum]");
    HP("shiftpercent [0..1]");
    HP("showaxis [-4..4]");
    HP("showerror [0/1]");
    HP("showpoints [0/1]");
    HP("showtitle [0/1]");
    HP("sline [sx1][sy1][sx2][sy2]");
    HP("snapmode [0/1]");
    HP("sort");
    HP("stext [sx][sy][s][font]");
    HP("stepthrough [0/1]");
    HP("ticksize [i]");
    HP("thetax [f]");
    HP("thetay [f]");
    HP("thetaz [f]");
    HP("wait [sec]");
    HP("wline [wx1 wy1 wz1 wx2 wy2 wz2]");
    HP("wtext [wx][wy][s][font]");
    HP("wxmax [f]");
    HP("wxmin [f]");
    HP("wymin [f]");
    HP("wymax [f]");
    HP("xdatarange [f][f][auto]");
    HP("xgrid [0/1]");
    HP("xhi [i]");
    HP("xintcpt [f]");
    HP("xjustify [c/r/l]");
    HP("xlabels [0/1]");
    HP("xlog [0/1]");
    HP("xlo [i]");
    HP("xoffset [f]");
    HP("xoffsetaxis [f]");
    HP("xprecision [i]");
    HP("xrange [f][f]");
    HP("xrastaxis [0/1]");
    HP("xscale [f]");
    HP("xsubticks [i]");
    HP("xtickinc [f]");
    HP("xtitle [s]");
    HP("ylabels [0/1]");
    HP("ylog [0/1]");
    HP("ygrid [0/1]");
    HP("yscale [f]");
    HP("ysubticks [i]");
    HP("ytickinc [f]");
    HP("ytitle [s]");
    HP("ydatarange [f][f][auto]");
    HP("yintcpt [f]");
    HP("yrange [f][f]");
    HP("yoffset [f]");
    HP("yoffsetaxis [f]");
    HP("yprecision [i]");
    HP("yrastaxis [0/1]");
    HP("yraster [f]");
    HP("xraster [f]");
    HP("zmode [i]");
    fprintf(stderr,"\n");
}

char *LookupVariable(g,p,line)
Graph	*g;
Plot	*p;
char 	*line;
{
    if(Match("wxmin")){
	CP;
	CG;
	return(Ftoa(g->wxmin));
    } else 
    if(Match("wxmax")){
	CP;
	CG;
	return(Ftoa(g->wxmax));
    } else 
    if(Match("wymin")){
	CP;
	CG;
	return(Ftoa(g->wymin));
    } else 
    if(Match("wymax")){
	CP;
	CG;
	return(Ftoa(g->wymax));
    } else 
    return(NULL);
}

