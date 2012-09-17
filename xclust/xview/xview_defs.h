
#define setchar(i,j,s)  for (k=i; k<= j; k++) ch[k] = s 
#define mvp(x,y,p) move(y,x); printw("%s",p)

#define MAXASC 		50
#define VERSION		"2.4"

#define MGP		0
#define TEXT		1
#define DEFAULTMIN	-80
#define DEFAULTMAX	20
#define HEADER_SIZE	(3 * sizeof (int) + sizeof (float))
#define STATOFS 		140
#define FILLEDBOX		0
#define BOX			1
#define COLORBOX		2
#define SURFACE			3
#define COLORSURFACE		4
#define FILLEDSURFACE		5
#define COLORFILLEDSURFACE	6
#define CONTOUR			7
#define COLORCONTOUR		8
#define NUMBERBOX		9
#define POINT		10
#define GRAYSCALE 0
#define SPECTRALSCALE 1
#define RGRAYSCALE 2
#define SQUEEZESCALE 4
#define FILESCALE 5


#define AREA		0
#define LENGTH		1

#define MAXTITLES	10	
#define MAXCOLORS	256	
#define MAXCONTOURS	500

#define SHIFT 		5

#define FONT_FOR_TEXT		"6x13p"

#define WHITE 0
#define BLACK 1
