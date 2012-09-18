#include 	<stdio.h>
#include 	<math.h>
#include 	<stdlib.h>
#include	<X11/Xlib.h>
#include 	"header.h"
#include 	"xclust_defs.h"
#include 	"xclust_struct.h"

int32_t		debug;
Graph		*G;
Frame		*F;

char	*standardargv[1] = {"xclust3"};
int32_t	standardargc = 1;
int32_t	suppresswarnings;
