#include "xview_ext.h"

GetaRecord()
{
int	i;
char	*charptr;
short	*shortptr;
int	*intptr;
float	*floatptr;
double	*doubleptr;
unsigned char nitems;
unsigned char frame;
unsigned long timestamp;
short xval;
unsigned char yval;

    if(V->posdata){
	/*
	** read in nitems
	*/
	if(fread (&nitems, sizeof(unsigned char),1,fp) != 1){
	    return(0);
	}
	/*
	** read in frame
	*/
	if(fread (&frame, sizeof(unsigned char),1,fp) != 1){
	    return(0);
	}
	/*
	** read in timestamp
	*/
	if(fread (&timestamp, sizeof(unsigned long),1,fp) != 1){
	    return(0);
	}
	bzero(data,sizeof(float)*V->cellnum);
	for(i=0;i<nitems;i++){
	    /*
	    ** read in timestamp
	    */
	    if(fread (&xval, sizeof(short),1,fp) != 1){
		return(0);
	    }
	    if(fread (&yval, sizeof(unsigned char),1,fp) != 1){
		return(0);
	    }
	    if((xval > V->xmax + 1) || (yval > V->ymax + 1)){
		fprintf(stderr,"invalid coordinate %d %d\n",(int)xval, (int)yval);
		continue;
	    }
	    data[xval + yval*(V->xmax +1)] = 1;
	}
	return(1);
    }
    if(fread ((char *)tmpdata, V->datasize,V->cellnum,fp) != V->cellnum){ 
	return(0);
    }
    switch(V->datatype){
    case CHAR :
	charptr = (char *)tmpdata;
	for(i=0;i<V->cellnum;i++){
	    curdata[i] = (float)(charptr[i]);
	}
	break;
    case INT :
	intptr = (int *)tmpdata;
	for(i=0;i<V->cellnum;i++){
	    curdata[i] = (float)intptr[i];
	}
	break;
    case SHORT :
	shortptr = (short *)tmpdata;
	for(i=0;i<V->cellnum;i++){
	    curdata[i] = (float)shortptr[i];
	}
	break;
    case FLOAT :
	for(i=0;i<V->cellnum;i++){
	    curdata[i] = (float)tmpdata[i];
	}
	break;
    case DOUBLE :
	doubleptr = (double *)tmpdata;
	for(i=0;i<V->cellnum;i++){
	    curdata[i] = (float)doubleptr[i];
	}
	break;
    default:
	return(0);
    }
    /*
    ** transfer the type specific data into the generic
    ** data array
    */
    if(cumulative ){
	if( !backup)
	for(i=0;i<V->cellnum;i++)
	    data[i] += curdata[i];
    } else {
	for(i=0;i<V->cellnum;i++)
	    data[i] = curdata[i];
    }
    return(1);
}

#define MAX(x,y) ((x < y) ? y : x)
#define MIN(x,y) ((x < y) ? x : y)

NormalizeData()
{
float	val;
float	lower,upper;
int	i;

    imageptr = image;
    for (i = 0; i < V->cellnum; i++) {
	/* 
	* normalize the data using the scale factor 
	*/
	if(posneg){
	    if(data[i] >= 0){
		lower = MAX(minval,0);
		val = data[i] / (maxval - lower); 
		imageptr->sign = 1;
	    } else {
		upper = MIN(maxval,0);
		val = -data[i] / (upper - minval); 
		imageptr->sign = -1;
	    }
	    /* 
	    * enforce upper and lower bounds 
	    */
	    val = MAX(val,0);
	    val = MIN(val,1);
	} else {
	    val = (data[i] - minval) / scale; 

	    /* 
	    * enforce upper and lower bounds 
	    */
	    val = MAX(val,0);
	    val = MIN(val,1);
	    imageptr->sign = 1;
	}
	imageptr->value = val;
	imageptr++;
    }
}

LoadNextFrame(){
    /*
    ** read in the next record
    */
    if(!GetaRecord()){
	V->singlestep = TRUE;
    }
    /*
    ** convert the data into a normalized form
    */
    NormalizeData();
}

int GetFrame(time)
float	time;
{
int	record;

    if(time < V->start_time){
	V->singlestep = TRUE;
	return(0);
    }
    /*
    ** find the location in the data file of the specified time
    */
    record = (time - V->start_time)/V->dt +.5;
    if(!V->posdata){
	fseek (fp, (long) (V->headersize + V->cellnum*record*V->datasize), 0L);
    }
    /*
    ** read in the record
    */
    if(!GetaRecord()){
	V->singlestep = TRUE;
	return(0);
    }
    /*
    ** convert the data into a normalized form
    */
    NormalizeData();
    return(1);
}

ReadCommandFile(source,filename)
int	source;
char	*filename;
{
FILE	*fp;
char	line[1001];

    if(source == 0){
	/*
	** open the data file associated with the plot
	*/
	if(strcmp(filename,"STDIN") == 0){
	    fp = stdin;
	} else
	if((fp = fopen(filename,"r")) == NULL){
	    printf("cant open file '%s'\n",filename);
	    return(0);
	}
	/*
	** read the data source file
	*/
	while(!feof(fp)){
	    /*
	    ** read a line from the data file
	    */
	    if(fgets(line,1000,fp) == NULL) {
		break;
	    }

	    line[1000] = '\0';
	    DoCommand(line);
	}
	fclose(fp);
    } else {
	DoCommand(filename);
    }
}

DoCommand(lineptr)
char *lineptr;
{
char *strchr();

    do {
	if(lineptr[0] == ';'){
	    lineptr++;
	    /* skip white space */
	    while(lineptr && *lineptr == ' '){
		if(*lineptr == '\0' || *lineptr == '\n') break;
		lineptr++;
	    }
	}
	/*
	** check to see if it is a plot command
	*/
	if(lineptr[0] == '/'){
	    InterpretCommand(lineptr);
	    RefreshXview();
	} 
	if(*lineptr == '\0') break;
    } while(lineptr = strchr(lineptr+1,';'));
}
