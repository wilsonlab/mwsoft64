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
    int			screen_number; 	\
    Font		font; 		\
    XFontStruct		*fontinfo; 	\
    int			wwidth,	 	\
    			wheight; 	\
    char		*fontname; 	\
    int			fontwidth, 	\
    			fontheight; 	\
    int			linewidth; 	\
    unsigned long	background, 	\
    			foreground; 	\
    unsigned long	color; 		\
    unsigned long	supercolor;	\
    int			color_mode;	\
    int			colormaptype;	\
    int			scale_type;	\
    double		**matrix;	\
    float		disparityscale;	\
    int			inverse;	\
    int			hide_labels;	\
    Label		*label;

#define WINDOW_STRUCT 		\
    SHARED_WINDOW_STRUCT \
    int			mapped;

struct polypoint_type{
    float		wx,wy,wz;		/* line start */
    short		sx,sy;
    struct polypoint_type	*next;
};

typedef struct
{
	int pixel;
	int r, g, b;
}ColorTable;


typedef struct job_type {
    struct job_type	*next;
    PFI		func;
    int		id;
    int		priority;
    char	*description;
    char	*data;
    int		numdata[10];
} Job;

typedef struct worldview_type {
    int	        used;		/* flag: is there a view saved here */
    float	wxmin;
    float	wxmax;
    float	wymin;
    float	wymax;
    int		projection[3];	/* projection ids of the view plane */
} WorldView;


typedef struct labeltype {
    char		*name;		/* optional label name */
    short		type;		/* 0 = string; 1 = box; 2 = line */
    short		priority;	/* 0 = temporary; 1 = permanent */
    short		coord_mode;	/* 0=world coord; 1=screen coord */
    short		color;
    short		visible;	/* 0 = hidden; 1 = visible */
    struct labeltype 	*next;
    union {
	struct stringlabel {
	    float		wx,wy,wz;		/* text start */
	    short		sx,sy;
	    Font		font; 		
	    char		line[MAX_TEXT_LINE];
	} string;
	struct bitmaplabel {
	    float		wx,wy,wz;		/* bitmap center */
	    short		sx,sy;
	    short		w,h;
	    Pixmap		bitmap;
	} bitmap;
	struct boxlabel {
	    float		wx,wy,wz,wr;	/* box center/size */
	    short		sx,sy,sr;
	} box;
	struct linelabel {
	    float		wx,wy,wz;		/* line start */
	    float		wx2,wy2,wz2;	/* line end */
	    short		sx,sy;
	    short		sx2,sy2;
	} line;
	struct polylinelabel {
	    struct polypoint_type	*points;
	    short		closed;
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
    short	x,y;
} Coord;

typedef struct {
    float	x,y;
} FCoord;

typedef struct {
    char	*data;
    int		datasize;
    short	clusterid;
} DataCoord;

typedef struct plotoffset_type {
    int		index;
    long	byteoffset;
    int		lineoffset;
    struct plotoffset_type *next;
} PlotOffset;


typedef struct source_type {
    char	*filename;
    FILE	*fp;
    int		headersize;	/* size of the file header */
    int		fileformat;	/* ascii/binary file data */
    int		loadmode;	/* full or partial load */
    int		startline;	/* restrict the number of points read in */
    int		endline;	/* during a partial load */
    int		nlines;		/* total number of records in file */
    int		currentline;	/* current line in the data file */
    int		type;		/* file or other */
    int		startplot;	/* restrict the number of plots read in */
    int		endplot;	/* during a partialload */
    int		currentplot;
    int		nsourcefields;	/* used for binary loads */
    int		*datatype;	/* binary format of vector components */
    float	*defaults;
    int		convert;
    int		recordsize;	/* total bytes in each record */
    int		nchannels;	/* used for eeg files */
    int		datablocksize;	/* used for eeg files */
    float	eegdt;		/* used for eeg files */
    int		subsetparmfile; /* flag: file contains a subset of spike ids from a spikefile */
    int		notimefield;	/* file has no 'time' or 'timestamp' fields */
    int		timelookupfile; /* file to use for time lookups */
    struct plot_type	*plot;
    PlotOffset	*plotoffset;
    BasicWindow *parent;
    struct source_type *next;
    int       starttimestamp; /* timestamp of first currently loaded spike */
    int		endtimestamp; /* timestamp of last currently loaded spike */
    int		mintimestamp; /* timestamp of last spike in file*/
    int		maxtimestamp; /* timestamp of last spike in file*/
  int		inmovieflag; /* flag: last point load was a movie frame */
} DataSource;

typedef struct axis_type {
    int			autoscale;		/* autoscale flags */
    int			type;			/* linear/log */
    int			show;			/* axis display flags */
    int			exponent;		/* axis label exponent */
    int			leftdp;			/* axis decimal precision */
    int			rightdp;		/* axis decimal precision */
    char		*title;			/* axis labels */
    int			style;			/* axis style */
    float		xintcpt,
			yintcpt,
			zintcpt;		/* axis intercepts */
    float		tickmin;		/* tick information */
    int			nticks;			/* actual number of ticks */
    int			nsubticks;		/* number of subticks */
    int			desired_nticks;		/* desired number of ticks */
    float		tickinc;		/* tick spacing */
    float		desired_tickinc;	/* desired tick spacing */
    float		tickstart;		/* starting tick */
    int			title_offset;
    int			ticklabel_offset;
    int			show_labels;		/* flag to show labels */
    int			show_grid;		
} Axis;

typedef struct plot_type {
    struct plot_type 	*next;
    struct graph_type 	*graph;			/* parent graph */
    int			datasource;		/* 0=file 1=stdin 2=other */
    DataSource		*source;
    char		*filename;		/* data file information */
    char		*title;			/* line title */
    int			linestyle;		/* line style */
    float		xmax;			/* scale parameters */
    float		xmin;
    float		ymax;
    float		ymin;
    float		zmax;
    float		zmin;
    int			xhi;			/* data limiting bounds */
    int			xlo;
    int			npoints;		/* number of data points */
    int			arraysize;
    int			ndatafields;		/* size of data vector */
    DataCoord		*rawdata;		/* data vector, clusterid*/
    Coord		*coord;			/* plot data */
    FCoord		*fcoord;		/* precise plot data */
    int			selected;		/* flag */
    int			visible;		/* flag */
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
    int			coffset;		/* color offset */
    int			plot_type;		/* scatter/bar/normal */
    float		bar_width;		/* width in world coords */
    int			bar_filled;		/* flag to fill the bars */
    int			point_symbol;		/* icon to use to mark points */
    int			showpoints;		/* flag to show point icons */
    int			pointfreq;		/* how often to mark points */
    float		auto_x;			/* auto x index */
    float		auto_xstart;		/* start for auto x index */
    int			xaxis;			/* flag */
    int			yaxis;			/* flag */
    int			zaxis;			/* flag */
    Label		*label;
    int			zmode;			/* 3d display */
    int			errorbartype;		/* error bar,box, or circle */
    float		pointscale;
    float		refreshtime;
} Plot;

typedef struct clusterinfo_type {
    short	clusterid;
    short	color;
    struct clusterinfo_type *next;
} ClusterInfo;

typedef struct projectioninfo_type {
    short	projectionid;		/* projection number */
    char	*name;			/* projection name */
    float	wmin,wmax;		/* projection viewing range */
    short	randomize;
    short	type;
    short	size;
    int		offset;
    struct projectioninfo_type *next;
} ProjectionInfo;

typedef struct clusterbounds_type {
    int		ncoords;
    FCoord	*fcoord;
    Coord	*coord;
    short	complete;	/* flag */
    short	type;		/* bounding region type box/polygon */
    short	enabled;	/* box enabled flag */
    short 	clusterid;	/* cluster id of the box */
    int		projection[2];	/* plane of the box defined by */
				/* vector components of the projection */
    struct clusterbounds_type *next;
} ClusterBounds;

typedef struct clusterlist_type {
    short	clusterid;
    short	modified;	/* flag */
    short	enabled;	/* flag */
    short	hidden;		/* flag */
    short	showonly;	/* flag */
    int		npoints;
    int		csi;
    int		ncsi;
    int		csiclust;
    int		ncsi_plus;
    int		ncsi_refract_minus;
    int		burstlen;
    int		score;
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
    int			button;			/* which button is pressed */
    float		dragwx1,dragwy1;
    int			dragx1,dragy1,dragx2,dragy2;
    char		*title;			/* plot title */
    Axis		xaxis;
    Axis		yaxis;
    Axis 		zaxis;

    int			ticksize;		/* size of ticks in pixels */
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
    int			show_legend;		/* flag to show legend */
    int			show_title;		/* flag to show title */
    int			auto_axes;		/* flag for auto axes  */
    int			resolution;		/* postscript resolution 0/1 */
    int			quadrants;		/* axis quadrants to display */
    int			gridcolor;		
    int			xtitlejustify;		/* location of xaxis title */
    int			optimize;		/* optimize line drawing */ 
    int			optimizepoints;		/* optimize point drawing */ 
    Plot		*stepthrough;
    float		thetax;			/* rotation about x */
    float		thetay;			/* rotation about y */
    float		thetaz;			/* rotation about z */
    int			currentprojection[3];	/* displayed vec components */
    int			selectedprojection[3];	/* requested vec components */
    Backup		*backup;		/* state info to undo ops */
    ClusterBounds	*clusterbounds;		/* cluster bounds info */
    ClusterList		*clusterlist;		/* list of defined clusters */
    ClusterInfo		*clusterinfo;		/* cluster attributes */
    ProjectionInfo	*projectioninfo;	/* projection attributes */
    short		selectedcluster;	/* selected cluster */
    short		defaultcluster;		/* default cluster assigned to
						   points 
						*/
    float		*defaults;		/* default plot parms */
    int			overlay;		/* suppress refresh clear */
    char		hiddencluster[MAXCLUSTER];
    int			showclusterlabels;	/* flag */
    int			cbmodified;		/* cluster bounds mod flag */
    WorldView		savedview[MAXSAVEDVIEWS];	/* saved world views */
    int			lastsavedview;		/* Function key number
						   of last saved view
						   loaded */
/*     int			scalemode;		/\* FROMRC, AUTOSCALE, */
/* 						   or KEEPLAST *\/ */
/*     int			insavedview;            /\* flag to indicate */
/* 						   whether current */
/* 						   view is from a */
/* 						   saved view (used by */
/* 						   KEEPLAST mode) *\/ */
  
    int			changed;		/* flag for refresh */
    int			datalength;		/* bytes in each xclust data vector*/
    int			nprojections;
    int                 rotpointfreq;           /* skip points during interactive rotation*/
    int                 rotorigin;            /* 0 = use 0,0,0 as rotation origin, 1 = use middle of screen */
    int                 showbounds; /*flag */
} Graph;

typedef struct {
    int		id;
    PFI		func;
    BasicWindow	*parent;
} WindowData;

/*
typedef struct {
    WINDOW_STRUCT
    struct _Frame	*frame;
    char	line[MAX_TEXT_LINE];
    int		index;
} TextWindow;
*/

typedef struct menu_type{
    WINDOW_STRUCT
    struct _Frame	*frame;
    char	line[MAX_TEXT_LINE];
    int		index;
    struct _MenuItem	*item;
    struct menu_type	*next;
    int		itembackground;	/* default background color */
    int		itemforeground;	/* default foreground color */
    int		cluststatoffset;/* height at which to draw the cluster info window  */
    struct menu_frame_type *parent;
} MenuWindow;

typedef MenuWindow TextWindow; 

typedef struct menu_frame_type {
  WINDOW_STRUCT
  struct menu_container_type *container; /* parent container */
  struct menu_frame_type *next;          /* linked list of menu frames */
  struct menu_type *menu;                /* the only child menu */
  int collapsible;                       /* can menu be expanded / collapsed? */
  int expanded;                          /* current state of menu */
  int showtitle;                         /* flag to set visibility of titlebar, incl. expand/collapse button */
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
    int		x,y;
    int		width,height;
    Job		*joblist;
} Frame;

typedef struct _MenuItem {
    WINDOW_STRUCT
    MenuWindow		*menu;
    struct _MenuItem	*next;
    char		*string;
    char		*string2;
    int			invert;
    int			invert2;
    int			x,y,w,h;
    PFI			func;		/* execute when item activated */
    PFI			redrawfunc;	/* execute to redraw item */
    Pixmap		bitmap1;
    Pixmap		bitmap2;
    int			state;
    int			type;
    char		*name;
    char		*value;
    char		*class;
    int			itemforeground;		/* foreground color */
    int			itembackground;		/* background color */
    int			enabled;
    int			entered;
    char                *data;
} MenuItem;




