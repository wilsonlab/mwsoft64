typedef struct {
    float x,y,z;
} Point;

typedef struct {
    int32_t	pixel;
    int32_t	r,g,b;
} ColorTable;

typedef struct {
    Point	p1,p2,p3;
    float 	a,b,c,d;
    float	zmin,zmax;
} Plane;

typedef struct {
    Point	p1,p2;
    float 	a,b,c;
    int32_t		infinite;
} Line;

typedef struct	image_type {
    float	value;
    int16_t	sign;
} Image;

typedef struct {
    int32_t	ncontours;
    float value[MAXCONTOURS];
} ContourList;

typedef struct {
    int16_t	x,y;
} Coord;

typedef struct {
    Display		*display;
    GC			context;
    Drawable		drawable;
    Visual		*visual;
    int32_t			screen_number;
    Font		font;
    XFontStruct		*fontinfo;
    Window		imagewindow;
    int32_t			wwidth;
    int32_t			wheight;
    int32_t			fontheight;
    int32_t			fontwidth;
    int32_t			linewidth;
    uint32_t	background;
    uint32_t	foreground;
    uint32_t	color;
    int32_t			scale_type;
    int32_t			mapped;
    int32_t			inverse;
} GlobalContext;

typedef struct {
    char		*filename;
    int32_t			headersize;
    float		start_time;
    float		dt;
    int32_t			xmax;
    int32_t			ymax;
    int32_t			cellnum;
    int32_t			datatype;
    int32_t			datasize;
    float		view_time;
    float		view_step;
    int32_t			display_mode;
    int32_t			singlestep;
    int32_t			plain;
    int32_t			valid_frame;
    int32_t			posdata;
} ViewContext;
