#include "pop_ext.h"

DrawArrowHead(fpout,x,y,ang,size)
FILE	*fpout;
float	x,y,ang;
float	size;
{
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x + size*sin(ang), y - size*cos(ang), 0,
	x - size*sin(ang), y + size*cos(ang), 0);
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x + size*sin(ang), y - size*cos(ang), 0,
	x + size*cos(ang), y + size*sin(ang), 0);
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x - size*sin(ang), y + size*cos(ang), 0,
	x + size*cos(ang), y + size*sin(ang), 0);
}

DrawSquareHead(fpout,x,y,ang,size)
FILE	*fpout;
float	x,y,ang;
float	size;
{
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x - size*sin(ang), y + size*cos(ang), 0,
	x + size*(2*cos(ang) - sin(ang)), y + size*(2*sin(ang) + cos(ang)), 0);
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x + size*(2*cos(ang) - sin(ang)), y + size*(2*sin(ang) + cos(ang)), 0,
	x + size*(2*cos(ang) + sin(ang)), y + size*(2*sin(ang) - cos(ang)), 0);
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x + size*(2*cos(ang) + sin(ang)), y + size*(2*sin(ang) - cos(ang)), 0,
	x + size*sin(ang), y - size*cos(ang), 0);
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x + size*sin(ang), y - size*cos(ang), 0,
	x - size*sin(ang), y + size*cos(ang), 0);
}

DrawLineHead(fpout,x,y,ang,size)
FILE	*fpout;
float	x,y,ang;
float	size;
{
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x + size*sin(ang), y - size*cos(ang), 0,
	x - size*sin(ang), y + size*cos(ang), 0);
}

DrawDiamondHead(fpout,x,y,ang,size)
FILE	*fpout;
float	x,y,ang;
float	size;
{
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x,y,0,
	x + size*(cos(ang) - sin(ang)), y + size*(sin(ang) + cos(ang)), 0);
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x + size*(cos(ang) - sin(ang)), y + size*(sin(ang) + cos(ang)), 0,
	x + size*(2*cos(ang)), y + size*(2*sin(ang)), 0);
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x + size*(2*cos(ang)), y + size*(2*sin(ang)), 0,
	x + size*(cos(ang) + sin(ang)), y + size*(sin(ang) - cos(ang)), 0);
    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
	x + size*(cos(ang) + sin(ang)), y + size*(sin(ang) - cos(ang)), 0,
	x, y, 0);
}

float ComputeAngle(x,y)
float	x,y;
{
float	ang;

    if(x == 0){
	if(y < 0){
	    ang = 3*M_PI/2;
	} else {
	    ang = M_PI/2;
	}
    } else 
    if(x < 0){
	ang = M_PI + atan(y/x);
    } else {
	ang = atan(y/x);
    }
    return(ang);
}

ReplayFields(result)
Result	*result;
{
int	i;	
FILE	*fpout;
int	bin;
float	px,py;
float	px2,py2;
float	lagpx,lagpy;
float	lagpx2,lagpy2;
double	netvector_x;
double	netvector_y;
double	netvector_ang;
double	netvector_mag;
double	vector_x;
double	vector_y;
double	vector_ang;
double	vector_mag;
double	behav_ang;
double	behav_mag;
float	bestx,besty;
double	bestang;
int	bestbin;
float	bestdot;
float	bestval;
float	bestdist;
float	val;
float	distance;
float	dotproduct;
int	q;
double	tot_vector_mag;
int	tot_vector_n;
double	totx,toty;
double	avgx,avgy;
double	oldpx,oldpy;
int	oldbin;
double 	velocity_mag;
double	velocity_ang;
int	totn;
float	*dot;
double	net_distance;
    
    /*
    ** create a root node for each component of the population
    ** vector
    */
    if(verbose){
	fprintf(stderr,"Working on time bin          ");
    }
    fpout = result->fpreplayout;
    /*
    ** allocate some arrays
    */
    if(result->position_bin_search > 0){
	dot = (float *)calloc(result->position_bin_search*2 +1,sizeof(float));
	result->valcorr = (float *)calloc(result->position_bin_search*2 +1,
	sizeof(float));
	result->nvalcorr = (int *)calloc(result->position_bin_search*2 +1,
	sizeof(int));
    }
    /*
    ** now walk through the time series for the vector component
    */
    oldpx = 0;
    oldpy = 0;
    oldbin = 0;
    for(bin=0;bin < result->ntimebins;bin++){
	if(verbose){
	    fprintf(stderr,"\b\b\b\b\b\b\b\b%7d ",bin);
	}
	fprintf(fpout,"/newplot\n",bin);
	fprintf(fpout,"/plotname %d\n",bin);
	if(result->hasposition){
	    /*
	    ** keep track of actual xy position if
	    ** available
	    ** convert from actual coords to grid coords
	    */
	    GetConvertedPosition(result,bin,&px,&py,&px2,&py2);
	    /*
	    ** output the position of the front diode
	    ** use cluster id -1
	    */
	    fprintf(fpout,"%g %g %g %d\n",px,py,1.0,-1);
	    /*
	    ** try to output the position of the back diode
	    ** use cluster id -2
	    */
	    if(px2 > 0){
		fprintf(fpout,"%g %g %g %d\n",
		    px2,py2,0.25,-2);
		/*
		** and draw a line between the front and back diodes
		*/
		fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
		    px,py,0,
		    px2,py2,0);
	    }
	    if(oldbin > 0){
		/*
		** compute the velocity
		*/
		velocity_mag = sqrt((double)((px - oldpx)*(px - oldpx) + 
		    (py - oldpy)*(py - oldpy)))/(bin - oldbin);
		velocity_ang = ComputeAngle(px - oldpx,py - oldpy);
	    } else {
		velocity_mag = -1;
	    }
	    if(px > 0){
		oldpx = px;
		oldpy = py;
		oldbin = bin;
	    }
	}
	/*
	** go through each cluster looking for activity during the
	** current time bin
	*/
	netvector_x = 0;
	netvector_y = 0;
	tot_vector_n = 0;
	tot_vector_mag = 0;
	for(i = 0;i < result->nclusters;i++){
	    if(clusterdir[i].ignore) continue;
	    /*
	    ** only process sufficiently active clusters
	    */
	    if(result->datavector[bin][i] > result->replay_threshold){
		/*
		** compute the resultant spatial field vector doing
		** a little vector addition based on
		** the current position and the field centers 
		** of all active clusters
		*/
		if(result->hasposition){
		    /*
		    ** make sure the field center is defined
		    */
		    if(result->peakloc[i].x >= 0){
			/*
			** compute the vector angle
			*/
			vector_x = result->peakloc[i].x - px;
			vector_y = result->peakloc[i].y - py;
			vector_ang = ComputeAngle(vector_x,vector_y);
			/*
			** keep track of the number of vectors in
			** the resultant
			*/
			tot_vector_n++;
			/*
			** compute magnitude using the rate-weighted
			** vector
			*/
			vector_mag = result->datavector[bin][i];
			tot_vector_mag += vector_mag;
			/*
			** now add it in the net vector
			netvector_x += cos(vector_ang)*vector_mag;
			netvector_y += sin(vector_ang)*vector_mag;
			*/
			netvector_x += vector_x*vector_mag;
			netvector_y += vector_y*vector_mag;
		    }
		}
		if(!result->suppress_replay_firing){
		    /*
		    ** output the position of the field center and the number
		    ** of spikes in the bin
		    */
		    fprintf(fpout,"%d %d %g %d\n",
		    result->peakloc[i].x,
		    result->peakloc[i].y,
		    result->datavector[bin][i],
		    i);
		}
		
	    }
	}
	if(result->hasposition && px > 0){
	    /*
	    ** normalize the resultant vector components
	    */
	    netvector_x /= tot_vector_mag;
	    netvector_y /= tot_vector_mag;
	    /*
	    ** compute the resultant vector angle
	    */
	    netvector_ang = ComputeAngle(netvector_x,netvector_y);
	    /*
	    ** and magnitude
	    */
	    netvector_mag = sqrt((double)((netvector_x*netvector_x) + 
		(netvector_y*netvector_y)));
	    if(netvector_mag > 0){
		/*
		** output the resultant field vector
		*/
		fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
		    px,py,0,netvector_x + px,netvector_y + py,0);
		/*
		** draw a little arrow at the end
		*/
		DrawArrowHead(fpout,netvector_x+px,netvector_y+py,
		    netvector_ang,0.75);
	    }
	    if(result->position_bin_offset != 0 && px > 0){
		/*
		** compute the behavioral trajectory vector from the current
		** position to the position offset by the requested number
		** of bins
		*/
		GetConvertedPosition(result,bin + result->position_bin_offset,
		    &lagpx,&lagpy,&lagpx2,&lagpy2);
		if(lagpx > 0){
		    /*
		    ** get the vector angle
		    */
		    behav_ang = ComputeAngle(lagpx - px,lagpy - py);
		    /*
		    ** draw a line from the current position to the
		    ** offset position
		    */
		    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
			px,py,0,lagpx,lagpy,0);
		    /*
		    ** draw a little square at the end
		    */
		    DrawSquareHead(fpout,lagpx,lagpy,behav_ang,0.35);
		}
	    } else 
	    if(result->position_bin_avg != 0 && px > 0){
		/*
		** compute the mean behavioral trajectory vector over
		** the requested number of bins before or after
		** the current position including the current position
		*/
		totx = 0;
		toty = 0;
		totn = 0;
		if(result->position_bin_avg > 0){
		    /*
		    ** search future bins
		    */
		    for(q=0;q<result->position_bin_avg;q++){
			GetConvertedPosition(result,bin + q,
			&lagpx,&lagpy,&lagpx2,&lagpy2);
			if(lagpx > 0){
			    /*
			    ** keep a running total
			    */
			    totx += lagpx;
			    toty += lagpy;
			    totn++;
			}
		    }
		} else 
		if(result->position_bin_avg < 0){
		    /*
		    ** search prior bins
		    */
		    for(q=0;q> result->position_bin_avg;q--){
			GetConvertedPosition(result,bin + q,
			&lagpx,&lagpy,&lagpx2,&lagpy2);
			if(lagpx > 0){
			    /*
			    ** keep a running total
			    */
			    totx += lagpx;
			    toty += lagpy;
			    totn++;
			}
		    }
		}  
		if(totn > 0){
		    /*
		    ** get the vector angle
		    */
		    avgx = totx/totn;
		    avgy = toty/totn;
		    behav_ang = ComputeAngle(avgx - px,avgy- py);
		    behav_mag = sqrt((double)((avgx-px)*(avgx-px) + 
		    (avgy-py)*(avgy-py)));
		    /*
		    ** compute the dot product between the mean vector
		    ** and the resultant field vector
		    */
		    if(netvector_mag > 0 && behav_mag>0){
			dotproduct = ((avgx - px)*(netvector_x) + 
			    (avgy-py)*(netvector_y))/
			    (netvector_mag*behav_mag);
			fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
			px,py,0,avgx,avgy,0);
			fprintf(fpout,"/plotstext %d %d %g\n",
			    50,50,dotproduct);
			/*
			** draw a little square at the end
			*/
			DrawSquareHead(fpout,avgx,avgy,behav_ang,0.5);
			if(result->fpreplaydot){
			    net_distance = sqrt(
				(double)((avgx-px-netvector_x)*
				(avgx-px-netvector_x) +
				(avgy-py-netvector_y)*
				(avgy-py-netvector_y)));
			    /*
			    ** output the dot product and optimal lag to
			    ** a file
			    */
			    fprintf(result->fpreplaydot,"%d\t%g\t%g\t%g\n",
			    bin,dotproduct,net_distance,
			    net_distance/netvector_mag);
			}
		    }
		}
	    } else
	    if(result->position_bin_search > 0 && px > 0){
		/*
		** look for the behavioral trajectory vector which is 
		** closest to the resultant field vector as measured
		** by the dot product
		*/
		bestbin = 0;
		bzero(dot,sizeof(float)*(2*result->position_bin_search +1));
		/*
		** search future bins
		*/
		for(q=result->position_bin_search;q>0;q--){
		    GetConvertedPosition(result,bin + q,
		    &lagpx,&lagpy,&lagpx2,&lagpy2);
		    if(lagpx > 0){
			/*
			** get the vector angle
			*/
			behav_ang = ComputeAngle(lagpx - px,lagpy - py);
			behav_mag = sqrt((double)((lagpx - px)*(lagpx - px) + 
			(lagpy - py)*(lagpy - py)));
			if(netvector_mag > 0 && behav_mag>0){
			    /*
			    ** compute the measure to optimize
			    */
			    /*
			    ** and compute the dot product between
			    ** the resultant vector and the future
			    ** positional vector
			    */
			    dotproduct = ((lagpx - px)*(netvector_x) + 
				(lagpy - py)*(netvector_y))/
				(netvector_mag*behav_mag);
			    /*
			    ** compute the distance between the 
			    ** resultant vector and the future positional
			    ** vector
			    */
			    distance = sqrt((double)((lagpx - px - 
				netvector_x)*(lagpx - px - netvector_x) +
				(lagpy - py - netvector_y)*
				(lagpy - py - netvector_y)));
			    if(result->replay_optimize == OPTIMIZE_ANGLE){
				val = dotproduct;
			    } else
			    if(result->replay_optimize == OPTIMIZE_DISTANCE){
				/*
				** use the fractional change in distance
				** with respect to the resultant vector
				*/
				val = distance/netvector_mag;
			    }
			    /*
			    ** put the value of the measure it in the lag array
			    */
			    dot[result->position_bin_search + q] = val;
			    result->valcorr[result->position_bin_search + q] +=
				val;
			    result->nvalcorr[result->position_bin_search + q]++;
			    if((bestbin == 0) ||
			    (result->replay_optimize == OPTIMIZE_ANGLE &&
			    val >= bestval) || 
			    (result->replay_optimize == OPTIMIZE_DISTANCE &&
			    val <= bestval)){
				bestbin = q; 
				bestval = val;
				bestdist = distance;
				bestdot = dotproduct;
				bestx = lagpx;
				besty = lagpy;
				bestang = behav_ang;
			    }
			}
		    }
		}
		/*
		** find out which was best and
		** draw the vector
		*/
		if(bestbin != 0){
		    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
		    px,py,0,bestx,besty,0);
		    fprintf(fpout,"/plotstext %d %d %d:%g\n",
			50,50,bestbin,bestval);
		}
		/*
		** output the dot product and optimal lags to
		** a file
		*/
#ifdef OLD
		if(bestbin != 0 && result->fpreplaydot){
		    for(q=result->position_bin_search;q>0;q--){
			/*
			** dump all the best lags. This deals with
			** any ties that may occur
			*/
			if(lagval[result->position_bin_search + q] >= bestval){
			    fprintf(result->fpreplaydot,"%d %d %g\n",
			    bin,q,bestval);
			}
		    }
		}
#else
		if(bestbin != 0 && result->fpreplaydot){
		    /*
		    ** output statistics to the file
		    */
		    fprintf(result->fpreplaydot,"%d\t%d\t%g\t%g\t%g\t%g\t%g\n",
		    bin,bestbin,bestdot,sqrt((bestx -px)*(bestx - px) + 
		    (besty-py)*(besty-py)),
		    bestdist,
		    bestdist/netvector_mag,velocity_mag);
		}
#endif
		if(bestbin > 0){
		    /*
		    ** draw a little square at the end
		    */
		    DrawSquareHead(fpout,bestx,besty,bestang,0.75);
		}  
		bestbin = 0;
		bzero(dot,sizeof(float)*(2*result->position_bin_search +1));
		/*
		** search previous bins
		*/
		for(q= -result->position_bin_search;q<0;q++){
		    GetConvertedPosition(result,bin + q,
		    &lagpx,&lagpy,&lagpx2,&lagpy2);
		    if(lagpx > 0){
			/*
			** get the vector angle
			*/
			behav_ang = ComputeAngle(lagpx - px,lagpy - py);
			behav_mag = sqrt((double)((lagpx-px)*(lagpx-px) + 
			(lagpy-py)*(lagpy-py)));
			/*
			** and compute the dot product
			*/
			if(netvector_mag > 0 && behav_mag>0){
			    /*
			    ** compute the distance between the 
			    ** resultant vector and the future positional
			    ** vector
			    */
			    distance = sqrt((double)((lagpx - px - 
				netvector_x)*(lagpx - px - netvector_x) +
				(lagpy - py - netvector_y)*
				(lagpy - py - netvector_y)));
			    /*
			    ** and compute the dot product between
			    ** the resultant vector and the future
			    ** positional vector
			    */
			    dotproduct = ((lagpx - px)*(netvector_x) + 
				(lagpy-py)*(netvector_y))/
				(netvector_mag*behav_mag);
			    if(result->replay_optimize == OPTIMIZE_ANGLE){
				val = dotproduct;
			    } else
			    if(result->replay_optimize == OPTIMIZE_DISTANCE){
				/*
				** use the fractional change in distance
				** with respect to the resultant vector
				*/
				val = distance/netvector_mag;
			    }
			    /*
			    ** put it in the dot product lag array
			    */
			    dot[result->position_bin_search + q] = val;
			    result->valcorr[result->position_bin_search + q] +=
				val;
			    result->nvalcorr[result->position_bin_search + q]++;
			    if((bestbin == 0) ||
			    (result->replay_optimize == OPTIMIZE_ANGLE &&
			    val >= bestval) || 
			    (result->replay_optimize == OPTIMIZE_DISTANCE &&
			    val <= bestval)){
				bestbin = q; 
				bestdot = dotproduct;
				bestdist = distance;
				bestval = val;
				bestx = lagpx;
				besty = lagpy;
				bestang = behav_ang;
			    }
			}
		    }
		}
		/*
		** find out which was best and
		** draw the vector
		*/
		if(bestbin != 0){
		    fprintf(fpout,"/plotwline %g %g %d %g %g %d\n",
		    px,py,0,bestx,besty,0);
		    fprintf(fpout,"/plotstext %d %d %d:%g\n",
			50,70,bestbin,bestval);
		}
		if(bestbin < 0){
		    /*
		    ** draw a little line at the end
		    */
		    DrawLineHead(fpout,bestx,besty,bestang,0.5);
		}
		/*
		** output the dot product and optimal lags to
		** a file
		*/
#ifdef OLD
		if(bestbin != 0 && result->fpreplaydot){
		    for(q= -result->position_bin_search;q<0;q++){
			/*
			** dump all the best lags. This deals with
			** any ties that may occur
			*/
			if(dot[result->position_bin_search + q] >= bestdot){
			    fprintf(result->fpreplaydot,"%d %d %g\n",
			    bin,q,bestdot);
			}
		    }
		}
#else
		if(bestbin != 0 && result->fpreplaydot){
		    fprintf(result->fpreplaydot,"%d\t%d\t%g\t%g\t%g\t%g\t%g\n",
		    bin,bestbin,bestdot,
		    sqrt((double)((bestx -px)*(bestx - px)+ 
		    (besty-py)*(besty-py))),bestdist,
		    bestdist/netvector_mag,velocity_mag);
		}
#endif
	    }
	}
    }
    /*
    ** if correlation output was selected then dump the correlation
    ** array
    */
    if(result->position_bin_search > 0 && result->fpreplaycorr){
	for(q= -result->position_bin_search;q<result->position_bin_search;q++){
	    if(result->nvalcorr[result->position_bin_search + q] > 0){
		fprintf(result->fpreplaycorr,"%d %g\n",q,
		result->valcorr[result->position_bin_search + q]/
		result->nvalcorr[result->position_bin_search + q]);
	    }
	}
    }
}
