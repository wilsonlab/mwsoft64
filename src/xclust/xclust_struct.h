#include <stdint.h>
/*
** basic window stuctures
*/
#define SHARED_WINDOW_STRUCT 		\
    char		*windowname; 	\
    char		*display_name; 	\
    Display		*display; 	\
    GC			context; 	\
    Drawable		drawable; 	\
    Window		window;		\
    Visual		*visual; 	\
    int32_t			screen_number; 	\
    Font		font; 		\
    XFontStruct		*fontinfo; 	\
    int32_t			wwidth,	 	\
    			wheight; 	\
    char		*fontname; 	\
    int32_t			fontwidth, 	\
    			fontheight; 	\
    int32_t			linewidth; 	\
    uint32_t	background, 	\
    			foreground; 	\
    uint32_t	color; 		\
    uint32_t	supercolor;	\
    int32_t			color_mode;	\
    int32_t			colormaptype;	\
    int32_t			scale_type;	\
    double		**matrix;	\
    float		disparityscale;	\
    int32_t			inverse;	\
    int32_t			hide_labels;	\
    Label		*label;

#define WINDOW_STRUCT 		\
    SHARED_WINDOW_STRUCT \
    int32_t			mapped;

struct polypoint_type{
    float		wx,wy,wz;		/* line start */
    int16_t		sx,sy;
    struct polypoint_type	*next;
};

typedef struct
{
	int32_t pixel;
	int32_t r, g, b;
}ColorTable;


typedef struct job_type {
    struct job_type	*next;
    PFI		func;
    int32_t		id;
    int32_t		priority;
    char	*description;
    char	*data;
    int32_t		numdata[10];
} Job;

typedef struct worldview_type {
    int32_t	        used;		/* flag: is there a view saved here */
    float	wxmin;
    float	wxmax;
    float	wymin;
    float	wymax;
    int32_t		projection[3];	/* projection ids of the view plane */
} WorldView;


typedef struct labeltype {
    char		*name;		/* optional label name */
    int16_t		type;		/* 0 = string; 1 = box; 2 = line */
    int16_t		priority;	/* 0 = temporary; 1 = permanent */
    int16_t		coord_mode;	/* 0=world coord; 1=screen coord */
    int16_t		color;
    int16_t		visible;	/* 0 = hidden; 1 = visible */
    struct labeltype 	*next;
    union {
	struct stringlabel {
	    float		wx,wy,wz;		/* text start */
	    int16_t		sx,sy;
	    Font		font; 		
	    char		line[MAX_TEXT_LINE];
	} string;
	struct bitmaplabel {
	    float		wx,wy,wz;		/* bitmap center */
	    int16_t		sx,sy;
	    int16_t		w,h;
	    Pixmap		bitmap;
	} bitmap;
	struct boxlabel {
	    float		wx,wy,wz,wr;	/* box center/size */
	    int16_t		sx,sy,sr;
	} box;
	struct linelabel {
	    float		wx,wy,wz;		/* line start */
	    float		wx2,wy2,wz2;	/* line end */
	    int16_t		sx,sy;
	    int16_t		sx2,sy2;
	} line;
	struct polylinelabel {
	    struct polypoint_type	*points;
	    int16_t		closed;
	} polyline;
    } u;
} Label;

typedef struct {
    SHARED_WINDOW_STRUCT
} SharedWindow;

typedef struct {
    WINDOW_STRUCT
} BasicWindow;

typedef struct {
    int16_t	x,y;
} Coord;

typedef struct {
    float	x,y;
} FCoord;

typedef struct {
    char	*data;
    int32_t		datasize;
    int16_t	clusterid;
} DataCoord;

typedef struct plotoffset_type {
    int32_t		index;
    int32_t	byteoffset;
    int32_t		lineoffset;
    struct plotoffset_type *next;
} PlotOffset;


typedef struct source_type {
    char	*filename;
    FILE	*fp;
    int32_t		headersize;	/* size of the file header */
    int32_t		fileformat;	/* ascii/binary file data */
    int32_t		loadmode;	/* full or partial load */
    int32_t		startline;	/* restrict the number of points read in */
    int32_t		endline;	/* during a partial load */
    int32_t		nlines;		/* total number of records in file */
    int32_t		currentline;	/* current line in the data file */
    int32_t		type;		/* file or other */
    int32_t		startplot;	/* restrict the number of plots read in */
    int32_t		endplot;	/* during a partialload */
    int32_t		currentplot;
    int32_t		nsourcefields;	/* used for binary loads */
    int32_t		*datatype;	/* binary format of vector components */
    float	*defaults;
    int32_t		convert;
    int32_t		recordsize;	/* total bytes in each record */
    int32_t		nchannels;	/* used for eeg files */
    int32_t		datablocksize;	/* used for eeg files */
    float	eegdt;		/* used for eeg files */
    int32_t		subsetparmfile; /* flag: file contains a subset of spike ids from a spikefile */
    int32_t		notimefield;	/* file has no 'time' or 'timestamp' fields */
    int32_t		timelookupfile; /* file to use for time lookups */
    struct plot_type	*plot;
    PlotOffset	*plotoffset;
    BasicWindow *parent;
    struct source_type *next;
    int32_t       starttimestamp; /* timestamp of first currently loaded spike */
    int32_t		endtimestamp; /* timestamp of last currently loaded spike */
    int32_t		mintimestamp; /* timestamp of last spike in file*/
    int32_t		maxtimestamp; /* timestamp of last spike in file*/
  int32_t		inmovieflag; /* flag: last point load was a movie frame */
} DataSource;

typedef struct axis_type {
    int32_t			autoscale;		/* autoscale flags */
    int32_t			type;			/* linear/log */
    int32_t			show;			/* axis display flags */
    int32_t			exponent;		/* axis label exponent */
    int32_t			leftdp;			/* axis decimal precision */
    int32_t			rightdp;		/* axis decimal precision */
    char		*title;			/* axis labels */
    int32_t			style;			/* axis style */
    float		xintcpt,
			yintcpt,
			zintcpt;		/* axis intercepts */
    float		tickmin;		/* tick information */
    int32_t			nticks;			/* actual number of ticks */
    int32_t			nsubticks;		/* number of subticks */
    int32_t			desired_nticks;		/* desired number of ticks */
    float		tickinc;		/* tick spacing */
    float		desired_tickinc;	/* desired tick spacing */
    float		tickstart;		/* starting tick */
    int32_t			title_offset;
    int32_t			ticklabel_offset;
    int32_t			show_labels;		/* flag to show labels */
    int32_t			show_grid;		
} Axis;

typedef struct plot_type {
    struct plot_type 	*next;
    struct graph_type 	*graph;			/* parent graph */
    int32_t			datasource;		/* 0=file 1=stdin 2=other */
    DataSource		*source;
    char		*filename;		/* data file information */
    char		*title;			/* line title */
    int32_t			linestyle;		/* line style */
    float		xmax;			/* scale parameters */
    float		xmin;
    float		ymax;
    float		ymin;
    float		zmax;
    float		zmin;
    int32_t			xhi;			/* data limiting bounds */
    int32_t			xlo;
    int32_t			npoints;		/* number of data points */
    int32_t			arraysize;
    int32_t			ndatafields;		/* size of data vector */
    DataCoord		*rawdata;		/* data vector, clusterid*/
    Coord		*coord;			/* plot data */
    FCoord		*fcoord;		/* precise plot data */
    int32_t			selected;		/* flag */
    int32_t			visible;		/* flag */
    float		scale;			/* data scale factor */
    float		xscale;			/* x scale factor */
    float		zscale;			/* z scale factor */
    float		escale;			/* error scale factor */
    float		cscale;			/* color scale factor */
    float		xoffset_axis;		/* x axis offset */
    float		yoffset_axis;		/* y axis offset */
    float		zoffset_axis;		/* z axis offset */
    float		xoffset;		/* data x offset */
    float		yoffset;		/* data y offset */
    float		zoffset;		/* data z offset */
    int32_t			coffset;		/* color offset */
    int32_t			plot_type;		/* scatter/bar/normal */
    float		bar_width;		/* width in world coords */
    int32_t			bar_filled;		/* flag to fill the bars */
    int32_t			point_symbol;		/* icon to use to mark points */
    int32_t			showpoints;		/* flag to show point icons */
    int32_t			pointfreq;		/* how often to mark points */
    float		auto_x;			/* auto x index */
    float		auto_xstart;		/* start for auto x index */
    int32_t			xaxis;			/* flag */
    int32_t			yaxis;			/* flag */
    int32_t			zaxis;			/* flag */
    Label		*label;
    int32_t			zmode;			/* 3d display */
    int32_t			errorbartype;		/* error bar,box, or circle */
    float		pointscale;
    float		refreshtime;
} Plot;

typedef struct clusterinfo_type {
    int16_t	clusterid;
    int16_t	color;
    struct clusterinfo_type *next;
} ClusterInfo;

typedef struct projectioninfo_type {
    int16_t	projectionid;		/* projection number */
    char	*name;			/* projection name */
    float	wmin,wmax;		/* projection viewing range */
    int16_t	randomize;
    int16_t	type;
    int16_t	size;
    int32_t		offset;
    struct projectioninfo_type *next;
} ProjectionInfo;

typedef struct clusterbounds_type {
    int32_t		ncoords;
    FCoord	*fcoord;
    Coord	*coord;
    int16_t	complete;	/* flag */
    int16_t	type;		/* bounding region type box/polygon */
    int16_t	enabled;	/* box enabled flag */
    int16_t 	clusterid;	/* cluster id of the box */
    int32_t		projection[2];	/* plane of the box defined by */
				/* vector components of the projection */
    struct clusterbounds_type *next;
} ClusterBounds;

typedef struct clusterlist_type {
    int16_t	clusterid;
    int16_t	modified;	/* flag */
    int16_t	enabled;	/* flag */
    int16_t	hidden;		/* flag */
    int16_t	showonly;	/* flag */
    int32_t		npoints;
    int32_t		csi;
    int32_t		ncsi;
    int32_t		csiclust;
    int32_t		ncsi_plus;
    int32_t		ncsi_refract_minus;
    int32_t		burstlen;
    int32_t		score;
    struct clusterlist_type *next;
} ClusterList;

typedef struct backup_type {
    ClusterBounds	*clusterbounds;
    ClusterList		*clusterlist;
} Backup;

typedef struct graph_type {
    WINDOW_STRUCT
    struct _Frame	*frame;
    Plot		*plot;
    DataSource		*source;
    struct graph_type 	*next;
    int32_t			button;			/* which button is pressed */
    float		dragwx1,dragwy1;
    int32_t			dragx1,dragy1,dragx2,dragy2;
    char		*title;			/* plot title */
    Axis		xaxis;
    Axis		yaxis;
    Axis 		zaxis;

    int32_t			ticksize;		/* size of ticks in pixels */
    float		wxmin,			/* world screen coordinates */
    			wxmax;
    float		wymin,
    			wymax;
    float		wxscale,		/* screen coord scale factor */
    			wyscale;
    float		xmin,			/* min/max data coordinates */
    			xmax;
    float		ymin,
    			ymax;
    float		zmin,
    			zmax;
    int32_t			show_legend;		/* flag to show legend */
    int32_t			show_title;		/* flag to show title */
    int32_t			auto_axes;		/* flag for auto axes  */
    int32_t			resolution;		/* postscript resolution 0/1 */
    int32_t			quadrants;		/* axis quadrants to display */
    int32_t			gridcolor;		
    int32_t			xtitlejustify;		/* location of xaxis title */
    int32_t			optimize;		/* optimize line drawing */ 
    int32_t			optimizepoints;		/* optimize point drawing */ 
    Plot		*stepthrough;
    float		thetax;			/* rotation about x */
    float		thetay;			/* rotation about y */
    float		thetaz;			/* rotation about z */
    int32_t			currentprojection[3];	/* displayed vec components */
    int32_t			selectedprojection[3];	/* requested vec components */
    Backup		*backup;		/* state info to undo ops */
    ClusterBounds	*clusterbounds;		/* cluster bounds info */
    ClusterList		*clusterlist;		/* list of defined clusters */
    ClusterInfo		*clusterinfo;		/* cluster attributes */
    ProjectionInfo	*projectioninfo;	/* projection attributes */
    int16_t		selectedcluster;	/* selected cluster */
    int16_t		defaultcluster;		/* default cluster assigned to
						   points 
						*/
    float		*defaults;		/* default plot parms */
    int32_t			overlay;		/* suppress refresh clear */
    char		hiddencluster[MAXCLUSTER];
    int32_t			showclusterlabels;	/* flag */
    int32_t			cbmodified;		/* cluster bounds mod flag */
    WorldView		savedview[MAXSAVEDVIEWS];	/* saved world views */
    int32_t			lastsavedview;		/* Function key number
						   of last saved view
						   loaded */
/*     int32_t			scalemode;		/\* FROMRC, AUTOSCALE, */
/* 						   or KEEPLAST *\/ */
/*     int32_t			insavedview;            /\* flag to indicate */
/* 						   whether current */
/* 						   view is from a */
/* 						   saved view (used by */
/* 						   KEEPLAST mode) *\/ */
  
    int32_t			changed;		/* flag for refresh */
    int32_t			datalength;		/* bytes in each xclust data vector*/
    int32_t			nprojections;
    int32_t                 rotpointfreq;           /* skip points during interactive rotation*/
    int32_t                 rotorigin;            /* 0 = use 0,0,0 as rotation origin, 1 = use middle of screen */
    int32_t                 showbounds; /*flag */
} Graph;

typedef struct {
    int32_t		id;
    PFI		func;
    BasicWindow	*parent;
} WindowData;

/*
typedef struct {
    WINDOW_STRUCT
    struct _Frame	*frame;
    char	line[MAX_TEXT_LINE];
    int32_t		index;
} TextWindow;
*/

typedef struct menu_type{
    WINDOW_STRUCT
    struct _Frame	*frame;
    char	line[MAX_TEXT_LINE];
    int32_t		index;
    struct _MenuItem	*item;
    struct menu_type	*next;
    int32_t		itembackground;	/* default background color */
    int32_t		itemforeground;	/* default foreground color */
    int32_t		cluststatoffset;/* height at which to draw the cluster info window  */
    struct menu_frame_type *parent;
} MenuWindow;

typedef MenuWindow TextWindow; 

typedef struct menu_frame_type {
  WINDOW_STRUCT
  struct menu_container_type *container; /* parent container */
  struct menu_frame_type *next;          /* linked list of menu frames */
  struct menu_type *menu;                /* the only child menu */
  int32_t collapsible;                       /* can menu be expanded / collapsed? */
  int32_t expanded;                          /* current state of menu */
  int32_t showtitle;                         /* flag to set visibility of titlebar, incl. expand/collapse button */
  char *title;                           /* title shown in title bar */
} MenuFrame;

typedef struct menu_container_type {
  WINDOW_STRUCT
  struct _Frame *frame;                /* parent frame */
  struct menu_container_type *next;    /* linked list of menu containers */
  struct menu_frame_type *menuframe;   /* linked list of menu frames */
} MenuContainer;


typedef struct _Frame {
    WINDOW_STRUCT
    Graph	*graph;
    TextWindow	*text;
    MenuContainer *menucontainer;
    MenuWindow	*menu;
    MenuWindow	*cluststat;
    MenuWindow	*colorstat;
    Colormap	cmap;
    int32_t		x,y;
    int32_t		width,height;
    Job		*joblist;
} Frame;

typedef struct _MenuItem {
    WINDOW_STRUCT
    MenuWindow		*menu;
    struct _MenuItem	*next;
    char		*string;
    char		*string2;
    int32_t			invert;
    int32_t			invert2;
    int32_t			x,y,w,h;
    PFI			func;		/* execute when item activated */
    PFI			redrawfunc;	/* execute to redraw item */
    Pixmap		bitmap1;
    Pixmap		bitmap2;
    int32_t			state;
    int32_t			type;
    char		*name;
    char		*value;
    char		*class;
    int32_t			itemforeground;		/* foreground color */
    int32_t			itembackground;		/* background color */
    int32_t			enabled;
    int32_t			entered;
    char                *data;
} MenuItem;




