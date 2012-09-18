#include <sys/types.h>
#include <sys/stat.h>
#include "xview_ext.h"

char	numstr[20];
static float plot_scale = 0.7;
static int psheader = 1;
int line_width = 1;

SetPSHeader(val)
int	val;
{
    psheader = val;
}

PrintToFile(filename,mode)
char *filename;
char *mode;
{
    /*
    ** create a postscript file
    */
    SetPSFileOutput(1);
    SetPSFilename(filename);
    SetPSFilemode(mode);
    PreparePS(G->display,G->imagewindow,plot_scale,0,psheader,G->wwidth,G->wheight);
    RefreshXview();
    /* DrawBorder(); */
    FinishPS();
    SetPSFileOutput(0);
}

PrintOut()
{
    PreparePS(G->display,G->imagewindow,plot_scale,0,psheader,G->wwidth,G->wheight);
    RefreshXview();
    /* DrawBorder(); */
    FinishPS();
}

BackStep()
{
    IncrementViewTime(-V->view_step);
    DisplayView();
}

ForwardStep()
{
    IncrementViewTime(V->view_step);
    DisplayView();
}

long	fsize(name)
char	*name;
{
struct stat stbuf;

    if(stat(name,&stbuf) == -1) {
	fprintf(stderr,"fsize: can't find %s\n",name);
	return;
    }
    if((stbuf.st_mode & S_IFMT) == S_IFDIR) {
	fprintf(stderr,"fsize: %s is a directory\n",name);
	return;
    }
    return(stbuf.st_size);
}

int LocateCell(x,y,cx,cy,cn)
int	x,y;
int	*cx,*cy,*cn;
{
    *cx = (x/(float)width)*(V->xmax+1);	
    *cy = ((yb + height - y)/(float)(height)) *(V->ymax+1);	
    *cn = *cy*(V->xmax+1)+*cx;
    if(*cn >= V->cellnum || *cn < 0){
	return(0);
    }
    return(1);
}

GraphCell(x,y)
int	x,y;
{
int	cell_x;
int	cell_y;
int	cell_n;
char	comstr[200];

    /*
    ** calculate the cell coordinates
    ** based on their location in the window
    */
    if(LocateCell(x,y,&cell_x,&cell_y,&cell_n)){
	/*
	** plot it
	sprintf(comstr,"x11simplot %s -c %d -a -dx %e &",
	fname,cell_n,V->dt);
	system(comstr);
	*/
	PlotData(cell_x,cell_y,cell_n);
    } else {
	Bell(0);
    }
}

DisplayLocation(x,y)
int	x,y;
{
int	cell_x;
int	cell_y;
int	cell_n;
char	comstr[200];

    /*
    ** calculate the cell coordinates
    ** based on their location in the window
    */
    if(LocateCell(x,y,&cell_x,&cell_y,&cell_n)){
	/*
	** print the coordinates and the data value
	** of the cell pointed to by the cursor
	*/
	if(fabs(data[cell_n]) < 0.1){
	    sprintf(comstr,"x=%d y=%d n=%d v=%7.1e       ",
	    cell_x,cell_y,cell_n,data[cell_n]);
	} else {
	    sprintf(comstr,"x=%d y=%d n=%d v=%7.2f       ",
	    cell_x,cell_y,cell_n,data[cell_n]);
	}
	SetColor(G->foreground);
	Text(G->fontwidth,G->fontheight*3,comstr);
    } else {
	Bell(0);
    }
}

KeyAction(event)
XKeyEvent	*event;
{
char	buffer[100];
int	nbytes;
char	c;
KeySym	key;
float	time;

    buffer[0] = '\0';
    /* 
    ** do key mapping to determine the actual key pressed
    */
    XLookupString(event,buffer,100, &key, NULL);
    c = *buffer;
    /*
    ** check for numeric strings
    */
    if((c >= '0' && c <='9') || (c == '.') || (c == '-') || (c =='e')){
	sprintf(numstr,"%s%c",numstr,c);
    } else{
	switch(c){
	case '':
	    /*
	    ** append to a postscript file
	    */
	    PrintToFile("xview.ps","a");
	    RefreshXview();
	    break;
	case '': 
	    Quit();
	    break;
	case '':
	    /*
	    ** erase the current command string
	    */
	    numstr[0]='\0';
	    break;
	case 'a':
	    do_autoscale(fp);
	    V->valid_frame = GetFrame(V->view_time);
	    NormalizeData();
	    RefreshXview();
	    break;
	/*
	** backup one timestep
	*/
	case 'b':
	    BackStep();
	    break;
	/*
	** go to a specific time step
	*/
	case 'c':
	    /*
	    ** set the number of contours
	    */
	    if(strlen(numstr) > 0){
		ncontours = atoi(numstr);
	    }
	    numstr[0]='\0';
	    NormalizeData();
	    RefreshXview();
	    break;
	case 'd':
	    DumpData();
	    break;
	case 'f':
	    /*
	    ** forward step
	    */
	    ForwardStep();
	    break;
	case 'g':
	    if(strlen(numstr) > 0)
		    time = atof(numstr);
	    else
		    time = 0;
	    numstr[0]='\0';
	    SetViewTime(time);
	    DisplayView();
	    break;
	case 'i':
	    if(strlen(numstr) > 0)
		    V->view_step = atof(numstr);
	    else
		    V->view_step = V->dt;
	    numstr[0]='\0';
	    break;
	case 'm':
	    /*
	    ** set the display mode
	    */
	    if(strlen(numstr) > 0)
		V->display_mode = atoi(numstr);
	    else
		V->display_mode = 0;
	    numstr[0]='\0';
	    RefreshXview();
	    break;
	case 'n':
	    /*
	    ** cycle through the titles displayed
	    ** in the legend
	    */
	    title_mode = (title_mode +1) % ntitles;
	    legend_background();
	    break;
	case 'o':
	    /*
	    ** use oriented lines as the 
	    ** display mode
	    */
	    if(strlen(numstr) > 0)
		orient = atoi(numstr);
	    else
		orient = 0;
	    numstr[0]='\0';
	    RefreshXview();
	    break;
	case 'p':
	    /*
	    ** plot the data corresponding to the cell
	    ** pointed to by the cursor
	    */
	    GraphCell(event->x,event->y);
	    break;
	case 'r':
	    /*
	    ** set the display representation
	    */
	    if(strlen(numstr) > 0)
		representation = atoi(numstr);
	    else
		representation = 0;
	    numstr[0]='\0';
	    RefreshXview();
	    break;
	case 's':
	    /*
	    ** toggle single stepping
	    */
	    V->singlestep = !V->singlestep;
	    break;
	case 't':
	    /*
	    ** output view time as a timestamp (100 usec units)
	    */
	    fprintf(stdout,"%d\n",(int)(V->view_time*10));
	    fprintf(stderr,"%d\n",(int)(V->view_time*10));
	    break;
	case 'w':
	    /*
	    ** set the animation speed
	    */
	    if(strlen(numstr) > 0)
		speed = atoi(numstr);
	    else
		speed = 0;
	    numstr[0]='\0';
	    RefreshXview();
	    break;
	case 'x':
	    /*
	    ** quit
	    */
	    Quit();
	    break;
	case 'z':
	    /*
	    ** set the z scale factor used in the 
	    ** surface display modes
	    */
	    if(strlen(numstr) > 0)
		zsize = atof(numstr);
	    else
		zsize = 0;
	    numstr[0]='\0';
	    RefreshXview();
	    break;
	case 'C':
	    /*
	    ** set the color mode
	    */
	    if(strlen(numstr) > 0){
		SetPSColor(atoi(numstr));
	    }
	    numstr[0]='\0';
	    break;
	case 'D':
	    DumpXviewData();
	    break;
	case 'F':
	    PrintToFile("xview.ps","w");
	    RefreshXview();
	    break;
	case 'L':
	    /*
	    ** set the line_width
	    */
	    if(strlen(numstr) > 0)
		line_width = atoi(numstr);
	    else
		line_width = 1;
	    numstr[0]='\0';
	    break;
	case 'P':
	    /*
	    ** do the postscript output at the desired scale
	    ** which blanks the screen
	    */
	    PrintOut();
	    /*
	    ** redisplay the screen
	    */
	    RefreshXview();
	    break;
	case 'R':
	    break;
	case 'S':
	    /*
	    ** set the plotting scale from 0-1
	    */
	    if(strlen(numstr) > 0)
		plot_scale = atof(numstr);
	    else
		plot_scale = 0.7;
	    numstr[0]='\0';
	    break;
	case 'Z':
	    /*
	    ** writes out the current color map
	    */
	    WriteColorMap("colormap");
	    break;
	case '+':
	    /*
	    ** set the rectification mode
	    */
	    if(strlen(numstr) > 0)
		posneg = atoi(numstr);
	    else
		posneg = 0;
	    numstr[0]='\0';
	    NormalizeData();
	    RefreshXview();
	    break;
	case ']':
	    /*
	    ** set the upper z bound 
	    */
	    if(strlen(numstr) > 0){
		if(atof(numstr) -minval != 0){
		    maxval = atof(numstr);
		    scale = maxval-minval;
		}
	    }
	    numstr[0]='\0';
	    NormalizeData();
	    RefreshXview();
	    break;
	case '[':
	    /*
	    ** set the lower z bound
	    */
	    if(strlen(numstr) > 0){
		if(maxval - atof(numstr) != 0){
		    minval = atof(numstr);
		    scale = maxval-minval;
		}
	    }
	    numstr[0]='\0';
	    NormalizeData();
	    RefreshXview();
	    break;
	}
    }
}

PlotData(x,y,n)
int x,y;
int n;
{
int	dnum;
float	fval;
float	val;
double	dval;
int	ival;
short	sval;
FILE	*tfp;
float	time;
char	comstr[100];
char	*tmpname;
char	*mktemp();
char	template[20];

    /*
    strcpy(template,"/tmp/xviewXXXXXX");
    tmpname = mktemp(template);
    */
    tmpname = "/tmp/xview.plot";
    if((tfp = fopen(tmpname,"w")) == NULL){
	fprintf(stderr,"unable to open temporary file\n");
	return;
    }
    fseek(fp,n*V->datasize+V->headersize,0);
    time = V->start_time;
    sprintf(comstr,"/graphtitle \"%s x=%d y=%d n=%d\"",
    V->filename,x,y,n);
    fprintf(tfp,"%s\n",comstr);
    while(!feof(fp)){
	switch(V->datatype){
	case FLOAT :
	    dnum = fread (&fval, V->datasize,1,fp);
	    val = fval;
	    break;
	case DOUBLE :
	    dnum = fread (&dval, V->datasize,1,fp);
	    val = dval;
	    break;
	case INT :
	    dnum = fread (&ival, V->datasize,1,fp);
	    val = ival;
	    break;
	case SHORT :
	    dnum = fread (&sval, V->datasize,1,fp);
	    val = sval;
	    break;
	}
	if(dnum <= 0) break;
	time += V->dt;
	fprintf(tfp,"%g %g\n",time,val);
	fseek (fp, (long)( (V->cellnum-1)*V->datasize), 1); 
    }
    fclose(tfp);
    sprintf(comstr,"xplot %s &",tmpname);
    system(comstr);
}

DumpData()
{
int	i;
FILE	*fp;
int	x,y;

    if((fp = fopen("xview.data","w")) == NULL){
	fprintf(stderr,"unable to open file xview.data\n");
	return;
    }
    for(i=0;i<V->cellnum;i++) {
	x = i%(V->xmax +1);
	y = i/(V->xmax+1);
	fprintf(fp,"%d %d %d %g\n",i,x,y,data[i]);
    }
    fclose(fp);
}

DumpXviewData()
{
int	i;
FILE	*fp;
int	ival;

    if((fp = fopen("xview.data","w")) == NULL){
	fprintf(stderr,"unable to open file xview.data\n");
	return;
    }
    ival = V->xmax;
    fwrite (&ival, sizeof (int),1,fp);
    ival = V->ymax;
    fwrite (&ival, sizeof (int),1,fp);
    fwrite (&V->dt, sizeof (float),1,fp);
    fwrite (&V->datatype, sizeof (int),1,fp);

    fwrite(data,sizeof(float),V->cellnum,fp);
    fclose(fp);
}
