#define DATA_PRINT 		0
#define DEBUG 			0
#define MEM_PRINT 		0
#define MINIMAL_PRINT 		0

#define SYSLOG 			0
#define INET_INTERFACE 		"eth0"
#define HPID 			311
//if things aren't working from the start, or you are getting strange default values parsing files, suspect these paths, they are relative to where the progam is started from!
#define CONFIG_FILE 	"./config.txt"	//depends on where the program was started from!
#define KSEQ_FILE	"./ecom_emulator/kseqdictionary.txt"
#define CCM_FILE	"./ecom_emulator/ccmdictionary.txt"

//some debugging macros
#define Ps(x)		printf("x:%s\n",x);
#define Px(x)		printf("x:%x\n",x);
#define Pd(x)		printf("x:%d\n",x);
#define P1		printf("Print Marker 1\n");
#define P2		printf("Print Marker 2\n");
#define P3		printf("Print Marker 3\n");
#define P4		printf("Print Marker 4\n");
#define P5		printf("Print Marker 5\n");
#define sizearray(a)  	(sizeof(a) / sizeof((a)[0]))



#define P180(y)		u_char const* p = &(((HEIDevice*)*pDevice)->y); for(n = 0; n<sizeof((((HEIDevice*)*pDevice)->y));n++) printf("||"); printf("\n");for(n = 0; n<80;n++) printf("%02x", p[n]); printf("\n");

#define P280(y)		p = &(((HEIDevice*)*pDevice)->y); for(n = 0; n<80;n++) printf("%02x", p[n]); printf("\n");



#define P380(y)		u_char const* p = &(y); for(n = 0; n<sizeof(y);n++) printf("||"); printf("\n");for(n = 0; n<80;n++) printf("%02x", p[n]); printf("\n");

#define P480(y)		p = &(y); printf("\n");for(n = 0; n<80;n++) printf("%02x", p[n]); printf("\n");


#include "DEFS.H"
#include "HEI.H"









