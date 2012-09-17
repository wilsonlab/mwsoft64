#include "behav_ext.h"

void AllocateArrays(result)
Result	*result;
{
int	i;

    result->vector = (float *)calloc(MAXTHETA,sizeof(float));
    result->vectorn = (int *)calloc(MAXTHETA,sizeof(int));
    result->vectortotaln = (int *)calloc(MAXTHETA,sizeof(int));
    result->vectorsumsqr = (float *)calloc(MAXTHETA,sizeof(float));
    result->grid = (float **)calloc(result->xsize,sizeof(float *));
    result->gridn = (int **)calloc(result->xsize,sizeof(int *));
    result->gridtotaln = (int **)calloc(result->xsize,sizeof(int *));
    result->gridsumsqr = (float **)calloc(result->xsize,sizeof(float *));
    result->field_estimate = (float **)calloc(result->xsize,sizeof(float *));
    result->occupancy_gridsqr = (float **)calloc(result->xsize,sizeof(float *));
    result->occupancy_grid = (float **)calloc(result->xsize,sizeof(float *));
    result->occupancy_gridn = (int **)calloc(result->xsize,sizeof(int *));
    for(i=0;i<result->xsize;i++){
	result->grid[i] = (float *)calloc(result->ysize,sizeof(float));
	result->gridsumsqr[i] = (float *)calloc(result->ysize,sizeof(float));
	result->gridn[i] = (int *)calloc(result->ysize,sizeof(int));
	result->gridtotaln[i] = (int *)calloc(result->ysize,sizeof(int));
	result->field_estimate[i] = (float *)calloc(result->ysize,sizeof(float));
	result->occupancy_gridsqr[i]=(float *)calloc(result->ysize,sizeof(float));
	result->occupancy_grid[i] = (float *)calloc(result->ysize,sizeof(float));
	result->occupancy_gridn[i] = (int *)calloc(result->ysize,sizeof(int));
    }
}
