#include "xclust_ext.h"

#define CLUSTERBLOCKSIZE	50000

/*
******************************************************************
**                                                              **
**            Cluster File Input/Output Routines                **
**                                                              **
******************************************************************
*/
double GetDoubleFromRawData(data,pinfo)
char	*data;
ProjectionInfo	*pinfo;
{
double	dval;
    if(data == NULL){
	fprintf(stderr,"ERROR: NULL data vector\b");
	return(0);
    }
    switch(pinfo->type){
	case INT:
	    dval = *((int *)(data + pinfo->offset));
	    break;
	case FLOAT:
	    dval = *((float *)(data + pinfo->offset));
	    break;
	case SHORT:
	    dval = *((short *)(data + pinfo->offset));
	    break;
	case ULONG:
	    dval = *((unsigned long *)(data + pinfo->offset));
	    break;
	case DOUBLE:
	    dval = *((double *)(data + pinfo->offset));
	    break;
	case CHAR:
	    dval = *((char *)(data + pinfo->offset));
	    break;
	default:
	    fprintf(stderr,"ERROR: unrecognized data type %d\n",pinfo->type);
	    return(0);
	}
    return(dval);
	
}

AssignRawDataFromDouble(data,pinfo,dval)
char	*data;
ProjectionInfo	*pinfo;
double	dval;
{
    if(data == NULL){
	fprintf(stderr,"ERROR: NULL data vector\b");
	return;
    }
    switch(pinfo->type){
	case INT:
	    *((int *)(data + pinfo->offset)) = dval;
	    break;
	case FLOAT:
	    *((float *)(data + pinfo->offset)) = dval;
	    break;
	case SHORT:
	    *((short *)(data + pinfo->offset)) = dval;
	    break;
	case ULONG:
	    *((unsigned long *)(data + pinfo->offset)) = dval;
	    break;
	case DOUBLE:
	    *((double *)(data + pinfo->offset)) = dval;
	    break;
	case CHAR:
	    *((char *)(data + pinfo->offset)) = dval;
	    break;
	default:
	    fprintf(stderr,"ERROR: unrecognized data type %d\n",pinfo->type);
	    return(0);
	}
    return;
	
}


WriteClusterBounds(graph,cb,fp)
Graph		*graph;
ClusterBounds	*cb;
FILE		*fp;
{
int	i;
ProjectionInfo	*pinfo;

    fprintf(fp,"\n");
    fprintf(fp,"%d\n",cb->clusterid);
    fprintf(fp,"%d\t%d\n",cb->projection[0],cb->projection[1]);
    if(pinfo = GetProjectionInfo(graph,cb->projection[0])){
	fprintf(fp,"%s\n",pinfo->name);
    } else {
	fprintf(fp,"P%d\n",cb->projection[0]);
    }
    if(pinfo = GetProjectionInfo(graph,cb->projection[1])){
	fprintf(fp,"%s\n",pinfo->name);
    } else {
	fprintf(fp,"P%d\n",cb->projection[1]);
    }
    fprintf(fp,"%d\n",cb->ncoords);
    for(i=0;i<cb->ncoords;i++){
	fprintf(fp,"%g\t%g\n", cb->fcoord[i].x,cb->fcoord[i].y);
    }
}

WriteAllClusterBounds(graph,file)
Graph	*graph; 
char	*file;
{
ClusterBounds	*cb;
FILE		*fp;
Plot		*plot;
char		*tstring;
char		comstr[100];

    /*
    ** backup the previous bounds file
    */
    sprintf(comstr,"/bin/cp %s %s~",file,file);
    system(comstr);
    /*
    ** open the file
    */
    fprintf(stderr,"Writing bounds file '%s' (backup saved to %s~)\n",
	file,file);
    if((fp = fopen(file,"w")) == NULL){
	fprintf(stderr,"ERROR: unable to write cluster bounds file '%s'\n",file);
	return;
    }
    BeginStandardHeader(fp,standardargc,standardargv,VERSION);
    fprintf(fp,"%% File type:\tAscii\n");
    fprintf(fp,"%% Data files:\n");
    for(plot=graph->plot;plot;plot=plot->next){
	fprintf(fp,"%%\t%s : %d points\n",plot->filename,plot->npoints);
    }
    fprintf(fp,"%%\n");
    fprintf(fp,"%% Cluster Bounds definition file format:\n");
    fprintf(fp,"%% 	clusterid\n");
    fprintf(fp,"%% 	projection numbers\n");
    fprintf(fp,"%% 	projection names\n");
    fprintf(fp,"%% 	number of bounds coordinates\n");
    fprintf(fp,"%% 	wx1 wy1\n");
    fprintf(fp,"%% 	wx2 wy2\n");
    fprintf(fp,"%% 	... ...\n");
    fprintf(fp,"%% 	wxn wyn\n");
    fprintf(fp,"%%\n");

    /*
    ** write out the time range if defined
    */
/*     tstring = GetItemValue("/epochmenu/tstartspike"); */
/*     if(tstring){ */
/* 	fprintf(fp,"%% Start time:\t%s\n",tstring); */
/*     } */
/*     tstring = GetItemValue("/epochmenu/tendspike"); */
/*     if(tstring){ */
/* 	fprintf(fp,"%% End time:\t%s\n",tstring); */
/*     } */

    /*
    ** write out cluster scores, if any 
    */
    WriteClusterScores(fp,graph);

    /*
    ** end of header
    */
    EndStandardHeader(fp);

    /*
    ** go through each cluster box and write it out
    */
    for(cb=graph->clusterbounds;cb;cb=cb->next){
	WriteClusterBounds(graph,cb,fp);
    }
    fclose(fp);
    /*
    ** clear the cb modified flag
    */
    graph->cbmodified = 0;
    fprintf(stderr,"done\n");
}

WriteClusterScores(fp,graph)
     FILE	*fp;
     Graph	*graph;
{
  ClusterList *cl;
  char s[10];
  char sline[1000] = "";


  /* collect all non-zero scores */
  for(cl=graph->clusterlist; cl; cl=cl->next){
    if (cl->score){
      sprintf(s,"\t%d,%d",
	      cl->clusterid,
	      cl->score);
      strcat(sline,s);
    }
  }

  /* only print out scores if we have some to print! */
  if (strlen(sline) != 0){
    fprintf(fp, "%% Cluster Scores:%s\n", sline);
  }
}

AssignClusterScores(char *scores)
{
  char *ptr; 
  int clusterid, score;
  ClusterList *cl;

  ptr = scores;

  while(ptr){
    
    /* eat whitespace */
    if (ptr[0] == '\t' ||  ptr[0] == ' '){
      ptr++;
      continue;
    }

    /* parse cluster/score pair */
    sscanf(ptr,"%d,%d",&clusterid,&score);
    
    /* set score */
    if ((cl = GetClusterInfo(clusterid))){
      (cl->score) = score;
    } else {
      fprintf(stderr, "ERROR: Cluster '%d' not found, no score assigned\n", clusterid);
      return;
    }
    
    /* advance to next token (return NULL at end)*/
    ptr = strpbrk(ptr,"\t ");
  }
}
  
/*
** write out all clusters
*/
WriteClusters(graph,prefix)
Graph	*graph;
char	*prefix;
{
ClusterList	*clist;
int		clusterid;
char		fname[100];
FILE		*fp;
Plot		*plot;
int		count;
char		*tstring;
int		i;
ProjectionInfo	*pinfo;
int		totaloverlap;

    /*
    ** evaluate cluster overlap
    */
    totaloverlap = EvaluateOverlap(graph);
    /*
    ** go through all of the defined clusters
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	clusterid = clist->clusterid;
	sprintf(fname,"%s%d",prefix,clusterid);
	fprintf(stderr,"Writing cluster file '%s' :",fname);
	if((fp = fopen(fname,"w")) == NULL){
	    fprintf(stderr,"ERROR: unable to create file '%s'\n",fname);
	    continue;
	}
	BeginStandardHeader(fp,standardargc,standardargv,VERSION);
	for(plot=graph->plot;plot;plot=plot->next){
	    fprintf(fp,"%% Source file:\t%s\n",plot->filename);
	}
	fprintf(fp,"%% File type:\tAscii\n");
	if(graph->plot){
	    fprintf(fp,"%% Fields: ");
	    for(i=0;i< graph->plot->ndatafields;i++){
		pinfo = GetProjectionInfo(graph,i);
		fprintf(fp,"\t%s,%d,%d,%d",pinfo->name,pinfo->type,pinfo->size,1);
	    }
	    fprintf(fp,"\n");
	}
	fprintf(fp,"%% Cluster:\t%d\n",clusterid);
	/*
	** write out the time range if defined
	*/
/* 	tstring = GetItemValue("/epochmenu/tstartspike"); */
/* 	if(tstring){ */
/* 	    fprintf(fp,"%% Start time:\t%s\n",tstring); */
/* 	} */
/* 	tstring = GetItemValue("/epochmenu/tendspike"); */
/* 	if(tstring){ */
/* 	    fprintf(fp,"%% End time:\t%s\n",tstring); */
/* 	} */

	/* write out cluster score, if any */
	if (clist->score)
	  fprintf(fp,"%% Cluster Score:\t%d\n",clist->score);
		  
	EndStandardHeader(fp);
	count = WriteClusterPoints(graph,clusterid,fp,0);
	fclose(fp);
	fprintf(stderr,"%d points\n",count);
    }
    fprintf(stderr,"%d overlapping cluster points\n",totaloverlap);
    fprintf(stderr,"done\n");
}

/* /\* */
/* ** write out all clusters for all points in a partially loaded file */
/* *\/ */
/* #ifdef OLD */
/* LoadAndWriteClusters(graph,prefix) */
/* Graph	*graph; */
/* char	*prefix; */
/* { */
/* ClusterList	*clist; */
/* int		clusterid; */
/* char		fname[100]; */
/* FILE		*fp; */
/* Plot		*plot; */
/* int		count; */
/* int		npoints; */
/* int		startline; */
/* int		endline; */
/* char		*tstring; */

/*     /\* */
/*     ** go through all of the defined clusters */
/*     *\/ */
/*     for(clist=graph->clusterlist;clist;clist=clist->next){ */
/* 	if((plot = SelectedPlot(graph)) == NULL){ */
/* 	    fprintf(stderr,"ERROR: no plot is selected\n"); */
/* 	    break; */
/* 	} */
/* 	clusterid = clist->clusterid; */
/* 	sprintf(fname,"%s%d",prefix,clusterid); */
/* 	fprintf(stderr,"Preparing cluster file '%s'\n",fname); */
/* 	if((fp = fopen(fname,"w")) == NULL){ */
/* 	    fprintf(stderr,"ERROR: unable to create file '%s'\n",fname); */
/* 	    continue; */
/* 	} */
/* 	BeginStandardHeader(fp,standardargc,standardargv,VERSION); */
/* 	fprintf(fp,"%% Source file: %s\n",plot->filename); */
/* 	fprintf(fp,"%% Cluster: %d\n",clusterid); */
/* 	/\* */
/* 	** write out the time range if defined */
/* 	*\/ */
/* /\* 	tstring = GetItemValue("/epochmenu/tstartspike"); *\/ */
/* /\* 	if(tstring){ *\/ */
/* /\* 	    fprintf(fp,"%% Start time:\t%s\n",tstring); *\/ */
/* /\* 	} *\/ */
/* /\* 	tstring = GetItemValue("/epochmenu/tendspike"); *\/ */
/* /\* 	if(tstring){ *\/ */
/* /\* 	    fprintf(fp,"%% End time:\t%s\n",tstring); *\/ */
/* /\* 	} *\/ */
/* 	EndStandardHeader(fp); */
/* 	/\* */
/* 	** now load and write out cluster points for the selected */
/* 	** plot */
/* 	*\/ */
/* 	count = 0; */
/* 	npoints = 0; */
/* 	fprintf(stderr,"Processing plot datafile '%s'\n",plot->filename); */
/* 	startline = 0; */
/* 	endline = CLUSTERBLOCKSIZE - 1; */


/* 	do { */
/* 	    /\* */
/* 	    ** load in data points from the requested range */
/* 	    *\/ */
/* 	    if((plot = PartialLoadPlot(graph,plot,startline,endline,-1,-1)) */
/* 	    == NULL){ */
/* 		fprintf(stderr,"ERROR: unable to load points\n"); */
/* 		break; */
/* 	    } */
/* 	    /\* */
/* 	    ** write out the cluster points to the cluster file */
/* 	    *\/ */
/* 	    count += WriteClusterPoints(graph,clusterid,fp,0); */
/* 	    /\* */
/* 	    ** keep track of the total number of points loaded */
/* 	    *\/ */
/* 	    npoints += plot->npoints; */
/* 	    /\* */
/* 	    ** move to the next block */
/* 	    *\/ */
/* 	    startline += CLUSTERBLOCKSIZE; */
/* 	    endline += CLUSTERBLOCKSIZE; */
/* 	    /\* */
/* 	    **  keep processing until a block smaller than the requested */
/* 	    ** block is loaded indicating the end of the file */
/* 	    *\/ */
/* 	}while(plot->npoints >= CLUSTERBLOCKSIZE); */

/* 	fclose(fp); */
/* 	fprintf(stderr,"Processed a total of %d points\n",npoints); */
/* 	fprintf(stderr,"%d points written to cluster file '%s'\n",count,fname); */
/*     } */
/*     fprintf(stderr,"done\n"); */
/* } */
/* #else */
/* LoadAndWriteClusters(graph,prefix) */
/* Graph	*graph; */
/* char	*prefix; */
/* { */
/* ClusterList	*clist; */
/* int		clusterid; */
/* char		fname[100]; */
/* FILE		*fp; */
/* Plot		*plot; */
/* int		count; */
/* int		npoints; */
/* int		startline; */
/* int		endline; */
/* char		*tstring; */
/* int		endspike; */
/* char		*endstr; */
/* int		endcheck; */
/* int		i; */
/* ProjectionInfo	*pinfo; */
/* int		totaloverlap; */

/*     /\* */
/*     ** go through all of the defined clusters */
/*     *\/ */
/*     if((plot = SelectedPlot(graph)) == NULL){ */
/* 	fprintf(stderr,"ERROR: no plot is selected\n"); */
/* 	return; */
/*     } */
/*     /\* */
/*     ** now load and write out cluster points for the selected */
/*     ** plot */
/*     *\/ */
/*     count = 0; */
/*     npoints = 0; */
/*     totaloverlap = 0; */
/*     fprintf(stderr,"Processing plot datafile '%s'\n",plot->filename); */
/*     startline = 0; */
/*     endline = CLUSTERBLOCKSIZE - 1; */
/* #ifndef OLD */
/*     startline = Atoi(GetItemValue("/epochmenu/startspike")); */
/*     endstr = GetItemValue("/epochmenu/endspike"); */
/*     if(strncmp(endstr,"end",3) == 0){ */
/* 	endspike = startline + CLUSTERBLOCKSIZE - 1; */
/* 	endcheck = 0; */
/*     } else { */
/* 	endspike = Atoi(endstr); */
/* 	endcheck = 1; */
/*     } */
/*     if((endspike - startline) > CLUSTERBLOCKSIZE - 1){ */
/* 	endline = startline + CLUSTERBLOCKSIZE - 1; */
/*     } else { */
/* 	endline = endspike; */
/*     } */
/* #endif */
/*     /\* */
/*     ** prepare each cluster id file */
/*     *\/ */
/*     for(clist=graph->clusterlist;clist;clist=clist->next){ */
/* 	clusterid = clist->clusterid; */
/* 	sprintf(fname,"%s%d",prefix,clusterid); */
/* 	fprintf(stderr,"Preparing cluster file '%s'\n",fname); */
/* 	if((fp = fopen(fname,"w")) == NULL){ */
/* 	    fprintf(stderr,"ERROR: unable to create file '%s'\n",fname); */
/* 	    continue; */
/* 	} */
/* 	BeginStandardHeader(fp,standardargc,standardargv,VERSION); */
/* 	fprintf(fp,"%% Source file: %s\n",plot->filename); */
/* 	if(graph->plot){ */
/* 	    fprintf(fp,"%% Fields: "); */
/* 	    for(i=0;i< graph->plot->ndatafields;i++){ */
/* 		if((pinfo = GetProjectionInfo(graph,i)) == NULL){ */
/* 		    fprintf(fp,"\tP%d,%d,%d,%d",i,FLOAT,sizeof(float),1); */
/* 		} else { */
/* 		    fprintf(fp,"\t%s,%d,%d,%d",pinfo->name,pinfo->type,pinfo->size,1); */
/* 		} */
/* 	    } */
/* 	    fprintf(fp,"\n"); */
/* 	} */
/* 	fprintf(fp,"%% Cluster: %d\n",clusterid); */
/* 	/\* */
/* 	** write out the time range if defined */
/* 	*\/ */
/* /\* 	tstring = GetItemValue("/epochmenu/tstartspike"); *\/ */
/* /\* 	if(tstring){ *\/ */
/* /\* 	    fprintf(fp,"%% Start time:\t%s\n",tstring); *\/ */
/* /\* 	} *\/ */
/* /\* 	tstring = GetItemValue("/epochmenu/tendspike"); *\/ */
/* /\* 	if(tstring){ *\/ */
/* /\* 	    fprintf(fp,"%% End time:\t%s\n",tstring); *\/ */
/* /\* 	} *\/ */
/* 	EndStandardHeader(fp); */

/* 	fclose(fp); */
/*     } */
/*     do { */
/* 	fprintf(stderr,"loading points %d to %d\n",startline,endline); */
/* 	/\* */
/* 	** load in data points from the requested range */
/* 	*\/ */
/* 	if((plot = PartialLoadPlot(graph,plot,startline,endline,-1,-1)) */
/* 	== NULL){ */
/* 	    fprintf(stderr,"ERROR: unable to load points\n"); */
/* 	    break; */
/* 	} */
/* 	/\* */
/* 	** evaluate the points for overlapping clusters */
/* 	*\/ */
/* 	totaloverlap += EvaluateOverlap(graph); */
/* 	/\* */
/* 	** write out the clusters */
/* 	*\/ */
/* 	for(clist=graph->clusterlist;clist;clist=clist->next){ */
/* 	    clusterid = clist->clusterid; */
/* 	    sprintf(fname,"%s%d",prefix,clusterid); */
/* 	    fprintf(stderr,"Appending to cluster file '%s'\n",fname); */
/* 	    if((fp = fopen(fname,"a")) == NULL){ */
/* 		fprintf(stderr,"ERROR: unable to open file '%s'\n",fname); */
/* 		continue; */
/* 	    } */

/* 	    /\* */
/* 	    ** write out the cluster points to the cluster file */
/* 	    *\/ */
/* 	    count += WriteClusterPoints(graph,clusterid,fp,0); */
/* 	    fclose(fp); */
/* 	} */

/* 	/\* */
/* 	** keep track of the total number of points loaded */
/* 	*\/ */
/* 	npoints += plot->npoints; */
/* 	/\* */
/* 	** move to the next block */
/* 	*\/ */
/* 	startline += CLUSTERBLOCKSIZE; */
/* 	endline += CLUSTERBLOCKSIZE; */
/* #ifndef OLD */
/* 	if(endcheck && (endline > endspike)){ */
/* 	    endline = endspike; */
/* 	} */
/* #endif */
/* 	/\* */
/* 	**  keep processing until a block smaller than the requested */
/* 	** block is loaded indicating the end of the file */
/* 	*\/ */
/*     }while(plot->npoints >= CLUSTERBLOCKSIZE); */

/*     fprintf(stderr,"Processed a total of %d points\n",npoints); */
/*     fprintf(stderr,"%d points written to cluster files\n",count); */
/*     fprintf(stderr,"%d overlapping cluster points\n",totaloverlap); */
/*     fprintf(stderr,"done\n"); */
/* } */
/* #endif */

/*
** write out all cluster indices
*/
WriteClusterIndices(graph,file)
Graph	*graph;
char	*file;
{
ClusterList	*clist;
int		clusterid;
char		fname[100];
FILE		*fp;
Plot		*plot;
int		count;

    /*
    ** go through all of the points
    */
    sprintf(fname,"%s",file);
    fprintf(stderr,"Writing cluster transfer file '%s' :",fname);
    if((fp = fopen(fname,"w")) == NULL){
	fprintf(stderr,"ERROR: unable to create file '%s'\n",fname);
	return;
    }
    BeginStandardHeader(fp,standardargc,standardargv,VERSION);
    for(plot=graph->plot;plot;plot=plot->next){
	fprintf(fp,"%% Source file: %s\n",plot->filename);
    }
    EndStandardHeader(fp);
    count = WriteClusterIndexPoints(graph,fp);
    fclose(fp);
    fprintf(stderr,"%d points\n",count);
    fprintf(stderr,"done\n");
}

int WriteClusterIndexPoints(graph,fp)
Graph	*graph;
FILE	*fp;
{
ClusterBounds	*cb;
ClusterList	*clist;
int	p0,p1;
int	success;
Plot	*plot;
int	i,j;
int	count;
int	npoints;
int	clusterid;
ProjectionInfo	*pinfo;
int	idoffset;
double	p0val,p1val;
int	idtype;

    npoints = 0;
    /*
    ** get the data field for the spike id
    */
    if((pinfo=GetProjectionInfoByName(graph,"id")) == NULL){
	fprintf(stderr,"ERROR: Unable to find spike id field\n");
	return(0);
    }
    idoffset = pinfo->offset;
    /*
    ** confirm that this the id is an integer type
    if(pinfo->type != INT){
	fprintf(stderr,"ERROR: spike id field is not an integer\n");
	return(0);
    }
    */
    idtype = pinfo->type;
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->rawdata == NULL) continue;
	for(i=0;i<plot->npoints;i++){
	    count = 0;
	    if(idtype == INT){
		fprintf(fp,"%d",*((int *)(plot->rawdata[i].data + idoffset)));
	    } else 
	    if(idtype == FLOAT){
		fprintf(fp,"%d",(int)(*((float *)(plot->rawdata[i].data + idoffset))));
	    } else {
		fprintf(stderr,"ERROR: spike id field is not an int or float\n");
		return(0);
	    }

	    for(clist=graph->clusterlist;clist;clist=clist->next){
		clusterid = clist->clusterid;
		/*
		** initially assume that it does not belong to the cluster
		*/
		success = 0;
		/*
		** go through all of the cluster bounds
		*/
		for(cb=graph->clusterbounds;cb;cb=cb->next){
		    /*
		    ** check for matching clusterid
		    */
		    if(cb->clusterid != clusterid) continue;
		    /*
		    ** ok, found a cluster box matching the cluster id
		    */
		    success = 1;
		    p0 = cb->projection[0];
		    p1 = cb->projection[1];
		    if((pinfo=GetProjectionInfo(graph,p0)) == NULL){
			fprintf(stderr,"ERROR: Unable info on field %d\n",p0);
			return(0);
		    }
		    p0val = GetDoubleFromRawData(plot->rawdata[i].data,pinfo);
		    if((pinfo=GetProjectionInfo(graph,p1)) == NULL){
			fprintf(stderr,"ERROR: Unable info on field %d\n",p1);
			return(0);
		    }
		    p1val = GetDoubleFromRawData(plot->rawdata[i].data,pinfo);
		    /*
		    ** make sure the vector is large enough
		    */
		    if((p0 > plot->ndatafields) || 
			(p1 > plot->ndatafields)) continue;
		    if(cb->type == BOXBOUND){
			/*
			** now see whether the point actually falls within the 
			** bounding region
			*/
			if(p0val < cb->fcoord[0].x ||
			p0val > cb->fcoord[2].x ||
			p1val < cb->fcoord[0].y ||
			p1val > cb->fcoord[2].y){
			    /*
			    ** it's outside of the cluster box so no point in 
			    ** testing membership in this cluster any further
			    */
			    success = 0;
			    break;
			}
		    } else {
			/*
			** run the polygon enclosure test
			*/
			if(!IsInPolygon(p0val,p1val,
			cb->fcoord,cb->ncoords)){
			    success = 0;
			    break;
			}
		    }
		}
		if(success){
		    /*
		    ** fell within all defined cluster bounds
		    ** so write out the point to the cluster file
		    */
		    fprintf(fp,"\t%d",clusterid);
		    count++;
		}
	    }
	    if(count > 0){
		fprintf(fp,"\n");
	    } else {
		fprintf(fp,"\t0\n");
	    }
	    npoints++;
	}
    }
    return(npoints);
}


/*
** write out the timestamps in a single cluster
*/
int WriteClusterTimestamps(graph,clusterid,fp)
Graph	*graph;
int	clusterid;
FILE	*fp;
{
ClusterBounds	*cb;
ProjectionInfo	*pinfo;
ClusterList	*clist;
int	p0,p1;
int	success;
Plot	*plot;
int	i,j;
int	count;
ProjectionInfo	*timeprojection;
unsigned long	timestamp;
double	p0val,p1val;

    count = 0;
    /*
    ** look for a time parameter
    */
    if((pinfo = GetProjectionInfoByName(graph,"time")) == NULL){
	/*
	** if not found then return failure
	*/
	return(0);
    }
    timeprojection = pinfo;
    /*
    ** only write out points for the selected plot
    */
    if((plot = SelectedPlot(graph)) == NULL){
	fprintf(stderr,"ERROR: no plot selected\n");
    } else {
	if(plot->rawdata == NULL) return(count);
	for(i=0;i<plot->npoints;i++){
	    /*
	    ** initially assume that it does not belong to the cluster
	    */
	    success = 0;
	    /*
	    ** go through all of the cluster bounds
	    */
	    for(cb=graph->clusterbounds;cb;cb=cb->next){
		/*
		** check for matching clusterid
		*/
		if(cb->clusterid != clusterid) continue;
		/*
		** ok, found a cluster box matching the cluster id
		*/
		success = 1;
		p0 = cb->projection[0];
		p1 = cb->projection[1];
		/*
		** make sure the vector is large enough
		*/
		if((p0 > plot->ndatafields) || 
		    (p1 > plot->ndatafields)) continue;
		if((pinfo=GetProjectionInfo(graph,p0)) == NULL){
		    fprintf(stderr,"ERROR: Unable info on field %d\n",p0);
		    return(0);
		}
		p0val = GetDoubleFromRawData(plot->rawdata[i].data,pinfo);
		if((pinfo=GetProjectionInfo(graph,p1)) == NULL){
		    fprintf(stderr,"ERROR: Unable info on field %d\n",p1);
		    return(0);
		}
		p1val = GetDoubleFromRawData(plot->rawdata[i].data,pinfo);
		if(cb->type == BOXBOUND){
		    /*
		    ** now see whether the point actually falls within the 
		    ** bounding region
		    */
		    if(p0val < cb->fcoord[0].x ||
		    p0val > cb->fcoord[2].x ||
		    p1val < cb->fcoord[0].y ||
		    p1val > cb->fcoord[2].y){
			/*
			** it's outside of the cluster box so no point in 
			** testing for membership in this cluster any further
			*/
			success = 0;
			break;
		    }
		} else {
		    /*
		    ** run the polygon enclosure test
		    */
		    if(!IsInPolygon(p0val,p1val,
		    cb->fcoord,cb->ncoords)){
			success = 0;
			break;
		    }
		}
	    }
	    if(success){
		/*
		** fell within all defined cluster bounds
		** so write out the timestamp
		*/
		timestamp = TSCALE*GetDoubleFromRawData(plot->rawdata[i].data,timeprojection);
		fwrite(&timestamp,sizeof(unsigned long),1,fp);
		count++;
	    }
	}
    }
    return(count);
}

/*
** write out the points in a single cluster
*/
int WriteClusterPoints(graph,clusterid,fp,indexonly)
Graph	*graph;
int	clusterid;
FILE	*fp;
int	indexonly;
{
ClusterBounds	*cb;
ClusterList	*clist;
int	p0,p1;
int	success;
Plot	*plot;
int	i,j;
int	count;
ProjectionInfo	*pinfo;
double	p0val,p1val;
int	idoffset;
int	idtype;

    count = 0;
    /*
    ** only write out points for the selected plot
    */
    /*
    ** get the data field for the spike id
    */
    if((pinfo=GetProjectionInfoByName(graph,"id")) == NULL){
	fprintf(stderr,"ERROR: Unable to find spike id field\n");
	return(0);
    }
    /*
    ** confirm that this the id is an integer type
    if((pinfo->type != INT) && indexonly){
	fprintf(stderr,"ERROR: spike id field is not an integer\n");
	return(0);
    }
    */
    idtype = pinfo->type;
    idoffset = pinfo->offset;
    if((plot = SelectedPlot(graph)) == NULL){
	fprintf(stderr,"ERROR: no plot selected\n");
	return(0);
    } 
    if(plot->rawdata == NULL) return(0);
    for(i=0;i<plot->npoints;i++){
	/*
	** initially assume that it does not belong to the cluster
	*/
	success = 0;
	/*
	** go through all of the cluster bounds
	*/
	for(cb=graph->clusterbounds;cb;cb=cb->next){
	    /*
	    ** check for matching clusterid
	    */
	    if(cb->clusterid != clusterid) continue;
	    /*
	    ** ok, found a cluster box matching the cluster id
	    */
	    success = 1;
	    p0 = cb->projection[0];
	    p1 = cb->projection[1];
	    /*
	    ** make sure the vector is large enough
	    */
	    if((p0 > plot->ndatafields) || 
		(p1 > plot->ndatafields)) continue;
	    if((pinfo=GetProjectionInfo(graph,p0)) == NULL){
		fprintf(stderr,"ERROR: Unable info on field %d\n",p0);
		return(0);
	    }
	    p0val = GetDoubleFromRawData(plot->rawdata[i].data,pinfo);
	    if((pinfo=GetProjectionInfo(graph,p1)) == NULL){
		fprintf(stderr,"ERROR: Unable info on field %d\n",p1);
		return(0);
	    }
	    p1val = GetDoubleFromRawData(plot->rawdata[i].data,pinfo);
	    if(cb->type == BOXBOUND){
		/*
		** now see whether the point actually falls within the 
		** bounding region
		*/
		if(p0val < cb->fcoord[0].x ||
		p0val > cb->fcoord[2].x ||
		p1val < cb->fcoord[0].y ||
		p1val > cb->fcoord[2].y){
		    /*
		    ** it's outside of the cluster box so no point in 
		    ** testing for membership in this cluster any further
		    */
		    success = 0;
		    break;
		}
	    } else {
		/*
		** run the polygon enclosure test
		*/
		if(!IsInPolygon(p0val, p1val,
		cb->fcoord,cb->ncoords)){
		    success = 0;
		    break;
		}
	    }
	}
	if(success){
	    /*
	    ** fell within all defined cluster bounds
	    ** so write out the point to the cluster file
	    */
	    if(indexonly){
	      switch(idtype){
	      case INT:
		fprintf(fp,"%d\n",*((int *)(plot->rawdata[i].data+idoffset)));
		break;
	      case FLOAT:
		fprintf(fp,"%d\n",(int)(*((float *)(plot->rawdata[i].data+idoffset))));
		break;
	      case SHORT:
		fprintf(fp,"%d\n",(int)(*((short *)(plot->rawdata[i].data+idoffset))));
		break;
	      case ULONG:
		fprintf(fp,"%d\n",(int)(*((unsigned long *)(plot->rawdata[i].data+idoffset))));
		break;
	      case DOUBLE:
		fprintf(fp,"%d\n",(int)(*((double *)(plot->rawdata[i].data+idoffset))));
		break;
	      default:
		fprintf(stderr,"ERROR: unrecognized data type %d\n",idtype);
		return(0);		
	      }
	    } else  {
		for(j=0;j<plot->ndatafields;j++){
		    if((pinfo=GetProjectionInfo(graph,j)) == NULL){
			fprintf(stderr,"ERROR: Unable info on field %d\n",j);
			return(0);
		    }
		    switch(pinfo->type){
		    case INT:
			fprintf(fp,"%d\t",
			*((int *)(plot->rawdata[i].data + pinfo->offset)));
			break;
		    case FLOAT:
			fprintf(fp,"%.16g\t",
			*((float *)(plot->rawdata[i].data + pinfo->offset)));
			break;
		    case SHORT:
			fprintf(fp,"%d\t",
			*((short *)(plot->rawdata[i].data + pinfo->offset)));
			break;
		    case ULONG:
			fprintf(fp,"%u\t",
			*((unsigned long *)(plot->rawdata[i].data + pinfo->offset)));
			break;
		    case DOUBLE:
			fprintf(fp,"%.16g\t",
			*((double *)(plot->rawdata[i].data + pinfo->offset)));
			break;
		    default:
			fprintf(stderr,"ERROR: unrecognized data type %d\n",pinfo->type);
			return(0);
		    }
		}
		fprintf(fp,"\n");
	    }
	    count++;
	}
    }
    return(count);
}

ReadClusterBounds(graph,file)
Graph	*graph; 
char	*file;
{
char	line[200];
char	pname0[200];
char	pname1[200];
int	clusterid;
int	p0,p1;
float	wx1,wy1,wx2,wy2;
float	f1,f2;
FILE	*fp;
int	sp0,sp1;
int	level;
int	ncoords;
int	count;
ProjectionInfo	*pinfo;
char	**header;
int	headersize;
    /*
    ** open the file
    */
    fprintf(stderr,"Reading bounds file '%s'\n",file);
    if((fp = fopen(file,"r")) == NULL){
	fprintf(stderr,"ERROR: unable to read cluster bounds file '%s'\n");
	return;
    }
    /*
    ** backup the current bounds
    */
    BackupClusterBounds(graph);
    level = 0;
    /*
    ** save original projections
    */
    sp0 = graph->currentprojection[0];
    sp1 = graph->currentprojection[1];
    /*
    ** clear the current bounds
    */
    DeleteAllClusterBounds(graph);
    while(DeleteClusterFromList(graph,-1));
    SetClusterModified(graph,-1,-1);
    count = 0;

    /* Get Header (for later cluster score assignment */
    header = ReadHeader(fp, &headersize); 

    while(!feof(fp)){
	if(fgets(line,1000,fp) == NULL) break;
	/*
	** ignore comments
	*/
	if(line[0] == '%') continue;
	switch(level){
	case 0:
	    /*
	    ** read in the clusterid
	    */
	    if(sscanf(line,"%d",&clusterid) == 1){
		level = 1;
	    }
	    break;
	case 1:			/* projection numbers */
	    if(sscanf(line,"%d%d",&p0,&p1)  == 2){
		graph->currentprojection[0] = p0;
		graph->currentprojection[1] = p1;
		level = 2;
	    }
	    break;
	case 2:			/* projection 0 name */
	    if(strlen(line) > 0){
		strcpy(pname0,line);
		/*
		** try to look up the projection number for the name
		*/
		if((pinfo =	GetProjectionInfoByName(graph,pname0)) !=
		NULL){
		    p0 = graph->currentprojection[0] = pinfo->projectionid;
		} else {
		    fprintf(stderr,"could not find projection %s\n",
		    pname0);
		}
		level = 3;
	    }
	    break;
	case 3:			/* projection 1 name */
	    if(strlen(line) > 0){
		strcpy(pname1,line);
		/*
		** try to look up the projection number for the name
		*/
		if((pinfo =	GetProjectionInfoByName(graph,pname1)) !=
		NULL){
		    p1 = graph->currentprojection[1] = pinfo->projectionid;
		    
		} else {
		    fprintf(stderr,"could not find projection %s\n",
		    pname1);
		}
		level = 4;
	    }
	    break;
	case 4:			/* number of coordinates */
	    if(sscanf(line,"%d",&ncoords)  == 1){
		level = 5;
	    }
	    break;
	case 5:			/* coordinates */
	    if(sscanf(line,"%f%f",&wx1,&wy1)  == 2){
		if(count == ncoords -1){
		    SetClusterModified(graph,clusterid,1);
		    AssignClusterBounds(graph,clusterid,p0,p1,
		    ncoords,count,wx1,wy1,COMPLETE);
		    level = 0;
		    count = 0;
		} else {
		    AssignClusterBounds(graph,clusterid,p0,p1,
		    ncoords,count,wx1,wy1,INCOMPLETE);
		    count++;
		}
	    }
	    break;
	}
    }
    
    /* Set Cluster scores (has to wait until clusters have been
    ** defined) */
    AssignClusterScores(GetHeaderParameter(header,"Cluster Scores:"));

    fclose(fp);
    /*
    ** restore original projections
    */
    graph->currentprojection[0] = sp0;
    graph->currentprojection[1] = sp1;

    if(level != 0){
	fprintf(stderr,"error in reading bounds file\n");
    } else {
	AssignClusterPoints(graph);
	AssignClusterButtons();
	RefreshClusterScoreButtons();
	ScaleAndRefreshGraph(graph);
	fprintf(stderr,"done reading in bounds from file\n");
    }
}

