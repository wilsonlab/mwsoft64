#define VERSION		"3.3"
/*
** 2.15c added projection attributes to the saved views
**
** 2.15b modified the InPolygon routine to detect points that equal wy by 
** adding an equality to the inequality bounds tests
**
** 2.2 major overhaul of color code allowing support of greater than 8-bit
** servers
** 3.0 major overhaul of parmameter data.  Variable data type support 
** added allowing Fields: to have mixed data type.  All data stored
** internally the specified type.  Timestamps can now have better that 
** floating point precision.
** servers
** 3.1 added in proper handling of files with special extraction types specified in the header
** 3.2 bug-fixes from Fabian and Tom D. See release notes.
*/
#define XCLUST
#define MENU_WIDTH	320

/* number of epoch input boxes */
#define NUM_EPOCHS	6

/* max score assigned to clusters, min score is 1 */
#define MAX_CLUST_SCORE	5

/* number of timestamps per second */
#define TSCALE	10000

/* ?? unused ?? */
/* #define MAXFLOATBITS 24 */

#define DEFAULTFONT	"-adobe-helvetica-bold-r-normal--10-100-75-75-p-60-iso8859-1"
#define FIXEDWIDTHFONT  "-adobe-courier-medium-r-normal--10-100-75-75-m-60-iso8859-1"
#define FIXEDWIDTHFONT2 "-schumacher-clean-bold-r-normal--10-100-75-75-c-60-iso646.1991-irv"

#define FULL_LOAD 	0
#define PARTIAL_LOAD	1
#define PARTIAL_PLOT_LOAD	2

#define FROM_FILE 	0
#define FROM_OTHER 	1

#define LINE_GRAPH	0
#define BAR_GRAPH	1
#define SCATTER_GRAPH	2
#define STEP_GRAPH	3

#define XMODE 0
#define YMODE 1
#define ZMODE 2

#define BOX_PT		0
#define FBOX_PT		1
#define CIRCLE_PT	2
#define FCIRCLE_PT	3
#define TRIANGLE_PT	4
#define FTRIANGLE_PT	5
#define X_PT		6
#define CROSS_PT	7
#define DOT_PT		8
#define LINE_PT		9
#define NO_PT		10
#define VLINE_PT		11

#define	LINEAR_AXIS	0
#define LOG10_AXIS	1

#define FRAME_WINDOW	1
#define GRAPH_WINDOW	2
#define TEXT_WINDOW	3
#define MENU_WINDOW	4
#define CLUSTSTAT_WINDOW 5
#define COLORSTAT_WINDOW 6
#define MENU_CONTAINER 7
#define MENU_FRAME_OFFSET 100
#define MENU_OFFSET 200

#define SOLID_LINE	0
#define MAX_TEXT_LINE	100

#define WORLD_LBL	0
#define SCREEN_LBL	1
#define WINDOW_LBL	2

#define TEMPORARY_LBL	0
#define PERMANENT_LBL	1

#define STRING_LBL	0
#define BOX_LBL		1
#define LINE_LBL	2
#define BITMAP_LBL	3

/*
** define button modes
*/
#define DATAMODE	0
#define DRAWMODE	1
#define TEXTMODE	2
#define ZAPMODE		3
#define PINCHMODE	4
#define POLYLINEMODE	5
#define CLUSTERBOXMODE	6
#define CLUSTERPOLYLINEMODE	7
#define ZOOMBOXMODE     8
#define ZOOMINTERACTIVEMODE 9
#define PANINTERACTIVEMODE  10
#define WOBBLEMODE      11
#define EDITBOUNDMODE   12
#define FLASHSELECTMODE 13
#define RULERLINEMODE   14
#define DRAGLINEMODE    15

/*
** define item types
*/
#define BUTTON		1
#define TOGGLE		2
#define DIALOG		4
#define INPUTAREA	8
#define RADIO		16

#define RIGHTJUSTIFY		0
#define LEFTJUSTIFY		1
#define CENTERJUSTIFY		2

/* #define OMAXCOLORS	256	 */
/* #define MAXCOLORS	256	 */
/* #define GRAYSCALE 	0 */
/* #define SPECTRALSCALE 	1 */
/* #define RGRAYSCALE 	2 */
/* #define RANDOMSCALE 	3 */
/* #define FILESCALE 	4 */

#define NORMAL		0
#define DISPARITY	1

/* graph->defaults indices */
#define PLOTTYPE	0
#define POINTTYPE	1
#define ZMODE		2
#define POINTFREQ	3
#define SHOWPOINTS	4
#define FOREGROUND	5
#define BACKGROUND	6
#define ESCALE		7
#define LINESTYLE	8

#define MAXDEFAULTS	9

#define MAXSAVEDVIEWS	13

#ifndef ASCII
#define ASCII		0
#define BINARY		1
#endif
#define POSITION	2
#define XVIEW		3
#define POSITION2	4
#define EEG		5

#define POLYBOUND	0
#define BOXBOUND	1

#define	INCOMPLETE	0
#define COMPLETE	1

/* #define MAXCLUSTER	(MAXCOLORS - 1) */
#define MAXCLUSTER      250
#define MAXPROJECTION	255 /* totally arbitrary limit */

#define MAXINTERFACECOLORS      50 /* including 0/1 for black and white */
#define MININTERFACECOLOR	2
#define MAXCOLORS	        (MAXCLUSTER + MAXINTERFACECOLORS)

/* #define GRAY1 	20 */
/* #define GRAYLEVEL1 	80 */

#define FRAMEFOREGROUND 	0
#define FRAMEBACKGROUND		1
#define MENUBACKGROUND		2
#define MENUFOREGROUND		3
/* #define CLUSTSTATBACKGROUND	4 */
/* #define CLUSTSTATFOREGROUND	5 */
#define TEXTBACKGROUND		6
#define TEXTFOREGROUND		7
#define GRAPHBACKGROUND		8
#define GRAPHFOREGROUND		9
#define MENUITEMBACKGROUND	10
#define MENUITEMFOREGROUND	11
#define MENUITEMLIGHTEDGE	12
#define MENUITEMDARKEDGE	13
#define MENUITEMDISABLE		14
#define MENUITEMINPUT		15
#define MENUFRAMEBACKGROUND     16
#define MENUFRAMEFOREGROUND     17
#define HIGHLIGHT1              18
#define SELECTION1              19
#define SELECTION2              20
#define ZEROCLUSTER             21
#define XAXIS                   22
#define YAXIS                   23
#define ZAXIS                   24
#define AXESLABELS              25
#define AXESTICKS               26
#define AXESTICKLABELS          27
#define GRIDCOLOR               28

#define MINCLUSTER		MAXINTERFACECOLORS

#define ERRORNONE     	0
#define ERRORBAR        1
#define ERRORLINE       2
#define ERRORCIRCLE     3
#define ERRORBOX        4
#define ERRORFILLEDBOX	5
#define ERRORCOLORBOX	6
#define ERRORFILLEDCOLORBOX	7

#define FILLEDCOLORBOX	8
#define ZLINE      9

#define SHARED  0
#define DEFAULT 1
#define PRIVATE 2

#define WHITE	1
#define BLACK	0

/* values for source->timelookupfile */
#define INVALIDFILE -1
#define PARMFILE 0
#define TTFILE 1

/* number of projection and cluster buttons to show in the main menu */
#define PROJECTION_BLOCK_SIZE	12
#define CLUSTER_BLOCK_SIZE	12

/* strings for cluster mode buttons */
#define POLYGONSTRING "POLYGON"
#define BOXSTRING "BOX"

/* values/strings for graph->scalemode */
#define FROMRC 0
#define AUTOSCALE 1
#define KEEPLAST 2
#define FROMRCSTRING "FROMRC"
#define AUTOSCALESTRING "AUTOSCALE"
#define KEEPLASTSTRING "KEEPLAST"


/**********/
/* Macros */
/**********/

/* useful macros */

#define SQR(x)		(x)*(x)
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

/* Saved view macros */

/* cyclical decrement over range 2-12 */
#define SAVEDVIEWDEC(sv) ((sv <= 2 || sv > 12) ? sv = 12 : sv--)
/* cyclical increment over range 2-12 */
#define SAVEDVIEWINC(sv) ((sv >= 12 || sv < 2) ? sv = 2 : sv++)
