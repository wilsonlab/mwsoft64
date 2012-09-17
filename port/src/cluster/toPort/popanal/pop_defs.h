#define VERSION "2.82"
#define MAXCLUSTERS	1000
#define MAXTIMEBINS	3000000
#define POSITION_INTERVAL	33e-3
#define MAXRANGES 10000

#define TREE	0
#define LIST	1
#define XPLOT	2

#define OPTIMIZE_ANGLE	0
#define OPTIMIZE_DISTANCE	1

#define TINYVAL 1e-30

#define ERRORPHS	-999
#define YPOSITION_RESOLUTION	256.0
#define XPOSITION_RESOLUTION	364.0

/*
** 2.77		added min_totalcorr_events option to ignore corr histograms
**		that have less than a total number of specified events
*/
/*
** 2.78		added spike train smoothing
**		the SmoothSpikes function for trajectory reconstruction
*/

/*
** 2.80		added Bayesian reconstruction
*/

/*
** 2.80b	added expected/sd/zero corrhist output
*/

/*
** 2.81		updated for iolib v3.0 (pass VERSION to BeginStandardHeader)
*/

/*
** 2.82		version now reports RPM name
*/
