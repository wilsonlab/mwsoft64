#include "behav_ext.h"

void AllocateArrays(result)
Result	*result;
{
int32_t	i;

    result->vector = (float *)calloc(MAXTHETA,sizeof(float));
    result->vectorn = (int32_t *)calloc(MAXTHETA,sizeof(int32_t));
    result->vectortotaln = (int32_t *)calloc(MAXTHETA,sizeof(int32_t));
    result->vectorsumsqr = (float *)calloc(MAXTHETA,sizeof(float));
    result->grid = (float **)calloc(result->xsize,sizeof(float *));
    result->gridn = (int32_t **)calloc(result->xsize,sizeof(int32_t *));
    result->gridtotaln = (int32_t **)calloc(result->xsize,sizeof(int32_t *));
    result->gridsumsqr = (float **)calloc(result->xsize,sizeof(float *));
    result->field_estimate = (float **)calloc(result->xsize,sizeof(float *));
    result->occupancy_gridsqr = (float **)calloc(result->xsize,sizeof(float *));
    result->occupancy_grid = (float **)calloc(result->xsize,sizeof(float *));
    result->occupancy_gridn = (int32_t **)calloc(result->xsize,sizeof(int32_t *));
    for(i=0;i<result->xsize;i++){
	result->grid[i] = (float *)calloc(result->ysize,sizeof(float));
	result->gridsumsqr[i] = (float *)calloc(result->ysize,sizeof(float));
	result->gridn[i] = (int32_t *)calloc(result->ysize,sizeof(int32_t));
	result->gridtotaln[i] = (int32_t *)calloc(result->ysize,sizeof(int32_t));
	result->field_estimate[i] = (float *)calloc(result->ysize,sizeof(float));
	result->occupancy_gridsqr[i]=(float *)calloc(result->ysize,sizeof(float));
	result->occupancy_grid[i] = (float *)calloc(result->ysize,sizeof(float));
	result->occupancy_gridn[i] = (int32_t *)calloc(result->ysize,sizeof(int32_t));
    }
}
