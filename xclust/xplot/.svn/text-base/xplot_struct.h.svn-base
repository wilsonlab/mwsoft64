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
    int			color_mode;	\
    int			scale_type;	\
    double		**matrix;	\
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

typedef struct labeltype {
    short		coord_mode;	/* 0=world coord; 1=screen coord */
    short		color;
    struct labeltype 	*next;
    short		priority;	/* 0 = temporary; 1 = permanent */
    short		type;		/* 0 = string; 1 = box; 2 = line */
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
    float	value;
} ErrorData;

typedef struct source_type {
    char	*filename;
    int		type;
    BasicWindow *parent;
    struct source_type *next;
} DataSource;

typedef struct worldview_type {
    float	wxmin;
    float	wxmax;
    float	wymin;
    float	wymax;
} WorldView;


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
    int			datasource;		/* 0=file 1=stdin 2=other */
    char		*filename;		/* data file information */
    char		*title;			/* line title */
    int			linestyle;		/* line style */
    float		xmax;			/* scale parameters */
    float		xmin;
    float		ymax;
    float		ymin;
    int			xhi;			/* data limiting bounds */
    int			xlo;
    int			npoints;		/* number of data points */
    int			arraysize;
    FCoord		*data;			/* original data */
    Coord		*coord;			/* plot data */
    FCoord		*fcoord;		/* precise plot data */
    ErrorData		*edata;			/* error bar data */
    struct plot_type *next;
    struct graph_type 	*graph;			/* parent graph */
    int			selected;		/* flag */
    int			visible;		/* flag */
    float		scale;			/* data scale factor */
    float		xscale;			/* x scale factor */
    float		escale;			/* error scale factor */
    float		xoffset_axis;		/* x axis offset */
    float		yoffset_axis;		/* y axis offset */
    float		zoffset_axis;		/* y axis offset */
    float		xoffset;		/* data x offset */
    float		yoffset;		/* data y offset */
    float		zoffset;		/* data y offset */
    int			plot_type;		/* scatter/bar/normal */
    float		bar_width;		/* width in world coords */
    int			bar_filled;		/* flag to fill the bars */
    int			show_error;		/* flag to display error bars */
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
    int			randomizex;
    int			randomizey;
} Plot;

typedef struct graph_type {
    WINDOW_STRUCT
    struct _Frame	*frame;
    Plot		*plot;
    DataSource		*source;
    struct graph_type 	*next;
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
    int			show_legend;		/* flag to show legend */
    int			show_title;		/* flag to show title */
    int			auto_xaxis;		/* flag for auto x axis  */
    int			auto_yaxis;		/* flag for auto y axis  */
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
    float		disparityscale;		/* stero disparity scaling */
    WorldView		savedview[MAXSAVEDVIEWS];	/* saved world views */
} Graph;

typedef struct {
    int		id;
    PFI		func;
    BasicWindow	*parent;
} WindowData;

typedef struct {
    WINDOW_STRUCT
    struct _Frame	*frame;
    char	line[MAX_TEXT_LINE];
    int		index;
} TextWindow;

typedef struct {
    WINDOW_STRUCT
    struct _Frame	*frame;
    char	line[80];
    int		index;
    struct _MenuItem	*item;
} MenuWindow;

typedef struct _Frame {
    WINDOW_STRUCT
    Graph	*graph;
    TextWindow	*text;
    MenuWindow	*menu;
    int		x,y;
    int		width,height;
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
    PFI			func;
    Pixmap		bitmap1;
    Pixmap		bitmap2;
    int			state;
    int			type;
} MenuItem;

typedef struct {
    int pixel;
    int r,g,b;
} ColorTable;

