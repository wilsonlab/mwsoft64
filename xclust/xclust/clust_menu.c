#include "xclust_ext.h"

/* We need these to redraw the button bitmaps when clusters change color */
#include "eye.bitmap"
#include "eye2.bitmap"
#include "ibutton.bitmap"
#include "ibutton2.bitmap"

#define SYSTEM(S)	System(S)


static int menu_block = 0;
static int menu_cluster_block = 0;
static unsigned short	redsave,bluesave,greensave;


fswap(f1,f2)
float	*f1,*f2;
{
float	ftmp;

    ftmp = *f1;
    *f1 = *f2;
    *f2 = ftmp;
}

/* listed here as they are used for reassignment of item->func for
** projection selection using projection name buttons */

int MenuSetProjection0();
int MenuSetProjection1();


char *GetParmFilename()
{
Plot *plot;
char *parmfile;

    if((plot = SelectedPlot(GetGraph("/graph"))) == NULL){
      fprintf(stderr,"ERROR: unable to get selected plot (GetParmFilename)\n");
      return(NULL);
    }
    if((parmfile = plot->filename) == NULL){
      fprintf(stderr, "ERROR: unable to get parameter filename from plot (GetParmFilename)\n");
      return(NULL);
    }
    return(parmfile);
}

DataSource *GetCurrentSource()
{
  Plot	*plot;

  if((plot = SelectedPlot(GetGraph("/graph"))) == NULL){
    fprintf(stderr,"ERROR: unable to get selected plot\n");
    return(NULL);
  }
  return(plot->source);
}

char *RunFindspike(char *args)
{
  pid_t waitstatus;
  char command[200];
  static char	tstring[30];
  char	*ptr;
  FILE	*fp;

  sprintf(command,"findspike %s", args);

  if (debug){
    fprintf(stderr,"Executing command: %s\n",command); 
  }

  if((fp = popen(command,"r")) == NULL){
    fprintf(stderr,"ERROR: unable to execute findspike command with args '%s'\n", args);
    return(NULL);
  }
  
  /* get 30 chars of findspike output from the pipe we opened */
  fgets(tstring,30,fp);

  if (debug){
    fprintf(stderr,"\n\nfindspike answer: %s\n",tstring);
  }

  /* close the pipe, and get the special return code (see man wait4) */
  waitstatus = pclose(fp);
  
  if (debug){
    fprintf(stderr,"waitstatus after fgets: %d\n",waitstatus);
    fprintf(stderr,"WIFEXITED after fgets: %d\n",WIFEXITED(waitstatus));
  }

  /* WIFEXITED is a macro that determines if we had a normal (not
     necessarily successful) exit */
  if (!WIFEXITED(waitstatus)){ 
    fprintf(stderr,"ERROR: problem calling findspike; command was: '%s'.\n", command);
    return(NULL);}

  if (debug){
    fprintf(stderr,"WEXITSTATUS after fgets: %d\n",WEXITSTATUS(waitstatus));
  }
    
  /* WEXITSTATUS returns the exit status of findspike */

  if(tstring == NULL ||         /* empty answer from findspike */
     WEXITSTATUS(waitstatus)){  /* return value from findspike */

    switch (WEXITSTATUS(waitstatus)){
    case 0: /* Null tstring & return value = 0 (success) */
      fprintf(stderr,"ERROR: findspike returned no result\n");
      break;
      
    case 1: /* Generic error from findspike */
      fprintf(stderr,"ERROR: findspike 'generic' error; ");
      break;

    case 2:
      fprintf(stderr,"ERROR: findspike 'file' error; ");
      break;
	
    case 3:
      fprintf(stderr,"ERROR: findspike 'out of range' error; ");
      break;

    case 4:
      fprintf(stderr,"ERROR: findspike 'no time/timestamp field' error; ");
      break;
    }    
    
    fprintf(stderr,"command was: '%s'.\n", command);
    sprintf(tstring,"-1"); 
    return(tstring);
  } 

  /* we got a non-null answer */    
    
  /* trim off the CR */
  if(ptr = strchr(tstring,'\n')){
    *ptr = '\0';
  }
  return(tstring);
}

char *LookupSpikeIndex(DataSource *source, char *time)
{
  /*DataSource *source;*/
char args[100];
 
 if (source == NULL) {
   if ((source = GetCurrentSource()) == NULL){
     fprintf(stderr,"ERROR: unable to get selected data source\n");   
     return(NULL);
   }
 }

 switch(source->timelookupfile){
 case PARMFILE:
   /* use -recid flag to findspike so that we get the index into
   ** the parm file, and not the spike 'id' field */
   sprintf(args, "%s -t %s -recid", GetParmFilename(),time);
   return(RunFindspike(args));
   
 case TTFILE:
   sprintf(args, "%s -t %s", GetItemValue("/analysismenu/spikefile"),time);     
   return(RunFindspike(args));  
   
 default:
   if(!suppresswarnings)
     fprintf(stderr,"ERROR: epochs not supported for this parmfile (see warnings in stderr)\n");   
   return(NULL);
 }
}

char *LookupSpikeTime(DataSource *source, char *index)
{
  /*DataSource *source;*/
char args[100];
 
 if (source == NULL) {
   if ((source = GetCurrentSource()) == NULL){
     fprintf(stderr,"ERROR: unable to get selected data source\n");   
     return(NULL);
   }
 }

 switch(source->timelookupfile){
 case PARMFILE:
   /* use -recid flag to findspike so that we get the index into
   ** the parm file, and not the spike 'id' field */
   sprintf(args, "%s -i %s", GetParmFilename(),index);
   return(RunFindspike(args));
   
 case TTFILE:
   sprintf(args, "%s -i %s", GetItemValue("/analysismenu/spikefile"),index);     
   return(RunFindspike(args));  
   
 default:
   if (!suppresswarnings)
     fprintf(stderr,"ERROR: epochs not supported for this parmfile (see warnings in stderr)\n");   
   return(NULL);
 }
}

/*
******************************************************************
**                                                              **
**                   Cluster Menu Routines                      **
**                                                              **
******************************************************************
*/
MenuSetShowClusters(item)
MenuItem	*item;
{
    SetShowClusters(!item->state);
    ScaleAndRefreshGraph(G);
}

MenuToggleXAutoscale(item)
MenuItem	*item;
{
}

RestorePixel()
{
    SetPixelRGB(LookupClusterColor(G,G->selectedcluster),
		redsave,
		greensave,
		bluesave);
}

SavePixel()
{
    GetPixelRGB(LookupClusterColor(G,G->selectedcluster),
		&redsave,
		&greensave,
		&bluesave);
}

MenuSetClusterColor(item)
MenuItem	*item;
{
  MenuWindow *menu;
	SetClusterColor(G,G->selectedcluster,item->itemforeground);
	RefreshSelectedClusterButton();
	/*
	** update the cluster info panel
	*/
	menu = GetMenu("/cluststat");
	if(menu->mapped){
	    DrawClustStat(menu);
	}
	RefreshGraph(G);
}

DrawColorStat(colorstat)
MenuWindow	*colorstat;
{
}


DrawClustStat(cluststat)
MenuWindow	*cluststat;
{
int	clusterid;
ClusterBounds	*cb;
ProjectionInfo	*pinfo;
Graph		*graph;
char		*name;
int		row;
int		npoints;
ClusterList	*clist;
char		line[100];
int		width;
int		height;

    if(cluststat == NULL) return;
    /*SetBackground(cluststat,cluststat->background);*/
    ClearWindow(cluststat);
    graph = GetGraph("/graph");
    /*
    ** first handle the selected cluster info
    */
    /*
    ** draw the 3d edges
    */
    width = 0.37*cluststat->wwidth;
    height = cluststat->wheight;
    _SetColor(cluststat,MININTERFACECOLOR + MENUITEMLIGHTEDGE);
    _DrawLine(cluststat,width-2,height-2,
	width-2,1);
    _DrawLine(cluststat,width-2,height-2,
	1,height-2);
    _SetColor(cluststat,MININTERFACECOLOR + MENUITEMDARKEDGE);
    _DrawLine(cluststat,1,1,1,height-2);
    _DrawLine(cluststat,1,1,width-2,1);

    clusterid = graph->selectedcluster;
    _SetColor(cluststat,LookupClusterColor(graph,clusterid));
    sprintf(line,"%d",clusterid);
    Text(cluststat,5,20,"Cluster id: ");
    Text(cluststat,75,20,line);

    _SetColor(cluststat,cluststat->foreground);
    if((clist = GetClusterInfo(clusterid)) != NULL){
	/*
	** and get the number of points in the cluster
	*/
	npoints = clist->npoints;
    } else {
	npoints = 0;
    }
    sprintf(line,"%d",npoints);
    Text(cluststat,5,35,"Npoints: ");
    Text(cluststat,75,35,line);

    Text(cluststat,5,50,"Bounding Regions: ");
    /*
    ** go through the cluster bounds list and display the
    ** projections the current selected cluster is defined
    ** on
    */
    row = 65;
    for(cb=graph->clusterbounds;cb;cb=cb->next){
	if(cb->clusterid == clusterid){
	    if(pinfo = GetProjectionInfo(G,cb->projection[0])){
		name = pinfo->name;
	    }  else {
		name = "";
	    }
	    Text(cluststat,5,row,name);
	    if(pinfo = GetProjectionInfo(G,cb->projection[1])){
		name = pinfo->name;
	    }  else {
		name = "";
	    }
	    Text(cluststat,65,row,name);
	    row +=15;
	}
    }
    /*
    ** then do the comprehensive cluster info
    */
    /*
    ** draw the 3d edges
    */
    width = 0.37*cluststat->wwidth;
    height = cluststat->wheight;
    _SetColor(cluststat,MININTERFACECOLOR + MENUITEMLIGHTEDGE);
    _DrawLine(cluststat,cluststat->wwidth-4,height-2,
	cluststat->wwidth-4,1);
    _DrawLine(cluststat,cluststat->wwidth-4,height-2,
	1,height-2);

    _SetColor(cluststat,MININTERFACECOLOR + MENUITEMDARKEDGE);
    _DrawLine(cluststat,width,1,width,height-2);
    _DrawLine(cluststat,width,1,cluststat->wwidth-4,1);

    clusterid = graph->selectedcluster;
    _SetColor(cluststat,cluststat->foreground);
    Text(cluststat,width+5,20,"Defined Clusters");

    Text(cluststat,width+5,35,"Clust");
    Text(cluststat,width+40,35,"Npts");
    if(graph->clusterlist){
	if(graph->clusterlist->csiclust > 0){
	    sprintf(line,"CSI Eval %d",graph->clusterlist->csiclust);
	} else {
	    sprintf(line,"CSI Eval All");
	}
	Text(cluststat,width+100,20,line);

	sprintf(line,"CSI");
	Text(cluststat,width+80,35,line);
	sprintf(line,"BLen");
	Text(cluststat,width+110,35,line);
	sprintf(line,"N+");
	Text(cluststat,width+145,35,line);
	sprintf(line,"R-");
	Text(cluststat,width+175,35,line);
    }
    row = 50;
    for(clist=graph->clusterlist;clist;clist=clist->next){
	_SetColor(cluststat,LookupClusterColor(graph,clist->clusterid));
	sprintf(line,"%d",clist->clusterid);
	Text(cluststat,width+15,row,line);
	sprintf(line,"%d",clist->npoints);
	Text(cluststat,width+40,row,line);
	if(clist->ncsi > 0 && clist->npoints > 0){
	    sprintf(line,"%-7.3g",(float)100.0*clist->csi/clist->npoints);
	} else {
	    sprintf(line,"?");
	}
	Text(cluststat,width+80,row,line);
	if(clist->ncsi_plus > 0){
	    sprintf(line,"%-7.3g",(float)1.0*clist->burstlen/clist->ncsi_plus);
	} else {
	    sprintf(line,"?");
	}
	Text(cluststat,width+115,row,line);
	if(clist->ncsi_plus > 0){
	    sprintf(line,"%d",clist->ncsi_plus);
	} else {
	    sprintf(line,"?");
	}
	Text(cluststat,width+145,row,line);
	if(clist->ncsi_refract_minus > 0 && clist->npoints > 0){
	    sprintf(line,"%-7.3g",(float)100.0*clist->ncsi_refract_minus/clist->npoints);
	} else {
	    sprintf(line,"?");
	}
	Text(cluststat,width+175,row,line);
	row += 15;
    }

    _SetColor(cluststat,cluststat->foreground);

    cluststat->wheight = GetMenuWindowHeight(cluststat);
    XResizeWindow(cluststat->display,cluststat->window,
		  cluststat->wwidth, cluststat->wheight);
    CollapseMenu(cluststat->parent);

}


BackupClusterBounds(graph)
Graph	*graph;
{
ClusterBounds	*cb;
ClusterBounds	*newcb;
ClusterBounds	*newcbptr;
ClusterBounds	*tmpcb;
ClusterBounds	*nextcb;
ClusterList	*clist;
ClusterList	*nextclist;
ClusterList	*newclist;
ClusterList	*newclistptr;
ClusterList	*tmpclist;
Backup		*backup;

    if(graph == NULL) return;
    /*
    ** make sure there is a backup structure defined
    */
    if(graph->backup == NULL){
	graph->backup = (Backup *)calloc(1,sizeof(Backup));
    }
    /*
    ** make a copy of the current bounds
    */
    newcb = NULL;
    for(cb=graph->clusterbounds;cb;cb=cb->next){
	/*
	** make a new cluster bounds structure
	*/
	if((tmpcb = (ClusterBounds *)calloc(1,sizeof(ClusterBounds))) == NULL){
	    fprintf(stderr,"unable to allocate new cluster bounds\n");
	    continue;
	}
	bcopy(cb,tmpcb,sizeof(ClusterBounds));
	tmpcb->next = NULL;
	/*
	** copy the cluster bounds coordinates
	*/
	tmpcb->fcoord = (FCoord *)malloc(tmpcb->ncoords*sizeof(FCoord));
	tmpcb->coord = (Coord *)malloc(tmpcb->ncoords*sizeof(Coord));
	bcopy(cb->fcoord,tmpcb->fcoord,tmpcb->ncoords*sizeof(FCoord));
	bcopy(cb->coord,tmpcb->coord,tmpcb->ncoords*sizeof(Coord));
	if(newcb == NULL){
	    newcb = tmpcb;
	} else {
	    /*
	    ** stick it at the end of the new cluster bounds list
	    */
	    newcbptr->next = tmpcb;
	}
	/*
	** update the new cluster bounds list pointer
	*/
	newcbptr = tmpcb;
    }
    /*
    ** delete the previous backup
    */
    for(cb=graph->backup->clusterbounds;cb;cb=nextcb){
	if(cb->fcoord){
	    free(cb->fcoord);
	}
	if(cb->coord){
	    free(cb->coord);
	}
	nextcb=cb->next;
	free(cb);
    }
    /*
    ** save the backup
    */
    graph->backup->clusterbounds = newcb;
    /*
    ** make a copy of the current list
    */
    newclist = NULL;
    for(clist=graph->clusterlist;clist;clist=clist->next){
	/*
	** make a new cluster list structure
	*/
	tmpclist = (ClusterList *)calloc(1,sizeof(ClusterList));
	bcopy(clist,tmpclist,sizeof(ClusterList));
	tmpclist->next = NULL;
	if(newclist == NULL){
	    newclist = tmpclist;
	} else {
	    /*
	    ** stick it at the end of the new cluster list
	    */
	    newclistptr->next = tmpclist;
	}
	/*
	** update the new cluster list pointer
	*/
	newclistptr = tmpclist;
    }
    /*
    ** delete the previous backup
    */
    for(clist=graph->backup->clusterlist;clist;clist=nextclist){
	nextclist=clist->next;
	free(clist);
    }
    /*
    ** save the backup
    */
    graph->backup->clusterlist = newclist;
    /*
    ** set the cluster bounds modified flag
    */
    graph->cbmodified = 1;
}

MenuEvalOverlap(item)
MenuItem	*item;
{
Graph	*graph;

    if((graph = GetGraph("/graph"))== NULL){
	fprintf(stderr,"ERROR: cant find graph\n");
	return;
    }
    EvaluateOverlap(graph);
}

MenuUndoClusterChange(item)
MenuItem	*item;
{
Graph	*graph;
ClusterBounds	*cb = NULL;
ClusterList	*clist = NULL;
Backup		*backup;

    /*
    ** restore the cluster bounds to the last backup
    */
    graph = GetGraph("/graph");
    /*
    ** get the cluster bounds backup
    */
    if ( (backup = graph->backup) != NULL ) {
      /*
      ** swap the current bounds and list for the last backup
      */
      if((cb = backup->clusterbounds) == NULL){
	/*
	  fprintf(stderr,"No bounds backup found\n");
	  return;
	*/
      }
      if((clist = backup->clusterlist) == NULL){
	/*
	  fprintf(stderr,"No cluster list backup found\n");
	  return;
	*/
      }
    
      /*
      ** save the current cluster bounds
      */
      backup->clusterbounds = graph->clusterbounds;
      backup->clusterlist = graph->clusterlist;
      /*
      ** and replace them with the backup
      */
      graph->clusterbounds = cb;
      graph->clusterlist = clist;
      /*
      ** recompute and redisplay
      */
      TouchClusters(graph);
      AssignClusterPoints(graph);
      /*
      ** reassess the active cluster list
      */
      UpdateClusterMenu();
      ScaleAndRefreshGraph(graph);
    }
}

MenuBackwardXhilo(item)
MenuItem	*item;
{
Graph	*graph;
Plot	*plot;
int	block;

    /*
    ** go through each plot and increment the xhi/xlo
    */
    graph = GetGraph("/graph");
    for(plot=graph->plot;plot;plot=plot->next){
	block = plot->xhi - plot->xlo + 1;
	if(block > 0 && ((plot->xlo - block) >= 0)){
	    plot->xhi -= block;
	    plot->xlo -= block;
	}
    }
    ScaleAndRefreshGraph(graph);
}

MenuForwardXhilo(item)
MenuItem	*item;
{
Graph	*graph;
Plot	*plot;
int	block;

    /*
    ** go through each plot and increment the xhi/xlo
    */
    graph = GetGraph("/graph");
    for(plot=graph->plot;plot;plot=plot->next){
	block = plot->xhi - plot->xlo + 1;
	if(block > 0 && ((plot->xhi+block) < plot->npoints)){
	    plot->xhi += block;
	    plot->xlo += block;
	}
    }
    ScaleAndRefreshGraph(graph);
}

/*
** this is a hack to run a special autoprocessing command.
** it points in the direction of a more general autoprocessing interface
*/
MenuAutoprocess(item)
MenuItem	*item;
{
char	*tstring;
char	*command;
char	*dir;
char	line[100];
char	*subdir;
Plot	*plot;
char	*ptr;
char	parmfile[100];

    /*
    ** execute the selected autoprocessing command in the
    ** selected directory relative to the current directory
    */

    /*
    ** get the command
    */
    if((command = GetItemValue("/analysismenu/autocommand")) == NULL){
	fprintf(stderr,"ERROR: autoprocess command not set\n");
	return;
    }
    /*
    ** get the directory to perform the processing in
    */
    if((dir = item->value) == NULL){
	fprintf(stderr,"ERROR: must set a directory for processing\n");
	return;
    }

    /*
    ** if processing a subdirectory, then modify the search paths
    ** for position, parameter, and spike files accordingly
    */
    if(strcmp(dir,".") != 0){
	/*
	** back up an additional level
	*/
	subdir = "../";
    } else {
	subdir = "";
    }

    /*
    ** get the position file
    */
    if((tstring = GetItemValue("/analysismenu/positionfile")) != NULL){
	/*
	** set the PFILE environment variable
	*/
	sprintf(line,"%s%s",subdir,tstring);
	Setenv("PFILE",line);

    }
    /*
    ** get the spike file
    */
    if((tstring = GetItemValue("/analysismenu/spikefile")) != NULL){
	/*
	** set the STFILE environment variable
	*/
	sprintf(line,"%s%s",subdir,tstring);
	Setenv("STFILE",line);

    }
    /*
    ** get the parameter file
    */
    if((plot = SelectedPlot(GetGraph("/graph"))) == NULL){
	fprintf(stderr,"ERROR: unable to get selected plot\n");
	return;
    }
    if((tstring = plot->filename) != NULL){
	/*
	** set the parmfile
	*/
	sprintf(parmfile,"%s%s",subdir,tstring);

    } else {
	fprintf(stderr,
	"ERROR: unable to get parameter filename from plot\n");
	return;
    }
    /*
    ** get the cluster prefix string
    */
    if((tstring = GetItemValue("/clustermenu/writeclusters")) != NULL){
	/*
	** eliminate any subdirectories
	*/
	while(ptr = strchr(tstring,'/')){
	    tstring = ptr+1;
	}
	/*
	** set the CLBASE environment variable
	*/
	sprintf(line,"%s",tstring);
	Setenv("CLBASE",line);

    }
    /*
    ** change into the subdirectory to be processed
    if(chdir(dir) != 0){
	fprintf(stderr,
	"ERROR: unable to change into processing subdirectory\n");
	return;
    }
    */
    /*
    ** invoke the command in the background
    */
    sprintf(line,"cd %s ; %s %s &",dir,command,parmfile);
    SYSTEM(line);
}

SaveDefaults(fp,itemname)
FILE	*fp;
char	*itemname;
{
char	*tstring;

    if((tstring = GetItemValue(itemname)) != NULL){
	if(strlen(tstring) != 0){
	    fprintf(fp,"/setmenu\t %s\t \"%s\"\n",itemname,tstring);
	}
    }
}

MenuSaveDefaults(item)
MenuItem	*item;
{
FILE	*fp;
MenuItem	*tmpitem;
int	defaults;
char	*home;
char	*getenv();
char	dir[100];
unsigned short	r,g,b;
int	pixel;
int	i;
int	clustercolor;
char    *itemname;

char ename[50];
char *enameval;
char estart[50];
char *estartval;
char eend[50];
char *eendval;

ProjectionInfo *pinfo;
char s1[1000] = "";
char s0[1000] = "";

#define GLOBAL 1
#define LOCAL 0

    /*
    ** check to see whether local or global defaults are selected
    */

 if(strcmp(item->name, "/controlmenu/savedefaultsglobal") == 0)
   defaults = GLOBAL;
 else
   defaults = LOCAL;
 
    if(defaults == LOCAL){
	/*
	** write out the local defaults file
	*/
	if((fp = fopen(".xclust3rc","w")) == NULL){
	    fprintf(stderr,
	    "ERROR: unable to create local defaults file .xclust3rc\n");
	    return;
	}
	fprintf(stderr,"writing local defaults file '.xclust3rc'\n");
    } else {
	/*
	** write out the home directory defaults file
	*/
	if((home = getenv("HOME")) != NULL){
	    sprintf(dir,"%s/.xclust3rc",home);
	    if((fp = fopen(dir,"w")) == NULL){
		fprintf(stderr,
		"ERROR: unable to create defaults file '%s'\n",dir);
		return;
	    }
	    fprintf(stderr,"writing defaults file '%s'\n",dir);
	} else {
	    fprintf(stderr,
	    "ERROR: unable to find home directory environment variable\n");
	    return;
	}
    }
    BeginStandardHeader(fp,standardargc,standardargv,VERSION);
    fprintf(fp,"%% File type:\t %s\n","Ascii");
    fprintf(fp,"%% File contents:\t %s\n","xclust startup defaults");
    EndStandardHeader(fp);
    /*
    ** write out the commands to restore select menu item values
    */
    SaveDefaults(fp,"/analysismenu/gridsize");
    SaveDefaults(fp,"/analysismenu/binsize");
    SaveDefaults(fp,"/analysismenu/tmax");
    SaveDefaults(fp,"/analysismenu/positionfile");
    SaveDefaults(fp,"/analysismenu/trangefile");
    SaveDefaults(fp,"/analysismenu/spikefile");
    SaveDefaults(fp,"/clustermenu/writebounds");
    SaveDefaults(fp,"/clustermenu/readbounds");
    SaveDefaults(fp,"/clustermenu/writeclusters");
    SaveDefaults(fp,"/analysismenu/viewxcorr");
    SaveDefaults(fp,"/analysismenu/csimin");
    SaveDefaults(fp,"/analysismenu/csimax");
    SaveDefaults(fp,"/analysismenu/csbwin");
    SaveDefaults(fp,"/epochmenu/epochfile");

    /* Write out the epochs (redundant now that we have epoch files,
    ** but keep for compatibility) */

    for(i = 1;
	i <= NUM_EPOCHS; /* specified in xclust_defs.h */
	i++){

      sprintf(ename,"/epochmenu/epoch/%d",i);
      sprintf(estart,"/epochmenu/epochstart/%d",i);
      sprintf(eend,"/epochmenu/epochend/%d",i);

      enameval = GetItemValue(ename);
      estartval = GetItemValue(estart);
      eendval = GetItemValue(eend);

      /* Don't write out totally empty epochs ( test for NULL and empty string ) */
      if ((!enameval || !(strcmp((enameval = GetItemValue(ename)),""))) &&
	  (!estartval || !(strcmp((estartval = GetItemValue(estart)),""))) &&
	  (!eendval || !(strcmp((eendval = GetItemValue(eend)),""))))
	continue;
	
      fprintf(fp,"/epoch\t%d\t\"%s\"\t\"%s\"\t\"%s\"\n",
	      i,
	      (enameval ? enameval : ""), /* print the empty string, not (null) */
	      (estartval ? estartval : ""),
	      (eendval ? eendval : ""));
    }	      

    /*
    ** Save the views
    */
    SaveViewDefaults(G,fp);

    /*
    ** Save randomization state of projections 
    **
    ** s1: names of projections to randomize 
    ** s0: names of projections to unrandomize (necessary so that
    ** we don't end up 'and'ing the global and local .xclust3rc
    ** randomization states on load-in)
    */

    for(i=0;i<MAXPROJECTION-1;i++){

      if((pinfo = GetProjectionInfo(G,i)) != NULL)      
	if (pinfo->randomize)
	  sprintf(s1,"%s %s",s1,pinfo->name);
	else
	  sprintf(s0,"%s %s",s0,pinfo->name);
    }

    /* write out randomized projections */
    if (strlen(s1) != 0)
      fprintf(fp,"/randomizeprojectionbyname%s 1\n", s1);

    /* write out non-randomized projections (so we can override a
    ** global .xclust3rc) */
    if (strlen(s0) != 0)
      fprintf(fp,"/randomizeprojectionbyname%s 0\n", s0);


/*     /\* */
/*     ** write out the color map */
/*     *\/ */
/*     GetPixelRGB(0,&r,&g,&b); */
/*     fprintf(fp,"/rgb %d %d %d %d\n",0,r>>8,g>>8,b>>8); */
/*     GetPixelRGB(1,&r,&g,&b); */
/*     fprintf(fp,"/rgb %d %d %d %d\n",1,r>>8,g>>8,b>>8); */
/*     for(i=2;i<MAXCOLORS;i++){ */
/* 	GetPixelRGB(i,&r,&g,&b); */
/* 	fprintf(fp,"/rgb %d %d %d %d\n",i,r>>8,g>>8,b>>8); */
/*     } */

    /*
    ** save altered cluster colors
    */
    for(i=0;i<MAXCLUSTER;i++){
	if((clustercolor=LookupClusterColor(G,i)) != MINCLUSTER+i){
	    fprintf(fp,"/clustercolor %d %d\n",i,clustercolor);
	}
    }


    /*
    ** Clean up and go home 
    */
    fclose(fp);
    fprintf(stderr,"done\n");
}

MenuEpochName(item)
MenuItem	*item;
{
char		line[100];
char		ename[100];
char		cname[100];
char		*ptr;
char		*tstring;
char		*cstring;
int		invalidtime;

 char *start=NULL, *end=NULL, *epochname;
 int pointstart, pointend;
 char str_start[30], str_end[30];
 char tmpstr[30];
    /*
    ** load the epoch times into the spiketime dialogs
    */
    /*
    ** get the epoch number
    */
    strcpy(line,item->name);
    if((ptr = strchr(line+1,'/')) == NULL){
	fprintf(stderr,"ERROR: unable to parse epoch number from '%s'\n",
	item->name);
    }
    if((ptr = strchr(ptr+1,'/')) == NULL){
	fprintf(stderr,"ERROR: unable to parse epoch number from '%s'\n",
	item->name);
    }
    invalidtime = 0;

    /* get epoch start time */
    sprintf(ename,"/epochmenu/epochstart/%s",ptr+1);
    start = GetItemValue(ename);
    /* get epoch end time */
    sprintf(ename,"/epochmenu/epochend/%s",ptr+1);
    end = GetItemValue(ename);

    if (start==NULL || end == NULL || start[0]=='\0' || end[0] == '\0') {
      fprintf(stderr, "Load Epoch: No start and/or end point defined\n");
      return;
    }

    /* convert time to points */
    strcpy(str_start, LookupSpikeIndex(NULL, start));
    strcpy(str_end, LookupSpikeIndex(NULL, end));

    if (start && end) {

      pointstart = Atoi(str_start);
      pointend = Atoi(str_end);

    } else {
	fprintf(stderr, "Error loading points: Invalid times\n");
	return;
    }

    sprintf(ename,"/epochmenu/epoch/%s",ptr+1);
    epochname = GetItemValue(ename);   

    LoadPointsAndUpdateStatus(pointstart, pointend, epochname);


    if (GetItemState("/epochmenu/units_id")) {
      PutItemValue("/epochmenu/start", str_start);
      PutItemValue("/epochmenu/end", str_end);
      PutItemValue("/epochmenu/moviestart", str_start);
      PutItemValue("/epochmenu/movieend", str_end);
    } else {
      PutItemValue("/epochmenu/start", start);
      PutItemValue("/epochmenu/moviestart", start);
      PutItemValue("/epochmenu/end", end);
      PutItemValue("/epochmenu/movieend", end);
    }

    RefreshItem("/epochmenu/start");
    RefreshItem("/epochmenu/end");
    RefreshItem("/epochmenu/moviestart");
    RefreshItem("/epochmenu/movieend");


 /*    if((tstring = GetItemValue(ename)) != NULL){ */
/* 	PutItemValue("/epochmenu/epochname",item->value); */


    /*
    ** prepend the epochname to the write cluster dialog
    */
    if((cstring = GetItemValue("/clustermenu/writeclusters")) != NULL){
      /*
      ** search for any previous subdirectory prefix
      */
      if((ptr = strchr(cstring,'/')) != NULL){
	ptr++;
      } else {
	ptr = cstring;
      }
      if (item->value == NULL)
	sprintf(cname,"%s",ptr);
      else
	sprintf(cname,"%s/%s",item->value,ptr);
      
      PutItemValue("/clustermenu/writeclusters",cname);
      RefreshItem("/clustermenu/writeclusters");
    }
    

/*     if((tstring = GetItemValue(ename)) != NULL){ */
/* 	PutItemValue("/epochmenu/start",tstring); */
/*     } else { */
/* 	invalidtime = 1; */
/*     } */


/*     if((tstring = GetItemValue(ename)) != NULL){ */
/* 	PutItemValue("/epochmenu/end",tstring); */
/*     } else { */
/* 	invalidtime = 1; */
/*     } */

/*     /\* */
/*     ** call the tstartspike and tendspike dialog */
/*     *\/ */
/*     if(!invalidtime){ */
/* /\* 	CallMenuItem("/epochmenu/tstartspike"); *\/ */
/* /\* 	CallMenuItem("/epochmenu/tendspike"); *\/ */

/*       LoadPointsAndUpdateStatus() */

/*     } */
    /*
    ** refresh the menu display
    */
    DrawMenu(item->menu);
    
}

MenuEpochKill(MenuItem *item)
{
char		line[100];
char		*ptr;
 int i;

    /*
    ** load the epoch times into the spiketime dialogs
    */
    /*
    ** get the epoch number
    */
    strcpy(line,item->name);
    if((ptr = strchr(line+1,'/')) == NULL){
	fprintf(stderr,"ERROR: unable to parse epoch number from '%s'\n",
	item->name);
	return(0);
    }
    if((ptr = strchr(ptr+1,'/')) == NULL){
	fprintf(stderr,"ERROR: unable to parse epoch number from '%s'\n",
	item->name);
	return(0);
    }

    i = atoi(ptr+1);

    SetEpochValues(i, NULL, NULL, NULL);

}

SetEpochValues(int epochnum, char *ename, char *estart, char *eend)
{
  char *itemstr;

  itemstr = (char *)malloc(40);  

/*   fprintf(stderr, */
/* 	  "E#: %d   ename: %s  estart: %s   eend: %s\n", */
/* 	  epochnum, */
/* 	  ename, */
/* 	  estart, */
/* 	  eend); */

    sprintf(itemstr,"/epochmenu/epoch/%d",epochnum);
    PutItemValue(itemstr,ename);
    RefreshItem(itemstr);
    
    sprintf(itemstr,"/epochmenu/epochstart/%d",epochnum);
    PutItemValue(itemstr,estart);
    RefreshItem(itemstr);

    sprintf(itemstr,"/epochmenu/epochend/%d",epochnum);
    PutItemValue(itemstr,eend);
    RefreshItem(itemstr);

}
				 
DefineNewEpoch(char *epochname)
{
 ProjectionInfo *pinfo;
 char tstart[15]; 
 char tend[15];
 char *epoch_name;
 char *epoch_tstart;
 char *epoch_tend;
 int i, j;
 char itemstr[100];
 int axis=-1;
 double wmin[2], wmax[2];

 wmin[0] = G->wxmin;
 wmin[1] = G->wymin;
 wmax[0] = G->wxmax;
 wmax[1] = G->wymax;

 /* is there a time projection? */
 if(pinfo = GetProjectionInfoByName(G,"time")){
   /* is it the current x-axis?*/
   for (j=0; j<2; j++) {
     if (G->currentprojection[j] == pinfo->projectionid) {
       /* find empty slot */
       for(i = 1; i <= NUM_EPOCHS;i++){
	 sprintf(itemstr,"/epochmenu/epoch/%d",i);
	 epoch_name = GetItemValue(itemstr);
	 sprintf(itemstr,"/epochmenu/epochstart/%d",i);
	 epoch_tstart = GetItemValue(itemstr);
	 sprintf(itemstr,"/epochmenu/epochend/%d",i);
	 epoch_tend = GetItemValue(itemstr);
	 if ( (epoch_name==NULL || strlen(epoch_name)<1) && (epoch_tstart==NULL || strlen(epoch_tstart)<1) && (epoch_tend==NULL || strlen(epoch_tend)<1) ) {
	   break;
	 }
       }
     
       if (i<=NUM_EPOCHS) {
	 if ( wmin[j] <0 )
	   sprintf(tstart, "%.4f", 0.);
	 else
	   sprintf(tstart, "%.4f", wmin[j] );
	 
	 if ( wmax[j]  <0 )
	   sprintf(tend, "%.4f", 0.);
	 else
	 sprintf(tend, "%.4f", wmax[j] );
	 
	 SetEpochValues(i, epochname, tstart, tend);
	 return;
       } else
	 fprintf(stderr,"No more empty epoch definition slots available.\n");
       
       break;
     } /* end of if current projection == time projection */
   } /* end of for loop */
   fprintf(stderr,"ERROR: Can't add epoch; no time projection selected.\n");
   return;
 } /* end of time projection test */
 fprintf(stderr,"ERROR: Can't add epoch; no 'time' field in parm file.\n");
 return;
}


MenuLabelClusters(item)
MenuItem	*item;
{
    if(item == NULL) return;
    if(item->state == 0){
	HideClusterLabels();
	RefreshGraph(G);
    } else {
	LabelClusters();
    }
}

LabelClusters()
{
ClusterBounds	*cb;
Graph	*graph;
float	wx,wy;
int	sx,sy;
int	clusterid;
char	name[20];
ClusterList	*clist;
int	npoints;
Label	*label;
char	labelname[100];

    graph = GetGraph("/graph");
    graph->showclusterlabels = 1;
    SetColor(graph->foreground);
    for(cb=graph->clusterbounds;cb;cb=cb->next){
	/*
	** is this a cluster bound defined on the currently 
	** selected projections
	*/
	if(cb->enabled && ( ( (graph->currentprojection[0] == cb->projection[0]) &&
	(graph->currentprojection[1] == cb->projection[1]) ) ||  ( (graph->currentprojection[0] == cb->projection[1]) &&
	(graph->currentprojection[1] == cb->projection[0]) ) ) ){
	    /*
	    ** it is so we can label it
	    */
	    clusterid = cb->clusterid;
	    /*
	    ** get the cluster list associated with the clusterid
	    */
	    if((clist = GetClusterInfo(clusterid)) != NULL){
		/*
		** and get the number of points in the cluster
		*/
		npoints = clist->npoints;
	    } else {
		fprintf(stderr,
		"ERROR: cant find cluster list for cluster %d\n",
		clusterid);
		npoints = 0;
	    }
	    /*
	    ** place the label at the first cluster bounds coordinate
	    */
	    if (graph->currentprojection[0] == cb->projection[1]) {
	      wx = cb->fcoord[0].y;
	      wy = cb->fcoord[0].x;
	    } else {
	      wx = cb->fcoord[0].x;
	      wy = cb->fcoord[0].y;
	    }
	    ScreenTransform(graph,wx,wy,&sx,&sy);
	    sprintf(name,"CL-%d (%d)",clusterid,npoints);
	    /*
	    ** look for an existing label
	    */
	    sprintf(labelname,"clusterlabel%d",clusterid);
	    if((label = GetNamedLabel(graph,labelname)) != NULL){
		/*
		** found a preexisting label, so just replace the
		** old label string with the new
		*/
		strcpy(label->u.string.line,name);
		label->u.string.wx = wx;
		label->u.string.wy = wy;
		label->u.string.wz = 0;
		label->visible = 1;
	    } else {
		/*
		** doesnt exist so add a new one
		*/
		label = AddLabelString(graph, name,0,graph->fontheight,
		    wx,wy,WORLD_LBL,TEMPORARY_LBL,graph->fontname);
		/*
		** and give it a name
		*/
		label->name = (char *)malloc(strlen(labelname)+1);
		strcpy(label->name,labelname);
	    }
	    /*
	    ** dont do a refresh to get the label up there, just
	    ** draw it
	    */
	    Text(graph,sx,sy,name);
	}
    }
}

HideClusterLabels()
{
Label	*label;
Graph	*graph;

    graph = GetGraph("/graph");
    graph->showclusterlabels = 0;
    for(label=graph->label;label;label=label->next){
	if(label->name && (strncmp(label->name,"clusterlabel",
	strlen("clusterlabel")) == 0)){
	    /*
	    ** found a cluster label
	    */
	    label->visible = 0;
	}
    }
}

DeleteClusterLabels()
{
Label	*label;
Graph	*graph;

    graph = GetGraph("/graph");
    for(label=graph->label;label;label=label->next){
	if(label->name && (strncmp(label->name,"clusterlabel",
	strlen("clusterlabel")) == 0)){
	    /*
	    ** found a cluster label
	    */
	    DeleteLabel(graph,label);
	}
    }
}

UpdateClusterLabels()
{
ClusterBounds	*cb;
Graph	*graph;
float	wx,wy;
int	sx,sy;
int	clusterid;
char	name[20];
ClusterList	*clist;
int	npoints;
Label	*label;
char	labelname[100];

    graph = GetGraph("/graph");
    SetColor(graph->foreground);
    /*
    ** assume each label is initially invisible until a cluster
    ** bound on this projection pair is found
    */
    for(label=graph->label;label;label=label->next){
	if(label->name && (strncmp(label->name,"clusterlabel",
	strlen("clusterlabel")) == 0)){
	    label->visible = 0;
	}
    }
    for(cb=graph->clusterbounds;cb;cb=cb->next){
	/*
	** is this a cluster bound defined on the currently 
	** selected projections
	*/
	if(cb->enabled && ( ( (graph->currentprojection[0] == cb->projection[0]) &&
	(graph->currentprojection[1] == cb->projection[1]) ) ||  ( (graph->currentprojection[0] == cb->projection[1]) &&
	(graph->currentprojection[1] == cb->projection[0]) ) ) ){
	    /*
	    ** it is so we can label it
	    */
	    clusterid = cb->clusterid;
	    /*
	    ** get the cluster list associated with the clusterid
	    */
	    if((clist = GetClusterInfo(clusterid)) != NULL){
		/*
		** and get the number of points in the cluster
		*/
		npoints = clist->npoints;
	    } else {
		fprintf(stderr,
		"ERROR: cant find cluster list for cluster %d\n",
		clusterid);
		npoints = 0;
	    }
	    /*
	    ** place the label at the first cluster bounds coordinate
	    */
	    if (graph->currentprojection[0] == cb->projection[1]) {
	      wx = cb->fcoord[0].y;
	      wy = cb->fcoord[0].x;
	    } else {
	      wx = cb->fcoord[0].x;
	      wy = cb->fcoord[0].y;
	    }
	    ScreenTransform(graph,wx,wy,&sx,&sy);
	    sprintf(name,"CL-%d (%d)",clusterid,npoints);
	    /*
	    ** look for an existing label
	    */
	    sprintf(labelname,"clusterlabel%d",clusterid);
	    if((label = GetNamedLabel(graph,labelname)) == NULL){
		/*
		** doesnt exist so add a new one
		*/
		label = AddLabelString(graph, name,0,graph->fontheight,
		    wx,wy,WORLD_LBL,TEMPORARY_LBL,graph->fontname);
		/*
		** and give it a name
		*/
		label->name = (char *)malloc(strlen(labelname)+1);
		strcpy(label->name,labelname);
	    }
	    /*
	    ** assign the label string and coordinates
	    */
	    strcpy(label->u.string.line,name);
	    label->u.string.wx = wx;
	    label->u.string.wy = wy;
	    label->u.string.wz = 0;
	    label->visible = graph->showclusterlabels;
	    /*
	    ** dont do a refresh to get the label up there, just
	    ** draw it
	    Text(graph,sx,sy,name);
	    */
	}
    }
    /*
    ** update cluster status window
    */

    DrawClustStat(GetMenu("/cluststat"));
}


/* one function for forward and backward. item->value determines
   forward or backward step */

MenuMovieFrame(item, value)
MenuItem	*item;
{
int	moviestart;
int	movieend;
int	loadstart;
int	loadend;
int	filestart;
int	fileend;
int	blocksize;
int	stepsize;

static int lastframestarttime = -1; /* Only initialized on first pass! */

char	str[256];
char    *ptr;
MenuItem *startitem;
DataSource *source;

 source = SelectedPlot(G)->source;

 /* Movie frame loading code is largely shared whether units are ids
 ** or times (all times converted to timestamps).
 **
 ** Note that 'id' is spike record id (i.e. the index into the parm
 ** file) and not the 'id' field of the spike record. */

 /* what units are we using ?*/
 if(GetItemState("/epochmenu/units_id")){ 

   /*
   ** Calculate id-specific values
   */

   /* file start/end spike record ids */
   filestart = 0;
   fileend =  source->nlines;

   /* movie start/end is the spike range for the whole movie */
   /* default moviestart/movieend = file start/end */
   ptr = GetItemValue("/epochmenu/moviestart");
   if (!ptr || ptr[0] == '\0')
     moviestart = 0;
   else
     moviestart = Atoi(ptr);

   ptr = GetItemValue("/epochmenu/movieend");   
   if (!ptr || ptr[0] == '\0')
     movieend = source->nlines;
   else
     movieend = Atoi(ptr);

   /* get start/end of current load*/
   loadstart = source->startline;
   if (loadstart == -1) loadstart = 0;

   loadend = source->endline;
   if (loadend == -1) loadend = source->nlines;

   /* default blocksize is size of currently loaded points */
   ptr = GetItemValue("/epochmenu/blocksize");
   if (!ptr || ptr[0] == '\0')
     blocksize = loadend - loadstart;
   else
     blocksize = abs(Atoi(ptr));

 } else {

   /*
   ** Calculate time-specific values
   */

   /* internally, use timestamps for all values */

   /* file start and end timestamps */
   filestart = source->mintimestamp;
   fileend = source->maxtimestamp;

   /* movie start/end is the time range for the whole movie */
   /* default : file start/end */
   ptr = GetItemValue("/epochmenu/moviestart");
   if (!ptr || ptr[0] == '\0')
     moviestart = filestart;
   else
     moviestart = ParseTimestamp(ptr);

   ptr = GetItemValue("/epochmenu/movieend");   
   if (!ptr || ptr[0] == '\0')
     movieend = fileend;
   else
     movieend = ParseTimestamp(ptr);

   /* we need to increment against the *requested* start time of the
      last frame (rather than the time of the first spike in the last
      load) to avoid jitter and hangs (if the increment isn't enough
      to get us to the next spike). 'lastframestarttime' is stored
      below, right after loading points. */
   if(source->inmovieflag)
     loadstart = lastframestarttime;
   else
     /* if last load wasn't a movie frame, then use the time of the
	first loaded spike */
     loadstart = source->starttimestamp;

   /* default blocksize is size of currently loaded points */
   ptr = GetItemValue("/epochmenu/blocksize");
   if (!ptr || ptr[0] == '\0')
     blocksize =   source->endtimestamp - source->starttimestamp;
   else
     /* or use value given by user */
     blocksize = ParseTimestamp(ptr);   
 }
 

 /*
 ** Begin code that is shared when using either record id or time
 ** units
 */

 /* Calculate stepsize (in % of blocksize) */

 /* special case: on first frame, don't increment */
 if (!source->inmovieflag)
   stepsize = 0;

 else{
   ptr = GetItemValue("/epochmenu/stepsize");
   if (!ptr || ptr[0] == '\0')
     /* default stepsize = 100% for frame by frame, 10% for movie */
     if (blockmovie_job_id)
       stepsize = 10;
     else
       stepsize = 100;
   else
     /* use stepsize provided by user */
     stepsize = abs(Atoi(ptr));
     
   /* convert from % value to actual step size (either timestamps or ids) */
   stepsize = (stepsize * 
	       Atoi(item->value) * /* +1 or -1, for forward/backward */
	       blocksize / 100); /* step is given as a % of blocksize */
 }


 /* use start of currently loaded points as start of current frame,
    unless moviestart is later in file (as after a full load) */
 loadstart = MAX(loadstart,moviestart);
 loadend = loadstart + blocksize;

 /* test for end of movie / out of bounds requests */
 /* note: we test *before* incrementing so that we go one frame 'past' start/end */
 if ((stepsize > 0 && 
      loadend >= movieend) ||
     (stepsize < 0 && 
      loadstart <= moviestart)){

   /* at the end, either stop the movie job (if exists), or loop and restart */
   if (GetItemState("/epochmenu/looptoggle")){
     if (stepsize > 0){
       loadstart = moviestart - stepsize;
       loadend = loadstart + blocksize;
     } else {
       loadend = movieend - stepsize;
       loadstart = loadend - blocksize;
     }
   } else{
     EndMovieJob();
     return;
   }
 }

 /* now increment window (note stepsize can be negative) */
 loadstart += stepsize;
 loadend += stepsize;

 /* 'crop' window end to requested movie end or file end */
 loadend = MIN(loadend, movieend);
 loadend = MIN(loadend, fileend);
 
 /* 'crop' window end to requested movie start or file start */
 loadstart = MAX(loadstart, moviestart);
 loadstart = MAX(loadstart, filestart);

 /* stuff values into point loading boxes and load points */
 sprintf(str,"%d",loadstart);
 PutItemValue("/epochmenu/start",str);
 sprintf(str,"%d",loadend);
 PutItemValue("/epochmenu/end",str);
 MenuLoadPoints(item);

 /* refresh menu */
 DrawMenu(item->menu);

 /* save the last requested frame start time (see above) */
 lastframestarttime = loadstart;

 /* this flag is unset by LoadPointsAndUpdateStatus (so we know when a
 ** non-movie point load has taken place in the interim); we reset it
 ** here */
 source->inmovieflag = 1;

}

ConvertBlocksizeToTime(itemname)
     char *itemname;
{
  char *val;
  char tmpstr[30];
  double tsperpoint;
  DataSource *source;

  val = GetItemValue(itemname);
  if (val!=NULL && val[0]!='\0') {
    source = SelectedPlot(G)->source;
    if (source->nlines) /* avoid div by zero */
      tsperpoint = (double) source->maxtimestamp / (double) source->nlines;

    sprintf(tmpstr, "%.4f", (Atoi(val)*tsperpoint/1e4));
    PutItemValue(itemname, tmpstr);
    RefreshItem(itemname);
  }
}

ConvertBlocksizeToPoints(itemname)
     char *itemname;
{
  char *val;
  char tmpstr[30];
  float tsperpoint;
  DataSource *source;

  val = GetItemValue(itemname);
  if (val!=NULL && val[0]!='\0') {
    source = SelectedPlot(G)->source;
    if (!source->nlines) /* no points in file; avoid div by zero */
      return;
    
    tsperpoint = (float) source->maxtimestamp / (float) source->nlines;
    
    sprintf(tmpstr, "%d", (lrint(ParseTimestamp(val)/tsperpoint)));
    PutItemValue(itemname, tmpstr);
    RefreshItem(itemname);
  }
}

ConvertPointsToTime(itemname)
     char *itemname;
{

  char *val=NULL;
  char tmpstr[30];

  val = GetItemValue(itemname);
  if (val!=NULL && val[0]!='\0') {
    sprintf(tmpstr, "%.4f", (double) (ParseTimestamp(LookupSpikeTime(NULL, val))/1e4));
    PutItemValue(itemname, tmpstr);
    RefreshItem(itemname);
  }

}

ConvertTimeToPoints(itemname)
     char *itemname;
{

  char *val=NULL;

  val = GetItemValue(itemname);
  if (val!=NULL && val[0]!='\0') {
    PutItemValue(itemname, LookupSpikeIndex(NULL, val));
    RefreshItem(itemname);
  }

}

MenuChangeUnits(item)
     MenuItem *item;
{


  if (strcmp(item->name, "/epochmenu/units_id")==0) {
    /* convert from time to points */
    ConvertTimeToPoints("/epochmenu/start");
    ConvertTimeToPoints("/epochmenu/moviestart");
    ConvertTimeToPoints("/epochmenu/end");
    ConvertTimeToPoints("/epochmenu/movieend");
    ConvertBlocksizeToPoints("/epochmenu/blocksize");

  } else {
    /* convert from points to time */
    ConvertPointsToTime("/epochmenu/start");
    ConvertPointsToTime("/epochmenu/moviestart");
    ConvertPointsToTime("/epochmenu/end");
    ConvertPointsToTime("/epochmenu/movieend");
    ConvertBlocksizeToTime("/epochmenu/blocksize");
  }


}

LoadPointsAndUpdateStatus(start, end, epoch)
     int start;
     int end;
     char *epoch;
{
  MenuWindow *menu;
  char startid[100], endid[100];
  Plot *p;
  int s, e;

  menu = GetMenu("/epochmenu");

  p = PartialLoadPlot(G,SelectedPlot(G),
		  start,end,-1,-1);

    /*
    ** determine the state of the toggle x autoscale button
    */
    if(GetItemState("/epochmenu/xautoscaletoggle")) {
	AutoScale(G,1,0);
    } 
    /*
    ** refresh the newly loaded points
    */
    ScaleAndRefreshGraph(G);

    /* update status */
/*     if (start<0) */
/* 	  start = 0; */
/*     if (end <0) */
/*       end = start + p->npoints-1; */
/*     sprintf(startid, "%d", start); */
/*     sprintf(endid, "%d", end); */
/*     s = Atoi(LookupSpikeTime(NULL, startid)); */
/*     e = Atoi(LookupSpikeTime(NULL, endid)); */

    UpdateStatusTime();    
    UpdateStatusEpoch(epoch);

    /* will be reset by movie load, if appropriate */
    SelectedPlot(G)->source->inmovieflag = 0;
 


}

MenuLoadAllPoints(item)
     MenuItem *item;
{
  char tmpstr[30];
  Plot *plot;
  int tempstart = 0;
  LoadPointsAndUpdateStatus(-1, -1, "");

  plot = SelectedPlot(F->graph);

  if (GetItemState("/epochmenu/units_id")) {

    if (plot->source->startline<0)
      tempstart = 0;
    else
      tempstart = plot->source->startline;

    sprintf(tmpstr, "%d", tempstart);
    PutItemValue("/epochmenu/start", tmpstr);
    PutItemValue("/epochmenu/moviestart", tmpstr);
    sprintf(tmpstr, "%d", tempstart + plot->npoints-1);
    PutItemValue("/epochmenu/end", tmpstr);
    PutItemValue("/epochmenu/movieend", tmpstr);
  } else {
    sprintf(tmpstr, "%.4f",(float) plot->source->starttimestamp/TSCALE);
    PutItemValue("/epochmenu/start", tmpstr);
    PutItemValue("/epochmenu/moviestart", tmpstr);
    sprintf(tmpstr, "%.4f", (float) plot->source->endtimestamp/TSCALE);
    PutItemValue("/epochmenu/end", tmpstr);
    PutItemValue("/epochmenu/movieend", tmpstr);
  }

  RefreshItem("/epochmenu/start");
  RefreshItem("/epochmenu/end");
  RefreshItem("/epochmenu/moviestart");
  RefreshItem("/epochmenu/movieend");

}


MenuLoadPoints(item)
MenuItem	*item;
{
/* char	*tstringstart; */
/* char	*tstringend; */
/* char	*epochname; */
/* Label	*label; */
/* char	name[100]; */
/* Graph	*graph; */

  char *start=NULL, *end=NULL;
  int   pointstart, pointend;
  MenuWindow *menu;

  if(!item) return;

  menu = GetMenu("/epochmenu");

  start = GetItemValue("/epochmenu/start");
  end = GetItemValue("/epochmenu/end");
 
  if (start==NULL || end == NULL || start[0]=='\0' || end[0] == '\0') {
    fprintf(stderr, "Load points: No start and/or end point defined\n");
    return;
  }

  /* are start and end given in time or points? */
  if(GetItemState("/epochmenu/units_id")) {

    /* points */
    pointstart = Atoi(start);
    pointend = Atoi(end);
    LoadPointsAndUpdateStatus(pointstart, pointend, "");
    return;

  } else {  /* time */
    
    /* convert to points first */
    if(start = LookupSpikeIndex(NULL, start)){
      pointstart = Atoi(start);
      if (end = LookupSpikeIndex(NULL, end)){
	pointend = Atoi(end);
	LoadPointsAndUpdateStatus(pointstart, pointend, "");
	return;
      }
    }
  }
	
  fprintf(stderr, "Error loading points: Invalid times\n");
  return;
}


MenuLoadEpochFile(MenuItem *item)
{
  if(item->value) /* test for NULL value */
    if(strcmp(item->value,"") !=0)
      LoadEpochFile(item->value);
}

int MenuSaveEpochFile(MenuItem *item)
{
  if(item->value!=NULL && strcmp(item->value,"") !=0)
   SaveEpochFile(item->value);
}

int MenuDefineNewEpoch(MenuItem *item)
{

  if(!item)
    return;
  
  if(item->state == 1){

    DefineNewEpoch(NULL);
  }
}


MenuGrab(item)
MenuItem	*item;
{
Plot	*plot;

    if(!item) return;
    plot = AddPlot(G,G->source);
    SelectPlot(plot);
    RefreshGraph(G);
}

MenuSetOverlay(item)
MenuItem	*item;
{
    if(!item) return;
    G->overlay = item->state;
}

MenuViewAvg(item)
MenuItem	*item;
{
char	*spikefile;
int	clusterid;
FILE	*fp;
int	count;
char	*tstart;
char	*tend;
char	command[200];
Plot	*plot;
char	*title;

    /*
    ** assume the the name of the spike file is in the menu item
    ** called /analysismenu/spikefile
    */
    spikefile = GetItemValue("/analysismenu/spikefile");
    tstart = GetItemValue("/analysismenu/tstart");
    tend = GetItemValue("/analysismenu/tend");
    if((plot = SelectedPlot(G)) == NULL){
	title = "";
    } else {
	title = plot->title;
    }
    /*
    clusterid = Atoi(item->value);
    */
    clusterid = G->selectedcluster;

    /*
    ** open a temporary file to hold the cluster ids
    */
    if((fp = fopen("@tmpclust@","w")) == NULL){
	fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	return;
    }
    count = WriteClusterPoints(G,clusterid,fp,1);
    fclose(fp);
    if(count <= 0){
	fprintf(stderr,"No points in the cluster\n");
	unlink("%tmpclust%");
	return;
    } else {
	fprintf(stderr,"Viewing average of %d spikes\n",count);
    }
    /*
    ** run the commands to view the spikes
    */
    sprintf(command,
    "spikeavg %s -tstart %s -tend %s -if @tmpclust@ | xplot -title \"%s cluster %d\" -command \"/global ; /color 0 ; /stepthrough 1\"&",
    spikefile,tstart,tend,title,clusterid);
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
}

MenuViewSpikes(item)
MenuItem	*item;
{
char	*spikefile;
int	clusterid;
FILE	*fp;
int	count;
char	*tstart;
char	*tend;
char	command[200];
Plot	*plot;
char	*title;
int	status;

    /*
    ** assume the the name of the spike file is in the menu item
    ** called /analysismenu/spikefile
    */
    spikefile = GetItemValue("/analysismenu/spikefile");
    tstart = GetItemValue("/analysismenu/tstart");
    tend = GetItemValue("/analysismenu/tend");
    if((plot = SelectedPlot(G)) == NULL){
	title = "";
    } else {
	title = plot->title;
    }
    /*
    clusterid = Atoi(item->value);
    */
    clusterid = G->selectedcluster;

    /*
    ** open a temporary file to hold the cluster ids
    */
    if((fp = fopen("@tmpclust@","w")) == NULL){
	fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	return;
    }
    count = WriteClusterPoints(G,clusterid,fp,1);
    fclose(fp);
    if(count <= 0){
	fprintf(stderr,"No points in the cluster\n");
	unlink("%tmpclust%");
	return;
    } else {
	fprintf(stderr,"Viewing %d spikes\n",count);
    }
    /*
    ** run the commands to view the spikes
    */
#ifdef OLD
    sprintf(command,
    "spikeavg %s -show -tstart %s -tend %s -if @tmpclust@ | xplot -title \"%s cluster %d\" -command \"/global ; /color 10 ; /stepthrough 1\"&",
    spikefile,tstart,tend,title,clusterid);
#else
    sprintf(command,
    "spikeavg %s -show -tstart %s -tend %s -if @tmpclust@ -o @tmps@\n",
    spikefile,tstart,tend);
#endif
    fprintf(stderr,"Executing command: %s\n",command);
    status = SYSTEM(command);
    fprintf(stderr,"status: %d\n",status);
    if(status != 0) return;

    /* embarassing HACK!: the 'nextspike' and 'prevspike' commands issued
       here are to workaround a bug that I can't figure out: namely
       why the first partialplotload never gets plotted, but will
       appear if you go forward then back one */
    sprintf(command,
    "xclust3 -suppresswarnings -partialplotload @tmps@ 1 1 -title \"%s cluster %d\" -command \"/global ; /color %d ; /geometry 750x350 ; /xnticks 0 ; /showaxis -3 ; /nextspike ; /prevspike\"&",
    title,clusterid, LookupClusterColor(G,clusterid));
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
}

MenuViewISI(item)
MenuItem	*item;
{
char	*spikefile;
int	clusterid;
FILE	*fp;
int	count;
char	command[200];
char	*binsize;
char	*tmax;
char	*tstart;
char	*tend;
Plot	*plot;
char	*title;
int	status;
int	gottime;
double	binarg;
double	tmaxarg;

    /*
    ** assume the the name of the spike file is in the menu item
    ** called /analysismenu/spikefile
    */
    spikefile = GetItemValue("/analysismenu/spikefile");
    binsize = GetItemValue("/analysismenu/binsize");
    tmax = GetItemValue("/analysismenu/tmax");
    tstart = GetItemValue("/analysismenu/tstart");
    tend = GetItemValue("/analysismenu/tend");
    if((plot = SelectedPlot(G)) == NULL){
	title = "";
    } else {
	title = plot->title;
    }
    /*
    clusterid = Atoi(item->value);
    */
    clusterid = G->selectedcluster;

    gottime = 0;
    /*
    ** open a temporary file to hold the cluster ids
    */
    if((fp = fopen("@tmpt@","w")) == NULL){
	fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	return;
    }
    if(count = WriteClusterTimestamps(G,clusterid,fp)){
	gottime = 1;
	fclose(fp);
    } else {
	fclose(fp);
	/*
	** open a temporary file to hold the cluster ids
	*/
	if((fp = fopen("@tmpclust@","w")) == NULL){
	    fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	    return;
	}
	count = WriteClusterPoints(G,clusterid,fp,1);
	fclose(fp);
    }
    if(count <= 0){
	fprintf(stderr,"No points in the cluster\n");
	unlink("%tmpclust%");
	return;
    } else {
	fprintf(stderr,"Inverse log ISI for %d spikes\n",count);
    }
    /*
    ** run the commands to view the isi
    */
    if(!gottime){
	sprintf(command,
	"spikeavg %s -showtimes -binary -tstart %s -tend %s -if @tmpclust@ -o @tmpt@",
	spikefile,tstart,tend);
	fprintf(stderr,"Executing command: %s\n",command);
	status = SYSTEM(command);
	fprintf(stderr,"status: %d\n",status);
	if(status != 0) return;
    }

/*     Turn the interface values (tmax, binsize), given in msec, into
**     sensible inputs to spikeanal -isi -logtime:
**
**     For a max_isi of x in msec, choose an argument that is log10 of
**     that time, as expected by spikeanal -isi -logtime
*/
    tmaxarg = log10(atof(tmax));

/*
**     For a max isi of 10^t msec, and a desired binsize of y msec, we
**     want to choose a binsize argument x (in increments of powers of
**     10) such that the middle-most bin (approx 10^t/2) is that size,
**     so: 
**
**         10^(t/2 + x) - 10^(t/2) = y                                           
**                    10^(t/2 + x) = y + 10^(t/2)                                
**                        t/2 + x  = log10 (y + 10^(t/2)) 
**                              x  = log10 (y + 10^(t/2)) - t/2 
**
*/      
    binarg  = log10(atof(binsize) + pow(10.0,(tmaxarg / 2.0))) - (tmaxarg / 2.0);


    sprintf(command,
    "spikeanal @tmpt@ -logtime -isi -bin %f -tmax %f | xplot -title \"logisi %s cluster %d\" -command \"/xlog 1 ; /autoscale ; /refresh\" &",
    binarg,tmaxarg,title,clusterid);
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
}

MenuViewAcorr(item)
MenuItem	*item;
{
char	*spikefile;
int	clusterid;
FILE	*fp;
int	count;
char	command[200];
char	*binsize;
char	*tmax;
char	*tstart;
char	*tend;
Plot	*plot;
char	*title;
int	status;
int	gottime;

    /*
    ** assume the the name of the spike file is in the menu item
    ** called /analysismenu/spikefile
    */
    spikefile = GetItemValue("/analysismenu/spikefile");
    binsize = GetItemValue("/analysismenu/binsize");
    tmax = GetItemValue("/analysismenu/tmax");
    tend = GetItemValue("/analysismenu/tend");
    tstart = GetItemValue("/analysismenu/tstart");
    /*
    clusterid = Atoi(item->value);
    */
    clusterid = G->selectedcluster;
    if((plot = SelectedPlot(G)) == NULL){
	title = "";
    } else {
	title = plot->title;
    }

    gottime = 0;
    /*
    ** open a temporary file to hold the cluster ids
    */
    if((fp = fopen("@tmpt@","w")) == NULL){
	fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	return;
    }
    if(count = WriteClusterTimestamps(G,clusterid,fp)){
	gottime = 1;
	fclose(fp);
    } else {
	fclose(fp);
	/*
	** open a temporary file to hold the cluster ids
	*/
	if((fp = fopen("@tmpclust@","w")) == NULL){
	    fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	    return;
	}
	count = WriteClusterPoints(G,clusterid,fp,1);
	fclose(fp);
    }
    if(count <= 0){
	fprintf(stderr,"No points in the cluster\n");
	unlink("%tmpclust%");
	return;
    } else {
	fprintf(stderr,"Autocorrelation for %d spikes\n",count);
    }
    /*
    ** run the commands to perform autocorr
    */
    if(!gottime){
	sprintf(command,
	"spikeavg %s -showtimes -binary -tstart %s -tend %s -if @tmpclust@ -o @tmpt@",
	spikefile,tstart,tend);
	fprintf(stderr,"Executing command: %s\n",command);
	status = SYSTEM(command);
	fprintf(stderr,"status: %d\n",status);
	if(status != 0) return;
    }
    sprintf(command,
    "spikeanal @tmpt@ -corr -bin %s -tmax %s | xplot -title \"acorr %s cluster %d\" &",
    binsize,tmax,title,clusterid);
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
}

MenuViewXcorr(item)
MenuItem	*item;
{
char	*spikefile;
int	clusterid;
int	clusterid2;
FILE	*fp;
int	count;
char	command[200];
char	*binsize;
char	*tmax;
char	*tstart;
char	*tend;
Plot	*plot;
char	*title;
int	status;
int	gottime;
char	*target;
char	*second;
char	*epoch;

    /*
    ** assume the the name of the spike file is in the menu item
    ** called /analysismenu/spikefile
    */
    spikefile = GetItemValue("/analysismenu/spikefile");
    binsize = GetItemValue("/analysismenu/binsize");
    tmax = GetItemValue("/analysismenu/tmax");
    tend = GetItemValue("/analysismenu/tend");
    tstart = GetItemValue("/analysismenu/tstart");
    target = GetItemValue("/analysismenu/viewxcorr");
    epoch = GetItemValue("/epochmenu/epochname");
    /*
    clusterid = Atoi(item->value);
    */
    clusterid = G->selectedcluster;
    if((plot = SelectedPlot(G)) == NULL){
	title = "";
    } else {
	title = plot->title;
    }

    gottime = 0;
    /*
    ** PRIMARY
    ** open a temporary file to hold the cluster ids of the primary spike
    ** train
    */
    if((fp = fopen("@tmpt@","w")) == NULL){
	fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	return;
    }
    if(count = WriteClusterTimestamps(G,clusterid,fp)){
	gottime = 1;
	fclose(fp);
    } else {
	gottime = 0;
	fclose(fp);
	/*
	** open a temporary file to hold the cluster ids
	*/
	if((fp = fopen("@tmpclust@","w")) == NULL){
	    fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	    return;
	}
	count = WriteClusterPoints(G,clusterid,fp,1);
	fclose(fp);
    }
    if(count <= 0){
	fprintf(stderr,"No points in the cluster\n");
	unlink("@tmpclust@");
	return;
    } else {
	fprintf(stderr,"Crosscorrelation of %d spikes\n",count);
	/*
	** make the tfile if necessary
	*/
	if(!gottime){
	    sprintf(command,
	    "spikeavg %s -showtimes -binary -tstart %s -tend %s -if @tmpclust@ -o @tmpt@",
	    spikefile,tstart,tend);
	    fprintf(stderr,"Executing command: %s\n",command);
	    status = SYSTEM(command);
	    fprintf(stderr,"status: %d\n",status);
	    if(status != 0) return;
	}
    }
    /*
    ** SECONDARY
    ** open a temporary file to hold the cluster ids of the secondary spike
    ** train
    */
    /*
    ** is the target a clusterid or a file
    */
    if(target[0] > '0' && target[0] <= '9'){
	clusterid2 = atoi(target);
	if((fp = fopen("@tmpt2@","w")) == NULL){
	    fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	    return;
	}
	if(count = WriteClusterTimestamps(G,clusterid2,fp)){
	    gottime = 1;
	    fclose(fp);
	} else {
	    gottime = 0;
	    fclose(fp);
	    /*
	    ** open a temporary file to hold the cluster ids
	    */
	    if((fp = fopen("@tmpclust2@","w")) == NULL){
		fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
		return;
	    }
	    count = WriteClusterPoints(G,clusterid2,fp,1);
	    fclose(fp);
	}
	if(count <= 0){
	    fprintf(stderr,"No points in the cluster\n");
	    unlink("@tmpclust2@");
	    return;
	} else {
	    fprintf(stderr,"with %d spikes\n",count);
	}
	if(!gottime){
	    sprintf(command,
	    "spikeavg %s -showtimes -binary -tstart %s -tend %s -if @tmpclust2@ -o @tmpt2@",
	    spikefile,tstart,tend);
	    fprintf(stderr,"Executing command: %s\n",command);
	    status = SYSTEM(command);
	    fprintf(stderr,"status: %d\n",status);
	    if(status != 0) return;
	}
	second = "@tmpt2@";
    } else {
	second = target;
    }
    /*
    ** run the commands to perform xcorr
    */
    sprintf(command,
    "spikeanal @tmpt@ %s -corr -bin %s -tmax %s | xplot -title \"xcorr %s %s cluster %d and %s\" &",
    second,binsize,tmax,title,epoch,clusterid,target);
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
}

MenuViewRate(item)
MenuItem	*item;
{
char	*spikefile;
int	clusterid;
FILE	*fp;
int	count;
char	command[200];
char	*binsize;
char	*tmax;
char	*tstart;
char	*tend;
Plot	*plot;
char	*title;
int	status;
int	gottime;

    /*
    ** assume the the name of the spike file is in the menu item
    ** called /analysismenu/spikefile
    */
    spikefile = GetItemValue("/analysismenu/spikefile");
    binsize = GetItemValue("/analysismenu/binsize");
    tmax = GetItemValue("/analysismenu/tmax");
    tstart = GetItemValue("/analysismenu/tstart");
    tend = GetItemValue("/analysismenu/tend");
    /*
    clusterid = Atoi(item->value);
    */
    clusterid = G->selectedcluster;
    if((plot = SelectedPlot(G)) == NULL){
	title = "";
    } else {
	title = plot->title;
    }

    gottime = 0;
    /*
    ** open a temporary file to hold the cluster ids
    */
    if((fp = fopen("@tmpt@","w")) == NULL){
	fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	return;
    }
    if(count = WriteClusterTimestamps(G,clusterid,fp)){
	gottime = 1;
	fclose(fp);
    } else {
	fclose(fp);
	/*
	** open a temporary file to hold the cluster ids
	*/
	if((fp = fopen("@tmpclust@","w")) == NULL){
	    fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	    return;
	}
	count = WriteClusterPoints(G,clusterid,fp,1);
	fclose(fp);
    }
    if(count <= 0){
	fprintf(stderr,"No points in the cluster\n");
	unlink("%tmpclust%");
	return;
    } else {
	fprintf(stderr,"Firing rate for %d spikes\n",count);
    }
    /*
    ** run the commands to perform autocorr
    */
    if(!gottime){
	sprintf(command,
	"spikeavg %s -showtimes -binary -tstart %s -tend %s -if @tmpclust@ -o @tmpt@",
	spikefile,tstart,tend);
	fprintf(stderr,"Executing command: %s\n",command);
	status = SYSTEM(command);
	fprintf(stderr,"status: %d\n",status);
	if(status != 0) return;
    }
    sprintf(command,
    "spikeanal @tmpt@ -rate -bin %s -tmax %s -o @tmprate@",
    binsize,tmax);
    fprintf(stderr,"Executing command: %s\n",command);
    status = SYSTEM(command);
    fprintf(stderr,"status: %d\n",status);
    if(status != 0) return;
    sprintf(command, 
    "header @tmprate@; xplot @tmprate@ -title \"rate %s cluster %d\"&",
    title,clusterid);
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
}

MenuViewBehav(item)
MenuItem	*item;
{
char	*positionfile;
char	*spikefile;
int	clusterid;
FILE	*fp;
int	count;
char	command[200];
int	gridsize;
char	*tstart;
char	*tend;
char	*title;
Plot	*plot;
int	status;
char	trangestr[100];
char	*trangefile;
int	gottime;

    /*
    ** assume the the name of the spike file is in the menu item
    ** called /analysismenu/spikefile
    */
    if((positionfile = GetItemValue("/analysismenu/positionfile")) == NULL) {
	fprintf(stderr,"No position file specified\n");
	return;
    }
    /*
    ** get the name of the timestamp range file if any
    */
    trangefile = GetItemValue("/analysismenu/trangefile");
    if(IsStringEmpty(trangefile)){
	trangestr[0] = '\0';
    } else {
	sprintf(trangestr,"-trangefile %s",trangefile);
    }
    if((spikefile = GetItemValue("/analysismenu/spikefile")) == NULL){
	fprintf(stderr,"No spike file specified\n");
	return;
    }
    gridsize = atoi(GetItemValue("/analysismenu/gridsize"));
    tstart = GetItemValue("/analysismenu/tstart");
    tend = GetItemValue("/analysismenu/tend");
    if((plot = SelectedPlot(G)) == NULL){
	title = "";
    } else {
	title = plot->title;
    }
    /*
    clusterid = Atoi(item->value);
    */
    clusterid = G->selectedcluster;

    gottime = 0;
    /*
    ** open a temporary file to hold the cluster ids
    */
    if((fp = fopen("@tmpt@","w")) == NULL){
	fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	return;
    }
    if(count = WriteClusterTimestamps(G,clusterid,fp)){
	gottime = 1;
	fclose(fp);
    } else {
	fclose(fp);
	/*
	** open a temporary file to hold the cluster ids
	*/
	if((fp = fopen("@tmpclust@","w")) == NULL){
	    fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	    return;
	}
	count = WriteClusterPoints(G,clusterid,fp,1);
	fclose(fp);
    }
    if(count <= 0){
	fprintf(stderr,"No points in the cluster\n");
	unlink("%tmpclust%");
	return;
    } else {
	fprintf(stderr,"Behavioral correlate for %d spikes\n",count);
    }
    /*
    ** run the commands to perform behavioral correlation
    */
    if(!gottime){
	sprintf(command,
	"spikeavg %s -showtimes -binary -tstart %s -tend %s -if @tmpclust@ -o @tmpt@",
	spikefile,tstart,tend);
	fprintf(stderr,"Executing command: %s\n",command);
	status = SYSTEM(command);
	fprintf(stderr,"status: %d\n",status);
	if(status != 0) return;
    }
    sprintf(command,
    "behav %s -v -t @tmpt@ -tstart %s -tend %s %s -xsize %d -ysize %d -xview -o @tmpbehav@",
    positionfile,tstart,tend,trangestr,328*gridsize/244,gridsize);
    fprintf(stderr,"Executing command: %s\n",command);
    status = SYSTEM(command);
    fprintf(stderr,"status: %d\n",status);
    if(status != 0) return;
    sprintf(command, "xview @tmpbehav@ -t \"%s cluster %d\"&",
    title,clusterid);
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
}

MenuViewSpot(item)
MenuItem	*item;
{
char	*positionfile;
char	*spikefile;
int	clusterid;
FILE	*fp;
int	count;
char	command[200];
int	gridsize;
char	*tstart;
char	*tend;
char	*title;
Plot	*plot;
int	status;
char	*trangefile;
char	trangestr[100];
int	gottime;

    /*
    ** assume the the name of the spike file is in the menu item
    ** called /analysismenu/spikefile
    */
    if((positionfile = GetItemValue("/analysismenu/positionfile")) == NULL) {
	fprintf(stderr,"No position file specified\n");
	return;
    }
    if((spikefile = GetItemValue("/analysismenu/spikefile")) == NULL){
	fprintf(stderr,"No spike file specified\n");
	return;
    }
    gridsize = atoi(GetItemValue("/analysismenu/gridsize"));
    tstart = GetItemValue("/analysismenu/tstart");
    tend = GetItemValue("/analysismenu/tend");
    if((plot = SelectedPlot(G)) == NULL){
	title = "";
    } else {
	title = plot->title;
    }
    /*
    ** get the name of the timestamp range file if any
    */
    trangefile = GetItemValue("/analysismenu/trangefile");
    if(IsStringEmpty(trangefile)){
	trangestr[0] = '\0';
    } else {
	sprintf(trangestr,"-trangefile %s",trangefile);
    }
    /*
    clusterid = Atoi(item->value);
    */
    clusterid = G->selectedcluster;

    gottime = 0;
    /*
    ** open a temporary file to hold the cluster ids
    */
    if((fp = fopen("@tmpt@","w")) == NULL){
	fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	return;
    }
    if(count = WriteClusterTimestamps(G,clusterid,fp)){
	gottime = 1;
	fclose(fp);
    } else {
	fclose(fp);
	/*
	** open a temporary file to hold the cluster ids
	*/
	if((fp = fopen("@tmpclust@","w")) == NULL){
	    fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	    return;
	}
	count = WriteClusterPoints(G,clusterid,fp,1);
	fclose(fp);
    }
    if(count <= 0){
	fprintf(stderr,"No points in the cluster\n");
	unlink("%tmpclust%");
	return;
    } else {
	fprintf(stderr,"Behavioral correlate for %d spikes\n",count);
    }
    /*
    ** run the commands to perform behavioral correlation
    */
    if(!gottime){
	sprintf(command,
	"spikeavg %s -showtimes -binary -tstart %s -tend %s -if @tmpclust@ -o @tmpt@",
	spikefile,tstart,tend);
	fprintf(stderr,"Executing command: %s\n",command);
	status = SYSTEM(command);
	fprintf(stderr,"status: %d\n",status);
	if(status != 0) return;
    }
    sprintf(command,
    "behav %s -v -t @tmpt@ %s -tstart %s -tend %s -xsize %d -ysize %d -directionline -oo @tmpocc@ -o /dev/null",
    positionfile,trangestr,tstart,tend,gridsize*328/244,gridsize);
    fprintf(stderr,"Executing command: %s\n",command);
    status = SYSTEM(command);
    fprintf(stderr,"status: %d\n",status);
    if(status != 0) return;
    sprintf(command, "y -col 1 -col 2 -col 4 @tmpocc@ > @tmpspot@ &");
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
    if(status != 0) return;
    sprintf(command, "grep wline @tmpocc@ > @tmpline@ &");
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
    if(status != 0) return;
    sprintf(command, "xplot @tmpspot@ @tmpline@ -title \"%s cluster %d\" -command \"/scatter ; /pointstyle 8 ; /etype 2 ; /escale .2 \"&",
    title,clusterid);
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
}

MenuViewDir(item)
MenuItem	*item;
{
char	*positionfile;
char	*spikefile;
int	clusterid;
FILE	*fp;
int	count;
char	command[200];
char	*gridsize;
char	*tstart;
char	*tend;
char	*title;
Plot	*plot;
int	status;
char	*trangefile;
char	trangestr[100];
int	gottime;

    /*
    ** assume the the name of the spike file is in the menu item
    ** called /analysismenu/spikefile
    */
    if((positionfile = GetItemValue("/analysismenu/positionfile")) == NULL) {
	fprintf(stderr,"No position file specified\n");
	return;
    }
    if((spikefile = GetItemValue("/analysismenu/spikefile")) == NULL){
	fprintf(stderr,"No spike file specified\n");
	return;
    }
    gridsize = GetItemValue("/analysismenu/gridsize");
    tstart = GetItemValue("/analysismenu/tstart");
    tend = GetItemValue("/analysismenu/tend");
    if((plot = SelectedPlot(G)) == NULL){
	title = "";
    } else {
	title = plot->title;
    }
    /*
    ** get the name of the timestamp range file if any
    */
    trangefile = GetItemValue("/analysismenu/trangefile");
    if(IsStringEmpty(trangefile)){
	trangestr[0] = '\0';
    } else {
	sprintf(trangestr,"-trangefile %s",trangefile);
    }
    /*
    clusterid = Atoi(item->value);
    */
    clusterid = G->selectedcluster;

    gottime = 0;
    /*
    ** open a temporary file to hold the cluster ids
    */
    if((fp = fopen("@tmpt@","w")) == NULL){
	fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	return;
    }
    if(count = WriteClusterTimestamps(G,clusterid,fp)){
	gottime = 1;
	fclose(fp);
    } else {
	fclose(fp);
	/*
	** open a temporary file to hold the cluster ids
	*/
	if((fp = fopen("@tmpclust@","w")) == NULL){
	    fprintf(stderr,"ERROR: unable to open temporary cluster index file\n");
	    return;
	}
	count = WriteClusterPoints(G,clusterid,fp,1);
	fclose(fp);
    }
    if(count <= 0){
	fprintf(stderr,"No points in the cluster\n");
	unlink("%tmpclust%");
	return;
    } else {
	fprintf(stderr,"Behavioral correlate for %d spikes\n",count);
    }
    /*
    ** run the commands to perform behavioral correlation
    */
    if(!gottime){
	sprintf(command,
	"spikeavg %s -showtimes -binary -tstart %s -tend %s -if @tmpclust@ -o @tmpt@",
	spikefile,tstart,tend);
	fprintf(stderr,"Executing command: %s\n",command);
	status = SYSTEM(command);
	fprintf(stderr,"status: %d\n",status);
	if(status != 0) return;
    }
    sprintf(command,
    "behav %s -v -t @tmpt@ %s -tstart %s -tend %s -dpolar -dsize %s -o @tmpd@",
    positionfile,trangestr,tstart,tend,gridsize);
    fprintf(stderr,"Executing command: %s\n",command);
    status = SYSTEM(command);
    fprintf(stderr,"status: %d\n",status);
    if(status != 0) return;
    sprintf(command, "xplot @tmpd@ -title \"%s cluster %d\" &",
    title,clusterid);
    fprintf(stderr,"Executing command: %s\n",command);
    SYSTEM(command);
}

MenuHideAllBounds(item)
     MenuItem *item;
{
  F->graph->showbounds = !F->graph->showbounds;
  RefreshGraph(F->graph);
}

MenuHideCluster(item)
MenuItem	*item;
{
int	clusterid;

    if(item && item->value){
#ifdef OLD
	if(item->state == 1){
	    clusterid = Atoi(item->value);
	} else {
	    clusterid = -Atoi(item->value) - 1;
	}
#endif
	clusterid = Atoi(item->value);
	HideCluster(G,clusterid,item->state);
	ScaleAndRefreshGraph(G);
    }
}

MenuButtonHideCluster(item)
MenuItem	*item;
{
int	clusterid;
char	clustname[80];

    if(item && item->value){
#ifdef OLD
	if(item->state == 1){
	    clusterid = Atoi(item->value);
	} else {
	    clusterid = -Atoi(item->value) - 1;
	}
#endif
	sprintf(clustname,"/controlmenu/cl%s",item->value);
	clusterid = Atoi(GetItemValue(clustname));
	HideCluster(G,clusterid,item->state);
	ScaleAndRefreshGraph(G);
    }
}

MenuButtonShowOnlyCluster(item)
MenuItem	*item;
{
int	clusterid;
char	clustname[80];
ClusterList	*clist;
ClusterBounds	*cb;
Graph		*graph;
Plot		*plot;
int		i;
int		state;
MenuItem	*tmpitem;

    if(item && item->value){
	sprintf(clustname,"/controlmenu/cl%s",item->value);
	clusterid = Atoi(GetItemValue(clustname));
	graph = GetGraph("/graph");
	/*
	** send all points back to the zero cluster
	*/
	for(plot=graph->plot;plot;plot=plot->next){
	    for(i=0;i<plot->npoints;i++){
		plot->rawdata[i].clusterid = 0;
	    }
	}
	TouchClusters(graph);
	/*
	** go through the cluster list 
	*/
	if(item->state == 1){
	    for(clist=graph->clusterlist;clist;clist=clist->next){
		if(clist->clusterid == clusterid){
		    /*
		    ** only enable the selected cluster
		    */
		    clist->enabled = 1;
		    clist->showonly = 1;
		} else {
		    /*
		    ** disable everything else
		    */
		    clist->enabled = 0;
		    clist->showonly = 0;
		}
	    }
	    for(cb=graph->clusterbounds;cb;cb=cb->next){
		if(cb->clusterid == clusterid){
		    cb->enabled = 1;
		} else {
		    cb->enabled = 0;
		}
	    }
	    /*
	    ** and redo the cluster assignments
	    */
	    AssignClusterPointsToCluster(graph,clusterid);
	    /*
	    ** and hide the zero cluster
	    HideCluster(graph,0,1);
	    */
	} else {
	    /*
	    ** reenable all clusters
	    */
	    for(clist=graph->clusterlist;clist;clist=clist->next){
		clist->enabled = 1;
		clist->showonly = 0;
	    }
	    for(cb=graph->clusterbounds;cb;cb=cb->next){
		cb->enabled = 1;
	    }
	    /*
	    ** and redo the cluster assignments
	    */
	    AssignClusterPoints(graph);
	    /*
	    ** and put the zero cluster hidden state back to the
	    ** way it was
	    state = GetItemState("/menu/hidecluster");
	    HideCluster(graph,0,state);
	    */
	}
	/*
	** select the cluster
	*/
	CallMenuItem(clustname);
	AssignClusterButtons();
	UpdateClusterLabels();
	RefreshGraph(graph);
    }
}

MenuEnableCluster(item)
MenuItem	*item;
{
int	clusterid;
char	clustname[80];

    if(item && item->value){
	sprintf(clustname,"/controlmenu/cl%s",item->value);
	clusterid = Atoi(GetItemValue(clustname));
	SetClusterEnable(G,clusterid,!item->state);
	/*
	ScaleAndRefreshGraph(G);
	*/
	RefreshGraph(G);
    }
}

MenuReadClusterBounds(item)
MenuItem	*item;
{
    if(item && item->value){
	ReadClusterBounds(G,item->value);
    }
}

MenuWriteClusterBounds(item)
MenuItem	*item;
{
    if(item && item->value){
	WriteAllClusterBounds(G,item->value);
    }
}

MenuCopyCluster(item,event)
MenuItem	*item;
XButtonReleasedEvent *event;
{
  int	i, newval;

  if (!item)
    return (0);

  i = 0;
  newval = 0;    
  
  /* get current value, if available */
  if (item->value){
    i = Atoi(item->value);

    /* test for out-of-bounds value */
    if (i < 1 || i > MAXCLUSTER){
      free(item->value);
      item->value = NULL;
      return(0);
    }
  }

  switch (event->button) {
  case 1: /* left-click, copy cluster */
    if(item->value &&
       GetClusterInfo(G->selectedcluster) &&  /* don't copy empty clusters */
       G->selectedcluster != i)  /* don't copy a cluster to itself */
      CopyClusterBounds(G,i);
    return(0);

  case 4: /*mousewheel up, prev empty, but no rollover*/
    for (i--; i > 0; i--){
      if (!LookupCluster(i)){
	newval = i;
	break;
      }
    }
    break;

  case 5: /*mousewheel down, next empty*/
    for (i++; i < MAXCLUSTER; i++){
      if (!LookupCluster(i)){
	newval = i;
	break;
      }
    }
  }
  if (newval){ /* we found an empty cluster smaller than i */
    
    free(item->value);
    item->value = (char *)malloc(ceil(log10((double)MAXCLUSTER))*sizeof(char));
    sprintf(item->value, "%d",newval);
  }
}

MenuSetClusterScore(item,event)
MenuItem	*item;
XButtonReleasedEvent *event;
{
  ClusterList *cl;
  char s[256];
  int i;

  if (!item ||
      !item->value)
    return (0);

/* commented out as replaced with menuclassdrawitems  */

/*   /\* Just redraw items if this is not a real click*\/ */
/*   if (event == NULL){  */
/*     DrawItem(item);  */
/*     return (0);  */
/*   } */

  /* get current value */
  if((cl = GetClusterInfo(G->selectedcluster)))
    i = cl->score;
  
  /* ... or return if selected cluster is empty */
  else {
    item->state = 0;
    return(0);
  }
  
  if(event){ /* MenuClassCallItems calls all items with event NULL */
    switch (event->button) {
    case 1: /* left-click, set cluster score*/
	cl->score = Atoi(item->value);
      break;
    
    case 4: /*mousewheel up, next lowest score*/
      if (i > 1){
	i--;
	sprintf(s,"/clustermenu/clusterscore%d",i);
	CallMenuItem(s);
      }
      break;

    case 5: /*mousewheel down, next empty*/
      if (i < MAX_CLUST_SCORE){
	i++;
	sprintf(s,"/clustermenu/clusterscore%d",i);
	CallMenuItem(s);
      }
      break;
    }
  }
}

MenuUnsetClusterScore(item,event)
MenuItem	*item;
XButtonReleasedEvent *event;
{
  ClusterList *cl;

  if((cl = (GetClusterInfo(G->selectedcluster))))
    cl->score = 0;
  RefreshClusterScoreButtons();

}

/* Set the state of the cluster score buttons to reflect the score of
**   the currently selected cluster 
*/
RefreshClusterScoreButtons()
{
  int clusterid;
  char itemstr[100];
  ClusterList *cl;
  MenuItem *item;
  int score;
  MenuWindow *menu;

  clusterid = G->selectedcluster;
  menu = GetMenu("/clustermenu");

  /* does cluster not exist ?*/
  if (!LookupCluster(clusterid)){
    /* set all buttons to 'off' */
    MenuClassSetState(NULL,menu,"score",0);
    MenuClassDrawItems(NULL,menu,"score"); /* redraw buttons */
    return;
  }
  
  /* cluster exists; get cluster info */
  score = (GetClusterInfo(clusterid))->score;

  /* has cluster not been scored yet? */
  if (score == 0){
    /* set all buttons to 'off' */
    MenuClassSetState(NULL,menu,"score",0);
    MenuClassDrawItems(NULL,menu,"score"); /* redraw buttons */
    return;
  }

  /* get cluster score button */
  sprintf(itemstr,
	  "/clustermenu/clusterscore%d",
	  score);
  item = LookupMenuItem(itemstr);

  /* set button states */
  item->state = 1;
  MenuClassSetState(item,item->menu,item->class,0);

  /* redraw score buttons */
  MenuClassDrawItems(NULL,menu,"score"); /* redraw buttons */
}

MenuDeleteCluster()
{
    DoDeleteCluster(G);
}

MenuDeleteClusterBounds()
{
    DoDeleteClusterBounds(G);
}

MenuDeleteAllClusters()
{
    DoDeleteAllClusters(G);
}

MenuWriteClusters(item)
MenuItem	*item;
{
/* int	applyflag; */

    if(!item) return;
/*     /\* */
/*     ** check the status of the apply to points flag */
/*     *\/ */
/*     applyflag = GetItemState("/clustermenu/applytopoints"); */

/*     if(applyflag == 0){		 */
/* 	/\* */
/* 	** apply to loaded points */
/* 	*\/ */
/* 	if(item->value){ */
/* 	    WriteClusters(G,item->value); */
/* 	} */
/*     } else { */
/* 	if(item->value){ */
/* 	    LoadAndWriteClusters(G,item->value); */
/* 	} */
/*     } */

    /*
    ** apply to loaded points
    */
    if(item->value){
      WriteClusters(G,item->value);
    }
 }


MenuWriteClusterIndices(item)
MenuItem	*item;
{
    if(item && item->value){
	WriteClusterIndices(G,item->value);
    }
}

MenuSetCluster(item)
MenuItem	*item;
{
  MenuWindow *menu;
    if(!item){
	return;
    }
    /*
    ** set the color of the item based on the color of the cluster
    */
    item->itemforeground =
	LookupClusterColor(G,Atoi(item->value));
    if(item->state == 1){
	/*
	** if colors are being rotated then restore the previous
	** clusters original color
	*/
	if(rotatecolor){
	    RestorePixel();
	    RefreshGraph(G);
	    }
	G->selectedcluster = Atoi(item->value);
	RefreshClusterScoreButtons();
	RefreshSelectedClusterButton();
	SavePixel();
	MenuClassDrawItems(NULL,item->menu,item->class);
	/*
	** update the cluster info panel
	*/
	menu = GetMenu("/cluststat");
	if(menu->mapped){
	    DrawClustStat(menu);
	}
	G->supercolor = item->itemforeground;
    } 
}

SelectCluster(int clusterid)
{

  MenuItem *item;
  char cname[100];
  MenuWindow *menu;

  if (clusterid > MAXCLUSTER ||
      clusterid < 1){
    fprintf(stderr,"ERROR: Can't select cluster # %d. Out of permitted range 1 - %d\n",clusterid, MAXCLUSTER);
    return;
  }
  
  /* scroll down to correct block, if necessary */
  while(clusterid > menu_cluster_block + CLUSTER_BLOCK_SIZE){
    MenuClusterBlockUp(NULL);
  }

  /* scroll up to correct block, if necessary */
  while(clusterid <= menu_cluster_block){
    MenuClusterBlockDown(NULL);
  }

  sprintf(cname, "/controlmenu/cl%d",(((clusterid-1)%CLUSTER_BLOCK_SIZE)
			       +1));
  if ((item = LookupMenuItem(cname)) !=NULL ) {
    /*
    ** set the color of the item based on the color of the cluster
    */
    item->itemforeground =
      LookupClusterColor(G,clusterid);
    item->state = 1;
    if(rotatecolor){

      /* To change the selected cluster while in 'flash mode':*/

      /*  - restore the 'real' color to the old cluster */
      RestorePixel();

      /*  -redraw the graph so the old cluster's points are drawn in 
	  the 'real' color */
      RefreshGraph(G);

      /*  -select the new cluster */
      G->selectedcluster = clusterid;    

      /*  -save the new cluster's 'real' color */
      SavePixel();

    }
    else {
      G->selectedcluster = clusterid;
    }
    RefreshSelectedClusterButton();
    RefreshClusterScoreButtons();

    /* Set cluster button states and redraw */
    MenuClassSetState(item,item->menu,item->class,0);
    MenuClassCallItems(item,item->menu,item->class);
    /*
    ** update the cluster info panel
    */
    menu = GetMenu("/cluststat");
    if(menu->mapped)
      DrawClustStat(menu);
    
    G->supercolor = item->itemforeground;
  }
}

RefreshSelectedClusterButton()
{
int	i;
int		cid;
int		cval;
 int		ccol;
char		cname[100];
MenuItem	*citem;

    for(i=0;i<CLUSTER_BLOCK_SIZE;i++){
	cid = i+1;
	cval = i+menu_cluster_block+1;
	
	/*
	** lookup the buttons for the cluster  
	*/
	sprintf(cname,"/controlmenu/cl%d",cid);
	if((citem = LookupMenuItem(cname)) != NULL){
	  
	  /* set the color of the items based on the color of the cluster */
	  ccol = LookupClusterColor(G,cval);
	  citem->itemforeground = ccol;
	  
	  /* set the supercolor */
	  if(G->selectedcluster == cval){
	    G->supercolor = ccol;
	  }
	  
	  DrawItem(citem);
	  
	  
	  sprintf(cname,"/controlmenu/clenable%d",cid);
	  if((citem = LookupMenuItem(cname)) != NULL){
	    /* change the color of the bitmap to use the cluster color */
 	    if (citem->bitmap1) XFreePixmap(citem->display,citem->bitmap1);
 	    if (citem->bitmap2) XFreePixmap(citem->display,citem->bitmap2);
 	    citem->bitmap1 = CreateBitmap(citem->menu,button_bits,16,16,BLACK,ccol);
	    citem->bitmap2 = CreateBitmap(citem->menu,button2_bits,16,16,BLACK,ccol);
	    DrawItem(citem);
	  }
	  
	  sprintf(cname,"/controlmenu/clhide%d",cid);
	  if((citem = LookupMenuItem(cname)) != NULL){
	    /* change the color of the bitmap to use the cluster color */
 	    if (citem->bitmap1) XFreePixmap(citem->display,citem->bitmap1);
 	    if (citem->bitmap2) XFreePixmap(citem->display,citem->bitmap2);
 	    citem->bitmap1 = CreateBitmap(citem->menu,eye_bits,16,16,BLACK,ccol);
	    citem->bitmap2 = CreateBitmap(citem->menu,eye2_bits,16,16,BLACK,ccol);
	    DrawItem(citem);
	  }

	  sprintf(cname,"/controlmenu/clshowonly%d",cid);
	  if((citem = LookupMenuItem(cname)) != NULL){
	    /* change the color of the bitmap to use the cluster color */
	    if (citem->bitmap1) XFreePixmap(citem->display,citem->bitmap1);
 	    if (citem->bitmap2) XFreePixmap(citem->display,citem->bitmap2);
	    citem->bitmap1 = CreateBitmap(citem->menu,button_bits,16,16,BLACK,ccol);
	    citem->bitmap2 = CreateBitmap(citem->menu,button2_bits,16,16,BLACK,ccol);
	    DrawItem(citem);
	  }
	}
    }
}



AssignClusterButtons()
{
int 	i;
ProjectionInfo	*pinfo;
char		cname[100];
char		*tmpname;
MenuItem	*citem;
int		cid;
int		cval;
Graph		*graph;
ClusterList	*clist;

    if((graph = GetGraph("/graph")) == NULL){
	fprintf(stderr,"ERROR: undefined graph\n");
	return;
    }
    for(i=0;i<CLUSTER_BLOCK_SIZE;i++){
	cid = i+1;
	cval = i+menu_cluster_block+1;
	/*
	** lookup up the buttons for the cluster  
	*/
	sprintf(cname,"/controlmenu/cl%d",cid);
	if((citem = LookupMenuItem(cname)) != NULL){
	    /*
	    ** assign cval to the item value 
	    */
	    sprintf(cname,"%d",cval);
	    tmpname = (char *) malloc(strlen(cname)+1);
	    sprintf(tmpname,cname);
	    citem->value = tmpname;
	    /*
	    ** is this cluster selected
	    */
	    if(graph->selectedcluster == cval){
		citem->state = 1;
	    } else {
		citem->state = 0;
	    }
	    /*
	    ** set the color of the item based on the color of the cluster
	    */
	    citem->itemforeground =
		LookupClusterColor(graph,cval);
	    /*
	    ** redraw the updated item
	    */
	    DrawItem(citem);
	}
	/*
	** is this cluster defined
	*/
	sprintf(cname,"/controlmenu/clmask%d",cid);
	if((citem = LookupMenuItem(cname)) != NULL){
	    if(LookupCluster(cval)){
		/*
		** if so then unmask it by unmapping the obscuring button
		*/
		citem->mapped = 0;
	    } else {
		/*
		** otherwise cover it up
		*/
		citem->mapped = 1;
	    }
	    /*
	    ** redraw the updated item
	    */
	    UpdateItemMapping(citem);
	    DrawItem(citem);
	}
	/*
	** is this cluster hidden
	*/
	sprintf(cname,"/controlmenu/clhide%d",cid);
	if((citem = LookupMenuItem(cname)) != NULL){
	    if(graph->hiddencluster[cval]){
		citem->state = 1;
	    } else {
		citem->state = 0;
	    }
	    /*
	    ** redraw the updated item
	    */
	    DrawItem(citem);
	}
	/*
	** is this cluster enabled
	*/
	sprintf(cname,"/controlmenu/clenable%d",cid);
	if((citem = LookupMenuItem(cname)) != NULL){
	    if((clist = GetClusterInfo(cval)) != NULL){
		if(clist->enabled){
		    citem->state = 0;
		} else {
		    citem->state = 1;
		}
		/*
		** redraw the updated item
		*/
		DrawItem(citem);
#ifdef OLD
		/*
		** if it is disabled then make sure that
		** the show-only button is also off
		*/
		if(!clist->enabled){
		    sprintf(cname,"/controlmenu/clshowonly%d",cid);
		    if((citem = LookupMenuItem(cname)) != NULL){
			citem->state = 0;
			/*
			** redraw the updated item
			*/
			DrawItem(citem);
		    }
		}
#endif
	    }
	}
	/*
	** is this cluster selected for showonly
	*/
	sprintf(cname,"/controlmenu/clshowonly%d",cid);
	if((citem = LookupMenuItem(cname)) != NULL){
	    if((clist = GetClusterInfo(cval)) != NULL){
		if(clist->showonly){
		    citem->state = 1;
		} else {
		    citem->state = 0;
		}
		/*
		** redraw the updated item
		*/
		DrawItem(citem);
	    }
	}
    }
}

AssignProjectionButtons()
{
int 	i;
ProjectionInfo	*pinfo;
char		pname[100];
char		*tmpname;
MenuItem	*pitem;
int		projectionid;


/*
** Main loop over projection grid
*/
 
 for(i=0;i<PROJECTION_BLOCK_SIZE;i++){
   /*
   ** lookup the button for the projection  
   */
   sprintf(pname,"/controlmenu/projection/name/p%d",i);
   if((pitem = LookupMenuItem(pname)) != NULL){
     /*
     ** assign the item string to the projection name
     */
     if((pinfo = GetProjectionInfo(G,i+menu_block)) != NULL){
       pitem->state = 0;

       /* reset the item's function to p0 */
       /*pitem->func = MenuSetProjection0;*/

       /*
       ** assign the projection name to the button string
       */
       if(pinfo->name){
	 pitem->string = (char *)malloc(strlen(pinfo->name)+1);
	 strcpy(pitem->string,pinfo->name);
	 pitem->string2 = pitem->string;
	 pitem->value = NULL;
	 sprintf(pitem->data,"%d",pinfo->projectionid);
       } 
     } else {
       /*
       ** use the projection number as the item value
       */
       sprintf(pname,"%d",i+menu_block);
       tmpname = (char *)malloc(strlen(pname)+1);
       strcpy(tmpname,pname);
       pitem->value = tmpname;
       sprintf(pitem->data,"%d",i+menu_block);
       pitem->string = "\0";
       pitem->string2 = "\0";
     }
     /*
     ** redraw the updated item
     */
     pitem->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
     if( Atoi(pitem->data) == G->selectedprojection[0]){
       pitem->itembackground = MININTERFACECOLOR + SELECTION2;
     }

     pitem->enabled = pinfo ? 1 : 0;
     DrawItem(pitem);
	    
     sprintf(pname,"/controlmenu/projection/x/p%d",i);
     if((pitem = LookupMenuItem(pname)) != NULL){
       pitem->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
       if( Atoi(pitem->data) == G->selectedprojection[0]){
	 pitem->itembackground = MININTERFACECOLOR + SELECTION2;
       }
       if( Atoi(pitem->data) == G->currentprojection[0]){
	 pitem->itembackground = MININTERFACECOLOR +  XAXIS;
       }
       pitem->enabled = pinfo ? 1 : 0;
       DrawItem(pitem);
     }
	    
     sprintf(pname,"/controlmenu/projection/y/p%d",i);
     if((pitem = LookupMenuItem(pname)) != NULL){
       pitem->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
       if( Atoi(pitem->data) == G->selectedprojection[1]){
	 pitem->itembackground = MININTERFACECOLOR +  SELECTION2;
       }
       if( Atoi(pitem->data) == G->currentprojection[1]){
	 pitem->itembackground = MININTERFACECOLOR + YAXIS;
       }
       pitem->enabled = pinfo ? 1 : 0;
       DrawItem(pitem);
     }	    

     sprintf(pname,"/controlmenu/projection/z/p%d",i);
     if((pitem = LookupMenuItem(pname)) != NULL){
       pitem->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
       if( Atoi(pitem->data) == G->selectedprojection[2]){
	 pitem->itembackground = MININTERFACECOLOR +  SELECTION2;
       }
       if( Atoi(pitem->data) == G->currentprojection[2]){
	 pitem->itembackground = MININTERFACECOLOR + ZAXIS;
       }
       pitem->enabled = pinfo ? 1 : 0;
       DrawItem(pitem);
     }

     sprintf(pname,"/controlmenu/projection/rand/p%d",i);
     if((pitem = LookupMenuItem(pname)) != NULL){
       if (pinfo){
	 pitem->state = pinfo->randomize;
       } else {
	 pitem->state = 0;
       }
       pitem->enabled = pinfo ? 1 : 0;
       DrawItem(pitem);
     }
   }
 }

}


MenuFlashCluster(item)
MenuItem	*item;
{
Graph	*graph;

    rotatecolor = item->state;
    if(rotatecolor == 1){
	/*
	** save the current value of the pixel
	*/
	SavePixel();
    } else {
	/*
	** restore the previous value of the pixel
	*/
	RestorePixel();
	RefreshGraph(G);
    }
}

MenuAutoscale(item)
MenuItem	*item;
{
Graph	*graph;

    graph = GetGraph("/graph");
    AutoScale(graph,graph->xaxis.autoscale,graph->yaxis.autoscale);
    ScaleAndRefreshGraph(graph);
}

MenuClusterBlockUp(item)
MenuItem	*item;
{

    /*
    ** increment all of the cluster selection buttons
    */
    if((menu_cluster_block += CLUSTER_BLOCK_SIZE) >= 
	MAXCLUSTER - CLUSTER_BLOCK_SIZE-1){
	menu_cluster_block = MAXCLUSTER - CLUSTER_BLOCK_SIZE-1;
    }
    AssignClusterButtons();
}

MenuClusterBlockDown(item)
MenuItem	*item;
{

    /*
    ** decrement all of the cluster selection buttons
    */
    if((menu_cluster_block -= CLUSTER_BLOCK_SIZE) < 0){
	menu_cluster_block = 0;
    }
    AssignClusterButtons();
}

void UpdateProjectionMenu(void)
{
    AssignProjectionButtons();
}

MenuProjectionBlockUp(item)
MenuItem	*item;
{

    /*
    ** increment all of the projection selection buttons
    */
  if((menu_block += PROJECTION_BLOCK_SIZE) >= 
     MAXPROJECTION - PROJECTION_BLOCK_SIZE-1){
      menu_block = MAXPROJECTION - PROJECTION_BLOCK_SIZE - 1;
    }
  AssignProjectionButtons();
}

MenuProjectionBlockDown(item)
MenuItem	*item;
{

    /*
    ** decrement all of the projection selection buttons
    */
    if((menu_block -= PROJECTION_BLOCK_SIZE) < 0){
	menu_block = 0;
    }

    AssignProjectionButtons();
}

UpdateProjections()
{
ProjectionInfo	*pinfo;
int	svindex;
char	*pname0;
char	*pname1;
char	name[100];
Label	*label;
Graph	*graph;
MenuItem	*item;

    pname0 = "";
    pname1 = "";

    graph = GetGraph("/graph");

    /*
    ** set the scale values to those previously stored in
    ** the projection info if available
    */
    if(pinfo = GetProjectionInfo(G,G->currentprojection[0])){
	if(pinfo->wmin != pinfo->wmax){
	    G->wxmin = pinfo->wmin;
	    G->wxmax = pinfo->wmax;
	}
	pname0 = pinfo->name;
    }
    if(pinfo = GetProjectionInfo(G,G->currentprojection[1])){
	if(pinfo->wmin != pinfo->wmax){
	    G->wymin = pinfo->wmin;
	    G->wymax = pinfo->wmax;
	}
	pname1 = pinfo->name;
    }
    /*
    ** try to change the projection labels in the graph
    */
    sprintf(name,"%s  %s",pname0,pname1);
    if((label = GetNamedLabel(graph,"projections")) != NULL){
	strcpy(label->u.string.line,name);
    } else {
	/*
	** add the label from scratch
	*/
	label = AddLabelString(graph, name,100,20,
	    0.0,0.0,SCREEN_LBL,PERMANENT_LBL,graph->fontname);
	label->name = "projections";
    }
    /*
    ** update cluster labels 
    */
    UpdateClusterLabels();
}


MenuSetProjection0(item, event)
MenuItem	*item;
XButtonEvent	*event;
{
ProjectionInfo	*pinfo;
MenuItem	*pitem;
char		s[100];

    if(!item){
	return;
    }

    if((pinfo = GetProjectionInfo(G,Atoi(item->data))) == NULL){
      /* projection doesn't exist */
      item->state = 0;
      item->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
      DrawItem(item);
      return;
    }      

    if ( (Atoi(item->data)==F->graph->selectedprojection[0]) ) {
      /* do nothing */
      F->graph->selectedprojection[0] = -1;
      MenuClassSetFunc(NULL,item->menu, "pname", MenuSetProjection0);
    } else {
      /* undo color of previously selected projection */
/*       sprintf(s,"/controlmenu/projection/x/p%d",F->graph->selectedprojection[0]); */
/*       pitem = LookupMenuItem(s); */
/*       pitem->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND; */
/*       DrawItem(pitem); */
      /* set new selected projection */
      F->graph->selectedprojection[0] = Atoi(item->data);
      /* set color of selection */
/*       if  ( (Atoi(item->data)!=F->graph->currentprojection[0]) ) { */
/* 	item->itembackground = MININTERFACECOLOR + MENUFRAMEBACKGROUND; */
/*       } */
      MenuClassSetFunc(NULL,item->menu, "pname", MenuSetProjection1);
    }

    AssignProjectionButtons();
}

MenuSetProjection1(item, event)
MenuItem	*item;
XButtonEvent	*event;
{
ProjectionInfo	*pinfo;
MenuItem	*pitem;
char		s[100];

 if(!item){
   return;
 }


 if((pinfo = GetProjectionInfo(G,Atoi(item->data))) == NULL){
   /* projection doesn't exist */
   item->state = 0;
   item->itembackground = MININTERFACECOLOR + MENUITEMBACKGROUND;
   DrawItem(item);
   return;
 }      

 if (F->graph->selectedprojection[0] == Atoi(item->data)) {
   if(item->class &&
      strcmp(item->class, "pname") == 0){
     /* undo selection */
     F->graph->selectedprojection[0] = -1;
     MenuClassSetFunc(NULL,item->menu, "pname", MenuSetProjection0);
     AssignProjectionButtons();
     return;
   }
 }

 /*
 ** before switching projections save the data ranges
 ** for the current projections in the projection info
 */
 if(pinfo = GetProjectionInfo(G,G->currentprojection[0])){
   pinfo->wmin = G->wxmin;
   pinfo->wmax = G->wxmax;
 }
 if(pinfo = GetProjectionInfo(G,G->currentprojection[1])){
   pinfo->wmin = G->wymin;
   pinfo->wmax = G->wymax;
 }

 if (F->graph->selectedprojection[0] != F->graph->currentprojection[0] && F->graph->selectedprojection[0]!=-1) {
   /* i.e. the x projection changed */
   /* make it definitive */
   F->graph->currentprojection[0] = F->graph->selectedprojection[0];
 }


 /* now select y projection */
 if ( (Atoi(item->data) != F->graph->currentprojection[1]) ) {
   /* y projection changed */
   F->graph->selectedprojection[1] = F->graph->currentprojection[1] = Atoi(item->data);
 }


 /* reset callback function of projection name buttons */
 MenuClassSetFunc(NULL,item->menu, "pname", MenuSetProjection0);
 F->graph->selectedprojection[0] = -1;

 /*
 ** set buttons and update projections 
 */
 UpdateProjections();
 AssignProjectionButtons();
      
 /*
 ** and refresh the display
 */
 ScaleAndRefreshGraph(G);
}

MenuSetProjection2(item)
MenuItem	*item;
{

  ProjectionInfo *pinfo;

    if(!item){
	return;
    }

    if((pinfo = GetProjectionInfo(G,Atoi(item->data))) == NULL){
      /* projection doesn't exist */
      item->state = 0;
      return;
    }      

    /* we don't need to rescale or redraw, because Z-projection is
    ** only used in explicit redraws */

/*     if(item->state == 1){ */
/*       G->currentprojection[2] = pinfo->projectionid; */
/*     } */ 

    F->graph->selectedprojection[2] = F->graph->currentprojection[2] = pinfo->projectionid;
    AssignProjectionButtons();
    /*
    ** and refresh the display
    */
    ScaleAndRefreshGraph(G);

}


RandomizeProjection(ProjectionInfo *pinfo, int state)
{
  if (!pinfo)
    return;

  pinfo->randomize = state;

  /* only redraw if we have randomized a current projection */
  if (pinfo->projectionid == G->currentprojection[0] ||
      pinfo->projectionid == G->currentprojection[1])
    ScaleAndRefreshGraph(G);
}

  

MenuRandomizeProjection(item)
MenuItem	*item;
{

  ProjectionInfo *pinfo;
  int	*state;
  
    if(!item){
	return;
    }

    /* lookup the projectioninfo */
    if((pinfo = GetProjectionInfo(G,Atoi(item->data))) == NULL) {
      /* if it doesn't exist, then don't let user select the projection */
      item->state = 0;
      return;
    }
    
    /* set the randomization of the projection */
    RandomizeProjection(pinfo,item->state);
    
}

LookupCluster(cid)
int	cid;
{
ClusterList	*clist;
Graph		*graph;

    graph = GetGraph("/graph");
    for(clist=graph->clusterlist;clist;clist=clist->next){
	if(clist->clusterid != cid) continue;
	/*
	** found an active cluster 
	*/
	return(1);
    }
    /*
    ** no match found
    */
    return(0);
}

UpdateClusterMenu()
{
ClusterList	*clist;
Graph		*graph;
MenuItem	*tmpitem;
char		itemname[100];
int		i;
int		done;
int		cid;

    /*
    ** scan the cluster list and see which clusters are
    ** being used
    */
    graph = GetGraph("/graph");
    i = 0;
    done = 0;
    /*
    ** go through all of the hidden buttons
    */
    while(!done){
	i++;
	/*
	** look for the cluster buttons
	*/
	sprintf(itemname,"/controlmenu/cl%d",i);
	if((tmpitem = LookupMenuItem(itemname)) != NULL){
	    cid = Atoi(tmpitem->value);
	} else {
	    /*
	    ** stop when reaching a nonexistant button number
	    */
	    done = 1;
	    break;
	}
	/*
	** look for the cluster visibility buttons
	*/
	sprintf(itemname,"/controlmenu/clmask%d",i);
	if((tmpitem = LookupMenuItem(itemname)) != NULL){
	    /*
	    ** found the button. Turn it off so that it 
	    ** obscures the buttons underneath
	    */
	    tmpitem->mapped = 1;
	}
	for(clist=graph->clusterlist;clist;clist=clist->next){
	    if(clist->clusterid != cid) continue;
	    /*
	    ** found an active cluster so unmap the obscuring button
	    */
	    tmpitem->mapped = 0;
	}
	/*
	** refresh the menu display
	*/
	UpdateItemMapping(tmpitem);
    }
    RefreshClusterScoreButtons();
}
