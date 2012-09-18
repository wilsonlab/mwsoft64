typedef struct {
    float x,y,z;
} Point;

typedef struct {
    int	pixel;
    int	r,g,b;
} ColorTable;

typedef struct {
    Point	p1,p2,p3;
    float 	a,b,c,d;
    float	zmin,zmax;
} Plane;

typedef struct {
    Point	p1,p2;
    float 	a,b,c;
    int		infinite;
} Line;

typedef struct	image_type {
    float	value;
    short	sign;
} Image;

typedef struct {
    int	ncontours;
    float value[MAXCONTOURS];
} ContourList;

typedef struct {
    short	x,y;
} Coord;

typedef struct {
    Display		*display;
    GC			context;
    Drawable		drawable;
    Visual		*visual;
    int			screen_number;
    Font		font;
    XFontStruct		*fontinfo;
    Window		imagewindow;
    int			wwidth;
    int			wheight;
    int			fontheight;
    int			fontwidth;
    int			linewidth;
    unsigned long	background;
    unsigned long	foreground;
    unsigned long	color;
    int			scale_type;
    int			mapped;
    int			inverse;
} GlobalContext;

typedef struct {
    char		*filename;
    int			headersize;
    float		start_time;
    float		dt;
    int			xmax;
    int			ymax;
    int			cellnum;
    int			datatype;
    int			datasize;
    float		view_time;
    float		view_step;
    int			display_mode;
    int			singlestep;
    int			plain;
    int			valid_frame;
    int			posdata;
} ViewContext;
