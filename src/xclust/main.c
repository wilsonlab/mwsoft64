/*
** xclust3 - program to graph sets of spike parameter data under the
**           X11 windowing system and apply boundaries for cluster 
**           assignment
**		  
**		Copyright 1992  Matt Wilson 
**		University of Arizona
**		Tucson, AZ 85724
**		wilson@nsma.arizona.edu
**
** Permission to use, copy, modify, and distribute this software and
** its documentation for any purpose and without fee is hereby
** granted, provided that the above copyright notice appear in all
** copies and that both that copyright notice and this permission
** notice appear in supporting documentation, and that the name of the
** University of Arizona not be used in advertising or publicity
** pertaining to distribution of the software without specific,
** written prior permission.  Neither the author nor University of
** Arizona make any representations about the suitability of this
** software for any purpose. It is provided "as is" without express or
** implied warranty.
*/

#include	"xclust.h"
extern int WhitePixelIdx();
extern int BlackPixelIdx();

main(argc, argv,envp)
int	argc;
char	**argv;
{
TextWindow	*T;
MenuWindow	*M;
MenuWindow	*C;
MenuWindow	*CO;
MenuContainer   *MC;

    F = (Frame *)calloc(1,sizeof(Frame));
    F->mapped = 1;
    F->height = 200;
    F->width = 200;
    F->wheight = 200;
    F->wwidth = 200;

    G = (Graph *)calloc(1,sizeof(Graph));
    G->frame = F;
    G->mapped = 1;
    G->fontname = DEFAULTFONT;
    G->overlay = 0;
    G->defaults = (float *)calloc(MAXDEFAULTS,sizeof(float));

    T = (TextWindow *)calloc(1,sizeof(TextWindow));
    T->frame = F;
    T->mapped = 1;
    T->fontname = DEFAULTFONT;

    MC = (MenuContainer*)calloc(1, sizeof(MenuContainer));
    MC->frame = F;
    MC->next = NULL;
    MC->menuframe = NULL;
    MC->mapped = 1;
    MC->fontname = DEFAULTFONT;

    PopulateMenuContainer(MC);

/*     M = (MenuWindow *)calloc(1,sizeof(MenuWindow)); */
/*     strcpy(M->line,"MENU"); */
/*     M->frame = F; */
/*     M->mapped = 1; */
/*     M->fontname = DEFAULTFONT; */

/*     C = (MenuWindow *)calloc(1,sizeof(MenuWindow)); */
/*     strcpy(C->line,"CLUSTSTAT"); */
/*     C->frame = F; */
/*     C->mapped = 0; */
/*     C->fontname = DEFAULTFONT; */

/*     CO = (MenuWindow *)calloc(1,sizeof(MenuWindow)); */
/*     strcpy(CO->line,"COLORSTAT"); */
/*     CO->frame = F; */
/*     CO->mapped = 0; */
/*     CO->fontname = DEFAULTFONT; */

    F->graph = G;
    F->text = T;
    F->menucontainer = MC;
/*     F->menu = M; */
/*     F->cluststat = C; */
/*     F->colorstat = CO; */
    /*
    F->fontname = DEFAULTFONT;
    */

    InitializeDefaults(G);
    ParseArgList(F,argc,argv);
    if(F->mapped){
	InitX(F);
    }

    G->defaults[FOREGROUND] = WhitePixelIdx();
    G->defaults[BACKGROUND] = BlackPixelIdx();

    LoadGraphData(G);
    InitGraph(G);
    InitMenu(MC);
    ReadDefaults(F);
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
