/*
**************************************************
PROGRAM:
    editheader

DESCRIPTION:
    Edits the standard file header and writes out the new version
    with verification.

AUTHOR:
    Written by Matthew Wilson
    Massachussetts Institute of Technology
    wilson@ladyday.mit.edu

DATES:
    update 6/96

**************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iolib.h>

#include "header.h"

#define VERSION "1.04"

#define BLOCKSIZE 1024

#define DEFAULTEDITOR "/usr/bin/vi"

int verbose;

int main(argc,argv)
int	argc;
char 	**argv;
{
int	nxtarg = 0;
FILE	*fpout = NULL;
FILE	*fpin = NULL;
FILE	*fp = NULL;
int	fd;
int	i;
int	done;
char	*editorname;
char	*fnameout = NULL;
char	*fname = NULL;
char	tmpheader[10] = "eh-XXXXXX";
char	tmpfile[10] = "eh-XXXXXX";
char	**header;
long	headersize;
struct stat	buf;
time_t	start_mtime;
char	comstr[200];
int	nread;
int	nwrite;
char	buffer[BLOCKSIZE];
char	line[1000];
int	status;

    verbose = 0;

    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,"usage: %s file [-o file][-v]\n",argv[0]);
	    exit(-1);
	} else
	if(strcmp(argv[nxtarg],"-version") == 0){
	    fprintf(stderr,"%s : version %s : updated %s\n",argv[0],VERSION,DATE);

/* passed in from makefile when building an RPM*/
#ifdef MWSOFT_RPM_STRING 
	    fprintf(stderr,"From RPM: %s\n",
		    MWSOFT_RPM_STRING);
#endif

	    fprintf(stderr,"%s\n",COPYRIGHT);

	    exit(0);
	} else
	if(strcmp(argv[nxtarg],"-v") == 0){
	    verbose = 1;
	} else
	if(strcmp(argv[nxtarg],"-o") == 0){
	    fnameout = argv[++nxtarg];
	} else 
	if(argv[nxtarg][0] != '-'){
	    fname = argv[nxtarg];
	    if((fpin = fopen(argv[nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else {
	    fprintf(stderr,"invalid option '%s'\n",argv[nxtarg]);
	    exit(-1);
	}
    }
    if(fpin == NULL){
	fprintf(stderr,"Must specify a file to edit\n"); 
	exit(-1);
    }
    if(fnameout == NULL){
	/*
	** if no output file is specified then copy over the input file
	*/
	fnameout = fname;
    }
    if((editorname = getenv("EDITOR")) == NULL){
	editorname = DEFAULTEDITOR;
    }
    /*
    ** read in the header 
    */
    if((header = ReadHeader(fpin,&headersize)) == NULL){
	fprintf(stderr,"No header to edit\n");
	fprintf(stderr,"continue [y/n]?");
	fgets(line,1000,stdin);
	if(line[0] != 'y' && line[0] != 'Y'){
	    fprintf(stderr,"Edit aborted.\n");
	    exit(0);
	}
    }

    fd = -1;
    if((fd = mkstemp(tmpheader)) == -1 ||(fp = fdopen(fd, "w+")) == NULL) {
      fprintf(stderr,"unable to create temporary header filename\n");
      exit(-1);
    }

    /*
    ** copy the header to the temporary header file
    */
    if(header){
	for(i=0;;i++){
	    if(header[i] == NULL) break;
	    fprintf(fp,"%s\n",header[i]);
	}
    }
    fclose(fp);
    /*
    ** get the modification time
    */
    stat(tmpheader,&buf);
    start_mtime = buf.st_mtime;

    /*
    ** edit the temporary header
    */
    sprintf(comstr,"%s %s",editorname,tmpheader);
    system(comstr);
    /*
    ** see whether it was modified
    */
    stat(tmpheader,&buf);
    if(start_mtime == buf.st_mtime){
	/*
	** it was not modified
	*/
	fprintf(stderr,"Header not changed. Edit aborted.\n");
	unlink(tmpheader);
	exit(0);
    }
    /*
    ** create the new data file
    */
    /*
    ** create a temporary file
    */

    fd = -1;
    if((fd = mkstemp(tmpfile)) == -1 ||(fpout = fdopen(fd, "w+")) == NULL) {
      fprintf(stderr,"unable to create temporary data file\n");
      unlink(tmpheader);
      exit(-1);
    }
    /*
    ** write out the new header
    */
    fprintf(fpout,"%s\n",MAGIC_SOH_STR);
    /*
    ** read in the header from the edited temporary header file 
    */
    if((fp = fopen(tmpheader,"r")) == NULL){
	fprintf(stderr,"unable to read temporary header file\n");
	unlink(tmpfile);
	exit(-1);
    }
    /*
    ** copy the header to the temporary data file
    */
    while(!feof(fp)){
	if(fgets(line,1000,fp) == NULL) break;
	fprintf(fpout,"%s",line);
    }
    fprintf(fpout,"%s\n",MAGIC_EOH_STR);
    /*
    ** write out the data to the temporary data file
    */
    fseek(fpin,headersize,0L);
    done = 0;
    while(!done){
      if((nread = fread(buffer,sizeof(char),BLOCKSIZE,fpin)) != BLOCKSIZE){
	done = 1;
      }
      if((nwrite = fwrite(buffer,sizeof(char),nread,fpout)) != nread){
	fprintf(stderr,"error writing data file. Edit aborted\n");
	unlink(tmpheader);
	exit(-1);
      }
    }
    fclose(fpout);
    fclose(fpin);
    fclose(fp);
    /*
    ** get rid of the temporary header file
    */
    unlink(tmpheader);

    /*
    ** verify the new data file against the original data file
    */
    if(VerifyIdentical(fname,tmpfile) != 0){
      fprintf(stderr,"Data files not identical. Edit aborted.\n");
      unlink(tmpfile);
      exit(-1);
    }
    /*
    ** if replacing the original data file
    ** then get verification
    */
    if(strcmp(fnameout,fname) == 0) {
      fprintf(stderr,"Files verified identical beyond header\n");
      fprintf(stderr,"Replace original file with edited file [y/n]?");
      fgets(line,1000,stdin);
      if(line[0] != 'y' && line[0] != 'Y'){
	fprintf(stderr,"Edit aborted. Original file unchanged.\n");
	unlink(tmpfile);
	exit(0);
      }      
      fprintf(stderr,"OK. Replacing original file.\n");
    }
    /*
    ** move the new file to its destination
    */
    sprintf(comstr,"/bin/mv %s %s",tmpfile,fnameout);
    if(verbose){
	fprintf(stderr,"%s\n",comstr);
    }
    if((status = system(comstr)) != 0){
	perror("unable to replace old file");
	unlink(tmpfile);
	exit(status);
    }
    if(verbose){
	fprintf(stderr,"Old file replaced\n");
    }
    unlink(tmpfile);
    exit(0);
}

