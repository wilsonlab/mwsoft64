/*
**************************************************
DESCRIPTION:
popanal
    performs a variety of analyses on population activity.
    Note that this is a VERY picky program when it comes to 
    specifying the source of spike information. This is
    designed to work with some very specific data sets and
    needs to be reworked for more general application

AUTHOR:
    Written by Matthew Wilson
    Massachusetts Institute of Technology
    Departments of Brain and Cognitive Sciences and Biology
    Cambridge MA 02139
    wilson@ai.mit.edu
DATES:
    original 4/92
    last update	9/96

**************************************************
*/

#include "pop.h"

main(argc,argv)
int	argc;
char 	**argv;
{
FILE	*fpin;
int	i,j;
char	dirname[80];
int	headersize;
char	**header;
Result	result;
int	count;
CorrResult	*corr_result;
int	testval;
int	tmpcount;


    verbose = 0;
    /*
    ** process command line arguments
    */
    ProcessArgs(argc,argv,&result);

    /*
    ************************* PREPARE *************************
    **
    ** Read and prepare the data for analysis 
    **
    **********************************************************
    */
    if(result.spatially_restrict_corr && (!result.hasspatialfiring || !result.hasposition)){
	fprintf(stderr,"ERROR: must specify -p and -pdir options with -overlapcorr\n");
	exit(-1);
    }
    /*
    ** read the time range file information
    */
    if(result.fprange){
	ReadRange(result.fprange,&result);
	fclose(result.fprange);
	if(verbose){
	    fprintf(stderr,"Read %d time ranges\n",result.nranges);
	}
    }
    /*
    ** go through the file list and construct the index
    */
    if(result.fpdir == NULL){
	fprintf(stderr,"%s: must provide a cluster directory file\n",argv[0]);
	exit(-1);
    }
    if(result.binsize <= 0){
	fprintf(stderr,"ERROR: must provide nonzero binsize\n");
	exit(-1);
    }
    /*
    ** read the header of the cluster directory file to find the
    ** start and end times
    */
    ReadClusterDirectoryHeader(&result,result.has_tstart,result.has_tend);

    /*
    ** read the header of the position directory file to find the
    ** bounding regions
    */
    if(result.fppdir){
	ReadPositionDirectoryHeader(&result);
    }

    /*
    ** compute the number of time bins required
    */
    result.ntimebins = (result.tend - result.tstart)/result.binsize;
    if(result.ntimebins <= 0){
	fprintf(stderr,"ERROR: invalid time range\n");
	exit(-1);
    }
    if(result.ntimebins > MAXTIMEBINS){
	fprintf(stderr,"ERROR: too many time bins (%d) max is %d\n",
	result.ntimebins,MAXTIMEBINS);
	exit(-1);
    }

    /*
    ** read the entries of the directory file
    */
    ReadClusterDirectory(&result);

    /*
    ** read in parameters if available
    */
    if(result.hasparameters){
	ReadParameterDirectory(&result);
    }

    /*
    ** read the entries of the position directory file
    */
    if(result.fppdir){
	ReadPositionDirectory(&result);
    }

    if(verbose){
	fprintf(stderr,"\nAllocating data arrays...");
    }
    /*
    ** allocate the grid array
    */
    if((result.positiongrid = 
    (float ***)calloc(result.nclusters,sizeof(float **))) == NULL){
	fprintf(stderr,
	"ERROR: out of memory. Unable to allocate position grid\n");
	exit(-1);
    }
    if(result.binspikes){
	/*
	** allocate the binned spike array
	*/
	if((result.datavector = (float **)malloc(result.ntimebins*
	sizeof(float *)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate binned spike data array\n");
	    exit(-1);
	}
	/*
	** allocate the timebin status array
	*/
	if((result.timebinstatus = (TimeBinStatus *)calloc(result.ntimebins,
	sizeof(TimeBinStatus)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate bin status array\n");
	    exit(-1);
	}
	for(j=0;j<result.ntimebins;j++){
	    if((result.datavector[j] = 
	    (float *)calloc(result.nclusters,sizeof(float))) == NULL){
		fprintf(stderr,
		"ERROR: out of memory. Unable to allocate spike data array\n");
		exit(-1);
	    }
	}
    }
    if(result.loadspikes){
	/*
	** allocate the spike time array
	*/
	if((result.spikearray = (unsigned long **)malloc(result.nclusters*
	sizeof(unsigned long *)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate spike time array\n");
	    exit(-1);
	}
	/*
	** allocate the spike statistics arrays
	*/
	if((result.binmean = (double *)calloc(result.nclusters,sizeof(double)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate bin mean array\n");
	    exit(-1);
	}
	if((result.binsd = (double *)calloc(result.nclusters,sizeof(double)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate bin sd array\n");
	    exit(-1);
	}
    }
    if(result.normalize){
	if((result.datasum = (float *)calloc(result.nclusters,sizeof(float)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate data array\n");
	    exit(-1);
	}
	if((result.datasqr = (float *)calloc(result.nclusters,sizeof(float)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate data array\n");
	    exit(-1);
	}
    }
    if(verbose){
	fprintf(stderr,"done\n");
    }
    if(result.hasspatialfiring){
	/*
	** open each position file and construct the position vectors
	*/
	for(i=0;i<result.nclusters;i++){
	    if(result.pdir[i].ignore) continue;
	    result.nactivep++;
	    /*
	    ** open the position file
	    */
	    sprintf(dirname,"%s",
	    result.pdir[i].path);
	    if((fpin = fopen(dirname,"r")) == NULL){
		fprintf(stderr,
		"ERROR: unable to open spatial firing file '%s'\n",
		dirname);
		exit(-1);
	    }
	    /*
	    ** fill the grid bins for the data vector component
	    */
	    result.positiongrid[i] = FillGrid(fpin,&result,i);
	    /*
	    ** close the data file and go the the next
	    */
	    fclose(fpin);
	}
    }

    /*
    ** if a dual diode position file has been specified then
    ** open it and read in the actual positions
    */
    if(result.hasposition){
	if(verbose){
	    fprintf(stderr,"Reading position file...");
	}
	/*
	** allocate the position array
	*/
	if((result.position = (DualPosition *)calloc(result.ntimebins,
	sizeof(DualPosition)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate position data array\n");
	    exit(-1);
	}
	/*
	** allocate speed array
	*/
	if((result.speed = (float *)calloc(result.ntimebins,
	    sizeof(float)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate velocity data array\n");
	    exit(-1);
	}
	/*
	** allocate the result.reconstructed position array
	*/
	if((result.calcposition = (Position *)calloc(result.ntimebins,
	    sizeof(Position)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate result.reconstructed position array\n");
	    exit(-1);
	}
	count = ReadPositions(&result);
	if(verbose){
	    fprintf(stderr,"filled %d position bins\n",count);
	}
    }

    /*
    ** open each timestamp file and bin the rate over the desired interval
    */
    if((result.spikecount = (int *)calloc(result.nclusters,sizeof(int)))
    == NULL){
	fprintf(stderr,
	"ERROR: out of memory. Unable to allocate spike count array\n");
	exit(-1);
    }
    if((result.clustername = (char **)malloc(result.nclusters*sizeof(char *)))
    == NULL){
	fprintf(stderr,
	"ERROR: out of memory. Unable to cluster name array\n");
	exit(-1);
    }
    if(verbose){
	fprintf(stderr,"\nReading spikes\n");
    }
    for(i=0;i<result.nclusters;i++){
	if(clusterdir[i].ignore) continue;
	/*
	** open the binary timestamp file
	** assume that the timestamp file is of the form dir/tn where n is
	** the cluster number
	*/
	if(result.useprefix){
	    sprintf(dirname,"%s/%s%d",
	    clusterdir[i].dirname,result.prefix,clusterdir[i].clusterid);
	} else {
	    sprintf(dirname,"%s/%s",
	    clusterdir[i].dirname,clusterdir[i].filename);
	}
	if((fpin = fopen(dirname,"r")) == NULL){
	    fprintf(stderr,"ERROR: unable to open cluster timestamp file '%s'\n",
	    dirname);
	    fprintf(stderr,"ignoring cluster\n");
	    clusterdir[i].ignore = 1;
	    continue;
	}
	result.nactivec++;
	/*
	** load and bin the individual spikes of the spike trains
	*/
	if(result.binspikes){
	    /*
	    ** fill the time bins for the data vector component
	    */
	    result.spikecount[i] = LoadAndBinSpikes(fpin,&result,i);
	}
	/*
	** smooth the spike trains
	*/
	if(result.smoothspikes){
	    /*
	    if(verbose) {
		fprintf(stderr,"Smoothing spike train\n");
	    }
	    ** fill the time bins for the data vector component
	    */
	    SmoothSpikes(fpin,&result,i);
	}
	/*
	** load the individual spikes of the spike trains
	*/
	if(result.loadspikes){
	    result.spikecount[i] = LoadSpikes(fpin,&result,i);
	}
	/*
	** close the data file and go the the next
	*/
	fclose(fpin);

	/*
	** apply the min and max spike criteria
	*/
	if((result.spikecount[i] < result.minspikes) && !result.showallcorrs) {
	    clusterdir[i].ignore = 1;
	    result.pdir[i].ignore = 1;
	    continue;
	}
	if(((result.maxspikes >0) && (result.spikecount[i] > result.maxspikes)) 
	&& !result.showallcorrs){
	    clusterdir[i].ignore = 1;
	    result.pdir[i].ignore = 1;
	    continue;
	}

	if(verbose){
	    fprintf(stderr,"%d: %d spikes in %s cluster %d\n",
	    i,result.spikecount[i],dirname,clusterdir[i].clusterid);
	}
    }
    if(result.limitclusters > 0){
	/*
	** limit the number of active clusters
	*/
	tmpcount = 0;
	for(i=0;i<result.nclusters;i++){
	    if(clusterdir[i].ignore) continue;
	    tmpcount++;
	    if(tmpcount >= result.limitclusters){
		result.nclusters = i+1;
		break;
	    }
	}
    }

    WriteOutputHeaders(&result,argc,argv);

    /*
    ** this is a special format for G. Gerstein's analysis
    */
    if(result.gerstein){
	ProcessGerstein(&result);
	fprintf(stderr,"done\n");
	exit(0);
    }
    if(result.bayesian){
	/*
	** read the header of the cluster directory file to find the
	** start and end times
	*/
	ReadReconClusterDirectoryHeader(&result,result.reconhas_tstart,result.reconhas_tend);

	/*
	** compute the number of time bins required
	*/
	result.nrecontimebins = (result.recontend - result.recontstart)/result.binsize;
	if(result.nrecontimebins <= 0){
	    fprintf(stderr,"ERROR: invalid time range\n");
	    exit(-1);
	}
	if(result.nrecontimebins > MAXTIMEBINS){
	    fprintf(stderr,"ERROR: too many time bins (%d) max is %d\n",
	    result.nrecontimebins,MAXTIMEBINS);
	    exit(-1);
	}

	/*
	** read the entries of the directory file
	*/
	ReadReconClusterDirectory(&result);
	/*
	** allocate the binned spike array
	*/
	if((result.reconvector = (float **)malloc(result.nrecontimebins*
	sizeof(float *)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate recon spike data array\n");
	    exit(-1);
	}
	for(j=0;j<result.nrecontimebins;j++){
	    if((result.reconvector[j] = 
	    (float *)calloc(result.nreconclusters,sizeof(float))) == NULL){
		fprintf(stderr,
		"ERROR: out of memory. Unable to allocate recon spike data array\n");
		exit(-1);
	    }
	}
	if((result.reconspikecount = (int *)calloc(result.nreconclusters,sizeof(int)))
	== NULL){
	    fprintf(stderr,
	    "ERROR: out of memory. Unable to allocate spike count array\n");
	    exit(-1);
	}
	for(i=0;i<result.nreconclusters;i++){
	    if(reconclusterdir[i].ignore) continue;
	    /*
	    ** open the binary timestamp file
	    ** assume that the timestamp file is of the form dir/tn where n is
	    ** the cluster number
	    */
	    if(result.useprefix){
		sprintf(dirname,"%s/%s%d",
		reconclusterdir[i].dirname,result.prefix,reconclusterdir[i].clusterid);
	    } else {
		sprintf(dirname,"%s/%s",
		reconclusterdir[i].dirname,reconclusterdir[i].filename);
	    }
	    if((fpin = fopen(dirname,"r")) == NULL){
		fprintf(stderr,"ERROR: unable to open recon cluster timestamp file '%s'\n",
		dirname);
		fprintf(stderr,"ignoring recon cluster\n");
		reconclusterdir[i].ignore = 1;
		continue;
	    }
	    result.nreconactivec++;
	    /*
	    ** load and bin the individual spikes of the spike trains
	    */
	    /*
	    ** fill the time bins for the data vector component
	    */
	    result.reconspikecount[i] = ReconLoadAndBinSpikes(fpin,&result,i);
	    /*
	    ** smooth the spike trains
	    */
	    if(result.smoothspikes){
		/*
		if(verbose) {
		    fprintf(stderr,"Smoothing spike train\n");
		}
		** fill the time bins for the data vector component
		*/
		SmoothSpikes(fpin,&result,i);
	    }
	    /*
	    ** close the data file and go the the next
	    */
	    fclose(fpin);

	    /*
	    ** apply the min and max spike criteria
	    */
	    if((result.reconspikecount[i] < result.minspikes) && !result.showallcorrs) {
		reconclusterdir[i].ignore = 1;
		continue;
	    }
	    if(((result.maxspikes >0) && (result.reconspikecount[i] > result.maxspikes)) 
	    && !result.showallcorrs){
		reconclusterdir[i].ignore = 1;
		continue;
	    }

	    if(verbose){
		fprintf(stderr,"%d: %d spikes in %s cluster %d\n",
		i,result.reconspikecount[i],dirname,reconclusterdir[i].clusterid);
	    }
	}
    }

    /*
    ** population vector train output
    */
    if(result.fppoptrainout || result.fppopvecout){
	ProcessPopulationSpikeTrain(&result);
	fprintf(stderr,"done\n");
	exit(0);
    }

    if(result.normalize){
	if(verbose){
	    fprintf(stderr,"Normalizing vector components...\n");
	}
	/*
	** compute component means
	*/
	ComputeComponentSums(&result);
	NormalizeVectors(&result);
    }
    AllocateCorrGrids(&result);
    /*
    ** compute spatial firing distribution features
    */
    if(result.hasspatialfiring){
	if(verbose){
	    fprintf(stderr,"\nEvaluating spatial firing distributions...");
	}
	/*
	** allocate correlation grids
	*/
	AllocateSpatialGrids(&result);

	/*
	** compute the magnitude of the position grid vectors
	*/
	ComputeMagnitudeGrid(&result);

	/*
	** compute the peak of the position grid vectors
	*/
	ComputePositionPeak(&result);

	/*
	** normalize the each component of the position grid vectors by
	** the peak rate
	*/
	NormalizePositionGridVectors(&result);

	/*
	** compute the normalized magnitude of the position grid vectors
	*/
	ComputeNormalizedMagnitudeGrid(&result);

	/*
	** compute the sparsity of the position grid vectors
	*/
	ComputeSparsity(&result);

	/*
	** if grid sparsity output is selected then dump it
	*/
	if(result.fpsparsegridout){
	    WriteSparsity(&result);
	}
	/*
	** if grid magnitude output is selected then dump it
	*/
	if(result.fpmagout){
	    WriteGridMagnitude(&result);
	}

	if(verbose){
	    fprintf(stderr,"done\n");
	}
    }

    /*
    ************************* CRUNCH *************************
    **
    ** OK. Finished with all the data preparation, now onto the 
    ** crunching
    **
    **********************************************************
    */

    if(result.bayesian){
	if(verbose){
	    fprintf(stderr,"Processing Bayesian reconstruction...");
	}
	ComputeBayesianDistributions(&result);
	ComputeBayesianReconstruction(&result);
	fprintf(stderr,"done\n");
    }

    /*
    ** spike train output
    */
    if(result.fpspiketrainout){
	if(verbose){
	    fprintf(stderr,"Processing spike train output...");
	}
	ProcessSpikeTrain(&result);
	fprintf(stderr,"done\n");
    }

    /*
    ** create the full spike cross correlation distribution
    */
    if(result.spike_correlations){
	if(verbose){
	    fprintf(stderr,"\nComputing spike correlations...\n");
	    fprintf(stderr,"Working on component      ");
	}
	for(i=0;i<result.nclusters;i++){
	    if(clusterdir[i].ignore) continue;
	    if(verbose){
		fprintf(stderr,"%3d\b\b\b",i);
	    }
	    if((corr_result = 
	    ComputeCorrelation(&result,i,i,NULL,0)) == NULL){
		fprintf(stderr,
		"ERROR: unable to evaluate autocorrelation for %d\n",i);
		testval = 0;
	    } else {
		testval = corr_result->zerocorr*result.corrtest;
	    }
	    if(!corr_result->valid && !result.showallcorrs) continue;

	    /*
	    ** put in the autocorrelation 
	    result.corrmatrix[i][i] = 1;
	    for(j=0;j<result.nclusters;j++){
	    */
	    /*
	    ** only do the upper half of the matrix
	    */
	    for(j=i+1;j<result.nclusters;j++){
		if(clusterdir[j].ignore) continue;
		if(i==j) continue;
		if(corr_result = ComputeCorrelation(&result,i,j,result.fpspikecorr,1)){
		    if(!corr_result->valid) continue;
		    /*
		    ** if correlation testing is selected then
		    ** look for unusually large correlations
		    ** Note that it is looking at spike counts not
		    ** actual r values
		    */
		    if(corr_result->zerocorr > testval){
			fprintf(stderr,
			"WARNING: unusually large xcorr detected between %d and %d (%g vs %d) (>%g)\n",
			i,j,corr_result->zerocorr,testval,result.corrtest);
			if(result.fperr){
			    fprintf(result.fperr,
			    "WARNING: unusually large xcorr detected between %d and %d (%g vs %d) (>%g)\n",
			    i,j,corr_result->zerocorr,testval,result.corrtest);
			}
		    }
		    result.corrmatrix[i][j] = (corr_result->zerocorr -
		    corr_result->expected_corr)/corr_result->sd_corr;
		}

	    }
	}
	if(result.fpspikecorr){
	    fclose(result.fpspikecorr);
	}
	/*
	** was sorting of the correlations requested
	*/
	if(result.sortcorr){
	    SortCorrelations(&result);
	}
    }
    /*
    ** create the spike/field replay output
    */
    if(result.replay_fields){
	if(verbose){
	    fprintf(stderr,"\nReplaying population spatial fields...\n");
	}
	ReplayFields(&result);
	if(verbose){
	    fprintf(stderr,"done\n");
	}
    }
    /*
    ** create the trajectory tree
    */
    if(result.make_trajectory_tree){
	if(verbose){
	    fprintf(stderr,"\nCreating population trajectory tree\n");
	}
	CreateTrajectoryTree(&result,result.requested_node_id);
	if(verbose){
	    fprintf(stderr,"Writing trajectory tree...");
	    if(result.requested_node_id >=0){
		fprintf(stderr,"node %d...",result.requested_node_id);
	    }
	}
	WriteTrajectoryTree(&result,result.requested_node_id);
	if(verbose){
	    fprintf(stderr,"done\n");
	}
    }

    if(result.fpcorrmatrixout){
	if(verbose){
	    fprintf(stderr,"Writing correlation matrix...\n");
	}
	/*
	** output the correlation matrix
	*/
	WriteCorrelationMatrix(&result);
    }
    if(result.fpout){
	if(verbose){
	    fprintf(stderr,"Writing vectors...\n");
	}
	/*
	** output the vectors for each time bin
	*/
	WriteVectors(&result);
    }
    /*
    ** if position result.reconstruction is selected then do it
    */
    if(result.reconstruct && ! result.bayesian){
	if(verbose){
	    fprintf(stderr,"Reconstructing positions...\n");
	}
	ReconstructPosition(&result);

	if(result.hasposition){
	    if(verbose){
		fprintf(stderr,"Evaluating error distributions...\n");
	    }
	    EvaluateErrors(&result,argc,argv);

	    /*
	    ** if grid position error output is selected then dump it
	    */
	    WritePositionErrorGrid(&result);
	}
    }
    if(result.fpstatout){
	WriteClusterStats(&result);
    }
    /*
    if(result.fpspeedout){
	WriteRunningSpeed(&result);
    }
    */
    if(verbose){
	fprintf(stderr,"Done\n");
    }
    exit(0);
}

