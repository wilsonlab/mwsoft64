extern char	*iolibversion();
extern char	*iolibrevision();

/* Create/Read Headers */
extern char 	**ReadHeader();
extern void	BeginStandardHeader();
extern void	EndStandardHeader();
extern int	AppendToHeader();
extern void	DisplayHeader();

/* Header access */
extern int	GetFileType();
extern int	GetFieldCount();
extern char	*GetFieldString();
extern char	*GetHeaderParameter();
extern int	GetFieldInfoByNumber();
extern int	GetFieldInfoByName();

/* Data access */
extern int	VerifyIdentical();

/* Architecture-specific */
extern int	GetFileArchitecture();
extern int	GetLocalArchitecture();
extern char	*GetFileArchitectureStr();
extern char	*GetLocalArchitectureStr();
extern void	ConvertData();

/* Data Format helpers */
extern unsigned long	ParseTimestamp();
extern char	*TimestampToString();
extern void	FormatTime();
extern int	*ReadBinaryFormatInfo();
extern int	*ReadXviewFormatInfo();
extern int	sgetargs();

/* General helpers */
extern char	*TFstr();
extern int	IsStringEmpty();

/* System  */
extern int	BgSystemProcess();
extern int	System();
extern void	Usleep();


/*  VERSION 2.5 */
/*  REVISION 3.0 */

/*
** this is the magic start of header string
*/
#define MAGIC_SOH_STR "%%BEGINHEADER"
/*
** this is the magic end of header string
*/
#define MAGIC_EOH_STR "%%ENDHEADER"
/*
** this is the length of the magic start of header string %%BEGINHEADER
*/
#define MAGIC_SOH_STRSIZE	14
/*
** this is the length of the magic end of header string %%ENDHEADER
*/
#define MAGIC_EOH_STRSIZE	12

#define INVALID_TYPE	-1
#define ASCII	0
#define BINARY	1

typedef struct field_info_type {
    char	*name;	
    int		column;	
    int		type;
    int		size;
    int		count;
} FieldInfo;

