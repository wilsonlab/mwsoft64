#define VERSION "2.32"
/*
** 2.28 modified mean rate in the status output to reflect total spikes
** between start and end time only
** 2.29 changed the ascii formatting option to override the binary format 
** specification.  Added a field parameter line to the header output for vgrid
** output
** 2.29a modified the InPolygon routine to detect points that equal wy by 
** adding an equality to the inequality bounds tests
** 2.29b added field peak statistics to the stat output
** 2.29c modified centroid to handle missing back and front diodes
** 2.29d added info printout in behav_stat
** 2.30 changed BeginStandardHeader calls to pass VERSION, moved
**      behav.h to behav_ext.h
** 2.31 fixed (new) bug in behav_write.c that caused us to exit with an error 
**      incorrectly if no rate/pos output file was given.
** 2.32 changed -version results to include RPM name
*/

/*
*************************************
**            DEFINES              **
*************************************
*/

#define MAXCLUSTERS 		10
#define YPOSITION_RESOLUTION	256.0
#define XPOSITION_RESOLUTION	364.0
/*#define POSITION_INTERVAL	50e-3*/
#define POSITION_INTERVAL	33.3e-3 
/*#define POSITION_INTERVAL	100e-3 */
#define MAXTHETA		360
#define MAXVELOCITY		200
#define MAXANGVELOCITY		2000
#define TINYVAL			1e-20

#define GRID			0
#define VECTOR			1
#define RAW			2

#define LINEAR			0
#define LOG			1
#define POLAR			2

#define FRONT			0
#define CENTROID		1

#define POSITION		0
#define DIRECTION		1
#define VELOCITY		2
#define ANGVELOCITY		3

#define ASCII			0
#define BINARY			1

#define EVENTS			0
#define INTERVALS		1
#define INTERVALTHRESH		2

#define DEF_MAX_RADIUS		30
#define OLDDEF_MIN_RADIUS		8
#define DEF_MIN_RADIUS		1
#define DEF_MAX_CONSEC_MISS	3
#define LOOKAHEAD		500
#define FLIPANG			90
#define LINEARANG		45
#define DIODE_ANGFACTOR		1.0
#define INVALIDDIR		999
#define INVALIDMAG		-1
#define INVALIDANGV		-9999

#define CORR_RESULT		0
#define RELIABILITY_RESULT	1
#define NONZFIRING_RESULT	2
#define ZFIRING_RESULT		3
#define SAMPLEBIAS_RESULT	4
#define OCC_RESULT		5
#define OCCRATE_RESULT		6
#define CUM_RESULT		7
#define POSITION_SAMPLES	8

#define DIRBINSIZE 		45

#define STANDARD_OCCUPANCY	1

#define GRID_ROTATION		0

#define HUGECHI			1e10
#define TINYVAR			1e-10

#define CHI_MINOCCUPANCY	5

#define LINELEN			4
#define MINRATE			0

#define INF			0
#define VERBOSE			0
