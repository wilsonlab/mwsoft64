#include <stdint.h>
#include <inttypes.h>

extern char	*iolibversion();
extern char	*iolibrevision();

/* Create/Read Headers */
extern char 	**ReadHeader();
extern void	BeginStandardHeader();
extern void	EndStandardHeader();
extern int16_t	AppendToHeader();
extern void	DisplayHeader();

/* Header access */
extern int16_t	GetFileType();
extern int16_t	GetFieldCount();
extern char	*GetFieldString();
extern char	*GetHeaderParameter();
extern int16_t	GetFieldInfoByNumber();
extern int16_t	GetFieldInfoByName();

/* Data access */
extern int16_t	VerifyIdentical();

/* Architecture-specific */
extern int16_t	GetFileArchitecture();
extern int16_t	GetLocalArchitecture();
extern char	*GetFileArchitectureStr();
extern char	*GetLocalArchitectureStr();
extern void	ConvertData();

/* Data Format helpers */
extern uint32_t	ParseTimestamp(char *);
extern char	*TimestampToString();
extern void	FormatTime();
extern int16_t	*ReadBinaryFormatInfo();
extern int16_t	*ReadXviewFormatInfo();
extern int16_t	sgetargs();

/* General helpers */
extern char	*TFstr();
extern int16_t	IsStringEmpty();

/* System  */
extern int16_t	BgSystemProcess();
extern int16_t	System();
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
    int16_t		column;	
    int16_t		type;
    int16_t		size;
    int16_t		count;
} FieldInfo;

