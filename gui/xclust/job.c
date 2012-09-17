#include 	"xclust_ext.h"

int current_job_id = 1;
int continuous_step_job_id = 0;
int blockmovie_job_id = 0;
int intermittent_save_job_id = 0;

int AddJob(job)
Job	*job;
{
int	id;

    /*
    ** add the job to the list
    */
    job->next = F->joblist;
    F->joblist = job;
    /*
    ** assign it a job id
    */
    id = current_job_id;
    job->id = id;;
    current_job_id++;
    return(id);
}

Job *GetJob(id)
int	id;
{
Job	*job;

    for(job=F->joblist;job;job=job->next){
	if(job->id == id){
	    return(job);
	}
    }
    return(NULL);
}

DeleteJob(id)
int	id;
{
Job	*job;
Job	*prevjob;

    prevjob = NULL;
    for(job=F->joblist;job;job=job->next){
	if(job->id == id){
	    if(prevjob == NULL){
		F->joblist = job->next;
	    } else {
		prevjob->next = job->next;
	    }
	    return(1);
	}
	prevjob = job;
    }
    if(job == NULL){
	return(0);
    }
}

/*
*********************************************
** section defining the partial plot replay job
*********************************************
*/

PartialPlotLoadJob(job)
Job	*job;
{
MenuItem	*item;

    /*
    ** call the menu function with the job data
    */
    item = (MenuItem *)job->data;
    MenuPartialPlotLoad(item);
}

MenuContinuousForward(item)
MenuItem	*item;
{
Job	*newjob;

 DeleteJob(continuous_step_job_id);
 continuous_step_job_id = 0;
 if(item->state == 1){
   newjob = (Job *)calloc(1,sizeof(Job));
   newjob->func = PartialPlotLoadJob;
   newjob->data = (char *)LookupMenuItem("/controlmenu/forwardstep");
   newjob->description = "continuous forward step";
   newjob->priority = atoi(item->value);
   continuous_step_job_id = AddJob(newjob);
 }
}

MenuContinuousBackward(item)
MenuItem	*item;
{
Job	*newjob;

 DeleteJob(continuous_step_job_id);
 continuous_step_job_id = 0;    
 if(item->state == 1){
   newjob = (Job *)calloc(1,sizeof(Job));
   newjob->func = PartialPlotLoadJob;
   newjob->data = (char *)LookupMenuItem("/controlmenu/backwardstep");
   newjob->description = "continuous forward step";
   newjob->priority = atoi(item->value);
   continuous_step_job_id = AddJob(newjob);
 }
}


/*
** Job to scroll through a block of spikes in time (blockmovie)
*/

#define BLOCKSIZEI 0
#define STEPSIZEI 1
#define MOVIESTARTI 2
#define MOVIEENDI 3
#define LOOPFLAGI 4
#define TIMEUNITSFLAGI 5

BlockMovieJob(job)
Job	*job;
{
 MenuMovieFrame((MenuItem*)job->data);
}

EndMovieJob()
{
  MenuWindow *menu;

  menu = GetMenu("/epochmenu");
  DeleteJob(blockmovie_job_id);
  blockmovie_job_id = 0;
  MenuClassSetState(NULL,menu,"blockmovie",0);
  DrawMenu(menu);
}

MenuBlockMovie(item)
MenuItem	*item;
{
Job	*newjob;
int	priority;
char	data[20]; 

 priority = Atoi(GetItemValue("/epochmenu/delay")); /* how many job
						      cycles between
						      update. 0.05 sec?*/

 DeleteJob(blockmovie_job_id);
 blockmovie_job_id = 0;
 
 if(item->state == 1 &&
    SelectedPlot(G)->source->loadmode == PARTIAL_LOAD){
   /*
   ** if a block movie job is not currently on the list then go ahead
   ** and add one
   */
   newjob = (Job *)calloc(1,sizeof(Job));
   newjob->func = BlockMovieJob;
   newjob->data = (char *)item;
   newjob->description = "block movie job";
   newjob->priority = priority;
   blockmovie_job_id = AddJob(newjob); /* global */
 }
}


/*
*********************************************
** section defining the intermittent save job
*********************************************
*/

IntermittentSaveJob(job)
Job	*job;
{
char	*file;
Graph	*graph;

    if((file = job->data) == NULL){
	fprintf(stderr,"invalid temporary intermittent save file\n");
	return;
    }
    if((graph = GetGraph("/graph")) == NULL){
	fprintf(stderr,"ERROR: undefined graph\n");
	return;
    }
    /*
    ** check to see whether the bounds have been modified
    */
    if(graph->cbmodified){
	WriteAllClusterBounds(graph,file);
    }
}

ChangeIntermittentSaveDelay(delay)
int	delay;
{
Job	*job;

    /*
    ** find the current intermittent save job
    */
    if(job = GetJob(intermittent_save_job_id)){
	job->priority = delay;
	fprintf(stderr,
	"Automatic cluster bounds save interval set to ~%g min.\n",
	    job->priority*1e-2/60);
    } else {
	fprintf(stderr,"intermittent save is not active\n");
    }
}


ActivateIntermittentSave(delay)
int	delay;
{
Job	*newjob;
    
    /*
    ** if an intermittent save job is not currently on the list then go 
    ** ahead and add one
    */
    if(intermittent_save_job_id == 0){
	newjob = (Job *)calloc(1,sizeof(Job));
	newjob->func = IntermittentSaveJob;
	newjob->data = "cbfile.int";
	newjob->description = "intermittent save";
	newjob->priority = delay;
	intermittent_save_job_id = AddJob(newjob);
    }
}

DeactivateIntermittentSave()
{
    if(DeleteJob(intermittent_save_job_id) == 0){
	fprintf(stderr,"ERROR: unable to stop intermittent save job\n");
    }
    intermittent_save_job_id = 0;
}

