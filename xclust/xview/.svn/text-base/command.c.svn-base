#include "xview_ext.h"

extern char *strchr();

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

/* 
** return an argument string delimited by white space or by quotes
*/
char *CommandArgument(line,argnumber)
char *line;
int argnumber;
{
int	i;
int	count;
char	*ptr;
char	*ptr2;
char	*copy;
char	tmp[1000];
int	quote;

    ptr = line;
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
	** advance past the open delimiter
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
	copy = (char *)malloc((count+1)*sizeof(char));
	strncpy(copy,ptr,count);
	copy[count] = '\0';
    } else {
	copy = NULL;
	fprintf(stderr,"missing argument #%d\n",argnumber);
    }
    return(copy);
}

#define Match(S) (strncmp(line,S,strlen(S)) == 0)
#define	ARG(N)	CommandArgument(line,N)

/*
** interpret / commands
*/
InterpretCommand(line)
char *line;
{
int	val;
    if(Match("/setenv")){
	Setenv(ARG(1),ARG(2));
    } else
    if(Match("/color")){
	SetPSColor(Atoi(ARG(1)));
    } else
    if(Match("/psheader")){
	SetPSHeader(Atoi(ARG(1)));
    } else
    if(Match("/mode")){
	V->display_mode = Atoi(ARG(1));
    } else
    if(Match("/forward")){
	ForwardStep();
    } else
    if(Match("/back")){
	BackStep();
    } else
    if(Match("/quit")){
	Quit();
    } else
    if(Match("/printtofile")){
	PrintToFile(ARG(1),"w");
    } else
    if(Match("/appendtofile")){
	PrintToFile(ARG(1),"a");
    } else
    if(Match("/wait")){
	sleep(Atoi(ARG(1)));
    } else
    if(Match("/print")){
	PrintOut();
    } else
    if(Match("/help")){
	Help();
    } else {
	if(line[0] != '\0'){
	    fprintf(stderr,"xview: unrecognized command : %s\n",line);
	}
    }
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
    HP("color [0/1]");
    HP("mode [0-9]");
    HP("printtofile [file]");
    HP("appendtofile [file]");
    HP("print");
    HP("setenv [var value]");
    HP("geometry [str]");
    HP("wait [sec]");
    HP("psheader [0/1]");
    HP("quit");
    fprintf(stderr,"\n");
}
