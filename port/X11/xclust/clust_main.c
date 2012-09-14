#include "xclust_ext.h"

/*
******************************************************************
**                                                              **
**                   Cluster Maintenance Routines               **
**                                                              **
******************************************************************
*/

AddClusterToList(graph,clusterid)
Graph	*graph;
int	clusterid;
{
ClusterList	*clist;

    if(graph == NULL) return;
    /*
    ** look for matches
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	if(clist->clusterid == clusterid) break;
    }
    if(clist == NULL){
	/*
	** no match found. Add the cluster id to the list
	*/
	if((clist = (ClusterList *)calloc(1,sizeof(ClusterList))) == NULL){
	    fprintf(stderr,"MEMORY ERROR: unable to allocate cluster list\n");
	    return;
	}
	clist->clusterid = clusterid;
	clist->enabled = 1;
	clist->showonly = 0;
	/*
	** insert into the cluster list
	*/
	clist->next = graph->clusterlist;
	graph->clusterlist = clist;
    }
    /*
    ** update the menu display
    */
    UpdateClusterMenu();
}

AssignClusterBounds(graph,clusterid,p0,p1,ncoords,index,wx1,wy1,complete)
Graph	*graph;
int	clusterid;
int	p0,p1;
int	ncoords;
int	index;
float	wx1,wy1;
int	complete;
{
ClusterBounds	*cb;
int	sx1,sx2,sy1,sy2;
int	deletedbounds;

    if(graph == NULL) return;
    if(index >= ncoords && index != -1 && ncoords != -1){
	fprintf(stderr,"coordinate index greater than ncoords\n");
	return;
    }
    /*
    ** compute the equivalent screen coords
    */
    ScreenTransform(graph,wx1,wy1,&sx1,&sy1);
    /*
    ** look for an existing cluster box in the list with the
    ** same clusterid as the current selection and the same
    ** projection parameters as the current selection
    */
    deletedbounds = 0;
    for(cb=graph->clusterbounds;cb;cb=cb->next){
	if(cb->clusterid == clusterid &&
	( (cb->projection[0] == p0 && cb->projection[1] == p1) || (cb->projection[1] == p0 && cb->projection[0] == p1) ) ){
	    /*
	    ** got a match, so replace this with the new values
	    */
	    /*
	    ** was this a previously completed bound?
	    */
	    if(cb->complete){
		/*
		** if so then replace it
		*/
#ifdef OLD
		/*
		** make a backup of the current cluster bounds
		*/
		BackupClusterBounds(graph);
		deletedbounds = 1;
#endif

		if (cb->projection[0] == p1)  /* mirror */
		  DeleteClusterBounds(graph,clusterid,p0,p1);
		else
		  DeleteClusterBounds(graph,clusterid,p1,p0);

		cb = NULL;
	    }
	    break;			/* no need to continue */
	}
    }
    if(cb == NULL){
#ifdef OLD
	/*
	** if adding a new box then save the previous bounds
	*/
	if(!deletedbounds){
	    /*
	    ** make a backup of the current cluster bounds
	    */
	    BackupClusterBounds(graph);
	}
#endif
	/*
	** create a new cluster box
	*/
	if((cb = (ClusterBounds *)calloc(1,sizeof(ClusterBounds))) == NULL){
	    fprintf(stderr,"MEMORY ERROR: unable to allocate cluster bounds\n");
	    return;
	}
	
	/*
	** insert into the cluster box list
	*/
	cb->next = graph->clusterbounds;
	graph->clusterbounds = cb;
	if(ncoords == -1){
	    /*
	    ** auto allocation of the array for unknown ncoords
	    */
	    ncoords = 1;
	    index = 0;
	}
	if((cb->fcoord = (FCoord *)malloc(sizeof(FCoord)*ncoords)) == NULL){
	    fprintf(stderr,"MEMORY ERROR: unable to allocate cluster bounds coordinates\n"); 
	    return;
	}
	if((cb->coord = (Coord *)malloc(sizeof(Coord)*ncoords)) == NULL){
	    fprintf(stderr,"MEMORY ERROR: unable to allocate cluster bounds coordinates\n"); 
	    return;
	}
	cb->ncoords = ncoords;
    } else {
	if(ncoords == -1){
	    /*
	    ** auto allocation of the array for unknown ncoords
	    */
	    index = cb->ncoords;
	    cb->ncoords++;
	    if((cb->fcoord = 
	    (FCoord *)realloc(cb->fcoord,sizeof(FCoord)*cb->ncoords)) == NULL){
		fprintf(stderr,
		"MEMORY ERROR: unable to reallocate cluster bounds coordinates\n");
		return;
	    }
	    if((cb->coord = 
	    (Coord *)realloc(cb->coord,sizeof(Coord)*cb->ncoords)) == NULL){
		fprintf(stderr,
		"MEMORY ERROR: unable to reallocate cluster bounds coordinates\n");
		return;
	    }
	}
    }
    cb->fcoord[index].x = wx1;		/* world coords */
    cb->fcoord[index].y = wy1;
    cb->coord[index].x = sx1;		/* screen coords */
    cb->coord[index].y = sy1;
    cb->enabled = 1;
    cb->clusterid = clusterid;
    cb->projection[0] = p0;
    cb->projection[1] = p1;
    cb->complete = complete;
    /*
    ** try adding to the cluster list
    */
    AddClusterToList(graph,clusterid);
    /*
    ** draw the new cluster box
    SetColor(LookupClusterColor(graph,clusterid));
    DrawClusterBounds(graph,cb);
    */

}

AddClusterInfo(graph,clusterid,color)
{
}

int CheckClusterModified(graph,clusterid)
Graph	*graph;
int	clusterid;
{
ClusterList	*clist;

    if(graph == NULL) return(1);
    if(clusterid == 0) return(1);
    for(clist=graph->clusterlist;clist;clist=clist->next){
	if(clist->clusterid == clusterid){
	    return(clist->modified);
	}
    }
    return(1);
}

/*
** returns the clusterlist assigned to the cluster
*/
ClusterList *GetClusterInfo(clusterid)
int	clusterid;
{
Graph	*graph;
ClusterList	*clist;

    graph = GetGraph("/graph");
    /*
    ** go through all of the defined clusters
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	/*
	** find the one with the matching clusterid
	*/
	if(clusterid == clist->clusterid){
	    /*
	    ** return it
	    */
	    return(clist);
	}
    }
    return(NULL);
}

/*
** returns the clusterlist assigned to the point
*/
ClusterList *TestAndAssignToCluster(graph,rawdata,vectorsize,clusterid)
Graph	*graph;
DataCoord	*rawdata;
int	vectorsize;
int	clusterid;
{
ClusterBounds	*cb;
ClusterList	*clist;
int	p0,p1;
int	success;
ProjectionInfo	*pinfo;
double	p0val,p1val;

    if(graph == NULL) return(NULL);
    /*
    ** go through all of the defined clusters
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	if(clusterid != clist->clusterid) continue;
	/*
	** initially assume that it does not belong to the cluster
	*/
	success = 0;
	/*
	** go through all of the cluster bounds
	*/
	for(cb=graph->clusterbounds;cb;cb=cb->next){
	    if(!cb->enabled) continue;
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
	    p0val = GetDoubleFromRawData(rawdata->data,pinfo);
	    if((pinfo=GetProjectionInfo(graph,p1)) == NULL){
		fprintf(stderr,"ERROR: Unable info on field %d\n",p1);
		return(0);
	    }
	    p1val = GetDoubleFromRawData(rawdata->data,pinfo);
	    /*
	    ** make sure the vector is large enough
	    */
	    if((p0 > vectorsize) || (p1 > vectorsize)) continue;
	    /*
	    ** use a simpler test if it is a bounding box
	    */
	    if(cb->type == BOXBOUND){
		/*
		** now see whether the point actually falls within the bounding
		** region
		*/
		if(p0val < cb->fcoord[0].x ||
		p0val > cb->fcoord[2].x ||
		p1val < cb->fcoord[0].y ||
		p1val > cb->fcoord[2].y){
		    /*
		    ** it's outside of the cluster box so no point in testing 
		    ** for membership in this cluster any further
		    */
		    success = 0;
		    break;
		}
	    } else {
		/*
		** otherwise run the polygon enclosure test
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
	    ** so assign it to the cluster
	    */
	    rawdata->clusterid = clusterid;
	    /*
	    ** note that this will cause points which belong to 
	    ** more than one cluster to be assigned to the first
	    ** cluster box in the list (which is the one defined
	    ** last). The file output of the program will assign
	    ** the point to both clusters, independent of this cluster
	    ** assignment.
	    */
	    return(clist);
	}
    }
    return(NULL);
}


#define MAXOVERLAP	50

/*
** yes, this MAXOVERLAP thing is a hack.
** clean it up later
*/
int EvaluateOverlap(graph)
Graph	*graph;
{
Plot	*plot;
int	i,j,k;
int	overlap[MAXOVERLAP];
int	noverlap;
int	overlaplist[MAXOVERLAP][MAXOVERLAP];
int	haveoverlap;
int	totaloverlap;

    if(graph == NULL) return(0);
    fprintf(stderr,"\nEvaluating overlapping cluster points:      ");
    /*
    ** clear the overlap counts
    */
    bzero(overlaplist,sizeof(int)*MAXOVERLAP*MAXOVERLAP);
    haveoverlap=0;
    totaloverlap = 0;
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->rawdata == NULL) continue;
	for(i=0;i<plot->npoints;i++){
	    if((plot->npoints > 100) && (i%(plot->npoints/100) == 0)){
		fprintf(stderr,"\b\b\b\b%3d%%",i*100/plot->npoints);
	    }
	    /*
	    ** check the cluster affiliation of this point
	    */
	    noverlap = TestClusterPointOverlap(graph,&(plot->rawdata[i]),
	    plot->ndatafields,overlap,MAXOVERLAP);
	    /*
	    ** how many clusters does this point belong to?
	    */
	    if(noverlap > 1){
		/*
		** flag the presence of at least one overlapping point
		*/
		haveoverlap=1;
		/*
		** ok, have an overlapping cluster point 
		** so go through and increment counters for
		** the offending clusters
		*/
		/*
		** use the first cluster in the list as the
		** reference
		*/
		if(overlap[0] > MAXOVERLAP){
		    fprintf(stderr,"ERROR: exceeded max cluster in overlap eval\n");
		    return(0);
		}
		overlaplist[overlap[0]][0]++;
		for(j=1;j<noverlap;j++){
		    if(overlap[j] > MAXOVERLAP){
			fprintf(stderr,"ERROR: exceeded max cluster in overlap eval\n");
			return(0);
		    }
		    overlaplist[overlap[0]][overlap[j]]++;
		}
	    }
	}
    }
    /*
    ** give the results
    */
    fprintf(stderr,"\n");
    if(haveoverlap){
	fprintf(stderr,"%10s %30s\n","ClusterID","Overlaps with (npoints)");
	for(i=1;i<MAXOVERLAP;i++){
	    if(overlaplist[i][0] > 0){
		fprintf(stderr,"%10d ",i);
		for(k=1;k<MAXOVERLAP;k++){
		    if(overlaplist[i][k] > 0){
			fprintf(stderr,"\t%d (%d)",k,overlaplist[i][k]);
			totaloverlap += overlaplist[i][k];
		    }
		}
		fprintf(stderr,"\n");
	    }
	}
	fprintf(stderr,"done\n");
    } else {
	fprintf(stderr,"No overlapping cluster points\n");
    }
    return(totaloverlap);
}

/*
** tests points for multiple cluster assignments
*/
int TestClusterPointOverlap(graph,rawdata,vectorsize,overlap,overlapsize)
Graph	*graph;
DataCoord	*rawdata;
int	vectorsize;
int	*overlap;
int	overlapsize;
{
ClusterBounds	*cb;
ClusterList	*clist;
int	p0,p1;
int	clusterid;
int	success;
int	noverlap;
double	p0val,p1val;
ProjectionInfo	*pinfo;

    if(graph == NULL) return(0);
    /*
    ** OK, this is a cheesy hack, but hey, I dont have all day here
    ** I'll tidy up later
    ** return the number of clusters to which the point belongs 
    ** and fill the overlap array with their cluster ids 
    */
    noverlap = 0;
    /*
    ** go through all of the defined clusters
    */
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
	    if(!cb->enabled) continue;
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
	    p0val = GetDoubleFromRawData(rawdata->data,pinfo);
	    if((pinfo=GetProjectionInfo(graph,p1)) == NULL){
		fprintf(stderr,"ERROR: Unable info on field %d\n",p1);
		return(0);
	    }
	    p1val = GetDoubleFromRawData(rawdata->data,pinfo);
	    /*
	    ** make sure the vector is large enough
	    */
	    if((p0 > vectorsize) || (p1 > vectorsize)) continue;
	    /*
	    ** use a simpler test if it is a bounding box
	    */
	    if(cb->type == BOXBOUND){
		/*
		** now see whether the point actually falls within the bounding
		** region
		*/
		if(p0val < cb->fcoord[0].x ||
		p0val > cb->fcoord[2].x ||
		p1val < cb->fcoord[0].y ||
		p1val > cb->fcoord[2].y){
		    /*
		    ** it's outside of the cluster box so no point in testing 
		    ** for membership in this cluster any further
		    */
		    success = 0;
		    break;
		}
	    } else {
		/*
		** otherwise run the polygon enclosure test
		*/
		if(!IsInPolygon(p0val,p1val,
		cb->fcoord,cb->ncoords)){
		    success = 0;
		    break;
		}
	    }
	}
	if(success){
	    if(noverlap >= overlapsize){
		fprintf(stderr,"Warning: exceeded overlap array bounds\n");
		return(noverlap);
	    }
	    overlap[noverlap] = clusterid;
	    noverlap++;
	}
    }
    return(noverlap);
}

/*
** returns the clusterlist assigned to the point
*/
ClusterList *AssignClusterPoint(graph,rawdata,vectorsize,requested_clusterid)
Graph	*graph;
DataCoord	*rawdata;
int	vectorsize;
int	requested_clusterid;
{
ClusterBounds	*cb;
ClusterList	*clist;
int	p0,p1;
int	clusterid;
int	success;
ProjectionInfo	*pinfo;
double	p0val,p1val;

    if(graph == NULL) return(NULL);
    /*
    ** go through all of the defined clusters
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	clusterid = clist->clusterid;
	if(requested_clusterid > 0 && clusterid != requested_clusterid){
	    continue;
	}
	/*
	** initially assume that it does not belong to the cluster
	*/
	success = 0;
	/*
	** go through all of the cluster bounds
	*/
	for(cb=graph->clusterbounds;cb;cb=cb->next){
	    if(!cb->enabled) continue;
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
	    p0val = GetDoubleFromRawData(rawdata->data,pinfo);
	    if((pinfo=GetProjectionInfo(graph,p1)) == NULL){
		fprintf(stderr,"ERROR: Unable info on field %d\n",p1);
		return(0);
	    }
	    p1val = GetDoubleFromRawData(rawdata->data,pinfo);
	    /*
	    ** make sure the vector is large enough
	    */
	    if((p0 > vectorsize) || (p1 > vectorsize)) continue;
	    /*
	    ** use a simpler test if it is a bounding box
	    */
	    if(cb->type == BOXBOUND){
		/*
		** now see whether the point actually falls within the bounding
		** region
		*/
		if(p0val < cb->fcoord[0].x ||
		p0val > cb->fcoord[2].x ||
		p1val < cb->fcoord[0].y ||
		p1val > cb->fcoord[2].y){
		    /*
		    ** it's outside of the cluster box so no point in testing 
		    ** for membership in this cluster any further
		    */
		    success = 0;
		    break;
		}
	    } else {
		/*
		** otherwise run the polygon enclosure test
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
	    ** so assign it to the cluster
	    */
	    rawdata->clusterid = clusterid;
	    /*
	    ** note that this will cause points which belong to 
	    ** more than one cluster to be assigned to the first
	    ** cluster box in the list (which is the one defined
	    ** last). The file output of the program will assign
	    ** the point to both clusters, independent of this cluster
	    ** assignment.
	    */
	    return(clist);
	}
    }
    return(NULL);
}

/*
** use the currently selected cluster as the focus of the
** interval analysis
*/

ComputeClusterIntervals(graph,clusterid,individual)
Graph	*graph;
int	clusterid;
int	individual;
{
Plot	*plot;
int	i,j,k;
ClusterList	*clist;
ProjectionInfo	*pinfo;
int	ptmp;
ProjectionInfo	*ptime,*pid,*pht,*pdh,*pint,*pblen;
int	blen;
int	maxid;
/*float	time;*/
double	minint;
double	minht;
double	interval;
double	t0,h0;
double	hprev;
char	tmpstring[80];
char	tmpstring2[80];
MenuItem	*item;
short	first;
int	forward_start;
int	back_start;
int	found_forward_point;
int	have_forward_points;
int	csi_plus;
double	csi_minint;
double	csi_maxint;
double	bwin;
double	tprev;
int	use_dh;
int	offset;
int	newoffset;
int	nfields;

    if(graph == NULL) return;
    plot = graph->plot;

    csi_minint = strtod(GetItemValue("/analysismenu/csimin"), NULL)*1e-3;
    csi_maxint = strtod(GetItemValue("/analysismenu/csimax"), NULL)*1e-3;
    bwin = strtod(GetItemValue("/analysismenu/csbwin"),NULL)*1e-3;
    use_dh = GetItemState("/analysismenu/csusedh");
    fprintf(stderr,"computing intervals for cluster %d\n",
    clusterid);
    /*
    ** find the id projection
    */

    if((pinfo = GetProjectionInfoByName(graph,"id")) == NULL){
	fprintf(stderr,"Unable to find 'id' parameter\n");
	return;
    }
    pid = pinfo;
    /*
    ** save the current projection
    */
    ptmp = graph->currentprojection[0];
    graph->currentprojection[0] = pid->projectionid;
    /*
    ** sort the points on the current projection
    */
    Sort(graph);
    graph->currentprojection[0] = ptmp;
    /*
    ** get the current data vector size
    */
    offset = 0;
    for(pinfo=graph->projectioninfo;pinfo != NULL; pinfo=pinfo->next){
	offset += pinfo->size;
    }
    /*
    ** add the additional projections
    */
    if((pinfo = GetProjectionInfoByName(graph,"int")) == NULL){
	/*
	** if it doesnt exist then add it
	*/
	/*
	** find the last id
	*/
	maxid = 0;
	for(pinfo=graph->projectioninfo;pinfo != NULL; pinfo=pinfo->next){
	    if(pinfo->projectionid > maxid){
		maxid = pinfo->projectionid;
	    }
	}
	fprintf(stderr,"adding interval projection %d\n",maxid+1);
	pinfo = AddProjectionInfo(graph,maxid+1,"int",DOUBLE,sizeof(double));
    } else {
	/*
	fprintf(stderr,"found int projection %d\n",pinfo->projectionid);
	*/
    }
    /*
    ** find the differential amplitude projection
    */
    if((pinfo = GetProjectionInfoByName(graph,"delh")) == NULL){
	/*
	** if it doesnt exist then add it
	*/
	/*
	** find the last id
	*/
	maxid = 0;
	for(pinfo=graph->projectioninfo;pinfo != NULL; pinfo=pinfo->next){
	    if(pinfo->projectionid > maxid){
		maxid = pinfo->projectionid;
	    }
	}
	fprintf(stderr,"adding delh projection %d\n",maxid+1);
	pinfo = AddProjectionInfo(graph,maxid+1,"delh",DOUBLE,sizeof(double));
    } else {
	/*
	fprintf(stderr,"found delh projection %d\n",pinfo->projectionid);
	*/
    }
    /*
    ** find the burst projection
    */
    if((pinfo = GetProjectionInfoByName(graph,"blen")) == NULL){
	/*
	** if it doesnt exist then add it
	*/
	/*
	** find the last id
	*/
	maxid = 0;
	for(pinfo=graph->projectioninfo;pinfo != NULL; pinfo=pinfo->next){
	    if(pinfo->projectionid > maxid){
		maxid = pinfo->projectionid;
	    }
	}
	fprintf(stderr,"adding blen projection %d\n",maxid+1);
	pinfo = AddProjectionInfo(graph,maxid+1,"blen",INT,sizeof(int));
    } else {
	/*
	fprintf(stderr,"found blen projection %d\n",pinfo->projectionid);
	*/
    }
    /*
    ** if the size of the data vector has changed then reallocate all 
    **  data points to account for the new projections
    */
    newoffset = 0;
    nfields=0;
    for(pinfo=graph->projectioninfo;pinfo != NULL; pinfo=pinfo->next){
	newoffset += pinfo->size;
	nfields++;
    }
    /*
    if(newoffset > offset){
    */
    if(newoffset > plot->rawdata[0].datasize){
	fprintf(stderr,"reallocating\n");
	for(i=0;i<plot->npoints;i++){
	    plot->rawdata[i].data = (char *)realloc(plot->rawdata[i].data,newoffset*sizeof(char));
	    plot->rawdata[i].datasize = newoffset*sizeof(char);
	}
	/*
	** and update the number of datafields in the plot based upon
	** the new fields
	*/
	plot->ndatafields = nfields;
    }

    if((pinfo = GetProjectionInfoByName(graph,"blen")) == NULL){
	fprintf(stderr,"Unable to find 'blen' parameter\n");
	return;
    }
    pblen = pinfo;
    if((pinfo = GetProjectionInfoByName(graph,"int")) == NULL){
	fprintf(stderr,"Unable to find 'int' parameter\n");
	return;
    }
    pint = pinfo;
    if((pinfo = GetProjectionInfoByName(graph,"delh")) == NULL){
	fprintf(stderr,"Unable to find 'delh' parameter\n");
	return;
    }
    pdh = pinfo;
    if((pinfo = GetProjectionInfoByName(graph,"t_maxht")) == NULL){
	fprintf(stderr,"Unable to find 't_maxht' parameter\n");
	return;
    }
    pht = pinfo;
    if((pinfo = GetProjectionInfoByName(graph,"time")) == NULL){
	fprintf(stderr,"Unable to find 'time' parameter\n");
	return;
    }
    ptime = pinfo;

    sprintf(tmpstring2," %d",clusterid);

    /*this is not really a fix, it just introduces another bug */
    /*     strcat(pint->name, tmpstring2); */
    /*     strcat(pdh->name, tmpstring2); */
    /*     strcat(pblen->name, tmpstring2); */
    
/*     sprintf(tmpstring,"/clustermenu/projection/x/p%d",pint->projectionid); */
/*     PutItemValue(tmpstring,tmpstring2); */
/*     sprintf(tmpstring,"/clustermenu/projection/y/p%d",pint->projectionid); */
/*     PutItemValue(tmpstring,tmpstring2); */

/*     sprintf(tmpstring,"/clustermenu/projection/x/p%d",pdh->projectionid); */
/*     PutItemValue(tmpstring,tmpstring2); */
/*     sprintf(tmpstring,"/clustermenu/projection/y/p%d",pdh->projectionid); */


    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->rawdata == NULL) continue;
	/*
	** initialize the complex spike index for each cluster
	*/
	if(individual){
	    for(clist=graph->clusterlist;clist;clist=clist->next){
		if(clist->clusterid == clusterid){
		    break;
		}
	    }
	    if(clist){
		clist->csi = 0;
		clist->ncsi = 0;
		clist->csiclust = 0;
		clist->burstlen = 0;
		clist->ncsi_plus = 0;
		clist->ncsi_refract_minus = 0;
	    }
	} else
	for(clist=graph->clusterlist;clist;clist=clist->next){
	    clist->csi = 0;
	    clist->ncsi = 0;
	    clist->csiclust = clusterid;
	    clist->burstlen = 0;
	    clist->ncsi_plus = 0;
	    clist->ncsi_refract_minus = 0;
	}
	back_start = 0;
	forward_start = 1;
	have_forward_points = 1;
	/*
	** go through each point outside of the cluster
	*/
	for(i=0;i<plot->npoints;i++){
	    if(i%1000 == 0){
		fprintf(stderr,"%7i\b\b\b\b\b\b\b",i);
	    }
	    if(plot->rawdata[i].clusterid == 0){
	        AssignRawDataFromDouble(plot->rawdata[i].data, pblen, 0);
		AssignRawDataFromDouble(plot->rawdata[i].data, pint, 0);
		AssignRawDataFromDouble(plot->rawdata[i].data, pdh, 0);
		/*
		*((float *)(plot->rawdata[i].data + pblen->offset)) = 0;
		*((float *)(plot->rawdata[i].data + pint->offset)) = 0;
		*((float *)(plot->rawdata[i].data + pdh->offset)) = 0;
		*/
		continue;
	    }
	    if(individual && (plot->rawdata[i].clusterid != clusterid)){
		/*
		** if CSI evaluation is restricted to target cluster
		** then skip all others
		*/
		continue;
	    }
	    /*
	    ** is this point in the cluster to be analyzed
	    */
	    /*
	    ** if the points are ordered then the search time can
	    ** be dramatically reduced
	    */
	    /*
	    ** otherwise look for the minimal interval between it
	    ** and any point in the target cluster
	    */
	    t0 = GetDoubleFromRawData(plot->rawdata[i].data,ptime);
	    h0 = GetDoubleFromRawData(plot->rawdata[i].data,pht);
	    minint = 0;
	    minht = 0;
	    first = 1;
	    /*
	    ** we can optimize the search further by recognizing
	    ** that after testing the points adjacent to the
	    ** current point, we can immediately start the search
	    ** at the first point in the previous search
	    */
	    /*
	    ** test earlier points
	    */
	    j = i-1;
	    if(j >= 0){
		/*
		** note that one of these two conditions should 
		** be met after the first search that finds a point
		*/
		if(plot->rawdata[j].clusterid == clusterid){
		    first = 0;
		    /*
		    ** take note of this point so that the search
		    ** from the next time through the main loop
		    ** can be started here
		    */
		    back_start = j;
		    minint = t0 - GetDoubleFromRawData(plot->rawdata[j].data,ptime);
		    minht = h0 - GetDoubleFromRawData(plot->rawdata[j].data,pht);
		} else
		/*
		** then the first point from the last pass
		*/
		if(plot->rawdata[back_start].clusterid == clusterid){
		    first = 0;
		    minint = t0 - GetDoubleFromRawData(plot->rawdata[back_start].data,ptime);
		    minht = h0 - GetDoubleFromRawData(plot->rawdata[back_start].data,pht);
		}
	    }
	    /*
	    ** the current point will be the first back_start for the
	    ** next pass
	    */
	    if(plot->rawdata[i].clusterid == clusterid){
		back_start = i;
	    }
	    /*
	    ** now do the forward search
	    */
	    found_forward_point = 0;
	    j = i+1;
	    if((j <= plot->npoints) && have_forward_points){
		/*
		** test the first point from the last pass
		*/
		if((forward_start > i) && 
		(plot->rawdata[forward_start].clusterid == clusterid)){
		    interval = t0 - GetDoubleFromRawData(plot->rawdata[forward_start].data,ptime);
		    if(first || (fabs(interval) < fabs(minint))){
			first = 0;
			minint = interval;
			minht = h0 - GetDoubleFromRawData(plot->rawdata[forward_start].data,pht);
		    }
		    found_forward_point = 1;
		}
		/*
		** if no point was found then search forward for the first
		** point. 
		*/
		if(!found_forward_point){
		    for(j=i+1;j<plot->npoints;j++){
			if(plot->rawdata[j].clusterid == clusterid){
			    interval = t0 - GetDoubleFromRawData(plot->rawdata[j].data,ptime);
			    if(first || (fabs(interval) < fabs(minint))){
				minint = interval;
				minht = h0 - GetDoubleFromRawData(plot->rawdata[j].data,pht);
			    }
			    forward_start = j;
			    break;
			}
		    }
		    if(j == plot->npoints){
			/*
			** it should only get to this point if there
			** are no more matching points in the data set.
			** this indicates that any future forward 
			** search will fail, so dont bother
			*/
			have_forward_points = 0;
		    }
		}
	    }
	    /*
	    ** assign the min interval
	    */
	    AssignRawDataFromDouble(plot->rawdata[i].data, pint, minint);
	    AssignRawDataFromDouble(plot->rawdata[i].data, pdh, minht);
	    /*
	    *((float *)(plot->rawdata[i].data + pint->offset)) = minint;
	    *((float *)(plot->rawdata[i].data + pdh->offset)) = minht;
	    */
	    /*
	    ** compute the complex spike index for the cluster
	    ** in the distribution of interval vs dh
	    ** add points in quadrants 2 and 4 
	    ** subtract points in quadrants 1 and 3 
	    ** subtract points with intervals of less than 3 msec
	    ** only consider points with intervals of less than 15 msec
	    */
	    for(clist=graph->clusterlist;clist;clist=clist->next){
		if(clist->clusterid == plot->rawdata[i].clusterid){
		    break;
		}
	    }
	    /*
	    ** dont compute a csi for the zero cluster
	    */
	    if(clist == NULL){
		continue;
	    }
	    blen = 0;
	    if(fabs(minint) <= csi_maxint){
		csi_plus = 0;
		if(fabs(minint) < csi_minint){
		    clist->csi--;
		} else
		if(!use_dh){
		    clist->csi++;
		    csi_plus = 1;
		} else
		if(minht <= 0 && minint > 0){
		    clist->csi++;
		    csi_plus = 1;
		} else
		if(minht > 0 && minint < 0){
		    clist->csi++;
		    csi_plus = 1;
		} else
		if(minht <= 0 && minint < 0){
		    clist->csi--;
		    clist->ncsi_refract_minus++;
		} else
		if(minht > 0 && minint > 0){
		    clist->csi--;
		}
		/*
		** keep track of the number of points going into the csi
		*/
		clist->ncsi++;
		/*
		** now scan for the number of events in either of the
		** two clusters within a time window
		*/
		if(csi_plus){
		    clist->ncsi_plus++;
		    /*
		    ** count the spike itself
		    */
		    clist->burstlen++;
		    blen++;
		    tprev = t0;
		    hprev = h0;
		    for(k=i-1;k>0;k--){
			if((t0 - GetDoubleFromRawData(plot->rawdata[k].data,ptime)) < bwin){
			    /*
			    ** and any others within 50 msec of it
			    */
			    if((plot->rawdata[k].clusterid == clusterid)
			    ||(plot->rawdata[k].clusterid == clist->clusterid)){
				/*
				** check for interval and amplitude constraints
				*/
				if(
				((tprev - GetDoubleFromRawData(plot->rawdata[k].data,ptime)) < csi_maxint) &&
				((tprev - GetDoubleFromRawData(plot->rawdata[k].data,ptime)) > csi_minint) &&
				(((hprev - GetDoubleFromRawData(plot->rawdata[k].data,pht)) < 0) || !use_dh)){ 
				    clist->burstlen++;
				    blen++;
				} else {
				    break;
				}
				tprev = GetDoubleFromRawData(plot->rawdata[k].data,ptime);
				hprev = GetDoubleFromRawData(plot->rawdata[k].data,pht);
			    }
			} else {
			    break;
			}
		    }
		    tprev = t0;
		    hprev = h0;
		    for(k=i+1;k<plot->npoints;k++){
			if((GetDoubleFromRawData(plot->rawdata[k].data,ptime) - t0) < bwin){
			    /*
			    ** and any others within 50 msec of it
			    */
			    if((plot->rawdata[k].clusterid == clusterid)
			    ||(plot->rawdata[k].clusterid == clist->clusterid)){
				/*
				** check for interval and amplitude constraints
				*/
				if(
				((GetDoubleFromRawData(plot->rawdata[k].data,ptime) - tprev) < csi_maxint) &&
				((GetDoubleFromRawData(plot->rawdata[k].data,ptime) - tprev) > csi_minint) &&
				(((GetDoubleFromRawData(plot->rawdata[k].data,pht) - hprev) < 0) || !use_dh)){ 
				    clist->burstlen++;
				    blen++;
				} else {
				    break;
				}
				tprev = GetDoubleFromRawData(plot->rawdata[k].data,ptime);
				hprev = GetDoubleFromRawData(plot->rawdata[k].data,pht);
			    }
			} else {
			    break;
			}
		    }
		}
	    }
	    *((float *)(plot->rawdata[i].data + pblen->offset)) = blen;
	}
    }
    fprintf(stderr,"\ndone\n");
}

ReturnClusterToZero(graph,clusterid)
Graph	*graph;
int	clusterid;
{
Plot	*plot;
int	i;
ClusterList	*clist;

    if(graph == NULL) return;
    /*
    ** clear the cluster point count
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	if(clist->clusterid == clusterid){
	    clist->npoints = 0;
	}
    }
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->rawdata == NULL) continue;
	for(i=0;i<plot->npoints;i++){
	    /*
	    ** is this point in the cluster to be modified
	    */
	    if(plot->rawdata[i].clusterid == clusterid){
		/*
		** place the point in cluster 0
		*/
		plot->rawdata[i].clusterid = 0;
		/*
		** then try to assign it to a cluster by evaluating
		** bounding cluster boxes
		*/
		clist = AssignClusterPoint(graph,&(plot->rawdata[i]),
		plot->ndatafields,0);
	    }
	}
    }
    UpdateClusterLabels();
    graph->changed = 1;
}

AssignClusterFromZero(graph,clusterid)
Graph	*graph;
int	clusterid;
{
Plot	*plot;
int	i;
ClusterList	*clist;

    /*
    ** clear the cluster point count
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	if(clist->clusterid == clusterid){
	    clist->npoints = 0;
	}
    }
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->rawdata == NULL) continue;
	for(i=0;i<plot->npoints;i++){
	    /*
	    ** is this point in the zero cluster
	    */
	    if(plot->rawdata[i].clusterid == 0){
		/*
		** then try to assign it to a cluster by evaluating
		** bounding cluster boxes
		*/
		if(clist = TestAndAssignToCluster(graph,&(plot->rawdata[i]),
		plot->ndatafields,clusterid)){
		    clist->npoints++;
		}
	    }
	}
    }
    UpdateClusterLabels();
    graph->changed = 1;
}

TouchClusters(graph)
Graph		*graph;
{
ClusterList	*clist;

    if(graph == NULL) return;
    /*
    ** mark all clusters as modified
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	clist->modified = 1;
    }
    graph->changed = 1;
}

AssignClusterPointsToCluster(graph,clusterid)
Graph	*graph;
int	clusterid;
{
Plot	*plot;
int	i;
ClusterList	*clist;

    if(graph == NULL) return;
    /*
    ** clear the cluster point counts
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	if(clist->modified){
	    clist->npoints = 0;
	}
    }
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->rawdata == NULL) continue;
	for(i=0;i<plot->npoints;i++){
	    /*
	    ** always process zero cluster points
	    */
	    if(plot->rawdata[i].clusterid == 0){
		/*
		** then try to assign it to a cluster by evaluating
		** bounding cluster boxes
		*/
		clist = AssignClusterPoint(graph,&(plot->rawdata[i]),
		plot->ndatafields,clusterid);
		/*
		** was the point in a cluster other than the 0 cluster
		*/
		if(clist){
		    /*
		    ** if it was then keep a tally of it
		    */
		    clist->npoints++;
		}
	    } else
	    /*
	    ** check to see whether the cluster bounds of the
	    ** current cluster assignment have changed
	    */
	    if(CheckClusterModified(graph,plot->rawdata[i].clusterid)){
		/*
		** test for a hard cluster. if so then dont reassign
		*/
		if(plot->rawdata[i].clusterid < 0) break;
		/*
		** place the point in cluster 0
		*/
		plot->rawdata[i].clusterid = 0;
		/*
		** then try to assign it to the requested cluster by evaluating
		** bounding cluster boxes
		*/
		clist = AssignClusterPoint(graph,&(plot->rawdata[i]),
		plot->ndatafields,clusterid);
		/*
		** was the point in the cluster 
		*/
		if(clist){
		    /*
		    ** if it was then keep a tally of it
		    */
		    clist->npoints++;
		} else {
		    /*
		    ** if it wasnt then try to put it in another
		    ** cluster
		    */
		    clist = AssignClusterPoint(graph,&(plot->rawdata[i]),
		    plot->ndatafields,0);
		    /*
		    ** was the point in a cluster other than the 0 cluster
		    */
		    if(clist){
			/*
			** if it was then keep a tally of it
			*/
			clist->npoints++;
		    }
		}
	    }
	}
    }
    /*
    ** clear the cluster modified flags
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	clist->modified = 0;
    }
    /*
    ** update the cluster labels if any
    */
    UpdateClusterLabels();
    graph->changed = 1;
}

AssignClusterPoints(graph)
Graph	*graph;
{
Plot	*plot;
int	i;
ClusterList	*clist;

    if(graph == NULL) return;
    /*
    ** clear the cluster point counts
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	if(clist->modified){
	    clist->npoints = 0;
	}
    }
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->rawdata == NULL) continue;
	for(i=0;i<plot->npoints;i++){
	    /*
	    ** check to see whether the cluster bounds of the
	    ** current cluster assignment have changed
	    */
	    if(CheckClusterModified(graph,plot->rawdata[i].clusterid)){
		/*
		** test for a hard cluster. if so then dont reassign
		*/
		if(plot->rawdata[i].clusterid < 0) break;
		/*
		** place the point in cluster 0
		*/
		plot->rawdata[i].clusterid = 0;
		/*
		** then try to assign it to a cluster by evaluating
		** bounding cluster boxes
		*/
		clist = AssignClusterPoint(graph,&(plot->rawdata[i]),
		plot->ndatafields,0);
		/*
		** was the point in a cluster other than the 0 cluster
		*/
		if(clist){
		    /*
		    ** if it was then keep a tally of it
		    */
		    clist->npoints++;
		}
	    }
	}
    }
    /*
    ** clear the cluster modified flags
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	clist->modified = 0;
    }
    /*
    ** update the cluster labels if any
    */
    UpdateClusterLabels();
    graph->changed = 1;
}

int IsInPolygon(wx,wy,fcoord,ncoords)
double	wx,wy;
FCoord	*fcoord;
int	ncoords;
{
int	i;
int	pcross;
double	FY,bx;
    
    if(fcoord == NULL) return(0);
    /*
    ** look for odd number of intersections with poly segments
    */
    pcross = 0;
    /*
    ** extend a horizontal line along the positive x axis and
    ** look at intersections
    */
    for(i=0;i<ncoords-1;i++){
	/*
	** only examine segments whose endpoint y coords
	** bracket those of the test coord
	*/
	if((wy > fcoord[i].y && wy <= fcoord[i+1].y) ||
	(wy < fcoord[i].y && wy >= fcoord[i+1].y)){
	    /*
	    ** count those which are on the positive x side 
	    ** by computing the intercept.
	    ** find the x value of the line between the two fcoords
	    ** which is at wy
	    */
/*
	    (fcoord[i].y - wy)/(fcoord[i].x - bx) = 
		(fcoord[i+1].y - wy)/(fcoord[i+1].x - bx);

	    (f2x - bx)/(f1x - bx) = (f2y - wy)/(f1y - wy);
	    (f2x - bx) = (f1x - bx)*(f2y - wy)/(f1y - wy);
	    bx((f2y-wy)/(f1y-wy) -1)= f1x*(f2y-wy)/(f1y-wy) - f2x;
	    bx = (f1x*(f2y-wy)/(f1y-wy)-f2x)/((f2y-wy)/(f1y-wy)-1)

	    FY = (f2y-wy)/(f1y-wy);
	    bx = (f1x*FY - f2x)/(FY-1);
*/
	    FY = (fcoord[i+1].y-wy)/(fcoord[i].y-wy);
	    if(FY == 1){
		pcross++;
	    } else {
		bx = (fcoord[i].x*FY - fcoord[i+1].x)/(FY-1);

		if(bx >= wx){
		    pcross++;
		}
	    }
	}
    }
    if(i == ncoords-1){
	/*
	** compute the final point which closes the polygon
	*/
	if((wy > fcoord[i].y && wy <= fcoord[0].y) ||
	(wy < fcoord[i].y && wy >= fcoord[0].y)){
	    FY = (fcoord[0].y-wy)/(fcoord[i].y-wy);
	    if(FY == 1){
		pcross++;
	    } else {
		bx = (fcoord[i].x*FY - fcoord[0].x)/(FY-1);
		if(bx > wx){
		    pcross++;
		}
	    }
	}
    }
    /*
    ** now look for an odd number of crossings
    */
    if(pcross > 0 && (pcross%2 == 1)){
	return(1);
    } else {
	return(0);
    }
}

int _IsInPolygon(wx,wy,fcoord,ncoords)
double	wx,wy;
FCoord	*fcoord;
int	ncoords;
{
int	i;
register double	x1,x2,y1,y2;
double	cross,pcross;
int	success;
    
    if(fcoord == NULL) return(0);
    /*
    ** look for consistent sign in the cross product
    ** i.e is the point always to the left or right of the
    ** sides.
    */
    pcross = 0;
    success = 1;
    for(i=0;i<ncoords-1;i++){
	x1 = wx - fcoord[i].x; 
	y1 = wy - fcoord[i].y; 
	/*
	** these could actually be precomputed to speed things up a bit
	*/
	x2 = fcoord[i+1].x - fcoord[i].x; 
	y2 = fcoord[i+1].y - fcoord[i].y; 
	/*
	** compute the determinant
	*/
	cross = x1*y2 - x2*y1; 
	/*
	** and check for inconsistencies
	*/
	if((cross < 0 && pcross > 0) || (cross > 0 && pcross < 0)){
	    success = 0;
	    break;
	}
	pcross = cross;
    }
    if(i == ncoords-1){
	/*
	** compute the final point which closes the polygon
	*/
	x1 = wx - fcoord[i].x; 
	y1 = wy - fcoord[i].y; 
	/*
	** these could actually be precomputed to speed things up a bit
	*/
	x2 = fcoord[0].x - fcoord[i].x; 
	y2 = fcoord[0].y - fcoord[i].y; 
	/*
	** compute the determinant
	*/
	cross = x1*y2 - x2*y1; 
	/*
	** and check for inconsistencies
	*/
	if((cross < 0 && pcross > 0) || (cross > 0 && pcross < 0)){
	    success = 0;
	}
    }
    return(success);
}

DeleteAllClusterBounds(graph)
Graph	*graph;
{
ClusterBounds	*cb;
ClusterBounds	*cbnext;

    if(graph == NULL || graph->clusterbounds == NULL) return;
    cbnext = graph->clusterbounds->next;
    for(cb=graph->clusterbounds;cb;cb=cbnext){
	cbnext = cb->next;
	/*
	** free the cluster bounds data  
	*/
	if(cb->fcoord){
	    free(cb->fcoord);
	}
	if(cb->coord){
	    free(cb->coord);
	}
	free(cb);
    }
    graph->clusterbounds = NULL;
}

/*
** specifying projections p0 or p1 as negative causes them
** to be ignored allowing deletion of cluster boxes over
** all projections
*/
DeleteClusterBounds(graph,clusterid,p0,p1)
Graph	*graph;
int	clusterid;
int	p0;
int	p1;
{
ClusterBounds	*cb;
ClusterBounds	*lastcb;

    if(graph == NULL) return;
    /*
    ** initialize the pointer to the most recent valid cb
    */
    lastcb = NULL;
    /*
    ** search the cluster box list for a match
    */
    for(cb=graph->clusterbounds;cb;cb=cb->next){
	/*
	** test for a match
	*/
      if(cb->clusterid == clusterid &&
	 ( ( (p0 < 0 || cb->projection[0] == p0) && (p1 < 0 || cb->projection[1] == p1) ) ||
	 ( (p0 < 0 || cb->projection[0] == p1) && (p1 < 0 || cb->projection[1] == p0) ) ) ) {

	fprintf(stderr, "delete\n");

	    /*
	    ** delete it
	    */
	    if(lastcb == NULL){
		/*
		** first entry in the list
		*/
		graph->clusterbounds = cb->next;
	    } else {
		lastcb->next = cb->next;
	    }
	    /*
	    ** dont change the lastcb pointer since the most
	    ** recent valid cb has not changed
	    */
	    /*
	    ** free the cluster bounds data  
	    */
	    if(cb->fcoord){
		free(cb->fcoord);
		cb->fcoord = NULL;
	    }
	    if(cb->coord){
		free(cb->coord);
		cb->coord = NULL;
	    }
	} else {
	    /*
	    ** update the pointer to the most recent valid cb in
	    ** the list
	    */
	    lastcb = cb;
	}
    }

}

int DeleteClusterFromList(graph,clusterid)
Graph	*graph;
int	clusterid;
{
ClusterList	*clist;
ClusterList	*lastcl;
ClusterList	*currentcl;

    if(graph == NULL) return(0);
    /*
    ** search the cluster box list for a match
    */
    lastcl = NULL;
    /*
    ** go through the cluster list
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	currentcl = clist;
	if(clist->clusterid == clusterid || (clusterid == -1)){
	    /*
	    ** delete it
	    */
	    if(lastcl == NULL){
		/*
		** first entry in the list
		*/
		graph->clusterlist = clist->next;
		currentcl = NULL;
	    } else {
		lastcl->next = clist->next;
		currentcl = lastcl;
	    }
	    break;
	}
	lastcl = currentcl;
    }
    if(clist){
    /*
    ** update the menu display
    */
    UpdateClusterMenu();
	return(1);
    } else {
	return(0);
    }

}

SetClusterModified(graph,clusterid,value)
Graph	*graph;
int	clusterid;
int	value;
{
ClusterList	*clist;
    /*
    ** go through the cluster list
    */
    for(clist=graph->clusterlist;clist;clist=clist->next){
	if((clist->clusterid == clusterid) || (clusterid == -1)){
	    clist->modified = value;
	}
    }
}

DoDeleteAllClusterBounds(g)
Graph	*g;
{
    /*
    ** make a backup of the current cluster bounds
    */
    BackupClusterBounds(g);
    DeleteAllClusterBounds(g);
    SetClusterModified(g,g->selectedcluster,-1);
    AssignClusterPoints(g);
    ScaleAndRefreshGraph(g);
}

DoDeleteClusterBounds(g)
Graph	*g;
{
    /*
    ** make a backup of the current cluster bounds
    */
    BackupClusterBounds(g);
    DeleteClusterBounds(g,g->selectedcluster, g->currentprojection[0], g->currentprojection[1]);
    SetClusterModified(g,g->selectedcluster,1);
    AssignClusterPoints(g);
    ScaleAndRefreshGraph(g);
}

DoDeleteCluster(g)
Graph	*g;
{
    /*
    ** make a backup of the current cluster bounds
    */
    BackupClusterBounds(g);
    DeleteClusterBounds(g,g->selectedcluster,-1,-1);
    DeleteClusterFromList(g,g->selectedcluster);
    SetClusterModified(g,g->selectedcluster,1);
    AssignClusterPoints(g);
    ScaleAndRefreshGraph(g);
}

DoDeleteAllClusters(g)
Graph	*g;
{
    /*
    ** make a backup of the current cluster bounds
    */
    BackupClusterBounds(g);
    DeleteAllClusterBounds(g);
    while(DeleteClusterFromList(g,-1));
    SetClusterModified(g,-1,1);
    AssignClusterPoints(g);
    ScaleAndRefreshGraph(g);
}

CopyClusterBounds(graph,newclusterid)
Graph	*graph;
int	newclusterid;
{
ClusterBounds	*cb;
ClusterBounds	*lastcb;
ClusterBounds	*currentcb;
int	p0;
int	p1;
int	clusterid;
ClusterBounds	*newcb;
ClusterBounds	*newclist;

    if(graph == NULL) return;
    /*
    ** make a backup of the current cluster bounds
    */
    BackupClusterBounds(graph);
    /*
    ** delete any existing cluster bounds on the target cluster
    */
    DeleteClusterBounds(graph,newclusterid,-1,-1);
    /*
    ** search the cluster box list for a match
    */
    newclist = NULL;
    clusterid = graph->selectedcluster;
    lastcb = NULL;
    for(cb=graph->clusterbounds;cb;cb=cb->next){
	lastcb = cb;
	/*
	** test for a match
	*/
	if(cb->clusterid == clusterid){
	    /*
	    ** copy it
	    */
	    if((newcb = (ClusterBounds *)malloc(sizeof(ClusterBounds))) 
	    == NULL){
		fprintf(stderr,"MEMORY ERROR: unable to allocate new cluster bounds\n");
		return;
	    }
	    bcopy(cb,newcb,sizeof(ClusterBounds));
	    /*
	    ** copy the bounds coordinates
	    */
	    if(((newcb->fcoord=(FCoord *)malloc(newcb->ncoords*sizeof(FCoord))) 
	    == NULL) ||
	    ((newcb->coord=(Coord *)malloc(newcb->ncoords*sizeof(Coord))) 
	    == NULL)){
		fprintf(stderr,
		"MEMORY ERROR: allocating new cluster bounds\n");
		return;
	    }
	    bcopy(cb->fcoord,newcb->fcoord,newcb->ncoords*sizeof(FCoord));
	    bcopy(cb->coord,newcb->coord,newcb->ncoords*sizeof(Coord));
	    newcb->next = NULL;
	    newcb->clusterid = newclusterid;
	    if(newclist != NULL){
		newcb->next = newclist;
	    }
	    newclist = newcb;
	}
    }
    /*
    ** append the new cluster into the graph list
    */
    if(newclist){
	lastcb->next = newclist;
    }
    fprintf(stderr,"Copied cluster %d to cluster %d\n",clusterid,newclusterid);
    AddClusterToList(graph,newclusterid);
    SetClusterModified(graph,newclusterid,1);
    AssignClusterPoints(graph);
    ScaleAndRefreshGraph(graph);
}

SetClusterEnable(graph,clusterid,state)
Graph	*graph;
int	clusterid;
int	state;
{
ClusterBounds	*cb;
ClusterList	*clist;

    if((clist = GetClusterInfo(clusterid)) != NULL){
	clist->enabled = state;
    }
    for(cb=graph->clusterbounds;cb;cb=cb->next){
	if(cb->clusterid == clusterid){
	    cb->enabled = state;
	}
    }
#ifdef OLD
    SetClusterModified(graph,clusterid,1);
    AssignClusterPoints(graph);
#else
    if(state == 0){
	ReturnClusterToZero(graph,clusterid);
    } else {
	AssignClusterFromZero(graph,clusterid);
    }
#endif
}

HideCluster(graph,clusterid,state)
Graph	*graph;
int	clusterid;
int	state;
{
Plot	*plot;
int	i;

    /*
    ** set the hidden flag
    */
    clusterid = abs(clusterid);
    if(clusterid < 0 || clusterid >= MAXCLUSTER){
	fprintf(stderr,"WARNING: attempt to hide invalid clusterid %d\n",
	clusterid);
	return;
    }
    graph->hiddencluster[clusterid]  = state;
    graph->changed = 1;
}

/*
******************************************************************
**                                                              **
**                   Projection Manipulation Routines           **
**                                                              **
******************************************************************
*/

ProjectionInfo *AddProjectionInfo(graph,id,name,type,size)
Graph	*graph;
int	id;
char	*name;
short	type;
int	size;
{
ProjectionInfo	*pinfo;
ProjectionInfo	*newinfo;
ProjectionInfo	*first, *second, *follow, *prev;
int	minid;
short	swap;
int	offset;

    if(graph == NULL) return(NULL);
    newinfo = NULL;
    /*
    ** go through the projection info list 
    */
    for(pinfo=graph->projectioninfo;pinfo;pinfo=pinfo->next){
	/*
	** locate the projection with the matching id
	*/
	if(pinfo->projectionid == id){
	    /*
	    ** just change the existing info
	    */
	    newinfo = pinfo;
	    break;
	}
    }
    /*
    ** if none exists then add one
    */
    if(newinfo == NULL){
	if((newinfo = (ProjectionInfo *)calloc(1,sizeof(ProjectionInfo)))
	== NULL){
	    fprintf(stderr,"ERROR: unable to allocate projection info\n");
	    return(NULL);
	}
	newinfo->projectionid = id; 
	/*
	fprintf(stderr,"allocating new projection info for %d\n",id);
	*/
	newinfo->next = graph->projectioninfo;
	graph->projectioninfo = newinfo;
    }
    if(name){
	newinfo->name = (char *)malloc(strlen(name)+1);
	strcpy(newinfo->name,name);
    }
    newinfo->type = type;
    newinfo->size = size;
    /*
    fprintf(stderr,"assigning new projection info for %d: %s %d %d\n",
	id,newinfo->name,type,size);
    */
    /*
    ** must recalculate all offsets whenever a projection is added
    */
    /*
    ** sort the list by id and then add sizes to compute new offsets
    */
    swap = 1;
    while(swap){
	swap = 0;
	for(pinfo=graph->projectioninfo;pinfo;pinfo=pinfo->next){
	    /*
	    if((pinfo->next != NULL) && (pinfo->projectionid == pinfo->next->projectionid)){
		fprintf(stderr,"ERROR: corrupt projection list\n");
		exit(-1);
	    }
	    */
	    /*
	    ** compare ids of adjacent entrys
	    */
	    if((pinfo->next != NULL) && (pinfo->projectionid > pinfo->next->projectionid)){
		/*
		** swapthem
		*/
		first = pinfo->next;
		second = pinfo;
		follow = first->next;
		if(pinfo == graph->projectioninfo){
		    /*
		    ** insert into the head of the list
		    */
		    graph->projectioninfo = first;
		} else {
		    prev->next = first;
		}
		first->next = second;
		second->next = follow;
		swap = 1;
	    }
	    prev = pinfo;
	}
    }
    /*
    ** with the list sorted, recompute the offsets by summing sizes
    */
    offset = 0;
    graph->nprojections = 0;
    for(pinfo=graph->projectioninfo;pinfo;pinfo=pinfo->next){
	pinfo->offset = offset;
	offset += pinfo->size;
	graph->nprojections++;
    }
    graph->datalength = offset;
    return(newinfo);
}

ProjectionInfo	*GetProjectionInfoByName(graph,name)
Graph	*graph;
char	*name;
{
ProjectionInfo	*pinfo;
char	*ptr;
char	*strchr();

    if(graph == NULL) return(NULL);
    /*
    ** trim off any leading white space
    */
    while((name != NULL) && (*name != '\0')){
	if((*name != ' ') && (*name != '\t')){
	    break;
	}
	name++;
    }
    /*
    ** and any CRs at the end
    */
    if((ptr = strchr(name,'\n')) != NULL){
	*ptr = '\0';
    }
    /*
    ** go through the projection info list 
    */
    for(pinfo=graph->projectioninfo;pinfo;pinfo=pinfo->next){
	/*
	** locate the projection with the matching id
	*/
	if(strcmp(pinfo->name,name) == 0){
	    /*
	    ** return the info structure
	    */
	    return(pinfo);
	}
    }
    /*
    ** if none exists then return failure
    */
    return(NULL);
}

ProjectionInfo	*GetProjectionInfo(graph,id)
Graph	*graph;
int	id;
{
ProjectionInfo	*pinfo;

    if(graph == NULL) return(NULL);
    /*
    ** go through the projection info list 
    */
    for(pinfo=graph->projectioninfo;pinfo;pinfo=pinfo->next){
	/*
	** locate the projection with the matching id
	*/
	if(pinfo->projectionid == id){
	    /*
	    ** return the info structure
	    */
	    return(pinfo);
	}
    }
    /*
    ** if none exists then return failure
    */
    return(NULL);
}

int SetProjectionName(graph,id,name)
Graph	*graph;
int	id;
char	*name;
{
ProjectionInfo	*pinfo;

    if(graph == NULL) return(0);
    /*
    ** go through the projection info list 
    */
    for(pinfo=graph->projectioninfo;pinfo;pinfo=pinfo->next){
	/*
	** locate the projection with the matching id
	*/
	if(pinfo->projectionid == id){
	    /*
	    ** copy the name string into the projection info name
	    */
	    pinfo->name = (char *)malloc(strlen(name)+1);
	    strcpy(pinfo->name,name);
	    /*
	    ** return success
	    */
	    return(1);
	}
    }
    /*
    ** if none exists then return failure
    */
    return(0);
}


