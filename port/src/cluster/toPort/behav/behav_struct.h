
/*
*************************************
**            STRUCTURES           **
*************************************
*/
typedef struct {
    float	x,y;
} FCoord;

typedef struct spikedata_type {
    unsigned long	timestamp;
    /*
    int			index;
    */
} SpikeData;

typedef struct position_type {
    unsigned long	timestamp;
    short		xy[4];
    int			direction;
    short		valid;
} Position;

typedef struct positionlist_type {
    Position		*positionarray;
    int			npositions;
    int			nprocessedpositions;
    FILE		*fp;
    char		*fname;
    long		headersize;
} PositionList;

typedef struct minmaxtheta_type {
    int		mintheta;
    int		maxtheta;
} MinMaxTheta;

typedef struct angular_range_map_type {
    int		xsize;
    int		ysize;
    MinMaxTheta	**grid;
} AngularRangeMap;


typedef struct spikelist_type {
    SpikeData 		*data;
    int			nspikes;
    int			timeshift;
    int			currentspike;
    int			cumspikes;
    FILE		*fp;
    char		*fname;
    long		headersize;
    int			convert;
    unsigned long	tstart;
    unsigned long	tend;
} SpikeList;

typedef struct trange_type {
    unsigned long	tstart;
    unsigned long	tend;
    struct trange_type	*next;
} TRange;

typedef struct result_type {
    FILE		*fp;
    FILE		*fpvar;
    FILE		*fpdir;
    FILE		*fpocc;
    FILE		*fprate;
    FILE		*fpangularrange;
    FILE		*fpfix;
    FILE		*fptrange;
    FILE		*fpreport;
    char		*pfname;
    char		*fpoutname;
    char		*fpoccname;
    char		*fpvarname;
    char		*fpdirname;
    char		*fpboundname;
    char		*fpratename;
    char		*fpangularrangename;
    char		*fptrangename;
    char		*fpfixname;
    float		**grid;
    float		**gridsumsqr;
    int			**gridn;
    int			**gridtotaln;
    float		*vector;
    float		*vectorsumsqr;
    int			*vectorn;
    int			*vectortotaln;
    float		**occupancy_grid;
    float		**occupancy_gridsqr;
    int			**occupancy_gridn;
    float		**field_estimate;
    int			nclusters;
    int			xoffset;
    int			yoffset;
    int			xsize;
    int			ysize;
    int			dsize;
    int			binsize;
    int			mintheta;
    int			maxtheta;
    int			savemode;
    int			behavmode;
    int			format;
    int			spatial_measure;
    int			upperthresh;
    int			lowerthresh;
    int			normalize;
    int			resulttype;
    int			positionmissing;
    int			directionmissing;
    int			exceededradius;
    int			gapflip;
    int			corrected_gapflips;
    int			corrected_flips;
    int			consecutivemissing;
    int			maxconsecutive;
    int			nofrontdiode;
    int			nobackdiode;
    int			validposition;
    int			validdirection;
    int			ndirectiongaps;
    float		directionsum;
    float		flipdirectionsum;
    float		sumvelocitydiff;
    int			nvelocityang;
    int			nvangdiff;
    int			nvmagdiff;
    int			nvmagok;
    unsigned long	occupancystart;
    int			occupancyspikes;
    int			occx;
    int			occy;
    float		occradius;			
    int			occdir;
    float		occdirangle;
    int			limited_occupancy;
    int			rotation;
    FILE		*fpbound;
    int			bound_x1;
    int			bound_x2;
    int			bound_y1;
    int			bound_y2;
    int			hasbounds;
    int			enable_occdirection_change;
    int			positioneval;
    AngularRangeMap	*angular_range_map;
    int			*xhistory;
    int			*yhistory;
    int			historysize;
    int			xviewformat;
    int			plotmode;
    int			smooth;
    TRange		*trange;
    int			directionline;
    int			totaltime;
    float		minrate;
    int			minpsamples;
    int			ignore_missing_back_diode;
    int			ignore_missing_front_diode;
    int			timeshift;
    float		minvelocity;
    float		maxvelocity;
	 float		velspan;
    int			noflip;
    int			useheaderrange;
    float		fieldthresh;
    FCoord		*fcoord;
    int			ncoords;
    float		pinterval;
    int			ascii;
    float               xres;
    float               yres;
} Result;

