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

extern int32_t		verbose;
extern uint32_t	starttime;
extern uint32_t	endtime;
extern int32_t		startspike;
extern int32_t		endspike;
extern int32_t		inf;
extern int32_t		min_rad;
extern int32_t		max_rad;
extern int32_t		max_consecutive_miss;
extern int32_t		maxlookahead;
extern int32_t		maxgaplen;

extern int32_t		datacompare();
extern int32_t		positioncompare();

/* functions defined in behav_alloc.c */

extern void AllocateArrays(Result *result);

/* functions defined in behav_ang.c */

extern int32_t DeltaDirection(int32_t d1,int32_t d2);
extern int32_t SignedDeltaDirection(int32_t d1,int32_t d2);
extern int32_t ComputeAngle(int32_t x1,int32_t y1,int32_t x2,int32_t y2);

/* functions defined in behav_bounds.c */

/* not called externally */
/* extern int32_t IsInPolygon(float wx,float wy, FCoord *fcoord,int32_t ncoords); */

extern int32_t CheckTimestampRange(uint32_t	timestamp,Result *result);
extern void ScaleClusterBounds(Result *result);
extern int32_t BoundsTest(Result *result, int32_t x,int32_t y);
extern int32_t RangeMapValue(Result *result,int32_t x,int32_t y,int32_t *mintheta,int32_t *maxtheta);
extern void ReadClusterBounds(Result *result,FILE *fp);

/* functions defined in behav_corr.c */

/* not called externally */
/* extern void SkipSpikes(SpikeList *spike,int32_t nclusters,uint32_t newptimestamp); */

extern void ComputeCorrelate(PositionList *plist,Result *result,SpikeList *spike,int32_t nclusters);

/* functions defined in behav_flip.c */

extern int32_t Flip(int32_t direction);
extern void EvaluateBehavior(Result *result,PositionList *plist);
extern int32_t CountFlips(int32_t j,int32_t testdirection,PositionList *plist,int32_t lookahead);
extern int32_t TestFlip(PositionList *plist,int32_t newdirection,int32_t j,int32_t lookahead);



/* functions defined in behav_func.c */

extern int32_t datacompare(SpikeData *s1,SpikeData *s2);
extern int32_t positioncompare(Position *p1,Position *p2);
extern void ResetVectors(Result *result);
extern void ComputeCoordinates(int32_t px,int32_t py,Result *result,int32_t *x,int32_t *y);

/* functions defined in behav_occ.c */

extern void ComputeOccupancy(Result *result,int32_t x,int32_t y,int32_t direction,uint32_t timestamp,int32_t gap);
extern double MeanOccupancyRate(Result *result);


/* functions defined in behav_read.c */
extern int32_t ReadAngularRangeMap(Result *result);
extern int32_t ReadRange(Result *result);
extern int32_t LoadPositions(PositionList *plist);
extern void LoadSpikes(Result *result,SpikeList *spike,int32_t timeshift);
extern void ScanSpikes(SpikeList *spike);
extern void LoadAndSortPositions(Result *result,PositionList *plist);
extern void LoadAndSortSpikes(Result *result,SpikeList *spikearray);

/* functions defined in behav_stat.c */

/* extern double ChiSquare(Result *result,double testmean,double *dchi, int32_t *nchi); */

extern void EvaluateSpatialFiring(Result *result,SpikeList *spikearray,int32_t cell);


/* functions defined in behav_write.c */

extern void WriteXviewHeader(Result *result);
extern void WriteBinaryHeader(Result *result);
extern void WriteGrid(Result *result);
extern void WriteVector(Result *result);
extern void PrintResults(Result *result,PositionList *plist);
extern void WritePositionFile(Result *result,uint32_t timestamp,int32_t x1,int32_t y1,int32_t x2,int32_t y2);
extern void WriteRateFile(Result *result,uint32_t timestamp,int32_t x,int32_t y,int32_t dir,float rate);
extern void OutputFileHeader(FILE *fp,Result *result,SpikeList *spikearray,char *filename,char *formatstr,int32_t argc,char **argv);
extern void PrepareOutputFiles(Result *result,SpikeList *spikearray,int32_t argc,char **argv);


