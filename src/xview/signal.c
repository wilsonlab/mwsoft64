#include "xview_ext.h"
#include <signal.h>
#include <sys/time.h>

void
sigadvance(sig)
int32_t sig;
{
    got_signal = 1;
    signal(SIGUSR1,sigadvance);
}

void
sigtoggle(sig)
int32_t sig;
{
    signal_step = !signal_step;
    got_signal = 1;
    fprintf(stderr,"signal step is ");
    if(signal_step){
	fprintf(stderr,"ON\n");
    } else {
	fprintf(stderr,"OFF\n");
    }
    signal(SIGUSR2,sigtoggle);
}

#ifdef OLD
Usleep(usec)
int32_t usec;
{
struct timeval  timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = usec;
    select(32,NULL,NULL,NULL,&timeout);
}
#endif
