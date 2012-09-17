#include "xclust_ext.h"

extern char *strchr();
/* extern void SetRotationMatrix(double***, float, float, float); */
extern void AffineRotationMatrixEuler(double***, float, float, float);
int skipevent = 0;

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
	if(s == NULL ||
	   *s == '\0'){
	  /* unmatched quote */
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
	    ((*(s+1) != ' ') && 
	     (*(s+1) != '\t') && 
	     (*(s+1) != '"') && 
	     (*(s+1) != '\0'))) {
	    break;
	}
    }
    return(s);
}

char *Setenv(var,value)
char	*var,*value;
{
char *newenv;

    newenv = (char *)malloc(strlen(var) + strlen(value) + 2);
    sprintf(newenv,"%s=%s",var,value);

    if(putenv(newenv) != 0){
	fprintf(stderr,"could not set/change %s\n",var);
	return(NULL);
    }
    return(newenv);
}

int CommandArgumentCount(line)
char 	*line;
{
int	i;
int	quote;
char    *nxtcommand;

    quote = 0;

    /* only count arguments up to the next semicolon (or end of
    ** string) */
    sscanf(line,"%a[^;]",&nxtcommand); 

    /*
    ** advance to the correct argument
    */
    for(i=0;;i++){
	/*
	** locate the open delimiter
	*/
	nxtcommand = NextDelimiter(nxtcommand,&quote);
	if(nxtcommand == NULL || *nxtcommand == '\0'){
	    break;
	}
	/*
	** advance past the open delimiter
	*/
	nxtcommand++;
    }
    if(!quote)
      return(i);

    /* unmatched quotes */
    fprintf(stderr,"ERROR: Unmatched quotes in arguments, last arg ignored: '%s'\n",line);
    argerror = 1;
    return(i-1);
}
/* 
** return an argument string delimited by white space or by
** quotes. The empty string "" is a valid argument.
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
	if(*ptr2 == '"'){ /* close quotes */
	  /* handle special case of "" empty string argument */
	  if (count == 0)
	    return("");
	  else {
	    quote = 0;
	    break;
	  }
	}
      } else {
	if(*ptr2 == ' ' || *ptr2 == '\t' || *ptr2 == '\n') break;
      }
    }

    /* test for unmatched quotes */
    if(quote){
      fprintf(stderr,"ERROR: Unmatched quotes at argument %d, in command '%s'\n",argnumber, line);
      argerror = 1;
      return(NULL);
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
	fprintf(stderr,"ERROR: missing argument #%d '%s'\n",argnumber,line);
	argerror = 1;
    }
    return(copy);
}

/* #define Match(S) (strncmp(line,S,strlen(S)) == 0) */
#define Match(S) (strlen(S) == strcspn(line," ;\t\n") && strncmp(line,S,strlen(S)) == 0)
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
ProjectionInfo  *pinfo;
int	val;
int	fval;
extern int verbose_legend;
short sx,sy,sx2,sy2;
float wx,wy,wz,wx2,wy2,wz2;
char *s;
char	*fontname;
Plot	*PartialLoadPlot();
DataSource	*source;
MenuItem	*item;
char		tmps[100];
unsigned short red,green,blue;
int	sv;
int	i;

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
    if(Match("/refreshtime")){
	CP;
	p->refreshtime= Atof(ARG(1));
    } else
    if(Match("/boldfont")){
	PSSetFont("/Helvetica-Bold");
    } else
    if(Match("/normalfont")){
	PSSetFont("/Helvetica");
    } else
    if(Match("/linewidth")){
	CG;
	SetLineWidth(Atoi(ARG(1)));
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
    if(Match("/psheader")){
	SetHeader(Atoi(ARG(1)));
	return(1);
    } else
    if(Match("/psbox")){
	SetBox(Atoi(ARG(1)));
	return(1);
    } else
    if(Match("/pscolor")){
	SetPSColor(Atoi(ARG(1)));
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
    if(Match("/xlabels")){
	CG;
	g->xaxis.show_labels = Atoi(ARG(1));
    } else
    if(Match("/xtickinc")){
	CG;
	g->xaxis.desired_tickinc = Atof(ARG(1));
    } else
    if(Match("/xnticks")){
	CG;
	g->xaxis.desired_nticks = Atof(ARG(1));
    } else
    if(Match("/ytickinc")){
	CG;
	g->yaxis.desired_tickinc = Atof(ARG(1));
    } else
    if(Match("/ynticks")){
	CG;
	g->yaxis.desired_nticks = Atof(ARG(1));
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
    if(Match("/plotstext")){
	CG;
	CP;
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
	AddPlotLabelString(p,s,sx,sy,wx,wy,SCREEN_LBL,PERMANENT_LBL,fontname);
    } else
    if(Match("/plotwtext")){
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
    if(Match("/sline")){
	CG;
	sx = Atoi(ARG(1));
	sy = Atoi(ARG(2));
	sx2 = Atoi(ARG(3));
	sy2 = Atoi(ARG(4));
	AddLabelLine(g,sx,sy,sx2,sy2,wx,wy,wz,wx2,wy2,wz2,SCREEN_LBL,
	PERMANENT_LBL,fontname);
    } else
    if(Match("/plotsline")){
	CG;
	CP;
	sx = Atoi(ARG(1));
	sy = Atoi(ARG(2));
	sx2 = Atoi(ARG(3));
	sy2 = Atoi(ARG(4));
	AddPlotLabelLine(p,sx,sy,sx2,sy2,wx,wy,wz,wx2,wy2,wz2,SCREEN_LBL,
	PERMANENT_LBL,fontname);
    } else
    if(Match("/plotwline")){
	CG;
	CP;
	wx = Atof(ARG(1));
	wy = Atof(ARG(2));
	wz = Atof(ARG(3));
	wx2 = Atof(ARG(4));
	wy2 = Atof(ARG(5));
	wz2 = Atof(ARG(6));
	AddPlotLabelLine(p,sx,sy,sx2,sy2,wx,wy,wz,wx2,wy2,wz2,WORLD_LBL,
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
	if(ARGC < 1){
	    sprintf(tmps,"%d",p->xlo);
	    TextWindowLabel(tmps);
	} else {
	    p->xlo = Atoi(ARG(1));
	    if(p->xlo < 0) p->xlo = 0;
	    if(p->xlo >= p->npoints) p->xlo = p->npoints-1;
	}
    } else
    if(Match("/xhi")){
	CP;
	if(ARGC < 1){
	    sprintf(tmps,"%d",p->xhi);
	    TextWindowLabel(tmps);
	} else {
	    p->xhi = Atoi(ARG(1));
	    if(p->xhi >= p->npoints) p->xhi = p->npoints-1;
	    if(p->xhi < 0) p->xhi = 0;
	}
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
    if(Match("/squarerange")){
	CG;
	if (g->wymin < g->wxmin) {
	  g->wxmin = g->wymin;
	} else {
	  g->wymin = g->wxmin;
	}
	if (g->wymax > g->wxmax) {
	  g->wxmax = g->wymax;
	} else {
	  g->wymax = g->wxmax;
	}
	SuppressWymin();
	SuppressWymax();
	SuppressWxmin();
	SuppressWxmax();
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
    if(Match("/zoffset")){
	CP;
	p->zoffset = Atof(ARG(1));
    } else
    if(Match("/coffset")){
	CP;
	p->coffset = Atof(ARG(1));
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
    if(Match("/zscale")){
	CP;
	p->zscale = Atof(ARG(1));
	RescaleGraph(g);
    } else
    if(Match("/escale")){
	CP;
	p->escale = Atof(ARG(1));
	RescaleGraph(g);
/*     } else */
/*     if(Match("/cscale")){ */
/* 	CP; */
/* 	p->cscale = Atof(ARG(1)); */
    } else
    if(Match("/pointstyle")){
	CP;
	CG;
	p->point_symbol = Atoi(ARG(1));
	p->showpoints = 1;
    } else
    if(Match("/pointscale")){
	CP;
	CG;
	p->pointscale = Atof(ARG(1));
    } else
    if(Match("/longlegend")){
	CP;
	verbose_legend = Atoi(ARG(1));
    } else
    if(Match("/showpoints")){
	CP;
	p->showpoints = Atoi(ARG(1));
    } else
      if(Match("/rotorigin")){
	CG;
	g->rotorigin = !Atoi(ARG(1));
    } else
      if(Match("/rotpointfreq")){
	CG;
	g->rotpointfreq = Atoi(ARG(1));
	if(g->rotpointfreq < 1) 
	  g->rotpointfreq = 1;
    } else
	if(Match("/rotpointsmax")){
	  CG;
	  if (Atoi(ARG(1)) > g->plot->npoints)
	    g->rotpointfreq = 1;
	  else 
	    g->rotpointfreq = ceil((double)(g->plot->npoints) / (double)(Atoi(ARG(1))));	  
    } else
    if(Match("/pointfreq")){
	CP;
	p->pointfreq = Atoi(ARG(1));
	if(p->pointfreq < 1) p->pointfreq = 1;
	RescaleGraph(g);
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
    if(Match("/setmenu")){
      if(ARGC != 2){
	fprintf(stderr,"ERROR: missing arguments (usage: /setmenu <menu item name> \"<menu value>\")\n");
	return(1);
      }
      if(PutItemValue(ARG(1),ARG(2)))
	if ((item = LookupMenuItem(ARG(1))))
	  DrawItem(item);
      return(1);
    } else
    if(Match("/color")){
	CP;
	p->linestyle = Atoi(ARG(1));
    } else
    if(Match("/setcolor")){
	CG;
	SetForeground(g,Atoi(ARG(1)));
    } else
    if(Match("/clustercolor")){
	CG;
	SetClusterColor(g,Atoi(ARG(1)),Atoi(ARG(2)));
	AssignClusterButtons();
    } else
    if(Match("/showrgb")){
	GetPixelRGB(Atoi(ARG(1)),
	&red,&green,&blue);
	fprintf(stderr,"color table entry %d: %d %d %d\n",Atoi(ARG(1)),
	red,green,blue);
    } else
      /* we don't allow direct manipulation of the colormap anymore */
/*     if(Match("/clusterrgb")){ */
/* 	SetPixelRGB(MINCLUSTER+Atoi(ARG(1)), */
/* 	(unsigned short)(Atoi(ARG(2)) << 8), */
/* 	(unsigned short)(Atoi(ARG(3)) << 8), */
/* 	(unsigned short)(Atoi(ARG(4)) << 8)); */
/*     } else */
/*     if(Match("/rgb")){ */
/* 	SetPixelRGB(Atoi(ARG(1)), */
/* 	(unsigned short)(Atoi(ARG(2)) << 8), */
/* 	(unsigned short)(Atoi(ARG(3)) << 8), */
/* 	(unsigned short)(Atoi(ARG(4)) << 8)); */
/*     } else */
    
    if(Match("/loadview")){
      switch(ARGC){
      case 8:
	/* load the saved view */
	LoadViewDefaults(G,
			 Atoi(ARG(1)), /* view index */
			 Atoi(ARG(2)), /* X proj # */
			 Atoi(ARG(3)), /* Y proj # */
			 Atoi(ARG(4)), /* Z proj # */
			 Atof(ARG(5)), /* X range min */
			 Atof(ARG(6)), /* Y range min */
			 Atof(ARG(7)), /* X range max */
			 Atof(ARG(8))); /* Y range max */
	break;
      case 7: /* X and Y only, Xrange, Yrange (for backwards compatibility)*/
	LoadViewDefaults(G,
			 Atoi(ARG(1)), /* view index */
			 Atoi(ARG(2)), /* X proj # */
			 Atoi(ARG(3)), /* Y proj # */
			 -1,           /* No Z proj */
			 Atof(ARG(4)), /* X range min */
			 Atof(ARG(5)), /* Y range min */
			 Atof(ARG(6)), /* X range max */
			 Atof(ARG(7)));/* Y range max */
	break;
      default: 
	/* bad # of args */
	fprintf(stderr,
		"ERROR: /loadview takes 7 or 8 args, not %d (/loadview index px py [pz] xmin ymin xmax ymax) : %s\n",
		ARGC,line);
      } /* end switch */
    } else
    if(Match("/prevsavedview")){

      if (g->lastsavedview == 0) /* first attempt to load a saved view */
	g->lastsavedview = 2;

      for (sv=g->lastsavedview, SAVEDVIEWDEC(sv); /* cyclical decrement over range */
	   sv != (g->lastsavedview);
	   SAVEDVIEWDEC(sv)) { /* cyclical decrement over range 2-12 */
	
	if(ProcessSavedView(g, 0, sv)){

	  /* ProcessSavedView failed: skip to next */
	  continue;
	}
	/* successfully loaded saved view, return */
	else return;
      }
      if(!suppresswarnings)
	fprintf(stderr,"WARNING: No valid saved views found (/prevsavedview)\n");
      return;
    } else      

    if(Match("/nextsavedview")){

      if (g->lastsavedview == 0) /* first attempt to load a saved view */
	g->lastsavedview = 12; /* will be incremented to 2 */


      /* there are only saved views in F2-F12 */
      for (sv=g->lastsavedview, SAVEDVIEWINC(sv); /* cyclical increment over range 2-12*/
	   sv != g->lastsavedview;
	   SAVEDVIEWINC(sv)) { 
	
	if(ProcessSavedView(g, 0, sv)){

	  /* ProcessSavedView failed: skip to next */
	  continue;
	}
	/* successfully loaded saved view, return */
	else return;
      }
      if(!suppresswarnings)
	fprintf(stderr,"WARNING: No valid saved views found (/nextsavedview)\n");
      return;
 
   } else      
    if(Match("/selectemptycluster")){
      for (i=1; i < MAXCLUSTER; i++)
	/* is this cluster empty? */
	if (!GetClusterInfo(i)){
	  SelectCluster(i);
	  break;
	}
    } else
    if(Match("/selectnextenabledcluster")){
      for (i=1; i < MAXCLUSTER; i++)
	/* is this cluster non-empty? */
	if (GetClusterInfo((G->selectedcluster+i)%MAXCLUSTER)){
	  SelectCluster((G->selectedcluster+i)%MAXCLUSTER);
	  break;
	}
    } else
    if(Match("/selectprevenabledcluster")){
      for (i = MAXCLUSTER-1; i > 0; i--)
	/* is this cluster non-empty? */
	if (GetClusterInfo((G->selectedcluster+i)%MAXCLUSTER)){
	  SelectCluster((G->selectedcluster+i)%MAXCLUSTER);
	  break;
	}
    } else
    if(Match("/selectnextcluster")){
      SelectCluster((G->selectedcluster+1)%MAXCLUSTER);
    } else
    if(Match("/selectprevcluster")){
	  SelectCluster((G->selectedcluster+MAXCLUSTER-1)%MAXCLUSTER);
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
	    SelectPlotByNumber(g,Atoi(ARG(1)));
	    *plot = SelectedPlot(g);
	}
    } else
    if(Match("/flash")){
      CG;
      if (ARGC == 0){ 
	/* with no arguments, toggle flash */
	CallMenuItem("/controlmenu/flashcluster");
	return;
      }
      /* do we need to change the state ?*/
      if (rotatecolor != (Atoi(ARG(1)) != 0))
	CallMenuItem("/controlmenu/flashcluster");	

    } else
    if(Match("/defaultcluster")){
	CG;
	g->defaultcluster = Atoi(ARG(1));
    } else
    if(Match("/nextspike")){
	CG;
	CallMenuItem("/controlmenu/forwardstep");
    } else
    if(Match("/prevspike")){
	CG;
	CallMenuItem("/controlmenu/backwardstep");
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
    if(Match("/zmode")){
	CP;
	p->zmode = Atoi(ARG(1));
    } else
    if(Match("/thetay")){
	CG;
	g->thetay = 2*M_PI*Atof(ARG(1))/360;
/* 	SetRotationMatrix(&g->matrix, g->thetax, g->thetay, g->thetaz); */
	AffineRotationMatrixEuler(&g->matrix, g->thetax, g->thetay, g->thetaz);
	RescaleGraph(g);
    } else
    if(Match("/thetaz")){
	CG;
	g->thetaz = 2*M_PI*Atof(ARG(1))/360;
/* 	SetRotationMatrix(&g->matrix, g->thetax, g->thetay, g->thetaz); */
	AffineRotationMatrixEuler(&g->matrix, g->thetax, g->thetay, g->thetaz);
	RescaleGraph(g);
    } else
    if(Match("/thetax")){
	CG;
	g->thetax = 2*M_PI*Atof(ARG(1))/360;
/* 	SetRotationMatrix(&g->matrix, g->thetax, g->thetay, g->thetaz); */
	AffineRotationMatrixEuler(&g->matrix, g->thetax, g->thetay, g->thetaz);
	RescaleGraph(g);
    } else
    if(Match("/shiftpercent")){
	SetShiftPercent(Atof(ARG(1)));
    } else
    if(Match("/zoompercent")){
	SetZoomPercent(Atof(ARG(1)));
    } else
    if(Match("/readfile")){
	CG;
	LoadPlot(g,ARG(1),FULL_LOAD,0,0,-1,-1);
	RescaleGraph(g);
    } else
    if(Match("/partialplotreadfile")){
	CG;
	LoadPlot(g,ARG(1),PARTIAL_PLOT_LOAD,-1,-1,Atoi(ARG(2)),Atoi(ARG(3)));
	RescaleGraph(g);
    } else
    if(Match("/partialreadfile")){
	CG;
	LoadPlot(g,ARG(1),PARTIAL_LOAD,Atoi(ARG(2)),Atoi(ARG(3)),-1,-1);
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
	SetSourceDefault(p->source,PLOTTYPE,p->plot_type);
    } else
    if(Match("/line")){
	CP;
	p->plot_type = LINE_GRAPH;
	SetSourceDefault(p->source,PLOTTYPE,p->plot_type);
    } else
    if(Match("/scatter")){
	CP;
	p->plot_type = SCATTER_GRAPH;
	SetSourceDefault(p->source,PLOTTYPE,p->plot_type);
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
	RefreshGraph(g);
    } else
    if(Match("/grab")){
	CP;
	CG;
	/*
	** give the new plot the same data source as the old plot
	*/
	source = p->source;
	*plot = AddPlot(g,source);
	SelectPlot(*plot);
	RefreshGraph(g);
    } else 
    if(Match("/newplot")){
	CP;
	CG;
	/*
	** give the new plot the same data source as the old plot
	*/
	source = p->source;
	ClosePlot(p);
	/*
	** store the file offset corresponding to the 
	** beginning of the plot
	*/
	source->currentplot++;
	SavePlotOffset(source,source->currentplot,ftell(source->fp),
	source->currentline);
	*plot = AddPlot(g,source);
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
/*     if(Match("/printtofile")){ */
/* 	CG; */
/* 	PrintToFile(g,ARG(1),"w"); */
/*     } else */
    if(Match("/global")){
	SetGlobalOperation();
	return(1);
    } else
    if(Match("/autosavedelay")){
	ChangeIntermittentSaveDelay((int)(60e2*Atof(ARG(1))));
    } else
    if(Match("/appendtofile")){
	CG;
	PrintToFile(g,ARG(1),"a");
    } else
    if(Match("/print")){
	CG;
	PrintOut(g);
    } else
/*      if(Match("/simacquire")){ */
/*  	CG; */
/*  	SimulateDataAcquisition(g,ARG(1),ARG(2),Atoi(ARG(3))); */
/*  	return(1); */
/*     } else */
    if(Match("/wait")){
	if((val = sleep(Atoi(ARG(1)))) > 0){
	    fprintf(stderr,"%d unslept seconds\n",val);
	}
	return(1);
    } else
    if(Match("/autoorigin")){
	CG;
	AutoOrigin(g);
	RescaleGraph(g);
    } else
    if(Match("/autoscale")){
	CG;
	AutoScale(g,g->xaxis.autoscale,g->yaxis.autoscale);
	RescaleGraph(g);
    } else
    if(Match("/cluster")||
       Match("/selectcluster")){
	CG;
	SelectCluster(Atoi(ARG(1)));
    } else
    if((Match("/int")) ||(Match("/csi"))){
	CG;
	MenuCSI();
    } else
    if(Match("/csall")){
	CG;
	MenuAllCSI();
    } else
    if(Match("/projection")){
	CG;

	if(pinfo = GetProjectionInfo(g,g->currentprojection[0])){
	  pinfo->wmin = g->wxmin;
	  pinfo->wmax = g->wxmax;
	}
	if(pinfo = GetProjectionInfo(g,g->currentprojection[1])){
	  pinfo->wmin = g->wymin;
	  pinfo->wmax = g->wymax;
	}

	g->currentprojection[0] = Atoi(ARG(1));
	g->currentprojection[1] = Atoi(ARG(2));
	g->selectedprojection[0] = -1;
	g->selectedprojection[1] = -1;

	/* set button states to reflect new view */
	AssignProjectionButtons();

	UpdateProjections();
	RescaleGraph(g);
    } else
    if(Match("/zprojection")){
	CG;
	g->currentprojection[2] = Atoi(ARG(1));

	/* set button states to reflect new view */
	AssignProjectionButtons();

	RescaleGraph(g);
    } else
    if(Match("/randomizeprojectionbyname")){
	CG; 
	if (ARGC < 2){
	  fprintf(stderr,
		  "ERROR: /randomizeprojectionbyname takes at least 2 args (/randomizepojectionbyname <projection name> ... 0/1 ) : %s\n",
		  ARGC,line);
	  return;
	}
	
	/* iterate over projections to be set */
	for (i=1;i<ARGC; i++){
	  if((pinfo = GetProjectionInfoByName(G,ARG(i))) == NULL) {
	    if (!suppresswarnings){
	      fprintf(stderr, "WARNING: projection '%s' does not exist; skipping (/randomizeprojectionbyname)\n", ARG(i));
	    }
	    continue;
	  }

	  /* set the randomization of the projection */
	  pinfo->randomize = Atoi(ARG(ARGC));
	}

	/* update the projection button grid */
	AssignProjectionButtons();

	/* redraw the points, if necessary */
	if (pinfo &&
	    (pinfo->projectionid == g->currentprojection[0] ||
	     pinfo->projectionid == g->currentprojection[1]))
	  ScaleAndRefreshGraph(g);
    } else
    if(Match("/randomizeprojection")){
	CG; 
	if (ARGC < 2){
	  fprintf(stderr,
		  "ERROR: /randomizeprojection takes at least 2 args (/randomizepojection <projection index> ... 0/1 ) : %s\n",
		  ARGC,line);
	  return;
	}

	/* iterate over projections to be set */
	for (i=1;i<ARGC;i++){
	  if((pinfo = GetProjectionInfo(G,Atoi(ARG(i)))) == NULL) {
	    fprintf(stderr,
		    "WARNING: projection number '%s' does not exist in this session (/randomizeprojection)\n",
		    ARG(i));
	    continue;
	  }
	
	  /* set the randomization of the projection */
	  pinfo->randomize = Atoi(ARG(ARGC));
	}
	
	/* update the projection button grid */
	AssignProjectionButtons();

	/* redraw the points, if necessary */
	if (pinfo->projectionid == g->currentprojection[0] ||
	    pinfo->projectionid == g->currentprojection[1])
	  ScaleAndRefreshGraph(g);

    } else            
    if(Match("/partialplotload")){
	CG;
	p = PartialLoadPlot(g,p,-1,-1,Atoi(ARG(1)),Atoi(ARG(2)));
	RescaleGraph(g);
    } else
    if(Match("/partialload")){
	CG;
	p = PartialLoadPlot(g,p,Atoi(ARG(1)),Atoi(ARG(2)),-1,-1);
	RescaleGraph(g);
    } else
    if(Match("/deleteclusterbox")){
	CG;
	DoDeleteClusterBounds(g);
    } else
    if(Match("/overlay")){
	CG;
	g->overlay = Atoi(ARG(1));
    } else
    if(Match("/move")){
	MoveMenuWindow(Atoi(ARG(1)));
    } else
    if(Match("/deletecluster")){
	CG;
	DoDeleteCluster(g);
    } else
    if(Match("/readbounds")){
	CG;
	ReadClusterBounds(g,ARG(1));
    } else
    if(Match("/evaloverlap")){
	CG;
	EvaluateOverlap(g);
    } else
    if(Match("/listjobs")){
	ListJobs();
    } else
    if(Match("/epoch")){
      if(ARGC == 4){
	val = Atoi(ARG(1));
	if(val > 0 && val <= NUM_EPOCHS){
	  SetEpochValues(Atoi(ARG(1)),ARG(2),ARG(3),ARG(4));
	  return(1);
	} else /* epoch out of range */
	  fprintf(stderr,
		  "ERROR: epoch # %d out of range (1 - %d): %s\n",
		  val, NUM_EPOCHS,line);
      } else /* failed ARGC count */
	fprintf(stderr,
		"ERROR: epoch takes 4 args, not %d (/epoch [#] [name] [start] [end]) : %s\n",
		ARGC,line);


    } else 
    if(Match("/loadepoch")){
      switch (ARGC){
      case 0: /* no file argument, load from button */
	fprintf(stderr, "Loading epoch file specified in menu button.\n");
	CallMenuItem("/epochmenu/epochfile");
	break;
      case 1:
	if(s = ARG(1)){
	  fprintf(stderr, "Loading epoch file '%s'.\n",s);
	  LoadEpochFile(s);
	}
	break;
      default:
	fprintf(stderr,
		"ERROR: /loadepoch takes 1 arg, not %d (/loadepoch [filename]) : %s\n",
		ARGC,line);
	break;
      }
      return(1);
    } else
    if(Match("/saveepoch")){
      switch (ARGC){
      case 0: /* no file argument, load from button */
	fprintf(stderr, "Saving epoch file specified in menu button.\n");
	CallMenuItem("/epochmenu/saveepochs");
	break;
      case 1:
	if(s = ARG(1)){
	  fprintf(stderr, "Saving epoch file '%s'.\n",s);
	  SaveEpochFile(s);
	}
	break;
      default:
	fprintf(stderr,
		"ERROR: /saveepoch takes 1 arg, not %d (/saveepoch [filename]) : %s\n",
		ARGC,line);
	break;
      }
      return(1);
    } else
    if (Match("/defepoch")){
      switch (ARGC) {
      case 0: /* no argument, no cluster name */
	DefineNewEpoch("");
	break;
      case 1: /* cluster name specified */
	if (s = ARG(1)){
	  DefineNewEpoch(s);
	}
	break;
      default:
	fprintf(stderr,
		"ERROR: /defepoch takes 0 or 1 args, not %d (/defepoch [epochname=""]) : %s\n",
		ARGC,line);
	break;
      }
    } else 
    if(Match("/help")){
	Help();
	return(1);
    } else {
	if(line[0] != '\0'){
	    Beep();
	    fprintf(stderr,"unrecognized command : %s\n",line);
	}
    }
    return(0);
}

ListJobs()
{
Job	*job;

    fprintf(stderr,"Active Jobs:\n");
    for(job=F->joblist;job;job=job->next){
	fprintf(stderr,"\t[%d] priority %d: %s\n",
	job->id,
	job->priority,
	job->description);
    }
}

static helpcol;

HP(s)
char *s;
{

  int strlength, nreqcols;
  char formatstr[20];

  if (s==NULL) {
    fprintf(stderr,"\n");
    helpcol = 0;
    return;
  }

  strlength = strlen(s);
  nreqcols = ceil((float)strlength/20);

  if (nreqcols>4)
    nreqcols = 4;

  sprintf(formatstr, "%%-%d.%ds", nreqcols*20, nreqcols*20);

 /*  fprintf(stderr, "\nnreqcols: %d, strlen: %d, format: %s, cols: %d\n", nreqcols, strlength, formatstr, helpcol ); */

  if (nreqcols==0) {
    return;
  } else if (nreqcols>(4-(helpcol%4))) {
    if ((helpcol%4)!=0) {
      fprintf(stderr,"\n");
      helpcol = 0;
    }
  }

  helpcol = helpcol + nreqcols;

  fprintf(stderr, formatstr,s);

  if((helpcol)%4 == 0)
    fprintf(stderr,"\n");


}

Help()
{
    fprintf(stderr,"AVAILABLE COMMANDS:\n");

    helpcol = 0;
    HP("appendtofile [file]");
    HP("assigncolors start inc"); 
    HP("autoorigin");
    HP("autosavedelay [f]");
    HP("barfilled");
    HP("bargraph");
    HP("boldfont");
    HP("barwidth [f][auto percent]");
    HP("closeplot");
    HP("color [0..255]");
    HP("clustercolor [i][0..255]");
/*     HP("clusterrgb [i][rval][gval][bval]"); */
/*     HP("rgb [i][rval][gval][bval]"); */
    HP("cscale [f]");
    HP("defepoch");
    HP("deleteplot");
    HP("disparityscale [f]");
    HP("epoch [#] [name] [tstart] [tend]");
    HP("escale [f]");
    HP("fastsearch [0/1]");
    HP("flash ([0/1])");
    HP("geometry [str]");
    HP("global");
    HP("gridcolor [0..255]");
    HP("graphtitle [s]");
    HP("init");
    HP("legends [0/1]");
    HP("line");
    HP("linewidth [i]");
    HP("loadepoch ([file])");
    HP("loadview projx projy [projz] xmin ymin xmax ymax");
    HP("longlegend [0/1]");
    HP("newplot");
    HP("normalfont");
    HP("optimizeplots [0/1]");
    HP("optimizepoints searchlen");
    HP("overlay [0/1]");
    HP("plotsline [sx1 sy1 sx2 sy2]");
    HP("plotwline [wx1 wy1 wz1 wx2 wy2 wz2]");
    HP("plotname [s]");
    HP("pointfreq [i]");
    HP("pointstyle [i]");
    HP("pointscale [f]");
    HP("print");
    HP("printtofile [file]");
    HP("psres [0/1]");
    HP("psbox [0/1]");
    HP("psheader [0/1]");
    HP("pscolor [0/1]");
    HP("plotstext [sx][sy][s][font]");
    HP("plotwtext [wx][wy][s][font]");
    HP("pwd");
    HP("quit");
    HP("readfile [s]");
    HP("randomizeprojection [i] ... 0/1");
    HP("randomizeprojectionbyname [proj] ... 0/1");
    HP("refresh");
    HP("restoregraph [file]");
    HP("saveepoch [file]");
/*     HP("savegraph [file]"); */
    HP("scatter");
    HP("setcolor [0..255]");
    HP("setenv [var value]");
    HP("setmenu [var value]");
    HP("select [plotnum]");
    HP("selectcluster [i]");
    HP("selectemptycluster");
    HP("selectnextcluster");
    HP("selectnextenabledcluster");
    HP("selectprevcluster");
    HP("selectprevenabledcluster");
    HP("shiftpercent [0..1]");
    HP("showaxis [-4..4]");
    HP("showpoints [0/1]");
    HP("showtitle [0/1]");
    HP("snapmode [0/1]");
    HP("sort");
    HP("sline [sx1 sy1 sx2 sy2]");
    HP("startcluster [i]");
    HP("stext [sx][sy][s][font]");
    HP("stepthrough [0/1]");
    HP("ticksize [i]");
    HP("thetax [f]");
    HP("thetay [f]");
    HP("thetaz [f]");
    HP("wait");
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
    HP("xlo [i]");
    HP("xlog [0/1]");
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
    HP("zoffset [f]");
    HP("zmode [i]");
    HP("zprojection [i]");
    HP("zscale [f]");
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

