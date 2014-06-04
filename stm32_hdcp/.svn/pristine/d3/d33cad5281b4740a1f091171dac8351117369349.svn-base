#ifndef _GLOBAL_MACRO_H_B3652D8C_4F1D_45F2_AEF6_C889D9B54500
#define _GLOBAL_MACRO_H_B3652D8C_4F1D_45F2_AEF6_C889D9B54500


#define DIMM(x)		(sizeof(x)/sizeof(x[0]))

typedef unsigned char	uint8;
typedef char	int8;
typedef unsigned short	uint16;
typedef short	int16;
typedef unsigned int	uint32;
typedef int		int32;
typedef unsigned long	uint64;
typedef long	int64;

#if !defined(MIN)
#define MIN(a,b)		((a)>(b)?(b):(a))
#endif
#if !defined(MAX)
#define MAX(a,b)		((a)>(b)?(a):(b))
#endif

#if defined(DEBUG_CH)
#define dbglog(fmt,a...) printf(fmt,##a)
#else
#define dbglog(fmt,a...) 
#endif

#endif
