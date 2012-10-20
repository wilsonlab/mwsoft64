#define COPYRIGHT "Copyright (c) 1992-2005 Matthew Wilson, MIT"
#define INVALID	0
#define CHAR	1
#define SHORT	2
#define INT	3
#define FLOAT	4
#define DOUBLE	5
#define FUNC	6
#define FFUNC	7
#define ULONG	8

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define next_arg_is(s)	(strcmp(argv[++nxtarg],s) == 0)
#define arg_is(s)	(strcmp(argv[nxtarg],s) == 0)
#define iarg()	(atoi(argv[++nxtarg]))
#define farg()	(atof(argv[++nxtarg]))
#define arg_starts_with(c)	(argv[nxtarg][0] == c)

/* long    random (); */
#ifndef MAXLONG
#define MAXLONG		    2147483641L
#endif
#define frandom(l,h)    (((float)random()/MAXLONG)*((h)-(l))+(l))

typedef unsigned char   BYTE;
typedef int	(*PFI)();
typedef float	(*PFF)();

#define	ARCH_UNKNOWN	0
#define	ARCH_I386	1
#define ARCH_SPARC	2
