#include "pop_ext.h"

ProcessArgs(argc,argv,result)
int	argc;
char	**argv;
Result	*result;
{
int	nxtarg;
char	line[1000];
long	seed;
FILE	*fp;

    /*
    ** zero out the Result structure
    */
    bzero(result,sizeof(Result));
    strcpy(result->prefix,"t");
    result->xresolution = XPOSITION_RESOLUTION;
    result->yresolution = YPOSITION_RESOLUTION;
    result->spatially_restrict_corr = 0;
    result->limitclusters = 0;
    result->rawhist = 0;
    result->posinterpolate = 0;
    result->skew_window = 0;
    result->gerstein = 0;
    result->make_trajectory_tree = 0;
    result->requested_node_id = -1;	/* output all nodes in the tree */
    result->reconstruct = 0;
    result->replay_fields = 0;
    result->spike_correlations = 0;
    result->loadspikes = 0;
    result->binspikes = 0;
    result->useallclusters = 0;
    result->fprange = NULL;
    result->fpstatout = NULL;
    result->fpdir = NULL;
    result->fppdir = NULL;
    result->fpout = NULL;
    result->fppout = NULL;
    result->fpmagout = NULL;
    result->fptreeout = NULL;
    result->fpsparsegridout = NULL;
    result->fpsparsevecout = NULL;
    result->fpperrgrid = NULL;
    result->fppouterr = NULL;
    result->fpcorrout = NULL;
    result->fpspeedout = NULL;
    result->position = NULL;
    result->fpreplayout = NULL;
    result->fpreplaydot = NULL;
    result->fpreplaycorr = NULL;
    result->fpfieldcenter = NULL;
    result->fpcorrhistout = NULL;
    result->fppoptrainout = NULL;
    result->fppopvecout = NULL;
    result->fpparms = NULL;
    result->useprefix = 1;
    result->peakcutoff = 0;
    result->smoothspikes = 0;

    if((result->fperr = fopen("popanal.err","w")) == NULL){
	fprintf(stderr,"WARNING: unable to open error output file 'popanal.err'\n");
    }
    result->popthresh = 0;
    result->speed = NULL;
    result->normalize = 0;
    result->peaknormalize = 0;
    result->positiongrid = NULL;
    result->vshuffle = 0;
    result->lump = 0;
    result->hasposition = 0;
    result->nactivep = 0;
    result->nactivec = 0;
    result->nreconactivec = 0;
    result->tstart = 0;
    result->tend = 0;
    result->recontstart = 0;
    result->recontend = 0;
    result->comparepositions = 0;
    result->hasspatialfiring = 0;
    result->pdir = (ClusterDir *)calloc(MAXCLUSTERS,sizeof(ClusterDir));
    result->sparsity_cutoff = 0;
    result->has_tstart = 0;
    result->has_tend = 0;
    result->reconhas_tstart = 0;
    result->reconhas_tend = 0;
    result->xlo = -1;
    result->ylo = -1;
    result->xhi = -1;
    result->yhi = -1;
    result->gxlo = -1;
    result->gylo = -1;
    result->gxhi = -1;
    result->gyhi = -1;
    result->smoothcorr = 0;
    result->smoothgrid = 0;
    result->meanpositionerror;
    result->minpeak = 0;
    result->occcutoff = 0;
    result->fraction = 0;
    result->traversal_threshold = 0;
    result->maxtraversal = 100;
    result->treeoutputformat = TREE;
    result->replay_threshold = 0;
    result->spike_bin_offset = 0;
    result->position_bin_offset = 0;
    result->suppress_replay_firing = 0;
    result->position_bin_avg = 0;
    result->replay_optimize = OPTIMIZE_DISTANCE;
    result->peak_search_window = 0;
    result->minspikes = 0;
    result->maxspikes = 0;
    result->minrate = 0;
    result->maxrate = 0;
    result->min_corr_events = 0;
    result->min_totalcorr_events = 0;
    result->peakfrac = 0;		/* half-max peak cutoff */ 
    result->corrtest = 0.97;
    result->pshuffle = 0;
    result->maxranges = MAXRANGES;
    result->corrhistsignbias = 0;
    result->zerosqueeze = 0;
    result->skewnormalize = 0;
    result->skewcount = 0;
    result->asciiout = 0;
    result->zerolevel = 0;
    result->bayesian = 0;
    result->range = (Range *)calloc(result->maxranges,sizeof(Range));
    time(&seed);
    nxtarg = 0;
    while(++nxtarg < argc){
	if(strcmp(argv[nxtarg],"-usage") == 0){
	    fprintf(stderr,
	    "usage: %s dirfile -binsize msec [options]\n",
	    argv[0]);
	    fprintf(stderr,
	    "Available options:\n");
	    fprintf(stderr,
	    "\t-version\n");
	    fprintf(stderr,
	    "\t-tstart timestamp\n");
	    fprintf(stderr,
	    "\t-tend timestamp\n");
	    fprintf(stderr,"\t[-v][-norm][-lump][-pdir pdirfile]\n");
	    fprintf(stderr,"\t[-o file]		xviewable popvector vs time\n");
	    fprintf(stderr,"\t[-asciio file]	ascii popvector vs time\n");
	    fprintf(stderr,"\t-parmdir file		parameter input file\n");
	    fprintf(stderr,
	    "\t-smoothspikes nbins 	smooth each spike train by n bins\n");
	    fprintf(stderr,
	    "\t-peaknorm 	normalize vector components by position grid peak\n");
	    fprintf(stderr,"\t-skewwin		number of bins from zero to skip\n");
	    fprintf(stderr,"\t-skewnorm		use normalized correlogram for skew\n");
	    fprintf(stderr,"\t-skewcount	dont normalize skew value\n");
	    fprintf(stderr,"\t-rawhist		unnormalized correlation histogram output\n");
	    fprintf(stderr,
	    "\t-corrhisto file		correlation histogram output file\n");
	    fprintf(stderr,
	    "\t-corrhistbias 		correlation histogram sign bias\n");
	    fprintf(stderr,
	    "\t-po file		position reconstruction output file\n");
	    fprintf(stderr,
	    "\t-posinterpolate	average front/back diode in reconstruction\n");
	    fprintf(stderr,
	    "\t-co file		popvector correlation grid vs time output file\n");
	    fprintf(stderr,
	    "\t-do file		position reconstruction distance vs time file\n");
	    fprintf(stderr,
	    "\t-xo file		crosscorrelation matrix output file\n");
	    fprintf(stderr,
	    "\t-mo file		grid magnitude output file\n");
	    fprintf(stderr,
	    "\t-sgrido file		grid sparsity output file\n");
	    fprintf(stderr,
	    "\t-sveco file		population vector sparsity output file\n");
	    fprintf(stderr,
	    "\t-sc frac		sparsity cutoff ratio\n");
	    fprintf(stderr,
	    "\t-p pfile		input position file\n");
	    fprintf(stderr,
	    "\t-perro file		position error grid output file\n");
	    fprintf(stderr,
	    "\t-bound xmin ymin xmax ymax\n");
	    fprintf(stderr,
	    "\t-xresolution xsize of the grid");
	    fprintf(stderr,
	    "\t-yresolution ysize of the grid");
	    fprintf(stderr,
	    "\t-smoothcorr nbins\n");
	    fprintf(stderr,
	    "\t-smoothgrid nbins\n");
	    fprintf(stderr,
	    "\t-useallclusters\n");
	    fprintf(stderr,
	    "\t-limitclusters n		use only the first n clusters\n");
	    fprintf(stderr,
	    "\t-minpeak rate\n");
	    fprintf(stderr,
	    "\t-minrate n		min spikes/sec for cluster to be processed\n");
	    fprintf(stderr,
	    "\t-maxrate n		max spikes/sec for cluster to be processed\n");
	    fprintf(stderr,
	    "\t-minspikes n		min number of spikes for cluster to be processed\n");
	    fprintf(stderr,
	    "\t-maxspikes n		max number of spikes for cluster to be processed\n");
	    fprintf(stderr,
	"\t-mincorrevents n	min number of events for a peak correlation\n"); 
	    fprintf(stderr,
	"\t-mintotalcorrevents n	min number of total events for valid correlogram\n"); 
	    fprintf(stderr,"\t\t\t\tto be processed\n");
	    fprintf(stderr,
	"\t-significance value	significance cutoff for peak correlation\n");
	    fprintf(stderr,"\t\t\t\tto be processed\n");
	    fprintf(stderr,"\t-fraction value\n");
	    fprintf(stderr,"\t-prefix str\n");
	    fprintf(stderr,"\t-noprefix\n");
	    fprintf(stderr,"\t-gerstein\n");
	    fprintf(stderr,"\t-randseed val\n");
	    fprintf(stderr,"\t-sortcorr\n");
	    fprintf(stderr,"\t-overlapcorr\n");
	    fprintf(stderr,"\t-bayeszero	nspikes\n");
	    fprintf(stderr,"\t-bayesgrid ymax xmax file	bayesian reconstruction grd output file\n");
	    fprintf(stderr,"\t-compare			compare bayesian and actual traj\n");
	    fprintf(stderr,"\t-bayeso file		bayesian reconstruction output file\n");
	    fprintf(stderr,"\t-bayesdir file		dir file reconstruction\n");
	    fprintf(stderr,"\t-trangefile file		time range input file\n");
	    fprintf(stderr,"\t-cploto file		correlation plot output file\n");
	    fprintf(stderr,"\t-stato file		clusters stats output file\n");
	    fprintf(stderr,"\t-popveco file		population vector output file\n");
	    fprintf(stderr,"\t-popthresh val		rate threshold for population vector output file\n");
	    fprintf(stderr,"\t-popo file		population mean output file\n");
	    fprintf(stderr,"\t-traino file		spike train output file\n");
	    fprintf(stderr,"\t-speedo file		running speed output file\n");
	    fprintf(stderr,"\t-occcutoff noccupancies\n");
	    fprintf(stderr,
	    "\t-spikecorro file	spike corr analysis output file\n");
	    fprintf(stderr,"\t-vshuffle		shuffle vector between clusters\n");
	    fprintf(stderr,"\t-pshuffle		shuffle fields between clusters\n");
	    fprintf(stderr,"\t-zerosqueeze 	zerowidth zerobin\n");
	    fprintf(stderr,"\t-tmax msec\n");
	    fprintf(stderr,
	    "\t-peaksearch nbins	+-size of peak phase search window\n");
	    fprintf(stderr,"\t-trajectorytree\n");
	    fprintf(stderr,"\t-maxtraversal depth\n");
	    fprintf(stderr,"\t-traversalthresh nspikes\n");
	    fprintf(stderr,"\t-treeo file		trajectory tree output file\n");
	    fprintf(stderr,"\t-treeformat list tree xplot\n");
	    fprintf(stderr,
	    "\t-treenode nodeid	node for tree output\n");
	    fprintf(stderr,
	    "\t-replayo file		file for spike field output\n");
	    fprintf(stderr,"\t-replaythresh nspikes\n");
	    fprintf(stderr,
	    "\t-corrtest fraction	warn about zero lag xcorrs greater\n");
	    fprintf(stderr,"\t\t\t\tthan 'fraction' of the autocorr (def=0.95)\n");
	    fprintf(stderr,
	    "\t-withinprobe		compute correlations only within probes\n");
	    fprintf(stderr,
	    "\t-betweenprobe		compute correlations only between probes\n");
	    fprintf(stderr,
	    "\t-showallcorrs		output zero entries for invalid correlations\n");
	    fprintf(stderr,
	    "\t-fieldcentero file	file for field center output\n");
	    fprintf(stderr,"\t-optimizedist\n");
	    fprintf(stderr,"\t-optimizeang\n");
	    fprintf(stderr,
	    "\t-replaydoto file	file for replay dot product output\n");
	    fprintf(stderr,
	    "\t-suppressfiring		dont display firing during replay\n");
	    fprintf(stderr,
	    "\t-spikeoffset nbins	offset the spike train by nbins\n");
	    fprintf(stderr,
	    "\t-positionoffset nbins	compute actual trajectory vector during \n");
	    fprintf(stderr,
	    "\t\treplay using the position offset by this number of bins\n");
	    fprintf(stderr,
	    "\t-positionsearch nbins	compute best fit trajectory vector during replay\n");
	    fprintf(stderr,
	    "\t\t\t\tsearching the position offsets by +- this number of bins\n");
	    fprintf(stderr,
	    "\t-positionavg nbins	compute avg trajectory vector during replay\n");
	    fprintf(stderr,
	    "\t\t\t\tusing this number of bins in the future\n");
	    fprintf(stderr,"dirfile format:\n");
	    fprintf(stderr,"	dir/???n\n");
	    fprintf(stderr,"	dir/???n\n");
	    fprintf(stderr,"	...\n");
	    fprintf(stderr,"	where ? is any character and\n"); 
	    fprintf(stderr,"	and n is the cluster number\n"); 
	    fprintf(stderr,"pdirfile format:\n");
	    fprintf(stderr,"	dir/?n\n");
	    fprintf(stderr,"	dir/?n\n");
	    fprintf(stderr,"	...\n");
	    fprintf(stderr,"	where ? is any character and\n"); 
	    fprintf(stderr,"	and n is the cluster number\n"); 
	    exit(-1);
	} else
	if(strcmp(argv[nxtarg],"-version") == 0){
	    fprintf(stderr,"%s : version %s : updated %s\n",
		argv[0],VERSION,DATE);

/* passed in from makefile when building an RPM*/
#ifdef MWSOFT_RPM_STRING 
	    fprintf(stderr,"From RPM: %s\n",
		    MWSOFT_RPM_STRING);
#endif

	    fprintf(stderr,"%s\n",COPYRIGHT);
	    exit(0);
	} else
	if(strcmp(argv[nxtarg],"-v") == 0){
	    verbose = 1;
	} else
	if(strcmp(argv[nxtarg],"-norm") == 0){
	    result->normalize = 1;
	} else
	if(strcmp(argv[nxtarg],"-peaknorm") == 0){
	    result->peaknormalize = 1;
	} else
	if(strcmp(argv[nxtarg],"-rawhist") == 0){
	    result->rawhist = 1;
	} else
	if(strcmp(argv[nxtarg],"-corrhistbias") == 0){
	    result->corrhistsignbias = 1;
	} else
	if(strcmp(argv[nxtarg],"-useallclusters") == 0){
	    result->useallclusters = 1;
	} else
	if(strcmp(argv[nxtarg],"-zerosqueeze") == 0){
	    result->zerosqueeze = 0.5;
	} else
	if(strcmp(argv[nxtarg],"-prefix") == 0){
	    strcpy(result->prefix,argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-noprefix") == 0){
	    result->useprefix = 0;
	} else
	if(strcmp(argv[nxtarg],"-posinterpolate") == 0){
	    result->posinterpolate = 1;
	} else
	if(strcmp(argv[nxtarg],"-skewnorm") == 0){
	    result->skewnormalize = 1;
	} else
	if(strcmp(argv[nxtarg],"-skewcount") == 0){
	    result->skewcount = 1;
	} else
	if(strcmp(argv[nxtarg],"-lump") == 0){
	    result->lump = 1;
	} else
	if(strcmp(argv[nxtarg],"-overlapcorr") == 0){
	    result->spatially_restrict_corr = 1;
	} else
	if(strcmp(argv[nxtarg],"-gerstein") == 0){
	    result->gerstein = 1;
	    result->binspikes = 1;
	} else
	if(strcmp(argv[nxtarg],"-sortcorr") == 0){
	    result->sortcorr = 1;
	} else
	if(strcmp(argv[nxtarg],"-trajectorytree") == 0){
	    result->make_trajectory_tree = 1;
	    result->binspikes = 1;
	} else
	if(strcmp(argv[nxtarg],"-treenode") == 0){
	    result->requested_node_id = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-treeformat") == 0){
	    if(strcmp(argv[++nxtarg],"xplot") == 0){
		result->treeoutputformat = XPLOT;
	    } else
	    if(strcmp(argv[++nxtarg],"list") == 0){
		result->treeoutputformat = LIST;
	    } else
	    if(strcmp(argv[++nxtarg],"tree") == 0){
		result->treeoutputformat = TREE;
	    }
	} else
	if(strcmp(argv[nxtarg],"-bayeszero") == 0){
	    result->zerolevel = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-smoothspikes") == 0){
	    result->smoothspikes = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-maxtraversal") == 0){
	    result->maxtraversal = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-traversalthresh") == 0){
	    result->traversal_threshold = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-replaythresh") == 0){
	    result->replay_threshold = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-suppressfiring") == 0){
	    result->suppress_replay_firing = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-optimizeang") == 0){
	    result->replay_optimize = OPTIMIZE_ANGLE;
	} else
	if(strcmp(argv[nxtarg],"-optimizedist") == 0){
	    result->replay_optimize = OPTIMIZE_DISTANCE;
	} else
	if(strcmp(argv[nxtarg],"-limitclusters") == 0){
	    result->limitclusters = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-spikeoffset") == 0){
	    result->spike_bin_offset = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-positionoffset") == 0){
	    result->position_bin_offset = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-positionsearch") == 0){
	    result->position_bin_search = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-peaksearch") == 0){
	    result->peak_search_window = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-peakwidth") == 0){
	    result->peakwidth = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-popthresh") == 0){
	    result->popthresh = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-significance") == 0){
	    result->peakcutoff = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-positionavg") == 0){
	    result->position_bin_avg = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-pshuffle") == 0){
	    result->pshuffle = 1;
	} else
	if(strcmp(argv[nxtarg],"-vshuffle") == 0){
	    result->vshuffle = 1;
	} else
	if(strcmp(argv[nxtarg],"-tmax") == 0){
	    result->tmax = atof(argv[++nxtarg])*10;
	} else
	if(strcmp(argv[nxtarg],"-tstart") == 0){
	    result->tstart = ParseTimestamp(argv[++nxtarg]);
	    result->has_tstart = 1;
	} else
	if(strcmp(argv[nxtarg],"-tend") == 0){
	    result->tend = ParseTimestamp(argv[++nxtarg]);
	    result->has_tend = 1;
	} else
	if(strcmp(argv[nxtarg],"-binsize") == 0){
	    result->binsize = atof(argv[++nxtarg])*10;
	} else
	if(strcmp(argv[nxtarg],"-randseed") == 0){
	    seed = atoi(argv[++nxtarg])*10;
	} else
	if(strcmp(argv[nxtarg],"-xresolution") == 0){
	    result->xresolution = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-yresolution") == 0){
	    result->yresolution = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-withinprobe") == 0){
	    result->withinprobe = 1;
	} else
	if(strcmp(argv[nxtarg],"-betweenprobe") == 0){
	    result->betweenprobe = 1;
	} else
	if(strcmp(argv[nxtarg],"-showallcorrs") == 0){
	    result->showallcorrs = 1;
	} else
	if(strcmp(argv[nxtarg],"-occcutoff") == 0){
	    result->occcutoff = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-fraction") == 0){
	    result->fraction = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-minpeak") == 0){
	    result->minpeak = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-minspikes") == 0){
	    result->minspikes = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-maxspikes") == 0){
	    result->maxspikes = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-minrate") == 0){
	    /*
	    ** normalize to spikes/100 usec
	    */
	    result->minrate = atof(argv[++nxtarg])/1e4;
	} else
	if(strcmp(argv[nxtarg],"-maxrate") == 0){
	    /*
	    ** normalize to spikes/100 usec
	    */
	    result->maxrate = atof(argv[++nxtarg])/1e4;
	} else
	if(strcmp(argv[nxtarg],"-mincorrevents") == 0){
	    result->min_corr_events = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-mintotalcorrevents") == 0){
	    result->min_totalcorr_events = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-peakfrac") == 0){
	    result->peakfrac = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-corrtest") == 0){
	    result->corrtest = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-skewwin") == 0){
	    result->skew_window = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-compare") == 0){
	    result->comparepositions = 1;
	} else
	if(strcmp(argv[nxtarg],"-trangefile") == 0){
	    if((result->fprange = fopen(argv[++nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to open trangefile '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-bayeso") == 0){
	    result->bayesian = 1;
	    if((result->fppout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create bayesian output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else
	if(strcmp(argv[nxtarg],"-asciio") == 0){
	    result->asciiout = 1;
	    if((result->fpout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create vector output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else
	if(strcmp(argv[nxtarg],"-o") == 0){
	    if((result->fpout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create vector output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-do") == 0){
	    if((result->fpdout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create distance output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else
	if(strcmp(argv[nxtarg],"-po") == 0){
	    if((result->fppout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create position output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    sprintf(line,"%s.err",argv[nxtarg]);
	    if((result->fppouterr = fopen(line,"w")) == NULL){
		fprintf(stderr,"unable to create position error distribution file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->reconstruct = 1;
	    result->binspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-treeo") == 0){
	    if((result->fptreeout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create trajectory tree output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-corrhisto") == 0){
	    if((result->fpcorrhistout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create corr output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->spike_correlations = 1;
	    result->loadspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-cploto") == 0){
	    if((result->fpcorrplotout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create corr plot output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-xo") == 0){
	    if((result->fpcorrmatrixout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create corr matrix output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-co") == 0){
	    if((result->fpcorrout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create corr output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->reconstruct = 1;
	    result->binspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-mo") == 0){
	    if((result->fpmagout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create magnitude output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-spikecorro") == 0){
	    if((result->fpspikecorr = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create spike corr output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->spike_correlations = 1;
	    result->loadspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-fieldcentero") == 0){
	    if((result->fpfieldcenter = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create field center output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-sc") == 0){
	    result->sparsity_cutoff = atof(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-bayesgrid") == 0){
	    result->ymax = atoi(argv[++nxtarg]);
	    result->xmax = atoi(argv[++nxtarg]);
	    result->xlo = 0;
	    result->ylo = 0;
	    result->xhi = result->xmax;
	    result->yhi = result->ymax;
	    result->gxlo = 0;
	    result->gylo = 0;
	    result->gxhi = result->xmax;
	    result->gyhi = result->ymax;
	    if((result->fpcorrout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create corr output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else
	if(strcmp(argv[nxtarg],"-bound") == 0){
	    result->xlo = atoi(argv[++nxtarg]);
	    result->ylo = atoi(argv[++nxtarg]);
	    result->xhi = atoi(argv[++nxtarg]);
	    result->yhi = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-posbound") == 0){
	    result->gxlo = atoi(argv[++nxtarg]);
	    result->gylo = atoi(argv[++nxtarg]);
	    result->gxhi = atoi(argv[++nxtarg]);
	    result->gyhi = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-smoothcorr") == 0){
	    result->smoothcorr = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-smoothgrid") == 0){
	    result->smoothgrid = atoi(argv[++nxtarg]);
	} else
	if(strcmp(argv[nxtarg],"-replaycorro") == 0){
	    if((result->fpreplaycorr = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create replay corr output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->replay_fields = 1;
	    result->binspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-replaydoto") == 0){
	    if((result->fpreplaydot = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create replay dot output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->replay_fields = 1;
	    result->binspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-replayo") == 0){
	    if((result->fpreplayout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create replay output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->replay_fields = 1;
	    result->binspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-stato") == 0){
	    if((result->fpstatout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create cluster stats output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->loadspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-speedo") == 0){
	    if((result->fpspeedout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create running speed output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-popveco") == 0){
	    if((result->fppopvecout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create population output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->binspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-popo") == 0){
	    if((result->fppoptrainout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create population output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->binspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-traino") == 0){
	    if((result->fpspiketrainout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create spike train output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	    result->loadspikes = 1;
	} else 
	if(strcmp(argv[nxtarg],"-sgrido") == 0){
	    if((result->fpsparsegridout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create grid sparsity output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-sveco") == 0){
	    if((result->fpsparsevecout = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create vector sparsity output file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-perro") == 0){
	    if((result->fpperrgrid = fopen(argv[++nxtarg],"w")) == NULL){
		fprintf(stderr,"unable to create position error file '%s'\n",
		argv[nxtarg]);
		exit(-1);
	    }
	} else 
	if(strcmp(argv[nxtarg],"-p") == 0){
	    if((result->fppfile = fopen(argv[++nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read position file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	    result->hasposition = 1;
	} else 
	if(strcmp(argv[nxtarg],"-parmdir") == 0){
	    if((result->fpparms = fopen(argv[++nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read parameter file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	    result->hasparameters = 1;
	} else 
	if(strcmp(argv[nxtarg],"-pdir") == 0){
	    if((result->fppdir = fopen(argv[++nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read pdir file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	    result->comparepositions = 1;
	    result->hasspatialfiring = 1;
	} else 
	if(strcmp(argv[nxtarg],"-bayes") == 0){
	    if((result->fpdirrecon = fopen(argv[++nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read recon dir file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	    result->bayesian = 1;
	    result->binspikes = 1;
	} else 
	if(argv[nxtarg][0] != '-'){
	    if((result->fpdir = fopen(argv[nxtarg],"r")) == NULL){
		fprintf(stderr,"unable to read dir file '%s'\n",argv[nxtarg]);
		exit(-1);
	    }
	} else {
	    fprintf(stderr,"invalid option '%s'\n",argv[nxtarg]);
	    exit(-1);
	}
    }
    /*
    ** initialize the random number generator
    */
    srandom(seed);
    srand48(seed);
}
