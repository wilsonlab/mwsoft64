
/*
*************************************
**            STRUCTURES           **
*************************************
*/
typedef struct {
    float	x,y;
} FCoord;

typedef struct spikedata_type {
    uint32_t	timestamp;
    /*
    int32_t			index;
    */
} SpikeData;

typedef struct position_type {
    uint32_t	timestamp;
    int16_t		xy[4];
    int32_t			direction;
    int16_t		valid;
} Position;

typedef struct positionlist_type {
    Position		*positionarray;
    int32_t			npositions;
    int32_t			nprocessedpositions;
    FILE		*fp;
    char		*fname;
    int32_t		headersize;
} PositionList;

typedef struct minmaxtheta_type {
    int32_t		mintheta;
    int32_t		maxtheta;
} MinMaxTheta;

typedef struct angular_range_map_type {
    int32_t		xsize;
    int32_t		ysize;
    MinMaxTheta	**grid;
} AngularRangeMap;


typedef struct spikelist_type {
    SpikeData 		*data;
    int32_t			nspikes;
    int32_t			timeshift;
    int32_t			currentspike;
    int32_t			cumspikes;
    FILE		*fp;
    char		*fname;
    int32_t		headersize;
    int32_t			convert;
    uint32_t	tstart;
    uint32_t	tend;
} SpikeList;

typedef struct trange_type {
    uint32_t	tstart;
    uint32_t	tend;
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
    int32_t			**gridn;
    int32_t			**gridtotaln;
    float		*vector;
    float		*vectorsumsqr;
    int32_t			*vectorn;
    int32_t			*vectortotaln;
    float		**occupancy_grid;
    float		**occupancy_gridsqr;
    int32_t			**occupancy_gridn;
    float		**field_estimate;
    int32_t			nclusters;
    int32_t			xoffset;
    int32_t			yoffset;
    int32_t			xsize;
    int32_t			ysize;
    int32_t			dsize;
    int32_t			binsize;
    int32_t			mintheta;
    int32_t			maxtheta;
    int32_t			savemode;
    int32_t			behavmode;
    int32_t			format;
    int32_t			spatial_measure;
    int32_t			upperthresh;
    int32_t			lowerthresh;
    int32_t			normalize;
    int32_t			resulttype;
    int32_t			positionmissing;
    int32_t			directionmissing;
    int32_t			exceededradius;
    int32_t			gapflip;
    int32_t			corrected_gapflips;
    int32_t			corrected_flips;
    int32_t			consecutivemissing;
    int32_t			maxconsecutive;
    int32_t			nofrontdiode;
    int32_t			nobackdiode;
    int32_t			validposition;
    int32_t			validdirection;
    int32_t			ndirectiongaps;
    float		directionsum;
    float		flipdirectionsum;
    float		sumvelocitydiff;
    int32_t			nvelocityang;
    int32_t			nvangdiff;
    int32_t			nvmagdiff;
    int32_t			nvmagok;
    uint32_t	occupancystart;
    int32_t			occupancyspikes;
    int32_t			occx;
    int32_t			occy;
    float		occradius;			
    int32_t			occdir;
    float		occdirangle;
    int32_t			limited_occupancy;
    int32_t			rotation;
    FILE		*fpbound;
    int32_t			bound_x1;
    int32_t			bound_x2;
    int32_t			bound_y1;
    int32_t			bound_y2;
    int32_t			hasbounds;
    int32_t			enable_occdirection_change;
    int32_t			positioneval;
    AngularRangeMap	*angular_range_map;
    int32_t			*xhistory;
    int32_t			*yhistory;
    int32_t			historysize;
    int32_t			xviewformat;
    int32_t			plotmode;
    int32_t			smooth;
    TRange		*trange;
    int32_t			directionline;
    int32_t			totaltime;
    float		minrate;
    int32_t			minpsamples;
    int32_t			ignore_missing_back_diode;
    int32_t			ignore_missing_front_diode;
    int32_t			timeshift;
    float		minvelocity;
    float		maxvelocity;
	 float		velspan;
    int32_t			noflip;
    int32_t			useheaderrange;
    float		fieldthresh;
    FCoord		*fcoord;
    int32_t			ncoords;
    float		pinterval;
    int32_t			ascii;
    float               xres;
    float               yres;
} Result;

