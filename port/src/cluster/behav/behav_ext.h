#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>

/* MWL libs */
#include <header.h>
#include <iolib.h>

#include "behav_defs.h"
#include "behav_struct.h"

/*
*************************************
**            GLOBALS              **
*************************************
*/

extern int		verbose;
extern unsigned long	starttime;
extern unsigned long	endtime;
extern int		startspike;
extern int		endspike;
extern int		inf;
extern int		min_rad;
extern int		max_rad;
extern int		max_consecutive_miss;
extern int		maxlookahead;
extern int		maxgaplen;

extern int		datacompare();
extern int		positioncompare();

/* functions defined in behav_alloc.c */

extern void AllocateArrays(Result *result);

/* functions defined in behav_ang.c */

extern int DeltaDirection(int d1,int d2);
extern int SignedDeltaDirection(int d1,int d2);
extern int ComputeAngle(int x1,int y1,int x2,int y2);

/* functions defined in behav_bounds.c */

/* not called externally */
/* extern int IsInPolygon(float wx,float wy, FCoord *fcoord,int ncoords); */

extern int CheckTimestampRange(unsigned long	timestamp,Result *result);
extern void ScaleClusterBounds(Result *result);
extern int BoundsTest(Result *result, int x,int y);
extern int RangeMapValue(Result *result,int x,int y,int *mintheta,int *maxtheta);
extern void ReadClusterBounds(Result *result,FILE *fp);

/* functions defined in behav_corr.c */

/* not called externally */
/* extern void SkipSpikes(SpikeList *spike,int nclusters,unsigned long newptimestamp); */

extern void ComputeCorrelate(PositionList *plist,Result *result,SpikeList *spike,int nclusters);

/* functions defined in behav_flip.c */

extern int Flip(int direction);
extern void EvaluateBehavior(Result *result,PositionList *plist);
extern int CountFlips(int j,int testdirection,PositionList *plist,int lookahead);
extern int TestFlip(PositionList *plist,int newdirection,int j,int lookahead);



/* functions defined in behav_func.c */

extern int datacompare(SpikeData *s1,SpikeData *s2);
extern int positioncompare(Position *p1,Position *p2);
extern void ResetVectors(Result *result);
extern void ComputeCoordinates(int px,int py,Result *result,int *x,int *y);

/* functions defined in behav_occ.c */

extern void ComputeOccupancy(Result *result,int x,int y,int direction,unsigned long timestamp,int gap);
extern double MeanOccupancyRate(Result *result);


/* functions defined in behav_read.c */
extern int ReadAngularRangeMap(Result *result);
extern int ReadRange(Result *result);
extern int LoadPositions(PositionList *plist);
extern void LoadSpikes(Result *result,SpikeList *spike,int timeshift);
extern void ScanSpikes(SpikeList *spike);
extern void LoadAndSortPositions(Result *result,PositionList *plist);
extern void LoadAndSortSpikes(Result *result,SpikeList *spikearray);

/* functions defined in behav_stat.c */

/* extern double ChiSquare(Result *result,double testmean,double *dchi, int *nchi); */

extern void EvaluateSpatialFiring(Result *result,SpikeList *spikearray,int cell);


/* functions defined in behav_write.c */

extern void WriteXviewHeader(Result *result);
extern void WriteBinaryHeader(Result *result);
extern void WriteGrid(Result *result);
extern void WriteVector(Result *result);
extern void PrintResults(Result *result,PositionList *plist);
extern void WritePositionFile(Result *result,unsigned long timestamp,int x1,int y1,int x2,int y2);
extern void WriteRateFile(Result *result,unsigned long timestamp,int x,int y,int dir,float rate);
extern void OutputFileHeader(FILE *fp,Result *result,SpikeList *spikearray,char *filename,char *formatstr,int argc,char **argv);
extern void PrepareOutputFiles(Result *result,SpikeList *spikearray,int argc,char **argv);


