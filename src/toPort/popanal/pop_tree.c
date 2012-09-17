#include "pop_ext.h"

WriteTrajectoryTree(result,requested_node_id)
Result	*result;
int	requested_node_id;
{
FILE	*fpout;
TNode	*node;
TNode	*child;
char	line[10000];
int	offset;
TNode	root;
float	distance;
float	cumdistance;
int	x3,x2,y3,y2;
int	px,py;

    fpout = result->fptreeout;
    for(node = result->trajectory_tree;node;node=node->next){
	if(requested_node_id >=0 && node->id != requested_node_id){
	    continue;
	}
	if(result->hasspatialfiring){
	    if(node->id < 0){
		x3 = -1;
		y3 = -1;
	    } else {
		x3 = result->peakloc[node->id].x;
		y3 = result->peakloc[node->id].y;
	    }
	}
	if(result->hasspatialfiring && result->hasposition){
	    if(node->npositions > 0){
		px = node->px/node->npositions;
		py = node->py/node->npositions;
	    } else {
		px = -1;
		py = -1;
	    }
	} 
	switch(result->treeoutputformat){
	case TREE:
	    if(result->hasspatialfiring && result->hasposition){
		fprintf(fpout,"%d (%d) {%d,%d} <%d,%d>\n",
		    node->id,node->ntraversals,x3,y3,px,py);
	    } else
	    if(result->hasspatialfiring){
		fprintf(fpout,"%d (%d) {%d,%d}\n",
		    node->id,node->ntraversals,x3,y3);
	    } else {
		fprintf(fpout,"%d (%d)\n",
		    node->id,node->ntraversals);
	    }
	    break;
	case XPLOT:
	    fprintf(fpout,"/plotname %d\n",node->id);
	    if(result->hasspatialfiring && result->hasposition){
		sprintf(line,"%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
		    node->id,node->ntraversals,x3,y3,0,0,px,py,0,0);
	    } else
	    if(result->hasspatialfiring){
		sprintf(line,"%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
		    node->id,node->ntraversals,x3,y3,0,0,0);
	    } else {
		sprintf(line,"%d\t%d\t%d\n",
		    node->id,node->ntraversals,0);
	    }
	    break;
	case LIST:
	    sprintf(line,"%d:(%d) ",node->id,node->ntraversals);
	    offset = strlen(line);
	    break;
	}
	for(child = node->child;child;child=child->next){
	    if(result->hasspatialfiring){
		if(child->id < 0){
		    x2 = -1;
		    y2 = -1;
		} else {
		    x2 = result->peakloc[child->id].x;
		    y2 = result->peakloc[child->id].y;
		}
		if(x3 != -1 && x2 != -1){
		    distance = sqrt(
		    (double)((x3 -x2)*(x3 -x2) + (y3 - y2)*(y3 -y2)));
		} else {
		    distance = -1;
		}
		if(result->hasspatialfiring && result->hasposition){
		    /*
		    ** if position information is available then
		    ** compute cumulative distance from the difference
		    ** between current spike estimated location 
		    ** and the actual starting location
		    */
		    if(px != -1 && x2 != -1){
			cumdistance = sqrt(
			(double)((px -x2)*(px -x2) + (py - y2)*(py -y2)));
		    } else {
			cumdistance = -1;
		    }
		} else {
		    if(x3 != -1 && x2 != -1){
			cumdistance = sqrt(
			(double)((x3 -x2)*(x3 -x2) + (y3 - y2)*(y3 -y2)));
		    } else {
			cumdistance = -1;
		    }
		}
	    }
	    PrintTree(result,node,child,fpout,1,line,offset,distance,
	    cumdistance,x2,y2);
	}
    }
}

PrintTree(result,parent,node,fpout,depth,line,offset,distance,cumdistance,
nodex,nodey)
Result	*result;
TNode	*parent;
TNode	*node;
FILE	*fpout;
int	depth;
char	*line;
int	offset;
float	distance;
float	cumdistance;
int	nodex,nodey;
{
int	i;
TNode	*child;
char	tmpline[80];
int	newoffset;
int	x1,x3,x2,y1,y3,y2;
float	pdistance;
float	pcumdistance;
int	px,py;
int	px2,py2;

    if(node == NULL) {
	return;
    }
    if(result->hasposition && result->hasspatialfiring){ 
	if(node->id >= 0){
	    if(node->npositions > 0){
		px = node->px/node->npositions;
		py = node->py/node->npositions;
		pcumdistance = node->pcumdistance/node->npositions;
	    } else {
		px = -1;
		py = -1;
		pcumdistance = -1;
	    }
	    if(node->npositions > 0){
		px2 = parent->px/parent->npositions;
		py2 = parent->py/parent->npositions;
	    } else {
		px2 = -1;
		py2 = -1;
	    }
	    pdistance = sqrt((double)((px - px2)*(px - px2) + 
		(py - py2)*(py - py2)));
	} else {
	    px = -1;
	    py = -1;
	    px2 = -1;
	    py2 = -1;
	    pdistance = -1;
	    pcumdistance = -1;
	}
    }
    switch(result->treeoutputformat){
    case TREE:
	/*
	** display all of the children of the node
	*/
	for(i=0;i<depth;i++){
	    fprintf(fpout,"%6s"," ");
	}
	if(result->hasposition && result->hasspatialfiring && 
	node->id >= 0){
	    fprintf(fpout,"%d (%d) {%d,%d}[%g : %g]<%d,%d:%g>\n",
	    node->id,node->ntraversals,nodex,nodey,distance,pcumdistance,
	    px,py,pdistance);
	} else
	if(result->hasspatialfiring && node->id >= 0){
	    fprintf(fpout,"%d (%d) {%d,%d}[%g : %g]\n",
	    node->id,node->ntraversals,nodex,nodey,
	    distance,cumdistance);
	} else {
	    fprintf(fpout,"%d (%d)\n",node->id,node->ntraversals);
	}
	break;
    case XPLOT:
	if(result->hasposition && result->hasspatialfiring && 
	node->id >= 0){
	    sprintf(tmpline,"%d\t%d\t%d\t%d\t%g\t%g\t%d\t%d\t%g\t%d\n",
	    node->id,node->ntraversals,nodex,nodey,distance,pcumdistance,
	    px,py,pdistance,depth);
	} else
	if(result->hasspatialfiring && node->id >= 0){
	    sprintf(tmpline,"%d\t%d\t%d\t%d\t%g\t%g\t%d\n",
	    node->id,node->ntraversals,nodex,nodey,
	    distance,cumdistance,depth);
	} else 
	if(node->id >= 0){
	    sprintf(tmpline,"%d\t%d\t%d\n",node->id,node->ntraversals,depth);
	} else {
	    sprintf(tmpline,"\n");
	}
	bcopy(tmpline,line + offset,strlen(tmpline)+1);
	break;
    case LIST:
	sprintf(tmpline,"%d:(%d) ",node->id,node->ntraversals);
	bcopy(tmpline,line + offset,strlen(tmpline)+1);
	break;
    }
    if(node->child == NULL){
	/*
	** upon reaching a terminal branch print out the entire
	** traversal path. 
	*/
	/*
	** if xplot output has been requested then
	** print out the trajectory reconstructions
	*/
	switch(result->treeoutputformat){
	case XPLOT:
	    /*
	    fprintf(fpout,"/newplot\n");
	    fprintf(fpout,"/plotname %d\n",parent->id);
	    */
	    fprintf(fpout,"%s\n",line);
	    break;
	case LIST:
	    fprintf(fpout,"%s\n",line);
	    break;
	default:
	    break;
	}
	return;
    }
    newoffset = strlen(line);
    for(child = node->child;child;child=child->next){
	/*
	** if spatial firing distributions have been specified
	** then compute distance measures between the field peaks
	*/
	if(result->hasspatialfiring){
	    if(node->id < 0){
		x1 = -1;
		y1 = -1;
	    } else {
		x1 = result->peakloc[node->id].x;
		y1 = result->peakloc[node->id].y;
	    }
	    if(child->id < 0){
		x2 = -1;
		y2 = -1;
	    } else {
		x2 = result->peakloc[child->id].x;
		y2 = result->peakloc[child->id].y;
	    }
	    if(parent->id < 0){
		x3 = -1;
		y3 = -1;
	    } else {
		x3 = result->peakloc[parent->id].x;
		y3 = result->peakloc[parent->id].y;
	    }
	    if(x1 != -1 && x2 != -1){
		distance = sqrt(
		(double)((x1 -x2)*(x1 -x2) + (y1 - y2)*(y1 -y2)));
	    } else {
		distance = -1;
	    }
	    if(result->hasposition){
		/*
		** if position information is available then
		** compute cumulative distance from the difference
		** between current spike estimated location 
		** and the actual starting location
		*/
		if(x2 != -1 && px2 != -1){
		    cumdistance = sqrt(
		    (double)((px2 -x2)*(px2 -x2) + (py2 - y2)*(py2 -y2)));
		} else {
		    cumdistance = -1;
		}
	    } else {
		if(x2 != -1 && x3 != -1){
		    cumdistance = sqrt(
		    (double)((x3 -x2)*(x3 -x2) + (y3 - y2)*(y3 -y2)));
		} else {
		    cumdistance = -1;
		}
	    }
	}
	PrintTree(result,parent,child,fpout,depth+1,line,newoffset,
	distance,cumdistance,x2,y2);
    }
}

CreateTrajectoryTree(result,requested_node_id)
Result	*result;
int	requested_node_id;
{
int	i;	
TNode	tree;
TNode	*node;
int	bin;
int	nbad;
    
    /*
    ** create a root node for each component of the population
    ** vector
    */
    if(verbose){
	fprintf(stderr,"Working on component       ");
    }
    result->trajectory_tree = NULL;
    nbad = 0;
    for(i = 0;i < result->nclusters;i++){
	if(clusterdir[i].ignore) continue;
	if(requested_node_id > 0 && i != requested_node_id){
	    continue;
	}
	if(verbose){
	    fprintf(stderr,"\b\b\b\b\b%4d ",i);
	}
	if((node = (TNode *)calloc(1,sizeof(TNode))) == NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Cannot continue tree traversal\n");
	    exit(0);
	};
	node->id = i;
	if(result->trajectory_tree == NULL){
	    result->trajectory_tree = node;
	} else {
	    /*
	    ** insert it into the list
	    */
	    node->next = result->trajectory_tree->next;
	    result->trajectory_tree->next = node;
	}
	/*
	** now walk through the time series for the vector component
	*/
	for(bin=0;bin < result->ntimebins;bin++){
	    /*
	    ** wait until a non-zero entry is found then start the
	    ** tree traversal
	    */
	    if(result->datavector[bin][i] > result->traversal_threshold){
		if(result->hasposition){
		    /*
		    ** keep track of actual xy position if
		    ** available
		    ** convert from actual coords to grid coords
		    */
		    if(result->position[bin].x > 0){
			node->px += result->position[bin].x*
			(result->xmax+1)/result->xresolution;
			node->py += result->position[bin].y*
			(result->ymax+1)/result->yresolution;
			node->npositions++;
		    } else {
			nbad++;
		    }
		}
		node->ntraversals++;
		EvaluateNextBin(result,node,bin,bin+1);
	    }
	}
    }
    if(verbose){
	fprintf(stderr,"\nTraversal complete.\n");
    }
}

TNode	*GetChildNode(node,id)
TNode	*node;
int	id;
{
TNode	*child;

    if(node == NULL){
	return(NULL);
    }
    for(child = node->child;child;child=child->next){
	if(child->id == id){
	    /*
	    fprintf(stderr,"<%d>",id);
	    ** got it
	    */
	    break;
	}
    }
    if(child == NULL){
	/*
	** none defined so just allocate a new one
	** and insert it in the list
	*/
	if((child = (TNode *)calloc(1,sizeof(TNode))) == NULL){
	    fprintf(stderr,
	    "ERROR: Out of memory. Unable to continue tree traversal\n");
	    exit(0);
	}
	child->next = node->child;
	child->id = id;
	node->child = child;
	/*
	fprintf(stderr,"[%d]",id);
	*/
    }
    return(child);
}

EvaluateNextBin(result,node,startbin,bin)
Result	*result;
TNode	*node;
int	startbin;
int	bin;
{
int	i;
int	activebin;
TNode	*child;
float	cpx,cpy;
float	ppx,ppy;
float	cx,cy;

    /*
    ** check the bin to make sure the requested traversal
    ** depth has not been exceeded
    */
    if((bin - startbin >= result->maxtraversal) || (bin >= result->ntimebins)){
	return;
    }
    if(node == NULL){
	return;
    }
    /*
    ** go through each vector component
    */
    activebin = 0;
    for(i = 0;i < result->nclusters;i++){
	if(clusterdir[i].ignore) continue;
	/*
	** and look for any component that is active during the
	** desired bin
	*/
	if(result->datavector[bin][i] > result->traversal_threshold){
	    activebin = 1;
	    /*
	    ** find the node
	    */
	    child = GetChildNode(node,i);
	    /*
	    ** if it is active then bump the node and continue the traversal
	    ** one level deeper
	    */
	    child->ntraversals++;
	    if(result->hasposition){
		/*
		** keep track of actual xy position if
		** available
		*/
		if(result->position[bin].x > 0 &&
		result->position[startbin].x > 0 &&
		node->id >=0){
		    cx = result->peakloc[node->id].x;
		    cy = result->peakloc[node->id].y;
		    cpx = result->position[bin].x*(result->xmax+1)/result->xresolution;
		    cpy = result->position[bin].y*(result->ymax+1)/result->yresolution;
		    ppx = result->position[startbin].x*(result->xmax+1)/result->xresolution;
		    ppy = result->position[startbin].y*(result->ymax+1)/result->yresolution;
		    child->px += cpx;
		    child->py += cpy;
		    child->pcumdistance += sqrt((double)((cx - ppx)*(cx-ppx) + 
		    (cy - ppy)*(cy-ppy)));
		    child->npositions++;
		}
	    }
	    EvaluateNextBin(result,child,startbin,bin+1);
	}
    }
    if(activebin == 0){
	/*
	** add in a gap node
	*/
	child = GetChildNode(node,-1);
	child->ntraversals++;
	EvaluateNextBin(result,child,startbin,bin+1);
    }

}

