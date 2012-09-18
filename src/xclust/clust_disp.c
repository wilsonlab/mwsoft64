#include "xclust_ext.h"

/*
******************************************************************
**                                                              **
**                   Cluster Display Routines                   **
**                                                              **
******************************************************************
*/

RescaleClusterBounds(graph)
Graph	*graph;
{
ClusterBounds	*cb;
int32_t	sx1,sy1;
int32_t	i;
double	wx,wy,wz;
double	nx,ny,nz;

    for(cb=graph->clusterbounds;cb;cb=cb->next){
	for(i=0;i<cb->ncoords;i++){
	    /*
	    ** apply transformations
	    */
	  wx = wy = wz = 0;

	  if (cb->projection[0] == graph->currentprojection[0])
	    wx = cb->fcoord[i].x;
	  else if (cb->projection[0] == graph->currentprojection[1])
	    wy = cb->fcoord[i].x;
	  else if (cb->projection[0] == graph->currentprojection[2])
	    wz = cb->fcoord[i].x;

	  if (cb->projection[1] == graph->currentprojection[0])
	    wx = cb->fcoord[i].y;
	  else if (cb->projection[1] == graph->currentprojection[1])
	    wy = cb->fcoord[i].y;
	  else if (cb->projection[1] == graph->currentprojection[2])
	    wz = cb->fcoord[i].y;


/* 	  if(graph->currentprojection[0] == cb->projection[0] && */
/* 	     graph->currentprojection[1] == cb->projection[1]){ */
/* 	    wx = cb->fcoord[i].x; */
/* 	    wy = cb->fcoord[i].y; */
/* 	  } else { */
/* 	    wx = cb->fcoord[i].y; */
/* 	    wy = cb->fcoord[i].x; */
/* 	  } */
/* 	    wz = 0; */
	    MatrixTransform(graph,wx,wy,wz,&nx,&ny,&nz);
	    /*
	    ** compute the equivalent screen coords
	    */
	    ScreenTransform(graph,(float)nx,(float)ny,&sx1,&sy1);
	    cb->coord[i].x = sx1;
	    cb->coord[i].y = sy1;
	}
    }
}

int32_t LookupClusterColor(graph,clusterid)
Graph	*graph;
int32_t		clusterid;
{
ClusterInfo	*cinfo;
int32_t		color;

    /* what is this used for? Who calls this with -ve numbers ??? */
    /* could be for partialplotload mode, but now a cluster # of -1 means
    ** to not draw that segment */
    clusterid = abs(clusterid);
 
    /*
    ** go through the clusterinfo to see whether an entry exists for
    ** the requested clusterid
    */
    for(cinfo=graph->clusterinfo;cinfo;cinfo=cinfo->next){
	if(cinfo->clusterid == clusterid){
	    return(cinfo->color);
	}
    }
    /*
    ** otherwise add a clusterinfo entry and assign it a color
    */
    color = MINCLUSTER + clusterid;
    SetClusterColor(graph,clusterid,color);
    return(color);
}

int32_t SetClusterColor(graph,clusterid,color)
Graph	*graph;
int32_t		clusterid;
int32_t		color;
{
ClusterInfo	*cinfo;

    clusterid = abs(clusterid);
    /*
    ** go through the clusterinfo to see whether an entry exists for
    ** the requested clusterid
    */
    for(cinfo=graph->clusterinfo;cinfo;cinfo=cinfo->next){
	if(cinfo->clusterid == clusterid){
	    /*
	    ** if so then use it
	    */
	    break;
	}
    }
    /*
    ** otherwise add a new one
    */
    if(cinfo == NULL){
	cinfo = (ClusterInfo *)calloc(1,sizeof(ClusterInfo));
	cinfo->next = graph->clusterinfo;
	graph->clusterinfo = cinfo;
	cinfo->clusterid = clusterid;
    }
    /*
    ** and give it the requested color
    */
    cinfo->color = color;
}

/* DrawAllClusterBounds(graph) */
/* Graph	*graph; */
/* { */
/* ClusterBounds	*cb; */

/*     for(cb=graph->clusterbounds;cb;cb=cb->next){ */
/* 	if(!cb->enabled) continue; */
/* 	if( (graph->currentprojection[0] == cb->projection[0] && */
/* 	   graph->currentprojection[1] == cb->projection[1]) || (graph->currentprojection[0] == cb->projection[1] && */
/* 	   graph->currentprojection[1] == cb->projection[0]) ){ */
	  
/* 	  SetColor(LookupClusterColor(graph,cb->clusterid)); */
/* 	  DrawClusterBounds(graph,cb); */

/* 	} */
/*     } */
/* } */

DrawAllClusterBounds(graph)
     Graph *graph;
{
ClusterBounds *cb;

    for(cb=graph->clusterbounds;cb;cb=cb->next){
	if(!cb->enabled) continue;
	if ( (cb->projection[0] == graph->currentprojection[0] || cb->projection[0] == graph->currentprojection[1] || cb->projection[0] == graph->currentprojection[2]) && 
	     (cb->projection[1] == graph->currentprojection[0] || cb->projection[1] == graph->currentprojection[1] || cb->projection[1] == graph->currentprojection[2]) ) {

	  SetColor(LookupClusterColor(graph,cb->clusterid));
	  DrawClusterBounds(graph,cb);
	  
	}
    }
}


DrawClusterBounds(graph,cb)
Graph	*graph;
ClusterBounds	*cb;
{
int32_t	xl,xr,yb,yt;
int32_t	x1,y1,x2,y2;

    if(graph == NULL || cb == NULL) return;
    if(cb->type == BOXBOUND){
	x1 = cb->coord[0].x;
	y1 = cb->coord[0].y;
	x2 = cb->coord[2].x;
	y2 = cb->coord[2].y;
	if(x1 > x2){
	    xl = x2;
	    xr = x1;
	} else {
	    xl = x1;
	    xr = x2;
	}
	if(y1 > y2){
	    yt = y1;
	    yb = y2;
	} else {
	    yt = y2;
	    yb = y1;
	}
	Box(xl,yb,xr,yt);
    } else {
	/*
	** draw the polygon
	*/
	MultipleLines(cb->coord,cb->ncoords);
	/*
	** and close it
	*/
	DrawLine(cb->coord[cb->ncoords-1].x,
	cb->coord[cb->ncoords-1].y,cb->coord[0].x,cb->coord[0].y);
    }
}

SetClusterRGB(clusterid,r,g,b)
int32_t	clusterid;
int32_t	r,g,b;
{

    SetPixelRGB(clusterid+MINCLUSTER,
    (int16_t)(r << 8),
    (int16_t)(g << 8),
    (int16_t)(b << 8));
}

