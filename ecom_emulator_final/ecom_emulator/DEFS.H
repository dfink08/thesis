/* Some usefull stuff! */
#if !defined(_DEFS_H)
#define _DEFS_H

typedef int		BOOL;
#define FALSE	0
#define TRUE	1

//size is important because raw structs are sent over the wire
// use numbers like '64' rather than qualifiers like 'long'
// on Gumstix, a 'long' is 4bytes on Laptop, 'long' is 8bytes
typedef unsigned char	BYTE;
typedef unsigned short	WORD;	//unsigned short
typedef	u_int64_t	DWORD;	//unsigned long	


typedef unsigned int	UINT;

#define LONG long
#define ULONG_MAX     0xffffffff    /* maximum unsigned long value */

#define LOBYTE(w)	((BYTE)(w))
#define HIBYTE(w)	((BYTE)(((UINT)(w) >> 8) & 0xFF))

#define LOWORD(l)	((WORD)(DWORD)(l))
#define HIWORD(l)	((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(low, high) ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))

#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define min(a,b)	(((a) < (b)) ? (a) : (b))



#define NULL    ((void *)0)

#define TimeDiff(StartTime, EndTime)  (((DWORD)EndTime > (DWORD)StartTime) ? ((DWORD)EndTime - (DWORD)StartTime) : ((DWORD)EndTime + ((DWORD)ULONG_MAX - (DWORD)StartTime) + 1))


#endif
