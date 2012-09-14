#include "xclust_ext.h"

static calcwxmin = 1;
static calcwymin = 1;
static calcwxmax = 1;
static calcwymax = 1;

SuppressWxmin()
{
    calcwxmin = 0;
}

SuppressWymin()
{
    calcwymin = 0;
}

SuppressWxmax()
{
    calcwxmax = 0;
}

SuppressWymax()
{
    calcwymax = 0;
}

RescaleGraph(graph)
Graph *graph;
{
Plot	*plot;
double val;

    /*
    ** move the current world view to slot one
    ** save the new world view in slot zero
    */
    graph->savedview[1] = graph->savedview[0];
    ProcessSavedView(graph,1,0);
    if((val = graph->wxmax - graph->wxmin) == 0){
	val = 1;
    }
    graph->wxscale = graph->wwidth/val;
    if((val = graph->wymax - graph->wymin) == 0){
	val = 1;
    }
    graph->wyscale = graph->wheight/val;
    for(plot=graph->plot;plot;plot=plot->next){
	if(plot->visible){
	    RescalePlot(plot);
	}
    }
    RescaleClusterBounds(graph);
    graph->changed = 1;
}

SetPlotVisibility(plot,val)
Plot	*plot;
int	val;
{
    if(plot == NULL) return;
    plot->visible = val;
    if(plot->visible){
	RescalePlot(plot);
    }
}

RescaleFrame(frame)
Frame	*frame;
{
XWindowAttributes	winfo;
Plot			*plot;
int			w,h;

    /*
    ** get the size of the frame
    */
    XGetWindowAttributes(frame->display,frame->window,&winfo);
    /*
    ** check to see whether the window size has changed
    */
    w = winfo.width;
    h = winfo.height;
    /*
    ** if it has then update the window parameters
    */
    if(w != frame->wwidth || h != frame->wheight){
	frame->wheight = winfo.height;
	frame->wwidth = winfo.width;

	/*
	** position the subwindows
	*/
	PositionSubwindows(frame);
	RescaleGraph(frame->graph);
	return(1);
    } 
    return(0);
}

int GetMenuWindowHeight(menu)
     MenuWindow *menu;
{
  MenuItem *item;
  int height = 0;
  Graph *graph;
  int clusterid, nbounds=0, nclusters=0;
  ClusterBounds *cb;
  ClusterList *clist;

  graph = F->graph;

  if (strcmp(menu->windowname, "/cluststat")==0) {
    clusterid = graph->selectedcluster;
    for(cb=graph->clusterbounds;cb;cb=cb->next) {
      if(cb->clusterid == clusterid)
	nbounds++;
    }
    height = 65 + 15 * nbounds;
    for(clist=graph->clusterlist;clist;clist=clist->next) {
      nclusters++;
    }
    if (height<(50 + 15 *nclusters))
      height = 50 + 15*nclusters;
  } else {
    for (item=menu->item; item; item=item->next) {
      if ( (item->y+item->h) > height)
	height = item->y+item->h;
    }
  }

  /* add additional space at bottom */
  if (height>0)
    height += 5;

  return height;
}

int GetMenuFrameHeight(menuframe)
     MenuFrame *menuframe;
{
  int height = 0;

  if (menuframe->expanded)
    height = GetMenuWindowHeight(menuframe->menu);

  if (menuframe->showtitle)
    height += 16;

  return height;

}

PositionSubwindows(frame)
Frame	*frame;
{
TextWindow		*text;
MenuContainer *menucontainer;
MenuWindow		*menu;
Graph			*graph;
int			text_height;
int			menu_width;
 int voffset;
 MenuFrame *menuframe;

    graph = frame->graph;
    text = frame->text;
    menucontainer = frame->menucontainer;
   /*  menu = frame->menu; */

    if(text->mapped){
	text_height = 2*frame->fontheight;
    } else {
	text_height = 0;
    }
    if(menucontainer->mapped){
	/* menu_width = 15*menu->fontwidth; */
	menu_width = MENU_WIDTH;
    } else {
	menu_width = 0;
    }
    if(graph->mapped){
	/*
	** adjust the graph window accordingly
	*/
	graph->wwidth = frame->wwidth - menu_width;
	graph->wheight = frame->wheight - text_height;
	if(graph->display){
	    XMoveResizeWindow(graph->display,graph->window,-1,-1,
	    graph->wwidth,graph->wheight);
	}
    }
    if(menucontainer->mapped){
	/*
	** adjust the menu window 
	*/
	menucontainer->wwidth = menu_width;
	menucontainer->wheight = frame->wheight;
	if(menucontainer->display){
	  XMoveResizeWindow(menucontainer->display,menucontainer->window,graph->wwidth,-1,
	    menucontainer->wwidth,menucontainer->wheight);
	}
	voffset = 0;
	/* loop through all menu frames */
	for (menuframe = menucontainer->menuframe; menuframe; menuframe = menuframe->next) {
	  if (menuframe->mapped) {
	    menuframe->wwidth = menu_width;
	    menuframe->wheight = GetMenuFrameHeight(menuframe);
	    if (menuframe->display) {
	      XMoveResizeWindow(menuframe->display,menuframe->window,0,voffset-1,
				menuframe->wwidth,menuframe->wheight);
	    }
	    if (menuframe->menu->mapped) {
	      if (menuframe->menu->display) {
		menuframe->menu->wwidth = menuframe->wwidth;
		menuframe->menu->wheight = GetMenuWindowHeight(menuframe->menu);
		XResizeWindow(menuframe->menu->display, menuframe->menu->window, menuframe->menu->wwidth, menuframe->menu->wheight);
	      }
	    }
	    voffset += menuframe->wheight;
	  }
	}
	
    }
/*     if(menu->mapped){ */
/* 	/\* */
/* 	** adjust the menu window  */
/* 	*\/ */
/* 	/\*menu->wwidth = menu_width;*\/ */
/* 	menu->wwidth = MENU_WIDTH; */
/* 	menu->wheight = frame->wheight; */
/* 	if(menu->display){ */
/* 	  /\*XMoveResizeWindow(menu->display,menu->window,graph->wwidth,-1, */
/* 	    menu->wwidth,menu->wheight);*\/ */
/* 	  XMoveResizeWindow(menu->display,menu->window,0,-1, */
/* 	    menu->wwidth,menu->wheight); */
/* 	} */
/*     } */
    if(text->mapped){
	/*
	** adjust the text window 
	*/
	text->wwidth = frame->wwidth - menu_width;
	text->wheight = text_height;
	if(text->display){
	    XMoveResizeWindow(text->display,text->window,-1,graph->wheight,
	    graph->wwidth,text_height);
	    /*
	    ** severe hack to position some text menu buttons
	    */
	    RepositionTextItems(text);
	}
    }

}


void FreeMatrix(double **matrix, int matsize)
{
  int i;

  if (!(matrix) || matsize<1)
    return;

    for (i=0; i<matsize; i++) {
      free( (matrix)[i] );
    }

    free(matrix);

    matrix=NULL;

}

MultiplyMatrix(double **m1, double **m2, double **retmat, int matsize)
{
 int i,j,k;

 if (!retmat || !m1 || !m2 || matsize<1)
   fprintf(stderr, "MultiplyMatrix: Null pointers!\n");

/*  for (i=0;i<4;i++) */
/*    for(j=0;j<4;j++) */
/*      fprintf(stderr, "i:%d, j:%d, %f  %f\n", i, j, m1[i][j], m2[i][j]); */


 for(i = 0; i < matsize; i++){  //Cycle through each vector of first matrix.
   for(j = 0; j < matsize; j++){
     for(k = 0; k < matsize; k++){

       retmat[i][j] += (m1[i][k] * m2[k][j]);
     }
   }
 }

/*  for (i=0;i<4;i++) */
/*    for(j=0;j<4;j++) */
/*      fprintf(stderr, "%f\n", retmat[i][j]); */

/*  for(i = 0; i < matsize; i++){  //Cycle through each vector of first matrix. */
/*    retmat[i][0] = m1[i][0] * m2[0][0] + */
/*      m1[i][1] * m2[1][0] + m1[i][2] * m2[2][0]; */
/*    retmat[i][1] = m1[i][0] * m2[0][1] + */
/*      m1[i][1] * m2[1][1] + m1[i][2] * m2[2][1]; */
/*    retmat[i][2] = m1[i][0] * m2[0][2] + */
/*      m1[i][1] * m2[1][2] + m1[i][2] * m2[2][2]; */
/*  } */

}

void AffineEmptyMatrix(double ***matrix)
{
  int i;

  FreeMatrix(*matrix, 4);

  /* allocate the matrix */
  *matrix = (double**)calloc(4, sizeof(double*));
  for (i=0; i<4; i++) {
    (*matrix)[i] = (double*)calloc(4, sizeof(double));
  }
}


void AffineIdentityMatrix(double ***matrix)
{
  int i;
  AffineEmptyMatrix(matrix);

  /* make it an identity affine matrix */

  for (i=0; i<4; i++)
    (*matrix)[i][i] = 1;

}

void AffineTranslationMatrix(double ***matrix, float offsetx, float offsety, float offsetz)
{

  double **tm;

  AffineIdentityMatrix(matrix);

  tm = *matrix;

  tm[0][3] = offsetx;
  tm[1][3] = offsety;
  tm[2][3] = offsetz;

}


void AffineRotationMatrixEuler(double ***matrix, float thetax, float thetay, float thetaz)
{
  double	Sx,Sy,Sz;
  double	Cx,Cy,Cz;
  double **tm;

  AffineIdentityMatrix(matrix);

  tm = *matrix;

  Sx = sin(thetax);
  Sy = sin(thetay);
  Sz = sin(thetaz);
  Cx = cos(thetax);
  Cy = cos(thetay);
  Cz = cos(thetaz);
  
  tm[0][0] = Cy*Cz;
  tm[1][0] = Sx*Sy*Cz - Cx*Sz;
  tm[2][0] = Sy*Cx*Cz + Sx*Sz;
  
  tm[0][1] = Cy*Sz;
  tm[1][1] = Sx*Sy*Sz + Cx*Cz;
  tm[2][1] = Sy*Cx*Sz - Sx*Cz;
  
  tm[0][2] = -Sy;
  tm[1][2] = Sx*Cy;
  tm[2][2] = Cx*Cy;

}

void AffineRotationMatrixAngleVector(double ***matrix, float vx, float vy, float vz, float angle)
{
  double **tm;
  double Sa, Ca, Ta;

/*   fprintf(stderr, "vx: %f, vy: %f, vz: %f, angle: %f\n", vx,vy,vz,angle); */

  AffineIdentityMatrix(matrix);

  tm = *matrix;

  Ca = cos (angle);
  Sa = sin (angle);
  Ta = 1-Ca;

  tm[0][0] = Ta*vx*vx + Ca;
  tm[1][0] = Ta*vx*vy + Sa*vz;
  tm[2][0] = Ta*vx*vz - Sa*vy;
  
  tm[0][1] = Ta*vx*vy - Sa*vz;
  tm[1][1] = Ta*vy*vy + Ca;
  tm[2][1] = Ta*vy*vz + Sa*vx;
  
  tm[0][2] = Ta*vx*vz + Sa*vy;
  tm[1][2] = Ta*vy*vz - Sa*vx;
  tm[2][2] = Ta*vz*vz + Ca;

}


void ApplyAffineTranslation(double ***matrix, float x, float y, float z)
{

  (*matrix)[3][0] += x;
  (*matrix)[3][1] += y;
  (*matrix)[3][2] += z;


}

double ***ApplyAffineRotation(double ***matrix, float vx, float vy, float vz, float angle)
{

  double **tm=NULL, **tm2=NULL;

  AffineEmptyMatrix(&tm);
  AffineRotationMatrixAngleVector(&tm2, (float) vx, (float) vy, (float)vz, (float)angle);

  MultiplyMatrix(*matrix, tm2,  tm, 4);

  FreeMatrix(*matrix,4);
  FreeMatrix(tm2,4);

  return &tm;

}

double ***AffineRotateRight(double ***matrix, float theta)
{
   /* Multiply the rotation matrix with a rotation around Right
      to get a new rotation matrix. */

   double **R=NULL;
   double **W=NULL;
   double c,s;

   AffineEmptyMatrix(&R);
   AffineEmptyMatrix(&W);

   c = cos(-theta);
   s = sin(-theta);

   R[0][0] = c;   R[1][0] = 0;   R[2][0]= s;   R[3][0] = 0;
   R[0][1] = 0;   R[1][1] = 1;   R[2][1] = 0; R[3][1] = 0;
   R[0][2] = -s;   R[1][2] = 0;   R[2][2]= c;   R[3][2]= 0;
   R[0][3] = 0;   R[1][3] = 0;   R[2][3]= 0;   R[3][3]= 1;

   MultiplyMatrix(R,*matrix,W,4);

   FreeMatrix(R,4);
   FreeMatrix(*matrix,4);
   return &W;

}

double ***AffineRotateUp(double ***matrix, float theta)
{
   /* Multiply the rotation matrix with a rotation around Right
      to get a new rotation matrix. */

   double **R=NULL;
   double **W=NULL;
   double c,s;

   AffineEmptyMatrix(&R);
   AffineEmptyMatrix(&W);

   c = cos(theta);
   s = sin(theta);

   R[0][0] = 1;   R[1][0] = 0;   R[2][0]= 0;   R[3][0] = 0;
   R[0][1] = 0;   R[1][1] = c;   R[2][1] = -s; R[3][1] = 0;
   R[0][2] = 0;   R[1][2] = s;   R[2][2]= c;   R[3][2]= 0;
   R[0][3] = 0;   R[1][3] = 0;   R[2][3]= 0;   R[3][3]= 1;

   MultiplyMatrix(R,*matrix,W,4);

   FreeMatrix(R,4);
   FreeMatrix(*matrix,4);

   return &W;

}

void SetRotationMatrix(double ***matrix, float thetax, float thetay, float thetaz)
{
double	Sx,Sy,Sz;
double	Cx,Cy,Cz;
int	i;
 double **tm;

 /*fprintf(stderr, "setrotmat: tx: %f, ty: %f, tz: %f\n", thetax, thetay, thetaz);*/

    /*
    ** check for the matrix
    */
    if(!(*matrix)){
	/*
	** allocate the matrix
	*/
	*matrix = (double **)malloc(3*sizeof(double *));
	for(i=0;i<3;i++){
	    (*matrix)[i] = (double *)malloc(3*sizeof(double));
	}
    }

    tm = *matrix;
    /*
    ** fill the matrix according to the current rotational parameters
    */
    Sx = sin(thetax);
    Sy = sin(thetay);
    Sz = sin(thetaz);
    Cx = cos(thetax);
    Cy = cos(thetay);
    Cz = cos(thetaz);

    tm[0][0] = Cy*Cz;
    tm[1][0] = Sx*Sy*Cz - Cx*Sz;
    tm[2][0] = Sy*Cx*Cz + Sx*Sz;

    tm[0][1] = Cy*Sz;
    tm[1][1] = Sx*Sy*Sz + Cx*Cz;
    tm[2][1] = Sy*Cx*Sz - Sx*Cz;

    tm[0][2] = -Sy;
    tm[1][2] = Sx*Cy;
    tm[2][2] = Cx*Cy;
}


/* void UpdateRotationMatrix(double ***matrix, float thetax, float thetay, float thetaz) */
/* { */
/*   int i; */
/*   double **rotmat; */
/*   double **tempmat; */
  
/*   if (debug) */
/*     fprintf(stderr, "tx: %f,ty: %f,tz: %f \n", thetax, thetay, thetaz); */

/*   rotmat = (double **)malloc(3*sizeof(double *)); */
/*   for(i=0;i<3;i++) { */
/*     rotmat[i] = (double *)malloc(3*sizeof(double)); */
/*   } */
	    
/*   SetRotationMatrix(&rotmat, thetax, thetay, thetaz); */
  
/*   if (!(*matrix)) */
/*     SetRotationMatrix(matrix, 0.0, 0.0, 0.0); */
  
/*   tempmat = (double **)malloc(3*sizeof(double *)); */
/*   for(i=0;i<3;i++) { */
/*     tempmat[i] = (double *)malloc(3*sizeof(double)); */
/*   }   */

/*   MultiplyMatrix(*matrix, rotmat, tempmat); */

/*   /\*free *matrix*\/ */
/*   for(i=0;i<3;i++) { */
/*     free( (*matrix)[i] ); */
/*   } */

/*   free(*matrix); */

/*   *matrix = tempmat; */

/* } */

MatrixTransform(basic,x,y,z,nx,ny,nz)
BasicWindow	*basic;
double	x,y,z;
double	*nx,*ny,*nz;
{

  double **m;

   if((basic == NULL) || (m = basic->matrix) == NULL){
	*nx = x;
	*ny = y;
	*nz = z;
    } else {
      *nx = m[0][0]*x + m[1][0]*y + m[2][0]*z + m[3][0];
      *ny = m[0][1]*x + m[1][1]*y + m[2][1]*z + m[3][1];
      *nz = m[0][2]*x + m[1][2]*y + m[2][2]*z + m[3][2];
    }
    /*
    ** apply disparity based on z
    */
/*     *nx += *nz*basic->disparityscale; */
}

/* MatrixTransform(basic,x,y,z,nx,ny,nz) */
/* BasicWindow	*basic; */
/* double	x,y,z; */
/* double	*nx,*ny,*nz; */
/* { */
/* double	**m; */

/*     if((basic == NULL) || (m = basic->matrix) == NULL){ */
/* 	*nx = x; */
/* 	*ny = y; */
/* 	*nz = z; */
/*     } else { */
/* 	*nx = m[0][0]*x + m[1][0]*y + m[2][0]*z; */
/* 	*ny = m[0][1]*x + m[1][1]*y + m[2][1]*z; */
/* 	*nz = m[0][2]*x + m[1][2]*y + m[2][2]*z; */
/*     } */
/*     /\* */
/*     ** apply disparity based on z */
/*     *\/ */
/*     *nx += *nz*basic->disparityscale; */
/* } */

GetDataPoint(plot,wx,wy,wz,index)
Plot	*plot;
float	*wx,*wy,*wz;
int	index;
{
double	x,y,z;
double	p0val,p1val,p2val;
int	p0,p1,p2;
ProjectionInfo	*pinfo;

    /*
    ** get original data points
    */
    p0 = plot->graph->currentprojection[0];
    p1 = plot->graph->currentprojection[1];
    p2 = plot->graph->currentprojection[2];

    if((pinfo=GetProjectionInfo(plot->graph,p0)) == NULL){
	fprintf(stderr,"ERROR: unable to find projection %d\n",p0);
	return;
    }
    p0val = GetDoubleFromRawData(plot->rawdata[index].data,pinfo);
    if((pinfo=GetProjectionInfo(plot->graph,p1)) != NULL){
	p1val = GetDoubleFromRawData(plot->rawdata[index].data,pinfo);
    } else {
	p1val = 0;
    }
    if((pinfo=GetProjectionInfo(plot->graph,p2)) != NULL){
	p2val = GetDoubleFromRawData(plot->rawdata[index].data,pinfo);
    } else {
	p2val = 0;
    }

    if(p0 >= 0 && p0 < plot->ndatafields + plot->auto_x){
	x = p0val*plot->xscale + plot->xoffset;
    } else {
	x = plot->xoffset;
    }
    if(p1 >= 0 && p1 < plot->ndatafields + plot->auto_x) {
	y = p1val*plot->scale + plot->yoffset;
    } else {
	y = plot->yoffset;
    }
#ifdef OLD
    if((plot->zmode > 0)  && (p2 >= 0) && (p2 < plot->ndatafields)){
	z = p2val*plot->zscale;
    } else {
	z = 0;
    }
#else
    if((p2 >= 0) && (p2 < plot->ndatafields + plot->auto_x)){
	z = p2val*plot->zscale + plot->zoffset;
    } else {
	z = 0;
    }
#endif
    /*
    ** adjust for axis scaling and offsets
    */
    switch(plot->graph->xaxis.type){
    case LINEAR_AXIS:
	*wx = x + plot->xoffset_axis;
	break;
    case LOG10_AXIS:
	*wx = log10(x) + plot->xoffset_axis;
	break;
    }
    switch(plot->graph->yaxis.type){
    case LINEAR_AXIS:
	*wy = y + plot->yoffset_axis;
	break;
    case LOG10_AXIS:
	*wy = log10(y) + plot->yoffset_axis;
	break;
    }
    switch(plot->graph->zaxis.type){
    case LINEAR_AXIS:
	*wz = z + plot->zoffset_axis;
	break;
    case LOG10_AXIS:
	*wz = log10(z) + plot->zoffset_axis;
	break;
    }
}

GetTransformedDataPoint(plot,wx,wy,wz,index)
Plot	*plot;
float	*wx,*wy,*wz;
int	index;
{
double	nx,ny,nz;

    /*
    ** get the original data points
    */
    GetDataPoint(plot,wx,wy,wz,index);
    /*
    ** apply transformations
    */
    MatrixTransform(plot->graph,(double)*wx,(double)*wy,(double)*wz,
    &nx,&ny,&nz);
    /*
    ** and assign the new values
    */
    *wx = nx;
    *wy = ny;
    *wz = nz;
}

RescalePlot(plot)
Plot	*plot;
{
int 	i;
float	sx,sy;
float	wx,wy,wz;
Coord	*coord;
FCoord	*fcoord;
int	randomizex;
int	randomizey;
 int count;

    if(plot == NULL) return;
    /*
    ** transform each data coordinate into a screen coordinate
    */
    coord = plot->coord;
    fcoord = plot->fcoord;
    if(coord == NULL || fcoord == NULL) return;
    coord += plot->xlo;
    fcoord += plot->xlo;
    if(plot->graph){
      randomizex = (GetProjectionInfo(G,G->currentprojection[0]))->randomize;
      randomizey = (GetProjectionInfo(G,G->currentprojection[1]))->randomize;
    } else {
	randomizex = 0;
	randomizey = 0;
    }
    count = 0;

    for(i=plot->xlo;i<=plot->xhi;i+=plot->pointfreq){
      if(plot->graph->hiddencluster[abs(plot->rawdata[i].clusterid)]) {
	/*
	** ignore hidden clusters
	*/
	continue;
      }

      /*if (i%plot->pointfreq == 0) {*/
	GetTransformedDataPoint(plot,&wx,&wy,&wz,i);
	/*
	** if randomization is selected then add a value
	*/
	if(randomizex){
	  wx += frandom(-0.5,0.5);
	}
	if(randomizey){
	  wy += frandom(-0.5,0.5);
	}
	if(count==0){
	  plot->xmin = plot->xmax = wx;
	  plot->ymin = plot->ymax = wy;
	  plot->zmin = plot->zmax = wz;
	} else {
	  if(wx < plot->xmin) plot->xmin = wx;
	  if(wx > plot->xmax) plot->xmax = wx;
	  if(wy < plot->ymin) plot->ymin = wy;
	  if(wy > plot->ymax) plot->ymax = wy;
	  if(wz < plot->zmin) plot->zmin = wz;
	  if(wz > plot->zmax) plot->zmax = wz;
	}
	FloatScreenTransform(plot->graph, wx,wy, &sx,&sy);
	coord[i].x = sx;
	coord[i].y = sy;
	fcoord[i].x = sx;
	fcoord[i].y = sy;
	/*}
      coord++;
      fcoord++;*/
	count++;
    }

}

/*
** transform world coordinates x,y into screen coordinate sx,sy
*/
void ScreenTransform(graph,wx,wy,sx,sy)
Graph	*graph;
double	wx,wy;
int	*sx,*sy;
{


    *sx = (wx - graph->wxmin)*graph->wxscale;
    if(*sx > 10000) *sx = 10000;
    if(*sx < -10000) *sx = -10000;
    /*
    ** invert the screen coordinates so that the top of the window is up
    */
    *sy = graph->wheight - (wy - graph->wymin)*graph->wyscale;
    if(*sy > 10000) *sy = 10000;
    if(*sy < -10000) *sy = -10000;
}

/*
** transform world coordinates x,y into screen coordinate sx,sy
*/
FloatScreenTransform(graph,wx,wy,sx,sy)
Graph	*graph;
double	wx,wy;
float	*sx,*sy;
{

    *sx = (wx - graph->wxmin)*graph->wxscale;
    /*
    ** try to keep X from losing its shorts over large integers
    */
    if(*sx > 10000) *sx = 10000;
    if(*sx < -10000) *sx = -10000;
    /*
    ** invert the screen coordinates so that the top of the window is up
    */
    *sy = graph->wheight - (wy - graph->wymin)*graph->wyscale;
    if(*sy > 10000) *sy = 10000;
    if(*sy < -10000) *sy = -10000;
}

/*
** transform screen coordinates sx,sy into world coordinate wx,wy
*/
WorldTransform(graph,sx,sy,wx,wy)
Graph *graph;
int sx,sy;
float *wx,*wy;
{
    if((graph->wxscale == 0) || (graph->wyscale == 0)){
	*wx = 0;
	*wy = 0;
    }
    /*
    ** do the inverse transform from screen coords to world coords
    */
    *wx = sx/graph->wxscale +graph->wxmin;
    /*
    ** invert the y screen coord before transforming
    */
    *wy = (graph->wheight - sy)/graph->wyscale +graph->wymin;
}

AutoScale(graph,autoscale_x,autoscale_y)
Graph *graph;
int autoscale_x,autoscale_y;
{
int	cnt;
float	val;
Plot	*plot;

    if(autoscale_x){
	cnt = 0;
	for(plot=graph->plot;plot;plot=plot->next){
	    if(plot->visible){
		if(cnt++ < 1){
		    graph->xmax = plot->xmax;
		    graph->xmin = plot->xmin;
		} else {
		    if(plot->xmin < graph->xmin) graph->xmin = plot->xmin;
		    if(plot->xmax > graph->xmax) graph->xmax = plot->xmax;
		}
	    }
	}
    }
    if(autoscale_y){
	cnt = 0;
	for(plot=graph->plot;plot;plot=plot->next){
	    if(plot->visible){
		if(cnt++ < 1){
		    graph->ymax = plot->ymax;
		    graph->ymin = plot->ymin;
		} else {
		    if(plot->ymin < graph->ymin) graph->ymin = plot->ymin;
		    if(plot->ymax > graph->ymax) graph->ymax = plot->ymax;
		}
	    }
	}
    }
    /*
    ** determine the world coordinate bounds of the window
    ** assume that the data area will take up a fraction of the window
    */
    val = graph->xmax - graph->xmin;
    if(val ==0) val=1;
    if(calcwxmin && autoscale_x){
	graph->wxmin = graph->xmin - 0.10*val;
    }
    if(calcwxmax && autoscale_x){
	graph->wxmax = graph->xmax + 0.02*val;
    }

    val = graph->ymax - graph->ymin;
    if(val ==0) val=1;
    if(calcwymin && autoscale_y){
	graph->wymin = graph->ymin - 0.10*val;
    }
    if(calcwymax && autoscale_y){
	graph->wymax = graph->ymax + 0.02*val;
    }

    calcwxmin = 1;
    calcwymin = 1;
    calcwxmax = 1;
    calcwymax = 1;
}

AutoOrigin(graph)
Graph *graph;
{
int sw,sh;
int tw,th;
float ww,wh;
char label[80];
int i;

    /*
    ** autolocate the origin within the current window
    ** make enough room to fit the labels
    */
    label[0] = '\0';
    for(i=0;i<graph->yaxis.rightdp + graph->yaxis.leftdp +2;i++){
	strcat(label,"0");
    }
    TextExtent(label,&tw,&th);
    sh = graph->wheight - th*3.5;
    sw = tw;

    WorldTransform(graph,sw,sh,&ww,&wh);
    graph->yaxis.xintcpt = ww;
    graph->xaxis.yintcpt = wh;
}

