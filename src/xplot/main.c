/*
** Xplot - program to graph sets of xy ascii formatted data
**	   under the X11 windowing system
**		  
**		Copyright 1989  Matt Wilson 
** 		California Institute of Technology
**		wilson@smaug.cns.caltech.edu
**
** Permission to use, copy, modify, and distribute this
** software and its documentation for any purpose and without
** fee is hereby granted, provided that the above copyright
** notice appear in all copies and that both that copyright
** notice and this permission notice appear in supporting
** documentation, and that the name of the California Institute
** of Technology not be used in advertising or publicity pertaining 
** to distribution of the software without specific, written prior 
** permission.  Neither the author nor California Institute of Technology 
** make any representations about the suitability of this software for 
** any purpose. It is provided "as is" without express or implied warranty.
*/

#include	"xplot.h"
#include <signal.h>

extern int shift_signal;

main(argc, argv,envp)
int	argc;
char	**argv;
{
TextWindow	*T;
MenuWindow	*M;
extern void sigshift();

    F = (Frame *)calloc(1,sizeof(Frame));
    F->mapped = 1;
    F->height = 200;
    F->width = 200;
    F->wheight = 200;
    F->wwidth = 200;

    G = (Graph *)calloc(1,sizeof(Graph));
    G->frame = F;
    G->mapped = 1;
    G->fontname = "fixed";

    T = (TextWindow *)calloc(1,sizeof(TextWindow));
    T->frame = F;
    T->mapped = 1;
    T->fontname = "fixed";

    M = (MenuWindow *)calloc(1,sizeof(MenuWindow));
    strcpy(M->line,"MENU");
    M->frame = F;
    M->mapped = 0;
    M->fontname = "fixed";

    F->graph = G;
    F->text = T;
    F->menu = M;
    /*
    F->fontname = "fixed";
    */
    F->fontname = NULL;
    shift_signal = 0;
    signal(SIGUSR1,sigshift);

    ParseArgList(F,argc,argv);
    if(F->mapped){
	InitX(F);
    }
    LoadGraphData(G);
    InitGraph(G);
    if(F->mapped){
	RescaleFrame(F);
    } else {
	PositionSubwindows(F);
	RescaleGraph(G);
    }
    if(F->mapped){
	EventLoop(F);
    } else {
	ExecuteCommands(G);
    }
    exit(0);
}
