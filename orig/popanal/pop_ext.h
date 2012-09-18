#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include "header.h"
#include "iolib.h"
#include "pop_defs.h"
#include "pop_struct.h"

extern ClusterDir	clusterdir[MAXCLUSTERS];
extern ClusterDir	reconclusterdir[MAXCLUSTERS];
extern int verbose;
extern char	*strchr();
extern float	**FillGrid();
extern CorrResult	*ComputeCorrelation();
