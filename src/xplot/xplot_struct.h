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
    int32_t			color_mode;	\
    int32_t			scale_type;	\
    double		**matrix;	\
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

typedef struct labeltype {
    int16_t		coord_mode;	/* 0=world coord; 1=screen coord */
    int16_t		color;
    struct labeltype 	*next;
    int16_t		priority;	/* 0 = temporary; 1 = permanent */
    int16_t		type;		/* 0 = string; 1 = box; 2 = line */
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
    float	value;
} ErrorData;

typedef struct source_type {
    char	*filename;
    int32_t		type;
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
    int32_t			datasource;		/* 0=file 1=stdin 2=other */
    char		*filename;		/* data file information */
    char		*title;			/* line title */
    int32_t			linestyle;		/* line style */
    float		xmax;			/* scale parameters */
    float		xmin;
    float		ymax;
    float		ymin;
    int32_t			xhi;			/* data limiting bounds */
    int32_t			xlo;
    int32_t			npoints;		/* number of data points */
    int32_t			arraysize;
    FCoord		*data;			/* original data */
    Coord		*coord;			/* plot data */
    FCoord		*fcoord;		/* precise plot data */
    ErrorData		*edata;			/* error bar data */
    struct plot_type *next;
    struct graph_type 	*graph;			/* parent graph */
    int32_t			selected;		/* flag */
    int32_t			visible;		/* flag */
    float		scale;			/* data scale factor */
    float		xscale;			/* x scale factor */
    float		escale;			/* error scale factor */
    float		xoffset_axis;		/* x axis offset */
    float		yoffset_axis;		/* y axis offset */
    float		zoffset_axis;		/* y axis offset */
    float		xoffset;		/* data x offset */
    float		yoffset;		/* data y offset */
    float		zoffset;		/* data y offset */
    int32_t			plot_type;		/* scatter/bar/normal */
    float		bar_width;		/* width in world coords */
    int32_t			bar_filled;		/* flag to fill the bars */
    int32_t			show_error;		/* flag to display error bars */
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
    int32_t			randomizex;
    int32_t			randomizey;
} Plot;

typedef struct graph_type {
    WINDOW_STRUCT
    struct _Frame	*frame;
    Plot		*plot;
    DataSource		*source;
    struct graph_type 	*next;
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
    int32_t			show_legend;		/* flag to show legend */
    int32_t			show_title;		/* flag to show title */
    int32_t			auto_xaxis;		/* flag for auto x axis  */
    int32_t			auto_yaxis;		/* flag for auto y axis  */
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
    float		disparityscale;		/* stero disparity scaling */
    WorldView		savedview[MAXSAVEDVIEWS];	/* saved world views */
} Graph;

typedef struct {
    int32_t		id;
    PFI		func;
    BasicWindow	*parent;
} WindowData;

typedef struct {
    WINDOW_STRUCT
    struct _Frame	*frame;
    char	line[MAX_TEXT_LINE];
    int32_t		index;
} TextWindow;

typedef struct {
    WINDOW_STRUCT
    struct _Frame	*frame;
    char	line[80];
    int32_t		index;
    struct _MenuItem	*item;
} MenuWindow;

typedef struct _Frame {
    WINDOW_STRUCT
    Graph	*graph;
    TextWindow	*text;
    MenuWindow	*menu;
    int32_t		x,y;
    int32_t		width,height;
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
    PFI			func;
    Pixmap		bitmap1;
    Pixmap		bitmap2;
    int32_t			state;
    int32_t			type;
} MenuItem;

typedef struct {
    int32_t pixel;
    int32_t r,g,b;
} ColorTable;

